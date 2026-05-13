#pragma once

#include "DspTables.hpp"
#include "DspUtils.hpp"
#include "EffectNode.hpp"
#include <algorithm>
#include <array>

namespace audio {
namespace effects {

class ChorusFlangerNode : public EffectNode {
private:
    static constexpr size_t kBufferSamples = 1024U;
    static constexpr size_t kBufferMask = kBufferSamples - 1U;

    std::array<int16_t, kBufferSamples> left_buffer_{};
    std::array<int16_t, kBufferSamples> right_buffer_{};
    int variation_ = 0;
    size_t write_index_ = 0U;
    float oscillator_x_ = 0.0f;
    float oscillator_y_ = 1.0f;

    static float clamp_oscillator_component(float value) {
        return detail::clampf(value, -1.0f, 1.0f);
    }

    static float compute_runtime_rate_per_sample(const tables::ChorusPreset& preset) {
        return 2.0f * detail::fast_sin(preset.feedback * tables::kChorusRateScalePerSample);
    }

    static float read_interpolated(const std::array<int16_t, kBufferSamples>& buffer,
                                   size_t write_index,
                                   float delay_samples) {
        const int whole_delay = static_cast<int>(delay_samples);
        const float fractional_delay = delay_samples - static_cast<float>(whole_delay);
        const size_t base_index =
            (write_index - static_cast<size_t>(whole_delay)) & kBufferMask;
        const size_t next_index = (base_index + 1U) & kBufferMask;
        const float sample_a = detail::i16_to_float(buffer[base_index]);
        const float sample_b = detail::i16_to_float(buffer[next_index]);
        return sample_a * (1.0f - fractional_delay) + sample_b * fractional_delay;
    }

public:
    void set_variation(int variation) {
        const int clamped_variation = std::clamp(variation, 0, 3);
        if (clamped_variation != variation_) {
            oscillator_x_ = 0.0f;
            oscillator_y_ = 1.0f;
        }
        variation_ = clamped_variation;
    }

    void set_sample_rate(float sample_rate) {
        (void)sample_rate;
    }

    void reset() {
        left_buffer_.fill(0);
        right_buffer_.fill(0);
        write_index_ = 0U;
        oscillator_x_ = 0.0f;
        oscillator_y_ = 1.0f;
    }

    void apply_in_place(float* out_buffer,
                        size_t num_samples,
                        detail::TransitionPhase phase =
                            detail::TransitionPhase::Steady) {
        const size_t frame_count = num_samples / 2U;
        if (frame_count == 0U) {
            return;
        }

        const auto& preset = tables::kChorusPresets[static_cast<size_t>(variation_)];
        const float rate_per_sample = compute_runtime_rate_per_sample(preset);

        for (size_t frame = 0; frame < frame_count; ++frame) {
            const float wet_mix =
                preset.wet * detail::transition_mix(phase, frame, frame_count);

            oscillator_x_ += rate_per_sample * oscillator_y_;
            oscillator_y_ += -rate_per_sample * oscillator_x_;
            oscillator_x_ = clamp_oscillator_component(oscillator_x_);
            oscillator_y_ = clamp_oscillator_component(oscillator_y_);

            const float left_delay =
                preset.center_delay_samples + preset.modulation_depth_samples * oscillator_x_;
            const float right_delay =
                preset.center_delay_samples + preset.modulation_depth_samples * oscillator_y_;

            const float delayed_left =
                read_interpolated(left_buffer_, write_index_, left_delay);
            const float delayed_right =
                read_interpolated(right_buffer_, write_index_, right_delay);

            const size_t base = frame * 2U;
            const float dry_left = out_buffer[base];
            const float dry_right = out_buffer[base + 1U];

            out_buffer[base] =
                dry_left * (1.0f - wet_mix) + delayed_left * wet_mix;
            out_buffer[base + 1U] =
                dry_right * (1.0f - wet_mix) + delayed_right * wet_mix;

            // The original writes dry int16 samples into the circular buffers;
            // the preset's third float is consumed earlier by `fill_buffer_p5()`
            // to derive the per-sample oscillator step.
            left_buffer_[write_index_] = detail::float_to_i16(dry_left);
            right_buffer_[write_index_] = detail::float_to_i16(dry_right);

            write_index_ = (write_index_ + 1U) & kBufferMask;
        }
    }

    void process(float* out_buffer, size_t num_samples) override {
        pull_source_or_silence(out_buffer, num_samples);
        apply_in_place(out_buffer, num_samples);
    }
};

}  // namespace effects
}  // namespace audio
