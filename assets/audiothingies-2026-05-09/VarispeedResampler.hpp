#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>

namespace audio {

class VarispeedResampler {
public:
    struct Position {
        int64_t sample_index = 0;
        uint32_t frac_q24 = 0;
    };

    void reset(uint32_t start_sample = 0U) {
        target_speed_ = 1.0f;
        current_speed_ = 1.0f;
        source_sample_index_ = start_sample;
        frac_q24_ = 0U;
    }

    void set_target_speed(float speed, bool immediate = false) {
        target_speed_ = std::clamp(speed, kMinSpeed, kMaxSpeed);
        if (immediate) {
            current_speed_ = target_speed_;
        }
    }

    float current_speed() const {
        return current_speed_;
    }

    float target_speed() const {
        return target_speed_;
    }

    Position position() const {
        return Position{source_sample_index_, frac_q24_};
    }

    void set_position(int64_t sample_index, uint32_t frac_q24 = 0U) {
        source_sample_index_ = sample_index;
        frac_q24_ = frac_q24 & kPhaseMask;
    }

    int64_t current_frame_index() const {
        return source_sample_index_;
    }

    uint32_t wrapped_frame_index(uint32_t total_frames) const {
        if (total_frames == 0U) {
            return 0U;
        }

        const int64_t total = total_frames;
        int64_t wrapped = source_sample_index_ % total;
        if (wrapped < 0) {
            wrapped += total;
        }
        return static_cast<uint32_t>(wrapped);
    }

    void set_current_speed(float speed) {
        current_speed_ = std::clamp(speed, kMinSpeed, kMaxSpeed);
    }

    void set_phase_q24(int64_t phase_q24) {
        source_sample_index_ = phase_q24 >> 24;
        frac_q24_ = static_cast<uint32_t>(phase_q24) & kPhaseMask;
    }

private:
    static constexpr float kMinSpeed = -8.0f;
    static constexpr float kMaxSpeed = 16.0f;
    static constexpr uint32_t kPhaseOne = 1U << 24;
    static constexpr uint32_t kPhaseMask = kPhaseOne - 1U;

    float target_speed_ = 1.0f;
    float current_speed_ = 1.0f;
    int64_t source_sample_index_ = 0;
    uint32_t frac_q24_ = 0U;
};

}  // namespace audio
