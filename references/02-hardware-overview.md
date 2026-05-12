# Hardware Overview

**Synthesized through:** Lines #846 (2026-05-06), Discord through 2026-05-11.

Authoritative source for chip identities, memory map, and high-level architecture. For pin-level mapping see `references/03-pcb-and-schematic.md`. For peripheral details see the topic-specific references (`05-power-and-battery.md`, `06-audio-codecs.md`, `07-bluetooth-module.md`, `08-emmc-storage.md`).

## Block diagram (functional)

```
┌─────────────────────────────────────────────────────────────────────────┐
│                          USB-C  (data + power)                          │
└──────────────┬────────────────────────────────────┬─────────────────────┘
               │ data                               │ +VBUS
               │                                    ▼
               │                            ┌───────────────┐
               │                            │  BQ24232      │
               │                            │  LiPo charger │
               │                            └───────┬───────┘
               │                                    │
               │                                    ▼
               │                            ┌───────────────┐
               │                            │  3.7 V LiPo   │
               │                            └───────┬───────┘
               │                                    │
               │                                    ▼
               │                            ┌───────────────┐
               │                            │  Regulators   │ → 3V3, 1V8
               │                            └───────┬───────┘
               │                                    │
               ▼                                    ▼
        ┌─────────────────────────────────────────────────┐
        │              nRF52840 QIAA (aQFN73)             │
        │   Cortex-M4F @ 64 MHz · 1 MB FLASH · 256 KB RAM │
        │                                                 │
        │   USB Device  I²C0  I²S0  SPIM3  SAADC  PWM2/3 │
        │   GPIO0/1     UART0 (BT)         GPIOTE        │
        └──┬──────┬──────┬──────┬──────┬──────┬──────┬───┘
           │      │      │      │      │      │      │
           │      │      │      │      │      │      └─── Track LEDs (4) + Play LEDs (4)
           │      │      │      │      │      └─── Faders (4) + button ladders (2) + battery
           │      │      │      │      └─── eMMC (CLK/CMD/DAT0, 1-bit mode, 32 MHz)
           │      │      │      └─── CYBT-353027-02 (Bluetooth module, UART-attached)
           │      │      └─── I²S audio bus (master, 48 kHz / 24-bit) ─┐
           │      └─── I²C: CS42L42 (0x48) + TAS2505 (0x18)            │
           └─── USB device (MIDI + CDC ACM)                            │
                                                                       ▼
                                                          ┌────────────────────┐
                                                          │ CS42L42 (headphone │
                                                          │ DAC/ADC, TRRS)     │
                                                          └────────────────────┘
                                                          ┌────────────────────┐
                                                          │ TAS2505 (speaker   │
                                                          │ amp, mono)         │
                                                          └────────────────────┘
```

## Bill of materials (key ICs)

| Component | Part | Role |
| --- | --- | --- |
| SoC | **Nordic nRF52840** (QIAA aQFN73 package) | Cortex-M4F + 2.4 GHz radio (radio unused — Bluetooth is via the CYBT module) |
| Audio codec | **Cirrus Logic CS42L42** (I²C 0x48) | Headphone DAC/ADC with TRRS jack support |
| Audio amplifier | **Texas Instruments TAS2505** (I²C 0x18) | Mono internal-speaker class-D amplifier with built-in DSP |
| eMMC | **Toshiba THGBMNG5D1LBAIL** | 4 GB eMMC 5.0, used in 1-bit mode at 32 MHz |
| Battery charger | **Texas Instruments BQ24232** | Single-cell LiPo charger; GPIO-controlled (no I²C) |
| Bluetooth module | **Cypress/Infineon CYBT-353027-02** | UART-attached Bluetooth Classic + LE module |
| FPC connector | **Omron XF3B-1945-31A** | 19-pin, 0.3 mm pitch; connects PCB to user-facing button/LED/jack flex |

