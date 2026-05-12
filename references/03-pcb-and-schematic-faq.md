# FAQ — PCB and Schematic

**Q: Is there an official schematic?**
A: No. TimK reverse-engineered a complete KiCad schematic in 2025; it is the community canonical source. Not currently in the public `SP-1-dev` repo. See `03-pcb-and-schematic.md`.

**Q: What FPC connector is used?**
A: **Omron XF3B-1945-31A** — 19 pins, 0.3 mm pitch. Top-and-bottom contact with a ZIF lever. Connects the main PCB to the user-facing flex (buttons, LEDs, jack). See `03-pcb-and-schematic.md`.

**Q: How fragile is the FPC?**
A: Fragile. Multiple users have thought they bricked devices, only to find the FPC was bent or seated wrong [Lines #118]. Reseat it before assuming a hardware failure. See `23-physical-disassembly.md`.

**Q: How many GPIOs are actually used?**
A: ~30 on Port 0 and ~16 on Port 1, with several specific pins like P1.05 (CY SPI CSn) defined in the pin header but not consumed by any public firmware. See full pin map in `03-pcb-and-schematic.md`.

**Q: Where's the BQ24232 ISET pin really?**
A: **P1.00**, per the DTS and TimK's pin header. The `sp1-midi/README.md` claims P0.19 but it's wrong. See `03-pcb-and-schematic.md` Discrepancies.

**Q: What pins are reclaimed from NFC?**
A: P0.09 (NFC1 → TAS2505 reset) and P0.10 (NFC2 → CYBT-353027-02 reset). The DTS enables `nfct-pins-as-gpios;` at the root. See `02-hardware-overview.md`.

**Q: Is the I²C bus fast or slow?**
A: **Fast mode (400 kHz)** as configured in `sp1-midi`. Both audio codecs share the bus. [code: `stem_player.dts` line 275]

**Q: Are there eMMC test points?**
A: Yes — DAT0–DAT3, CLK, and CMD are broken out [Discord #hardware, tkt1000, 2026-05-07]. Exact pad locations not yet documented publicly. Check `github.com/timknapen/SP-1-dev/wiki` or ask in Discord. See `04-debug-interfaces.md`.

**Q: Is there a factory SWD header?**
A: No. You must solder your own. Required for serious firmware development. See `04-debug-interfaces.md`.

**Q: What's the deal with P1.05?**
A: It's defined in TimK's `stemplayer_pins.h` as `PIN_CY_SPI_CSN` (Cypress SPI chip-select-not) but isn't used by `sp1-midi`. Probably a secondary control interface to the CYBT BT module. Treat as documented-but-unused. See `03-pcb-and-schematic.md`.

**Q: Does the device run on USB without a battery?**
A: Not reliably. USB feeds the BQ24232 charger which expects to see a battery as a buffer. If you remove the battery, the system rail can be unstable. See `05-power-and-battery.md`.

**Q: What's the 3.072 MHz oscillator for?**
A: I²S master reference clock. 48000 × 64 = 3.072 MHz. Enabled via GPIO hog on P0.13 at boot. [code: `stem_player.dts` lines 391–400]
