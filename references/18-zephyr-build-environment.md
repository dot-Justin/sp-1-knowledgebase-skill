# Zephyr Build Environment

**Synthesized through:** Lines #846 (2026-05-06), Discord through 2026-05-11. Code-of-record: `assets/sp1-midi-2026-05-13/{README.md, CMakeLists.txt, prj.conf, app.overlay}` 2026-05-09.

To build custom firmware for the SP-1 you need **Zephyr RTOS** with a specific configuration. The `sp1-midi` BSP gives you a working starting point. This file documents what's required, what's enabled, and how to extend it.

## Prerequisites

| Tool | Required version | How to install |
| --- | --- | --- |
| Zephyr base (RTOS source) | A version that supports nRF52840 + USB 2.0 stack (`CONFIG_USB_DEVICE_STACK_NEXT`) | Follow Zephyr's official "Getting Started" guide; typically a `west init` + `west update` |
| Zephyr SDK | The toolchain matching your Zephyr base | Install per Zephyr docs |
| CMake | 3.20+ | System package manager |
| Ninja | Any recent version | System package manager |
| Python virtualenv | A `zephyr-venv` for west and helper scripts | Standard Python venv |

The `sp1-midi` README's quickstart [code: `assets/sp1-midi-2026-05-13/README.md`]:

```sh
PATH="/path/to/zephyr-venv/bin:$PATH" \
ZEPHYR_BASE=/path/to/zephyrproject/zephyr \
ZEPHYR_SDK_INSTALL_DIR=/path/to/zephyr-sdk \
cmake -B build -GNinja -DBOARD=stem_player -DSNIPPET=cdc-acm-console

ninja -C build
ninja -C build flash
```

The `flash` target uses Nordic's nrfjprog (or West's flash backend) to push the binary over **SWD**. If you don't have SWD wired up (most users don't), use `ninja -C build` to produce `build/zephyr/zephyr.bin`, then flash that binary via `solderless.engineering` or a custom bootloader-protocol client.

## Required environment variables

| Variable | Purpose |
| --- | --- |
| `ZEPHYR_BASE` | Path to the `zephyr/` directory of your Zephyr checkout |
| `ZEPHYR_SDK_INSTALL_DIR` | Path to the Zephyr SDK (containing the cross-compiler) |
| `PATH` | Should include Python venv for west, plus any locally-installed cmake/ninja |

`west` is the orchestrator; calling `cmake` directly works too but you lose some convenience.

## Board target

```
-DBOARD=stem_player
```

This selects `assets/sp1-midi-2026-05-13/boards/teenageengineering/stem_player/` as the board definition. It includes:

- `stem_player.dts` — full device tree (see `02-hardware-overview.md`)
- `stem_player-pinctrl.dtsi` — pin assignments
- `stem_player_defconfig` — Kconfig defaults for the board
- `Kconfig.defconfig` — board-specific config overrides
- `board.cmake` — Zephyr integration
- `doc/index.rst` — board documentation

If you fork `sp1-midi` to start a new firmware, keep the `boards/teenageengineering/stem_player/` tree intact — your new app code goes in `app/` (or wherever you want), not in the board definition.

## Snippet: CDC ACM console

```
-DSNIPPET=cdc-acm-console
```

Adds a USB CDC ACM endpoint so you get a Zephyr shell over USB. Without this snippet, you have no serial output during normal runtime (only USB MIDI is exposed). Strongly recommended for development.

The snippet wires `CONFIG_STDOUT_CONSOLE` and `CONFIG_SHELL_BACKEND_SERIAL` to the USB CDC ACM endpoint. On the host, the device appears as `/dev/ttyACM0` (or similar); connect with `screen`, `picocom`, or any serial terminal.

The shell prompt is `stem> ` (configured via `CONFIG_SHELL_PROMPT_UART`).

## Required Kconfig enablements (from `prj.conf`)

Full list with brief annotations:

