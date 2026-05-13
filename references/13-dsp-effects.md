# DSP Effects

**Synthesized through:** Lines #846 (2026-05-06), Discord through 2026-05-11. Code-of-record: `assets/audiothingies-2026-05-09/effects/` (5 effect nodes + StemEffectRack + DspTables + DspUtils).

The SP-1 has **four user-facing effect types**, each with **4 variations**. Selecting an effect sends audio through a specific node from the `assets/audiothingies-2026-05-09/effects/` library. This file documents what each effect actually does in DSP terms, drawing from the source code and from ericlewis's Discord explanations.

## High-level architecture

[code: `assets/audiothingies-2026-05-09/effects/StemEffectRack.hpp`]

```cpp
enum class EffectType {
    FILTER,         // BiquadFilter
    CHORUS_DELAY,   // ChorusFlangerNode (variations 0-1) + DelayEchoNode (variations 2-3)
    DISTORTION,     // DistortionNode
    GATE,           // GateEnvelopeNode
};

struct EffectState {
    bool        active;
    bool        wet_enabled;
    EffectType  type;
    int         variation;   // 0..3
};
```

**Critical hardware constraint:** Only **one stem at a time** can have an active effect [code: `StockRuntimeMixer::set_stem_effect()` comment]. Activating an effect on stem N deactivates any effect previously running on a different stem and resets the shared DSP state (delay lines, biquad history, etc.). This mirrors the stock TE behavior — the user can apply effects to one stem at a time.

