# Power and Battery

**Synthesized through:** Lines #846 (2026-05-06), Discord through 2026-05-11.

The SP-1 runs from a single-cell LiPo battery, charged via USB-C through a **TI BQ24232** charge controller. The device does not power directly off USB; USB feeds the charger, which feeds the system rail.

This file covers what is wired up, the GPIO control interface for the charger, the battery measurement path, and the practical implications for custom-firmware power management. For physical disassembly + battery handling see `23-physical-disassembly.md`.

## High-level power path

```
USB-C VBUS (5 V)
      │
      ▼
┌────────────────┐         ┌──────────────────┐
│   BQ24232      │ ◄────── │  LiPo battery    │
│ (charger)      │         │  (3.7 V nominal) │
└────────┬───────┘         └────────┬─────────┘
         │                          │
         ▼                          ▼
       (system rail tracks battery while charging or running)
         │
         ▼
┌────────────────┐         ┌────────────────┐
│  3.3 V LDO     │         │  1.8 V LDO     │
└────────┬───────┘         └────────┬───────┘
         │                          │
         ▼                          ▼
    nRF52840 VDD,               eMMC core,
    codecs, eMMC VccQ,          codec analog
    BT module,
    GPIO drive
```

Without the battery in the loop, the system can be intermittent: USB → BQ24232 → system rail works in principle, but the charger expects to see a battery as the storage element and behaves unpredictably with it removed. Plan on having a charged battery connected when developing.

## BQ24232 — what it is

The **Texas Instruments BQ24232** is a single-cell LiPo charge management IC. Unlike its I²C-controlled siblings (BQ24296 etc.), the BQ24232 is **GPIO-controlled** — no register-level interface, just pins for enable, current-set override, and status indication. This makes the driver simple but means that detailed parameters (max charge current, termination current, etc.) are set by **external resistors** on the board, not in software.

Public datasheet: search "TI BQ24232" — Texas Instruments publishes the full spec.

## BQ24232 pin map (nRF52840 side)

