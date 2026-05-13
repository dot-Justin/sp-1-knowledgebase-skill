#include "Watchdog.hpp"
#include "ResetBreadcrumbs.hpp"

#include <zephyr/init.h>
#include <zephyr/kernel.h>

#include <nrf.h>
#include <hal/nrf_wdt.h>

namespace core::watchdog {
namespace {

NRF_WDT_Type* const kWdt = NRF_WDT0;
constexpr int kMaxChannels = 6;
constexpr uint32_t kServicePeriodMs = 100;
constexpr uint32_t kHardwareTimeoutMs = 2000;
constexpr uint8_t kExpiredChannelBase = 0xD0U;

struct ChannelState {
    const char* name = nullptr;
    uint32_t timeout_ms = 0;
    uint32_t deadline_ms = 0;
    bool active = false;
};

ChannelState channels[kMaxChannels];
struct k_spinlock channels_lock;
struct k_timer service_timer;
bool service_started = false;
int latched_expired_channel = -1;
int last_expired_channel = -1;

void feed_all_channels() {
    for (size_t i = 0; i < NRF_WDT_CHANNEL_NUMBER; ++i) {
        nrf_wdt_reload_request_set(kWdt, static_cast<nrf_wdt_rr_register_t>(i));
    }
}

void ensure_hardware_wdt_started() {
    if (nrf_wdt_started_check(kWdt)) {
        return;
    }

    constexpr uint32_t kLfclkHz = 32768U;
    const uint32_t reload_ticks = (kLfclkHz * kHardwareTimeoutMs) / 1000U;

    nrf_wdt_behaviour_set(kWdt, NRF_WDT_BEHAVIOUR_RUN_SLEEP_MASK);
    nrf_wdt_reload_value_set(kWdt, reload_ticks);
    nrf_wdt_reload_request_enable(kWdt, NRF_WDT_RR0);
    nrf_wdt_task_trigger(kWdt, NRF_WDT_TASK_START);
    nrf_wdt_reload_request_set(kWdt, NRF_WDT_RR0);
}

static bool service_should_feed_hardware_wdt() {
    const uint32_t now_ms = k_uptime_get_32();
    int expired_channel = -1;
    bool should_feed = true;

    k_spinlock_key_t key = k_spin_lock(&channels_lock);
    if (latched_expired_channel >= 0) {
        should_feed = false;
    } else {
        for (int i = 0; i < kMaxChannels; ++i) {
            const ChannelState& channel = channels[i];
            if (!channel.active) {
                continue;
            }

            if (static_cast<int32_t>(now_ms - channel.deadline_ms) >= 0) {
                should_feed = false;
                expired_channel = i;
                latched_expired_channel = i;
                break;
            }
        }
    }
    k_spin_unlock(&channels_lock, key);

    if (!should_feed && expired_channel >= 0 && expired_channel != last_expired_channel) {
        last_expired_channel = expired_channel;
        core::reset_breadcrumbs::mark_watchdog_expiry(static_cast<uint8_t>(expired_channel));
        NRF_POWER->GPREGRET = static_cast<uint8_t>(kExpiredChannelBase + expired_channel);
    }

    return should_feed;
}

void service_timer_handler(struct k_timer* timer) {
    ARG_UNUSED(timer);

    if (!is_running()) {
        return;
    }

    if (service_should_feed_hardware_wdt()) {
        feed();
    }
}

}  // namespace

bool is_running() {
    return nrf_wdt_started_check(kWdt);
}

void start() {
    ensure_hardware_wdt_started();

    if (service_started) {
        return;
    }

    k_timer_init(&service_timer, service_timer_handler, nullptr);
    k_timer_start(&service_timer, K_MSEC(kServicePeriodMs), K_MSEC(kServicePeriodMs));
    service_started = true;
}

int register_channel(const char* name, uint32_t timeout_ms) {
    if (timeout_ms == 0U) {
        return -EINVAL;
    }

    start();

    const uint32_t now_ms = k_uptime_get_32();
    k_spinlock_key_t key = k_spin_lock(&channels_lock);
    for (int i = 0; i < kMaxChannels; ++i) {
        if (channels[i].active) {
            continue;
        }

        channels[i].name = name;
        channels[i].timeout_ms = timeout_ms;
        channels[i].deadline_ms = now_ms + timeout_ms;
        channels[i].active = true;
        if (latched_expired_channel < 0) {
            last_expired_channel = -1;
        }
        k_spin_unlock(&channels_lock, key);
        return i;
    }
    k_spin_unlock(&channels_lock, key);

    return -ENOMEM;
}

void unregister_channel(int channel_id) {
    if (channel_id < 0 || channel_id >= kMaxChannels) {
        return;
    }

    k_spinlock_key_t key = k_spin_lock(&channels_lock);
    channels[channel_id] = {};
    if (latched_expired_channel < 0 && last_expired_channel == channel_id) {
        last_expired_channel = -1;
    }
    k_spin_unlock(&channels_lock, key);
}

void feed() {
    if (!is_running()) {
        return;
    }

    int fed_channels = 0;
    for (size_t i = 0; i < NRF_WDT_CHANNEL_NUMBER; ++i) {
        const auto rr = static_cast<nrf_wdt_rr_register_t>(i);
        if (nrf_wdt_reload_request_enable_check(kWdt, rr)) {
            nrf_wdt_reload_request_set(kWdt, rr);
            ++fed_channels;
        }
    }

    // Mirror the stock firmware's behavior if the bootloader handed us a
    // running WDT with an unexpected channel configuration.
    if (fed_channels == 0) {
        feed_all_channels();
    }
}

void feed(int channel_id) {
    if (channel_id < 0 || channel_id >= kMaxChannels) {
        return;
    }

    const uint32_t now_ms = k_uptime_get_32();
    k_spinlock_key_t key = k_spin_lock(&channels_lock);
    if (channels[channel_id].active) {
        channels[channel_id].deadline_ms = now_ms + channels[channel_id].timeout_ms;
    }
    k_spin_unlock(&channels_lock, key);
}

static int early_feed_init(void) {
    feed();
    return 0;
}

SYS_INIT(early_feed_init, APPLICATION, 0);

}  // namespace core::watchdog
