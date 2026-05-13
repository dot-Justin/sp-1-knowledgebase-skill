# MIDI Timing Encoding

**Synthesized through:** Lines #846 (2026-05-06), Discord through 2026-05-11, TKT wiki Audio-format page accessed 2026-05-12, and **solderless source archive** ingested 2026-05-13. Code-of-record for stock-firmware-read side: `assets/storagethingies-2026-05-09/DiskManager.hpp` `block_sync_words` field; `assets/audiothingies-2026-05-09/AudioEngine.hpp` `current_sync_word()`. Code-of-record for encoder-write side: `wav-parser.js::encodeToSP1` in solderless archive. See `update-log.md` 2026-05-13.

**Important clarification (2026-05-13):** The TKT wiki and `storagethingies` describe what **stock TE firmware reads** from each sector — up to **8 bytes of side data per TE-block**, split as 2 sync + 2 tempo + 4 LED, × 4 blocks = 32 bytes per sector. But the **solderless encoder writes only 6 bytes per sector**, at the end of block 0: 2 bytes tempo (uint16 LE) + 4 bytes per-stem envelope (uint8). The encoder writes **no sync counter and no LED data**. Custom albums produced by solderless still play correctly with stock firmware effects, which means either (a) most of the wiki-described side data is optional from the firmware's perspective, or (b) stock firmware uses default zero values where data is absent. Treat the wiki layout as a **read spec** (what the firmware understands), not a **write spec** (what the encoder must produce). See `corrections.md` 2026-05-13 and `references/21-original-firmware-stems.md` for the encoder's reduced 6-byte trailer.

The rest of this file describes what's known about the timing-data encoding from the wiki / audiothingies perspective. For custom encoder authoring, see `references/21-original-firmware-stems.md`.

The SP-1 stock firmware embeds **MIDI clock timing data directly into the audio stream** on the eMMC. Each TE-block (4 per sector) can carry **4 bytes of musical-timing side data** — split as a 16-bit sync counter and a 16-bit tempo field — that encodes the musical beat position [TKT wiki: Audio-format, accessed 2026-05-12]. This isn't decoration — the device's effects (looping, gate, delay, two of the LPF presets) **depend** on this timing data to sync musically with the audio.

**Note on terminology:** earlier synthesis described the per-block timing payload as a single "32-bit sync word." The TKT wiki splits this into **2 bytes sync + 2 bytes tempo** per block (followed by 4 bytes of LED data). The `assets/storagethingies-2026-05-09/DiskManager.hpp` `block_sync_words[4]` array packs both fields together as a `uint32_t`. See `corrections.md` for the reconciliation.

This file documents what is known about the encoding, how the firmware uses it, and what stem-encoding tools must produce.

## The high-level story

