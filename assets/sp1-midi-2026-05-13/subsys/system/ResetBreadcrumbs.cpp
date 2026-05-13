#include "ResetBreadcrumbs.hpp"

#include <zephyr/drivers/hwinfo.h>
#include <zephyr/linker/section_tags.h>

#include <nrf.h>

namespace core::reset_breadcrumbs {
namespace {

constexpr uint32_t kRetainedMagic = 0x53504252U; // "SPBR"

struct RetainedBreadcrumb {
    uint32_t magic;
    uint8_t code;
    uint8_t watchdog_channel;
    uint8_t flags;
    uint8_t control_phase;
    uint32_t sequence;
};

constexpr uint8_t kFlagProgressValid = BIT(0);
constexpr uint8_t kFlagWatchdogValid = BIT(1);
constexpr uint8_t kFlagControlPhaseValid = BIT(2);

Snapshot g_snapshot{};
bool g_captured = false;
RetainedBreadcrumb g_retained __noinit;

}  // namespace

void capture_early() {
    if (g_captured) {
        return;
    }

    g_snapshot.resetreas = NRF_POWER->RESETREAS;
    g_snapshot.gpregret = NRF_POWER->GPREGRET;
    g_snapshot.gpregret2 = NRF_POWER->GPREGRET2;
    g_snapshot.reset_cause_valid =
        hwinfo_get_reset_cause(&g_snapshot.reset_cause) == 0;
    if (g_retained.magic == kRetainedMagic) {
        g_snapshot.retained_code = g_retained.code;
        g_snapshot.retained_valid = (g_retained.flags & kFlagProgressValid) != 0U;
        g_snapshot.retained_watchdog_channel = g_retained.watchdog_channel;
        g_snapshot.retained_watchdog_valid = (g_retained.flags & kFlagWatchdogValid) != 0U;
        g_snapshot.retained_control_phase = g_retained.control_phase;
        g_snapshot.retained_control_phase_valid =
            (g_retained.flags & kFlagControlPhaseValid) != 0U;
    }
    g_captured = true;
}

Snapshot snapshot() {
    return g_snapshot;
}

void clear_live_markers() {
    g_retained.magic = kRetainedMagic;
    g_retained.code = 0U;
    g_retained.watchdog_channel = 0U;
    g_retained.flags = 0U;
    g_retained.control_phase = 0U;
    ++g_retained.sequence;
}

void mark(uint8_t code) {
    g_retained.magic = kRetainedMagic;
    g_retained.code = code;
    g_retained.flags |= kFlagProgressValid;
    ++g_retained.sequence;
}

void mark_watchdog_expiry(uint8_t channel) {
    g_retained.magic = kRetainedMagic;
    g_retained.watchdog_channel = channel;
    g_retained.flags |= kFlagWatchdogValid;
    ++g_retained.sequence;
}

void mark_control_phase(uint8_t code) {
    g_retained.magic = kRetainedMagic;
    g_retained.control_phase = code;
    g_retained.flags |= kFlagControlPhaseValid;
    ++g_retained.sequence;
}

}  // namespace core::reset_breadcrumbs
