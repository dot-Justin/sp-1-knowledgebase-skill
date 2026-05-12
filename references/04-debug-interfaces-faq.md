# FAQ — Debug Interfaces

**Q: How do I enter the bootloader?**
A: Press and hold **Track 1 + Track 4**, then plug in USB-C. You have **60 seconds** before the device boots normally. [Lines #556, B_E_N]

**Q: Is the power button part of the bootloader combo?**
A: No. An earlier version of B_E_N's instructions included it, but the corrected post removed it. See `corrections.md`.

**Q: Can the bootloader read out my firmware?**
A: **No.** *"you cannot"* [Discord #firmware, ericlewis, 2026-05-08]. The bootloader only exposes flash-write, not flash-read. To extract firmware you need a glitch attack (`14-approtect-glitch-attack.md`) or the LED bit-bang trick (`17-led-bitbang-dump.md`).

**Q: What's `solderless.engineering`?**
A: A browser-based firmware updater using Web Serial. Wraps the bootloader CDC protocol with a UI. Built by TimK + loksi + tunelight + keebstudios. Currently **offline** for an update as of 2026-05-09. See `04-debug-interfaces.md`.

**Q: How do I get a serial console at runtime?**
A: Build `sp1-midi` with `-DSNIPPET=cdc-acm-console` and the device exposes a Zephyr shell over USB CDC ACM after boot. See `04-debug-interfaces.md` section 3.

**Q: Do I need to open the device to develop custom firmware?**
A: For **flashing** finished firmware, no — solderless.engineering or the Track 1+4 trigger work without opening. For **active development** with breakpoints / source-level debug, yes — you need to solder an SWD header.

**Q: Where's the SWD header location?**
A: Not yet publicly documented (synthesis date 2026-05-11). TimK soldered one and promised to add documentation to `github.com/timknapen/SP-1-dev/wiki`. Ask in Discord if the wiki hasn't been updated.

**Q: Will SWD let me read the factory firmware?**
A: No. The nRF52840 has APPROTECT enabled from the factory. SWD can erase + reprogram but cannot read. To read, you must clear APPROTECT via a glitch attack first — see `14-approtect-glitch-attack.md`.

**Q: How does the LED bit-bang dump work?**
A: Flash a tiny custom firmware (1 page ≈ 4 KB) via the bootloader. That firmware reads the rest of flash and encodes bytes as LED on/off patterns. Capture with a camera, decode with a script. zee_33 used this in 2026-05-08 [Discord #firmware]. See `17-led-bitbang-dump.md`.

**Q: Can I dump the eMMC without desoldering?**
A: Yes — TimK says DAT0–DAT3, CLK, and CMD are broken out to test points [Discord #hardware, 2026-05-07]. Use clip leads or pogo pins. Exact pad locations pending wiki update. See `08-emmc-storage.md`.

**Q: What does the desolder-the-eMMC method look like?**
A: Hot-air desolder at 380–420 °C on a 175 °C preheater, then solder the chip onto an unpopulated USB-to-eMMC adapter (cheap on AliExpress). No reballing needed — fishdog_ built up solder on board + chip pads. The adapter enumerates as USB block device; `dd` it. [Discord #hardware, fishdog_, 2026-05-10 → 2026-05-11]

**Q: My custom firmware bricked the device — how do I recover?**
A: 1) Cold-start with Track 1 + Track 4 + USB-C and re-flash via solderless or your own tool. 2) If that doesn't enumerate, disconnect the battery, wait, reconnect, then try again. 3) If neither works, you may need SWD — meaning you have to open the device.

**Q: What protocol does the bootloader speak over CDC?**
A: A small set of single-byte opcodes (0x52, 0x70, 0x50, 0x37, 0x39, 0x43, 0x51, 0x54, 0x58, 0x66). See `15-bootloader-protocol.md` and `16-usb-upload-protocol.md`.
