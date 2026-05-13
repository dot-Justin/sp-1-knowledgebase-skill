#pragma once

#include <zephyr/device.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/disk.h>
#include "storage/emmc/StemEmmcDevice.hpp"
#include <array>
#include <atomic>
#include <cstdint>
#include <limits>
#include <zephyr/spinlock.h>

namespace storage {

// Teenage Engineering audio data format constants
static constexpr size_t kSectorSize = 8192;        // 1 TE sector = 8192 bytes
// Keep a small aligned slack region on each staging buffer. The async path now
// copies tightly packed payload blocks into these buffers, but the padding keeps
// the existing aligned staging layout intact.
static constexpr size_t kSectorDmaSize = kSectorSize + 4;
static constexpr size_t kBlockSize = 2048;          // 4 blocks per sector
static constexpr size_t kBlocksPerSector = 4;
static constexpr size_t kFramesPerSector = 340;     // 340 audio frames per sector
static constexpr size_t kFramesPerBlock = 85;
static constexpr size_t kFrameSize = 24;            // 8 channels × 3 bytes
static constexpr size_t kStemFrameSize = 6;         // 2 channels × 3 bytes
static constexpr size_t kStemsPerFrame = 4;
static constexpr size_t kBlockAudioSize = kFramesPerBlock * kFrameSize;
static constexpr size_t kEmmcBlocksPerSector = 16;  // 8192 / 512 = 16 eMMC blocks
static constexpr size_t kAudioPrefetchSlots = 10;
static constexpr size_t kSlotTrailerWords = 6;

// Full-sector playback still matches the stock layout better with 0,2,1,3.
static constexpr int kBlockOrder[4] = {0, 2, 1, 3};

static constexpr size_t kMaxSongs = 16;

struct SongInfo {
    uint32_t start_sector;   // Start offset in TE sectors (8192-byte units)
    uint32_t length_sectors; // Length in TE sectors
    char artist[65];
    char title[65];
};

struct AlbumInfo {
    bool valid = false;
    uint32_t total_sectors = 0;
    uint8_t song_count = 0;
    char title[65] = {};
    SongInfo songs[kMaxSongs] = {};
};

struct DecodedStemFrameI32 {
    std::array<int32_t, kStemsPerFrame> left{};
    std::array<int32_t, kStemsPerFrame> right{};
};

enum class AudioSlotState : uint8_t {
    Empty = 0,
    Queued = 1,
    Reading = 2,
    Ready = 3,
};

inline void decode_te_frame_payload_i32(const uint8_t* frame_payload,
    DecodedStemFrameI32* frame_out) {
    if (frame_payload == nullptr || frame_out == nullptr) {
        return;
    }

    for (size_t stem = 0; stem < kStemsPerFrame; ++stem) {
        const uint8_t* data = frame_payload + stem * kStemFrameSize;

        frame_out->left[stem] = (static_cast<int32_t>(static_cast<int8_t>(data[1])) << 16) |
                                (static_cast<uint32_t>(data[0]) << 8) |
                                static_cast<uint32_t>(data[3]);

        frame_out->right[stem] = (static_cast<int32_t>(static_cast<int8_t>(data[2])) << 16) |
                                (static_cast<uint32_t>(data[5]) << 8) |
                                static_cast<uint32_t>(data[4]);
    }
}


class DiskManager {
public:
    struct PerfStats {
        uint32_t service_hint_count = 0U;
        uint64_t service_hint_total_cycles = 0U;
        uint32_t service_hint_max_cycles = 0U;
        uint32_t prefetch_count = 0U;
        uint64_t prefetch_total_cycles = 0U;
        uint32_t prefetch_max_cycles = 0U;
        uint32_t poll_count = 0U;
        uint64_t poll_total_cycles = 0U;
        uint32_t poll_max_cycles = 0U;
        uint32_t start_read_count = 0U;
        uint64_t start_read_total_cycles = 0U;
        uint32_t start_read_max_cycles = 0U;
        uint32_t load_slot_count = 0U;
        uint64_t load_slot_total_cycles = 0U;
        uint32_t load_slot_max_cycles = 0U;
        uint32_t complete_count = 0U;
        uint64_t complete_total_cycles = 0U;
        uint32_t complete_max_cycles = 0U;
        uint32_t load_full_count = 0U;
        uint64_t load_full_total_cycles = 0U;
        uint32_t load_full_max_cycles = 0U;
        uint32_t load_half_count = 0U;
        uint64_t load_half_total_cycles = 0U;
        uint32_t load_half_max_cycles = 0U;
        uint32_t load_quarter_count = 0U;
        uint64_t load_quarter_total_cycles = 0U;
        uint32_t load_quarter_max_cycles = 0U;
        uint32_t sync_seed_count = 0U;
        uint64_t sync_seed_total_cycles = 0U;
        uint32_t sync_seed_max_cycles = 0U;
        uint32_t resolve_cache_hit_count = 0U;
        uint32_t resolve_slot_not_found_count = 0U;
        uint32_t resolve_slot_not_ready_count = 0U;
        uint32_t resolve_map_fail_count = 0U;
        uint32_t resolve_audio_ptr_fail_count = 0U;
    };