[Source: Galapagoose's resolution at Lines #739, plus JoseJX's earlier decoding work in #585–610 + #625]

| Concept | Value |
| --- | --- |
| Timing data carrier | A 16-bit counter embedded in the per-block side data |
| Counter range | 0 to 24,489 (one full song or bar cycle) |
| Steps per cycle | **96 steps** |
| Step encoding | 4 × 24 = 96 steps per cycle = 4 quarter notes × 24 PPQN (standard MIDI Pulse-Per-Quarter-Note) |
| MIDI clock byte | `0xF8` |
| Pulse width at 31,250 baud | 128 µs per `0xF8` pulse |

So the timing counter is a position-in-cycle indicator that lets the firmware (and any external tools the firmware drives, like MIDI clock output) know exactly where in the musical bar the current audio frame lies.

## Why 4 × 24 PPQN?

24 pulses per quarter note (PPQN) is the **standard MIDI clock resolution** — it's been the convention since the original MIDI 1.0 spec. A 4/4 bar at 24 PPQN per beat is 96 ticks total. The SP-1 chose 96 ticks per cycle so its on-disk timing aligns with hardware MIDI clock output.

The `0xF8` byte is the **MIDI System Real-Time "Timing Clock"** byte. External MIDI gear, when connected to the SP-1's stock firmware, will see one `0xF8` byte at 31,250 baud (standard MIDI baud rate) corresponding to each tick of the on-disk timing counter. 128 µs per pulse at 31,250 baud matches the byte duration for a single `0xF8`.

## Counter-to-tempo derivation

The counter ticks at one step per `1 / (BPM × 96)` minutes. Given a measured spacing in audio frames between adjacent counter increments, the firmware (and you) can derive the BPM:

```
samples_per_tick = 48000 / (BPM × 96 / 60)
                 = (48000 × 60) / (BPM × 96)
                 = 30000 / BPM
```

So at 120 BPM, one tick = 250 samples = ~5.2 ms. The 16-bit counter range of 24,489 wraps around in about 6.4 minutes at 120 BPM — long enough for a single song, the counter presumably resets at each song boundary.

## Where the timing data lives in the sector

**On disk (per TE-block, 8 bytes after audio):** [TKT wiki: Audio-format, accessed 2026-05-12]

```
Offset within block's side-data:
  0–1   sync counter   (16 bits)
  2–3   tempo          (16 bits)
  4–7   LED data       (32 bits)
```

Each of the 4 TE-blocks in a sector has this 8-byte trailer, totaling 32 bytes of side data per sector. Audio occupies bytes 0–2039 of each 2048-byte TE-block; side data occupies bytes 2040–2047.

**In memory after read** [code: `assets/storagethingies-2026-05-09/DiskManager.hpp` — `AudioSlot::Record`:]

```cpp
struct Record {
    uint8_t  payload[kSectorSize];                  // 8192 bytes (340 audio frames + per-block trailers)
    uint32_t block_sync_words[kBlocksPerSector];    // 4 per sector — packs (tempo:16, sync:16) or similar
    uint32_t block_led_words[kBlocksPerSector];     // 4 LED words per sector
};
```

The `uint32_t` in `block_sync_words` is the same 4 bytes that the wiki splits into sync+tempo. Bit-field ordering within the `uint32_t` (which 16 bits are sync, which are tempo, endianness) is **not** publicly documented and should be verified by inspecting `try_get_block_side_data_cached()` or a real album image's bytes. The audio engine queries `try_get_block_side_data_cached()` to retrieve the current block's sync word during playback.

[code: `assets/audiothingies-2026-05-09/AudioEngine.hpp` line 67:]

```cpp
uint32_t current_sync_word() const;
```

This is the engine's public accessor for "where am I in the musical cycle right now." It returns the sync word for the block being rendered at the current frame index.

The same `Record` structure also holds `block_led_words` — see `12-audio-engine-internals.md` for how the LEDs animate from this data.

## What the 4 bytes of timing data contain

Per TKT wiki [Audio-format, accessed 2026-05-12], the per-block timing payload is:

- **Sync counter (16 bits, bytes 0–1):** the 0–24,489 musical-position counter per Galapagoose's analysis [Lines #739]
- **Tempo (16 bits, bytes 2–3):** a separate tempo field. Exact encoding (BPM × N, fixed-point, something else?) is **not** publicly documented as of synthesis date.

This supersedes earlier synthesis which described the 4 bytes as a single 32-bit sync word with unknown upper 16 bits. See `corrections.md`.

The fact that there are **4 separate sync+tempo payloads per sector** (one per TE-block) means timing updates at a granularity of 85 frames ≈ 1.77 ms — fine enough to drive sample-accurate musical effects.

**Still unknown:**
- Whether stock firmware actually uses a separate per-block sync counter (since solderless-produced albums omit it and still play with synced effects).
- The exact unit of the 16-bit tempo field as the *firmware reads* it. **Update 2026-05-13:** the solderless encoder writes tempo as `(48000 * 60) / (24 * bpm)` (a samples-per-something derived count; for 120 BPM = 1000, for 80 BPM = 1500, for 60 BPM = 2000). Whether the firmware interprets this as samples-per-tick, samples-per-bar/N, or a free-form scalar is not stated; the value produces working playback so the formula is reliable for encoders, but the firmware-side semantic isn't named.
- Byte ordering on disk (confirmed little-endian for the tempo field via the encoder using `bytes[2042] = tempo & 0xFF; bytes[2043] = (tempo >> 8) & 0xFF`).
- Whether the tempo field carries per-block tempo (allowing intra-song tempo curves) or just per-song tempo replicated across blocks. (Solderless writes the same value at every sector for a song.)

For exact bit-level decoding, the audio engine's `current_sync_word()` consumer in the effects path (see `13-dsp-effects.md`) is the practical reference.

## What effects depend on this data

Per TimK [paraphrased from Lines cross-references]:

> The embedded timing data is essential for looping, gate, delay, and 2 of the LPF presets.

Specifically:

- **Looping** — the firmware can loop back to a previous tick boundary (e.g., "loop the last 4 beats") by comparing current and target counter values
- **Gate effect (rhythmic mute)** — the gate's "open" / "closed" cycles align to beat divisions
- **Delay (echo)** — the delay time is musically synchronized (e.g., quarter-note delay) by deriving the sample count from the BPM
- **Two LPF variations** — variations 2 and 3 of the filter use an LFO modulation that is presumably synchronized to the tick counter rather than free-running. (Variation 0 and 1 use static cutoff frequencies. See `13-dsp-effects.md` BiquadFilter section.)

Custom firmware that doesn't read the sync word can implement these effects as free-running (un-synchronized) — which is what early custom firmwares do — but the **musical feel** will be off compared to stock.

## What encoding tools must produce

For a custom album to behave correctly under stock TE firmware (or any custom firmware that uses the timing data), an encoder must:

1. **Determine the song's BPM** (or BPM curve if it varies).
2. **Generate a sync counter value for each TE-block** — increment by `48000 × 60 / (BPM × 96)` samples per tick, accumulating modulo 24,489.
3. **Write the sync counter as bytes 0–1 of the 8-byte block trailer** (presumed little-endian; verify against a real album).
4. **Write a tempo value as bytes 2–3 of the trailer.** Encoding unknown — try a real album's bytes first. A safe first-pass guess for constant-tempo songs: write the same 16-bit value for every block in the song.
5. **Write 4 bytes of LED data as bytes 4–7** (zero is acceptable for v1; LEDs will sit at firmware-default brightness).
6. **Reset the counter at song boundaries** (TBD — verify by examining a real album's sync words).

For a first-pass encoder you can leave tempo bytes zero and see whether stock firmware still drives the tempo-synced effects from the sync counter alone. If effects glitch, the tempo field is required and you'll need to reverse-engineer its encoding.

## How custom firmware can read the sync word

If you're writing a custom playback engine and want the effects to musically sync:

```cpp
// From within an audio render callback
uint32_t sync_word = audio_engine.current_sync_word();
uint16_t counter   = sync_word & 0xFFFF;      // 16-bit position
float    bpm       = ... ;                    // derived from rate of change
uint8_t  tick_in_beat = counter % 24;         // 0..23 within the current beat
uint8_t  beat_in_bar  = (counter / 24) % 4;   // 0..3 within the bar
```

This kind of decomposition lets you synchronize effect parameters (LFO phase, gate envelope, delay time) to the musical position.

## What the public BSP does not do

`ericlewis/sp1-midi` is a MIDI **controller** — the SP-1 sends MIDI clock + faders + buttons to a host. It does **not** play audio. So it doesn't decode the sync words from the eMMC. The MIDI clock it generates on the USB MIDI 2.0 endpoint is derived from its own free-running timing (or from external input), not from on-disk sync words.

If you want a custom firmware that **plays stems** and **emits MIDI clock**, you would combine:
1. The audio engine in `audiothingies` (which reads sync words)
2. A MIDI clock generator that converts `current_sync_word()` → `0xF8` bytes at the appropriate timing
3. The USB MIDI infrastructure from `sp1-midi`

That combination doesn't exist in any public release yet.

## Timing precision

At 48 kHz sample rate, one tick at 120 BPM corresponds to 250 samples = ~5.2 ms. The sync word updates at TE-block granularity (every 85 frames = 1.77 ms). So the **internal resolution is finer than the MIDI tick rate** — you can sample-accurately know which tick the current frame belongs to, with a sub-tick fractional offset.

If you derive BPM by measuring the sample distance between adjacent counter increments, expect quantization at the 85-frame granularity. For more precision, average over multiple ticks.

## Practical implications

- **For custom firmware authors:** you don't need to handle timing data if your firmware doesn't implement musical-sync'd effects. But if you want a feature like "loop the last bar" or "tap delay synced to the song," reading `current_sync_word()` is the right path.
- **For custom album encoders:** generating plausible counter values is necessary if you want effects to feel musically correct on stock TE firmware. The simplest approach is "assume constant BPM, increment counter by `48000 × 60 / (BPM × 96)` samples." More sophisticated: detect actual beat positions via beat-tracking (librosa, beat_this, madmom) and write per-block counter values that match.

## What is not yet known

- **The exact encoding of the 16-bit tempo field.** Raw BPM? Q8.8? Sample-period? Investigate by dumping the bytes 2–3 of a few blocks from a known-BPM song in a real album.
- **Byte order on disk** for sync and tempo fields (presumed little-endian; not explicitly confirmed).
- **Whether tempo can vary per-block within a song** (it has its own field per block, which would *allow* it; whether stock firmware honors changes is a separate question).
- **How the counter behaves at song boundaries.** Does it reset to 0? Does it continue accumulating? Hint: probably resets, given that songs have independent BPMs.
- **What 0xF8 frequency the stock firmware emits.** Galapagoose confirmed the pulse width is 128 µs at 31,250 baud, but the rate (how often `0xF8` is sent) depends on tempo. At 120 BPM × 24 PPQN = 2880 pulses/minute = 48 Hz.
- **Whether tempo curves are supported.** Albums with tempo changes between songs are easy (each song has its own BPM). Albums with tempo *changes within* a song are harder — the counter must track this. No public evidence yet of whether stock TE firmware supports this.

These gaps are good "next move" questions for ericlewis or Galapagoose in Discord.

## Where to go next

- For the side-band LED data that lives alongside sync words in each sector → `12-audio-engine-internals.md`
- For the audio frame layout that the timing data accompanies → `09-audio-format-spec.md`
- For the effects that use the sync word → `13-dsp-effects.md`
- For the MIDI 2.0 USB output (separate from on-disk clock encoding) → `19-sp1-midi-bsp.md`
- For canonical custom-album encoding workflow → `21-original-firmware-stems.md`
