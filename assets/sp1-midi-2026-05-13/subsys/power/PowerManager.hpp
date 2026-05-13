#pragma once

/// @file PowerManager.hpp
/// @brief Battery and charger hardware monitor for Stem Player BSP.
///
/// Polls battery voltage (via SAADC), charger state (BQ24232 GPIO),
/// USB VBUS presence, and die temperature.  Makes no power-state
/// decisions — your app HSM owns those.

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <atomic>

namespace power {

class PowerManager {
public:
    void init();

    /// Call periodically from a work queue (10–50 ms).
    void service_tick();

    // ── Charging / USB ──────────────────────────────────────────
    bool is_charging() const;
    bool is_external_power_present() const;
    void refresh_usb_power_state();

    // ── Battery ─────────────────────────────────────────────────
    float get_battery_level() const;
    int32_t get_battery_mv() const {
        return battery_mv_.load(std::memory_order_relaxed);
    }

    // ── Temperature ─────────────────────────────────────────────
    int32_t die_temperature_milli_c() const {
        return die_temperature_milli_c_.load(std::memory_order_relaxed);
    }

    // ── Shutdown hardware ops (called by HSM) ───────────────────
    /// Release GPIOs, arm function-button wake source for sys_poweroff.
    void prepare_for_system_off();
    /// True if the physical function button is currently pressed.
    bool function_button_pressed() const;

private:
    void run_poll();

    std::atomic<float> battery_level_{1.0f};
    std::atomic<int32_t> battery_mv_{0};
    std::atomic<bool> charging_state_{false};
    std::atomic<bool> external_power_state_{false};
    std::atomic<bool> charger_enabled_state_{false};
    std::atomic<int32_t> die_temperature_milli_c_{0};
    std::atomic<uint32_t> last_poll_ms_{0};

    const struct gpio_dt_spec function_button_ =
        GPIO_DT_SPEC_GET(DT_NODELABEL(button_fnc), gpios);
};

} // namespace power