    void init();
    bool init_blocking_for_audio();
    void set_storage_work_queue(struct k_work_q* work_q);

    void read_stem_frame(int stem_idx,
                         uint64_t frame_index,
                         float* left_sample,
                         float* right_sample);
    bool try_read_stem_frame_prefetched(int stem_idx,
                                        uint64_t frame_index,
                                        float* left_sample,
                                        float* right_sample) const;
    bool try_read_stem_frame_prefetched_cached(int stem_idx,
                                               uint64_t frame_index,
                                               uint32_t* cached_sector,
                                               int* cached_slot_index,
                                               float* left_sample,
                                               float* right_sample) const;
    bool try_read_mixed_frame_prefetched(uint64_t frame_index,
                                         float* left_sample,
                                         float* right_sample);
    bool try_read_mixed_frame_prefetched_cached(uint64_t frame_index,
                                                uint32_t* cached_sector,
                                                int* cached_slot_index,
                                                float* left_sample,
                                                float* right_sample);
    bool try_read_all_stems_frame_prefetched_cached(
        uint64_t frame_index,
        uint32_t* cached_sector,
        int* cached_slot_index,
        DecodedStemFrameI32* frame_out) const;
    bool try_get_frame_payload_cached(uint64_t frame_index,
                                      uint32_t* cached_sector,
                                      int* cached_slot_index,
                                      const uint8_t** frame_payload_out,
                                      size_t* frame_in_sector_out) const;
    bool try_get_frame_source_cached(uint64_t frame_index,
                                     uint32_t* cached_sector,
                                     int* cached_slot_index,
                                     uint32_t* te_sector_out,
                                     const uint8_t** frame_payload_out,
                                     size_t* frame_in_sector_out,
                                     uint32_t* read_blocks_out) const;
    bool try_get_sector_payload_cached(uint64_t frame_index,
                                       uint32_t* cached_sector,
                                       int* cached_slot_index,
                                       uint32_t* te_sector_out,
                                       const uint8_t** sector_payload_out,
                                       size_t* frame_in_sector_out,
                                       uint32_t* read_blocks_out) const;
    bool map_frame_to_sector(uint64_t frame_index,
                             uint32_t* te_sector_out,
                             size_t* frame_in_sector_out) const;
    bool resolve_block_run(uint64_t frame_index,
                           uint32_t* cached_sector,
                           int* cached_slot_index,
                           const uint8_t** audio_ptr_out,
                           size_t* frame_in_block_out,
                           size_t* frames_available_out) const;
    bool try_get_track_leds_cached(uint64_t frame_index,
                                   uint32_t* cached_sector,
                                   int* cached_slot_index,
                                   std::array<uint8_t, 4>* led_levels_out) const;
    bool try_get_block_side_data_cached(uint64_t frame_index,
                                        uint32_t* cached_sector,
                                        int* cached_slot_index,
                                        uint32_t* sync_word_out,
                                        uint32_t* led_word_out) const;
    bool try_read_mono_frame_prefetched_cached(uint64_t frame_index,
                                               uint32_t* cached_sector,
                                               int* cached_slot_index,
                                               float* mono_sample);
    void render_mono_right24_frames_prefetched_cached(uint64_t start_frame,
                                                      size_t frame_count,
                                                      uint32_t* cached_sector,
                                                      int* cached_slot_index,
                                                      int32_t* interleaved_output);
    void render_weighted_mono_right24_frames_prefetched_cached(
        uint64_t start_frame,
        size_t frame_count,
        uint32_t* cached_sector,
        int* cached_slot_index,
        const std::array<uint16_t, 4>& stem_weights_q12,
        int32_t* interleaved_output);
    void render_weighted_stereo_right24_frames_prefetched_cached(
        uint64_t start_frame,
        size_t frame_count,
        uint32_t* cached_sector,
        int* cached_slot_index,
        const std::array<uint16_t, 4>& stem_weights_q12,
        int32_t* interleaved_output);
    void set_playback_cursor(uint32_t frame_index);
    void set_playback_reverse(bool reverse);
    void set_playback_prefetch_enabled(bool enabled);
    void prime_playback_window(uint32_t frame_index);
    void seed_playback_window_sync(uint32_t frame_index, size_t slot_count);
    void service_playback_prefetch_once();
    void start_streaming(uint32_t frame_index,
                         bool reverse,
                         bool scan_active,
                         uint32_t read_blocks = kEmmcBlocksPerSector);
    void rebase_streaming(uint32_t frame_index,
                          bool reverse,
                          bool scan_active,
                          uint32_t read_blocks = kEmmcBlocksPerSector);
    void update_streaming_hint(uint32_t frame_index,
                               bool reverse,
                               bool scan_active,
                               uint32_t read_blocks);
    bool service_streaming_hint_once();
    void service_streaming_hint();
    bool service_streaming(uint32_t frame_index,
                           bool reverse,
                           bool scan_active,
                           uint32_t read_blocks);
    void stop_streaming();
    bool playback_prefetch_enabled() const;
    bool playback_scan_active() const;
    void set_render_protected_sectors(uint32_t sector_a, uint32_t sector_b);

