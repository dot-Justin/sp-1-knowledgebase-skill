# FAQ — Bootloader Protocol

**Q: How do I enter the bootloader?**
A: Hold **Track 1 + Track 4** while plugging in USB-C. The device enumerates as USB CDC ACM and accepts commands for **60 seconds**. See `15-bootloader-protocol.md`.

**Q: Is the power button part of the trigger?**
A: **No.** An earlier version of B_E_N's instructions included it but the corrected post removed it. See `corrections.md`.

**Q: What's the canonical reference?**
A: TimK's GitHub wiki: `github.com/timknapen/SP-1-dev/wiki/Bootloader`. Verify current.

**Q: Can the bootloader read flash?**
A: **No.** Write-only. *"you cannot"* [Discord #firmware, ericlewis, 2026-05-08]. To dump firmware use the LED bit-bang trick or a glitch attack.

**Q: What are the opcodes?**
A: Single-byte commands: `0x52` (version), `0x54` (device ID), `0x58` (album title), `0x70` (set GPREGRET), `0x50` (reboot), `0x37` (stop playback), `0x39` (data packet), `0x43` (write counter), `0x51` (system reset), `0x66` (album valid). See `15-bootloader-protocol.md`.

**Q: What's parser 1 vs parser 2?**
A: The bootloader has two modes selected by GPREGRET. Parser 1 (default) does firmware flashing. Parser 2 (entered via `0x70 0x01` + reset) does album upload. See `15-bootloader-protocol.md`.

**Q: What's the GPREGRET magic value?**
A: `0x1A96`. Set via the `0x70 0x01` command pair, then trigger reboot. GPREGRET survives soft reset, so the next boot reads it and selects parser 2.

**Q: How long is the 60-second window?**
A: 60 seconds from USB enumeration. If you send any command (even `0x52` version check), the timer probably resets — but verify against TimK's wiki. If no activity, the device boots into the app.

**Q: What happens if my custom firmware doesn't work?**
A: Re-trigger Track 1 + Track 4 + USB-C from a cold start. Bootloader is in flash regardless of whether your app firmware crashed.

**Q: What if my custom firmware corrupts the bootloader?**
A: SWD recovery needed. **Don't write to the bootloader region (0x00000000–0x00020000)** from your custom firmware. Keep your code in slot 0 (`0x00020000`+).

**Q: How do I write a host client for this protocol?**
A: Three options: 1) wait for `solderless.engineering` to come back online; 2) ericlewis pointed at `theunflappable`'s `test_bootloader.py` (Discord #firmware 2026-05-08); 3) write your own against the opcode set documented in `15-bootloader-protocol.md`.

**Q: What's the relationship between `solderless.engineering` and the bootloader?**
A: solderless is a JavaScript Web Serial frontend to this same protocol. Same commands, just packaged in a browser UI.

**Q: Why is solderless offline?**
A: **It's not offline anymore.** As of 2026-05-18 solderless.engineering came back online with a multi-app launcher rewrite — 4 apps (stem loader, firmware utility, device info, spoom1). The pre-rewrite offline period was 2026-05-09 through ~2026-05-17. Local mirror at `assets/solderless-2026-05-18/`. See `27-tools-and-utilities.md` for the new architecture.

**Q: Is there a browser-based firmware flasher?**
A: **Yes** — solderless.engineering's "firmware utility" tab. Workflow: T1+T4-boot the device, click connect, switch to firmware utility tab, drag your `.bin`, click flash. Local archive at `assets/solderless-2026-05-18/utility/`. The dedicated app cleanly separates flash from album upload, making `utility/js/firmware.js` the cleanest reference for the flash flow alone.

**Q: Can I flash a partial firmware (e.g., just the data section)?**
A: Probably not without writing your own bootloader client that supports partial-flash commands. The published protocol covers full-app-image flash.
