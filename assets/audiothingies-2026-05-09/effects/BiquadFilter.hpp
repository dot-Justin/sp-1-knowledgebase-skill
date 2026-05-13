#pragma once

#include "DspTables.hpp"
#include "DspUtils.hpp"
#include "EffectNode.hpp"
#include <algorithm>

namespace audio {
namespace effects {

class BiquadFilter : public EffectNode {
private:
    int variation_ = 0;
    float sample_rate_ = detail::kDefaultSampleRate;
    float lfo_phase_ = 0.0f;
    detail::StereoBiquad biquad_;

    static detail::BiquadCoefficients coefficients_for_index(int filter_index) {
        const int clamped_index = std::clamp(
            filter_index, 0, static_cast<int>(tables::kButterworthLowpassTable.size()) - 1);
        const auto& table_entry =
            tables::kButterworthLowpassTable[static_cast<size_t>(clamped_index)];

        return {
            table_entry.b0,
            table_entry.b1,
            table_entry.b2,
            table_entry.a1,
            table_entry.a2,
        };
    }

    int current_filter_index() const {
        switch (variation_) {
        case 0:
            return tables::kBiquadFilterIndexA;
        case 1:
            return tables::kBiquadFilterIndexB;
        case 2:
        case 3:
            return static_cast<int>(
                tables::kBiquadSweepOffset +
                (detail::fast_sin(lfo_phase_ + tables::kBiquadPhaseOffset) + 1.0f) *
                    tables::kBiquadSweepScale);
        default:
            return tables::kBiquadFilterIndexA;
        }
    }

    float lfo_rate_hz() const {
        return variation_ >= 3 ? 0.45f : 0.18f;
    }

public:
    void set_cutoff(float freq) {
        variation_ = freq < 2500.0f ? 1 : 0;
    }

    void set_variation(int variation) {
        variation_ = std::clamp(variation, 0, 3);
    }

    void set_sample_rate(float sample_rate) {
        sample_rate_ = sample_rate;
    }

    void reset() {
        biquad_.reset();
        lfo_phase_ = 0.0f;
    }

    void apply_in_place(float* out_buffer,
                        size_t num_samples,
                        detail::TransitionPhase phase =
                            detail::TransitionPhase::Steady) {
        const size_t frame_count = num_samples / 2U;
        if (frame_count == 0U) {
            return;
        }

        const float phase_step =
            tables::kBiquadLfoScale * lfo_rate_hz() / std::max(sample_rate_, 1.0f);

        for (size_t frame = 0; frame < frame_count; ++frame) {
            const detail::BiquadCoefficients coefficients =
                coefficients_for_index(current_filter_index());
            const float wet_mix = detail::transition_mix(phase, frame, frame_count);

            float left = out_buffer[frame * 2U];
            float right = out_buffer[frame * 2U + 1U];
            biquad_.process_sample(left, right, coefficients, wet_mix);
            out_buffer[frame * 2U] = left;
            out_buffer[frame * 2U + 1U] = right;

            if (variation_ >= 2) {
                lfo_phase_ += phase_step;
                detail::wrap_phase(lfo_phase_);
            }
        }
    }

    void process(float* out_buffer, size_t num_samples) override {
        pull_source_or_silence(out_buffer, num_samples);
        apply_in_place(out_buffer, num_samples);
    }
};

} // namespace effects
} // namespace audio
