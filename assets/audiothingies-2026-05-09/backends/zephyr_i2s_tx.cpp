#include "audio/backends/zephyr_i2s_tx.hpp"

#include <zephyr/device.h>
#include <zephyr/logging/log.h>

#include <algorithm>

LOG_MODULE_REGISTER(zephyr_i2s_tx, LOG_LEVEL_INF);

namespace stem_player::audio {
namespace {

constexpr size_t kInitialPrimeBlocks = 2U;
constexpr k_timeout_t kStopJoinTimeout = K_MSEC(500);

}  // namespace

ZephyrI2sTx::ZephyrI2sTx() {
    for (auto& tag : buffer_tags_) {
        tag.store(0U, std::memory_order_relaxed);
    }
    for (auto& submitted : buffer_submitted_) {
        submitted.store(false, std::memory_order_relaxed);
    }
}

bool ZephyrI2sTx::start(const Config& config, const Client& client) {
    if (started_.load(std::memory_order_acquire)) {
        return false;
    }
    if (config.dev == nullptr || !device_is_ready(config.dev)) {
        LOG_ERR("I2S device not ready");
        return false;
    }
    if (client.fill_tx_words == nullptr) {
        return false;
    }

    config_ = config;
    client_ = client;
    stop_requested_.store(false, std::memory_order_relaxed);
    healthy_.store(false, std::memory_order_relaxed);
    refill_drops_.store(0U, std::memory_order_relaxed);

    k_mem_slab_init(&tx_mem_slab_,
                    tx_buffers_.data(),
                    kBlockSizeBytes,
                    kBufferCount);
    for (auto& tag : buffer_tags_) {
        tag.store(0U, std::memory_order_relaxed);
    }
    for (auto& submitted : buffer_submitted_) {
        submitted.store(false, std::memory_order_relaxed);
    }

    started_.store(true, std::memory_order_release);
    k_thread_create(&thread_,
                    thread_stack_,
                    K_THREAD_STACK_SIZEOF(thread_stack_),
                    &ZephyrI2sTx::thread_entry,
                    this, nullptr, nullptr,
                    config_.thread_priority,
                    K_FP_REGS,
                    K_NO_WAIT);
    k_thread_name_set(&thread_, "i2s_refill");
    return true;
}

void ZephyrI2sTx::stop() {
    if (!started_.load(std::memory_order_acquire)) {
        return;
    }

    stop_requested_.store(true, std::memory_order_release);
    if (config_.dev != nullptr && device_is_ready(config_.dev)) {
        (void)i2s_trigger(config_.dev, I2S_DIR_TX, I2S_TRIGGER_DROP);
    }
    (void)k_thread_join(&thread_, kStopJoinTimeout);
}

bool ZephyrI2sTx::running() const {
    return started_.load(std::memory_order_acquire);
}

bool ZephyrI2sTx::healthy() const {
    return healthy_.load(std::memory_order_relaxed);
}

uint32_t ZephyrI2sTx::refill_drops() const {
    return refill_drops_.load(std::memory_order_relaxed);
}

void ZephyrI2sTx::reset_refill_drops() {
    refill_drops_.store(0U, std::memory_order_relaxed);
}

uint32_t ZephyrI2sTx::slab_num_used() const {
    return k_mem_slab_num_used_get(const_cast<struct k_mem_slab*>(&tx_mem_slab_));
}

uint32_t ZephyrI2sTx::slab_num_free() const {
    return k_mem_slab_num_free_get(const_cast<struct k_mem_slab*>(&tx_mem_slab_));
}

uint32_t ZephyrI2sTx::slab_max_used() const {
    return k_mem_slab_max_used_get(const_cast<struct k_mem_slab*>(&tx_mem_slab_));
}

uint32_t ZephyrI2sTx::slab_num_blocks() const {
    return kBufferCount;
}

void ZephyrI2sTx::thread_entry(void* p1, void*, void*) {
    auto* self = static_cast<ZephyrI2sTx*>(p1);
    if (self != nullptr) {
        self->thread_main();
    }
}

bool ZephyrI2sTx::configure_i2s() {
    struct i2s_config cfg = {};
    cfg.word_size = config_.word_size;
    cfg.channels = config_.channels;
    cfg.format = I2S_FMT_DATA_FORMAT_I2S;
    cfg.options = I2S_OPT_BIT_CLK_SLAVE | I2S_OPT_FRAME_CLK_SLAVE;
    cfg.frame_clk_freq = config_.frame_rate_hz;
    cfg.mem_slab = &tx_mem_slab_;
    cfg.block_size = kBlockSizeBytes;
    cfg.timeout = config_.timeout_ms;

    const int ret = i2s_configure(config_.dev, I2S_DIR_TX, &cfg);
    if (ret < 0) {
        LOG_ERR("I2S configure failed: %d", ret);
        return false;
    }
    return true;
}

void ZephyrI2sTx::fill_block(uint8_t index) {
    if (index >= kBufferCount || client_.fill_tx_words == nullptr) {
        return;
    }

    if (client_.tx_tag != nullptr) {
        buffer_tags_[index].store(client_.tx_tag(client_.user), std::memory_order_relaxed);
    }
    client_.fill_tx_words(tx_buffers_[index].data(), kWordsPerBuffer, client_.user);
}

void ZephyrI2sTx::notify_health(bool healthy) {
    if (client_.on_tx_health_changed != nullptr) {
        client_.on_tx_health_changed(healthy, client_.user);
    }
}

uint8_t ZephyrI2sTx::block_index_for_ptr(const void* mem_block) const {
    for (uint8_t i = 0; i < kBufferCount; ++i) {
        if (tx_buffers_[i].data() == mem_block) {
            return i;
        }
    }
    return kBufferCount;
}

bool ZephyrI2sTx::queue_block(void* mem_block, uint8_t index) {
    if (mem_block == nullptr || index >= kBufferCount) {
        return false;
    }

    fill_block(index);
    const int ret = i2s_write(config_.dev, mem_block, kBlockSizeBytes);
    if (ret < 0) {
        LOG_ERR("I2S write failed: %d", ret);
        k_mem_slab_free(&tx_mem_slab_, mem_block);
        refill_drops_.fetch_add(1U, std::memory_order_relaxed);
        return false;
    }

    buffer_submitted_[index].store(true, std::memory_order_relaxed);
    return true;
}

void ZephyrI2sTx::thread_main() {
    if (!configure_i2s()) {
        started_.store(false, std::memory_order_release);
        return;
    }

    for (size_t i = 0; i < std::min(kInitialPrimeBlocks, kBufferCount); ++i) {
        void* mem_block = nullptr;
        if (k_mem_slab_alloc(&tx_mem_slab_, &mem_block, K_NO_WAIT) != 0) {
            LOG_ERR("I2S initial slab alloc failed at %u", static_cast<unsigned>(i));
            started_.store(false, std::memory_order_release);
            return;
        }

        const uint8_t index = block_index_for_ptr(mem_block);
        if (!queue_block(mem_block, index)) {
            started_.store(false, std::memory_order_release);
            return;
        }
    }

    const int start_ret = i2s_trigger(config_.dev, I2S_DIR_TX, I2S_TRIGGER_START);
    if (start_ret < 0) {
        LOG_ERR("I2S start trigger failed: %d", start_ret);
        started_.store(false, std::memory_order_release);
        return;
    }

    healthy_.store(true, std::memory_order_relaxed);
    notify_health(true);
    LOG_INF("Zephyr I2S TX stream started");

    while (!stop_requested_.load(std::memory_order_acquire)) {
        void* mem_block = nullptr;
        const int alloc_ret =
            k_mem_slab_alloc(&tx_mem_slab_, &mem_block, K_MSEC(config_.timeout_ms));
        if (stop_requested_.load(std::memory_order_acquire)) {
            break;
        }
        if (alloc_ret != 0 || mem_block == nullptr) {
            refill_drops_.fetch_add(1U, std::memory_order_relaxed);
            continue;
        }

        const uint8_t index = block_index_for_ptr(mem_block);
        if (index >= kBufferCount) {
            refill_drops_.fetch_add(1U, std::memory_order_relaxed);
            continue;
        }

        const bool submitted_before = buffer_submitted_[index].load(std::memory_order_relaxed);
        if (submitted_before && client_.on_tx_consumed != nullptr) {
            client_.on_tx_consumed(buffer_tags_[index].load(std::memory_order_relaxed),
                                   client_.user);
        }

        if (!queue_block(mem_block, index)) {
            break;
        }
    }

    if (healthy_.exchange(false, std::memory_order_relaxed)) {
        notify_health(false);
    }
    if (config_.dev != nullptr && device_is_ready(config_.dev)) {
        (void)i2s_trigger(config_.dev, I2S_DIR_TX, I2S_TRIGGER_DROP);
    }
    started_.store(false, std::memory_order_release);
}

}  // namespace stem_player::audio
