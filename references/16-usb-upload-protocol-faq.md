# FAQ — USB Upload Protocol

**Q: How do I upload a custom album?**
A: Trigger the bootloader, switch to parser 2 (via GPREGRET = `0x1A96`), send the album as 136-byte `0x39` packets (one per quarter of a native eMMC block), then reset. See `16-usb-upload-protocol.md`.

**Q: What's a `0x39` packet?**
A: 136 bytes total: 4-byte header + 4-byte sector address + 128 bytes of payload data. Four packets fill one 512-byte native eMMC block.

**Q: How many packets for a full album?**
A: For a 311 MB album: 607,888 native blocks × 4 packets per block = **2,431,552 packets**.

**Q: How fast can this go?**
A: ericlewis says **~4 MB/s theoretical**. moecal1947's working implementation achieves **~0.75 KB/s** — bottleneck is USB control-transfer overhead. A proper bulk-endpoint implementation should hit ericlewis's number.

**Q: How long for moecal1947's current uploader?**
A: ~4.5 days for 311 MB. Safe and resumable; just slow.

**Q: What's the exact byte format of the 4-byte header?**
A: **Not publicly documented.** moecal1947 asked ericlewis directly [Discord #general, 2026-05-09 09:47]; ericlewis acknowledged he has it but didn't share. See `known-unknowns.md`.

**Q: How are packets ACKed?**
A: Unknown — probably one ACK byte per packet (e.g., `0x06` for ACK, something else for NAK). Not publicly documented.

**Q: How do I get the missing framing details?**
A: 1) Wait for solderless.engineering to come back online; 2) ask ericlewis directly in Discord; 3) USB-monitor the solderless client when it's running. See `16-usb-upload-protocol.md` "How to make progress".

**Q: Is the sector address relative to the album or absolute on eMMC?**
A: Likely **native 512-byte block address** on the eMMC, based on the packet size matching native block granularity. The 4-packet/native-block math fits this interpretation. Verify with ericlewis if it matters.

**Q: Why are packets exactly 128 bytes payload?**
A: USB Full Speed packets are 64 bytes; 2 × 64 = 128. Efficient packing for the USB transport, plus an integer number of packets per native eMMC block (512 / 128 = 4).

**Q: Can I send packets out-of-order?**
A: Unknown. Probably not — the firmware likely buffers a native block worth of data before writing. If you send sector address 5 then sector address 3, the second probably either errors or causes a buffer flush. Verify before assuming.

**Q: Is the upload resumable across power-cycles?**
A: moecal1947's implementation is resumable. Whether the bootloader-side natively supports resume (e.g., "I already wrote up to sector X, continue from there") is unknown. moecal1947's resume works by verifying writes per-block and only resending missing blocks.

**Q: Will uploading a bad album brick the device?**
A: Probably not bootloader-bricking — the bootloader region isn't written by `0x39`. But the album content can be corrupt, causing audio playback to behave erratically. Recovery: re-upload a known-good album.

**Q: Can I write to the bootloader region via 0x39?**
A: Hopefully not — the bootloader should validate sector addresses against the eMMC address range. But "should" isn't "does"; don't experiment with out-of-range addresses.

**Q: Does the upload affect the slot-0 app firmware?**
A: No. Album upload targets the eMMC; app firmware lives in the nRF's internal flash. Independent operations.

**Q: Where's the album image format spec?**
A: See `09-audio-format-spec.md` for byte layout, `11-block-interleaving-tape-fx.md` for sector layout, `10-midi-timing-encoding.md` for timing side-data. Combine all three for a complete encoder.