    // Reads de-interleaved data for a specific stem into the buffer
    void read_stem_data(int stem_idx, float* out_buffer, size_t num_samples);

    // Direct block read access (for USB mass storage, flash dump, etc.)
    bool read_blocks(uint32_t block_addr, uint8_t* buffer, uint32_t num_blocks);

    bool is_emmc_ready() const { return emmc_ready_.load(std::memory_order_acquire); }
    uint32_t emmc_capacity_blocks() const;

    // Album info
    const AlbumInfo& album_info() const { return album_; }

    // Song selection (0-based)
    void set_current_song(uint8_t song_idx);
    uint8_t current_song() const { return current_song_.load(std::memory_order_relaxed); }
    uint8_t song_index_for_frame(uint32_t frame_index) const;
    uint32_t song_start_frame(uint8_t song_idx) const;
    uint32_t total_frames() const;
    PerfStats snapshot_perf_stats() const;
    void reset_perf_stats();

private:
    struct AudioSlot;

    static void init_work_handler(struct k_work* work);
    static void prefetch_work_handler(struct k_work* work);
    // Zephyr disk driver integration
    bool register_disk_driver();

    // Album header parsing
    bool parse_album_header();
    void reset_prefetch_tracking_locked();
    void abort_prefetch_locked();
    void wake_prefetch_worker();

    int find_audio_slot(uint32_t te_sector) const;
    bool load_audio_slot(size_t slot_index,
                         uint32_t te_sector,
                         uint32_t sample_anchor,
                         const uint8_t* source_sector,
                         uint32_t read_blocks = kEmmcBlocksPerSector);
    uint32_t album_total_sectors() const;
    uint32_t album_start_sector() const;
    uint32_t album_total_frames_internal() const;
    uint32_t song_start_frame_internal(uint8_t song_idx) const;
    bool album_sector_index_for_te_sector(uint32_t te_sector,
                                          uint32_t* album_sector_index_out) const;
    uint32_t te_sector_for_album_sector_index(uint32_t album_sector_index) const;
    bool resolve_album_frame(uint64_t frame_index,
                             uint8_t* song_index_out,
                             const SongInfo** song_out,
                             uint32_t* song_frame_index_out,
                             uint32_t* te_sector_out,
                             size_t* frame_in_sector_out) const;
    bool service_audio_prefetch_window(uint32_t frame_index,
                                       uint32_t read_blocks = kEmmcBlocksPerSector);
    bool start_missing_audio_slot_read(uint32_t frame_index,
                                       uint32_t read_blocks = kEmmcBlocksPerSector);
    uint32_t select_next_audio_sector_to_read(uint32_t frame_index,
                                              uint32_t* album_sector_index_out = nullptr) const;
    size_t choose_audio_slot_replacement(uint32_t frame_index,
                                         uint32_t te_sector) const;
    bool is_sector_render_protected(uint32_t te_sector) const;

