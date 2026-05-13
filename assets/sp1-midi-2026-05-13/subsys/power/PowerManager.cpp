#include "PowerManager.hpp"
#include "system/ResetBreadcrumbs.hpp"
#include "charger/charger_bq24232.h"
#include <zephyr/drivers/charger.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/hwinfo.h>
#include <zephyr/logging/log.h>
#include <hal/nrf_power.h>
#include <hal/nrf_temp.h>
#include <algorithm>
#include <cmath>

LOG_MODULE_REGISTER(power_manager, LOG_LEVEL_INF);

namespace {

constexpr int64_t kPowerPollIntervalMs = 50;
constexpr int32_t kBatteryEmptyMv = 3450;
constexpr int32_t kBatteryFullMv = 4180;
constexpr uint32_t kTempEventTimeoutUs = 1000U;

const struct device *const kChargerDev = DEVICE_DT_GET(DT_NODELABEL(bq24232));

bool charger_ready() {
    return device_is_ready(kChargerDev);
}

bool charger_set_enabled(bool enable) {
    return charger_charge_enable(kChargerDev, enable) == 0;
}

bool charger_sample_charge_active() {
    union charger_propval val = {};
    if (charger_get_prop(kChargerDev, CHARGER_PROP_STATUS, &val) < 0) {
        return false;
    }
    return val.status == CHARGER_STATUS_CHARGING;
}

float battery_level_from_mv(int32_t batt_mv) {
    const float normalized = static_cast<float>(batt_mv - kBatteryEmptyMv) /
                             static_cast<float>(kBatteryFullMv - kBatteryEmptyMv);
    return std::clamp(normalized, 0.0f, 1.0f);
}

int32_t sample_die_temperature_milli_c() {
    nrf_temp_event_clear(NRF_TEMP, NRF_TEMP_EVENT_DATARDY);
    nrf_temp_task_trigger(NRF_TEMP, NRF_TEMP_TASK_START);
    for (uint32_t elapsed = 0; elapsed < kTempEventTimeoutUs; ++elapsed) {
        if (nrf_temp_event_check(NRF_TEMP, NRF_TEMP_EVENT_DATARDY)) {
            nrf_temp_event_clear(NRF_TEMP, NRF_TEMP_EVENT_DATARDY);
            const int32_t quarter_c = nrf_temp_result_get(NRF_TEMP);
            nrf_temp_task_trigger(NRF_TEMP, NRF_TEMP_TASK_STOP);
            return quarter_c * 250;
        }
        k_busy_wait(1);
    }
    nrf_temp_task_trigger(NRF_TEMP, NRF_TEMP_TASK_STOP);
    return 25000;
}

}  // namespace

namespace power {

void PowerManager::init() {
    LOG_INF("Initializing Power Monitor (BQ24232)");

    uint32_t reset_cause = 0U;
    if (hwinfo_get_reset_cause(&reset_cause) == 0) {
        if (reset_cause & RESET_LOW_POWER_WAKE) {
            LOG_INF("Wakeup from System OFF by GPIO");
        }
        (void)hwinfo_clear_reset_cause();
    }

    if (gpio_is_ready_dt(&function_button_)) {
        (void)gpio_pin_configure_dt(&function_button_, GPIO_INPUT);
    }

    if (charger_ready()) {
        charger_enabled_state_.store(charger_set_enabled(true), std::memory_order_relaxed);
        LOG_INF("Charger enable asserted");
    } else {
        LOG_WRN("Charger driver not ready");
    }

    refresh_usb_power_state();
    charging_state_.store(charger_sample_charge_active(), std::memory_order_relaxed);
    die_temperature_milli_c_.store(sample_die_temperature_milli_c(), std::memory_order_relaxed);
    last_poll_ms_.store(0U, std::memory_order_relaxed);
}

float PowerManager::get_battery_level() const {
    return battery_level_.load(std::memory_order_relaxed);
}

bool PowerManager::is_charging() const {
    return charging_state_.load(std::memory_order_relaxed);
}

bool PowerManager::is_external_power_present() const {
    return external_power_state_.load(std::memory_order_relaxed);
}

void PowerManager::refresh_usb_power_state() {
    external_power_state_.store(
        nrf_power_usbregstatus_vbusdet_get(NRF_POWER),
        std::memory_order_relaxed);
}

void PowerManager::service_tick() {
    const uint32_t now_ms = k_uptime_get_32();
    const uint32_t last_ms = last_poll_ms_.load(std::memory_order_relaxed);
    if (last_ms != 0U && (now_ms - last_ms) < static_cast<uint32_t>(kPowerPollIntervalMs)) {
        return;
    }
    run_poll();
}

void PowerManager::run_poll() {
    refresh_usb_power_state();

    const bool charging_now = charger_sample_charge_active();
    const int32_t temp_x1000 = sample_die_temperature_milli_c();
    die_temperature_milli_c_.store(temp_x1000, std::memory_order_relaxed);

    // Charger thermal management: disable above 45C, normal below 15C.
    bool charger_enabled = charger_enabled_state_.load(std::memory_order_relaxed);
    if (temp_x1000 > 45000 && charger_enabled) {
        charger_enabled = !charger_set_enabled(false);
    } else if (temp_x1000 <= 14999 && !charger_enabled) {
        charger_enabled = charger_set_enabled(true);
    }
    charger_enabled_state_.store(charger_enabled, std::memory_order_relaxed);

    const bool prev_charging = charging_state_.exchange(charging_now, std::memory_order_relaxed);
    if (prev_charging != charging_now) {
        LOG_INF("Charging: %s", charging_now ? "yes" : "no");
    }

    last_poll_ms_.store(k_uptime_get_32(), std::memory_order_relaxed);
}

bool PowerManager::function_button_pressed() const {
    if (!gpio_is_ready_dt(&function_button_)) {
        return false;
    }
    const int value = gpio_pin_get_dt(&function_button_);
    return value > 0;
}

void PowerManager::prepare_for_system_off() {
    k_sleep(K_MSEC(2));

    // Release all GPIO to minimize leakage current
    NRF_P0->DIR = 0;
    NRF_P1->DIR = 0;

    // Arm function button as wake source (level-active interrupt)
    if (gpio_is_ready_dt(&function_button_)) {
        gpio_pin_configure_dt(&function_button_, GPIO_INPUT);
        gpio_pin_interrupt_configure_dt(&function_button_, GPIO_INT_LEVEL_ACTIVE);
    }
}

} // namespace power
