# FAQ — Disassembly and Ghidra

**Q: What's the disassembly toolchain?**
A: **Ghidra** with the **nRF52840 SVD** as register-overlay. Standard Cortex-M reverse engineering setup. See `22-disassembly-ghidra.md`.

**Q: Who led the disassembly?**
A: **Duloz** in early-to-mid 2025 [Lines #205–240]. Multiple others contributed.

**Q: Where's the firmware binary?**
A: Privately held in the community since murray's January 2025 dump. Not redistributed publicly out of respect for TE's IP. Ask community members directly if you need it for research.

**Q: How do I set up Ghidra for nRF52840?**
A: Install Ghidra → import binary as raw → set language to `ARM:LE:32:Cortex-M` → image base `0x00000000` → run auto-analysis → load SVD via `SVD-Loader-Ghidra` plugin (or import manually).

**Q: What is APPROTECT and how was it bypassed?**
A: Hardware bit on nRF52840 that blocks SWD read access. Bypassed via voltage glitch attack by murray (attempt #8,504). See `14-approtect-glitch-attack.md`.

**Q: What was learned from the disassembly?**
A: Bootloader structure (Track 1+4 + 60-second window), audio engine architecture (decoded into `audiothingies`), eMMC driver (decoded into `storagethingies`), effects implementations, USB CDC protocol opcodes.

**Q: What's the privilege-mode finding?**
A: Stock firmware uses Cortex-M unprivileged execution for some paths [Duloz, Lines #220–240]. Suggests TE partitioned trust between bootloader (privileged) and app (less-privileged).

**Q: What did murray initially worry about?**
A: That the dump was incomplete — the I²S peripheral only had TASKS_START and TASKS_STOP assigned [Lines #317]. Turned out the dump was complete; TE just uses minimal I²S register setup.

**Q: Are specific DSP constants verifiable in the disassembly?**
A: Yes. The smoothing coefficient `0.02f = 0x3CA3D70A`, the distortion pre-gain `16.0`, the polynomial waveshape `x − 0.5·x³`, the 8192-sample delay buffer — all identifiable in the binary.

**Q: What's still not fully disassembled?**
A: UI / menu state machine, Bluetooth HCI interactions, complete power-down sequence, the specific filter shapes implemented by the TAS2505 DSP coefficient table. See `22-disassembly-ghidra.md` "What hasn't been fully disassembled".

**Q: How does `audiothingies` differ from the stock binary?**
A: Clean-room reimplementation, equivalent in behavior but not byte-for-byte. The intent matches; the implementation is independent C++17 code. ericlewis writes that he could replicate "VERY tight to repro all their stuff" but the BSP isn't clean enough to release yet [Discord #firmware, 2026-05-09 00:10].

**Q: Can I dump my own SP-1's firmware?**
A: Via the LED bit-bang technique (`17-led-bitbang-dump.md`) without opening the device, or via the voltage glitch attack (`14-approtect-glitch-attack.md`) with hardware modification. The latter is harder but reads the full firmware including the app slot.

**Q: What's the bootloader's image base?**
A: `0x00000000`. The bootloader fills the first 128 KB. App firmware lives at `0x00020000`. See `08-emmc-storage.md` (memory map section).

**Q: Why was Ghidra chosen over IDA or Radare?**
A: Free, supports Cortex-M, has community-developed SVD loaders. Could also use radare2 (similar capabilities). IDA Pro requires a paid license.

**Q: Where can I learn more about nRF52 reverse engineering generally?**
A: limitedresults' "nRF52 Debug Resurrection" writeup is the foundational reference for APPROTECT bypass. The Nordic nRF52840 product spec is the foundational reference for peripheral details.

**Q: Can disassembly find new vulnerabilities?**
A: In principle yes — the bootloader's command parser, the album header parser, etc. could have memory corruption bugs exploitable from USB. No known vulnerabilities found yet. Defense-in-depth: don't feed untrusted album images to a device you care about.
