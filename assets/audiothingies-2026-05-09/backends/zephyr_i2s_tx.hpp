#pragma once

#include <zephyr/drivers/i2s.h>
#include <zephyr/kernel.h>

#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>

namespace stem_player::audio {

class ZephyrI2sTx {
public:
    static constexpr size_t kNumChannels = 2U;
    static constexpr size_t kWordsPerBuffer = 256U;
    static constexpr size_t kFramesPerBuffer = kWordsPerBuffer / kNumChannels;
    static constexpr size_t kBufferCount = 8U;

    struct Config {
        const struct device* dev = nullptr;
        uint32_t frame_rate_hz = 48000U;
        uint8_t word_size = 24U;
        uint8_t channels = 2U;
        uint32_t timeout_ms = 100U;
        int thread_priority = 5;
    };

    using FillCallback = void (*)(int32_t* buffer, size_t word_count, void* user);
    using TagProvider = uint32_t (*)(void* user);
    using ConsumedCallback = void (*)(uint32_t tag, void* user);
    using HealthCallback = void (*)(bool healthy, void* user);

    struct Client {
        FillCallback fill_tx_words = nullptr;
        TagProvider tx_tag = nullptr;
        ConsumedCallback on_tx_consumed = nullptr;
        HealthCallback on_tx_health_changed = nullptr;
        void* user = nullptr;
    };

    ZephyrI2sTx();

    bool start(const Config& config, const Client& client);
    void stop();

    bool running() const;
    bool healthy() const;
    uint32_t refill_drops() const;
    void reset_refill_drops();
    uint32_t slab_num_used() const;
    uint32_t slab_num_free() const;
    uint32_t slab_max_used() const;
    uint32_t slab_num_blocks() const;

private:
    static constexpr int kThreadStackSize = 2048;
    static constexpr size_t kBlockSizeBytes = kWordsPerBuffer * sizeof(int32_t);

    static void thread_entry(void* p1, void*, void*);

    bool configure_i2s();
    bool queue_block(void* mem_block, uint8_t index);
    void fill_block(uint8_t index);
    void notify_health(bool healthy);
    uint8_t block_index_for_ptr(const void* mem_block) const;
    void thread_main();

    Config config_{};
    Client client_{};

    std::atomic<bool> started_{false};
    std::atomic<bool> healthy_{false};
    std::atomic<bool> stop_requested_{false};
    std::atomic<uint32_t> refill_drops_{0U};

    alignas(16) std::array<std::array<int32_t, kWordsPerBuffer>, kBufferCount> tx_buffers_{};
    std::array<std::atomic<uint32_t>, kBufferCount> buffer_tags_{};
    std::array<std::atomic<bool>, kBufferCount> buffer_submitted_{};
    struct k_mem_slab tx_mem_slab_{};

    K_KERNEL_STACK_MEMBER(thread_stack_, kThreadStackSize);
    struct k_thread thread_{};
};

}  // namespace stem_player::audio