```
# C++17 with full libcpp
CONFIG_CPP=y
CONFIG_STD_CPP17=y
CONFIG_REQUIRES_FULL_LIBCPP=y

# Peripherals
CONFIG_I2C=y                        # CS42L42 + TAS2505 codec control
CONFIG_I2S=y                        # Audio data bus
CONFIG_I2S_NRFX_TX_BLOCK_COUNT=8    # 8 DMA blocks for deep buffering
CONFIG_UART_ASYNC_API=y             # BT module (CYBT) UART
CONFIG_UART_INTERRUPT_DRIVEN=y
CONFIG_UART_LINE_CTRL=y
CONFIG_SERIAL=y
CONFIG_GPIO=y
CONFIG_GPIO_HOGS=y                  # P0.13 oscillator-enable hog
CONFIG_PWM=y                        # LED PWMs
CONFIG_ADC=y                        # Faders + battery
CONFIG_CHARGER=y                    # BQ24232
CONFIG_REGULATOR=y                  # Button common rail
CONFIG_REGULATOR_FIXED=y
CONFIG_AUDIO=y
CONFIG_AUDIO_CODEC=y

# Input subsystem (button events)
CONFIG_INPUT=y
CONFIG_INPUT_MODE_SYNCHRONOUS=y
CONFIG_INPUT_ADC_KEYS=y             # Analog button ladders
CONFIG_INPUT_GPIO_KEYS=y            # Function button
CONFIG_INPUT_LONGPRESS=y            # 3-second longpress → power

# State Machine Framework
CONFIG_SMF=y
CONFIG_SMF_ANCESTOR_SUPPORT=y
CONFIG_SMF_INITIAL_TRANSITION=y

# Power management
CONFIG_POWEROFF=y                   # sys_poweroff()
CONFIG_REBOOT=y                     # sys_reboot()
CONFIG_PM=y
CONFIG_PM_DEVICE=y

# Storage
CONFIG_DISK_ACCESS=y                # Zephyr disk subsystem (for eMMC)

# USB (composite: MIDI 2.0 + CDC ACM)
CONFIG_USB_DEVICE_STACK_NEXT=y      # Next-gen USB stack (required)
CONFIG_USBD_MIDI2_CLASS=y           # MIDI 2.0 (UMP) class
CONFIG_MIDI2_UMP_STREAM_RESPONDER=y # UMP stream responder
CONFIG_USBD_CDC_ACM_CLASS=y         # CDC ACM (serial console)
CONFIG_CDC_ACM_SERIAL_INITIALIZE_AT_BOOT=n  # Manual init in app
CONFIG_STDOUT_CONSOLE=y
CONFIG_SHELL=y
CONFIG_SHELL_BACKEND_SERIAL=y
CONFIG_SHELL_BACKEND_SERIAL_CHECK_DTR=n     # Don't require DTR signal
CONFIG_SHELL_PROMPT_UART="stem> "
CONFIG_SHELL_STACK_SIZE=2048
CONFIG_SHELL_LOG_BACKEND=y

# Logging
CONFIG_LOG=y
CONFIG_LOG_BUFFER_SIZE=2048
CONFIG_ASSERT=y
CONFIG_THREAD_NAME=y

# Memory & threads
CONFIG_MAIN_STACK_SIZE=2048
CONFIG_SYSTEM_WORKQUEUE_STACK_SIZE=3072
CONFIG_HEAP_MEM_POOL_SIZE=16384
CONFIG_RING_BUFFER=y
CONFIG_USBD_THREAD_STACK_SIZE=768

# Performance
CONFIG_SPEED_OPTIMIZATIONS=y        # -O2 for size+speed balance
CONFIG_NRF_ENABLE_ICACHE=y          # Use the nRF52840 instruction cache
CONFIG_FPU=y
CONFIG_FPU_SHARING=y                # Allow FPU use across threads
```

## USB descriptor configuration

`app.overlay` defines the USB MIDI device and configures CDC ACM descriptors. Key elements:

