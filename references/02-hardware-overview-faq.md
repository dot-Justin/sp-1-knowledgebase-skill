# FAQ — Hardware Overview

**Q: What SoC is in the SP-1?**
A: Nordic Semiconductor **nRF52840** (QIAA aQFN73 package). Cortex-M4F at 64 MHz, 1 MB flash, 256 KB RAM. See `02-hardware-overview.md`.

**Q: How much storage is there?**
A: A **4 GB Toshiba THGBMNG5D1LBAIL eMMC**, separate from the nRF52840's internal 1 MB flash. The eMMC holds the audio album (~311 MB typical). See `08-emmc-storage.md`.

**Q: What audio chips are on board?**
A: **Cirrus Logic CS42L42** (I²C 0x48) for the headphone DAC/ADC, and **TI TAS2505** (I²C 0x18) for the internal speaker amplifier. See `06-audio-codecs.md`.

**Q: How does the device get power?**
A: USB-C → **BQ24232 LiPo charger** → 3.7 V battery → 3.3 V / 1.8 V regulators → everything else. The nRF doesn't run directly off USB. See `05-power-and-battery.md`.

**Q: Where's the Bluetooth?**
A: A **CYBT-353027-02** module on UART0 (115200, with hardware flow control). Not used by any public firmware. See `07-bluetooth-module.md`.

**Q: Why are there two LED groups?**
A: 4 Track LEDs on PWM2 (P0.29, P0.26, P1.15, P1.14) and 4 Play LEDs on PWM3 (P0.01, P1.12, P0.00, P1.13). Track LEDs show stem state; Play LEDs animate playback. Note the DTS and TimK's pin header label them in opposite orders — see `03-pcb-and-schematic.md` Discrepancies.

**Q: How does the button matrix work?**
A: Two analog "ladder" ADC channels read multi-button combinations (Track + Play on AIN0; vol/transport on AIN1), and one direct GPIO reads the function button (P0.27). The Track 1 + Track 4 bootloader combo works because the ladder has 8 threshold ranges per button to encode multi-press states. See `02-hardware-overview.md` "Button matrices".

**Q: What's the I²S clock setup?**
A: nRF is **master**. BCLK = P0.12 at 3.072 MHz (= 48 kHz × 64). LRCK = P0.11. SDOUT = P1.09. Reference clock is a 3.072 MHz external oscillator enabled via P0.13. See `02-hardware-overview.md`.

**Q: How is the flash partitioned for custom firmware?**
A: Bootloader 0x00000000–0x00020000 (128 KB), app slot0 0x00020000–0x000ff000 (892 KB), Zephyr settings storage 0x000ff000–0x00100000 (4 KB). [code: `assets/sp1-midi-2026-05-13/.../stem_player.dts`]

**Q: Are NFC pins used?**
A: They are **repurposed as GPIO** via `nfct-pins-as-gpios;` in the DTS. P0.09 → TAS2505 reset; P0.10 → BT module reset.

**Q: Where does the README disagree with the DTS?**
A: The `assets/sp1-midi-2026-05-13/README.md` lists the BQ24232 ISET on P0.19, but the DTS and TimK's pin header both say P1.00. **Trust P1.00.** See `02-hardware-overview.md` Discrepancy section.

**Q: How much headroom does custom firmware have?**
A: `sp1-midi` bare BSP uses ~168 KB flash and ~43 KB RAM. The full stock firmware uses ~264 KB flash and ~219 KB RAM. Plenty of room. [code: `assets/sp1-midi-2026-05-13/README.md`]
