# FAQ — LED Bit-Bang Dump

**Q: What is the LED bit-bang dump?**
A: A firmware-extraction technique that uses the SP-1's LEDs to encode flash bytes as visual patterns. Capture with a camera, decode. Avoids both glitch attacks and opening the device. See `17-led-bitbang-dump.md`.

**Q: Who came up with it?**
A: **zee_33**, demonstrated 2026-05-08 in Discord #firmware [zee_33, 2026-05-08 17:33 + 19:04 + 19:08].

**Q: Why does this work despite APPROTECT?**
A: APPROTECT blocks **external** SWD reads. It does not block code running on the chip from reading flash. Once you flash a custom firmware, it can read everything; you just need a side channel (LEDs) to exfiltrate.

**Q: How long does it take?**
A: Slow. Depends on encoding scheme (binary vs grayscale) and camera frame rate. For a 128 KB bootloader dump, expect hours of capture time + decode overhead.

**Q: How much of the existing firmware do I lose?**
A: As much as your dump firmware overwrites. The bootloader region (`0x00000000`–`0x00020000`, 128 KB) is preserved. Slot 0 is overwritten — you lose the TE app firmware unless you have a separate copy (e.g., murray's January 2025 dump).

**Q: What's the smallest practical dump firmware?**
A: zee_33 used **~1 page = 4 KB**. The smaller the better — minimizes what you overwrite. Bare-bones Zephyr + LED PWM + flash read + minimal logic fits comfortably.

**Q: What's the LED encoding scheme?**
A: zee_33 didn't publish the exact scheme. Likely binary (each LED on/off = 1 bit, 8 LEDs = 1 byte per frame). Grayscale PWM levels would encode more bits per frame but reduce reliability.

**Q: Can I dump RAM with this technique?**
A: Sure — just modify the dump firmware to read RAM addresses instead of flash. Same encoding/capture pipeline.

**Q: What's the decode pipeline?**
A: Python with OpenCV: read video frames, sample LED pixel regions, threshold/quantize to recover bits, assemble bytes, write to output file. The hardest part is finding the LED positions reliably.

**Q: Do I need slow-motion video?**
A: Recommended — gives more frames per LED transition for error correction. iPhone slow-mo at 240 fps works.

**Q: Where's zee_33's code?**
A: Not publicly published as of synthesis date. Ask in Discord if you want to reproduce.

**Q: Why use this instead of a glitch attack?**
A: No exotic hardware needed (vs BeagleBone + MOSFET + scope). No risk of brick (vs glitch attempts that can damage the chip). Works in ~hours instead of ~weeks. Trade-off is slower per-byte but lower-skill / lower-cost.

**Q: Why use this instead of SWD?**
A: SWD requires opening the device and soldering a header. LED bit-bang requires neither.

**Q: Is the technique unique to SP-1?**
A: No — it's a general approach for any device with (a) a firmware-flashing path that doesn't require physical opening, and (b) any user-visible output channel (LEDs, screen, audio, radio). It just happens to compose elegantly on the SP-1 because both pieces exist and are well-documented.

**Q: What can I extract that murray's glitch dump doesn't already have?**
A: Device-specific data: UICR contents (factory-programmed unique IDs), bootloader state, any per-device keys (if any exist; SP-1 likely has none). Also: lets you verify your specific SP-1's bootloader matches the canonical version, useful for confirming you haven't accidentally received a hardware variant with different firmware.

**Q: Is this a real security risk for nRF52840 devices?**
A: Conceptually yes — but only for devices that intentionally allow user firmware flashing. The SP-1 was always designed to accept firmware updates via its bootloader; LED bit-bang doesn't violate any assumed security boundary. For devices that should *not* allow user firmware, the lesson is: don't expose a flashing path to untrusted users.
