# FAQ — Power and Battery

**Q: How is the SP-1 powered?**
A: USB-C → **BQ24232 LiPo charger** → 3.7 V battery → 3.3 V / 1.8 V regulators → nRF, codecs, eMMC, BT module. The nRF doesn't run directly off USB. See `05-power-and-battery.md`.

**Q: What is the BQ24232?**
A: A TI single-cell LiPo charger IC. **GPIO-controlled** (no I²C), so the firmware just toggles enable / current-set lines. Charge parameters (max current, termination, etc.) are set by external resistors. See datasheet.

**Q: What GPIOs talk to the charger?**
A: P0.24 (nPGOOD, in), P0.22 (nCHG, in), P0.21 (CE, out), P1.00 (ISET override, out). Note the README's P0.19 claim is wrong. See `03-pcb-and-schematic.md` Discrepancies.

**Q: How do I measure battery voltage?**
A: SAADC channel 4 / AIN4 / P0.28, behind a resistor divider. Use `sp1-midi/subsys/power/PowerManager.cpp` for the calibrated scale rather than computing from raw mV.

**Q: How do I know if USB is plugged in?**
A: Read **nPGOOD** (P0.24, active low). Low = USB power present. The `PowerManager` exposes a higher-level "is charging / on USB / on battery" interface.

**Q: How do I cleanly power down in custom firmware?**
A: Sequence: mute codecs, reset audio chips, disable BT module, stop PWMs, disable eMMC supply (P0.14 low), disable button-common rail (P1.10 low), disable 3.072 MHz osc (P0.13 low), then `sys_poweroff()`. See `05-power-and-battery.md` "Power-down" section + `known-unknowns.md` Power-off section.

**Q: Does the function button trigger power-down?**
A: In `sp1-midi` it's mapped to `INPUT_KEY_MENU` with a 3-second longpress → `INPUT_KEY_POWER`. The app code is responsible for actually calling `sys_poweroff()` in response. [code: `stem_player.dts` lines 230–237]

**Q: Can I run on USB only without a battery?**
A: Not reliably. Charger expects a battery as the system-rail buffer. emvee1968 disconnected the battery as a way to force a hard reset for re-entering the bootloader [Discord #firmware, 2026-05-08].

**Q: Is there a fuel gauge IC?**
A: No. Charge state is inferred from the ADC measurement of battery voltage on AIN4. To show a percentage, compute it from the voltage curve in your app.

**Q: What charge current does it draw?**
A: Not publicly documented (depends on external resistor values TE chose). Empirically charges fine from a 1 A USB-C source. ISET override lets the firmware switch between two modes if your app implements slow/fast charge.

**Q: What's on the 3.3 V rail vs the 1.8 V rail?**
A: 3.3 V powers the nRF VDD, codec digital, eMMC VccQ. 1.8 V powers eMMC core (and possibly codec analog). Without schematic-level access the exact mapping isn't fully documented.

**Q: Why is power management still a known-unknown if `PowerManager.cpp` exists?**
A: The BSP has the building blocks (charger driver + battery ADC). But cleanly handling power-off — including waking from sleep, sequencing peripheral disable, and integrating with the function button — is not demonstrated by the example MIDI controller app. Custom firmware authors have to wire it together. See `known-unknowns.md`.
