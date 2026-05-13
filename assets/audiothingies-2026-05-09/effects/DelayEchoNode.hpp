#pragma once

#include "DspTables.hpp"
#include "DspUtils.hpp"
#include "EffectNode.hpp"
#include <algorithm>
#include <array>

namespace audio {
namespace effects {

class DelayEchoNode : public EffectNode {
private:
    static constexpr size_t kDelayBufferSamples = 8192U;
    static constexpr size_t kDelayBufferMask = kDelayBufferSamples - 1U;

    std::array<int16_t, kDelayBufferSamples> delay_buffer_{};
    int variation_ = 0;
    bool wet_enabled_ = false;
    size_t write_index_ = 0U;
    float fill_progress_ = 0.0f;

public:
    void set_variation(int variation) {
        variation_ = std::clamp(variation, 0, 3);
    }

    void set_wet_enabled(bool wet_enabled) {
        wet_enabled_ = wet_enabled;
        if (!wet_enabled_) {
            fill_progress_ = 0.0f;
        }
    }

    void reset() {
        delay_buffer_.fill(0);
        write_index_ = 0U;
        fill_progress_ = 0.0f;
    }

    void apply_in_place(float* out_buffer,
                        size_t num_samples,
                        int variation,
                        bool wet_enabled,
                        detail::TransitionPhase phase =
                            detail::TransitionPhase::Steady) {
        set_variation(variation);
        set_wet_enabled(wet_enabled);

        const size_t frame_count = num_samples / 2U;
        if (frame_count == 0U) {
            return;
        }

        const auto& wet_profile =
            tables::kDelayWetProfiles[static_cast<size_t>(variation_)];

        for (size_t frame = 0; frame < frame_count; ++frame) {
            const size_t left_index = write_index_;
            const size_t right_index = (write_index_ + 1U) & kDelayBufferMask;
            const float delayed_left = detail::i16_to_float(delay_buffer_[left_index]);
            const float delayed_right = detail::i16_to_float(delay_buffer_[right_index]);

            const size_t base = frame * 2U;
            const float dry_left = out_buffer[base];
            const float dry_right = out_buffer[base + 1U];

            float mixed_left = dry_left;
            float mixed_right = dry_right;
            float wet_phase = 0.0f;
            if (wet_enabled_) {
                mixed_left =
                    dry_left * wet_profile.input_gain + delayed_left * wet_profile.feedback;
                mixed_right =
                    dry_right * wet_profile.input_gain + delayed_right * wet_profile.feedback;
                wet_phase = detail::transition_mix(phase, frame, frame_count);
                if (wet_profile.build_fill_before_full_wet) {
                    const float fill_factor = std::clamp(
                        fill_progress_ / static_cast<float>(kDelayBufferSamples),
                        0.0f,
                        1.0f);
                    wet_phase *= fill_factor;
                    fill_progress_ += 2.0f;
                }
                out_buffer[base] = detail::clampf(
                    dry_left * (1.0f - wet_phase) +
                        mixed_left * (wet_phase * wet_profile.output_mix_scale),
                    -1.0f,
                    1.0f);
                out_buffer[base + 1U] = detail::clampf(
                    dry_right * (1.0f - wet_phase) +
                        mixed_right * (wet_phase * wet_profile.output_mix_scale),
                    -1.0f,
                    1.0f);
            } else {
                mixed_left =
                    dry_left * tables::kDelayDryInputGain +
                    delayed_left * tables::kDelayDryFeedback;
                mixed_right =
                    dry_right * tables::kDelayDryInputGain +
                    delayed_right * tables::kDelayDryFeedback;
            }

            const float write_left = mixed_left;
            const float write_right = mixed_right;
            delay_buffer_[left_index] = detail::float_to_i16(write_left);
            delay_buffer_[right_index] = detail::float_to_i16(write_right);

            write_index_ = (write_index_ + 2U) & kDelayBufferMask;
        }
    }

    void process(float* out_buffer, size_t num_samples) override {
        pull_source_or_silence(out_buffer, num_samples);
        apply_in_place(out_buffer, num_samples, variation_, wet_enabled_);
    }
};

}  // namespace effects
}  // namespace audio
