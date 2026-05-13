#include "StockRuntimeMixer.hpp"

#include <algorithm>
#include <cmath>
#include <zephyr/kernel.h>

namespace audio {

namespace {

constexpr float kInvStemWeightScaleQ12 =
    1.0f / static_cast<float>(StockRuntimeMixer::kStemWeightScaleQ12);
constexpr float kInvPhaseOne = 1.0f / 16777216.0f;
constexpr float kInvPcm24Scale = 1.0f / 8388608.0f;
constexpr float kWeightUnityEpsilon = 0.0001f;
constexpr float kSpeedSnapThreshold = 0.03f;
constexpr float kPerBlockSpeedSlew = 0.02f;
constexpr int32_t kCompanderMonoShift = 4;
constexpr int32_t kCompanderDcShift = 5;
constexpr int32_t kCompanderPeakReleaseShift = 9;
constexpr int32_t kCompanderGainAttackShift = 3;
constexpr int32_t kCompanderGainReleaseShift = 12;
constexpr int32_t kCompanderGainNumerator = 14453571;
constexpr int32_t kCompanderThresholdDenominator = 896;
constexpr int32_t kCompanderUnityGainQ25 = 58720256;
constexpr int32_t kCompanderInitialGainQ25 = 50331648;

int32_t float_to_q24(float x) {
    return static_cast<int32_t>(x * 16777216.0f);
}

std::array<float, 4> weights_q12_to_f32(
    const std::array<uint16_t, 4>& stem_weights_q12) {
    std::array<float, 4> weights{};
    for (size_t i = 0; i < weights.size(); ++i) {
        weights[i] = static_cast<float>(stem_weights_q12[i]) * kInvStemWeightScaleQ12;
    }
    return weights;
}

float low_speed_transport_gain(float speed) {
    const float speed_abs = std::fabs(speed);
    if (speed_abs >= 0.5f) {
        return 1.0f;
    }
    const float doubled = speed_abs + speed_abs;
    return doubled * doubled * doubled;
}

void update_perf_sample(uint64_t* total_cycles,
                        uint32_t* max_cycles,
                        uint32_t delta_cycles) {
    *total_cycles += delta_cycles;
    *max_cycles = std::max(*max_cycles, delta_cycles);
}

int32_t saturate_signed_bits(int32_t value, uint8_t bits) {
    const int32_t min_value = -(1 << (bits - 1U));
    const int32_t max_value = (1 << (bits - 1U)) - 1;
    return std::clamp(value, min_value, max_value);
}

}  // namespace

uint32_t StockRuntimeMixer::read_blocks_for_speed(float speed) {
    const float speed_abs = std::fabs(speed);
    constexpr float kFullReadMaxSpeed = 1.1f;
    constexpr float kHalfReadMaxSpeed = 2.1f;

    if (speed_abs <= kFullReadMaxSpeed) {
        return storage::kEmmcBlocksPerSector;
    }
    if (speed_abs <= kHalfReadMaxSpeed) {
        return 8U;
    }
    return 4U;
}

StockRuntimeMixer::PerfStats StockRuntimeMixer::snapshot_perf_stats() const {
    const k_spinlock_key_t key = k_spin_lock(&perf_lock_);
    const PerfStats snapshot = perf_stats_;
    k_spin_unlock(&perf_lock_, key);
    return snapshot;
}

void StockRuntimeMixer::reset_perf_stats() {
    const k_spinlock_key_t key = k_spin_lock(&perf_lock_);
    perf_stats_ = PerfStats{};
    k_spin_unlock(&perf_lock_, key);
}

void StockRuntimeMixer::clear_frame_cache() {
    held_frame_valid_ = false;
    held_frame_.fill(0);
    for (auto& entry : frame_cursors_) {
        entry.valid = false;
        entry.frame_index = UINT64_MAX;
        entry.te_sector = UINT32_MAX;
        entry.read_blocks = storage::kEmmcBlocksPerSector;
        entry.cached_sector = UINT32_MAX;
        entry.cached_slot_index = -1;
        entry.sector_base = nullptr;
        entry.frame_in_sector = 0U;
    }
}

size_t StockRuntimeMixer::canonical_frame_in_sector(size_t frame_in_sector,
                                                    uint32_t read_blocks) const {
    if (read_blocks >= storage::kEmmcBlocksPerSector) {
        return frame_in_sector;
    }

    const size_t block_index = frame_in_sector / storage::kFramesPerBlock;
    const size_t frame_offset = frame_in_sector % storage::kFramesPerBlock;

    if (read_blocks >= (storage::kEmmcBlocksPerSector / 2U)) {
        const size_t canonical_block = (block_index < 2U) ? 0U : 2U;
        return canonical_block * storage::kFramesPerBlock + frame_offset;
    }

    return frame_offset;
}

uint64_t StockRuntimeMixer::wrap_transport_frame_index(int64_t frame_index) const {
    if (disk_manager_ == nullptr) {
        return 0U;
    }

    const uint32_t total_frames = disk_manager_->total_frames();
    if (total_frames == 0U) {
        return 0U;
    }

    const int64_t total = total_frames;
    if (frame_index >= 0 && frame_index < total) {
        return static_cast<uint64_t>(frame_index);
    }
    if (frame_index >= total && frame_index < total + total) {
        return static_cast<uint64_t>(frame_index - total);
    }
    if (frame_index < 0 && frame_index >= -total) {
        return static_cast<uint64_t>(frame_index + total);
    }

    int64_t wrapped = frame_index % total;
    if (wrapped < 0) {
        wrapped += total;
    }
    return static_cast<uint64_t>(wrapped);
}

bool StockRuntimeMixer::resolve_frame_cursor(size_t cursor_index,
                                             uint64_t frame_index,
                                             const uint8_t** frame_payload_out) {
    if (frame_payload_out == nullptr || disk_manager_ == nullptr ||
        cursor_index >= frame_cursors_.size()) {
        return false;
    }

    auto& cursor = frame_cursors_[cursor_index];
    uint32_t te_sector = UINT32_MAX;
    uint32_t read_blocks = storage::kEmmcBlocksPerSector;
    size_t frame_in_sector = 0U;
    const uint8_t* sector_payload = nullptr;
    const bool ok = disk_manager_->try_get_sector_payload_cached(
        frame_index,
        &cursor.cached_sector,
        &cursor.cached_slot_index,
        &te_sector,
        &sector_payload,
        &frame_in_sector,
        &read_blocks);
    if (!ok) {
        cursor.valid = false;
        cursor.frame_index = UINT64_MAX;
        cursor.te_sector = UINT32_MAX;
        cursor.read_blocks = storage::kEmmcBlocksPerSector;
        cursor.cached_sector = UINT32_MAX;
        cursor.cached_slot_index = -1;
        cursor.sector_base = nullptr;
        cursor.frame_in_sector = 0U;
        return false;
    }

    const size_t canonical_frame =
        canonical_frame_in_sector(frame_in_sector, read_blocks);
    cursor.valid = true;
    cursor.frame_index = frame_index;
    cursor.te_sector = te_sector;
    cursor.read_blocks = read_blocks;
    cursor.frame_in_sector = canonical_frame;
    cursor.sector_base = sector_payload;
    *frame_payload_out = cursor.sector_base + canonical_frame * storage::kFrameSize;
    return true;
}

void StockRuntimeMixer::gather_transport_stem_block(size_t frame_count,
                                                    TransportRenderMode transport_mode,
                                                    int64_t* sample_index,
                                                    uint32_t* frac_q24,
                                                    int32_t phase_inc_q24,
                                                    int32_t phase_inc_delta_q24,
                                                    float transport_gain,
                                                    bool exact_unity_playback) {
    if (sample_index == nullptr || frac_q24 == nullptr || disk_manager_ == nullptr) {
        return;
    }

    const int64_t phase_mask = (static_cast<int64_t>(1) << 24) - 1;
    const uint32_t total_frames = disk_manager_->total_frames();
    const int64_t total_frames_i64 = total_frames;
    if (total_frames_i64 <= 0) {
        return;
    }

    int64_t local_sample_index = *sample_index;
    uint32_t local_frac_q24 = *frac_q24 & static_cast<uint32_t>(phase_mask);
    int64_t wrapped_sample_index =
        static_cast<int64_t>(wrap_transport_frame_index(local_sample_index));
    size_t rendered = 0U;
    uint32_t exact_frames = 0U;
    uint32_t interp_frames = 0U;
    uint32_t boundary_resolves = 0U;
    uint32_t miss_frames = 0U;
    uint32_t play_miss_frames = 0U;
    uint32_t slow_miss_frames = 0U;
    uint32_t ff_miss_frames = 0U;
    uint32_t rw_miss_frames = 0U;
    uint32_t cursor0_resolve_misses = 0U;
    uint32_t cursor1_resolve_misses = 0U;
    struct DecodedFrameCacheEntry {
        const uint8_t* payload = nullptr;
        storage::DecodedStemFrameI32 decoded{};
    };
    std::array<DecodedFrameCacheEntry, 2> decoded_frame_cache{};

    auto count_miss = [&](uint32_t amount) {
        miss_frames += amount;
        switch (transport_mode) {
        case TransportRenderMode::Play:
            play_miss_frames += amount;
            break;
        case TransportRenderMode::Slow:
            slow_miss_frames += amount;
            break;
        case TransportRenderMode::FastForward:
            ff_miss_frames += amount;
            break;
        case TransportRenderMode::Rewind:
            rw_miss_frames += amount;
            break;
        }
    };

    auto wrap_local_frame = [&](int64_t frame_index) -> int64_t {
        while (frame_index >= total_frames_i64) {
            frame_index -= total_frames_i64;
        }
        while (frame_index < 0) {
            frame_index += total_frames_i64;
        }
        return frame_index;
    };

    auto advance_phase = [&]() -> int64_t {
        const int64_t advanced_phase =
            static_cast<int64_t>(local_frac_q24) + static_cast<int64_t>(phase_inc_q24);
        phase_inc_q24 += phase_inc_delta_q24;
        const int64_t carry = advanced_phase >> 24;
        local_frac_q24 = static_cast<uint32_t>(advanced_phase & phase_mask);
        local_sample_index += carry;
        if (carry != 0) {
            wrapped_sample_index = wrap_local_frame(wrapped_sample_index + carry);
        }
        return carry;
    };

    auto write_silence_frame = [&](size_t frame) {
        const size_t base = frame * 2U;
        for (size_t stem = 0; stem < storage::kStemsPerFrame; ++stem) {
            stem_block_buffers_[stem][base] = 0;
            stem_block_buffers_[stem][base + 1U] = 0;
        }
    };

    auto write_held_frame = [&](size_t frame) {
        const size_t base = frame * 2U;
        for (size_t stem = 0; stem < storage::kStemsPerFrame; ++stem) {
            stem_block_buffers_[stem][base] = held_frame_[stem * 2U];
            stem_block_buffers_[stem][base + 1U] = held_frame_[stem * 2U + 1U];
        }
    };

    auto capture_held_frame = [&](size_t frame) {
        const size_t base = frame * 2U;
        for (size_t stem = 0; stem < storage::kStemsPerFrame; ++stem) {
            held_frame_[stem * 2U] = stem_block_buffers_[stem][base];
            held_frame_[stem * 2U + 1U] = stem_block_buffers_[stem][base + 1U];
        }
        held_frame_valid_ = true;
    };

    auto decode_exact_frame_to_block = [&](const uint8_t* frame_payload, size_t base) {
        for (size_t stem = 0; stem < storage::kStemsPerFrame; ++stem) {
            const uint8_t* data = frame_payload + stem * storage::kStemFrameSize;
            const uint32_t raw_left = (static_cast<uint32_t>(data[1]) << 16) |
                                      (static_cast<uint32_t>(data[0]) << 8) |
                                      static_cast<uint32_t>(data[3]);
            const uint32_t raw_right = (static_cast<uint32_t>(data[2]) << 16) |
                                       (static_cast<uint32_t>(data[5]) << 8) |
                                       static_cast<uint32_t>(data[4]);
            stem_block_buffers_[stem][base] = static_cast<int32_t>(raw_left << 8) >> 8;
            stem_block_buffers_[stem][base + 1U] = static_cast<int32_t>(raw_right << 8) >> 8;
        }
    };

    auto decode_cached_frame = [&](const uint8_t* frame_payload)
        -> const storage::DecodedStemFrameI32& {
        if (decoded_frame_cache[0].payload == frame_payload) {
            return decoded_frame_cache[0].decoded;
        }
        if (decoded_frame_cache[1].payload == frame_payload) {
            std::swap(decoded_frame_cache[0], decoded_frame_cache[1]);
            return decoded_frame_cache[0].decoded;
        }

        decoded_frame_cache[1] = decoded_frame_cache[0];
        decoded_frame_cache[0].payload = frame_payload;
        storage::decode_te_frame_payload_i32(frame_payload, &decoded_frame_cache[0].decoded);
        return decoded_frame_cache[0].decoded;
    };

    while (rendered < frame_count) {
        const uint8_t* sector_payload = nullptr;
        if (!resolve_frame_cursor(0U, static_cast<uint64_t>(wrapped_sample_index), &sector_payload)) {
            ++cursor0_resolve_misses;
            while (rendered < frame_count) {
                if ((transport_mode == TransportRenderMode::FastForward ||
                     transport_mode == TransportRenderMode::Rewind) &&
                    held_frame_valid_) {
                    write_held_frame(rendered);
                } else {
                    write_silence_frame(rendered);
                }
                count_miss(1U);
                ++rendered;
                (void)advance_phase();
            }
            break;
        }

        auto& cursor = frame_cursors_[0];
        if (cursor.sector_base == nullptr) {
            while (rendered < frame_count) {
                if ((transport_mode == TransportRenderMode::FastForward ||
                     transport_mode == TransportRenderMode::Rewind) &&
                    held_frame_valid_) {
                    write_held_frame(rendered);
                } else {
                    write_silence_frame(rendered);
                }
                count_miss(1U);
                ++rendered;
                (void)advance_phase();
            }
            break;
        }

        int64_t frame_in_sector = static_cast<int64_t>(cursor.frame_in_sector);
        const uint8_t* payload_a =
            cursor.sector_base + static_cast<size_t>(frame_in_sector) * storage::kFrameSize;
        if (exact_unity_playback) {
            const size_t frames_this_sector = std::min(
                frame_count - rendered,
                storage::kFramesPerSector - static_cast<size_t>(frame_in_sector));
            for (size_t local = 0; local < frames_this_sector; ++local) {
                const size_t base = (rendered + local) * 2U;
                decode_exact_frame_to_block(payload_a, base);
                capture_held_frame(rendered + local);
                payload_a += storage::kFrameSize;
            }
            rendered += frames_this_sector;
            exact_frames += static_cast<uint32_t>(frames_this_sector);
            local_sample_index += frames_this_sector;
            wrapped_sample_index += frames_this_sector;
            while (wrapped_sample_index >= total_frames_i64) {
                wrapped_sample_index -= total_frames_i64;
            }
            continue;
        }

        while (rendered < frame_count) {
            if (frame_in_sector < 0 ||
                frame_in_sector >= storage::kFramesPerSector) {
                break;
            }

            const size_t base = rendered * 2U;
            const auto& decoded_a = decode_cached_frame(payload_a);
            const uint8_t* payload_b = nullptr;
            if (frame_in_sector + 1 < storage::kFramesPerSector) {
                payload_b = payload_a + storage::kFrameSize;
            } else {
                ++boundary_resolves;
                const int64_t wrapped_next_index = wrap_local_frame(wrapped_sample_index + 1);
                if (!resolve_frame_cursor(1U,
                                          static_cast<uint64_t>(wrapped_next_index),
                                          &payload_b)) {
                    ++cursor1_resolve_misses;
                    if ((transport_mode == TransportRenderMode::FastForward ||
                         transport_mode == TransportRenderMode::Rewind) &&
                        held_frame_valid_) {
                        write_held_frame(rendered);
                    } else {
                        write_silence_frame(rendered);
                    }
                    count_miss(1U);
                    ++rendered;
                    const int64_t carry = advance_phase();
                    frame_in_sector += carry;
                    break;
                }
            }

            const auto& decoded_b = decode_cached_frame(payload_b);

            for (size_t stem = 0; stem < storage::kStemsPerFrame; ++stem) {
                const int64_t left_a = decoded_a.left[stem];
                const int64_t right_a = decoded_a.right[stem];
                const int64_t left_b = decoded_b.left[stem];
                const int64_t right_b = decoded_b.right[stem];
                const int64_t left =
                    left_a + (((left_b - left_a) * local_frac_q24) >> 24);
                const int64_t right =
                    right_a + (((right_b - right_a) * local_frac_q24) >> 24);
                stem_block_buffers_[stem][base] = static_cast<int32_t>(left);
                stem_block_buffers_[stem][base + 1U] = static_cast<int32_t>(right);
            }
            capture_held_frame(rendered);
            ++interp_frames;

            const int64_t carry = advance_phase();
            ++rendered;
            frame_in_sector += carry;
            if (carry != 0 &&
                frame_in_sector >= 0 &&
                frame_in_sector < storage::kFramesPerSector) {
                payload_a = cursor.sector_base +
                            static_cast<size_t>(frame_in_sector) * storage::kFrameSize;
            }
        }
    }

    *sample_index = local_sample_index;
    *frac_q24 = local_frac_q24;
    const k_spinlock_key_t key = k_spin_lock(&perf_lock_);
    perf_stats_.gather_exact_frames += exact_frames;
    perf_stats_.gather_interp_frames += interp_frames;
    perf_stats_.gather_boundary_resolves += boundary_resolves;
    perf_stats_.gather_miss_frames += miss_frames;
    perf_stats_.gather_play_miss_frames += play_miss_frames;
    perf_stats_.gather_slow_miss_frames += slow_miss_frames;
    perf_stats_.gather_ff_miss_frames += ff_miss_frames;
    perf_stats_.gather_rw_miss_frames += rw_miss_frames;
    perf_stats_.gather_cursor0_resolve_misses += cursor0_resolve_misses;
    perf_stats_.gather_cursor1_resolve_misses += cursor1_resolve_misses;
    k_spin_unlock(&perf_lock_, key);
}

void StockRuntimeMixer::mix_stem_block_to_output(
    int32_t* output,
    size_t frame_count,
    float transport_gain,
    const std::array<float, 4>& initial_stem_weights,
    const std::array<float, 4>& weight_step) {
    auto block_stem_weights = initial_stem_weights;
    const bool static_weights =
        std::all_of(weight_step.begin(), weight_step.end(), [](float step) {
            return std::fabs(step) <= kWeightUnityEpsilon;
        });
    const bool all_unity_static =
        static_weights &&
        std::all_of(initial_stem_weights.begin(), initial_stem_weights.end(), [](float weight) {
            return std::fabs(weight - 1.0f) <= kWeightUnityEpsilon;
        });

    if (dual_mono_) {
        if (all_unity_static) {
            for (size_t frame = 0; frame < frame_count; ++frame) {
                const size_t base = frame * 2U;
                int32_t mono = 0;
                for (size_t stem = 0; stem < storage::kStemsPerFrame; ++stem) {
                    mono += stem_block_buffers_[stem][base] +
                            stem_block_buffers_[stem][base + 1U];
                }
                const float mono_f =
                    static_cast<float>(mono) * kInvPcm24Scale * 0.5f * transport_gain * mix_gain_;
                const int32_t pcm = float_to_pcm_right24_fast(mono_f);
                output[base] = pcm;
                output[base + 1U] = pcm;
            }
            return;
        }

        for (size_t frame = 0; frame < frame_count; ++frame) {
            const size_t base = frame * 2U;
            const std::array<float, 4> clamped_weights = {
                std::clamp(block_stem_weights[0], 0.0f, 1.0f),
                std::clamp(block_stem_weights[1], 0.0f, 1.0f),
                std::clamp(block_stem_weights[2], 0.0f, 1.0f),
                std::clamp(block_stem_weights[3], 0.0f, 1.0f),
            };
            float mono = 0.0f;
            for (size_t stem = 0; stem < storage::kStemsPerFrame; ++stem) {
                mono += 0.5f *
                        (static_cast<float>(stem_block_buffers_[stem][base]) +
                         static_cast<float>(stem_block_buffers_[stem][base + 1U])) *
                        clamped_weights[stem];
            }
            mono *= kInvPcm24Scale * transport_gain * mix_gain_;
            const int32_t pcm = float_to_pcm_right24_fast(mono);
            output[base] = pcm;
            output[base + 1U] = pcm;

            for (size_t i = 0; i < block_stem_weights.size(); ++i) {
                block_stem_weights[i] += weight_step[i];
            }
        }
    } else {
        if (all_unity_static) {
            for (size_t frame = 0; frame < frame_count; ++frame) {
                const size_t base = frame * 2U;
                int32_t left = 0;
                int32_t right = 0;
                for (size_t stem = 0; stem < storage::kStemsPerFrame; ++stem) {
                    left += stem_block_buffers_[stem][base];
                    right += stem_block_buffers_[stem][base + 1U];
                }
                output[base] = float_to_pcm_right24_fast(
                    static_cast<float>(left) * kInvPcm24Scale * transport_gain * mix_gain_);
                output[base + 1U] = float_to_pcm_right24_fast(
                    static_cast<float>(right) * kInvPcm24Scale * transport_gain * mix_gain_);
            }
            return;
        }

        for (size_t frame = 0; frame < frame_count; ++frame) {
            const size_t base = frame * 2U;
            const std::array<float, 4> clamped_weights = {
                std::clamp(block_stem_weights[0], 0.0f, 1.0f),
                std::clamp(block_stem_weights[1], 0.0f, 1.0f),
                std::clamp(block_stem_weights[2], 0.0f, 1.0f),
                std::clamp(block_stem_weights[3], 0.0f, 1.0f),
            };
            float left = 0.0f;
            float right = 0.0f;
            for (size_t stem = 0; stem < storage::kStemsPerFrame; ++stem) {
                left += static_cast<float>(stem_block_buffers_[stem][base]) * clamped_weights[stem];
                right += static_cast<float>(stem_block_buffers_[stem][base + 1U]) * clamped_weights[stem];
            }
            output[base] = float_to_pcm_right24_fast(left * kInvPcm24Scale * transport_gain * mix_gain_);
            output[base + 1U] = float_to_pcm_right24_fast(right * kInvPcm24Scale * transport_gain * mix_gain_);

            for (size_t i = 0; i < block_stem_weights.size(); ++i) {
                block_stem_weights[i] += weight_step[i];
            }
        }
    }
}

bool StockRuntimeMixer::any_effect_active() const {
    return effect_active_stem_ >= 0;
}

void StockRuntimeMixer::apply_per_stem_effects(size_t frame_count) {
    // Fast bail — avoid any float conversion work when no effects are engaged.
    if (!effects_any_active_.load(std::memory_order_relaxed)) {
        return;
    }

    const int active_stem = effect_active_stem_;
    if (active_stem < 0 ||
        active_stem >= static_cast<int>(storage::kStemsPerFrame)) {
        return;
    }

    const size_t stem = static_cast<size_t>(active_stem);
    const size_t sample_count = frame_count * 2U;

    auto& buf = stem_block_buffers_[stem];
    auto& scratch = effect_scratch_buffer_;

    // int32 (24-bit right-justified) → float [-1.0, 1.0]
    for (size_t i = 0; i < sample_count; ++i) {
        scratch[i] = static_cast<float>(buf[i]) * kInvPcm24Scale;
    }

    // Run the shared effect rack in-place on the pre-filled float buffer.
    shared_effect_rack_.process_in_place(scratch.data(), sample_count);

    // float → int32 (24-bit right-justified)
    for (size_t i = 0; i < sample_count; ++i) {
        buf[i] = float_to_pcm_right24_fast(scratch[i]);
    }
}

void StockRuntimeMixer::apply_master_compander(int32_t* output, size_t frame_count) {
    if (output == nullptr) {
        return;
    }

    for (size_t frame = 0; frame < frame_count; ++frame) {
        const size_t base = frame * 2U;
        const int32_t mono = (output[base] + output[base + 1U]) << kCompanderMonoShift;

        compander_dc_fast_ += (mono - compander_dc_fast_) >> kCompanderDcShift;
        const int32_t dc_fast_sat = saturate_signed_bits(compander_dc_fast_, 24);
        compander_dc_slow_ +=
            (dc_fast_sat - compander_dc_slow_) >> std::max(1, kCompanderDcShift - 1);
        const int32_t highpassed = mono - compander_dc_fast_ + compander_dc_slow_;
        compander_hp_state_ +=
            (highpassed - compander_hp_state_) >> (kCompanderDcShift + 1);
        const int32_t filtered = highpassed - compander_hp_state_;

        const int32_t rectified = std::abs(filtered >> 3);
        if (rectified >= compander_peak_env_) {
            compander_peak_env_ = rectified;
        } else {
            compander_peak_env_ +=
                (rectified - compander_peak_env_) >> kCompanderPeakReleaseShift;
        }

        const int32_t threshold =
            (kCompanderGainNumerator / kCompanderThresholdDenominator) << 8;
        int32_t target_gain_q25 = kCompanderUnityGainQ25;
        if (compander_peak_env_ > threshold && (compander_peak_env_ >> 16) > 0) {
            target_gain_q25 =
                (kCompanderGainNumerator / (compander_peak_env_ >> 16)) << 8;
        }

        const int32_t gain_delta = target_gain_q25 - compander_gain_q25_;
        if (gain_delta < 0) {
            compander_gain_q25_ += gain_delta >> kCompanderGainAttackShift;
        } else {
            compander_gain_q25_ += gain_delta >> kCompanderGainReleaseShift;
        }

        const int32_t companded = saturate_signed_bits(
            static_cast<int32_t>(
                (static_cast<int64_t>(filtered >> 11) *
                 (compander_gain_q25_ >> 10)) >>
                8),
            22);
        output[base] = companded;
        output[base + 1U] = companded;
    }
}

void StockRuntimeMixer::update_render_protected_sectors() {
    if (disk_manager_ == nullptr) {
        return;
    }

    uint32_t sector_a = UINT32_MAX;
    uint32_t sector_b = UINT32_MAX;
    if (frame_cursors_[0].valid) {
        sector_a = frame_cursors_[0].te_sector;
    }
    if (frame_cursors_[1].valid) {
        sector_b = frame_cursors_[1].te_sector;
    } else {
        sector_b = sector_a;
    }
    if (sector_a == last_protected_sector_a_ &&
        sector_b == last_protected_sector_b_) {
        return;
    }
    disk_manager_->set_render_protected_sectors(sector_a, sector_b);
    last_protected_sector_a_ = sector_a;
    last_protected_sector_b_ = sector_b;
}

RenderBlockResult StockRuntimeMixer::render_block(
    int32_t* output,
    size_t word_count,
    TransportRenderMode transport_mode,
    const std::array<uint16_t, 4>& stem_weights_q12) {
    RenderBlockResult result{};
    if (output == nullptr) {
        return result;
    }

    if (disk_manager_ == nullptr || word_count < 2U || (word_count & 1U) != 0U) {
        std::fill_n(output, word_count, 0);
        return result;
    }

    const size_t frame_count = word_count / 2U;
    if (frame_count * 2U > stem_block_buffers_[0].size()) {
        std::fill_n(output, word_count, 0);
        return result;
    }
    const auto start_position = transport_.position();
    result.wrapped_start_frame = wrap_transport_frame_index(start_position.sample_index);

    const auto target_stem_weights = weights_q12_to_f32(stem_weights_q12);
    auto block_stem_weights = current_stem_weights_;
    std::array<float, 4> weight_step{};
    const float inv_frame_count =
        frame_count > 0U ? 1.0f / static_cast<float>(frame_count) : 0.0f;
    for (size_t i = 0; i < weight_step.size(); ++i) {
        weight_step[i] =
            (target_stem_weights[i] - block_stem_weights[i]) * inv_frame_count;
    }

    int64_t sample_index = start_position.sample_index;
    uint32_t frac_q24 = start_position.frac_q24;
    const float start_speed = transport_.current_speed();
    const float raw_target_speed = transport_.target_speed();
    const float target_delta = raw_target_speed - start_speed;
    const float end_speed =
        std::fabs(target_delta) > kSpeedSnapThreshold
            ? (start_speed + target_delta * kPerBlockSpeedSlew)
            : raw_target_speed;
    const int32_t phase_inc_q24 = float_to_q24(start_speed);
    const int32_t end_inc_q24 = float_to_q24(end_speed);
    const int32_t phase_inc_delta_q24 =
        frame_count > 0U
            ? static_cast<int32_t>((static_cast<int64_t>(end_inc_q24) -
                                    static_cast<int64_t>(phase_inc_q24)) /
                                   static_cast<int64_t>(frame_count))
            : 0;
    const float transport_gain = low_speed_transport_gain(0.5f * (start_speed + end_speed));
    const bool exact_unity_playback =
        transport_mode == TransportRenderMode::Play &&
        phase_inc_q24 == float_to_q24(1.0f) &&
        phase_inc_delta_q24 == 0 &&
        frac_q24 == 0U;

    const uint32_t gather_start = k_cycle_get_32();
    gather_transport_stem_block(frame_count,
                                transport_mode,
                                &sample_index,
                                &frac_q24,
                                phase_inc_q24,
                                phase_inc_delta_q24,
                                transport_gain,
                                exact_unity_playback);
    const uint32_t gather_cycles = k_cycle_get_32() - gather_start;
    update_render_protected_sectors();

    // Per-stem effect processing: int32 → float → DSP → int32.
    // Zero-cost when no effects are active (single atomic load).
    apply_per_stem_effects(frame_count);

    const uint32_t mix_start = k_cycle_get_32();
    mix_stem_block_to_output(output,
                             frame_count,
                             transport_gain,
                             block_stem_weights,
                             weight_step);
    const uint32_t mix_cycles = k_cycle_get_32() - mix_start;
    current_stem_weights_ = target_stem_weights;

    transport_.set_position(sample_index, frac_q24);
    transport_.set_current_speed(end_speed);

    const auto end_position = transport_.position();
    result.wrapped_end_frame = wrap_transport_frame_index(end_position.sample_index);
    result.reverse = end_speed < 0.0f;
    result.read_blocks =
        StockRuntimeMixer::read_blocks_for_speed(std::max(std::fabs(start_speed),
                                                          std::fabs(end_speed)));
    const uint32_t render_cycles = gather_cycles + mix_cycles;
    const k_spinlock_key_t key = k_spin_lock(&perf_lock_);
    ++perf_stats_.render_count;
    update_perf_sample(&perf_stats_.render_total_cycles, &perf_stats_.render_max_cycles, render_cycles);
    update_perf_sample(&perf_stats_.gather_total_cycles, &perf_stats_.gather_max_cycles, gather_cycles);
    update_perf_sample(&perf_stats_.mix_total_cycles, &perf_stats_.mix_max_cycles, mix_cycles);
    k_spin_unlock(&perf_lock_, key);
    return result;
}

}  // namespace audio