**Citations for IC identities:** [Lines #5–70 (community ID); Discord #hardware, tkt1000, 2026-05-07; code: `sp1-midi/boards/.../stem_player.dts`; code: `SP-1-dev/src/stemplayer_pins.h`].

## Memory map (nRF52840 flash)

[code: `sp1-midi/boards/.../stem_player.dts`]

| Partition | Offset | Size | Purpose |
| --- | --- | --- | --- |
| `bootloader` | `0x00000000` | `0x00020000` (128 KB) | TE bootloader (Track 1 + Track 4 trigger lives here) |
| `image-0` (slot0) | `0x00020000` | `0x000df000` (892 KB) | Application firmware (TE stock OR custom) |
| `storage` | `0x000ff000` | `0x00001000` (4 KB) | Zephyr settings storage (in `sp1-midi` BSP) |

Note: this partition table is the one `sp1-midi` ships with for custom firmware. The stock TE firmware uses the same bootloader region (0x00000000–0x00020000) because the bootloader is what `solderless.engineering` exploits, but the app region layout may differ inside the TE binary. The custom firmware fits comfortably in slot0; ericlewis's MIDI-controller BSP uses about 168 KB of flash and 43 KB of RAM, leaving plenty of headroom.

## Memory map (other storage)

| Storage | Size | Used for |
| --- | --- | --- |
| nRF52840 RAM (`sram0`) | 256 KB | Runtime state |
| eMMC (THGBMNG5D1LBAIL) | 4 GB | Audio album image (~311 MB per album) + metadata |

The eMMC is **separate from the nRF52840's internal flash**. Custom firmware lives in the nRF's flash; the audio data lives on the eMMC. Loading custom stems and loading custom firmware are independent operations.

## Power architecture

1. USB-C input voltage routes to the **BQ24232 LiPo charger**.
2. The charger powers the **LiPo battery** and outputs system voltage.
3. System voltage feeds onboard regulators that produce **3.3 V** and **1.8 V** rails.
4. The nRF52840, codecs, eMMC, and BT module run off the regulated rails.

**Implication:** when the battery is dead and USB is plugged in, the device powers up via the BQ24232's pass-through. But if the battery is **disconnected entirely** (e.g., during reassembly), the device will not power on through USB alone in some configurations. emvee1968 reported having to disconnect the battery to force a power-cycle for re-flashing [Discord #firmware, 2026-05-08 23:56].

The **3.072 MHz audio reference oscillator** is enabled via GPIO P0.13 (active high) [code: `stemplayer_pins.h` line 62 — `PIN_I2S_OSC_EN`]. This clock supplies the master reference for the audio codec / I²S subsystem. The DTS unconditionally enables this pin on boot via a GPIO hog [code: `stem_player.dts` lines 391–400].

## Peripheral assignments at a glance

| nRF peripheral | Used for | Pins (overview — see `03-pcb-and-schematic.md`) |
| --- | --- | --- |
| USB Device | MIDI 2.0 + CDC ACM (host link) | Standard nRF USB pins |
| I²C0 (TWIM0) | CS42L42 + TAS2505 control | SDA P1.07, SCL P1.11 |
| I²S0 | Audio stream to codecs | BCLK P0.12, LRCK P0.11, SDOUT P1.09 |
| SPIM3 | eMMC data reads (1-bit mode reuses SPI peripheral for DAT0) | CLK P0.06 (shared), MISO P0.07 (DAT0 shared) |
| GPIO bit-bang | eMMC CMD line | P0.08 |
| UART0 | Bluetooth (CYBT-353027-02) | TX P1.02, RX P1.04, RTS P1.01, CTS P1.03, Reset P0.10 |
| SAADC | Faders, button ladders, battery | 7 channels — see ADC table |
| PWM2 | Track LEDs (1–4) | P0.29, P0.26, P1.15, P1.14 |
| PWM3 | Play LEDs (1–4) | P0.01, P1.12, P0.00, P1.13 |
| GPIOTE | Function button + button-common regulator + reset lines | P0.27 (fnc), P1.10 (btn-com rail) |

## ADC channel map

[code: `stem_player.dts` `zephyr,user` node + `stemplayer_pins.h`]

| SAADC channel | nRF AIN | GPIO | Source |
| --- | --- | --- | --- |
| 0 | AIN0 | P0.02 | **Track button + Play button ladder** (analog ladder, 8 thresholds per button to support multi-press combos) |
| 1 | AIN1 | P0.03 | **Volume / transport rocker ladder** (4 buttons, single-press: rwd, vol−, fwd, vol+) |
| 2 | AIN2 | P0.04 | Fader 3 |
| 3 | AIN3 | P0.05 | Fader 1 |
| 4 | AIN4 | P0.28 | Battery voltage (resistor divider) |
| 6 | AIN6 | P0.30 | Fader 2 |
| 7 | AIN7 | P0.31 | Fader 4 |

All channels use gain `1/4` and reference `VDD/4` with 12-bit resolution and 3 µs acquisition time. The 4-channel resolution maps 0…4080 mV across the analog range.

Channel 5 (AIN5) is unused.

## Button matrices

[code: `stem_player.dts` `adc_keys_*` and `buttons` nodes]

The SP-1 reads buttons through two analog ladders (resistor networks) plus one direct GPIO. The ladder approach lets the device detect multi-button presses through threshold combinations.

**Track / Play ladder (AIN0):** the Track 1, Track 2, Track 3, Track 4, and Play buttons each have **8 voltage threshold ranges** (e.g., for Track 1: 179, 517, 779, 996, 1178, 1334, 1467, 1584 mV). The eight thresholds encode all possible single-and-multi-press combinations of the four track buttons plus Play. **This is why Track 1 + Track 4 can be detected as a distinct combination** for the bootloader trigger.

**Volume / transport rocker (AIN1):** four button thresholds, single-press only:

| Button | Threshold range (mV) | Mapped to (in `sp1-midi`) |
| --- | --- | --- |
| Rewind rocker | 355–441 | `INPUT_KEY_BACK` |
| Volume down | 638–793 | `INPUT_KEY_VOLUMEDOWN` |
| Fast-forward rocker | 1064–1321 | `INPUT_KEY_FASTFORWARD` |
| Volume up | 1598–1984 | `INPUT_KEY_VOLUMEUP` |

**Function button (P0.27, direct GPIO):** the "•• function" button is **not on a ladder**. It is a discrete GPIO with pull-up. In `sp1-midi` it's mapped to `INPUT_KEY_MENU` with a 3-second longpress to `INPUT_KEY_POWER` for graceful shutdown. The DTS uses `GPIO_PULL_UP | GPIO_ACTIVE_LOW`, meaning the pin reads high when not pressed and goes low when pressed.

**Button common rail (P1.10):** powers the ladder resistor networks. Implemented as a fixed regulator that boots on and stays on. Disabling it would deactivate all ladder-button reading without affecting the function button (since fnc is direct GPIO, not powered through the rail).

## LEDs

The SP-1 has 8 user-visible LEDs driven by two PWM peripherals.

| Group | PWM peripheral | Pins | Purpose |
| --- | --- | --- | --- |
| Track LEDs 1–4 | PWM2 | P0.29, P0.26, P1.15, P1.14 | One per stem; brightness indicates fader level / mute state |
| Play LEDs 1–4 | PWM3 | P0.01, P1.12, P0.00, P1.13 | The four LEDs between Play and the Function button; play position / animation |

PWM period in `sp1-midi`: 1024 µs (~976 Hz refresh).

**Note:** TimK's `stemplayer_pins.h` lists the playback LEDs in a different order than the DTS aliases. The DTS aliases `led0` through `led3` map to `play_1` through `play_4` (PWM3 outputs 0–3 in DTS order), while TimK's header defines `PIN_LED_1` as P1.13 (corresponding to PWM3 output 3). Either ordering is fine in practice but be careful when porting code across the two sources.

## Reset / power lines (GPIO)

| Pin | Function | Active level |
| --- | --- | --- |
| P0.09 | TAS2505 reset (NFC1 repurposed as GPIO) | Active low |
| P0.10 | CYBT-353027-02 (Bluetooth) reset (NFC2 repurposed as GPIO) | Active low |
| P0.13 | 3.072 MHz audio oscillator enable | Active high (held high by GPIO hog at boot) |
| P0.14 | eMMC VccQ enable | Active high |
| P0.15 | CS42L42 reset | Active low |
| P1.08 | eMMC reset | Active low |

The DTS includes `nfct-pins-as-gpios;` at the root — this reclaims the NFC peripheral pins (P0.09 and P0.10) as standard GPIO. Without that flag, those pins would be reserved for NFC and unavailable to drive the codec/Bluetooth resets. This is consistent with TimK's pin header [code: `stemplayer_pins.h` lines 53 — `// P0.09 / NFC1` and line 77 — `// P0.10 / NFC2`].

## Discrepancy: BQ24232 ISET pin

There is a small documentation conflict between three sources on which pin the BQ24232's **ISET override** is connected to.

- `sp1-midi/README.md` says: **P0.19**
- `sp1-midi/boards/.../stem_player.dts` says: **P1.00** [`iset-override-gpios = <&gpio1 0 GPIO_ACTIVE_LOW>`]
- `SP-1-dev/src/stemplayer_pins.h` line 80 says: **P1.00** [`PIN_BQ_ISET NRF_GPIO_PIN_MAP(1, 00)`]

The DTS and TimK's pin header agree on **P1.00**. The README appears to be the error. When working with the charger driver, trust the DTS / pin header. See `05-power-and-battery.md` for more on the BQ24232 driver.

## Where to go next

- For pin-level mapping with full GPIO table → `03-pcb-and-schematic.md`
- For SWD header + USB CDC + eMMC test points → `04-debug-interfaces.md`
- For the LiPo / charger / USB power flow → `05-power-and-battery.md`
- For CS42L42 + TAS2505 register configuration → `06-audio-codecs.md`
- For the CYBT Bluetooth module → `07-bluetooth-module.md`
- For the eMMC chip and protocol → `08-emmc-storage.md`
- For the bootloader region (memory map detail) → `15-bootloader-protocol.md`
