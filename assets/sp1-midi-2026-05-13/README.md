# Stem Player BSP

Board Support Package for the Stem Player SP-1 hardware (nRF52840 + CS42L42 + TAS2505 + BQ24232 + eMMC).

Fork this repo to build new firmware for the Stem Player hardware — synths, MIDI controllers, audio effects, or custom Stem Player variants.

## Quick Start

```sh
# Configure (one-time)
PATH="/path/to/zephyr-venv/bin:$PATH" \
ZEPHYR_BASE=/path/to/zephyrproject/zephyr \
ZEPHYR_SDK_INSTALL_DIR=/path/to/zephyr-sdk \
cmake -B build -GNinja -DBOARD=stem_player -DSNIPPET=cdc-acm-console

# Build
ninja -C build

# Flash
ninja -C build flash
```

## What's Included

| Directory | Contents |
|-----------|----------|
| `boards/` | Full board definition (DTS, pinctrl, defconfig) |
| `drivers/audio/` | CS42L42 headphone codec + TAS2505 speaker amp drivers |
| `drivers/charger/` | BQ24232 charger driver (GPIO-based) |
| `include/` | SMF C++ facade, utility headers |
| `subsys/system/` | Watchdog + reset breadcrumbs |
| `subsys/power/` | Battery/charger/USB monitor |
| `app/` | Minimal 2-state HSM skeleton |

## Hardware Map

### Audio (I2C0)
| Device | Address | Function |
|--------|---------|----------|
| CS42L42 | 0x48 | Headphone DAC/ADC codec |
| TAS2505 | 0x18 | Speaker amplifier |

### I2S0 (48kHz, 24-bit, master)
| Signal | Pin |
|--------|-----|
| BCLK | P0.12 |
| LRCK | P0.11 |
| SDOUT | P1.09 |

### Bluetooth UART (CYBT-353027-02)
| Signal | Pin |
|--------|-----|
| TX | P1.02 |
| RX | P1.04 |
| RTS | P1.01 |
| CTS | P1.03 |
| Reset | P0.10 |

### ADC Channels
| Channel | AIN | Pin | Function |
|---------|-----|-----|----------|
| 0 | AIN4 | P0.28 | Battery voltage (divider) |
| 1 | AIN3 | P0.05 | Fader 1 |
| 2 | AIN6 | P0.30 | Fader 2 |
| 3 | AIN2 | P0.04 | Fader 3 |
| 4 | AIN7 | P0.31 | Fader 4 |
| 5 | AIN0 | P0.02 | Track button ladder |
| 6 | AIN1 | P0.03 | Volume/transport ladder |

### PWM LEDs
| PWM | Outputs | Function |
|-----|---------|----------|
| PWM2 | P0.29, P0.26, P1.15, P1.14 | Track LEDs (1-4) |
| PWM3 | P0.01, P1.12, P0.00, P1.13 | Play LEDs (1-4) |

### GPIO
| Pin | Function |
|-----|----------|
| P0.27 | Function button (active high) |
| P1.10 | Button common rail (regulator) |
| P0.13 | Oscillator enable |
| P0.14 | eMMC VccQ enable |
| P1.08 | eMMC reset |
| P0.09 | TAS2505 reset |
| P0.15 | CS42L42 reset |

### Charger (BQ24232)
| Pin | Function |
|-----|----------|
| P0.24 | nPGOOD (power good, active low) |
| P0.22 | nCHG (charging active, active low) |
| P0.21 | CE (charge enable) |
| P0.19 | ISET override |

## Adding Your Code

1. Add source files to `APP_SOURCES` in `CMakeLists.txt`
2. Extend `AppMachine` in `AppStateMachine.hpp` with your subsystem pointers
3. Add new states to the `AppState` enum and state table
4. Initialize your subsystems in `main.cpp` before `app::run()`

The HSM uses Zephyr's SMF (State Machine Framework) with a C++ facade in `include/stem_player/Smf.hpp`.

## Memory Budget

Bare BSP: ~168 KB FLASH (18%), ~43 KB RAM (17%). Full Stem Player firmware uses ~264 KB FLASH, ~219 KB RAM. You have plenty of room.
