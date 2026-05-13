# Block Interleaving and Tape FX

**Synthesized through:** Lines #846 (2026-05-06), Discord through 2026-05-11, and **solderless source archive** ingested 2026-05-13. Primary sources: ericlewis's Discord explanation 2026-05-09 + `storagethingies/DiskManager.hpp` constant `kBlockOrder[4] = {0, 2, 1, 3}` + the canonical encoder `wav-parser.js::encodeToSP1` in the solderless archive, which uses the identical `BLOCK_ORDER = [0, 2, 1, 3]` constant and the formula `block_id = BLOCK_ORDER[frame_in_sector % 4]`. See `update-log.md` 2026-05-13.

The single cleverest design decision in the SP-1's audio format is the **on-disk block interleaving**. It lets the device do tape FF/RW effects at high speeds without exceeding the eMMC's read bandwidth, simply by **reading less data from each sector**. This file explains how it works.

> "this is why the TE audio format is clever. the chip obviously can't ACTUALLY handle 2.5-4x throughput." — ericlewis [Discord #firmware, 2026-05-09 00:28]

## The throughput problem

The eMMC runs at **32 MHz in 1-bit mode**, giving a theoretical maximum throughput of 4 MB/s. The audio data rate at normal playback (1.0x) is:

```
48000 samples/sec × 24 bytes/frame = 1,152,000 bytes/sec ≈ 1.15 MB/s
```

That's well within the eMMC's bandwidth at 1.0x. But the SP-1's stock firmware supports tape FF at up to 4x speed and tape RW at up to 8x. At 4x, the data rate would be 4.6 MB/s — **above the eMMC's max**. At 8x, 9.2 MB/s — way above. Sustaining those rates with conventional sequential reading is impossible on this chip.

The standard solutions would be (a) buffer aggressively and only support brief bursts, or (b) do real-time time-stretching DSP. Both have downsides: (a) limits how long you can FF/RW, and (b) eats CPU.

TE took a third path: **change the on-disk layout so that "reading less" produces a musically usable result.**

## The 0, 2, 1, 3 interleaving

Each TE sector (8192 bytes, 340 audio frames) is divided into 4 TE blocks of 2048 bytes (85 frames) each. Frames in the sector are distributed across the 4 blocks in a specific pattern.

[code: `storagethingies/DiskManager.hpp` line 34: `static constexpr int kBlockOrder[4] = {0, 2, 1, 3};`]

[Source: ericlewis, Discord #firmware, 2026-05-09 00:22:]

> Because of the 0, 2, 1, 3 interleaving:
> Block 0 contains frames: 0, 4, 8, 12...
> Block 1 contains frames: 2, 6, 10, 14...
> Block 2 contains frames: 1, 5, 9, 13...
> Block 3 contains frames: 3, 7, 11, 15...

So if you number the 340 frames in a sector from 0 to 339:

```
Block 0: frames 0,   4,   8,  12,  16, ..., 336   (frame_index % 4 == 0)
Block 1: frames 2,   6,  10,  14,  18, ..., 338   (frame_index % 4 == 2)
Block 2: frames 1,   5,   9,  13,  17, ..., 337   (frame_index % 4 == 1)
Block 3: frames 3,   7,  11,  15,  19, ..., 339   (frame_index % 4 == 3)
```

Note Block 1 holds the **even** frames and Block 2 holds the **odd** frames — the order is **not** monotonic across blocks. This is deliberate, as we'll see.

The reading order on disk goes 0 → 2 → 1 → 3 (the physical sequence the eMMC sees). Decoded to musical time, this gives the firmware:

- **Block 0 alone:** every 4th frame (frames 0, 4, 8, …)
- **Blocks 0 + 2:** every 2nd frame (frames 0, 2, 4, 6, 8, …) — i.e., reading the first half of the sector by raw byte position
- **Blocks 0 + 2 + 1 + 3:** every frame (full audio fidelity, normal playback)

## Why this enables tape FF / RW

The audio engine has three render modes that correspond to three sector read sizes:

| Mode | Speed | `read_blocks` | Bytes read per sector | Frames recovered |
| --- | --- | --- | --- | --- |
| Normal | 1.0x | 16 (all 16 native eMMC blocks) | 8192 | 340 (every frame) |
| **Half-sector mode** | 2.5x | 8 (first 8 native eMMC blocks = TE blocks 0 + 2) | 4096 | 170 (every other frame, with frame indices 0, 2, 4, ..., 338) |
| **Quarter-sector mode** | 4x – 16x | 4 (first 4 native eMMC blocks = TE block 0 only) | 2048 | 85 (every 4th frame, with frame indices 0, 4, 8, ..., 336) |

[code: `storagethingies/DiskManager.hpp` slot tracking shows `read_blocks` ∈ {16, 8, 4}; `audiothingies/StockRuntimeMixer::read_blocks_for_speed()` implements the mapping]

When in half-sector mode, the firmware **only reads the first 4 KB of each sector** (TE blocks 0 and 2, which together contain all even-numbered frames). The remaining 4 KB of the sector (TE blocks 1 and 3, with odd-numbered frames) is **skipped entirely**. The eMMC reads half the data, so the bandwidth required is half — which lets the firmware play back at 2.5x speed (consuming frames twice as fast as normal, but only reading half as many frames per sector means the per-sector bandwidth stays at the 1.0x rate).

When in quarter-sector mode, only TE block 0 is read — 2 KB per sector — and the engine plays at 4x or higher.

## Frame duplication fills the gap

When the engine reads half a sector, it gets every other frame (0, 2, 4, 6, ...). To produce audio that *sounds* like FF (faster playback, recognizable content), it **duplicates each frame** to fill the missing slots:

> It just duplicates the even frames to fill the gaps, effectively downsampling the audio to 24kHz on the fly without doing any math. — ericlewis, 2026-05-09 00:22

So instead of frames `[F0, F1, F2, F3, F4, F5, ...]` (which would be the full 48 kHz output), the engine sends `[F0, F0, F2, F2, F4, F4, ...]` — sample-and-hold style. At 2.5x playback speed, this sounds like the recording running fast (the audio is temporally correct: F0 at time 0, F2 at time 2*Δt, etc.), with a slight loss of high-frequency detail (since odd-frame samples are gone).

For quarter-sector mode: frames are quadruplicated. `[F0, F0, F0, F0, F4, F4, F4, F4, ...]`. The high-frequency loss becomes audible (sounds like a 12 kHz-sample-rate version of the original), but the **musical content** is still recognizable and "FF feel" works.

This is conceptually similar to what a vinyl player does at non-standard speeds — the audio sample-rate decreases but the content remains intact.

## Why 0, 2, 1, 3 specifically?

To make half-sector reads work as **"every other frame from the start of the sector"**, the firmware needs:

1. The first half of the sector (in physical byte position) to contain **every other frame**.
2. The remaining frames to be in the second half (which will be skipped at high speed).

Block layout 0, 2, 1, 3 satisfies this because:
- **Block 0 (first physical position) = even-modulo-4 frames** (0, 4, 8, ...)
- **Block 2 (second physical position) = even-not-modulo-4 frames** (2, 6, 10, ...)
- **Together (first half of sector) = ALL even-numbered frames** (0, 2, 4, 6, ...)

And:
- **Block 1 (third physical position) = odd-modulo-4 frames** (1, 5, 9, ...)
- **Block 3 (fourth physical position) = odd-not-modulo-4 frames** (3, 7, 11, ...)
- **Together (second half of sector) = ALL odd-numbered frames** (1, 3, 5, 7, ...)

And for quarter-sector mode:
- **Block 0 (first physical position) alone = every 4th frame** (0, 4, 8, ...)

This is the only ordering that gives **both** the half-sector and quarter-sector reads a clean every-Nth-frame result. If the blocks were stored in plain ascending order (0, 1, 2, 3), reading only block 0 would give you frames 0–84 (the first 85 frames) — a tiny temporal window of the song, not a fast-forwarded view of the whole sector.

## The CPU cost is zero

The genius of this is that **the CPU does no extra work** at high-speed playback. The engine just:

1. Reads fewer bytes from the eMMC per sector
2. Decodes the frames in the order they appear in the buffer
3. Sample-and-holds each frame to fill the output buffer

No real-time time-stretching DSP, no interpolation, no frame indexing math. The data layout itself does the heavy lifting.

> By arranging the bytes on the eMMC this way, the CPU doesn't have to do any extra work to skip samples when fast-forwarding, and the eMMC bus bandwidth is fine. — ericlewis, 2026-05-09 00:22

## What this means for stem encoders

When you produce a custom album image, you **must** lay out the blocks in 0/2/1/3 order, or the device's FF/RW will produce nonsense audio (you'd hear localized snippets instead of a sped-up overview of the song).

The simplest way to think about encoding:

```
For each 340-frame sector:
    Block 0 = frames where frame_index % 4 == 0
    Block 1 = frames where frame_index % 4 == 2
    Block 2 = frames where frame_index % 4 == 1
    Block 3 = frames where frame_index % 4 == 3
Write blocks to disk in the order [Block 0, Block 1, Block 2, Block 3]
   (which is the physical disk order matching kBlockOrder = {0,2,1,3})
```

Or equivalently:

```
For frame_index in 0..339:
    if frame_index % 4 == 0: write to TE Block 0
    if frame_index % 4 == 2: write to TE Block 1   (note swap)
    if frame_index % 4 == 1: write to TE Block 2   (note swap)
    if frame_index % 4 == 3: write to TE Block 3
```

The naming is confusing because `kBlockOrder[i]` describes which **logical** block ID is at position `i` on disk. If you index by physical disk position, the order is straightforward; if you think in logical block IDs, you need to map them.

## Variable read sizes in `DiskManager`

[code: `storagethingies/DiskManager.hpp` — `AudioSlot::read_blocks` field; `service_audio_prefetch_window()`]

The DiskManager records, per audio slot, how many native eMMC blocks were read into that slot's buffer (16, 8, or 4). The audio engine queries `read_blocks` when extracting frames so it can:

1. Index correctly within the partial buffer (a sector read at `read_blocks=8` only has audio data in the first 4 KB of the slot's 8 KB payload)
2. Skip the missing frame slots during decoding
3. Apply the frame-duplication policy

`audiothingies/StockRuntimeMixer::read_blocks_for_speed(speed)` is the mapping from desired playback speed to `read_blocks`. Speed thresholds:

- `speed < 2.5x` → `read_blocks = 16` (full sector)
- `2.5x ≤ speed < 4x` → `read_blocks = 8` (half sector)
- `speed ≥ 4x` → `read_blocks = 4` (quarter sector)

## What about reverse playback?

Tape RW (rewind) is **conceptually the same**: read fewer blocks per sector at higher RW speeds, just iterate sectors in descending order. The frame duplication for sample-and-hold output works identically.

`audiothingies/AudioEngine.hpp` has separate rate arrays:

```cpp
static constexpr std::array<float, 4> kFastForwardRates = {2.5f, 4.0f, 8.0f, 16.0f};
static constexpr std::array<float, 4> kRewindRates      = {1.0f, 2.5f, 4.0f, 8.0f};
```

Note rewind starts at **1.0x**, fast-forward at **2.5x**. The first rewind rate (1.0x reverse) doesn't trigger the block-skipping — it just reads sectors backward at the normal data rate.

**Discrepancy:** TimK said in Discord (2026-05-09 07:04) *"I got FFWD only up to 2X after that you need to jump eMMC blocks. I think I measured the original fw only going up to 2.5x"*. This appears to describe TimK's own custom firmware (which tops out at 2x) and his measurement of how high the stock TE firmware goes (2.5x). The audiothingies code lists rates up to 16x — these may be aspirational (intended for a more complete custom firmware) or may reflect the stock firmware's full capability that TimK didn't measure. **Awaiting TimK clarification.** See `corrections.md`.

## What emvee1968 ran into

emvee1968's first attempt at FF/RW in custom firmware did not use this block-skipping technique:

> One thing I have been struggling with is the tape effects for fast forward and rewind.. spent hours trying to get that working, every time it results in degraded audio / artifacts / glitches / drop outs… seems the eMMC cannot be read fast enough. [Discord #firmware, 2026-05-09 00:04]

ericlewis's reply pointed at this exact mechanism and shared `audiothingies.zip` and `storagethingies.zip` as the reference. Custom firmware that tries to FF/RW by "just reading sectors faster" will fail; the only way to do it is the block-skipping + frame-duplication trick.

## The pause / play "bend" effect

There's a related but distinct mechanism for the gentle pitch-bend you hear when pressing play or pause. From ericlewis [Discord #firmware, 2026-05-09 00:12]:

```
// Speed smoothing (one-pole lowpass, exponential):
current_speed = current_speed + ((target_speed - current_speed) * 0.02f);
```

The smoothing coefficient `0.02f` is represented internally as `0x3CA3D70A` in hex. Every audio block, the playback speed moves 2% of the remaining distance toward the target. This produces a smooth, logarithmic ramp:

- **Pause:** target_speed = 0.0, current_speed ramps down
- **Play:** target_speed = 1.0, current_speed ramps up
- **Switch transports:** target_speed = 1.0 → 2.5 (for example), current_speed bends through 1.5, 2.0 on the way

The fractional resampling that handles this — sub-sample interpolation — is implemented by `audiothingies/VarispeedResampler.hpp`:

```cpp
// Q24 fixed-point phase
static constexpr uint32_t kPhaseOne = 1U << 24;
// Speed range
static constexpr float kMinSpeed = -8.0f;
static constexpr float kMaxSpeed = 16.0f;
```

The phase increment is `current_speed × 2^24` per output sample. Linear interpolation between adjacent input samples reconstructs the smooth pitched output.

So there are **two mechanisms working together**:

1. **VarispeedResampler** handles speeds in roughly the range −2.5x to +2.5x (where there's enough data to interpolate between adjacent frames) — this is the smooth pitch-bend / 0.5x slow / brief FF up to 2.5x.
2. **Block skipping** kicks in at exactly 2.5x and above (FF) — the engine reads fewer blocks per sector and stops interpolating across "missing" frames.

The transition between the two is precisely at 2.5x because that's the speed where the half-sector layout produces exactly enough samples to match the playback rate.

## Why this format is fragile to encode wrong

If you produce a custom album with frames in **straight order** (frame 0, 1, 2, ..., 339 in physical disk order), the audio will play correctly at 1.0x — the engine reads all 16 native blocks and gets all 340 frames. But at 2.5x or higher, the half-sector read will pull frames 0–169 (the first half by raw position) instead of every-other-frame. The result will be:

- A brief 170-frame snippet of the song at the start of each sector
- Then a jump to the next sector, where you hear another 170-frame snippet
- Not at all like fast-forward — sounds like rapid skipping with audible discontinuities

This is the silent failure mode of getting the block layout wrong. Custom album encoders **must** lay out frames in the interleaved 0/2/1/3 order or the FF feature will be broken even if normal playback works fine.

## Where to go next

- For the audio engine that consumes this layout → `12-audio-engine-internals.md`
- For the eMMC sector model → `08-emmc-storage.md`
- For the on-disk byte order within a frame → `09-audio-format-spec.md`
- For the DSP effects (some of which are speed-dependent) → `13-dsp-effects.md`
- For canonical stem encoding → `21-original-firmware-stems.md`
