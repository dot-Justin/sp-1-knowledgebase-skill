# Audio Engine Internals

**Synthesized through:** Lines #846 (2026-05-06), Discord through 2026-05-11. Code-of-record: `assets/audiothingies-2026-05-09/AudioEngine.{hpp,cpp}`, `StockRuntimeMixer.{hpp,cpp}`, `VarispeedResampler.hpp`.

This file walks through the C++ audio engine that ericlewis published. It is a clean-room reimplementation of the stock TE playback engine, structurally aligned to how the stock firmware works. The DSP effects are covered separately in `13-dsp-effects.md`; this file covers the surrounding mixer, transport, resampling, output gain, and side-data plumbing.

## The pipeline

```
DiskManager
  ↓  (gives 8 KB sectors with audio + side-data)
StockRuntimeMixer::gather_transport_stem_block()
  ↓  (extracts per-stem int32 samples per frame)
StemEffectRack::process_in_place()
  ↓  (single-stem effects: filter/distortion/chorus-delay/gate, + always-on delay tap)
StockRuntimeMixer::mix_stem_block_to_output()
  ↓  (sum 4 stems with Q12 weights + global mix gain)
apply_master_compander()
  ↓  (peak detection + smooth gain reduction)
PcmPacking (float → right24)
  ↓  (clamp to ±1.0, scale to int32 with 24-bit range)
zephyr_i2s_tx
  ↓  (Zephyr I²S DMA to codecs)
[CS42L42 + TAS2505]
```

Each block below covers one stage.

## AudioEngine — high-level state machine

[code: `assets/audiothingies-2026-05-09/AudioEngine.hpp`]

`AudioEngine` is the top-level facade. It owns a `StockRuntimeMixer` and exposes:

- **Transport control:** `start_playback()`, `stop_playback()`, `pause_playback()`, `resume_playback()`, `rebase_transport()`
- **Mode selection:** `set_transport_mode(TransportMode mode, bool immediate)` where `TransportMode ∈ {Play, Slow, FastForward, Rewind}`
- **Rate stepping:** `step_transport_rate(delta)` — moves between rate indices for FF/RW (4 rates each, see `kFastForwardRates` and `kRewindRates`)
- **Mixer config:** `set_stem_mix_weights_q12()`, `set_all_stem_weights_q12()`, `set_solo_mask()`
- **Output gain:** `step_output_gain(delta)` with `current_output_volume_level()` and `current_output_gain_percent()`
- **Rendering:** `render_block(int32_t* output, size_t word_count)` — called by the I²S backend
- **Frame tracking:** `current_produced_frame()`, `current_consumed_frame()`, `current_sync_word()`, `current_track_led_word()`

It maintains atomic state for cross-thread safety (rendering happens on the audio thread; UI / app code calls into the engine from other threads):

```cpp
std::atomic<bool>       playing_, paused_, pending_unity_relock_;
std::atomic<TransportMode> transport_mode_;
std::atomic<uint8_t>    ff_rate_index_, rw_rate_index_, output_gain_step_;
std::atomic<uint32_t>   produced_frame_, consumed_frame_, sync_word_, led_word_;
std::array<std::atomic<uint16_t>, 4> stem_weights_q12_;
std::atomic<uint8_t>    solo_mask_;
```

## Transport modes and rates

[code: `assets/audiothingies-2026-05-09/AudioEngine.hpp` lines 17–29:]

```cpp
enum class TransportMode : uint8_t {
    Play = 0,
    Slow,
    FastForward,
    Rewind,
};

static constexpr std::array<float, 4> kFastForwardRates = {2.5f, 4.0f, 8.0f, 16.0f};
static constexpr std::array<float, 4> kRewindRates      = {1.0f, 2.5f, 4.0f, 8.0f};
```

- **Play** = 1.0x forward
- **Slow** = 0.5x forward (smooth half-speed)
- **FastForward** = 2.5/4/8/16x forward, advanced via `step_transport_rate()`
- **Rewind** = -1/-2.5/-4/-8x reverse

Below 2.5x (Slow and parts of FastForward at startup) the speed is achieved via the **VarispeedResampler** with linear interpolation between adjacent frames. At 2.5x and above, the **block-skipping mechanism** takes over (see `11-block-interleaving-tape-fx.md`).

### Smooth transitions (the "tape bend")

