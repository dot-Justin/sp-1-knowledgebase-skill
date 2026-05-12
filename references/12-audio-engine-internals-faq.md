# FAQ — Audio Engine Internals

**Q: What does the audio engine pipeline look like?**
A: DiskManager → StockRuntimeMixer.gather → StemEffectRack.process → StockRuntimeMixer.mix → master compander → float-to-int24 → Zephyr I²S TX. See `12-audio-engine-internals.md`.

**Q: How many transport modes are there?**
A: Four: **Play (1.0x), Slow (0.5x), FastForward, Rewind**. FastForward and Rewind have 4 rate stops each. See `audiothingies/AudioEngine.hpp` `TransportMode` enum.

**Q: What's the smoothing coefficient for play/pause bends?**
A: `0.02f` (= `0x3CA3D70A` in IEEE 754). Each audio block, `current_speed` moves 2% of the way toward `target_speed`. See `12-audio-engine-internals.md`.

**Q: What's Q12 weights?**
A: Per-stem fader values stored as 16-bit unsigned ints scaled by 4096 (= 1.0). Range 0–4096 maps to gain 0.0–1.0 with exact fractional precision. [code: `StockRuntimeMixer::kStemWeightScaleQ12 = 4096`]

**Q: How does solo work?**
A: 4-bit `solo_mask`. If any bit is set, non-soloed stems are forced to weight 0. Soloed stems play at their fader value.

**Q: What's the default mix gain?**
A: `0.18f` (~ -15 dB). Low enough to prevent clipping when all 4 stems play at unity. The master compander further limits peaks.

**Q: What's the master compander?**
A: Peak-limiting single-band compressor: high-pass DC removal, fast peak tracking, smooth gain reduction via Q25 fixed-point gain coefficient. Keeps the speaker from clipping under dense mixes. See `StockRuntimeMixer::apply_master_compander()`.

**Q: How many effect stems can run at once?**
A: **Just one.** Hardware constraint mirrored in the code. Activating effect on a new stem deactivates the previous and resets shared DSP state. [code: `StockRuntimeMixer::set_stem_effect()` comments]

**Q: What's `held_frame`?**
A: Fallback for cache misses. When prefetch hasn't yet loaded the next sector, the mixer replays the last good frame to avoid a discontinuity (brief audible glitch instead of a hard pop).

**Q: Why two `frame_cursors_`?**
A: Fractional resampling reads samples N and N+1; those may span a sector boundary. Two cursors avoid re-doing sector lookup work at every boundary.

**Q: What does `RenderBlockResult` carry?**
A: Wrapped start/end frame indices, reverse flag, current `read_blocks` (16/8/4). Caller uses this to update prefetch hints. See `StockRuntimeMixer.hpp` line 23.

**Q: How many output gain steps?**
A: **9 steps (0–8)**, cubic curve. Step 0 = mute, step 8 = unity. `current_output_gain_percent()` returns UI-friendly 0–100. See `AudioEngine::step_output_gain()`.

**Q: How big are the per-stem buffers?**
A: 128 frames × 2 channels × 4 bytes (int32) per stem = 1 KB. 4 stems = 4 KB. Plus effect scratch buffer (1 KB) and held-frame state. Total mixer state ~20 KB.

**Q: How does the engine track playback position?**
A: `produced_frame_` (what's been rendered) and `consumed_frame_` (what's been transmitted by I²S DMA). These wrap modulo album frame count. Reverse playback uses negative `source_sample_index_` in the VarispeedResampler.

**Q: What's the I²S TX buffer count?**
A: 8 (`CONFIG_I2S_NRFX_TX_BLOCK_COUNT=8` in `sp1-midi/prj.conf`). Deeper than typical to absorb transient eMMC cache misses without audible underrun.

**Q: How does the engine drive the Track LEDs?**
A: Reads `current_track_led_word()` from per-block side data on the eMMC. 4 bytes encode 4 brightness levels. Avoids real-time envelope-following. See `12-audio-engine-internals.md` "Track LED animation".

**Q: What does VarispeedResampler do at high speeds?**
A: Up to ~±2.5x it does linear interpolation between adjacent frames (`phase_inc_q24 = speed × 2^24`). Beyond 2.5x, the engine switches to block-skipping (`11-block-interleaving-tape-fx.md`). VarispeedResampler's `kMaxSpeed = 16.0f` and `kMinSpeed = -8.0f` reflect the engine's overall transport range.

**Q: Can the engine play audio over Bluetooth?**
A: No. The BT module is not in the audio path. Stock TE firmware presumably bridges audio over A2DP via the CYBT module, but that's not in the reference code.

**Q: Where is the buffer underrun detection?**
A: `StockRuntimeMixer::PerfStats.gather_miss_frames` (and per-transport-mode breakdowns: `play_miss_frames`, `slow_miss_frames`, `ff_miss_frames`, `rw_miss_frames`). Read via `snapshot_perf_stats()`. Spikes indicate the eMMC prefetch is falling behind.
