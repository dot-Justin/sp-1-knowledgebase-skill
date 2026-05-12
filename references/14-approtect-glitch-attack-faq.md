# FAQ — APPROTECT Glitch Attack

**Q: What is APPROTECT?**
A: A hardware bit on the nRF52840 that blocks SWD-based read access to firmware. SP-1 ships with it enabled. See `14-approtect-glitch-attack.md`.

**Q: Was APPROTECT bypassed on the SP-1?**
A: **Yes.** murray succeeded on attempt #8,504 of a voltage glitch attack on 2025-01-25, extracting the full firmware.

**Q: Why is the SP-1's APPROTECT glitch-able?**
A: It uses nRF52840 silicon from approximately 2019–2020 (pre-fix). The limitedresults voltage-glitch technique (published in 2020) applies. Newer Nordic silicon has hardened APPROTECT.

**Q: What hardware did murray use?**
A: A **BeagleBone Black** with its **PRU (Programmable Real-time Unit)** generating precise glitch pulse timing (~5 ns granularity at 200 MHz). MOSFET shorting DEC1 (or DEC4) decoupling capacitor. SWD probe for success detection. See `github.com/resinbeard/beaglebone-black-glitcher`.

**Q: Why 8,504 attempts?**
A: Glitch attacks have low success rates per attempt — typically 1-in-10,000. You sweep glitch parameters (timing offset, pulse width) and run for hours. 8,504 reflects the actual count from murray's logs.

**Q: Did anyone help murray scale up?**
A: Yes. **elew** joined with 5 additional SP-1 units running parallel glitch attempts, multiplying throughput by 5x.

**Q: Do I need to do the glitch to develop custom firmware?**
A: **No.** Use the Track 1 + Track 4 + USB-C bootloader trigger to flash custom firmware. APPROTECT is irrelevant for flashing slot 0. See `15-bootloader-protocol.md`.

**Q: Can I read the original TE firmware off my own SP-1 without glitching?**
A: Only via the **LED bit-bang dump** technique (slow, only useful for bootloader region) or by **glitching yourself**. See `17-led-bitbang-dump.md`.

**Q: Are the Kanye album stems part of the firmware dump?**
A: **No.** Album audio lives on the eMMC (separate chip). Firmware = TE's code. Stems = audio data. Reading the eMMC requires no APPROTECT bypass; reading TE firmware does. Community doesn't distribute either.

**Q: Once glitched, can the SP-1 be re-protected?**
A: Once APPROTECT is bypassed for SWD access, you can erase and reprogram. APPROTECT itself can be re-enabled by setting the UICR bit, but you'd just lose your debug access again. Most developers leave it disabled during development.

**Q: What firmware analysis tools were used?**
A: **Ghidra** with the nRF52840 SVD register file as overlay. Duloz and others led the disassembly. See `22-disassembly-ghidra.md`.

**Q: What was learned from the dump?**
A: Bootloader trigger details (Track 1 + Track 4), audio engine structure, eMMC driver design, audio format, effects implementations. Essentially everything the community subsequently built on.

**Q: Is reading public on YouTube / blog posts?**
A: limitedresults' nRF52 writeup is the foundational reference. murray's BeagleBone glitcher code is on GitHub. Specific SP-1 success details are scattered through Lines posts ~#100–200, #317.

**Q: Could I do this attack on another nRF52840 device today?**
A: Technically yes if the silicon is vulnerable. Don't do it on devices you don't own; respect chip vendors' security work; understand that the technique is well-known and counter-measures exist in newer silicon.
