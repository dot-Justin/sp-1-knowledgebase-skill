#include "DiskManager.hpp"
#include <zephyr/logging/log.h>
#include <nrf.h>

#include <algorithm>
#include <array>
#include <cstring>

LOG_MODULE_REGISTER(disk_manager, LOG_LEVEL_INF);

namespace storage {

// ----------------------------------------------------------------------------
// Zephyr Disk Driver Callbacks (C linkage for struct disk_operations)
// These bridge Zephyr's disk subsystem to our eMMC Zephyr device.
// ----------------------------------------------------------------------------

// We store a pointer to the DiskManager instance so the C callbacks can
// reach the C++ object. Set during register_disk_driver().
static DiskManager* g_disk_instance = nullptr;

static int emmc_disk_init(struct disk_info* disk) {
    (void)disk;
    // Hardware init is done in DiskManager::init() before registration.
    return 0;
}

static int emmc_disk_status(struct disk_info* disk) {
    (void)disk;
    if (g_disk_instance && g_disk_instance->is_emmc_ready()) {
        return DISK_STATUS_OK;
    }
    return DISK_STATUS_UNINIT;
}

static int emmc_disk_read(struct disk_info* disk, uint8_t* data_buf,
                          uint32_t start_sector, uint32_t num_sector) {
    (void)disk;
    if (!g_disk_instance || !g_disk_instance->is_emmc_ready()) {
        return -EIO;
    }
    if (!g_disk_instance->read_blocks(start_sector, data_buf, num_sector)) {
        return -EIO;
    }
    return 0;
}

static int emmc_disk_write(struct disk_info* disk, const uint8_t* data_buf,
                           uint32_t start_sector, uint32_t num_sector) {
    (void)disk;
    (void)data_buf;
    (void)start_sector;
    (void)num_sector;
    // eMMC write not implemented yet — read-only for now
    return -ENOTSUP;
}

static int emmc_disk_ioctl(struct disk_info* disk, uint8_t cmd, void* buff) {
    (void)disk;
    switch (cmd) {
    case DISK_IOCTL_GET_SECTOR_COUNT:
        if (g_disk_instance) {
            *(uint32_t*)buff = g_disk_instance->emmc_capacity_blocks();
        } else {
            *(uint32_t*)buff = 0;
        }
        return 0;
    case DISK_IOCTL_GET_SECTOR_SIZE:
        *(uint32_t*)buff = 512;
        return 0;
    case DISK_IOCTL_GET_ERASE_BLOCK_SZ:
        *(uint32_t*)buff = 1;  // Minimum erase block = 1 sector
        return 0;
    case DISK_IOCTL_CTRL_SYNC:
    case DISK_IOCTL_CTRL_INIT:
    case DISK_IOCTL_CTRL_DEINIT:
        return 0;
    default:
        return -EINVAL;
    }
}

static const struct disk_operations emmc_disk_ops = {
    .init = emmc_disk_init,
    .status = emmc_disk_status,
    .read = emmc_disk_read,
    .write = emmc_disk_write,
    .ioctl = emmc_disk_ioctl,
};

// ----------------------------------------------------------------------------
// Helpers
// ----------------------------------------------------------------------------

namespace {
// Strip trailing 'X' padding and null-terminate
void strip_x_padding(char* dst, const uint8_t* src, size_t max_len) {
    memcpy(dst, src, max_len);
    dst[max_len] = '\0';
    for (int i = static_cast<int>(max_len) - 1; i >= 0; i--) {
        if (dst[i] == 'X' || dst[i] == '\0') {
            dst[i] = '\0';
        } else {
            break;
        }
    }
}

constexpr bool kForceSyntheticStemData = false;
constexpr size_t kCenteredPrefetchWindow = 5U;

int32_t slot_distance_metric(uint32_t sample_anchor, uint32_t frame_index) {
    if (sample_anchor + kFramesPerSector - 1U < frame_index) {
        return static_cast<int32_t>(frame_index - sample_anchor);
    }
    return static_cast<int32_t>((sample_anchor + kFramesPerSector) - frame_index);
}

bool should_emit_debug_log(uint32_t* last_log_ms, uint32_t interval_ms = 500U) {
    const uint32_t now = k_uptime_get_32();
    if (now - *last_log_ms < interval_ms) {
        return false;
    }
    *last_log_ms = now;
    return true;
}

void update_perf_sample(uint64_t* total_cycles,
                        uint32_t* max_cycles,
                        uint32_t delta_cycles) {
    *total_cycles += delta_cycles;
    *max_cycles = std::max(*max_cycles, delta_cycles);
}

std::array<uint32_t, kCenteredPrefetchWindow> build_centered_sector_window(
    uint32_t center_sector_index,
    uint32_t total_sectors) {
    std::array<uint32_t, kCenteredPrefetchWindow> wanted{};
    const std::array<int32_t, kCenteredPrefetchWindow> deltas = {-2, -1, 0, 1, 2};
    for (size_t i = 0; i < deltas.size(); ++i) {
        int32_t wanted_index =
            static_cast<int32_t>(center_sector_index) + deltas[i];
        wanted_index %= static_cast<int32_t>(total_sectors);
        if (wanted_index < 0) {
            wanted_index += static_cast<int32_t>(total_sectors);
        }
        wanted[i] = static_cast<uint32_t>(wanted_index);
    }
    return wanted;
}

std::array<uint32_t, kAudioPrefetchSlots> build_directional_sector_window(
    uint32_t center_sector_index,
    uint32_t total_sectors,
    bool reverse,
    bool scan_active) {
    std::array<uint32_t, kAudioPrefetchSlots> wanted{};
    if (!scan_active) {
        const auto centered = build_centered_sector_window(center_sector_index, total_sectors);
        for (size_t i = 0; i < centered.size(); ++i) {
            wanted[i] = centered[i];
        }
        for (size_t i = centered.size(); i < wanted.size(); ++i) {
            wanted[i] = UINT32_MAX;
        }
        return wanted;
    }

    wanted[0] = center_sector_index;
    for (size_t i = 1; i < wanted.size(); ++i) {
        const int step = static_cast<int>(i);
        const int delta = reverse ? -step : step;
        int32_t wanted_index =
            (static_cast<int32_t>(center_sector_index) + delta) % static_cast<int32_t>(total_sectors);
        if (wanted_index < 0) {
            wanted_index += static_cast<int32_t>(total_sectors);
        }
        wanted[i] = static_cast<uint32_t>(wanted_index);
    }
    return wanted;
}

uint32_t sector_ring_distance(uint32_t a, uint32_t b, uint32_t total_sectors) {
    if (total_sectors == 0U) {
        return 0U;
    }
    const uint32_t forward = (a >= b) ? (a - b) : (a + total_sectors - b);
    const uint32_t reverse = (b >= a) ? (b - a) : (b + total_sectors - a);
    return std::min(forward, reverse);
}

}  // namespace

uint32_t DiskManager::album_total_sectors() const {
    if (!album_.valid) {
        return 0U;
    }
    uint32_t total = 0U;
    for (uint8_t i = 0; i < album_.song_count; ++i) {
        total += album_.songs[i].length_sectors;
    }
    return total;
}

uint32_t DiskManager::album_start_sector() const {
    if (!album_.valid || album_.song_count == 0U) {
        return 0U;
    }
    return album_.songs[0].start_sector;
}

uint32_t DiskManager::album_total_frames_internal() const {
    return album_total_sectors() * kFramesPerSector;
}

uint32_t DiskManager::song_start_frame_internal(uint8_t song_idx) const {
    if (!album_.valid || song_idx >= album_.song_count) {
        return 0U;
    }
    uint32_t frame = 0U;
    for (uint8_t i = 0; i < song_idx; ++i) {
        frame += album_.songs[i].length_sectors * kFramesPerSector;
    }
    return frame;
}

bool DiskManager::album_sector_index_for_te_sector(uint32_t te_sector,
                                                   uint32_t* album_sector_index_out) const {
    if (album_sector_index_out == nullptr || !album_.valid) {
        return false;
    }

    uint32_t sector_index = 0U;
    for (uint8_t i = 0; i < album_.song_count; ++i) {
        const SongInfo& song = album_.songs[i];
        if (te_sector >= song.start_sector &&
            te_sector < song.start_sector + song.length_sectors) {
            *album_sector_index_out = sector_index + (te_sector - song.start_sector);
            return true;
        }
        sector_index += song.length_sectors;
    }
    return false;
}

uint32_t DiskManager::te_sector_for_album_sector_index(uint32_t album_sector_index) const {
    if (!album_.valid || album_.song_count == 0U) {
        return UINT32_MAX;
    }

    uint32_t remaining = album_sector_index;
    for (uint8_t i = 0; i < album_.song_count; ++i) {
        const SongInfo& song = album_.songs[i];
        if (remaining < song.length_sectors) {
            return song.start_sector + remaining;
        }
        remaining -= song.length_sectors;
    }
    return UINT32_MAX;
}

bool DiskManager::resolve_album_frame(uint64_t frame_index,
                                      uint8_t* song_index_out,
                                      const SongInfo** song_out,
                                      uint32_t* song_frame_index_out,
                                      uint32_t* te_sector_out,
                                      size_t* frame_in_sector_out) const {
    if (!album_.valid || !emmc_ready_.load(std::memory_order_acquire) || album_.song_count == 0U) {
        return false;
    }

    const uint32_t total_frames = album_total_frames_internal();
    if (total_frames == 0U) {
        return false;
    }

    uint32_t wrapped = static_cast<uint32_t>(frame_index % total_frames);
    for (uint8_t i = 0; i < album_.song_count; ++i) {
        const SongInfo& song = album_.songs[i];
        const uint32_t song_frames =
            song.length_sectors * kFramesPerSector;
        if (wrapped < song_frames) {
            if (song_index_out != nullptr) {
                *song_index_out = i;
            }
            if (song_out != nullptr) {
                *song_out = &song;
            }
            if (song_frame_index_out != nullptr) {
                *song_frame_index_out = wrapped;
            }
            if (te_sector_out != nullptr) {
                *te_sector_out = song.start_sector + (wrapped / kFramesPerSector);
            }
            if (frame_in_sector_out != nullptr) {
                *frame_in_sector_out = wrapped % kFramesPerSector;
            }
            return true;
        }
        wrapped -= song_frames;
    }

    return false;
}

// ----------------------------------------------------------------------------
// DiskManager Implementation
// ----------------------------------------------------------------------------

bool DiskManager::register_disk_driver() {
    g_disk_instance = this;

    memset(&disk_info_, 0, sizeof(disk_info_));
    disk_info_.name = "emmc0";
    disk_info_.ops = &emmc_disk_ops;

    int ret = disk_access_register(&disk_info_);
    if (ret < 0) {
        LOG_ERR("Failed to register eMMC disk driver: %d", ret);
        return false;
    }

    LOG_INF("Zephyr disk driver registered as \"emmc0\"");
    return true;
}

DiskManager::PerfStats DiskManager::snapshot_perf_stats() const {
    const k_spinlock_key_t key = k_spin_lock(&perf_lock_);
    const PerfStats snapshot = perf_stats_;
    k_spin_unlock(&perf_lock_, key);
    return snapshot;
}

void DiskManager::reset_perf_stats() {
    const k_spinlock_key_t key = k_spin_lock(&perf_lock_);
    perf_stats_ = PerfStats{};
    k_spin_unlock(&perf_lock_, key);
}

bool DiskManager::parse_album_header() {
    // Read the first TE sector (8192 bytes = 16 eMMC blocks)
    // Reuse the prefetch staging buffer to avoid a dedicated init-only sector cache.
    uint8_t* header = prefetch_sectors_[0].data();
    if (!stem_emmc_read_blocks(emmc_dev_, 0, header, kEmmcBlocksPerSector)) {
        LOG_ERR("Failed to read album header");
        return false;
    }

    static constexpr char kMagic[] = "ALBUM_PRESENT";
    static constexpr size_t kMagicLen = 13;

    if (memcmp(header, kMagic, kMagicLen) != 0) {
        LOG_WRN("No ALBUM_PRESENT magic — eMMC may be blank");
        return false;
    }

    memcpy(&album_.total_sectors, &header[13], sizeof(uint32_t));
    album_.song_count = header[17];
    strip_x_padding(album_.title, &header[18], 64);

    if (album_.song_count > kMaxSongs) {
        LOG_WRN("Song count %u exceeds max %u, clamping",
                album_.song_count, kMaxSongs);
        album_.song_count = kMaxSongs;
    }

    LOG_INF("Album: \"%s\" — %u songs, %u sectors (%u KB)",
            album_.title, album_.song_count,
            album_.total_sectors,
            (album_.total_sectors * kSectorSize) / 1024);

    // Parse song metadata starting at offset 82
    // Each song entry: 4 bytes start + 4 bytes length + 64 bytes artist + 64 bytes title = 136 bytes
    static constexpr size_t kSongMetaOffset = 82;
    static constexpr size_t kSongMetaSize = 136;

    for (uint8_t i = 0; i < album_.song_count; i++) {
        size_t offset = kSongMetaOffset + i * kSongMetaSize;
        if (offset + kSongMetaSize > kSectorSize) {
            LOG_WRN("Song %u metadata beyond sector boundary", i);
            break;
        }

        memcpy(&album_.songs[i].start_sector, &header[offset], sizeof(uint32_t));
        memcpy(&album_.songs[i].length_sectors, &header[offset + 4], sizeof(uint32_t));
        strip_x_padding(album_.songs[i].artist, &header[offset + 8], 64);
        strip_x_padding(album_.songs[i].title, &header[offset + 72], 64);

    }

    album_.valid = true;
    return true;
}

void DiskManager::reset_prefetch_tracking_locked() {
    inflight_read_ = InflightRead{};
    completed_read_ = CompletedRead{};
    planned_read_ = PlannedRead{};
    next_prefetch_buffer_index_ = 0U;
}

void DiskManager::abort_prefetch_locked() {
    stem_emmc_abort_async_read(emmc_dev_);
    reset_prefetch_tracking_locked();
}

void DiskManager::set_storage_work_queue(struct k_work_q* work_q) {
    storage_work_q_ = work_q;
}

void DiskManager::init() {
    LOG_INF("Initializing Disk Manager (eMMC)");

    init_attempted_ = false;
    emmc_ready_.store(false, std::memory_order_release);
    playback_prefetch_enabled_.store(false, std::memory_order_relaxed);
    k_mutex_init(&prefetch_state_lock_);
    reset_prefetch_tracking_locked();
    LOG_INF("Disk Manager will probe eMMC asynchronously; audio can start on synthetic fallback");

    k_work_init_delayable(&init_work_, DiskManager::init_work_handler);
    k_work_init(&prefetch_work_, DiskManager::prefetch_work_handler);
    if (storage_work_q_ != nullptr) {
        k_work_schedule_for_queue(storage_work_q_, &init_work_, K_NO_WAIT);
    } else {
        k_work_schedule(&init_work_, K_NO_WAIT);
    }
}

bool DiskManager::init_blocking_for_audio() {
    init_attempted_ = false;
    emmc_ready_.store(false, std::memory_order_release);
    playback_prefetch_enabled_.store(false, std::memory_order_relaxed);
    k_mutex_init(&prefetch_state_lock_);
    reset_prefetch_tracking_locked();
    k_work_init(&prefetch_work_, DiskManager::prefetch_work_handler);
    run_init_attempt();
    return emmc_ready_.load(std::memory_order_acquire);
}

void DiskManager::decode_stem_from_frame_payload_i32(const uint8_t* frame_payload,
    int stem_idx,
    int32_t* left,
    int32_t* right) const {
    if (frame_payload == nullptr || left == nullptr || right == nullptr) {
        return;
    }

    const uint8_t* data =
        frame_payload + static_cast<size_t>(std::clamp(stem_idx, 0, 3)) * kStemFrameSize;

    // 1. Use int8_t cast for free hardware sign-extension.
    // 2. Cast to uint32_t before shifting to prevent strict C++17 Undefined Behavior 
    //    (left-shifting a negative signed integer) and signed/unsigned mix warnings.
    // 3. Bitwise OR the lower bytes, and cast the final 32-bit result back to int32_t.
    *left = static_cast<int32_t>(
        (static_cast<uint32_t>(static_cast<int8_t>(data[1])) << 16) |
        (static_cast<uint32_t>(data[0]) << 8) |
         static_cast<uint32_t>(data[3])
    );

    *right = static_cast<int32_t>(
        (static_cast<uint32_t>(static_cast<int8_t>(data[2])) << 16) |
        (static_cast<uint32_t>(data[5]) << 8) |
         static_cast<uint32_t>(data[4])
    );
}

void DiskManager::decode_stem_from_frame_payload(const uint8_t* frame_payload,
    int stem_idx,
    float* left,
    float* right) const {
    if (frame_payload == nullptr || left == nullptr || right == nullptr) {
        return;
    }

    int32_t left_i32 = 0;
    int32_t right_i32 = 0;
    decode_stem_from_frame_payload_i32(frame_payload, stem_idx, &left_i32, &right_i32);

    // Float multiplication is ~14x faster than float division!
    constexpr float kPcmToFloat = 1.0f / 8388608.0f; 

    *left = static_cast<float>(left_i32) * kPcmToFloat;
    *right = static_cast<float>(right_i32) * kPcmToFloat;
}

int DiskManager::find_audio_slot(uint32_t te_sector) const {
    for (size_t i = 0; i < audio_slots_.size(); ++i) {
        if (!audio_slots_[i].ready.load(std::memory_order_acquire)) {
            continue;
        }
        if (audio_slots_[i].te_sector.load(std::memory_order_relaxed) == te_sector) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

bool DiskManager::resolve_slot_for_sector(uint32_t te_sector,
                                          uint32_t* cached_sector,
                                          int* cached_slot_index,
                                          int* slot_index_out) const {
    if (slot_index_out == nullptr) {
        return false;
    }

    int slot_index = -1;
    if (cached_sector != nullptr && cached_slot_index != nullptr &&
        *cached_slot_index >= 0 && *cached_sector == te_sector) {
        const auto& cached_slot = audio_slots_[static_cast<size_t>(*cached_slot_index)];
        if (cached_slot.ready.load(std::memory_order_acquire) &&
            cached_slot.te_sector.load(std::memory_order_relaxed) == te_sector) {
            slot_index = *cached_slot_index;
            const k_spinlock_key_t key = k_spin_lock(&perf_lock_);
            ++perf_stats_.resolve_cache_hit_count;
            k_spin_unlock(&perf_lock_, key);
        } else {
            const k_spinlock_key_t key = k_spin_lock(&perf_lock_);
            ++perf_stats_.resolve_slot_not_ready_count;
            k_spin_unlock(&perf_lock_, key);
        }
    }

    if (slot_index < 0) {
        slot_index = find_audio_slot(te_sector);
        if (cached_sector != nullptr) {
            *cached_sector = te_sector;
        }
        if (cached_slot_index != nullptr) {
            *cached_slot_index = slot_index;
        }
    }

    if (slot_index < 0) {
        const k_spinlock_key_t key = k_spin_lock(&perf_lock_);
        ++perf_stats_.resolve_slot_not_found_count;
        k_spin_unlock(&perf_lock_, key);
        return false;
    }

    *slot_index_out = slot_index;
    return true;
}

const uint8_t* DiskManager::block_frame_payload_ptr(const AudioSlot& slot,
                                                    size_t block_index,
                                                    size_t frame_in_block) const {
    if (block_index >= kBlocksPerSector || frame_in_block >= kFramesPerBlock) {
        return nullptr;
    }
    const size_t frame_in_sector = block_index * kFramesPerBlock + frame_in_block;
    return slot.record.payload.data() + frame_in_sector * kFrameSize;
}

bool DiskManager::resolve_slot_frame(uint64_t frame_index,
                                     uint32_t* cached_sector,
                                     int* cached_slot_index,
                                     int* slot_index_out,
                                     size_t* block_index_out,
                                     size_t* frame_in_block_out,
                                     size_t* frame_in_sector_out) const {
    if (!album_.valid ||
        !emmc_ready_.load(std::memory_order_acquire) ||
        slot_index_out == nullptr ||
        block_index_out == nullptr ||
        frame_in_block_out == nullptr) {
        return false;
    }

    uint32_t te_sector = UINT32_MAX;
    size_t frame_in_sector = 0U;
    if (!resolve_album_frame(frame_index,
                             nullptr,
                             nullptr,
                             nullptr,
                             &te_sector,
                             &frame_in_sector)) {
        const k_spinlock_key_t key = k_spin_lock(&perf_lock_);
        ++perf_stats_.resolve_map_fail_count;
        k_spin_unlock(&perf_lock_, key);
        return false;
    }
    const size_t block_index = frame_in_sector / kFramesPerBlock;
    const size_t frame_in_block = frame_in_sector % kFramesPerBlock;

    int slot_index = -1;
    if (!resolve_slot_for_sector(te_sector,
                                 cached_sector,
                                 cached_slot_index,
                                 &slot_index)) {
        return false;
    }

    *slot_index_out = slot_index;
    *block_index_out = block_index;
    *frame_in_block_out = frame_in_block;
    if (frame_in_sector_out != nullptr) {
        *frame_in_sector_out = frame_in_sector;
    }
    return true;
}

bool DiskManager::load_audio_slot(size_t slot_index,
                                  uint32_t te_sector,
                                  uint32_t sample_anchor,
                                  const uint8_t* source_sector,
                                  uint32_t read_blocks) {
    if (slot_index >= audio_slots_.size()) {
        return false;
    }

    auto& slot = audio_slots_[slot_index];
    slot.ready.store(false, std::memory_order_relaxed);
    slot.state.store(AudioSlotState::Reading, std::memory_order_relaxed);

    if (source_sector == nullptr) {
        slot.state.store(AudioSlotState::Empty, std::memory_order_relaxed);
        return false;
    }

    auto load_u32 = [](const uint8_t* src) -> uint32_t {
        uint32_t value = 0U;
        std::memcpy(&value, src, sizeof(value));
        return value;
    };
    auto load_block_sync_word = [&](size_t physical_block_index) -> uint32_t {
        return load_u32(source_sector + physical_block_index * kBlockSize +
                        kFramesPerBlock * kFrameSize);
    };
    auto load_block_led_word = [&](size_t physical_block_index) -> uint32_t {
        return load_u32(source_sector + physical_block_index * kBlockSize +
                        kFramesPerBlock * kFrameSize + sizeof(uint32_t));
    };
    auto copy_frame = [&](size_t logical_frame_index, size_t physical_block_index, size_t frame_in_block) {
        const uint8_t* src =
            source_sector + physical_block_index * kBlockSize + frame_in_block * kFrameSize;
        uint8_t* dst = slot.record.payload.data() + logical_frame_index * kFrameSize;
        std::memcpy(dst, src, kFrameSize);
    };
    auto copy_block_audio = [&](size_t logical_block_index, size_t physical_block_index) {
        const uint8_t* src = source_sector + physical_block_index * kBlockSize;
        uint8_t* dst = slot.record.payload.data() + logical_block_index * kFramesPerBlock * kFrameSize;
        std::memcpy(dst, src, kBlockAudioSize);
    };
    const uint32_t load_branch_start = k_cycle_get_32();
    uint32_t* branch_max_cycles = nullptr;
    uint64_t* branch_total_cycles = nullptr;
    uint32_t* branch_count = nullptr;

    if (read_blocks >= kEmmcBlocksPerSector) {
        branch_max_cycles = &perf_stats_.load_full_max_cycles;
        branch_total_cycles = &perf_stats_.load_full_total_cycles;
        branch_count = &perf_stats_.load_full_count;
        for (size_t frame = 0; frame < kFramesPerBlock; ++frame) {
            for (size_t logical_block = 0; logical_block < kBlocksPerSector; ++logical_block) {
                copy_frame(frame * kBlocksPerSector + logical_block,
                           static_cast<size_t>(kBlockOrder[logical_block]),
                           frame);
            }
        }
        slot.record.block_sync_words = {
            load_block_sync_word(static_cast<size_t>(kBlockOrder[0])),
            load_block_sync_word(static_cast<size_t>(kBlockOrder[1])),
            load_block_sync_word(static_cast<size_t>(kBlockOrder[2])),
            load_block_sync_word(static_cast<size_t>(kBlockOrder[3])),
        };
        slot.record.block_led_words = {
            load_block_led_word(static_cast<size_t>(kBlockOrder[0])),
            load_block_led_word(static_cast<size_t>(kBlockOrder[1])),
            load_block_led_word(static_cast<size_t>(kBlockOrder[2])),
            load_block_led_word(static_cast<size_t>(kBlockOrder[3])),
        };
    } else if (read_blocks >= (kEmmcBlocksPerSector / 2U)) {
        branch_max_cycles = &perf_stats_.load_half_max_cycles;
        branch_total_cycles = &perf_stats_.load_half_total_cycles;
        branch_count = &perf_stats_.load_half_count;
        const uint8_t* src_a = source_sector;
        const uint8_t* src_b = source_sector + kBlockSize;
        uint8_t* dst0 = slot.record.payload.data();
        uint8_t* dst1 = dst0 + kBlockAudioSize;
        uint8_t* dst2 = dst1 + kBlockAudioSize;
        uint8_t* dst3 = dst2 + kBlockAudioSize;
        for (size_t frame = 0; frame < kFramesPerBlock; ++frame) {
            __builtin_memcpy(dst0, src_a, kFrameSize);
            __builtin_memcpy(dst1, src_a, kFrameSize);
            __builtin_memcpy(dst2, src_b, kFrameSize);
            __builtin_memcpy(dst3, src_b, kFrameSize);
            src_a += kFrameSize;
            src_b += kFrameSize;
            dst0 += kFrameSize;
            dst1 += kFrameSize;
            dst2 += kFrameSize;
            dst3 += kFrameSize;
        }
        slot.record.block_sync_words = {
            load_block_sync_word(0U),
            load_block_sync_word(0U),
            load_block_sync_word(1U),
            load_block_sync_word(1U),
        };
        slot.record.block_led_words = {
            load_block_led_word(0U),
            load_block_led_word(0U),
            load_block_led_word(1U),
            load_block_led_word(1U),
        };
    } else {
        branch_max_cycles = &perf_stats_.load_quarter_max_cycles;
        branch_total_cycles = &perf_stats_.load_quarter_total_cycles;
        branch_count = &perf_stats_.load_quarter_count;
        const uint8_t* src = source_sector;
        uint8_t* dst0 = slot.record.payload.data();
        uint8_t* dst1 = dst0 + kBlockAudioSize;
        uint8_t* dst2 = dst1 + kBlockAudioSize;
        uint8_t* dst3 = dst2 + kBlockAudioSize;
        for (size_t frame = 0; frame < kFramesPerBlock; ++frame) {
            __builtin_memcpy(dst0, src, kFrameSize);
            __builtin_memcpy(dst1, src, kFrameSize);
            __builtin_memcpy(dst2, src, kFrameSize);
            __builtin_memcpy(dst3, src, kFrameSize);
            src += kFrameSize;
            dst0 += kFrameSize;
            dst1 += kFrameSize;
            dst2 += kFrameSize;
            dst3 += kFrameSize;
        }
        const uint32_t sync0 = load_block_sync_word(0U);
        const uint32_t led0 = load_block_led_word(0U);
        slot.record.block_sync_words = {sync0, sync0, sync0, sync0};
        slot.record.block_led_words = {led0, led0, led0, led0};
    }
    const uint32_t load_branch_cycles = k_cycle_get_32() - load_branch_start;
    if (branch_max_cycles != nullptr && branch_total_cycles != nullptr && branch_count != nullptr) {
        const k_spinlock_key_t key = k_spin_lock(&perf_lock_);
        ++(*branch_count);
        update_perf_sample(branch_total_cycles, branch_max_cycles, load_branch_cycles);
        k_spin_unlock(&perf_lock_, key);
    }

    slot.sample_anchor.store(sample_anchor, std::memory_order_relaxed);
    slot.te_sector.store(te_sector, std::memory_order_relaxed);
    slot.read_blocks.store(read_blocks, std::memory_order_relaxed);
    slot.distance_hint.store(0, std::memory_order_relaxed);
    slot.read_needed.store(false, std::memory_order_relaxed);
    slot.state.store(AudioSlotState::Ready, std::memory_order_release);
    slot.ready.store(true, std::memory_order_release);
    return true;
}

uint32_t DiskManager::select_next_audio_sector_to_read(uint32_t frame_index,
                                                       uint32_t* album_sector_index_out) const {
    const uint32_t total_sectors = album_total_sectors();
    if (total_sectors == 0U) {
        return UINT32_MAX;
    }

    const uint32_t sector_offset = (frame_index / kFramesPerSector) % total_sectors;
    const uint32_t current_sector = te_sector_for_album_sector_index(sector_offset);
    const bool reverse = playback_reverse_.load(std::memory_order_relaxed);
    const bool scan_active = playback_scan_active_.load(std::memory_order_relaxed);
    if (current_sector == UINT32_MAX) {
        return UINT32_MAX;
    }

    struct SlotSnapshot {
        bool ready = false;
        bool reading = false;
        uint32_t te_sector = UINT32_MAX;
        uint32_t sample_anchor = 0U;
    };
    std::array<SlotSnapshot, kAudioPrefetchSlots> slots{};
    for (size_t i = 0; i < audio_slots_.size(); ++i) {
        const auto& slot = audio_slots_[i];
        slots[i].reading =
            slot.state.load(std::memory_order_relaxed) == AudioSlotState::Reading;
        slots[i].te_sector = slot.te_sector.load(std::memory_order_relaxed);
        slots[i].ready = slot.ready.load(std::memory_order_acquire);
        if (!slots[i].ready) {
            continue;
        }
        slots[i].sample_anchor = slot.sample_anchor.load(std::memory_order_relaxed);
    }

    auto sector_missing = [&](uint32_t te_sector) -> bool {
        for (const auto& slot : slots) {
            if ((slot.ready || slot.reading) && slot.te_sector == te_sector) {
                return false;
            }
        }
        if (te_sector == UINT32_MAX) {
            return false;
        }
        if (completed_read_.active && completed_read_.te_sector == te_sector) {
            return false;
        }
        if (inflight_read_.active && inflight_read_.te_sector == te_sector) {
            return false;
        }
        return true;
    };

    auto return_sector = [&](uint32_t te_sector, uint32_t album_sector_index) -> uint32_t {
        if (album_sector_index_out != nullptr) {
            *album_sector_index_out = album_sector_index;
        }
        return te_sector;
    };

    const auto wanted = build_directional_sector_window(sector_offset, total_sectors, reverse, scan_active);
    for (uint32_t wanted_index : wanted) {
        if (wanted_index == UINT32_MAX) {
            continue;
        }
        const uint32_t te_sector = te_sector_for_album_sector_index(wanted_index);
        if (sector_missing(te_sector)) {
            return return_sector(te_sector, wanted_index);
        }
    }

    return UINT32_MAX;
}

bool DiskManager::is_sector_render_protected(uint32_t te_sector) const {
    const uint32_t sector_a = render_protected_sector_a_.load(std::memory_order_relaxed);
    const uint32_t sector_b = render_protected_sector_b_.load(std::memory_order_relaxed);
    return te_sector == sector_a || te_sector == sector_b;
}

size_t DiskManager::choose_audio_slot_replacement(uint32_t frame_index,
                                                  uint32_t te_sector) const {
    const uint32_t total_sectors = album_total_sectors();
    const uint32_t current_sector_index =
        total_sectors > 0U ? (frame_index / kFramesPerSector) % total_sectors : 0U;
    const size_t reserved_completed_slot =
        completed_read_.active ? completed_read_.slot_index : audio_slots_.size();
    const size_t reserved_inflight_slot =
        inflight_read_.active ? inflight_read_.slot_index : audio_slots_.size();
    const uint32_t protected_sector_a =
        render_protected_sector_a_.load(std::memory_order_relaxed);
    const uint32_t protected_sector_b =
        render_protected_sector_b_.load(std::memory_order_relaxed);

    size_t replacement = audio_slots_.size();
    uint32_t worst_distance = 0U;

    for (size_t slot_idx = 0; slot_idx < audio_slots_.size(); ++slot_idx) {
        if (slot_idx == reserved_completed_slot || slot_idx == reserved_inflight_slot) {
            continue;
        }

        const auto& slot = audio_slots_[slot_idx];
        if (!slot.ready.load(std::memory_order_acquire)) {
            if (slot.state.load(std::memory_order_relaxed) == AudioSlotState::Reading) {
                continue;
            }
            return slot_idx;
        }

        const uint32_t slot_sector = slot.te_sector.load(std::memory_order_relaxed);
        if (slot_sector == te_sector) {
            return audio_slots_.size();
        }
        if (slot_sector == protected_sector_a || slot_sector == protected_sector_b) {
            continue;
        }

        const uint32_t slot_sector_index =
            slot.sample_anchor.load(std::memory_order_relaxed) / kFramesPerSector;
        const uint32_t distance = total_sectors > 0U
            ? sector_ring_distance(slot_sector_index, current_sector_index, total_sectors)
            : static_cast<uint32_t>(
                  slot_distance_metric(slot.sample_anchor.load(std::memory_order_relaxed),
                                       frame_index));
        if (distance > worst_distance) {
            worst_distance = distance;
            replacement = slot_idx;
        }
    }

    return replacement;
}

void DiskManager::set_render_protected_sectors(uint32_t sector_a, uint32_t sector_b) {
    render_protected_sector_a_.store(sector_a, std::memory_order_relaxed);
    render_protected_sector_b_.store(sector_b, std::memory_order_relaxed);
}

bool DiskManager::start_missing_audio_slot_read(uint32_t frame_index,
                                                uint32_t read_blocks) {
    static uint32_t last_async_fail_log_ms = 0U;
    const uint32_t start_read_start = k_cycle_get_32();
    PlannedRead plan{};
    if (planned_read_.valid && planned_read_.read_blocks == read_blocks) {
        plan = planned_read_;
        planned_read_.valid = false;
    } else {
        uint32_t album_sector_index = 0U;
        const uint32_t te_sector =
            select_next_audio_sector_to_read(frame_index, &album_sector_index);
        if (te_sector == UINT32_MAX) {
            return false;
        }

        const size_t replacement = choose_audio_slot_replacement(frame_index, te_sector);
        if (replacement >= audio_slots_.size()) {
            return false;
        }
        plan.valid = true;
        plan.slot_index = replacement;
        plan.te_sector = te_sector;
        plan.sample_anchor = album_sector_index * kFramesPerSector;
        plan.read_blocks = read_blocks;
    }

    const uint8_t buffer_index = next_prefetch_buffer_index_;
    audio_slots_[plan.slot_index].ready.store(false, std::memory_order_relaxed);
    inflight_read_.active = true;
    inflight_read_.slot_index = plan.slot_index;
    inflight_read_.te_sector = plan.te_sector;
    inflight_read_.sample_anchor = plan.sample_anchor;
    inflight_read_.buffer_index = buffer_index;
    inflight_read_.read_blocks = read_blocks;
    const uint32_t emmc_block = plan.te_sector * kEmmcBlocksPerSector;
    if (!stem_emmc_start_read_blocks_async(emmc_dev_,
                                           emmc_block,
                                           prefetch_sectors_[buffer_index].data(),
                                           read_blocks)) {
        inflight_read_.active = false;
        if (should_emit_debug_log(&last_async_fail_log_ms)) {
            LOG_WRN("prefetch async-start-failed frame=%u sector=%u read_blocks=%u",
                    frame_index,
                    plan.te_sector,
                    read_blocks);
        }
        return false;
    }
    LOG_DBG("prefetch async-start frame=%u sector=%u emmc_block=0x%08x blocks=%u slot=%u buf=%u",
            frame_index,
            plan.te_sector,
            emmc_block,
            read_blocks,
            plan.slot_index,
            buffer_index);
    next_prefetch_buffer_index_ ^= 1U;
    const uint32_t start_read_cycles = k_cycle_get_32() - start_read_start;
    const k_spinlock_key_t key = k_spin_lock(&perf_lock_);
    ++perf_stats_.start_read_count;
    update_perf_sample(&perf_stats_.start_read_total_cycles,
                       &perf_stats_.start_read_max_cycles,
                       start_read_cycles);
    k_spin_unlock(&perf_lock_, key);
    return true;
}

bool DiskManager::service_audio_prefetch_window(uint32_t frame_index,
                                                uint32_t read_blocks) {
    static uint32_t last_prefetch_skip_disabled_log_ms = 0U;
    static uint32_t last_prefetch_skip_state_log_ms = 0U;
    static uint32_t last_prefetch_skip_album_log_ms = 0U;
    static uint32_t last_prefetch_skip_poll_log_ms = 0U;
    const uint32_t prefetch_start = k_cycle_get_32();
    if (!playback_prefetch_enabled_.load(std::memory_order_relaxed)) {
        if (should_emit_debug_log(&last_prefetch_skip_disabled_log_ms)) {
            LOG_DBG("prefetch skip: disabled frame=%u", frame_index);
        }
        return false;
    }
    if (sync_seed_in_progress_.load(std::memory_order_acquire)) {
        return false;
    }

    bool had_inflight = false;
    bool made_progress = false;
    k_mutex_lock(&prefetch_state_lock_, K_FOREVER);

    if (!playback_prefetch_enabled_.load(std::memory_order_relaxed)) {
        k_mutex_unlock(&prefetch_state_lock_);
        if (should_emit_debug_log(&last_prefetch_skip_disabled_log_ms)) {
            LOG_DBG("prefetch skip after lock: disabled frame=%u", frame_index);
        }
        return false;
    }
    if (sync_seed_in_progress_.load(std::memory_order_acquire)) {
        k_mutex_unlock(&prefetch_state_lock_);
        return false;
    }

    const uint8_t current_song = current_song_.load(std::memory_order_relaxed);
    if (!album_.valid || !emmc_ready_.load(std::memory_order_acquire) ||
        current_song >= album_.song_count) {
        k_mutex_unlock(&prefetch_state_lock_);
        if (should_emit_debug_log(&last_prefetch_skip_state_log_ms)) {
            LOG_WRN("prefetch skip: album_valid=%d emmc_ready=%d current_song=%u song_count=%u frame=%u",
                    album_.valid ? 1 : 0,
                    emmc_ready_.load(std::memory_order_relaxed) ? 1 : 0,
                    current_song,
                    album_.song_count,
                    frame_index);
        }
        return false;
    }
    had_inflight = inflight_read_.active;
    k_mutex_unlock(&prefetch_state_lock_);

    bool read_success = false;
    uint32_t poll_cycles = 0U;
    if (had_inflight) {
        const uint32_t poll_start = k_cycle_get_32();
        if (!stem_emmc_poll_async_read_complete(emmc_dev_, &read_success)) {
            LOG_DBG("prefetch poll pending frame=%u", frame_index);
            if (should_emit_debug_log(&last_prefetch_skip_poll_log_ms)) {
                LOG_DBG("prefetch skip: inflight still pending frame=%u", frame_index);
            }
            return false;
        }
        poll_cycles = k_cycle_get_32() - poll_start;
        LOG_DBG("prefetch poll done frame=%u success=%d", frame_index, read_success ? 1 : 0);
    }

    k_mutex_lock(&prefetch_state_lock_, K_FOREVER);
    if (!playback_prefetch_enabled_.load(std::memory_order_relaxed)) {
        k_mutex_unlock(&prefetch_state_lock_);
        if (should_emit_debug_log(&last_prefetch_skip_disabled_log_ms)) {
            LOG_DBG("prefetch skip after relock: disabled frame=%u", frame_index);
        }
        return false;
    }

    if (had_inflight && inflight_read_.active) {
        const k_spinlock_key_t poll_key = k_spin_lock(&perf_lock_);
        ++perf_stats_.poll_count;
        update_perf_sample(&perf_stats_.poll_total_cycles,
                           &perf_stats_.poll_max_cycles,
                           poll_cycles);
        k_spin_unlock(&perf_lock_, poll_key);

        completed_read_.active = true;
        completed_read_.success = read_success;
        completed_read_.slot_index = inflight_read_.slot_index;
        completed_read_.te_sector = inflight_read_.te_sector;
        completed_read_.sample_anchor = inflight_read_.sample_anchor;
        completed_read_.buffer_index = inflight_read_.buffer_index;
        completed_read_.read_blocks = inflight_read_.read_blocks;
        inflight_read_.active = false;
        made_progress = true;

        if (!planned_read_.valid) {
            uint32_t next_album_sector_index = 0U;
            const uint32_t next_te_sector =
                select_next_audio_sector_to_read(frame_index, &next_album_sector_index);
            if (next_te_sector != UINT32_MAX) {
                const size_t next_replacement =
                    choose_audio_slot_replacement(frame_index, next_te_sector);
                if (next_replacement < audio_slots_.size()) {
                    planned_read_.valid = true;
                    planned_read_.slot_index = next_replacement;
                    planned_read_.te_sector = next_te_sector;
                    planned_read_.sample_anchor =
                        next_album_sector_index * kFramesPerSector;
                    planned_read_.read_blocks = read_blocks;
                }
            }
        }
    }

    if (album_total_sectors() == 0U) {
        k_mutex_unlock(&prefetch_state_lock_);
        if (should_emit_debug_log(&last_prefetch_skip_album_log_ms)) {
            LOG_WRN("prefetch skip: album has zero sectors frame=%u", frame_index);
        }
        return made_progress;
    }

    if (!inflight_read_.active) {
        made_progress |= start_missing_audio_slot_read(frame_index, read_blocks);
    }

    if (completed_read_.active) {
        const uint32_t complete_start = k_cycle_get_32();
        if (completed_read_.success) {
            const uint32_t load_slot_start = k_cycle_get_32();
            (void)load_audio_slot(completed_read_.slot_index,
                                  completed_read_.te_sector,
                                  completed_read_.sample_anchor,
                                  prefetch_sectors_[completed_read_.buffer_index].data(),
                                  completed_read_.read_blocks);
            LOG_DBG("prefetch slot loaded sector=%u slot=%u blocks=%u",
                    completed_read_.te_sector,
                    completed_read_.slot_index,
                    completed_read_.read_blocks);
            const uint32_t load_slot_cycles = k_cycle_get_32() - load_slot_start;
            const k_spinlock_key_t load_key = k_spin_lock(&perf_lock_);
            ++perf_stats_.load_slot_count;
            update_perf_sample(&perf_stats_.load_slot_total_cycles,
                               &perf_stats_.load_slot_max_cycles,
                               load_slot_cycles);
            k_spin_unlock(&perf_lock_, load_key);
        } else {
            audio_slots_[completed_read_.slot_index].ready.store(false,
                                                                 std::memory_order_relaxed);
        }
        completed_read_.active = false;
        made_progress = true;
        const uint32_t complete_cycles = k_cycle_get_32() - complete_start;
        const k_spinlock_key_t complete_key = k_spin_lock(&perf_lock_);
        ++perf_stats_.complete_count;
        update_perf_sample(&perf_stats_.complete_total_cycles,
                           &perf_stats_.complete_max_cycles,
                           complete_cycles);
        k_spin_unlock(&perf_lock_, complete_key);
    }

    k_mutex_unlock(&prefetch_state_lock_);
    const uint32_t prefetch_cycles = k_cycle_get_32() - prefetch_start;
    const k_spinlock_key_t key = k_spin_lock(&perf_lock_);
    ++perf_stats_.prefetch_count;
    update_perf_sample(&perf_stats_.prefetch_total_cycles,
                       &perf_stats_.prefetch_max_cycles,
                       prefetch_cycles);
    k_spin_unlock(&perf_lock_, key);
    return made_progress;
}

void DiskManager::set_playback_cursor(uint32_t frame_index) {
    playback_cursor_hint_.store(frame_index, std::memory_order_relaxed);
}

void DiskManager::set_playback_reverse(bool reverse) {
    playback_reverse_.store(reverse, std::memory_order_relaxed);
}

void DiskManager::set_playback_prefetch_enabled(bool enabled) {
    k_mutex_lock(&prefetch_state_lock_, K_FOREVER);
    playback_prefetch_enabled_.store(enabled, std::memory_order_relaxed);
    if (!enabled) {
        abort_prefetch_locked();
    } else {
        reset_prefetch_tracking_locked();
    }
    k_mutex_unlock(&prefetch_state_lock_);
    wake_prefetch_worker();
}

void DiskManager::start_streaming(uint32_t frame_index,
                                  bool reverse,
                                  bool scan_active,
                                  uint32_t read_blocks) {
    rebase_streaming(frame_index, reverse, scan_active, read_blocks);
}

void DiskManager::update_streaming_hint(uint32_t frame_index,
                                        bool reverse,
                                        bool scan_active,
                                        uint32_t read_blocks) {
    const uint32_t previous_frame = playback_cursor_hint_.load(std::memory_order_relaxed);
    const bool previous_reverse = playback_reverse_.load(std::memory_order_relaxed);
    const bool previous_scan_active = playback_scan_active_.load(std::memory_order_relaxed);
    const uint32_t previous_read_blocks =
        playback_read_blocks_hint_.load(std::memory_order_relaxed);

    playback_cursor_hint_.store(frame_index, std::memory_order_relaxed);
    playback_reverse_.store(reverse, std::memory_order_relaxed);
    playback_scan_active_.store(scan_active, std::memory_order_relaxed);
    playback_read_blocks_hint_.store(read_blocks, std::memory_order_relaxed);

    const uint32_t previous_sector = previous_frame / kFramesPerSector;
    const uint32_t next_sector = frame_index / kFramesPerSector;
    const bool sector_changed = previous_sector != next_sector;
    const bool mode_changed =
        previous_reverse != reverse ||
        previous_scan_active != scan_active ||
        previous_read_blocks != read_blocks;

    if (sector_changed || mode_changed) {
        wake_prefetch_worker();
    }
}

bool DiskManager::service_streaming_hint_once() {
    const uint32_t hint_start = k_cycle_get_32();
    const bool progress =
        service_streaming(playback_cursor_hint_.load(std::memory_order_relaxed),
                          playback_reverse_.load(std::memory_order_relaxed),
                          playback_scan_active_.load(std::memory_order_relaxed),
                          playback_read_blocks_hint_.load(std::memory_order_relaxed));
    const uint32_t hint_cycles = k_cycle_get_32() - hint_start;
    const k_spinlock_key_t key = k_spin_lock(&perf_lock_);
    ++perf_stats_.service_hint_count;
    update_perf_sample(&perf_stats_.service_hint_total_cycles,
                       &perf_stats_.service_hint_max_cycles,
                       hint_cycles);
    k_spin_unlock(&perf_lock_, key);
    return progress;
}

void DiskManager::service_streaming_hint() {
    (void)service_streaming_hint_once();
}

void DiskManager::rebase_streaming(uint32_t frame_index,
                                   bool reverse,
                                   bool scan_active,
                                   uint32_t read_blocks) {
    static uint32_t last_rebase_skip_state_log_ms = 0U;
    PlannedRead seed_plan{};
    k_mutex_lock(&prefetch_state_lock_, K_FOREVER);
    playback_prefetch_enabled_.store(true, std::memory_order_relaxed);
    playback_reverse_.store(reverse, std::memory_order_relaxed);
    playback_scan_active_.store(scan_active, std::memory_order_relaxed);
    playback_cursor_hint_.store(frame_index, std::memory_order_relaxed);
    playback_read_blocks_hint_.store(read_blocks, std::memory_order_relaxed);
    if (inflight_read_.active || stem_emmc_async_read_in_flight(emmc_dev_)) {
        abort_prefetch_locked();
    } else {
        reset_prefetch_tracking_locked();
    }
    for (auto& slot : audio_slots_) {
        slot.ready.store(false, std::memory_order_relaxed);
    }

    const uint8_t current_song = current_song_.load(std::memory_order_relaxed);
    if (!album_.valid || !emmc_ready_.load(std::memory_order_acquire) ||
        current_song >= album_.song_count) {
        k_mutex_unlock(&prefetch_state_lock_);
        if (should_emit_debug_log(&last_rebase_skip_state_log_ms)) {
            LOG_WRN("rebase skip: album_valid=%d emmc_ready=%d current_song=%u song_count=%u frame=%u",
                    album_.valid ? 1 : 0,
                    emmc_ready_.load(std::memory_order_relaxed) ? 1 : 0,
                    current_song,
                    album_.song_count,
                    frame_index);
        }
        return;
    }

    if (album_total_sectors() == 0U) {
        k_mutex_unlock(&prefetch_state_lock_);
        if (should_emit_debug_log(&last_rebase_skip_state_log_ms)) {
            LOG_WRN("rebase skip: album has zero sectors frame=%u", frame_index);
        }
        return;
    }

    uint32_t te_sector = UINT32_MAX;
    if (!resolve_album_frame(frame_index,
                             nullptr,
                             nullptr,
                             nullptr,
                             &te_sector,
                             nullptr)) {
        k_mutex_unlock(&prefetch_state_lock_);
        if (should_emit_debug_log(&last_rebase_skip_state_log_ms)) {
            LOG_WRN("rebase skip: resolve_album_frame failed frame=%u", frame_index);
        }
        return;
    }
    const size_t replacement = choose_audio_slot_replacement(frame_index, te_sector);
    if (replacement < audio_slots_.size()) {
        uint32_t album_sector_index = 0U;
        if (!album_sector_index_for_te_sector(te_sector, &album_sector_index)) {
            k_mutex_unlock(&prefetch_state_lock_);
            return;
        }
        const uint32_t sample_anchor =
            album_sector_index * kFramesPerSector;
        auto& slot = audio_slots_[replacement];
        slot.ready.store(false, std::memory_order_relaxed);
        slot.state.store(AudioSlotState::Reading, std::memory_order_relaxed);
        slot.te_sector.store(te_sector, std::memory_order_relaxed);
        slot.read_blocks.store(read_blocks, std::memory_order_relaxed);
        seed_plan.valid = true;
        seed_plan.slot_index = replacement;
        seed_plan.te_sector = te_sector;
        seed_plan.sample_anchor = sample_anchor;
        seed_plan.read_blocks = read_blocks;
        sync_seed_in_progress_.store(true, std::memory_order_release);
    }

    k_mutex_unlock(&prefetch_state_lock_);

    if (seed_plan.valid) {
        auto& seed_buffer = prefetch_sectors_[0];
        const uint32_t emmc_block = seed_plan.te_sector * kEmmcBlocksPerSector;
        if (stem_emmc_read_blocks(emmc_dev_, emmc_block, seed_buffer.data(), read_blocks)) {
            if (!load_audio_slot(seed_plan.slot_index,
                                 seed_plan.te_sector,
                                 seed_plan.sample_anchor,
                                 seed_buffer.data(),
                                 read_blocks)) {
                auto& slot = audio_slots_[seed_plan.slot_index];
                slot.state.store(AudioSlotState::Empty, std::memory_order_relaxed);
                slot.ready.store(false, std::memory_order_relaxed);
                slot.te_sector.store(UINT32_MAX, std::memory_order_relaxed);
            }
        } else {
            auto& slot = audio_slots_[seed_plan.slot_index];
            slot.state.store(AudioSlotState::Empty, std::memory_order_relaxed);
            slot.ready.store(false, std::memory_order_relaxed);
            slot.te_sector.store(UINT32_MAX, std::memory_order_relaxed);
            LOG_WRN("rebase_streaming seed-read-failed frame=%u sector=%u read_blocks=%u",
                    frame_index,
                    seed_plan.te_sector,
                    read_blocks);
        }
    }

    k_mutex_lock(&prefetch_state_lock_, K_FOREVER);
    sync_seed_in_progress_.store(false, std::memory_order_release);

    if (!inflight_read_.active) {
        (void)start_missing_audio_slot_read(frame_index, read_blocks);
    }
    k_mutex_unlock(&prefetch_state_lock_);
    wake_prefetch_worker();
}

bool DiskManager::service_streaming(uint32_t frame_index,
                                    bool reverse,
                                    bool scan_active,
                                    uint32_t read_blocks) {
    playback_cursor_hint_.store(frame_index, std::memory_order_relaxed);
    playback_reverse_.store(reverse, std::memory_order_relaxed);
    playback_scan_active_.store(scan_active, std::memory_order_relaxed);
    playback_read_blocks_hint_.store(read_blocks, std::memory_order_relaxed);
    return service_audio_prefetch_window(frame_index, read_blocks);
}

void DiskManager::stop_streaming() {
    set_playback_prefetch_enabled(false);
}

bool DiskManager::playback_prefetch_enabled() const {
    return playback_prefetch_enabled_.load(std::memory_order_relaxed);
}

bool DiskManager::playback_scan_active() const {
    return playback_scan_active_.load(std::memory_order_relaxed);
}

void DiskManager::wake_prefetch_worker() {
    if (storage_work_q_ == nullptr) {
        return;
    }
    prefetch_work_requested_.store(true, std::memory_order_release);
    if (!prefetch_work_queued_.exchange(true, std::memory_order_acq_rel)) {
        (void)k_work_submit_to_queue(storage_work_q_, &prefetch_work_);
    }
}

void DiskManager::prime_playback_window(uint32_t frame_index) {
    playback_cursor_hint_.store(frame_index, std::memory_order_relaxed);
    service_audio_prefetch_window(frame_index);
}

void DiskManager::service_playback_prefetch_once() {
    if (!playback_prefetch_enabled_.load(std::memory_order_relaxed)) {
        return;
    }
    service_audio_prefetch_window(playback_cursor_hint_.load(std::memory_order_relaxed));
}

void DiskManager::seed_playback_window_sync(uint32_t frame_index, size_t slot_count) {
    const uint32_t seed_start = k_cycle_get_32();
    std::array<PlannedRead, kAudioPrefetchSlots> seed_plans{};
    size_t seed_plan_count = 0U;
    k_mutex_lock(&prefetch_state_lock_, K_FOREVER);

    const uint8_t current_song = current_song_.load(std::memory_order_relaxed);
    if (!album_.valid || !emmc_ready_.load(std::memory_order_acquire) ||
        current_song >= album_.song_count || slot_count == 0U) {
        k_mutex_unlock(&prefetch_state_lock_);
        return;
    }

    if (inflight_read_.active || stem_emmc_async_read_in_flight(emmc_dev_)) {
        abort_prefetch_locked();
    }

    const uint32_t total_sectors = album_total_sectors();
    if (total_sectors == 0U) {
        k_mutex_unlock(&prefetch_state_lock_);
        return;
    }

    const uint32_t sector_offset = (frame_index / kFramesPerSector) % total_sectors;
    const bool reverse = playback_reverse_.load(std::memory_order_relaxed);
    const bool scan_active = playback_scan_active_.load(std::memory_order_relaxed);
    const auto wanted_sector_indexes =
        build_directional_sector_window(sector_offset, total_sectors, reverse, scan_active);
    std::array<uint32_t, kAudioPrefetchSlots> wanted{};
    for (size_t i = 0; i < wanted.size(); ++i) {
        wanted[i] = wanted_sector_indexes[i] == UINT32_MAX
            ? UINT32_MAX
            : te_sector_for_album_sector_index(wanted_sector_indexes[i]);
    }

    std::array<bool, kAudioPrefetchSlots> reserved_slots{};
    size_t seeded = 0U;
    for (size_t wanted_idx = 0; wanted_idx < wanted.size() && seeded < slot_count; ++wanted_idx) {
        const uint32_t te_sector = wanted[wanted_idx];
        if (te_sector == UINT32_MAX) {
            continue;
        }
        if (find_audio_slot(te_sector) >= 0) {
            ++seeded;
            continue;
        }

        size_t replacement = audio_slots_.size();
        for (size_t slot_idx = 0; slot_idx < audio_slots_.size(); ++slot_idx) {
            if (reserved_slots[slot_idx]) {
                continue;
            }
            const uint32_t slot_sector =
                audio_slots_[slot_idx].te_sector.load(std::memory_order_relaxed);
            bool keep_slot = false;
            for (uint32_t wanted_sector : wanted) {
                if (slot_sector == wanted_sector &&
                    audio_slots_[slot_idx].ready.load(std::memory_order_acquire)) {
                    keep_slot = true;
                    break;
                }
            }
            if (!keep_slot) {
                replacement = slot_idx;
                break;
            }
        }

        if (replacement >= audio_slots_.size()) {
            continue;
        }

        reserved_slots[replacement] = true;
        auto& slot = audio_slots_[replacement];
        slot.ready.store(false, std::memory_order_relaxed);
        slot.state.store(AudioSlotState::Reading, std::memory_order_relaxed);
        slot.te_sector.store(te_sector, std::memory_order_relaxed);
        slot.read_blocks.store(kEmmcBlocksPerSector, std::memory_order_relaxed);

        seed_plans[seed_plan_count].valid = true;
        seed_plans[seed_plan_count].slot_index = replacement;
        seed_plans[seed_plan_count].te_sector = te_sector;
        seed_plans[seed_plan_count].read_blocks = kEmmcBlocksPerSector;
        ++seed_plan_count;
        ++seeded;
    }

    k_mutex_unlock(&prefetch_state_lock_);

    for (size_t i = 0; i < seed_plan_count; ++i) {
        const auto& plan = seed_plans[i];
        if (!plan.valid) {
            continue;
        }

        uint32_t album_sector_index = 0U;
        if (!album_sector_index_for_te_sector(plan.te_sector, &album_sector_index)) {
            audio_slots_[plan.slot_index].state.store(AudioSlotState::Empty, std::memory_order_relaxed);
            audio_slots_[plan.slot_index].ready.store(false, std::memory_order_relaxed);
            audio_slots_[plan.slot_index].te_sector.store(UINT32_MAX, std::memory_order_relaxed);
            continue;
        }

        auto& seed_buffer = prefetch_sectors_[0];
        const uint32_t emmc_block = plan.te_sector * kEmmcBlocksPerSector;
        if (!stem_emmc_read_blocks(emmc_dev_,
                                   emmc_block,
                                   seed_buffer.data(),
                                   kEmmcBlocksPerSector)) {
            audio_slots_[plan.slot_index].state.store(AudioSlotState::Empty, std::memory_order_relaxed);
            audio_slots_[plan.slot_index].ready.store(false, std::memory_order_relaxed);
            audio_slots_[plan.slot_index].te_sector.store(UINT32_MAX, std::memory_order_relaxed);
            break;
        }

        const uint32_t sample_anchor =
            album_sector_index * kFramesPerSector;
        if (!load_audio_slot(plan.slot_index,
                             plan.te_sector,
                             sample_anchor,
                             seed_buffer.data(),
                             kEmmcBlocksPerSector)) {
            audio_slots_[plan.slot_index].state.store(AudioSlotState::Empty, std::memory_order_relaxed);
            audio_slots_[plan.slot_index].ready.store(false, std::memory_order_relaxed);
            audio_slots_[plan.slot_index].te_sector.store(UINT32_MAX, std::memory_order_relaxed);
        }
    }

    const uint32_t seed_cycles = k_cycle_get_32() - seed_start;
    const k_spinlock_key_t key = k_spin_lock(&perf_lock_);
    ++perf_stats_.sync_seed_count;
    update_perf_sample(&perf_stats_.sync_seed_total_cycles,
                       &perf_stats_.sync_seed_max_cycles,
                       seed_cycles);
    k_spin_unlock(&perf_lock_, key);
}

void DiskManager::read_stem_frame(int stem_idx,
                                  uint64_t frame_index,
                                  float* left_sample,
                                  float* right_sample) {
    if (left_sample == nullptr || right_sample == nullptr) {
        return;
    }

    if (kForceSyntheticStemData) {
        *left_sample = 0.0f;
        *right_sample = 0.0f;
        return;
    }

    int slot_index = -1;
    size_t block_index = 0U;
    size_t frame_in_block = 0U;
    if (resolve_slot_frame(frame_index,
                           nullptr,
                           nullptr,
                           &slot_index,
                           &block_index,
                           &frame_in_block)) {
        const auto& slot = audio_slots_[static_cast<size_t>(slot_index)];
        const uint8_t* frame_payload =
            block_frame_payload_ptr(slot, block_index, frame_in_block);
        if (frame_payload != nullptr) {
            decode_stem_from_frame_payload(frame_payload,
                                           stem_idx,
                                           left_sample,
                                           right_sample);
            return;
        }
    }

    *left_sample = 0.0f;
    *right_sample = 0.0f;
}

bool DiskManager::try_read_stem_frame_prefetched(int stem_idx,
                                                 uint64_t frame_index,
                                                 float* left_sample,
                                                 float* right_sample) const {
    if (left_sample == nullptr || right_sample == nullptr) {
        return false;
    }

    int slot_index = -1;
    size_t block_index = 0U;
    size_t frame_in_block = 0U;
    if (!resolve_slot_frame(frame_index,
                            nullptr,
                            nullptr,
                            &slot_index,
                            &block_index,
                            &frame_in_block)) {
        return false;
    }

    const auto& slot = audio_slots_[static_cast<size_t>(slot_index)];
    const uint8_t* frame_payload =
        block_frame_payload_ptr(slot, block_index, frame_in_block);
    if (frame_payload == nullptr) {
        return false;
    }
    decode_stem_from_frame_payload(frame_payload,
                                   stem_idx,
                                   left_sample,
                                   right_sample);
    return true;
}

bool DiskManager::try_read_stem_frame_prefetched_cached(int stem_idx,
                                                        uint64_t frame_index,
                                                        uint32_t* cached_sector,
                                                        int* cached_slot_index,
                                                        float* left_sample,
                                                        float* right_sample) const {
    if (left_sample == nullptr || right_sample == nullptr) {
        return false;
    }

    int slot_index = -1;
    size_t block_index = 0U;
    size_t frame_in_block = 0U;
    if (!resolve_slot_frame(frame_index,
                            cached_sector,
                            cached_slot_index,
                            &slot_index,
                            &block_index,
                            &frame_in_block)) {
        return false;
    }

    const auto& slot = audio_slots_[static_cast<size_t>(slot_index)];
    const uint8_t* frame_payload =
        block_frame_payload_ptr(slot, block_index, frame_in_block);
    if (frame_payload == nullptr) {
        return false;
    }
    decode_stem_from_frame_payload(frame_payload,
                                   stem_idx,
                                   left_sample,
                                   right_sample);
    return true;
}

bool DiskManager::try_read_mixed_frame_prefetched(uint64_t frame_index,
                                                  float* left_sample,
                                                  float* right_sample) {
    uint32_t cached_sector = UINT32_MAX;
    int cached_slot_index = -1;
    return try_read_mixed_frame_prefetched_cached(frame_index,
                                                  &cached_sector,
                                                  &cached_slot_index,
                                                  left_sample,
                                                  right_sample);
}

bool DiskManager::try_read_mixed_frame_prefetched_cached(uint64_t frame_index,
                                                        uint32_t* cached_sector,
                                                        int* cached_slot_index,
                                                        float* left_sample,
                                                        float* right_sample) {
    if (left_sample == nullptr || right_sample == nullptr) {
        return false;
    }

    int slot_index = -1;
    size_t block_index = 0U;
    size_t frame_in_block = 0U;
    if (!resolve_slot_frame(frame_index,
                            cached_sector,
                            cached_slot_index,
                            &slot_index,
                            &block_index,
                            &frame_in_block)) {
        return false;
    }

    const auto& slot = audio_slots_[static_cast<size_t>(slot_index)];
    const uint8_t* frame_payload = block_frame_payload_ptr(slot, block_index, frame_in_block);
    if (frame_payload == nullptr) {
        return false;
    }
    DecodedStemFrameI32 frame{};
    decode_te_frame_payload_i32(frame_payload, &frame);

    // Sum channels together using fast integer math
    int32_t mix_left_i32 = 0;
    int32_t mix_right_i32 = 0;
    for (size_t stem = 0; stem < kStemsPerFrame; ++stem) {
        mix_left_i32 += frame.left[stem];
        mix_right_i32 += frame.right[stem];
    }

    // Multiply at the very end
    constexpr float kMixMultiplier = 0.25f / 8388608.0f;

    *left_sample = static_cast<float>(mix_left_i32) * kMixMultiplier;
    *right_sample = static_cast<float>(mix_right_i32) * kMixMultiplier;
    return true;
}

bool DiskManager::try_read_all_stems_frame_prefetched_cached(
    uint64_t frame_index,
    uint32_t* cached_sector,
    int* cached_slot_index,
    DecodedStemFrameI32* frame_out) const {
    if (frame_out == nullptr) {
        return false;
    }

    int slot_index = -1;
    size_t block_index = 0U;
    size_t frame_in_block = 0U;
    if (!resolve_slot_frame(frame_index,
                            cached_sector,
                            cached_slot_index,
                            &slot_index,
                            &block_index,
                            &frame_in_block)) {
        return false;
    }

    const auto& slot = audio_slots_[static_cast<size_t>(slot_index)];
    const uint8_t* frame_payload =
        block_frame_payload_ptr(slot, block_index, frame_in_block);
    if (frame_payload == nullptr) {
        return false;
    }
    decode_te_frame_payload_i32(frame_payload, frame_out);
    return true;
}

bool DiskManager::try_get_frame_payload_cached(uint64_t frame_index,
                                               uint32_t* cached_sector,
                                               int* cached_slot_index,
                                               const uint8_t** frame_payload_out,
                                               size_t* frame_in_sector_out) const {
    if (frame_payload_out == nullptr || frame_in_sector_out == nullptr) {
        return false;
    }

    int slot_index = -1;
    size_t block_index = 0U;
    size_t frame_in_block = 0U;
    size_t frame_in_sector = 0U;
    if (!resolve_slot_frame(frame_index,
                            cached_sector,
                            cached_slot_index,
                            &slot_index,
                            &block_index,
                            &frame_in_block,
                            &frame_in_sector)) {
        return false;
    }

    const auto& slot = audio_slots_[static_cast<size_t>(slot_index)];
    *frame_payload_out = block_frame_payload_ptr(slot, block_index, frame_in_block);
    if (*frame_payload_out == nullptr) {
        return false;
    }
    *frame_in_sector_out = frame_in_sector;
    return true;
}

bool DiskManager::try_get_frame_source_cached(uint64_t frame_index,
                                              uint32_t* cached_sector,
                                              int* cached_slot_index,
                                              uint32_t* te_sector_out,
                                              const uint8_t** frame_payload_out,
                                              size_t* frame_in_sector_out,
                                              uint32_t* read_blocks_out) const {
    if (te_sector_out == nullptr || frame_payload_out == nullptr ||
        frame_in_sector_out == nullptr || read_blocks_out == nullptr) {
        return false;
    }

    int slot_index = -1;
    size_t block_index = 0U;
    size_t frame_in_block = 0U;
    size_t frame_in_sector = 0U;
    if (!resolve_slot_frame(frame_index,
                            cached_sector,
                            cached_slot_index,
                            &slot_index,
                            &block_index,
                            &frame_in_block,
                            &frame_in_sector)) {
        return false;
    }

    const auto& slot = audio_slots_[static_cast<size_t>(slot_index)];
    *te_sector_out = slot.te_sector.load(std::memory_order_relaxed);
    *read_blocks_out = slot.read_blocks.load(std::memory_order_relaxed);
    *frame_payload_out = block_frame_payload_ptr(slot, block_index, frame_in_block);
    if (*frame_payload_out == nullptr) {
        return false;
    }
    *frame_in_sector_out = frame_in_sector;
    return true;
}

bool DiskManager::try_get_sector_payload_cached(uint64_t frame_index,
                                                uint32_t* cached_sector,
                                                int* cached_slot_index,
                                                uint32_t* te_sector_out,
                                                const uint8_t** sector_payload_out,
                                                size_t* frame_in_sector_out,
                                                uint32_t* read_blocks_out) const {
    if (te_sector_out == nullptr || sector_payload_out == nullptr ||
        frame_in_sector_out == nullptr || read_blocks_out == nullptr) {
        return false;
    }

    int slot_index = -1;
    size_t block_index = 0U;
    size_t frame_in_block = 0U;
    size_t frame_in_sector = 0U;
    if (!resolve_slot_frame(frame_index,
                            cached_sector,
                            cached_slot_index,
                            &slot_index,
                            &block_index,
                            &frame_in_block,
                            &frame_in_sector)) {
        return false;
    }

    const auto& slot = audio_slots_[static_cast<size_t>(slot_index)];
    *te_sector_out = slot.te_sector.load(std::memory_order_relaxed);
    *read_blocks_out = slot.read_blocks.load(std::memory_order_relaxed);
    *sector_payload_out = slot.record.payload.data();
    *frame_in_sector_out = frame_in_sector;
    return true;
}

bool DiskManager::map_frame_to_sector(uint64_t frame_index,
                                      uint32_t* te_sector_out,
                                      size_t* frame_in_sector_out) const {
    if (te_sector_out == nullptr || frame_in_sector_out == nullptr) {
        return false;
    }

    return resolve_album_frame(frame_index,
                               nullptr,
                               nullptr,
                               nullptr,
                               te_sector_out,
                               frame_in_sector_out);
}

bool DiskManager::resolve_block_run(uint64_t frame_index,
                                    uint32_t* cached_sector,
                                    int* cached_slot_index,
                                    const uint8_t** audio_ptr_out,
                                    size_t* frame_in_block_out,
                                    size_t* frames_available_out) const {
    if (audio_ptr_out == nullptr || frame_in_block_out == nullptr ||
        frames_available_out == nullptr) {
        return false;
    }

    int slot_index = -1;
    size_t block_index = 0U;
    size_t frame_in_block = 0U;
    if (!resolve_slot_frame(frame_index,
                            cached_sector,
                            cached_slot_index,
                            &slot_index,
                            &block_index,
                            &frame_in_block)) {
        return false;
    }

    const auto& slot = audio_slots_[static_cast<size_t>(slot_index)];
    const uint8_t* audio_ptr =
        block_frame_payload_ptr(slot, block_index, frame_in_block);
    if (audio_ptr == nullptr) {
        const k_spinlock_key_t key = k_spin_lock(&perf_lock_);
        ++perf_stats_.resolve_audio_ptr_fail_count;
        k_spin_unlock(&perf_lock_, key);
        return false;
    }

    *audio_ptr_out = audio_ptr;
    *frame_in_block_out = frame_in_block;
    *frames_available_out = kFramesPerBlock - frame_in_block;
    return true;
}

bool DiskManager::try_get_track_leds_cached(uint64_t frame_index,
                                            uint32_t* cached_sector,
                                            int* cached_slot_index,
                                            std::array<uint8_t, 4>* led_levels_out) const {
    if (led_levels_out == nullptr) {
        return false;
    }

    uint32_t led_word = 0U;
    if (!try_get_block_side_data_cached(frame_index,
                                        cached_sector,
                                        cached_slot_index,
                                        nullptr,
                                        &led_word)) {
        return false;
    }
    (*led_levels_out)[0] = static_cast<uint8_t>(led_word & 0xFFU);
    (*led_levels_out)[1] = static_cast<uint8_t>((led_word >> 8) & 0xFFU);
    (*led_levels_out)[2] = static_cast<uint8_t>((led_word >> 16) & 0xFFU);
    (*led_levels_out)[3] = static_cast<uint8_t>((led_word >> 24) & 0xFFU);
    return true;
}

bool DiskManager::try_get_block_side_data_cached(uint64_t frame_index,
                                                 uint32_t* cached_sector,
                                                 int* cached_slot_index,
                                                 uint32_t* sync_word_out,
                                                 uint32_t* led_word_out) const {
    int slot_index = -1;
    size_t block_index = 0U;
    size_t frame_in_block = 0U;
    size_t frame_in_sector = 0U;
    if (!resolve_slot_frame(frame_index,
                            cached_sector,
                            cached_slot_index,
                            &slot_index,
                            &block_index,
                            &frame_in_block,
                            &frame_in_sector)) {
        return false;
    }

    const auto& slot = audio_slots_[static_cast<size_t>(slot_index)];
    if (block_index >= slot.record.block_led_words.size() ||
        block_index >= slot.record.block_sync_words.size()) {
        return false;
    }

    if (sync_word_out != nullptr) {
        *sync_word_out = slot.record.block_sync_words[block_index];
    }
    if (led_word_out != nullptr) {
        *led_word_out = slot.record.block_led_words[block_index];
    }
    return true;
}

bool DiskManager::try_read_mono_frame_prefetched_cached(uint64_t frame_index,
                                                        uint32_t* cached_sector,
                                                        int* cached_slot_index,
                                                        float* mono_sample) {
    if (mono_sample == nullptr) {
        return false;
    }

    int slot_index = -1;
    size_t block_index = 0U;
    size_t frame_in_block = 0U;
    if (!resolve_slot_frame(frame_index,
                            cached_sector,
                            cached_slot_index,
                            &slot_index,
                            &block_index,
                            &frame_in_block)) {
        return false;
    }

    const auto& slot = audio_slots_[static_cast<size_t>(slot_index)];
    const uint8_t* frame_payload = block_frame_payload_ptr(slot, block_index, frame_in_block);
    if (frame_payload == nullptr) {
        return false;
    }
    DecodedStemFrameI32 frame{};
    decode_te_frame_payload_i32(frame_payload, &frame);

    // Sum all 8 channels together as 32-bit integers first!
    // (Max possible sum of 8 24-bit ints fits perfectly inside a 32-bit int)
    int32_t mono_mix_i32 = 0;
    for (size_t stem = 0; stem < kStemsPerFrame; ++stem) {
        mono_mix_i32 += frame.left[stem];
        mono_mix_i32 += frame.right[stem];
    }

    // Multiply at the very end. The compiler precalculates this constant.
    constexpr float kMonoMixMultiplier = 0.125f / 8388608.0f;

    *mono_sample = static_cast<float>(mono_mix_i32) * kMonoMixMultiplier;
    return true;
}

void DiskManager::render_mono_right24_frames_prefetched_cached(
    uint64_t start_frame,
    size_t frame_count,
    uint32_t* cached_sector,
    int* cached_slot_index,
    int32_t* interleaved_output) {
    if (interleaved_output == nullptr) {
        return;
    }

    const uint32_t total = album_total_frames_internal();
    if (total == 0U) {
        std::fill_n(interleaved_output, frame_count * 2U, 0);
        return;
    }

    constexpr int32_t kDirectMonoPcmDivisor = 44;

    uint64_t wrapped = start_frame % total;
    size_t frame = 0;
    while (frame < frame_count) {
        uint32_t te_sector = UINT32_MAX;
        size_t frame_in_sector = 0U;
        if (!resolve_album_frame(wrapped,
                                 nullptr,
                                 nullptr,
                                 nullptr,
                                 &te_sector,
                                 &frame_in_sector)) {
            std::fill_n(interleaved_output + frame * 2U, (frame_count - frame) * 2U, 0);
            return;
        }
        const size_t block_index = frame_in_sector / kFramesPerBlock;
        const size_t frame_in_block = frame_in_sector % kFramesPerBlock;
        const size_t frames_this_block =
            std::min(frame_count - frame, kFramesPerBlock - frame_in_block);

        int slot_index = -1;
        if (!resolve_slot_for_sector(te_sector,
                                     cached_sector,
                                     cached_slot_index,
                                     &slot_index)) {
            for (size_t local = 0; local < frames_this_block; ++local) {
                const size_t base = (frame + local) * 2U;
                interleaved_output[base] = 0;
                interleaved_output[base + 1U] = 0;
            }
        } else {
            const auto& slot = audio_slots_[static_cast<size_t>(slot_index)];
            const uint8_t* frame_payload =
                block_frame_payload_ptr(slot, block_index, frame_in_block);
            if (frame_payload == nullptr) {
                for (size_t local = 0; local < frames_this_block; ++local) {
                    const size_t base = (frame + local) * 2U;
                    interleaved_output[base] = 0;
                    interleaved_output[base + 1U] = 0;
                }
            } else {
                for (size_t local = 0; local < frames_this_block; ++local) {
                    DecodedStemFrameI32 decoded{};
                    decode_te_frame_payload_i32(frame_payload, &decoded);
                    int32_t sum = 0;
                    for (size_t stem = 0; stem < kStemsPerFrame; ++stem) {
                        sum += decoded.left[stem];
                        sum += decoded.right[stem];
                    }

                    int32_t pcm = sum / kDirectMonoPcmDivisor;
                    pcm = std::clamp(pcm, -8388608, 8388607);
                    const size_t base = (frame + local) * 2U;
                    interleaved_output[base] = pcm;
                    interleaved_output[base + 1U] = pcm;
                    frame_payload += kFrameSize;
                }
            }
        }
        frame += frames_this_block;
        wrapped += frames_this_block;
        if (wrapped >= total) {
            wrapped %= total;
            if (cached_sector != nullptr) {
                *cached_sector = UINT32_MAX;
            }
            if (cached_slot_index != nullptr) {
                *cached_slot_index = -1;
            }
        }
    }
}

void DiskManager::render_weighted_mono_right24_frames_prefetched_cached(
    uint64_t start_frame,
    size_t frame_count,
    uint32_t* cached_sector,
    int* cached_slot_index,
    const std::array<uint16_t, 4>& stem_weights_q12,
    int32_t* interleaved_output) {
    if (interleaved_output == nullptr) {
        return;
    }

    const uint32_t total = album_total_frames_internal();
    if (total == 0U) {
        std::fill_n(interleaved_output, frame_count * 2U, 0);
        return;
    }

    constexpr int32_t kDirectWeightedMonoPcmDivisor = 44;
    constexpr int32_t kStemWeightScaleQ12 = 4096;

    uint64_t wrapped = start_frame % total;
    size_t frame = 0U;
    while (frame < frame_count) {
        uint32_t te_sector = UINT32_MAX;
        size_t frame_in_sector = 0U;
        if (!resolve_album_frame(wrapped,
                                 nullptr,
                                 nullptr,
                                 nullptr,
                                 &te_sector,
                                 &frame_in_sector)) {
            std::fill_n(interleaved_output + frame * 2U, (frame_count - frame) * 2U, 0);
            return;
        }
        const size_t block_index = frame_in_sector / kFramesPerBlock;
        const size_t frame_in_block = frame_in_sector % kFramesPerBlock;
        const size_t frames_this_block =
            std::min(frame_count - frame, kFramesPerBlock - frame_in_block);

        int slot_index = -1;
        if (!resolve_slot_for_sector(te_sector,
                                     cached_sector,
                                     cached_slot_index,
                                     &slot_index)) {
            for (size_t local = 0; local < frames_this_block; ++local) {
                const size_t base = (frame + local) * 2U;
                interleaved_output[base] = 0;
                interleaved_output[base + 1U] = 0;
            }
        } else {
            const auto& slot = audio_slots_[static_cast<size_t>(slot_index)];
            const uint8_t* frame_payload =
                block_frame_payload_ptr(slot, block_index, frame_in_block);
            if (frame_payload == nullptr) {
                for (size_t local = 0; local < frames_this_block; ++local) {
                    const size_t base = (frame + local) * 2U;
                    interleaved_output[base] = 0;
                    interleaved_output[base + 1U] = 0;
                }
            } else {
                for (size_t local = 0; local < frames_this_block; ++local) {
                    DecodedStemFrameI32 decoded{};
                    decode_te_frame_payload_i32(frame_payload, &decoded);
                    int64_t sum_q12 = 0;
                    for (size_t stem = 0; stem < kStemsPerFrame; ++stem) {
                        const int32_t stereo_sum = decoded.left[stem] + decoded.right[stem];
                        sum_q12 += static_cast<int64_t>(stereo_sum) *
                                   static_cast<int64_t>(stem_weights_q12[stem]);
                    }

                    int64_t pcm =
                        sum_q12 / (kStemWeightScaleQ12 *
                                   kDirectWeightedMonoPcmDivisor);
                    pcm = std::clamp<int64_t>(pcm, -8388608, 8388607);
                    const size_t base = (frame + local) * 2U;
                    interleaved_output[base] = static_cast<int32_t>(pcm);
                    interleaved_output[base + 1U] = static_cast<int32_t>(pcm);
                    frame_payload += kFrameSize;
                }
            }
        }

        frame += frames_this_block;
        wrapped += frames_this_block;
        if (wrapped >= total) {
            wrapped %= total;
            if (cached_sector != nullptr) {
                *cached_sector = UINT32_MAX;
            }
            if (cached_slot_index != nullptr) {
                *cached_slot_index = -1;
            }
        }
    }
}

void DiskManager::render_weighted_stereo_right24_frames_prefetched_cached(
    uint64_t start_frame,
    size_t frame_count,
    uint32_t* cached_sector,
    int* cached_slot_index,
    const std::array<uint16_t, 4>& stem_weights_q12,
    int32_t* interleaved_output) {
    if (interleaved_output == nullptr) {
        return;
    }

    const uint32_t total = album_total_frames_internal();
    if (total == 0U) {
        std::fill_n(interleaved_output, frame_count * 2U, 0);
        return;
    }

    constexpr int32_t kStemWeightScaleQ12 = 4096;
    constexpr int32_t kDirectWeightedStereoPcmDivisor = 22;

    uint64_t wrapped = start_frame % total;
    size_t frame = 0U;
    while (frame < frame_count) {
        uint32_t te_sector = UINT32_MAX;
        size_t frame_in_sector = 0U;
        if (!resolve_album_frame(wrapped,
                                 nullptr,
                                 nullptr,
                                 nullptr,
                                 &te_sector,
                                 &frame_in_sector)) {
            std::fill_n(interleaved_output + frame * 2U, (frame_count - frame) * 2U, 0);
            return;
        }
        const size_t block_index = frame_in_sector / kFramesPerBlock;
        const size_t frame_in_block = frame_in_sector % kFramesPerBlock;
        const size_t frames_this_block =
            std::min(frame_count - frame, kFramesPerBlock - frame_in_block);

        int slot_index = -1;
        if (!resolve_slot_for_sector(te_sector,
                                     cached_sector,
                                     cached_slot_index,
                                     &slot_index)) {
            for (size_t local = 0; local < frames_this_block; ++local) {
                const size_t base = (frame + local) * 2U;
                interleaved_output[base] = 0;
                interleaved_output[base + 1U] = 0;
            }
        } else {
            const auto& slot = audio_slots_[static_cast<size_t>(slot_index)];
            const uint8_t* frame_payload =
                block_frame_payload_ptr(slot, block_index, frame_in_block);
            if (frame_payload == nullptr) {
                for (size_t local = 0; local < frames_this_block; ++local) {
                    const size_t base = (frame + local) * 2U;
                    interleaved_output[base] = 0;
                    interleaved_output[base + 1U] = 0;
                }
            } else {
                for (size_t local = 0; local < frames_this_block; ++local) {
                    DecodedStemFrameI32 decoded{};
                    decode_te_frame_payload_i32(frame_payload, &decoded);
                    int64_t sum_left_q12 = 0;
                    int64_t sum_right_q12 = 0;
                    for (size_t stem = 0; stem < kStemsPerFrame; ++stem) {
                        sum_left_q12 += static_cast<int64_t>(decoded.left[stem]) *
                                        static_cast<int64_t>(stem_weights_q12[stem]);
                        sum_right_q12 += static_cast<int64_t>(decoded.right[stem]) *
                                         static_cast<int64_t>(stem_weights_q12[stem]);
                    }

                    // Shift out Q12 fraction first, dropping back to 32-bit bounds.
                    // The CPU will use the fast 2-cycle hardware divider for the / 22 !
                    int32_t pcm_left = static_cast<int32_t>(sum_left_q12 / kStemWeightScaleQ12) / kDirectWeightedStereoPcmDivisor;
                    int32_t pcm_right = static_cast<int32_t>(sum_right_q12 / kStemWeightScaleQ12) / kDirectWeightedStereoPcmDivisor;

                    pcm_left = std::clamp<int32_t>(pcm_left, -8388608, 8388607);
                    pcm_right = std::clamp<int32_t>(pcm_right, -8388608, 8388607);
                    const size_t base = (frame + local) * 2U;
                    interleaved_output[base] = pcm_left;
                    interleaved_output[base + 1U] = pcm_right;
                    frame_payload += kFrameSize;
                }
            }
        }

        frame += frames_this_block;
        wrapped += frames_this_block;
        if (wrapped >= total) {
            wrapped %= total;
            if (cached_sector != nullptr) {
                *cached_sector = UINT32_MAX;
            }
            if (cached_slot_index != nullptr) {
                *cached_slot_index = -1;
            }
        }
    }
}

void DiskManager::read_stem_data(int stem_idx, float* out_buffer, size_t num_samples) {
    if (out_buffer == nullptr) {
        return;
    }

    const size_t frame_count = num_samples / 2U;
    for (size_t frame = 0; frame < frame_count; ++frame) {
        float left = 0.0f;
        float right = 0.0f;
        read_stem_frame(stem_idx, frame, &left, &right);
        out_buffer[frame * 2U] = left;
        out_buffer[frame * 2U + 1U] = right;
    }

    if ((num_samples & 1U) != 0U) {
        out_buffer[num_samples - 1U] = 0.0f;
    }
}

bool DiskManager::read_blocks(uint32_t block_addr, uint8_t* buffer,
    uint32_t num_blocks) {
    if (!emmc_ready_.load(std::memory_order_acquire)) {
        return false;
    }
    
    k_mutex_lock(&prefetch_state_lock_, K_FOREVER);
    const bool async_busy =
    inflight_read_.active || stem_emmc_async_read_in_flight(emmc_dev_);
    if (async_busy) {
        k_mutex_unlock(&prefetch_state_lock_);
        return false;
    }
    // Use the sync serial path directly — the Zephyr FS buffer has no +2 padding.
    const bool success = stem_emmc_read_blocks_sync(emmc_dev_, block_addr, buffer, num_blocks);
    k_mutex_unlock(&prefetch_state_lock_);
    return success;
}

uint32_t DiskManager::emmc_capacity_blocks() const {
    return stem_emmc_capacity_blocks(emmc_dev_);
}

void DiskManager::set_current_song(uint8_t song_idx) {
    if (song_idx < album_.song_count) {
        k_mutex_lock(&prefetch_state_lock_, K_FOREVER);
        if (current_song_.load(std::memory_order_relaxed) != song_idx) {
            if (inflight_read_.active || stem_emmc_async_read_in_flight(emmc_dev_)) {
                abort_prefetch_locked();
            } else {
                reset_prefetch_tracking_locked();
            }
            for (auto& slot : audio_slots_) {
                slot.ready.store(false, std::memory_order_relaxed);
            }
            current_song_.store(song_idx, std::memory_order_relaxed);
        }
        k_mutex_unlock(&prefetch_state_lock_);
        LOG_INF("Song %u selected: \"%s\"", song_idx, album_.songs[song_idx].title);
    }
}

uint32_t DiskManager::song_start_frame(uint8_t song_idx) const {
    return song_start_frame_internal(song_idx);
}

uint8_t DiskManager::song_index_for_frame(uint32_t frame_index) const {
    uint8_t song_index = current_song_.load(std::memory_order_relaxed);
    if (!resolve_album_frame(frame_index,
                             &song_index,
                             nullptr,
                             nullptr,
                             nullptr,
                             nullptr)) {
        return current_song_.load(std::memory_order_relaxed);
    }
    return song_index;
}

uint32_t DiskManager::total_frames() const {
    return album_total_frames_internal();
}

void DiskManager::init_work_handler(struct k_work* work) {
    auto* delayable = k_work_delayable_from_work(work);
    auto* self = CONTAINER_OF(delayable, DiskManager, init_work_);
    if (self == nullptr || self->init_attempted_) {
        return;
    }

    self->run_init_attempt();
}

void DiskManager::prefetch_work_handler(struct k_work* work) {
    auto* self = CONTAINER_OF(work, DiskManager, prefetch_work_);
    if (self == nullptr) {
        return;
    }

    self->run_prefetch_work();
}

void DiskManager::run_init_attempt() {
    init_attempted_ = true;

    if (!device_is_ready(emmc_dev_)) {
        LOG_WRN("eMMC device wrapper is not ready");
        return;
    }

    LOG_INF("Disk Manager probing eMMC device (ready=%d)",
            device_is_ready(emmc_dev_) ? 1 : 0);
    const bool emmc_initialized = stem_emmc_card_init(emmc_dev_);
    if (!emmc_initialized) {
        LOG_WRN("eMMC init failed in background — keeping synthetic fallback audio");
        return;
    }

    LOG_INF("eMMC ready — %u MB available", stem_emmc_capacity_blocks(emmc_dev_) / 2048);

    if (!register_disk_driver()) {
        LOG_WRN("eMMC is usable for audio but disk registration failed");
    }

    if (!parse_album_header()) {
        LOG_WRN("Album header unavailable — storage stays ready, audio fallback remains synthetic");
    }

    emmc_ready_.store(true, std::memory_order_release);
}

void DiskManager::run_prefetch_work() {
    while (true) {
        prefetch_work_requested_.store(false, std::memory_order_release);

        while (playback_prefetch_enabled()) {
            const bool progress = service_streaming_hint_once();
            if (stem_emmc_async_read_in_flight(emmc_dev_)) {
                const bool scan_active = playback_scan_active();
                const k_timeout_t wait_timeout =
                    scan_active ? K_MSEC(1) : K_MSEC(4);
                (void)stem_emmc_wait_for_async_read_signal(emmc_dev_, wait_timeout);
                continue;
            }

            if (!progress) {
                break;
            }
        }

        if (!prefetch_work_requested_.exchange(false, std::memory_order_acq_rel)) {
            break;
        }
    }

    prefetch_work_queued_.store(false, std::memory_order_release);
    if (prefetch_work_requested_.exchange(false, std::memory_order_acq_rel) &&
        !prefetch_work_queued_.exchange(true, std::memory_order_acq_rel) &&
        storage_work_q_ != nullptr) {
        (void)k_work_submit_to_queue(storage_work_q_, &prefetch_work_);
    }
}

} // namespace storage