- USB VID = **0x1209** (PID.codes, the open-source vendor ID)
- USB PID = **0x0001**
- Product string = "SP-1 MIDI Controller"
- Manufacturer string = "Teenage Engineering"
- Bus speed = USB Full Speed (12 Mbps; nRF52840 doesn't support High Speed)
- Configuration = composite device (IAD with Misc/0x02/0x01 device codes)

Note that **VID 0x1209 PID 0x0001 is not registered to TE** — it's the PID.codes "test" PID. If you release custom firmware publicly, request a proper PID from PID.codes or use a different VID you own. The "Teenage Engineering" manufacturer string is also potentially misleading; consider changing it for non-official firmware.

## Source layout

```
sp1-midi/
├── CMakeLists.txt
├── prj.conf
├── app.overlay                  ← USB descriptor overlay
├── Kconfig                       ← App-level Kconfig (minimal)
├── README.md
├── boards/                       ← Board definition (DTS, pinctrl, defconfig)
│   └── teenageengineering/stem_player/
├── drivers/                      ← Custom drivers
│   ├── audio/                    ← CS42L42 + TAS2505
│   └── charger/                  ← BQ24232
├── include/                      ← Shared headers
│   ├── stem_player/Smf.hpp       ← SMF C++ facade
│   └── zpp/                      ← Misc utility headers
├── subsys/                       ← Subsystems
│   ├── power/                    ← PowerManager
│   ├── storage/emmc/             ← EmmcDriver + StemEmmcDevice
│   └── system/                   ← Watchdog + ResetBreadcrumbs
└── app/                          ← Application
    ├── main.cpp                  ← Boot + USB + input handling
    ├── AppStateMachine.{hpp,cpp} ← HSM definitions
    ├── MidiController.{hpp,cpp}  ← Per-fader / per-button MIDI logic
    └── Leds.{hpp,cpp}            ← PWM LED helpers
```

[code: `assets/sp1-midi-2026-05-13/` GitHub repository structure]

## Adding your code

Per `assets/sp1-midi-2026-05-13/README.md`:

> 1. Add source files to `APP_SOURCES` in `CMakeLists.txt`
> 2. Extend `AppMachine` in `AppStateMachine.hpp` with your subsystem pointers
> 3. Add new states to the `AppState` enum and state table
> 4. Initialize your subsystems in `main.cpp` before `app::run()`

The HSM uses Zephyr's SMF (State Machine Framework) with a C++ facade in `include/stem_player/Smf.hpp`. States are: `IDLE`, `RUNNING`, `SHUTTING_DOWN`, `DEEP_SLEEP`. Add your states alongside (e.g., `AUDIO_PLAYING`, `EFFECT_ACTIVE`) and add corresponding entry/exit/run handlers.

## Flashing

### Via SWD (development)

`ninja -C build flash` uses West's flash backend (typically nrfjprog). Requires SWD connection — `04-debug-interfaces.md` covers adding a header.

### Via solderless.engineering (recommended for end users)

When the tool is online:
1. Open `https://solderless.engineering/`
2. Hold Track 1 + Track 4, plug in USB-C
3. Select your `zephyr.bin` and click flash
4. The tool walks the bootloader protocol; no SWD needed

### Via custom bootloader client

If solderless is offline, use a Python (or other language) client against the bootloader protocol — `15-bootloader-protocol.md`. `theunflappable`'s `test_bootloader.py` (Discord #firmware, 2026-05-08) is a starting point.

## Memory budget

| Item | Bare BSP | Full TE firmware |
| --- | --- | --- |
| Flash | ~168 KB (18% of 1 MB) | ~264 KB (28%) |
| RAM | ~43 KB (17% of 256 KB) | ~219 KB (86%) |

Plenty of headroom for custom firmware that adds the audio engine. The TE firmware's RAM usage is dominated by the eMMC prefetch slots (10 × ~8 KB = 80 KB) and the per-stem mixer buffers (~16 KB), plus the delay buffers (~64 KB).

## Common build issues

### "BOARD=stem_player not found"

`-DBOARD=stem_player` requires Zephyr to find the board definition at `boards/teenageengineering/stem_player/`. Make sure you're running cmake from the `sp1-midi` repo root, where `boards/` is at the expected location.

### "CONFIG_USB_DEVICE_STACK_NEXT not available"

You're on an older Zephyr that doesn't have the next-gen USB stack. Update Zephyr base to a recent release.

### "I2S_NRFX_TX_BLOCK_COUNT undefined"

This is an NRFX-specific option. Make sure your Zephyr base includes nRF52 support and you're using a board that selects NRFX I²S.

### USB enumeration fails on host

Often the descriptor changes don't take effect because the host caches USB devices. Try:
- Unplug and re-plug
- On Linux: `sudo udevadm control --reload-rules`
- On macOS: occasionally needs an "Apple Configurator" reset or a USB-C cable swap

### Device boots but no audio

The `sp1-midi` BSP is a **MIDI controller** — it does NOT play audio out of the box. To get audio playback, integrate `assets/audiothingies-2026-05-09/` and `assets/storagethingies-2026-05-09/` into your app, set up the I²S TX path, and configure the codecs. See `12-audio-engine-internals.md`.

## When you don't have SWD

If you don't add an SWD header, your iteration loop is:

1. Edit code
2. Build
3. Flash via solderless (or your own bootloader client)
4. Plug in to USB CDC ACM
5. Watch logs in your serial terminal
6. Find the bug
7. Repeat

Without breakpoints. This works but is slow. For serious development, add SWD.

When `solderless.engineering` is offline (current state as of 2026-05-09), you must use your own client. Plan for this.

## Where to go next

- For what the `sp1-midi` BSP actually does → `19-sp1-midi-bsp.md`
- For published custom firmwares → `20-custom-firmware-state.md`
- For the audio engine to add to your build → `12-audio-engine-internals.md`
- For DTS-level details on pins → `02-hardware-overview.md` and `03-pcb-and-schematic.md`
- For the bootloader protocol when solderless is offline → `15-bootloader-protocol.md`
