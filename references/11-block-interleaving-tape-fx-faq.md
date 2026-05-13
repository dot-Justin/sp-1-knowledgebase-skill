# FAQ — Block Interleaving and Tape FX

**Q: What's the 0/2/1/3 thing?**
A: The physical order of the 4 TE blocks (2048 bytes each) within an 8192-byte sector. Designed so reading **less of each sector** at high FF/RW speeds gets a temporally correct sample of the audio without exceeding eMMC bandwidth. See `11-block-interleaving-tape-fx.md`.

**Q: Why is the eMMC bandwidth a problem?**
A: At 4x FF the audio data rate would be 4.6 MB/s — above the 32 MHz 1-bit eMMC's ~4 MB/s ceiling. Reading less per sector keeps the effective bandwidth at 1.0x while still producing fast-forward audio.

**Q: How does the engine pick `read_blocks`?**
A: 
- speed < 2.5x → `read_blocks=16` (full sector)
- 2.5x ≤ speed < 4x → `read_blocks=8` (half sector, blocks 0+2)
- speed ≥ 4x → `read_blocks=4` (quarter sector, block 0 only)
[code: `assets/audiothingies-2026-05-09/StockRuntimeMixer::read_blocks_for_speed()`]

**Q: How are frames distributed across blocks?**
A:
- Block 0 → frames where `index % 4 == 0` (0, 4, 8, 12, ...)
- Block 1 → frames where `index % 4 == 2` (2, 6, 10, 14, ...)
- Block 2 → frames where `index % 4 == 1` (1, 5, 9, 13, ...)
- Block 3 → frames where `index % 4 == 3` (3, 7, 11, 15, ...)

**Q: How does frame duplication work?**
A: At 2.5x (half-sector reads), the engine gets every other frame (0, 2, 4, ...) and duplicates each: `[F0, F0, F2, F2, F4, F4, ...]`. At 4x, every 4th frame is quadruplicated. Acts like a 24 kHz or 12 kHz sample-rate downsampling on the fly.

**Q: Does the CPU do any extra work for FF/RW?**
A: **No.** That's the cleverness — the data layout produces the right frames at the right time without runtime computation.

**Q: What happens if I encode a custom album with frames in natural order?**
A: 1.0x playback works fine (engine reads all 16 native blocks = all 340 frames). But at 2.5x the half-sector read pulls frames 0–169 (the first half by physical byte position) instead of every-other-frame. Audio will skip in obvious chunks rather than fast-forwarding smoothly.

**Q: What FF/RW rates are supported?**
A: FF: `{2.5, 4, 8, 16}x` per `assets/audiothingies-2026-05-09/AudioEngine.hpp::kFastForwardRates`. RW: `{1, 2.5, 4, 8}x` per `kRewindRates`. **TimK said in Discord that he measured stock firmware going only up to 2.5x and his own custom firmware up to 2x; the higher rates in code may be aspirational.** See `corrections.md` and pending question to TimK.

**Q: How does the smooth play/pause "bend" work?**
A: One-pole lowpass smoothing on `current_speed` toward `target_speed` with coefficient 0.02 (= 2% per audio block). Combined with `VarispeedResampler`'s fractional resampling. At target speeds below 2.5x this gives smooth musical pitch bending. See `12-audio-engine-internals.md`.

**Q: Why exactly 2.5x as the transition point?**
A: Because the half-sector read gives 170 frames per sector instead of 340. To consume at 2.5x relative to normal playback, you need the same byte rate from the eMMC. 1.0x normal = 1.0x eMMC bandwidth → 2.5x playback at half data per sector = 1.25x eMMC bandwidth → marginal headroom. Above 2.5x, the engine switches to quarter-sector for additional headroom.

**Q: Can I implement my own time-stretching instead of block-skipping?**
A: Yes but it's CPU-expensive and won't reach the high FF rates. Stock TE firmware doesn't do real-time time-stretching; ericlewis's `VarispeedResampler` handles up to 2.5x via interpolation, after which it relies on block-skipping.

**Q: What if my song has variable BPM?**
A: The block-skipping mechanism is BPM-independent — it just reads fewer bytes per sector. Variable BPM affects the timing/sync side data, not the block layout. Sync words can encode tempo changes; see `10-midi-timing-encoding.md`.

**Q: What did emvee1968 get wrong before reading audiothingies?**
A: emvee1968 tried to implement FF by reading sectors faster, which fails on this eMMC. ericlewis pointed at the block-skipping mechanism and shared `assets/audiothingies-2026-05-09.zip` and `assets/storagethingies-2026-05-09.zip` to demonstrate the proper approach. See `corrections.md`.
