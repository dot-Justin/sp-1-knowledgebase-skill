#pragma once

#include <arm_math.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>

namespace audio {
namespace effects {
namespace detail {

constexpr float kDefaultSampleRate = 48000.0f;
constexpr float kPi = 3.14159265358979323846f;
constexpr float kTwoPi = 6.28318530717958647692f;

enum class TransitionPhase {
    Steady,
    FadeIn,
    FadeOut,
};

inline float clampf(float value, float min_value, float max_value) {
    return std::clamp(value, min_value, max_value);
}

inline float fast_sin(float value) {
    return arm_sin_f32(value);
}

inline float fast_cos(float value) {
    return arm_cos_f32(value);
}

inline float transition_mix(TransitionPhase phase, size_t frame_index, size_t frame_count) {
    if (phase == TransitionPhase::Steady || frame_count <= 1U) {
        return phase == TransitionPhase::FadeOut ? 0.0f : 1.0f;
    }

    const float t = static_cast<float>(frame_index) /
                    static_cast<float>(frame_count - 1U);
    return phase == TransitionPhase::FadeIn ? t : (1.0f - t);
}

inline void wrap_phase(float& phase) {
    while (phase >= kTwoPi) {
        phase -= kTwoPi;
    }
    while (phase < 0.0f) {
        phase += kTwoPi;
    }
}

inline float soft_clip(float sample) {
    const float clamped = clampf(sample, -1.0f, 1.0f);
    return clamped - 0.5f * clamped * clamped * clamped;
}

inline float i16_to_float(int16_t sample) {
    return static_cast<float>(sample) / 32768.0f;
}

inline int16_t float_to_i16(float sample) {
    const float clamped = clampf(sample, -1.0f, 1.0f);
    return static_cast<int16_t>(clamped * 32767.0f);
}

inline float linear_interpolate_i16(const int16_t* buffer, size_t size, float index) {
    if (size == 0U) {
        return 0.0f;
    }

    float wrapped = index;
    const float size_as_float = static_cast<float>(size);
    while (wrapped < 0.0f) {
        wrapped += size_as_float;
    }
    while (wrapped >= size_as_float) {
        wrapped -= size_as_float;
    }

    const size_t index_a = static_cast<size_t>(wrapped);
    const size_t index_b = (index_a + 1U) % size;
    const float fraction = wrapped - static_cast<float>(index_a);
    const float sample_a = i16_to_float(buffer[index_a]);
    const float sample_b = i16_to_float(buffer[index_b]);
    return sample_a + fraction * (sample_b - sample_a);
}

struct BiquadCoefficients {
    float b0 = 1.0f;
    float b1 = 0.0f;
    float b2 = 0.0f;
    float a1 = 0.0f;
    float a2 = 0.0f;
};

struct BiquadState {
    float x1 = 0.0f;
    float x2 = 0.0f;
    float y1 = 0.0f;
    float y2 = 0.0f;
};

class StereoBiquad {
public:
    void reset() {
        left_ = {};
        right_ = {};
    }

    void process_sample(float& left_sample,
                        float& right_sample,
                        const BiquadCoefficients& coefficients,
                        float wet_mix) {
        const float dry_left = left_sample;
        const float dry_right = right_sample;
        const float filtered_left = process_channel(dry_left, left_, coefficients);
        const float filtered_right = process_channel(dry_right, right_, coefficients);

        left_sample = filtered_left * wet_mix + dry_left * (1.0f - wet_mix);
        right_sample = filtered_right * wet_mix + dry_right * (1.0f - wet_mix);
    }

private:
    static float process_channel(float input,
                                 BiquadState& state,
                                 const BiquadCoefficients& coefficients) {
        const float output =
            coefficients.b0 * input +
            coefficients.b1 * state.x1 +
            coefficients.b2 * state.x2 -
            coefficients.a1 * state.y1 -
            coefficients.a2 * state.y2;

        state.x2 = state.x1;
        state.x1 = input;
        state.y2 = state.y1;
        state.y1 = output;
        return output;
    }

    BiquadState left_{};
    BiquadState right_{};
};

inline BiquadCoefficients make_lowpass(float sample_rate, float cutoff_hz, float q) {
    const float clamped_sample_rate = std::max(sample_rate, 1.0f);
    const float clamped_cutoff =
        clampf(cutoff_hz, 20.0f, clamped_sample_rate * 0.45f);
    const float clamped_q = std::max(q, 0.1f);

    const float omega = kTwoPi * clamped_cutoff / clamped_sample_rate;
    const float sin_omega = fast_sin(omega);
    const float cos_omega = fast_cos(omega);
    const float alpha = sin_omega / (2.0f * clamped_q);
    const float a0 = 1.0f + alpha;

    BiquadCoefficients coefficients{};
    coefficients.b0 = ((1.0f - cos_omega) * 0.5f) / a0;
    coefficients.b1 = (1.0f - cos_omega) / a0;
    coefficients.b2 = coefficients.b0;
    coefficients.a1 = (-2.0f * cos_omega) / a0;
    coefficients.a2 = (1.0f - alpha) / a0;
    return coefficients;
}

}  // namespace detail
}  // namespace effects
}  // namespace audio
