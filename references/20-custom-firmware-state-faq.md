# FAQ — Custom Firmware State

**Q: How do I flash a custom firmware today, without writing my own flasher?**
A: Browser + USB-C cable. As of 2026-05-18 solderless.engineering's "firmware utility" tab is a public, browser-based custom-firmware flasher. T1+T4-boot the device, click connect in the launcher, switch to the firmware utility tab, drag your `.bin` file, click flash. No Python toolchain, no DFU utility, no SWD probe needed. Local archive at `assets/solderless-2026-05-18/utility/`. Practical size ceiling: **892 KB** (the app slot between `FLASH_START = 0x20000` and `FLASH_END = 0xFF000`).

**Q: What public custom firmware exists today?**
A: Only **`ericlewis/sp1-midi`** (a MIDI controller, not a stem player). TimK, emvee1968, and virtualflannel have unreleased work. See `20-custom-firmware-state.md`.

**Q: When will emvee1968's firmware be released?**
A: emvee said: *"awaiting 2 more sp-1s to dev with which I should get next week"* on 2026-05-08. So week of 2026-05-12 or later. Awaiting confirmation.

**Q: When will virtualflannel's custom OS be released?**
A: When `solderless.engineering` is back online — virtualflannel doesn't want to release without users being able to flash easily. **Update 2026-05-18:** solderless.engineering is back online with a dedicated firmware utility app, so this blocker is now removed; awaiting virtualflannel's release.

**Q: When will ericlewis's "proper BSP" with audio be released?**
A: No public ETA. ericlewis said: *"im in the middle of a proper bsp for zephyr for everyone but quite busy"* [Discord #firmware, 2026-05-09 00:20].

**Q: Is the stock TE firmware available to redistribute?**
A: **No.** The community treats it as TE's intellectual property. It's been extracted (murray 2025-01-25) but isn't shared in binary form publicly.

**Q: What does TimK's private firmware do?**
A: tkt1000 says "fully functional" with multiple variants tested via the update utility. He hasn't published yet because "working on something more important now that we want to release first" [Discord #hardware, 2026-05-08 14:51]. Speculation: this is either the solderless utility update or a complete BSP with audio.

**Q: What's in `audiothingies` and `storagethingies`?**
A: ericlewis's C++17 clean-room reimplementation of the stock audio engine + storage driver. Originally shared as Discord file attachments 2026-05-09. **Now bundled in this skill** at `assets/audiothingies-2026-05-09/` and `assets/storagethingies-2026-05-09/`. Will likely also be incorporated into ericlewis's eventual public BSP.

**Q: Can I get `audiothingies` / `storagethingies`?**
A: Read them right here — bundled in this skill at `assets/audiothingies-2026-05-09/` and `assets/storagethingies-2026-05-09/`. The originals were Discord attachments from ericlewis, shared to help moecal1947 understand the audio engine; they aren't intentionally hidden, just not in a public ericlewis-owned repo yet.

**Q: What can virtualflannel's custom OS do that stock can't?**
A: Beat repeat, bit crusher, send delay with performance feedback, filter with performance modulation, pitch ±12 cents. Stock has filter/distortion/gate/echo; virtualflannel adds beat repeat + bit crusher.

**Q: What can emvee's firmware do that stock can't?**
A: Currently nothing — emvee's is replicating stock functionality (basic playback + gate). Bluetooth pairing is the main novelty (working in custom firmware but not formally demonstrated working in stock either).

**Q: Has anyone implemented full stock parity?**
A: Not publicly. emvee has gate; virtualflannel has different (custom) effects; no public firmware has all 4 stock effects + tape FX + Bluetooth fully working yet.

**Q: What about `libpo32`?**
A: ericlewis's library for a **different TE product** (PO-32 acoustic transfers). Not directly relevant to SP-1 firmware, but useful as reference for the kind of work ericlewis does. See `26-community-and-authority.md`.

**Q: Can I do custom stems without custom firmware?**
A: Yes. Custom stems play on stock TE firmware if formatted correctly. See `21-original-firmware-stems.md`.

**Q: What's the safest custom firmware to flash right now?**
A: `ericlewis/sp1-midi`. It's public, well-documented, and recoverable (Track1+4 + USB-C still works to re-flash anything else). Just remember it makes your SP-1 into a MIDI controller — not a stem player.

**Q: How long until a public audio-playback firmware?**
A: Probably weeks to a couple of months. Multiple parallel efforts (ericlewis's BSP, emvee's, virtualflannel's). At least one is likely to ship publicly by mid-2026.
