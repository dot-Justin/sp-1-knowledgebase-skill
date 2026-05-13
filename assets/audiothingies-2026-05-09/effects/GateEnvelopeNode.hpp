#pragma once

#include "DspTables.hpp"
#include "DspUtils.hpp"
#include "EffectNode.hpp"
#include <algorithm>

namespace audio {
namespace effects {

class GateEnvelopeNode : public EffectNode {
private:
    int variation_ = 0;
    float current_gain_ = 1.0f;

    static float target_gain_for(int variation) {
        const size_t clamped =
            static_cast<size_t>(std::clamp(variation, 0, 3));
        return tables::kGateLevels[clamped];
    }

public:
    void set_variation(int variation) {
        variation_ = std::clamp(variation, 0, 3);
    }

    void reset() {
        current_gain_ = 1.0f;
    }

    void apply_in_place(float* out_buffer,
                        size_t num_samples,
                        detail::TransitionPhase phase =
                            detail::TransitionPhase::Steady) {
        const size_t frame_count = num_samples / 2U;
        if (frame_count == 0U) {
            return;
        }

        const float start_gain =
            phase == detail::TransitionPhase::FadeIn ? 1.0f : current_gain_;
        const float end_gain =
            phase == detail::TransitionPhase::FadeOut ? 1.0f : target_gain_for(variation_);
        const float gain_step =
            (end_gain - start_gain) / static_cast<float>(frame_count);

        float gain = start_gain;
        for (size_t frame = 0; frame < frame_count; ++frame) {
            const size_t base = frame * 2U;
            out_buffer[base] *= gain;
            out_buffer[base + 1U] *= gain;
            gain += gain_step;
        }

        current_gain_ = end_gain;
    }

    void process(float* out_buffer, size_t num_samples) override {
        pull_source_or_silence(out_buffer, num_samples);
        apply_in_place(out_buffer, num_samples);
    }
};

}  // namespace effects
}  // namespace audio
