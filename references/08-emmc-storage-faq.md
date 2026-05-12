# FAQ — eMMC Storage

**Q: What eMMC chip is in the SP-1?**
A: Toshiba **THGBMNG5D1LBAIL**, 4 GB, eMMC 5.0. Operates in 1-bit mode at **32 MHz** without High Speed mode enabled. See `08-emmc-storage.md`.

**Q: Is the eMMC content encrypted?**
A: **No.** *"Not encrypted just formatted strangely"* [Discord #hardware, tkt1000, 2026-05-07]. See `corrections.md`.

**Q: How does the nRF talk to the eMMC?**
A: Hybrid: GPIO bit-banging for CMD phase (P0.08), SPIM3 SCK/MISO for data reads (P0.06 CLK, P0.07 DAT0). PWM3 borrowed for burst timing. See `08-emmc-storage.md`.

**Q: How big is a TE sector?**
A: **8192 bytes** = 16 native eMMC blocks. Contains 4 TE blocks of 2048 bytes each. Contains **340 audio frames** of 24 bytes each plus ~32 bytes of side-band data.

**Q: How big is a frame?**
A: **24 bytes**. 4 stems × 6 bytes per stem. Each stem stores 2 channels × 24-bit samples interleaved in a non-obvious byte order. See `09-audio-format-spec.md`.

**Q: What's the 0, 2, 1, 3 block order?**
A: The physical disk order of the 4 TE blocks per sector. Enables tape FF/RW by reading less of each sector at high speeds. See `11-block-interleaving-tape-fx.md`.

**Q: How many songs per album?**
A: Up to **16**. Album header stores `SongInfo` with start_sector + length_sectors + 65-byte artist + 65-byte title strings.

**Q: What's the typical album size?**
A: ~311 MB for a typical album [Discord #general, moecal1947, 2026-05-09 09:33] = 37,993 TE sectors = 607,888 native eMMC blocks.

**Q: How does prefetch work?**
A: 10 audio slots, each holds 1 sector (8192 bytes + side data). State machine: Empty → Queued → Reading → Ready. Distance-hint based eviction. Two slots are "render protected" (the ones the audio engine is currently rendering from). See `08-emmc-storage.md`.

**Q: How do FF/RW reduce eMMC bandwidth?**
A: At 2.5x speed, only the first half (8 native blocks = TE blocks 0+2) is read; at 4x+, only the first quarter (4 native blocks = TE block 0). Frames are duplicated to fill gaps. See `11-block-interleaving-tape-fx.md`.

**Q: Can the public reference code write to the eMMC?**
A: **No.** Read-only. moecal1947 has a slow Python USB uploader (~0.75 KB/s) but no proper CMD24/CMD25 write driver exists publicly yet. See `known-unknowns.md`.

**Q: Can I read the eMMC from outside the device?**
A: Yes — via test points (TimK said DAT0–DAT3 + CLK + CMD are broken out; exact pads pending wiki update), or by desoldering + USB-to-eMMC adapter (fishdog_'s method, 380–420 °C, no reballing). See `04-debug-interfaces.md` and `24-emmc-direct-extraction.md`.

**Q: Does the eMMC fail at >32 MHz?**
A: Yes, unreliably. TimK's testing shows 32 MHz works without High Speed mode but anything higher is broken. Why is unclear — see `known-unknowns.md`.

**Q: What's `kEmmcBlocksPerSector = 16`?**
A: 16 native 512-byte eMMC blocks per 1 TE sector of 8192 bytes. The eMMC sees 512-byte blocks; the TE layout aggregates 16 of them into a logical 8 KB unit.
