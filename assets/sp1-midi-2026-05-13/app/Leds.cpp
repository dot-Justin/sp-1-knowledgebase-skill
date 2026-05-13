#include "Leds.hpp"
#include <algorithm>
#include <cmath>

namespace leds {

void Leds::apply(int index, uint8_t brightness) {
    const pwm_dt_spec* spec = nullptr;
    if (index < kNumTrack) {
        spec = &track[index];
    } else if (index < kNumTotal) {
        spec = &play[index - kNumTrack];
    } else {
        return;
    }
    const uint32_t pulse = (spec->period * brightness) / 255U;
    pwm_set_pulse_dt(spec, pulse);
}

void Leds::set_track(int index, uint8_t brightness) {
    if (index < 0 || index >= kNumTrack) {
        return;
    }
    current_[index] = static_cast<uint16_t>(brightness) << 8;
    apply(index, brightness);
}

void Leds::set_play(int index, uint8_t brightness) {
    if (index < 0 || index >= kNumPlay) {
        return;
    }
    const int i = kNumTrack + index;
    current_[i] = static_cast<uint16_t>(brightness) << 8;
    apply(i, brightness);
}

void Leds::set_all(const std::array<uint8_t, kNumTotal>& levels) {
    for (int i = 0; i < kNumTotal; ++i) {
        current_[i] = static_cast<uint16_t>(levels[i]) << 8;
        apply(i, levels[i]);
    }
}

void Leds::all_off() {
    for (int i = 0; i < kNumTotal; ++i) {
        current_[i] = 0;
        apply(i, 0);
    }
    fade_ticks_remaining_ = 0;
}

void Leds::start_fade(const std::array<uint8_t, kNumTotal>& target,
                       uint32_t duration_ms, uint32_t cadence_ms) {
    const int total_ticks = std::max<int>(static_cast<int>(duration_ms / cadence_ms), 1);
    for (int i = 0; i < kNumTotal; ++i) {
        target_[i] = static_cast<uint16_t>(target[i]) << 8;
        step_[i] = static_cast<int16_t>(
            (static_cast<int32_t>(target_[i]) - static_cast<int32_t>(current_[i])) / total_ticks);
    }
    fade_ticks_remaining_ = total_ticks;
}

bool Leds::tick_fade() {
    if (fade_ticks_remaining_ <= 0) {
        return true;
    }

    --fade_ticks_remaining_;

    for (int i = 0; i < kNumTotal; ++i) {
        if (fade_ticks_remaining_ == 0) {
            current_[i] = target_[i];
        } else {
            current_[i] = static_cast<uint16_t>(
                std::clamp<int32_t>(
                    static_cast<int32_t>(current_[i]) + step_[i], 0, 255 << 8));
        }
        apply(i, static_cast<uint8_t>(current_[i] >> 8));
    }

    if (fade_ticks_remaining_ == 0 && shutdown_active_) {
        // Shutdown fade done — ensure truly off
        all_off();
    }

    return fade_ticks_remaining_ == 0;
}

void Leds::start_shutdown_fade() {
    shutdown_active_ = true;
    // Flash all bright first
    std::array<uint8_t, kNumTotal> bright{};
    bright.fill(200);
    set_all(bright);
    // Then fade to black over 600ms
    std::array<uint8_t, kNumTotal> off{};
    off.fill(0);
    start_fade(off, 600, 20);
}

bool Leds::shutdown_fade_complete() const {
    return shutdown_active_ && fade_ticks_remaining_ <= 0;
}

}  // namespace leds
