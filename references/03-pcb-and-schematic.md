# PCB and Schematic

**Synthesized through:** Lines #846 (2026-05-06), Discord through 2026-05-11.

This file covers the physical board, the flexible printed circuit (FPC), test points, and the canonical pin-to-function mapping for every used pin on the nRF52840. For chip-level summary see `02-hardware-overview.md`. For the SWD header you have to add for development, see `04-debug-interfaces.md`.

## The PCB and the user-facing flex

The SP-1 is built from two boards:

1. **Main PCB** — carries the nRF52840, codecs, amplifier, eMMC, charger, BT module, and the user-facing surface-mount components.
2. **Flexible printed circuit (FPC)** — connects the main PCB to the front-panel button matrix, fader strips, LEDs, and the headphone jack. The two boards meet through a single FPC connector.

The FPC is the most fragile part of disassembly. Several community members have appeared to brick their devices, only to find that the FPC ribbon had simply bent or come slightly loose during reassembly [Lines #118, PedalsandChill — *"Also, just bricked one of my stem players … scratch that I just fixed it guess ribbon cable was slightly bent or something"*]. See `references/23-physical-disassembly.md` for the canonical disassembly procedure.

## FPC connector

| Property | Value |
| --- | --- |
| Manufacturer / part | **Omron XF3B-1945-31A** |
| Pins | 19 |
| Pitch | 0.3 mm |
| Type | Top-and-bottom contact, ZIF lever |

**Citation:** [Lines, TimK PCB reversal, posts in the #305–330 range; Lines thread summary]

The 0.3 mm pitch is fine — close to the limit of what is hand-rework-able without magnification. Replacement FPC is theoretically possible but no community member has needed to do this; if the FPC is damaged, the realistic recovery path is sourcing a donor device.

## Schematic provenance

There is **no official TE schematic**. The community-canonical schematic is the **KiCad reversal performed by TimK** in early-to-mid 2025 [Lines #305–330; Discord references throughout 2026-05]. This work also identified the FPC connector and confirmed power-tree routing.

The KiCad files are not (as of the synthesis date) in `github.com/timknapen/SP-1-dev` — the GitHub Wiki has narrative documentation and photos of the boards but the source schematic is held privately. To get specific net or component-value information beyond what is in this skill, ask TimK in Discord or check whether the wiki has been updated since the synthesis date.

## Complete pin map (nRF52840 GPIOs in use)

Pulled and cross-referenced from `sp1-midi/boards/.../stem_player.dts`, `sp1-midi/boards/.../stem_player-pinctrl.dtsi`, and `SP-1-dev/src/stemplayer_pins.h`. When the two sources disagree, the discrepancy is noted (see "Discrepancies" at bottom).

### Port 0

| GPIO | Function | Direction | Notes |
| --- | --- | --- | --- |
| P0.00 | Play LED 3 (PWM3 OUT2) | OUT | PWM, see `02-hardware-overview.md` |
| P0.01 | Play LED 1 (PWM3 OUT0) | OUT | PWM |
| P0.02 | Track/Play button ladder (AIN0) | IN (analog) | Multi-press encoded by threshold |
| P0.03 | Volume/transport rocker ladder (AIN1) | IN (analog) | 4 buttons, single-press |
| P0.04 | Fader 3 (AIN2) | IN (analog) | |
| P0.05 | Fader 1 (AIN3) | IN (analog) | |
| P0.06 | eMMC CLK | OUT | GPIO during command phase, **SPIM3 SCK** during data reads |
| P0.07 | eMMC DAT0 | I/O | GPIO during command phase, **SPIM3 MISO** during data reads |
| P0.08 | eMMC CMD | I/O | GPIO bit-bang always |
| P0.09 | TAS2505 reset (NFC1 repurposed) | OUT | Active low; held high in normal operation |
| P0.10 | CYBT-353027-02 BT reset (NFC2 repurposed) | OUT | Active low |
| P0.11 | I²S LRCK | OUT | Master mode |
| P0.12 | I²S BCLK / SCK | OUT | Master mode |
| P0.13 | 3.072 MHz audio oscillator enable | OUT | Held high at boot via GPIO hog |
| P0.14 | eMMC VccQ enable | OUT | Active high |
| P0.15 | CS42L42 reset | OUT | Active low |
| P0.19 | (see Discrepancies — README claims BQ24232 ISET here, but DTS + TimK header point at P1.00) | — | |
| P0.21 | BQ24232 CE (charge enable) | OUT | Active low (`ce-gpios = <… GPIO_ACTIVE_LOW>`) |
| P0.22 | BQ24232 nCHG (charging status) | IN | Active low, pull-up |
| P0.24 | BQ24232 nPGOOD (power good) | IN | Active low, pull-up |
| P0.26 | Track LED 2 (PWM2 OUT1) | OUT | PWM |
| P0.27 | Function button | IN | **Direct GPIO** (not on ladder); pull-up, active low |
| P0.28 | Battery voltage (AIN4) | IN (analog) | Via resistor divider |
| P0.29 | Track LED 1 (PWM2 OUT0) | OUT | PWM |
| P0.30 | Fader 2 (AIN6) | IN (analog) | |
| P0.31 | Fader 4 (AIN7) | IN (analog) | |

### Port 1

| GPIO | Function | Direction | Notes |
| --- | --- | --- | --- |
| P1.00 | BQ24232 ISET override | OUT | Active low (per DTS + TimK header — see Discrepancies) |
| P1.01 | UART RTS (to BT module) | OUT | |
| P1.02 | UART TX (to BT module RX) | OUT | |
| P1.03 | UART CTS (from BT module) | IN | |
| P1.04 | UART RX (from BT module TX) | IN | |
| P1.05 | CY SPI CSn | OUT | Documented in TimK header but unused by `sp1-midi` BSP — likely a secondary control path on the BT module |
| P1.07 | I²C0 SDA | I/O | |
| P1.08 | eMMC reset | OUT | Active low |
| P1.09 | I²S SDOUT (data to codecs) | OUT | |
| P1.10 | Button common rail (regulator enable) | OUT | Active high, boots on |
| P1.11 | I²C0 SCL | OUT | |
| P1.12 | Play LED 2 (PWM3 OUT1) | OUT | PWM |
| P1.13 | Play LED 4 (PWM3 OUT3) | OUT | PWM |
| P1.14 | Track LED 4 (PWM2 OUT3) | OUT | PWM |
| P1.15 | Track LED 3 (PWM2 OUT2) | OUT | PWM |

### Pins not currently used

Most other nRF52840 GPIOs are either unrouted, used internally by the chip (e.g., crystal oscillator pins), or unaccounted for in public documentation. The known-used pin count is small enough that there is significant room for custom firmware to repurpose pins if needed — but doing so requires schematic-level access to confirm the pin isn't routed somewhere not yet documented.

## I²C bus

| Property | Value |
| --- | --- |
| Bus | I²C0 (TWIM0 on nRF) |
| SDA | P1.07 |
| SCL | P1.11 |
| Frequency | **400 kHz** (`I2C_BITRATE_FAST` in DTS) |
| Devices | CS42L42 (`0x48`) + TAS2505 (`0x18`) |

Both audio chips share a single bus. The bus has only two slaves; addressing collision is not possible since the addresses are statically configured (no auto-detect needed in production firmware). See `06-audio-codecs.md` for register-level configuration.

## I²S audio bus

| Property | Value |
| --- | --- |
| Peripheral | I²S0 |
| Direction | nRF is **master** (drives BCLK + LRCK) |
| Sample rate | 48 kHz |
| Bit depth | 24-bit (per sample, packed in 32-bit slots on the wire) |
| Channels | 8 (4 stereo stems) on the data wire; codecs select which channels they consume |
| BCLK / SCK | P0.12 |
| LRCK | P0.11 |
| SDOUT | P1.09 |
| Reference clock | 3.072 MHz external oscillator, enabled via P0.13 (= 48000 × 64) |

I²S is transmit-only from the nRF in normal operation (`SDIN` is not pinned out). For headphone microphone input the CS42L42 has its own ADC that the nRF accesses over I²C status registers, not over the I²S data wire.

## Test points (eMMC + SWD)

TimK has indicated that the **eMMC DAT0–DAT3, CLK, and CMD lines are broken out to test points** on the main PCB [Discord #hardware, tkt1000, 2026-05-07 17:53 — *"Actually if I remember correctly DAT0-DAT3 are broken out to test points, so you could read much faster than the MCU!"*]. This means you can dump or write to the eMMC by attaching clip leads or fine probes without desoldering anything.

As of synthesis date, the **exact pad locations on the PCB are not yet documented** in the publicly visible material. TimK said on 2026-05-07 17:43 — *"I'll add documentation of the test points to the wiki"* — but it is unclear whether this has shipped. Check `github.com/timknapen/SP-1-dev/wiki` for an updated test-points page; otherwise ask in Discord.

There is **no factory SWD header**. TimK added his own SWD header to develop firmware [Discord #hardware, tkt1000, 2026-05-07 17:43 — *"No, I only soldered an SWD header"*]. See `04-debug-interfaces.md` for the SWD pin locations and how to add a header without breaking the device.

## Power tree (PCB-level)

| Stage | Source | Output | Notes |
| --- | --- | --- | --- |
| USB-C VBUS in | Host PC / charger | 5 V | Routes to BQ24232 only — does **not** directly feed the nRF |
| BQ24232 | VBUS or battery | System voltage (battery-tracking) | GPIO-controlled charge enable / ISET / status |
| LiPo battery | 3.7 V nominal | Battery rail | Single cell, charged by BQ24232 |
| 3.3 V regulator | Battery rail | 3.3 V | Powers nRF, codecs, eMMC VccQ rail |
| 1.8 V regulator | Battery rail | 1.8 V | Powers eMMC core, possibly codec analog |
| 3.072 MHz oscillator | 3.3 V via P0.13 enable | 3.072 MHz clock | Reference clock for I²S audio chain |
| Button common rail | 3.3 V via P1.10 enable (fixed regulator) | Powers button ladder resistors | Always on while device is on |

Pure-USB operation (battery removed) is generally NOT reliable; the system expects the LiPo as the primary buffer. See `05-power-and-battery.md`.

## Discrepancies

### BQ24232 ISET pin

`sp1-midi/README.md` lists ISET on **P0.19**. The DTS (`stem_player.dts`, line 259) defines `iset-override-gpios = <&gpio1 0 GPIO_ACTIVE_LOW>` which is **P1.00**. TimK's `stemplayer_pins.h` line 80 defines `PIN_BQ_ISET NRF_GPIO_PIN_MAP(1, 00)` which is also **P1.00**. The README is the outlier; trust the DTS and TimK's header.

### Play LED ordering

The DTS aliases `led0`/`led1`/`led2`/`led3` map to `led_play_1`/`led_play_2`/`led_play_3`/`led_play_4` in the order PWM3 outputs 0–3:

- PWM3 OUT0 → P0.01 → `led_play_1` (DTS) ≈ `PIN_LED_4` in `stemplayer_pins.h`
- PWM3 OUT1 → P1.12 → `led_play_2` (DTS) ≈ `PIN_LED_3` in `stemplayer_pins.h`
- PWM3 OUT2 → P0.00 → `led_play_3` (DTS) ≈ `PIN_LED_2` in `stemplayer_pins.h`
- PWM3 OUT3 → P1.13 → `led_play_4` (DTS) ≈ `PIN_LED_1` in `stemplayer_pins.h`

That is — the DTS labels them in PWM-output order; TimK's header labels them in their physical position on the device (LED 1 nearest one end, LED 4 nearest the other). Both are correct; pick a convention and stick with it in your code, but be careful when porting between the two.

### CYBT-related pins

TimK's header defines a `PIN_CY_SPI_CSN = P1.05` that is not surfaced in the `sp1-midi` DTS as of synthesis date. This suggests the CYBT-353027-02 has an SPI-style secondary interface in addition to UART, but no public firmware uses it. Treat P1.05 as "documented but unused" pending more information from TimK or ericlewis.

## Where to go next

- For the SWD header + USB CDC bring-up → `04-debug-interfaces.md`
- For the power architecture and battery handling → `05-power-and-battery.md`
- For codec register-level details → `06-audio-codecs.md`
- For the Bluetooth module → `07-bluetooth-module.md`
- For the eMMC chip and protocol → `08-emmc-storage.md`
- For physical disassembly → `23-physical-disassembly.md`
