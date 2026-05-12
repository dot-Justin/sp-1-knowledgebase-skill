# FAQ — Overview and History

**Q: What is the SP-1?**
A: A handheld 4-stem audio player prototype designed by Teenage Engineering, originally for Kanye West's *Donda* and *Jesus Is King* albums. Never released to retail. nRF52840 inside. See `01-overview-and-history.md`.

**Q: How many units exist?**
A: ~859 imported to Ye in Sep 2020 per Very Good Touring records; total in the wild is community-estimated at 1000–2000. [Lines #743, maybe]

**Q: When was the firmware first extracted?**
A: 2025-01-25, by murray, attempt #8,504 of a voltage glitch attack on the nRF52840's APPROTECT.

**Q: Who reverse-engineered the PCB?**
A: TimK (Lines / Discord tkt1000 / GitHub timknapen). Full KiCad schematic in early-to-mid 2025.

**Q: Who figured out the audio format?**
A: JoseJX (Lines), with refinements by Galapagoose (MIDI clock decoding) and ericlewis (block-interleaving for tape FX, audio engine internals).

**Q: Why did the Lines forum thread close?**
A: Moderators cited spam volume around copyright/dump requests. Thread is read-only, not deleted. Community moved to Discord 2026-05-06.

**Q: Where is the active discussion now?**
A: A private-invite Discord server, invite link from `github.com/timknapen/SP-1-dev` or other community channels. Not linked from this skill.

**Q: Are there custom firmwares I can use?**
A: As of 2026-05-11: ericlewis's MIDI-controller BSP is public (`github.com/ericlewis/sp1-midi`). emvee1968 and virtualflannel_46386 have demoed custom OSes but **not released them yet**. TimK has private custom firmwares not yet public.

**Q: Can I load custom stems on stock TE firmware?**
A: Yes. TimK confirmed this works without firmware modification — stock firmware doesn't validate content [Lines #799–805]. See `21-original-firmware-stems.md` (Phase 5).

**Q: Who should I trust on which topic?**
A: TimK on firmware / PCB / bootloader; ericlewis on audio engine / storage / DSP; murray on glitch attack; JoseJX on audio format; B_E_N on bootloader trigger. See `01-overview-and-history.md` "Who's who".

**Q: What's `solderless.engineering`?**
A: Web-based firmware updater that uses the Track 1 + Track 4 + USB-C bootloader trigger. Currently offline as of 2026-05-09 for an update. See `working-confirmed.md`.

**Q: Are the original Kanye stems still on the device?**
A: Yes, in the eMMC. Reading/copying them is legally questionable due to album copyright. The community does not distribute these.

**Q: Will my SP-1 brick if I mess up?**
A: Possibly. Custom firmware can put the device in a state where the only recovery is the Track 1 + Track 4 + USB-C trigger (which still works from cold boot if your firmware didn't disable USB enumeration), or in rare cases a battery disconnect to force a reset. See `04-debug-interfaces.md`.