[Source: `sp1-midi/boards/.../stem_player.dts` lines 253–260; `SP-1-dev/src/stemplayer_pins.h` lines 80–84. The `sp1-midi/README.md` lists P0.19 for ISET but this contradicts both the DTS and TimK's pin header — trust the DTS / header.]

| Signal | Pin | Direction | Active level | Function |
| --- | --- | --- | --- | --- |
| **nPGOOD** | P0.24 | IN | Active **low**, pull-up | Goes low when USB power is good (≥ input UVLO and < OVP). Reads high when on battery only. |
| **nCHG** | P0.22 | IN | Active **low**, pull-up | Goes low while a charge cycle is in progress. Reads high when not charging (full, fault, or no USB). |
| **CE** (charge enable) | P0.21 | OUT | Active **low** | Drive low to enable charging. Drive high (or float, with internal pull-up on the IC) to disable. |
| **ISET override** | P1.00 | OUT | Active **low** | When asserted, modifies the resistor-set charge current — used to switch between "fast charge" and "slow charge" modes if implemented. |

In `sp1-midi`'s charger driver these are configured as:

```
pgood-gpios       = <&gpio0 24 (GPIO_ACTIVE_LOW | GPIO_PULL_UP)>;
chg-gpios         = <&gpio0 22 (GPIO_ACTIVE_LOW | GPIO_PULL_UP)>;
ce-gpios          = <&gpio0 21 GPIO_ACTIVE_LOW>;
iset-override-gpios = <&gpio1 0 GPIO_ACTIVE_LOW>;
```

[code: `stem_player.dts` lines 256–259]

## Battery voltage measurement

| Signal | Pin | ADC config |
| --- | --- | --- |
| Battery rail (divided) | P0.28 (AIN4) | SAADC channel 4, gain 1/4, reference VDD/4, 12-bit, 3 µs acquisition |

The battery feeds a resistor divider to bring the voltage into the SAADC's input range. The exact divider ratio is not in the publicly visible documentation as of synthesis date — see `sp1-midi/subsys/power/PowerManager.cpp` for whether the BSP applies a calibrated scale factor, or measure on your own hardware.

A nominal mapping (subject to driver verification):
- 0 mV ADC reading ≈ battery near 0 V (impossible / disconnected)
- 4096 mV ADC reading ≈ upper rail (something like 4.2 V battery × divider × gain)

Use the `PowerManager.cpp` code as the authoritative scaling rather than computing your own.

## Driver: `charger_bq24232`

The `sp1-midi` BSP includes a Zephyr charger driver at `drivers/charger/charger_bq24232.c` / `.h`. It exposes the standard Zephyr charger API (`CONFIG_CHARGER=y`) and:

- Reads `nPGOOD` to know if USB is connected
- Reads `nCHG` to know if a charge cycle is in progress
- Drives `CE` to enable/disable charging
- Drives `ISET` override when current-mode switching is wanted

It does **not** report a fuel-gauge state of charge — there is no fuel gauge IC on the SP-1. Battery state is inferred from the ADC measurement on AIN4. If you want a percentage indicator, you compute it yourself from the voltage curve.

`subsys/power/PowerManager.cpp` consumes the charger + the battery ADC and provides the higher-level "is USB plugged in, am I charging, what's my charge level" interface used by the app.

## USB-only operation (without a battery)

Generally **don't try to run the device without the battery**. Some takeaways:

1. With a working battery installed, the system runs whether USB is plugged in or not. USB simply tops up the cell.
2. With the battery **disconnected** entirely, the BQ24232 still has a power path from VBUS to the system rail, but in practice the regulators see unstable voltage transients during boot, and the device may not enumerate.
3. emvee1968 reported having to **disconnect the battery and re-attach it** to recover their custom firmware from a state where the firmware was running but couldn't power down [Discord #firmware, 2026-05-08 23:56 — *"right now I have no functionality to turn off the device.. lol I had to disconnect the battery to get the bootloader once I flashed."*]. This works because cutting battery while USB is plugged collapses system voltage hard enough to reset the nRF, which then re-enumerates as Track1+4 bootloader if you hold the buttons.

## Power-down in custom firmware

This is a **known unknown** at synthesis date — see `known-unknowns.md` for the full state. Summary:

- The **function button** (P0.27, direct GPIO) is what stock firmware uses to power down. `sp1-midi` registers a 3-second longpress on this button that fires `INPUT_KEY_POWER` [code: `stem_player.dts` lines 230–237].
- ericlewis emphasized: *"ALWAYS REMEMBER TO PROGRAM IN YOUR POWER DOWN"* [Discord #firmware, ericlewis, 2026-05-09 01:02].
- The actual nRF52840 deep-sleep route uses Zephyr's `CONFIG_POWEROFF=y` (enabled in `sp1-midi/prj.conf`) and the `sys_poweroff()` API.
- For a complete power-off, custom firmware should:
  1. Mute/disable the audio codecs (CS42L42 reset asserted, TAS2505 reset asserted)
  2. Disable the BT module (assert P0.10 reset, then de-power its UART)
  3. Stop driving PWM LEDs
  4. Disable the eMMC supply (P0.14 low) and assert eMMC reset (P1.08 low)
  5. Disable the button common rail (P1.10 low)
  6. Disable the 3.072 MHz oscillator (P0.13 low)
  7. Call `sys_poweroff()` to put the nRF into SYSTEM_OFF mode

Steps 1–6 are best-effort cleanup; the device will wake on USB-attach or — depending on your wakeup configuration — on a GPIO interrupt from the function button. nRF SYSTEM_OFF wake sources are described in the Nordic spec; `sp1-midi` does not (as of synthesis date) demonstrate a complete wake configuration.

## Charging current — practical notes

Per [TKT wiki: Battery-charger, accessed 2026-05-12], the BQ24232 is configured to **allow up to 500 mA draw from USB**. This matches USB 2.0's default port current limit and means the SP-1 should charge normally on any USB-A or USB-C port without negotiation. The 500 mA cap is set by the ISET resistor network on the PCB, not by software.

The same wiki page describes the ISET pin behavior: the voltage at ISET reflects the **actual charging current** and can be monitored via a voltage divider for fuel-gauge-like inference. ISET-override (P1.00) flips between two charge-current setpoints — useful for switching between "fast charge when plugged into a wall adapter" and "slow charge when plugged into a low-power source" if the firmware wants to implement that distinction. `sp1-midi` doesn't actively switch this in the basic BSP, but the hook is there.

Specific resistor values setting the charge currents are not publicly documented (they're on the schematic, not in code).

## Where to go next

- For the function-button longpress mapping → `02-hardware-overview.md` (button matrices section)
- For the custom-firmware power-down sequence (status, gaps) → `known-unknowns.md` (Power-off section)
- For the eMMC supply pin behavior → `08-emmc-storage.md`
- For physical disassembly and battery removal procedure → `23-physical-disassembly.md`
