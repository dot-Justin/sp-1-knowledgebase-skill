#pragma once

#include <stem_player/Smf.hpp>
#include <zephyr/kernel.h>
#include <cstdint>

#include "Leds.hpp"
#include "MidiController.hpp"

namespace power { class PowerManager; }

namespace app {

enum AppState : uint8_t {
    STATE_IDLE,
    STATE_RUNNING,
    STATE_SHUTTING_DOWN,
    STATE_DEEP_SLEEP,
    STATE_COUNT,
};

struct AppMachine {
    smf_ctx ctx;
    midi::MidiController* midi = nullptr;
    leds::Leds* leds = nullptr;
    power::PowerManager* power = nullptr;
    int main_watchdog_channel = -1;

    // Function-button hold detection (for shutdown)
    int64_t fn_hold_start_ms = 0;
    bool fn_was_pressed = false;

    // Shutdown button-release debounce
    int64_t fn_released_ms = 0;
};

extern const smf_state app_states[STATE_COUNT];

void run(AppMachine& m);

}  // namespace app
