#pragma once

#include <cstdint>
#include <zephyr/sys/reboot.h>

namespace core::reset_breadcrumbs {

struct Snapshot {
    uint32_t reset_cause = 0U;
    uint32_t resetreas = 0U;
    uint8_t gpregret = 0U;
    uint8_t gpregret2 = 0U;
    uint8_t retained_code = 0U;
    uint8_t retained_watchdog_channel = 0U;
    uint8_t retained_control_phase = 0U;
    bool reset_cause_valid = false;
    bool retained_valid = false;
    bool retained_watchdog_valid = false;
    bool retained_control_phase_valid = false;
};

void capture_early();
Snapshot snapshot();
void clear_live_markers();
void mark(uint8_t code);
void mark_watchdog_expiry(uint8_t channel);
void mark_control_phase(uint8_t code);
void mark_software_reboot(int type);

}  // namespace core::reset_breadcrumbs
