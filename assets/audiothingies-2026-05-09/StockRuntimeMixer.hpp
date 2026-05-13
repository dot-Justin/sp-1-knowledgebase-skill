#pragma once

#include "PcmPacking.hpp"
#include "VarispeedResampler.hpp"
#include "effects/StemEffectRack.hpp"
#include "storage/DiskManager.hpp"

#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <zephyr/spinlock.h>

namespace audio {

enum class TransportRenderMode : uint8_t {
    Play = 0,
    Slow,
    FastForward,
    Rewind,
};

struct RenderBlockResult {
    uint32_t wrapped_start_frame = 0U;
    uint32_t wrapped_end_frame = 0U;
    bool reverse = false;
    uint32_t read_blocks = storage::kEmmcBlocksPerSector;
};

class StockRuntimeMixer {
public:
    static constexpr uint16_t kStemWeightScaleQ12 = 4096U;
    static uint32_t read_blocks_for_speed(float speed);
    struct PerfStats {
        uint32_t render_count = 0U;
        uint64_t render_total_cycles = 0U;
        uint32_t render_max_cycles = 0U;
        uint64_t gather_total_cycles = 0U;
        uint32_t gather_max_cycles = 0U;
        uint64_t mix_total_cycles = 0U;
        uint32_t mix_max_cycles = 0U;
        uint32_t gather_exact_frames = 0U;
        uint32_t gather_interp_frames = 0U;
        uint32_t gather_boundary_resolves = 0U;
        uint32_t gather_miss_frames = 0U;
        uint32_t gather_play_miss_frames = 0U;
        uint32_t gather_slow_miss_frames = 0U;
        uint32_t gather_ff_miss_frames = 0U;
        uint32_t gather_rw_miss_frames = 0U;
        uint32_t gather_cursor0_resolve_misses = 0U;
        uint32_t gather_cursor1_resolve_misses = 0U;
    };

public:

    void set_disk_manager(storage::DiskManager* disk_manager) {
        disk_manager_ = disk_manager;
    }

    void set_dual_mono(bool enabled) {
        dual_mono_ = enabled;
    }

    void set_mix_gain(float gain) {
        mix_gain_ = gain;
    }

    /// Set the per-stem effect state.  Thread-safe (spinlock inside StemEffectRack).
    /// Only one stem may have an active effect at a time (hardware constraint).
    /// Activating an effect on a new stem deactivates the previous one and resets
    /// the shared DSP state (delay lines, biquad history, etc.).
    void set_stem_effect(int stem_index, const ui::EffectState& state) {
        if (stem_index < 0 || stem_index >= static_cast<int>(stem_effect_states_.size())) {
            return;
        }
        const auto idx = static_cast<size_t>(stem_index);
        // If activating on a different stem, deactivate the old one and reset DSP
        if (state.active && effect_active_stem_ >= 0 &&
            effect_active_stem_ != stem_index) {
            stem_effect_states_[static_cast<size_t>(effect_active_stem_)].active = false;
        }
        stem_effect_states_[idx] = state;
        if (state.active) {
            if (effect_active_stem_ != stem_index) {
                // New stem — reset shared rack DSP state
                shared_effect_rack_.set_effect_state(state);
                effect_active_stem_ = stem_index;
            } else {
                shared_effect_rack_.set_effect_state(state);
            }
        } else if (effect_active_stem_ == stem_index) {
            shared_effect_rack_.set_effect_state(state);
            effect_active_stem_ = -1;
        }
        effects_any_active_.store(effect_active_stem_ >= 0, std::memory_order_relaxed);
    }

    /// Query per-stem effect state.
    const ui::EffectState& stem_effect_state(int stem_index) const {
        return stem_effect_states_[static_cast<size_t>(
            std::clamp(stem_index, 0, 3))];
    }

    void reset() {
        compander_dc_fast_ = 0;
        compander_dc_slow_ = 0;
        compander_hp_state_ = 0;
        compander_peak_env_ = 0;
        compander_gain_q25_ = 0x03000000;
        clear_frame_cache();
    }

    void reset_transport(uint32_t start_frame = 0U) {
        transport_.reset(start_frame);
        compander_dc_fast_ = 0;
        compander_dc_slow_ = 0;
        compander_hp_state_ = 0;
        compander_peak_env_ = 0;
        compander_gain_q25_ = 0x03000000;
        clear_frame_cache();
    }

