# `sp1-midi` BSP Walkthrough

**Synthesized through:** Lines #846 (2026-05-06), Discord through 2026-05-11. Code-of-record: `ericlewis/sp1-midi` GitHub repository, 2026-05-09.

`ericlewis/sp1-midi` is the **public, buildable Zephyr Board Support Package** for the SP-1. It is **not** a stem player. It turns the SP-1 into a **USB MIDI 2.0 controller**: faders, track buttons, transport rockers, volume, and function button all emit MIDI events on a USB MIDI 2.0 (UMP) endpoint, with a CDC ACM console alongside for debugging.

This file walks through what the BSP does, how the app behaves, and how you'd extend it to do something else (e.g., play audio). For the audio engine that you'd *add* to this BSP to play stems, see `12-audio-engine-internals.md`.

## What the app does (out of the box)

When you flash `sp1-midi` to an SP-1, the device behaves as follows:

| User action | MIDI event sent |
| --- | --- |
| Move Fader 1 | CC 1 on channel 1 |
| Move Fader 2 | CC 2 on channel 1 |
| Move Fader 3 | CC 3 on channel 1 |
| Move Fader 4 | CC 4 on channel 1 |
| Press Track 1 | Note 60 (C4) on channel 1, velocity 127 |
| Press Track 2 | Note 61 (C#4) |
| Press Track 3 | Note 62 (D4) |
| Press Track 4 | Note 63 (D#4) |
| Press Play | MMC Play (0x02) or Stop (0x01), toggling |
| Press Volume Up | CC 7 += 10 (with clamp at 127) |
| Press Volume Down | CC 7 -= 10 (with clamp at 0) |
| Press Fast-Forward rocker | Octave shift +1 (Track button notes move up an octave) |
| Press Rewind rocker | Octave shift -1 |
| Press Function | MMC Record On (0x06) or Record Off (0x07), toggling |
| Long-press Function (3 seconds) | `INPUT_KEY_POWER` fires (handler in app would call `sys_poweroff()`) |

[code: `sp1-midi/app/main.cpp` lines 129–209 — `on_input_event()`]

The faders use a 12-bit ADC range (0–4080 mV); the BSP polls at 20 ms intervals and emits CC events when the value changes by more than `kFaderDeadband = 8` (~0.2%).

## USB device descriptors

| Field | Value |
| --- | --- |
| Vendor ID | 0x1209 (PID.codes test VID) |
| Product ID | 0x0001 |
| Manufacturer | "Teenage Engineering" |
| Product | "SP-1 MIDI Controller" |
| Configuration | Composite: MIDI 2.0 + CDC ACM |
| Bus speed | Full Speed (12 Mbps) |
| Device class | Misc / IAD / 0x02 / 0x01 |

[code: `sp1-midi/app/main.cpp` lines 27–76]

The "Teenage Engineering" manufacturer string is presumably for compatibility with hosts that expect TE devices — **change this** if you're publishing custom firmware under your own name, and use a different PID (the 0x1209 0x0001 combo is not registered to anyone in particular and may collide with other PID.codes test devices).

## Architecture

```
app/main.cpp
  ├── init_usb()                    — composite USB device init
  ├── USB MIDI callbacks
  │     ├── on_midi_rx              — incoming MIDI from host
  │     └── on_midi_ready           — connection up/down callback
  ├── Input subsystem callback
  │     └── on_input_event          — buttons + rockers → MIDI
  └── main()
        ├── Watchdog start + main channel register
        ├── USB init
        ├── LED PWM setup (8 LEDs)
        ├── PowerManager init
        ├── Fader ADC channel setup
        ├── MidiController setup
        └── app::run(machine)        — enter HSM
app/AppStateMachine.cpp
  ├── AppMachine struct
  └── State handlers: IDLE, RUNNING, SHUTTING_DOWN, DEEP_SLEEP
app/MidiController.cpp
  ├── poll_faders()                  — periodic ADC read + CC emission
  ├── send_cc(), send_note(), send_mmc()
  └── on_rx()                        — incoming MIDI handler
```

## Globals

```cpp
static midi::MidiController g_midi;
static leds::Leds           g_leds;
static power::PowerManager  g_power;
```

[code: `sp1-midi/app/main.cpp` lines 92–94]

The app uses **static globals** rather than dependency injection. This is unusual for modern C++ but practical in embedded code where there's no need for multiple instances. If you fork this BSP, you can either keep this style or refactor to a more testable architecture.

## The state machine

`AppMachine` is the Zephyr SMF (State Machine Framework) instance with C++ facade [code: `sp1-midi/include/stem_player/Smf.hpp`]:

```cpp
enum AppState {
    STATE_IDLE,
    STATE_RUNNING,
    STATE_SHUTTING_DOWN,
    STATE_DEEP_SLEEP,
};
```

State responsibilities (inferred from `AppStateMachine.cpp`):

- **IDLE:** Initial state on boot. Wait for USB MIDI to come up.
- **RUNNING:** Normal operation. Poll faders, route buttons → MIDI, animate LEDs.
- **SHUTTING_DOWN:** Cleanup. Mute outputs, prepare for deep sleep.
- **DEEP_SLEEP:** `sys_poweroff()` called. Device is off until USB-attach or button wake.

The HSM is straightforward — a few hundred lines total. The skeleton is intentionally minimal; custom firmware authors add states (e.g., `AUDIO_PLAYING`, `EFFECT_ACTIVE`) and extend the state table.

The `AppMachine` struct carries pointers to subsystem objects (`midi`, `leds`, `power`) plus debouncing state for the function button and the watchdog channel ID.

## Subsystems

### PowerManager (`subsys/power/PowerManager.{hpp,cpp}`)

Monitors battery (via AIN4), charger status (via BQ24232 GPIOs), and USB attach. Provides query methods for battery percentage, charging state, and USB connection. Used by the HSM to decide when to enter `DEEP_SLEEP`.

### Watchdog (`subsys/system/Watchdog.{hpp,cpp}`)

Wraps the nRF52840's WDT peripheral. Supports multiple "channels" — each subsystem registers a channel with a timeout, and must periodically call `feed()` to prevent reset. The main app registers a channel with **5000 ms timeout** and feeds it during each major init step.

### ResetBreadcrumbs (`subsys/system/ResetBreadcrumbs.{hpp,cpp}`)

Records the reason for the last reset in a small persistent storage area. After a boot, you can call `get_last_reset_reason()` and see whether the previous boot was a normal shutdown, a watchdog reset, a panic, etc. Useful for debugging custom-firmware crashes.

### EmmcDriver + StemEmmcDevice (`subsys/storage/emmc/`)

The Zephyr disk driver wrapper around the eMMC bit-banging. **Available but not used by the basic MIDI controller app.** Custom firmware that plays audio would consume this. See `08-emmc-storage.md`.

### Audio codec drivers (`drivers/audio/cs42l42_codec.c`, `tas2505_codec.c`)

Zephyr `audio_codec` drivers. Bring up the codecs on I²C0 and configure the I²S path. **Configured at boot but only `cs42l42_codec_is_headphone_connected()` is exposed at the app level** — the BSP doesn't actually stream audio. Custom firmware that plays audio would use the `audio_codec_*` API to configure formats and the I²S driver to feed samples.

### Charger driver (`drivers/charger/charger_bq24232.c`)

Zephyr `charger` driver. Tracks USB attach via `nPGOOD`, charge state via `nCHG`, controls charging via `CE`, optional fast/slow current selection via `ISET`. Used by `PowerManager`.

## MidiController internals

[code: `sp1-midi/app/MidiController.hpp` + `.cpp`]

```cpp
struct MidiController {
    const struct device*       midi_dev;            // USB MIDI 2.0 device
    const struct pwm_dt_spec*  track_led_pwm;       // track LED PWM specs
    std::array<int16_t, 4>     last_fader_raw;      // Previous fader ADC values
    std::array<uint8_t, 4>     last_fader_cc;       // Previous CC values
    bool                       playing = false;
    bool                       recording = false;
    uint8_t                    volume_level = 100;
    int8_t                     octave_offset = 0;
    std::atomic<bool>          ready{false};
    std::array<adc_dt_spec, 4> fader_adc;
    
    void send_cc(uint8_t cc, uint8_t value);
    void send_note(uint8_t note, bool on, uint8_t velocity = 127);
    void send_mmc(uint8_t command);
    void poll_faders();
    void set_track_led(int index, uint8_t brightness);
    void on_rx(const struct midi_ump& ump);
};
```

`poll_faders()` reads all 4 ADC channels, applies deadband filtering, and emits CC events. `send_cc()`, `send_note()`, `send_mmc()` are the MIDI emission primitives.

`on_rx()` handles incoming MIDI from the host — e.g., a DAW could send CC values back to set fader LEDs or to control program state. Currently the implementation is minimal; extend it for round-trip control.

## MMC (MIDI Machine Control) command bytes

```cpp
constexpr uint8_t kMmcStop          = 0x01;
constexpr uint8_t kMmcPlay          = 0x02;
constexpr uint8_t kMmcFastForward   = 0x04;
constexpr uint8_t kMmcRewind        = 0x05;
constexpr uint8_t kMmcRecordOn      = 0x06;
constexpr uint8_t kMmcRecordOff     = 0x07;
constexpr uint8_t kMmcPause         = 0x09;
```

[code: `sp1-midi/app/MidiController.hpp` lines 28–36]

These are wrapped in a SysEx envelope: `F0 7F 7F 06 <cmd> F7`. The full SysEx is sent via the USB MIDI 2.0 endpoint.

## CC and note assignments

```cpp
constexpr uint8_t kCcFader1   = 1;     // Faders → CC 1, 2, 3, 4
constexpr uint8_t kCcVolume   = 7;     // Vol up/down → CC 7
constexpr uint8_t kNoteTrack1 = 60;    // Track 1 = note 60 (C4)
```

The track button base note (C4) plus the rocker-controlled octave offset (±5 octaves, mapping to range 0–127) gives the user a 9-octave range of access via just the 4 track buttons. This is a generic "trigger pad" mapping — change it for application-specific control.

## Watchdog usage

Main registers a 5000 ms channel and feeds it during init. Subsystems with long-running operations should register their own channels:

```cpp
const int my_channel = core::watchdog::register_channel("my_task", 1000);
// periodically...
core::watchdog::feed(my_channel);
```

If any channel times out, the device resets. This prevents bugs from putting the device in a dead state requiring battery disconnect.

## What this BSP doesn't do

A few important non-features:

- **No audio playback.** The codecs are configured but never fed I²S data. To play audio, integrate `assets/audiothingies-2026-05-09/` and add I²S TX setup.
- **No eMMC read.** The driver is available but the app doesn't call it.
- **No Bluetooth.** UART pins are defined but no stack is brought up.
- **No automatic power-off.** Long-press is detected but the app must explicitly call `sys_poweroff()` from its handler (the BSP doesn't do this in the example HSM).
- **No DSP effects.** No effects rack, no per-stem processing.

This is intentional — the BSP is a starting point, not a complete custom firmware. ericlewis describes it [Discord #firmware, 2026-05-09 00:11]:

> repo above turns your sp1 into a midi controller
> unless you have your own fw to reflash ur f'd tho
> so dont just flash for fun

Flash this BSP and your SP-1 is no longer a stem player — it's a USB MIDI controller until you reflash. Don't do this on your only device.

## How to extend the BSP

### Add audio playback

1. Drop `assets/audiothingies-2026-05-09/` and `assets/storagethingies-2026-05-09/` source into `sp1-midi/subsys/` (or a new `sp1-midi/audio/` tree)
2. Add to `APP_SOURCES` in `CMakeLists.txt`
3. Instantiate `audio::AudioEngine` and `storage::DiskManager` as globals or members of `AppMachine`
4. Initialize them in `main()` after the codecs come up
5. Set up the I²S TX backend (`assets/audiothingies-2026-05-09/backends/zephyr_i2s_tx.cpp`) to call `AudioEngine::render_block()` for each DMA buffer
6. Add HSM states for `AUDIO_PLAYING` and a transport sub-state for FF/RW/pause

### Add Bluetooth

1. Enable `CONFIG_BT=y` and `CONFIG_BT_HCI_UART=y` in `prj.conf`
2. Configure the HCI UART driver for the CYBT module (UART0 at 115200 baud)
3. Bring up the BT module (assert reset low, release, wait for HCI init)
4. Use Zephyr's Bluetooth API for the profile you want (A2DP source for streaming audio out, A2DP sink for receiving, etc.)

This is the path emvee1968 reports as "a few hours with Claude Code" — see `07-bluetooth-module.md`.

### Add a new effect

1. Drop `assets/audiothingies-2026-05-09/effects/*.hpp` into your effects directory
2. Instantiate `effects::StemEffectRack` per stem
3. Wire it into the audio render pipeline between mixer and codec
4. Map button combos to effect activation via `MidiController::on_rx()` (or via local button handlers)

See `13-dsp-effects.md` for the existing effect set.

## Where to go next

- For Zephyr build setup → `18-zephyr-build-environment.md`
- For unreleased custom firmwares with audio + effects → `20-custom-firmware-state.md`
- For the audio engine code → `12-audio-engine-internals.md`
- For the storage subsystem → `08-emmc-storage.md`
- For Bluetooth integration → `07-bluetooth-module.md`
