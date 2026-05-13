#pragma once

#include "DspTables.hpp"
#include "DspUtils.hpp"
#include "EffectNode.hpp"
#include <algorithm>

namespace audio {
namespace effects {

class DistortionNode : public EffectNode {
private:
    struct VariationProfile {
        float tone_cutoff_hz;
    };

    int variation_ = 0;
    float sample_rate_ = detail::kDefaultSampleRate;
    float current_mix_gain_ = 0.0f;
    detail::StereoBiquad tone_filter_;

    static VariationProfile profile_for(int variation) {
        switch (variation) {
        case 0:
            return {8000.0f};
        case 1:
            return {6000.0f};
        case 2:
            return {4300.0f};
        default:
            return {3200.0f};
        }
    }

public:
    void set_variation(int variation) {
        variation_ = std::clamp(variation, 0, 3);
    }

    void set_sample_rate(float sample_rate) {
        sample_rate_ = sample_rate;
    }

    void reset() {
        tone_filter_.reset();
        current_mix_gain_ = 0.0f;
    }

    void apply_in_place(float* out_buffer,
                        size_t num_samples,
                        detail::TransitionPhase phase =
                            detail::TransitionPhase::Steady) {
        const size_t frame_count = num_samples / 2U;
        if (frame_count == 0U) {
            return;
        }

        const VariationProfile profile = profile_for(variation_);
        const detail::BiquadCoefficients tone_coefficients =
            detail::make_lowpass(sample_rate_, profile.tone_cutoff_hz, 0.707f);
        const float target_mix_gain =
            tables::kDistortionTargetGains[static_cast<size_t>(variation_)];
        const float gain_step =
            (target_mix_gain - current_mix_gain_) / static_cast<float>(frame_count);

        for (size_t frame = 0; frame < frame_count; ++frame) {
            const float transition_wet =
                detail::transition_mix(phase, frame, frame_count);
            const float mix_gain = current_mix_gain_ + gain_step * static_cast<float>(frame);
            const float effective_wet = mix_gain * transition_wet;

            const size_t base = frame * 2U;
            const float dry_left = out_buffer[base];
            const float dry_right = out_buffer[base + 1U];

            float left =
                detail::soft_clip(detail::clampf(
                    dry_left * tables::kDistortionInputDrive,
                    tables::kDistortionSoftClipNegative,
                    tables::kDistortionSoftClipPositive));
            float right =
                detail::soft_clip(detail::clampf(
                    dry_right * tables::kDistortionInputDrive,
                    tables::kDistortionSoftClipNegative,
                    tables::kDistortionSoftClipPositive));

            left = dry_left * (1.0f - effective_wet) +
                   left * (effective_wet * tables::kDistortionOutputGain);
            right = dry_right * (1.0f - effective_wet) +
                    right * (effective_wet * tables::kDistortionOutputGain);

            tone_filter_.process_sample(left, right, tone_coefficients, 1.0f);

            out_buffer[base] = left;
            out_buffer[base + 1U] = right;
        }

        current_mix_gain_ = target_mix_gain;
    }

    void process(float* out_buffer, size_t num_samples) override {
        pull_source_or_silence(out_buffer, num_samples);
        apply_in_place(out_buffer, num_samples);
    }
};

}  // namespace effects
}  // namespace audio
