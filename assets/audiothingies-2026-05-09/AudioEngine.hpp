#pragma once

#include "PcmPacking.hpp"
#include "StockRuntimeMixer.hpp"
#include "storage/DiskManager.hpp"

#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <zephyr/spinlock.h>

namespace audio {

class AudioEngine {
public:
    enum class TransportMode : uint8_t {
        Play = 0,
        Slow,
        FastForward,
        Rewind,
    };

    static constexpr std::array<float, 4> kFastForwardRates = {
        2.5f, 4.0f, 8.0f, 16.0f,
    };
    static constexpr std::array<float, 4> kRewindRates = {
        1.0f, 2.5f, 4.0f, 8.0f,
    };
    struct PerfStats {
        uint32_t render_count = 0U;
        uint64_t render_total_cycles = 0U;
        uint32_t render_max_cycles = 0U;
    };

    bool init(storage::DiskManager* disk_manager);

    void start_playback(uint32_t start_frame = 0U);
    void stop_playback();
    bool playing() const;
    void pause_playback();
    void resume_playback();
    bool paused() const;
    void rebase_transport(uint32_t frame_index, bool reseed_cache = true);

    void set_transport_mode(TransportMode mode, bool immediate = false);
    TransportMode transport_mode() const;
    bool step_transport_rate(int delta);
    uint8_t transport_rate_index() const;
    float current_speed() const;

    void set_stem_mix_weights_q12(const std::array<uint16_t, 4>& weights_q12);
    void set_all_stem_weights_q12(uint16_t weight_q12);
    void set_solo_mask(uint8_t solo_mask);

    bool step_output_gain(int delta);
    float current_output_gain() const;
    uint8_t current_output_volume_level() const;
    int current_output_gain_percent() const;

    void render_block(int32_t* output, size_t word_count);

    uint32_t current_produced_frame() const;
    void on_buffer_consumed(uint32_t consumed_frame);
    uint32_t current_consumed_frame() const;
    uint32_t current_track_led_word() const;
    uint32_t current_sync_word() const;
    PerfStats snapshot_perf_stats() const;
    StockRuntimeMixer::PerfStats snapshot_mixer_perf_stats() const;
    void reset_perf_stats();

private:
    void apply_pending_mixer_state();
    void update_transport_target(bool immediate);
    float target_speed_for_mode(TransportMode mode) const;
    uint32_t read_blocks_for_transport(TransportMode mode, float speed) const;
    uint32_t read_blocks_for_target_speed(float speed) const;
    uint32_t current_transport_frame() const;
    bool transport_reversing() const;

    std::array<uint16_t, 4> snapshot_stem_weights_q12() const;

    storage::DiskManager* disk_manager_ = nullptr;
    StockRuntimeMixer stock_mixer_{};

    std::atomic<bool> playing_{false};
    std::atomic<bool> paused_{false};
    std::atomic<bool> pending_unity_relock_{false};
    std::atomic<TransportMode> transport_mode_{TransportMode::Play};
    std::atomic<uint8_t> ff_rate_index_{0U};
    std::atomic<uint8_t> rw_rate_index_{0U};
    std::atomic<uint8_t> output_gain_step_{8U};
    std::atomic<uint32_t> produced_frame_{0U};
    std::atomic<uint32_t> consumed_frame_{0U};
    std::atomic<uint32_t> sync_word_{0U};
    std::atomic<uint32_t> led_word_{0U};
    std::atomic<float> observed_speed_{1.0f};
    std::atomic<bool> mixer_reset_transport_pending_{true};
    std::atomic<uint32_t> mixer_reset_transport_frame_{0U};
    std::atomic<bool> mixer_rebase_pending_{false};
    std::atomic<uint32_t> mixer_rebase_frame_{0U};
    std::atomic<bool> target_immediate_pending_{false};
    uint32_t side_data_cached_sector_ = UINT32_MAX;
    int side_data_cached_slot_index_ = -1;
    float last_applied_mix_gain_ = -1.0f;
    uint32_t last_scan_debug_log_ms_ = 0U;
    mutable struct k_spinlock perf_lock_{};
    PerfStats perf_stats_{};
    std::array<std::atomic<uint16_t>, 4> stem_weights_q12_{{
        StockRuntimeMixer::kStemWeightScaleQ12,
        StockRuntimeMixer::kStemWeightScaleQ12,
        StockRuntimeMixer::kStemWeightScaleQ12,
        StockRuntimeMixer::kStemWeightScaleQ12,
    }};
    std::atomic<uint8_t> solo_mask_{0U};
};

}  // namespace audio