    bool resolve_slot_frame(uint64_t frame_index,
                            uint32_t* cached_sector,
                            int* cached_slot_index,
                            int* slot_index_out,
                            size_t* block_index_out,
                            size_t* frame_in_block_out,
                            size_t* frame_in_sector_out = nullptr) const;
    bool resolve_slot_for_sector(uint32_t te_sector,
                                 uint32_t* cached_sector,
                                 int* cached_slot_index,
                                 int* slot_index_out) const;
    const uint8_t* block_frame_payload_ptr(const AudioSlot& slot,
                                          size_t block_index,
                                          size_t frame_in_block) const;
    void decode_stem_from_frame_payload_i32(const uint8_t* frame_payload,
                                            int stem_idx,
                                            int32_t* left,
                                            int32_t* right) const;
    void decode_stem_from_frame_payload(const uint8_t* frame_payload,
                                        int stem_idx,
                                        float* left,
                                        float* right) const;

    void run_init_attempt();
    void run_prefetch_work();

    const struct device* emmc_dev_ = DEVICE_DT_GET(DT_NODELABEL(emmc));
    std::atomic<bool> emmc_ready_{false};
    bool init_attempted_ = false;
    struct k_work_delayable init_work_;
    struct k_mutex prefetch_state_lock_;
    struct k_work prefetch_work_;
    struct k_work_q* storage_work_q_ = nullptr;
    std::atomic<bool> prefetch_work_queued_{false};
    std::atomic<bool> prefetch_work_requested_{false};

    struct disk_info disk_info_;

    // Album metadata
    AlbumInfo album_;
    std::atomic<uint8_t> current_song_{0};

    alignas(4) std::array<std::array<uint8_t, kSectorDmaSize>, 2> prefetch_sectors_{};

    struct AudioSlot {
        struct Record {
            alignas(4) std::array<uint8_t, kSectorSize> payload{};
            alignas(4) std::array<uint32_t, kBlocksPerSector> block_sync_words{};
            alignas(4) std::array<uint32_t, kBlocksPerSector> block_led_words{};
        } record{};
        std::atomic<AudioSlotState> state{AudioSlotState::Empty};
        std::atomic<uint32_t> te_sector{UINT32_MAX};
        std::atomic<uint32_t> sample_anchor{0};
        std::atomic<uint32_t> read_blocks{kEmmcBlocksPerSector};
        std::atomic<int32_t> distance_hint{std::numeric_limits<int32_t>::max()};
        std::atomic<bool> read_needed{false};
        std::atomic<bool> ready{false};
    };

    struct InflightRead {
        bool active = false;
        size_t slot_index = 0;
        uint32_t te_sector = UINT32_MAX;
        uint32_t sample_anchor = 0;
        uint8_t buffer_index = 0;
        uint32_t read_blocks = kEmmcBlocksPerSector;
    };

    struct CompletedRead {
        bool active = false;
        bool success = false;
        size_t slot_index = 0;
        uint32_t te_sector = UINT32_MAX;
        uint32_t sample_anchor = 0;
        uint8_t buffer_index = 0;
        uint32_t read_blocks = kEmmcBlocksPerSector;
    };

    struct PlannedRead {
        bool valid = false;
        size_t slot_index = 0;
        uint32_t te_sector = UINT32_MAX;
        uint32_t sample_anchor = 0;
        uint32_t read_blocks = kEmmcBlocksPerSector;
    };

    std::array<AudioSlot, kAudioPrefetchSlots> audio_slots_{};
    std::atomic<uint32_t> playback_cursor_hint_{0};
    std::atomic<bool> playback_reverse_{false};
    std::atomic<bool> playback_scan_active_{false};
    std::atomic<uint32_t> playback_read_blocks_hint_{kEmmcBlocksPerSector};
    std::atomic<bool> playback_prefetch_enabled_{false};
    std::atomic<bool> sync_seed_in_progress_{false};
    std::atomic<uint32_t> render_protected_sector_a_{UINT32_MAX};
    std::atomic<uint32_t> render_protected_sector_b_{UINT32_MAX};
    InflightRead inflight_read_{};
    CompletedRead completed_read_{};
    PlannedRead planned_read_{};
    uint8_t next_prefetch_buffer_index_ = 0;
    mutable struct k_spinlock perf_lock_{};
    mutable PerfStats perf_stats_{};
};

} // namespace storage
