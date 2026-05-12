# FAQ — DSP Effects

**Q: How many user-facing effect types are there?**
A: **Four**: FILTER, CHORUS_DELAY, DISTORTION, GATE. Each has 4 variations (0–3). See `13-dsp-effects.md`.

**Q: How many stems can have an active effect at once?**
A: **One.** Hardware constraint. Activating an effect on a new stem deactivates the previous one and resets shared DSP state.

**Q: What's "always-on delay"?**
A: A DelayEcho buffer is always recording the post-FX audio of every stem, even when the user hasn't activated the echo effect. When CHORUS_DELAY variations 2/3 are activated, the delay starts mixing back to output — and because the buffer is already full of recent audio, the echo is **instantly audible** rather than ramping up from silence. See `13-dsp-effects.md` "always-listening".

**Q: What does FILTER do?**
A: Cascaded biquad lowpass.
- Variation 0/1: static cutoff (`kBiquadFilterIndexA` / `IndexB`)
- Variation 2: 0.18 Hz LFO sweep of cutoff
- Variation 3: 0.45 Hz LFO sweep of cutoff

**Q: What does DISTORTION do?**
A: Pre-gain (×16) → hard clip → polynomial waveshape `y = x − 0.5·x³` → post-lowpass (variation-specific cutoff) → wet/dry blend. Variations differ in post-filter cutoff (8 kHz / 6 kHz / 4.3 kHz / 3.2 kHz).

**Q: What does GATE do?**
A: It's a **linear volume envelope**, not a threshold-based noise gate. Smoothly ramps gain to `kGateLevels[variation]` (table-defined target levels). Variations select different attenuation depths. Stock firmware probably syncs the ramp to beat divisions via the sync word.

**Q: What does CHORUS_DELAY do?**
A: Mixed bag:
- Variations 0–1: ChorusFlanger (modulated delay with feedback) — variation 0 = chorus character, variation 1 = flanger character
- Variations 2–3: DelayEcho with `wet_enabled=true` (just echo)

**Q: How long is the delay buffer?**
A: **8192 samples ≈ 170 ms** at 48 kHz. Per-stem (so 4 of them, ~64 KB total). Stored as `int16_t` (not 24-bit) to halve the memory footprint.

**Q: Why a `int16_t` buffer when the engine is 24-bit?**
A: Trade precision for memory. 8 dB of dynamic range loss is acceptable for delay/echo (already a "lossy" effect by design); saves ~32 KB of RAM.

**Q: Which effects depend on the timing sync word?**
A: **Looping, gate, delay, and 2 of the 4 LPF presets** [per TimK]. Stock firmware likely drives the LFO/envelope phases from `current_sync_word()` rather than free-running. The public reference code's effects are free-running; not yet sync-locked.

**Q: Where are the biquad coefficients?**
A: `audiothingies/effects/DspTables.hpp` — `kButterworthLowpassTable` for the filter and distortion post-filter, `kGateLevels` for gate variations, `kDelayWetProfiles` for delay variations.

**Q: Why a `current_mix_gain_` in the distortion node?**
A: Crossfades the effect on/off over an audio block. Prevents the click that would happen if the distortion's high-gain output snapped on without ramping.

**Q: How does the rack handle effect changes?**
A: Spinlock-protected state with FadeIn/FadeOut/Steady phases. `set_effect_state()` is thread-safe. The next render block applies either fade-in of the new state or fade-out of the previous, depending on direction.

**Q: What about pitch shift / bit crusher / reverb?**
A: **Not in the reference code.** Stock TE firmware doesn't have these. virtualflannel_46386's custom OS adds beat repeat, bit crusher, send delay, filter with performance modulation, and pitch ±12 cents — but that's separate code, not yet released. See `20-custom-firmware-state.md`.

**Q: What's the `wet_enabled` flag for?**
A: Lets the rack run the DelayEcho node as either a recording-only buffer (`wet_enabled=false`, used by FILTER/DISTORTION/GATE for the always-on tap) or as an active effect (`wet_enabled=true`, used by CHORUS_DELAY variations 2/3).

**Q: How is the LFO phase advanced in the public code?**
A: Free-running based on sample count and `lfo_rate_hz()`. Stock firmware presumably uses sync word instead. To make custom firmware behave like stock, replace the free-running LFO with sync-word-driven phase.
