# FAQ — MIDI Timing Encoding

**Q: Does the SP-1 emit MIDI clock?**
A: Yes — stock TE firmware emits `0xF8` MIDI clock pulses at 31,250 baud, 128 µs pulse width, derived from on-disk timing data. The public `sp1-midi` BSP uses USB MIDI 2.0 + its own free-running clock; it doesn't emit on-disk-derived clock. See `10-midi-timing-encoding.md`.

**Q: What's encoded in each sector's side data?**
A: Four 32-bit "sync words" (one per TE-block) plus four 32-bit LED words. The sync word's low 16 bits hold a counter (0–24,489) representing position in a musical cycle of 96 ticks (= 4 × 24 PPQN).

**Q: Why 24 PPQN?**
A: 24 Pulses-Per-Quarter-Note is the **standard MIDI clock resolution** from the MIDI 1.0 spec. 4 × 24 = 96 ticks per bar (4 beats × 24 PPQN per beat).

**Q: How do I compute BPM from the counter?**
A: 
```
samples_per_tick = 30000 / BPM
```
or equivalently:
```
BPM = 30000 / samples_per_tick = 30000 / (48000 / (BPM × 96 / 60))
```

**Q: How fast does the 16-bit counter wrap?**
A: At 120 BPM, one counter cycle = 60/120 × 96 = 48 seconds. So the 16-bit space (max 65535) wouldn't wrap within a typical song. Actual max counter value seen is 24,489 — suggesting the counter is reset at song or section boundaries.

**Q: What effects depend on timing data?**
A: **Looping, gate, delay, and 2 of 4 LPF presets** [per TimK, paraphrased]. Without correct sync words, custom audio still plays but those effects won't be musically synchronized.

**Q: How do I access the sync word in custom firmware?**
A: 
```cpp
uint32_t sync = audio_engine.current_sync_word();
uint16_t counter = sync & 0xFFFF;
```
[code: `audiothingies/AudioEngine.hpp` line 67]

**Q: How often does the sync word update?**
A: **Every TE-block** = every 85 frames ≈ 1.77 ms. Much finer than the MIDI tick rate, so you can determine which tick is current with sample-accuracy.

**Q: What do the upper 16 bits of the sync word encode?**
A: **Not fully documented.** The lower 16 bits hold the counter; the upper 16 bits presumably carry flags (song boundary? beat-1-of-4? swing direction?). Investigate by dumping sync words from a real album. See `known-unknowns.md`.

**Q: Can I produce custom albums without timing data?**
A: You can leave sync words zero — the audio will play correctly but the tempo-synced effects will be off. For musically correct stock-firmware effects, generate counter values matching your song's tempo.

**Q: Where does the 128 µs pulse width come from?**
A: 31,250 baud × 1 byte (the 0xF8 byte) = 0.32 ms per byte = 320 µs. But the pulse-on-time is only 8 bits × ~12.8 µs each = 102.4 µs roughly. Galapagoose measured 128 µs which matches the byte transmission time for 0xF8 plus start/stop bits.

**Q: Are tempo changes within a song supported?**
A: Unknown / unverified. Probably yes — counter advance rate per sector is independent. But no public example of an album with a tempo curve exists for confirmation.
