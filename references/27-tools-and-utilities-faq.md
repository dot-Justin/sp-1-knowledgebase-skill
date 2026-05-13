# FAQ — Tools and Utilities

**Q: What's the canonical hardware docs repo?**
A: `github.com/timknapen/SP-1-dev` + its GitHub Wiki. Pin map, bootloader documentation, board overview.

**Q: What's the public custom firmware BSP?**
A: `github.com/ericlewis/sp1-midi`. The only buildable public starting point. See `19-sp1-midi-bsp.md`.

**Q: Where do I find `assets/audiothingies-2026-05-09.zip` / `assets/storagethingies-2026-05-09.zip`?**
A: Shared by ericlewis in the Discord #firmware channel on 2026-05-09. Not in a public repo (yet). Ask ericlewis if you need them.

**Q: Where's the Lines thread archive?**
A: `sp-1.dotjust.in` (web frontend) or `github.com/dot-Justin/TE-SP-1-lines-thread-archive` (source). Includes agent-friendly summaries for AI use.

**Q: What's `solderless.engineering`?**
A: Web-based firmware updater. **Offline for an update** as of 2026-05-09 [Discord #news]. Will return. Built by TimK + loksi + tunelight + keebstudios.

**Q: What's `libpo32`?**
A: ericlewis's library for the **PO-32 (different TE product)**. Not SP-1 related. Don't confuse the two. See `hallucination-watchlist.md`.

**Q: What's `theunflappable`'s tool?**
A: `test_bootloader.py` — Python implementation of the SP-1 bootloader protocol. Shared in Discord #firmware 2026-05-08. Implements firmware flashing; not album upload.

**Q: What stem-separation tool should I use?**
A: emvee1968 uses **HT Demucs FT** [Discord #firmware, 2026-05-08 23:54]. Other options: Demucs, Spleeter, MusicNN. All publicly available.

**Q: What disassembler do I use for nRF52840 RE?**
A: **Ghidra** with the **nRF52840 SVD** as register overlay. See `22-disassembly-ghidra.md`.

**Q: What BeagleBone glitcher is used for the APPROTECT bypass?**
A: `github.com/resinbeard/beaglebone-black-glitcher`. murray's tool. Public.

**Q: What about a public fast USB uploader?**
A: **Doesn't exist yet.** moecal1947's slow tool works (~4.5 days for 311 MB). ericlewis says ~4 MB/s theoretical max is achievable but the public implementation isn't there. See `16-usb-upload-protocol.md`.

**Q: What about a public album image construction tool?**
A: **Doesn't exist yet.** emvee1968 and virtualflannel_46386 have internal pipelines but neither has released. Open question for community contributors.

**Q: What about a public album image extractor?**
A: **Doesn't exist yet.** Could be built from `assets/storagethingies-2026-05-09/DiskManager`'s decoder + a custom host wrapper. Good contributor target.

**Q: What hardware do I need for eMMC desolder?**
A: Hot air rework station + 175 °C preheater + unpopulated USB-to-eMMC adapter PCB. See `24-emmc-direct-extraction.md`.

**Q: What hardware do I need for the LED bit-bang dump?**
A: Just the SP-1 + a smartphone with slow-motion video + a decoding script. See `17-led-bitbang-dump.md`.

**Q: How do I find the latest custom firmwares?**
A: Watch Discord #firmware and #news. Specifically follow emvee1968 (BT + gate effect), virtualflannel_46386 (custom OS), TimK (full BSP), ericlewis (BSP refinement).

**Q: Where do I report bugs in the public tools?**
A: GitHub issues on the relevant repo: `timknapen/SP-1-dev`, `ericlewis/sp1-midi`, `dot-Justin/TE-SP-1-lines-thread-archive`. For private tools (audiothingies, theunflappable's, etc.): ask the author in Discord.
