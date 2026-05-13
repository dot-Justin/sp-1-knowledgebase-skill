#pragma once

/// @file Leds.hpp
/// @brief Simple PWM LED driver for the Stem Player.
///
/// 8 LEDs: 4 track (PWM2) + 4 play (PWM3).
/// All values 0–255.  Animations run in set_all_fade() which
/// should be called from the HSM cadence loop.

#include <zephyr/drivers/pwm.h>
#include <array>
#include <cstdint>

namespace leds {

constexpr int kNumTrack = 4;
constexpr int kNumPlay = 4;
constexpr int kNumTotal = kNumTrack + kNumPlay;

struct Leds {
    // PWM specs — set by main from DT macros.
    std::array<pwm_dt_spec, kNumTrack> track{};
    std::array<pwm_dt_spec, kNumPlay> play{};

    /// Set a single track LED (0–3) to brightness 0–255.
    void set_track(int index, uint8_t brightness);

    /// Set a single play LED (0–3) to brightness 0–255.
    void set_play(int index, uint8_t brightness);

    /// Set all 8 LEDs at once.  track[0–3] then play[0–3].
    void set_all(const std::array<uint8_t, kNumTotal>& levels);

    /// All LEDs off.
    void all_off();

    /// Fade all LEDs toward target over `steps` ticks.
    /// Call once per cadence tick.  Returns true when fade is complete.
    bool tick_fade();

    /// Start a fade to `target` over `duration_ms` at the given cadence.
    void start_fade(const std::array<uint8_t, kNumTotal>& target,
                    uint32_t duration_ms, uint32_t cadence_ms);

    /// Start shutdown sequence: all bright → fade to black.
    void start_shutdown_fade();

    /// True when shutdown fade is done (all LEDs at 0).
    bool shutdown_fade_complete() const;

private:
    void apply(int index, uint8_t brightness);

    std::array<uint16_t, kNumTotal> current_{};  // Q8 fixed-point for smooth fading
    std::array<uint16_t, kNumTotal> target_{};
    std::array<int16_t, kNumTotal> step_{};
    int fade_ticks_remaining_ = 0;
    bool shutdown_active_ = false;
};

}  // namespace leds