When the transport mode changes, the engine doesn't switch speed abruptly. It applies a one-pole lowpass smoothing per audio block:

```cpp
current_speed = current_speed + (target_speed - current_speed) * 0.02f
```

The coefficient `0.02f` is `0x3CA3D70A` in IEEE 754 hex (this exact bit pattern is mentioned in ericlewis's Discord notes [Discord #firmware, 2026-05-09 00:12]). Each audio block moves the speed 2% of the way to the target. For typical block sizes of ~32 frames, that's about 1.5 ms of audio per step; the bend takes ~50 blocks ≈ 80 ms to complete (the "tape ramp" sound).

`set_transport_mode(mode, immediate=true)` skips the smoothing and snaps to the new target speed instantly.

## VarispeedResampler — fractional-speed playback

[code: `assets/audiothingies-2026-05-09/VarispeedResampler.hpp`]

```cpp
static constexpr float kMinSpeed = -8.0f;
static constexpr float kMaxSpeed = 16.0f;
static constexpr uint32_t kPhaseOne = 1U << 24;   // Q24 fractional
```

The resampler tracks a fractional source-sample position:

```cpp
int64_t  source_sample_index_;   // Integer part (signed for reverse)
uint32_t frac_q24_;              // Fractional part (Q24)
float    target_speed_;
float    current_speed_;
```

Per output sample, the engine advances:

```
phase_inc_q24 = current_speed × 2^24       // e.g., 0.5x → 8388608
new_phase = (source_sample_index << 24) + frac_q24 + phase_inc_q24
source_sample_index = new_phase >> 24
frac_q24 = new_phase & 0xFFFFFF
```

Output sample = linear interpolation between the integer-frame source samples at `source_sample_index` and `source_sample_index + 1`, weighted by `frac_q24`.

This works smoothly for any speed where adjacent frames are available — which is exactly the range below ±2.5x where block-skipping isn't in play. Above that, the engine switches to the block-skip path.

The signed `int64_t` for the sample index means **reverse playback** is the same code with negative phase increments — no special logic needed.

## StockRuntimeMixer — the heart of rendering

[code: `assets/audiothingies-2026-05-09/StockRuntimeMixer.hpp`]

This is the per-block rendering engine. Each call to `render_block()` produces a chunk of output (up to 128 frames = 256 stereo samples) from the eMMC data + per-stem weights + transport state + active effects.

### Key data members

```cpp
storage::DiskManager*               disk_manager_;          // source of audio
bool                                dual_mono_ = true;       // produce mono summation
float                               mix_gain_ = 0.18f;       // global gain (low — ~ -15 dB)
std::array<FramePayloadCursor, 2>   frame_cursors_;          // double-cached sector access
std::array<std::array<int32_t, 256>, 4>  stem_block_buffers_; // 4 stems × 128 frames stereo
std::array<float, 4>                current_stem_weights_;   // Q12 → float weights
bool                                held_frame_valid_;       // for cache miss recovery
std::array<int32_t, 8>              held_frame_;             // 8 channels of last good frame
int32_t  compander_dc_fast_, compander_dc_slow_;
int32_t  compander_hp_state_;
int32_t  compander_peak_env_;
int32_t  compander_gain_q25_ = 0x03000000;                   // Q25 fixed point
VarispeedResampler            transport_;
effects::StemEffectRack       shared_effect_rack_;
std::array<ui::EffectState, 4> stem_effect_states_;
int  effect_active_stem_ = -1;
```

### Why two frame cursors?

The mixer caches **two** sector-lookup cursors so it can satisfy fractional resampling (which reads samples N and N+1) when those samples span a sector boundary. Without two cursors, every boundary-crossing frame would re-do the sector lookup work.

### Why `held_frame`?

If a frame cache miss happens (the prefetch hasn't yet finished loading the next sector), the mixer falls back to **holding the previous frame** — replays the last known good sample. This produces a brief audible pause/glitch rather than a discontinuity, and the engine logs the miss in `PerfStats.gather_miss_frames` so app code can monitor cache health.

### Q12 stem weights

The four stem fader values are stored as Q12 fixed-point integers (0–4096, where 4096 = 1.0):

```cpp
static constexpr uint16_t kStemWeightScaleQ12 = 4096U;
```

[code: `StockRuntimeMixer.hpp` line 32]

Per-block, these are converted to float and applied as multiplicative weights during stem summation. The conversion is exact (`weight_float = weight_q12 / 4096.0f`).

The solo mask (`set_solo_mask(uint8_t)`) is a 4-bit mask: when a bit is set, that stem is "soloed." When *any* solo bit is set, all non-soloed stems are forced to weight 0. This implements Track button solo behavior.

### Dual mono mode

`dual_mono_ = true` (default) configures the mixer to **sum L+R per stem into mono** before sending to the output buffer. This is the speaker mode (the internal TAS2505 amplifier is mono). When headphones are detected and stereo output is desired, application code calls `set_dual_mono(false)` to switch to true stereo summation.

### Mix gain

The default `mix_gain_ = 0.18f` is **low** — about −15 dB. This is intentional. Per-stem audio at full fader weight (1.0) can sum to 4.0 across all stems, well above unity. Without the global mix gain, output would clip immediately. 0.18 gives headroom for 4 stems at unity to summed loudness around ~0.7 (~ -3 dB), which the master compander then catches if it goes over.

### Master compander

The compander (compressor + expander) is a single-band peak-limiter with separate fast and slow DC tracking + a high-pass state + a peak envelope and Q25-fixed-point gain.

```cpp
int32_t compander_dc_fast_ = 0;
int32_t compander_dc_slow_ = 0;
int32_t compander_hp_state_ = 0;
int32_t compander_peak_env_ = 0;
int32_t compander_gain_q25_ = 0x03000000;       // Q25: 0x03000000 / 2^25 ≈ 0.094 initial
```

The implementation is in `StockRuntimeMixer.cpp`'s `apply_master_compander()` (not shown here in full). High-level:

1. Run a high-pass through `compander_hp_state_` to remove DC bias
2. Track fast peaks (`compander_peak_env_`) with rapid attack, slow release
3. Compute target gain as a function of peak (smaller gain when peak is high)
4. Smoothly approach target via `compander_gain_q25_` update
5. Multiply output samples by current gain

This produces transparent peak control — keeps levels in range without obvious pumping. It's what stops the speaker from clipping when all four stems are at full and the mix gets dense.

## RenderBlockResult

[code: `StockRuntimeMixer.hpp` lines 23–28:]

```cpp
struct RenderBlockResult {
    uint32_t wrapped_start_frame = 0U;
    uint32_t wrapped_end_frame = 0U;
    bool reverse = false;
    uint32_t read_blocks = 16;
};
```

After each `render_block()` call, the engine reports back:
- The frame indices that were rendered (wrapped modulo album total frames)
- Whether the rendering was reverse-direction
- How many native eMMC blocks the prefetch is currently using per sector (16/8/4)

The caller uses this to update its prefetch hints to `DiskManager`.

## Frame address arithmetic

The render loop translates source-sample indices (continuous, possibly negative) to album-relative frame indices:

```cpp
uint64_t wrap_transport_frame_index(int64_t frame_index) const;
```

This wraps `frame_index` to the album's total-frame count (for looping the entire album) and handles negative indices (for reverse playback that's gone past the start). The wrapping respects song boundaries via `DiskManager::resolve_album_frame()`.

## Render-protected sectors

[code: `StockRuntimeMixer::update_render_protected_sectors()`]

The mixer keeps track of the **two sectors** it's currently rendering from (`last_protected_sector_a_` and `_b_`) and updates `DiskManager::set_render_protected_sectors()` whenever they change. This tells the prefetch engine "don't evict these two slots — they're in active use." Without this, a slot could get reassigned mid-frame, producing audible glitches.

## Side-data extraction (sync words + LEDs)

Per the eMMC layout (`08-emmc-storage.md`), each sector's record includes:

```cpp
uint32_t block_sync_words[4];   // MIDI clock counter per TE-block
uint32_t block_led_words[4];    // LED state per TE-block
```

Every render block, the engine queries `try_get_block_side_data_cached()` from `DiskManager` and:

- Stores the **sync_word** in `AudioEngine::sync_word_` (accessible via `current_sync_word()`)
- Stores the **led_word** in `AudioEngine::led_word_` (accessible via `current_track_led_word()`)

The sync word drives effect synchronization (see `13-dsp-effects.md` and `10-midi-timing-encoding.md`). The LED word drives the per-frame Track-LED brightness animation (see Track LEDs section below).

## Track LED animation

`current_track_led_word()` returns a 32-bit value that the LED renderer decodes. The exact format isn't fully documented but the structure is: 4 bytes (one per track LED), each carrying a brightness level (0–255). The PWM2 output (driving the 4 Track LEDs) consumes this each frame to produce the throbbing/lighting effect synced with the music.

This is **separate from PWM-driven brightness control by the firmware** — the LED levels come from data baked into the album, so the firmware doesn't need to compute envelope follower / RMS on the fly.

## Output gain (9-step cubic curve)

[code: `AudioEngine::output_gain_step_` with values 0–8 = 9 steps]

Volume is a 9-step output gain:

```
output_gain_step_: 0 (silence) → 8 (unity)
```

The step → linear gain mapping is **cubic** (not linear or log-tapered) to match the perceptual response. The exact curve is implemented in `step_output_gain()` / `current_output_gain()`. Step 0 mutes completely; step 8 is unity (no attenuation).

`current_output_gain_percent()` returns a UI-friendly 0–100 percentage; `current_output_volume_level()` returns the step number for UI display.

## Per-block performance tracking

`StockRuntimeMixer::PerfStats` and `AudioEngine::PerfStats` track:

- Render call counts and CPU cycle counts (min, max, total)
- Cache miss counts categorized by transport mode
- Cursor resolve failures
- Gather phase cycles (extracting per-stem samples)
- Mix phase cycles (summing to output)

Application code can call `snapshot_perf_stats()` (returns by value with spinlock protection) to monitor audio thread health. The "max_cycles" fields are especially useful — they spike when a cache miss happens and the engine had to stall the audio thread.

## I²S TX backend

[code: `assets/audiothingies-2026-05-09/backends/zephyr_i2s_tx.cpp`]

The Zephyr backend wraps the I²S0 peripheral. It:

1. Allocates DMA buffer blocks (`CONFIG_I2S_NRFX_TX_BLOCK_COUNT=8` in `assets/sp1-midi-2026-05-13/prj.conf` — gives 8 ping-pong buffers)
2. Sets up I²S in master mode at 48 kHz, 24-bit data, 32-bit slot
3. Registers a callback that calls `AudioEngine::render_block()` to fill each buffer as it's consumed
4. Hands buffers to the I²S DMA engine for transmission

The 8-buffer count is generous — typical I²S setups use 2 or 4. The deeper queue gives the audio engine more time to recover from a transient cache miss without producing audible underruns.

## What this engine doesn't do

A few things you might expect that aren't in this code:

- **No I²S audio capture** — `SDIN` is not wired (codec ADC inputs go over I²C, not I²S). The headphone microphone (CS42L42 ADC) is reachable via the codec driver but the audio engine doesn't loop it through.
- **No multi-track recording** — the engine renders fixed-content audio plus user-controlled effects/mixing. It can't record live audio onto stems.
- **No tempo-mapping** — assumes constant playback rate (modulated only by transport mode). For tempo curves within a song, the on-disk timing data would need to support it (and may, but isn't yet verified — see `known-unknowns.md`).
- **No crossfade between songs** — songs are bit-exact concatenations. Stock TE firmware presumably applies a short crossfade in playback code; this reference engine doesn't.
- **No audio over Bluetooth** — the BT module is not in the playback path here.

## Memory footprint

The engine is reasonably compact. Major buffer allocations:

- 4 stems × 128 frames × 2 channels × 4 bytes = **4 KB** per stem block buffer = **16 KB** total
- Effect scratch buffer: 128 frames × 2 channels × 4 bytes = **1 KB**
- `DiskManager` slot records: 10 × (8192 + 16 + 16) bytes = **~82 KB**

Plus the DSP effect state (biquad/distortion/chorus/delay buffers — delay alone is 8192 × 2 bytes = 16 KB).

Total runtime audio state: roughly **120 KB**, well within the nRF52840's 256 KB RAM, leaving plenty for the rest of the app.

## Where to go next

- For the on-disk audio sample layout that feeds this engine → `09-audio-format-spec.md`
- For the block-interleaving trick that supports tape FX → `11-block-interleaving-tape-fx.md`
- For the per-stem DSP effects → `13-dsp-effects.md`
- For the MIDI timing data used by effects → `10-midi-timing-encoding.md`
- For the I²S codec configuration → `06-audio-codecs.md`
- For how this fits into a custom firmware → `20-custom-firmware-state.md`