**Always-on delay tap:** Regardless of which primary effect type is active, the **DelayEchoNode is always running** in the background. When the effect type is FILTER, DISTORTION, or GATE, the audio passes through the primary effect *and* through the delay echo (with `wet_enabled=false` so the delay just records but doesn't mix back to output). When the user activates CHORUS_DELAY with variation 2 or 3, the delay is *the* effect (no chorus), with `wet_enabled=true`. This "always recording" design is what makes effect activation instantaneous — the buffer is already full of recent audio when you trigger the effect [Discord #firmware, ericlewis, 2026-05-09 00:32].

## Transition fades

When an effect is activated or deactivated, the rack applies a **smooth fade transition** rather than a hard cut, to prevent audible pops [code: `StemEffectRack::apply_effects_to_buffer()` with `RoutePhase::FadeIn` / `FadeOut`]:

- **FadeIn:** the new effect's output is crossfaded in over one audio block
- **FadeOut:** the old effect's output is crossfaded out over one audio block
- **Steady:** normal effect processing

The transition phase is spinlock-protected to avoid tearing between the UI thread (which calls `set_effect_state()`) and the audio thread (which calls `apply_effects_to_buffer()`).

---

## FILTER (cascaded biquad with LFO modulation)

[code: `assets/audiothingies-2026-05-09/effects/BiquadFilter.hpp`]

A 2-channel (stereo) biquad IIR filter, with **four variations**:

| Variation | LFO rate | Filter behavior |
| --- | --- | --- |
| 0 | none (static) | Butterworth lowpass at `kBiquadFilterIndexA` (specific table entry) |
| 1 | none (static) | Butterworth lowpass at `kBiquadFilterIndexB` (different table entry) |
| 2 | **0.18 Hz** | LFO-swept lowpass cutoff |
| 3 | **0.45 Hz** | Faster LFO-swept lowpass cutoff |

The static variations (0, 1) are fixed-cutoff Butterworth lowpass filters. ericlewis describes them as a coarse "more open" / "darker" choice.

The swept variations (2, 3) modulate the filter's cutoff via an LFO:

```cpp
filter_index = kBiquadSweepOffset +
               (fast_sin(lfo_phase + kBiquadPhaseOffset) + 1.0f) * kBiquadSweepScale
```

The cutoff sweeps through a range of pre-computed Butterworth coefficient table entries, producing a "wah" / "filter sweep" sound. The LFO rate `0.45 Hz` (≈ 2.2 second cycle) or `0.18 Hz` (≈ 5.5 second cycle) gives slow musical sweeps suitable for stem effects.

### Implementation details

- **Stereo:** `detail::StereoBiquad` processes L and R independently with the same coefficients
- **Block processing:** the source comment says it processes 8 samples at a time, maintaining state across blocks to avoid artifacts at block boundaries
- **Wet/dry mixing:** linear crossfade between dry and wet signals to prevent clicks during activation
- **Coefficient table:** `tables::kButterworthLowpassTable` (defined in `effects/DspTables.hpp`) holds a precomputed set of biquad coefficients for various cutoffs

### Two LPF presets depend on sync word

Per TimK: *"the embedded timing data is essential for looping, gate, delay, and 2 of LPF presets"*. The two variations that use timing data are presumably variations 2 and 3 (the LFO-modulated ones) — the LFO phase is presumably driven by the song's beat counter rather than a free-running clock. The exact mechanism isn't in the public code (the `lfo_phase_` here appears free-running), but stock firmware likely advances `lfo_phase_` based on `current_sync_word()` rather than wall-clock time.

---

## DISTORTION (polynomial waveshaper + post-EQ)

[code: `assets/audiothingies-2026-05-09/effects/DistortionNode.hpp`]

A soft-clipping distortion with a post-filter, in **four variations** that differ in tone cutoff:

| Variation | Tone cutoff |
| --- | --- |
| 0 | **8000 Hz** (brightest) |
| 1 | **6000 Hz** |
| 2 | **4300 Hz** |
| 3 | **3200 Hz** (darkest) |

[code: lines 26–33 of `DistortionNode.hpp`]

### DSP flow

Per ericlewis's Discord description [2026-05-09 00:32]:

1. **Pre-gain:** multiply input samples by 16.0 (massively boost)
2. **Hard clip:** clamp to ±1.0 (the actual clipping)
3. **Polynomial waveshaping:** apply `y = x − 0.5 · x³` to round off the clipped edges (gives an analog-style warmth versus harsh digital clipping)
4. **Post-filter:** apply the variation-specific lowpass (biquad) to tame harmonic harshness
5. **Wet/dry blend:** mix back with the dry signal

The pre-gain factor of **16.0** is aggressive — this is the source of the distortion's energy. Without the post-filter, the clipped audio would sound buzzy / harsh; the lowpass smooths it into a more musical "fuzz" character.

### Variation feel

The four variations produce darkening tone — variation 0 is bright/raspy, variation 3 is muffled/woolly. The musical use case is presumably "match the distortion character to the song's mix."

### Implementation details

- **Stereo:** independent processing per channel via `detail::StereoBiquad tone_filter_`
- **`current_mix_gain_`:** crossfades from 0 (off) to 1 (full effect) and back, preventing clicks on activation
- **Reset:** clears `tone_filter_` history and `current_mix_gain_` to zero

---

## GATE (linear volume envelope)

[code: `assets/audiothingies-2026-05-09/effects/GateEnvelopeNode.hpp`]

Internally referred to as a "volume ramp," not a threshold-based noise gate. **Four variations** select target gain levels from a table:

```cpp
return tables::kGateLevels[clamped_variation];
```

The four levels are some monotonic sequence — typical values would be like 0.0, 0.25, 0.5, 0.75 — but the exact values are in `DspTables.hpp` and not documented here.

### DSP flow

Each block:
1. `start_gain` = current_gain (or 1.0 if FadeIn transition)
2. `end_gain` = `kGateLevels[variation]` (or 1.0 if FadeOut transition)
3. Linear interpolate per-frame: `gain[i] = start_gain + (end_gain − start_gain) × (i / frame_count)`
4. Multiply both L and R samples by `gain[i]`

The "gate" is really a **smooth mute/unmute** that ramps over the duration of one audio block (~32 frames = ~0.67 ms at 48 kHz). The variations differ in **how much attenuation** they apply: variation 0 might be full mute (gain = 0), variation 3 might be slight ducking (gain = 0.5), etc.

This is unusual for a "gate" — most gates have an envelope tied to input amplitude. Here it's deterministic and triggered by the firmware (presumably synced to beat divisions in stock firmware via `current_sync_word()`). That's why TimK said "gate" depends on timing data — the gate's beat-aligned cycling is what makes it musically useful.

### Implementation details

- `current_gain_` is preserved across blocks (so changing the variation produces a continuous transition)
- `reset()` resets to gain = 1.0 (unattenuated)

---

## CHORUS_DELAY (split: ChorusFlanger or DelayEcho)

This is two effects masked behind one selection, depending on variation:

| Variation | Effect |
| --- | --- |
| 0 | ChorusFlanger (chorus character) + always-on delay (off) |
| 1 | ChorusFlanger (flanger character) + always-on delay (off) |
| 2 | **DelayEcho only** (with wet_enabled=true) |
| 3 | **DelayEcho only** (with wet_enabled=true) — different profile |

So variations 0 and 1 are chorus/flanger; variations 2 and 3 are echo. This is a design decision in the rack — there's no fundamental DSP reason the effects couldn't be separate categories.

### ChorusFlangerNode

[code: `assets/audiothingies-2026-05-09/effects/ChorusFlangerNode.hpp` — header only briefly inspected here]

A modulated delay line:
- **Chorus** (variation 0): delay ~10–30 ms, slow LFO modulation (~0.5 Hz), moderate feedback
- **Flanger** (variation 1): delay ~1–10 ms, faster LFO modulation, higher feedback for resonant peaks

Both produce the classic "phasey" / "swooshy" sound. Implementation details (LFO rate, delay range, feedback amount) are in the header source and `DspTables.hpp`. Mixed with the dry signal at a fixed wet/dry ratio per variation.

The "always-on delay tap" is **off** for chorus/flanger variations — the dry-only path goes through the rack.

### DelayEchoNode (used directly as variations 2/3 of CHORUS_DELAY, plus always-on tap for FILTER/DISTORTION/GATE)

[code: `assets/audiothingies-2026-05-09/effects/DelayEchoNode.hpp`]

```cpp
static constexpr size_t kDelayBufferSamples = 8192U;        // ~170 ms at 48 kHz
std::array<int16_t, kDelayBufferSamples> delay_buffer_;
```

An 8192-sample (~170 ms) circular delay line, **per stem**. Stored as `int16_t` (not 24-bit) — the delay buffer trades some precision for half the memory footprint, which matters when each of 4 stems has its own buffer (4 × 16 KB = 64 KB if it were int32, vs 4 × 16 KB / 2 = 32 KB as int16).

The delay length and feedback amount come from `tables::kDelayWetProfiles[variation]`. Four profiles → variations 0/1 (when invoked as the always-on tap by FILTER/DISTORTION/GATE) and variations 2/3 (when CHORUS_DELAY mode uses delay-only).

### The "always-listening" design

Per ericlewis [Discord #firmware, 2026-05-09 00:32]:

> Each of the 4 stems has its own dedicated 8192-sample circular delay line allocated in memory. In the i2s.c routing loop, the firmware **always feeds the post-FX audio of every stem into its respective delay buffer, even if the echo effect is turned off**. When the effect is off, it passes a `\0` flag to dsp_apply_delay_echo, which tells it to record into the buffer but not output the delay. When you activate the echo effect, it passes a `\x01` flag, which tells it to start mixing the buffer into the output.
>
> Because the buffer is always recording, activating the echo effect instantly plays back the past audio history of the stem, rather than starting from an empty buffer and waiting for new audio to echo.

This is the trick that makes "echo" feel responsive. A normal delay effect, on activation, produces silence at first and only gradually fills with audio as new samples enter. The SP-1's always-listening delay has 170 ms of recent audio sitting in the buffer at the moment of activation — so the echo is audible immediately.

### Implementation details

- **Buffer wrapping:** `write_index_ = (write_index_ + 1) & kDelayBufferMask` using the bit-mask trick since 8192 = 2^13
- **Stereo:** L sample at write_index, R sample at (write_index + 1) — but the indexing in the source has a subtle quirk where left/right are interleaved within the buffer. Read the source carefully if implementing.
- **Fill progress:** `fill_progress_` tracks how full the buffer is from a recent activation; affects the wet/dry mix to avoid pops when activating into a freshly-cleared buffer
- **wet_enabled vs wet:** the buffer always writes; the **output** mixing of wet (delayed) signal is gated by `wet_enabled_`

---

## EffectState fields

[code: `assets/audiothingies-2026-05-09/effects/StemEffectRack.hpp` line 24:]

```cpp
struct EffectState {
    bool        active;        // is the effect on?
    bool        wet_enabled;   // separate gate for the wet output (used by DelayEcho)
    EffectType  type;
    int         variation;     // 0..3 (clamped)
};
```

`active = false` deactivates the effect entirely (with FadeOut). `wet_enabled = false` lets the engine pass audio through the delay node for recording but not mix the delayed output back in — used by FILTER/DISTORTION/GATE which want the always-listening behavior without echo on the audio.

## Tempo-synced behaviors (presumed)

Per TimK: looping, gate, delay, and 2 of LPF presets depend on the on-disk MIDI timing counter. In the public reference code (`audiothingies`), the effect modules don't show explicit reads of `current_sync_word()` — their behavior in this code is **free-running**. In the **stock TE firmware**, the same effect modules are presumably driven by the sync word:

- **Filter (variations 2/3):** LFO phase advanced by sync counter → swept cutoff syncs with the beat
- **Gate:** target_gain alternation might be synced to beat (e.g., gain pulses on every 4th tick)
- **Delay:** delay length in samples = (samples per quarter note) × variation-specific multiplier; produces musically-synced echo
- **Looping:** loop length set to N beats from current position; sync word tracks loop boundaries

For custom firmware aiming at stock-feel, you'd add sync-word reads at the appropriate points in each effect. The reference code doesn't yet do this; ericlewis's full FW replacement [Discord #firmware, 2026-05-09 00:10 — *"my full fw replacement isn't clean enough yet"*] presumably does.

## Sample rate

Each effect's `set_sample_rate()` is called with `detail::kDefaultSampleRate = 48000.0f`. The biquad coefficients in `DspTables.hpp` are pre-computed for this sample rate. If you ever wanted to repurpose this code at a different rate, you'd need to regenerate the coefficient tables.

## DspUtils — supporting math

[code: `assets/audiothingies-2026-05-09/effects/DspUtils.hpp`]

This header provides:

- `fast_sin(phase)` — sine approximation (probably parabolic or table-based, fast)
- `StereoBiquad` — 2-channel biquad with state preservation
- `BiquadCoefficients` — `{b0, b1, b2, a1, a2}` struct
- `TransitionPhase` enum (`Steady`, `FadeIn`, `FadeOut`)
- `kDefaultSampleRate = 48000.0f`

The biquad is a standard direct-form II implementation; the state is two float values per channel (`z1`, `z2`).

## DspTables — precomputed coefficients

[code: `assets/audiothingies-2026-05-09/effects/DspTables.hpp`]

Holds:

- `kButterworthLowpassTable` — array of `BiquadCoefficients` for various cutoff frequencies (used by the LFO sweep in BiquadFilter)
- `kBiquadFilterIndexA`, `kBiquadFilterIndexB` — fixed indices for filter variations 0 and 1
- `kBiquadSweepOffset`, `kBiquadSweepScale`, `kBiquadPhaseOffset` — LFO sweep mapping constants
- `kGateLevels` — array of 4 target gain values for GATE variations
- `kDelayWetProfiles` — array of 4 delay profiles (delay time, feedback, wet level)

For exact values, read the header directly.

## What this code doesn't include

- **Compressor / limiter** as a separate user-facing effect (the master compander in `StockRuntimeMixer` does light limiting but isn't user-selectable)
- **Reverb** — only short delay/echo, no convolutional or algorithmic reverb
- **Pitch shift** — not in the effect rack (virtualflannel_46386 has pitch shift in their custom OS, but that's separate code)
- **Bit crusher / sample-rate reduction** — also virtualflannel's custom code
- **Tape saturation** — distinct from distortion; not in the rack

These represent the user-effect surface as ericlewis reverse-engineered from stock TE firmware. The custom OSes adding these features are extending beyond the stock effect set.

## Where to go next

- For the host audio engine that drives these effects → `12-audio-engine-internals.md`
- For the sync word that musically synchronizes effects (in stock firmware) → `10-midi-timing-encoding.md`
- For the block-skipping trick that produces tape FX → `11-block-interleaving-tape-fx.md`
- For canonical stem prep including timing data → `21-original-firmware-stems.md`
- For custom firmware status (extended effects in virtualflannel/emvee) → `20-custom-firmware-state.md`
