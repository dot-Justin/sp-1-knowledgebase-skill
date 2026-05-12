# FAQ — eMMC Direct Extraction

**Q: Do I need to remove the eMMC chip to read it?**
A: **No.** TimK says DAT0–DAT3, CLK, and CMD are broken out to test points. You can read in-circuit with clip leads. See `24-emmc-direct-extraction.md`.

**Q: Where exactly are the test points?**
A: **Not yet publicly documented** as of 2026-05-11. TimK promised to add them to `github.com/timknapen/SP-1-dev/wiki`. Check the wiki or ask in Discord.

**Q: What's the eMMC chip part number?**
A: **Toshiba THGBMNG5D1LBAIL**, 4 GB, eMMC 5.0.

**Q: How fast can I read via test points?**
A: Depends on your reader. eMMC 5.0 supports up to 200 MB/s in 8-bit HS400 mode. Most readers use 4-bit and lower clocks; ~50 MB/s realistic.

**Q: How do I desolder the eMMC?**
A: Hot air at **380-420 °C**, on a **175 °C preheater**, lift with tweezers. fishdog_'s reference [Discord #hardware, 2026-05-11 19:08].

**Q: Do I need to reball?**
A: No — fishdog_ built up solder on board + adapter pads instead [Discord #hardware, fishdog_, 2026-05-10 19:25]. Saves the reball station cost.

**Q: What USB adapter do I use?**
A: Unpopulated USB-to-eMMC adapter PCBs from AliExpress / Amazon. esko8444 linked an AliExpress alternative on 2026-05-11 [Discord #hardware]. Search "eMMC USB adapter unpopulated."

**Q: Can I put the chip back after desoldering?**
A: Technically yes, but the thermal cycling stresses the chip. Most fishdog-style users keep the chip on the adapter as their permanent setup.

**Q: What does the SP-1 do without the eMMC?**
A: It won't work — the firmware expects the eMMC to be present. The device may hang at boot, or display an error if it has one.

**Q: Is the eMMC encrypted?**
A: **No.** *"Not encrypted just formatted strangely"* [Discord #hardware, tkt1000, 2026-05-07]. See `corrections.md`.

**Q: How big is the SP-1's album?**
A: ~311 MB for a typical album. The full 4 GB eMMC has the album in the first portion; the rest is unused on stock firmware.

**Q: Can I dump the album for archival?**
A: Yes via any of the methods. **Don't distribute Kanye stems** — they're copyrighted.

**Q: Can I write a new album by removing the chip + dd'ing?**
A: Yes. Once on a USB adapter: `dd if=new_album.img of=/dev/sdX bs=4M`. Then resolder back into the SP-1 (or leave on adapter for use as a "stem source" without playback).

**Q: What about CRC / data integrity?**
A: eMMC has CRC at the protocol level. Read should never silently corrupt data unless the chip itself is failing. Application-level there's no checksum in the album format.

**Q: How does this compare to LED bit-bang dumping?**
A: LED bit-bang dumps **nRF52840 firmware** (no opening). eMMC direct extraction dumps **the audio data on eMMC** (requires opening). Different goals.

**Q: Can a commercial eMMC reader do this in-circuit (ISP mode)?**
A: Yes — UFI Box, EasyJTAG, Medusa Pro etc. all support ISP-mode reads via test point access. Map their pin assignments to SP-1's test points.