    void rebase_transport(uint32_t frame_index) {
        transport_.set_position(frame_index, 0U);
        compander_dc_fast_ = 0;
        compander_dc_slow_ = 0;
        compander_hp_state_ = 0;
        compander_peak_env_ = 0;
        compander_gain_q25_ = 0x03000000;
        clear_frame_cache();
    }

    void set_target_speed(float speed, bool immediate = false) {
        transport_.set_target_speed(speed, immediate);
    }

    float current_speed() const {
        return transport_.current_speed();
    }

    PerfStats snapshot_perf_stats() const;
    void reset_perf_stats();

    uint32_t current_frame_index_wrapped(uint32_t total_frames) const {
        return transport_.wrapped_frame_index(total_frames);
    }

    RenderBlockResult render_block(int32_t* output,
                                   size_t word_count,
                                   TransportRenderMode transport_mode,
                                   const std::array<uint16_t, 4>& stem_weights_q12);

private:
    void apply_per_stem_effects(size_t frame_count);
    bool any_effect_active() const;
    void apply_master_compander(int32_t* output, size_t frame_count);
    void gather_transport_stem_block(size_t frame_count,
                                     TransportRenderMode transport_mode,
                                     int64_t* sample_index,
                                     uint32_t* frac_q24,
                                     int32_t phase_inc_q24,
                                     int32_t phase_inc_delta_q24,
                                     float transport_gain,
                                     bool exact_unity_playback);
    void mix_stem_block_to_output(int32_t* output,
                                  size_t frame_count,
                                  float transport_gain,
                                  const std::array<float, 4>& initial_stem_weights,
                                  const std::array<float, 4>& weight_step);
    void update_render_protected_sectors();
    size_t canonical_frame_in_sector(size_t frame_in_sector,
                                     uint32_t read_blocks) const;
    uint64_t wrap_transport_frame_index(int64_t frame_index) const;
    bool resolve_frame_cursor(size_t cursor_index,
                              uint64_t frame_index,
                              const uint8_t** frame_payload_out);
    void clear_frame_cache();

    struct FramePayloadCursor {
        bool valid = false;
        uint64_t frame_index = UINT64_MAX;
        uint32_t te_sector = UINT32_MAX;
        uint32_t read_blocks = storage::kEmmcBlocksPerSector;
        uint32_t cached_sector = UINT32_MAX;
        int cached_slot_index = -1;
        const uint8_t* sector_base = nullptr;
        size_t frame_in_sector = 0U;
    };

    static constexpr size_t kMaxBlockFrames = 128U;

    storage::DiskManager* disk_manager_ = nullptr;
    bool dual_mono_ = true;
    float mix_gain_ = 0.18f;
    std::array<FramePayloadCursor, 2> frame_cursors_{};
    alignas(16) std::array<std::array<int32_t, kMaxBlockFrames * 2U>, storage::kStemsPerFrame>
        stem_block_buffers_{};
    std::array<float, 4> current_stem_weights_{
        1.0f, 1.0f, 1.0f, 1.0f};
    bool held_frame_valid_ = false;
    std::array<int32_t, storage::kStemsPerFrame * 2U> held_frame_{};
    int32_t compander_dc_fast_ = 0;
    int32_t compander_dc_slow_ = 0;
    int32_t compander_hp_state_ = 0;
    int32_t compander_peak_env_ = 0;
    int32_t compander_gain_q25_ = 0x03000000;
    uint32_t last_protected_sector_a_ = UINT32_MAX;
    uint32_t last_protected_sector_b_ = UINT32_MAX;
    VarispeedResampler transport_{};
    mutable struct k_spinlock perf_lock_{};
    PerfStats perf_stats_{};

    // Per-stem effect processing — single shared rack (one active stem at a time)
    mutable effects::StemEffectRack shared_effect_rack_{};
    std::array<ui::EffectState, storage::kStemsPerFrame> stem_effect_states_{};
    int effect_active_stem_ = -1;
    std::atomic<bool> effects_any_active_{false};
    alignas(16) std::array<float, kMaxBlockFrames * 2U> effect_scratch_buffer_{};
};

}  // namespace audio
