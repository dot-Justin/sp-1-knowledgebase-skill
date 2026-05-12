# Audio Format Spec

**Synthesized through:** Lines #846 (2026-05-06), Discord through 2026-05-11. Code-of-record: `storagethingies/DiskManager.hpp` (the `decode_te_frame_payload_i32` function is the authoritative byte-layout reference).

This is **the most important reference for understanding the SP-1.** The on-disk audio sample byte layout is not obvious — it's an interleaved layout where neighboring bytes belong to different samples. AI summaries have produced wrong claims about this format (`"24-bit PCM little-endian"`) and TimK has explicitly called those out as hallucinations.

This file describes the actual format, byte-by-byte, with code references.

## High-level format

| Property | Value |
| --- | --- |
| Sample rate | **48 kHz** |
| Bit depth | **24-bit signed** (per channel per stem) |
| Stems | **4** |
| Channels per stem | **2** (stereo: L, R) |
| Total channels per frame | **8** (4 stems × 2 channels) |
| Bytes per channel per frame | **3** |
| Bytes per stem per frame | **6** (2 channels × 3 bytes) |
| Bytes per frame | **24** (4 stems × 6 bytes) |
| Frames per TE sector | **340** |
| TE sector size | **8192 bytes** (16 native eMMC blocks × 512 bytes) |
| Audio bytes per sector | 340 × 24 = **8160 bytes** |
| Non-audio bytes per sector | 8192 − 8160 = **32 bytes** — split as **8 bytes of side-data per TE-block** × 4 blocks [TKT wiki: Audio-format, accessed 2026-05-12]

The 8 side-data bytes per TE-block are: **2 bytes sync counter + 2 bytes tempo + 4 bytes LED data** [TKT wiki: Audio-format, accessed 2026-05-12]. See `10-midi-timing-encoding.md` for what those fields mean.

## Frame structure on disk

One frame contains **24 bytes** = 4 stems × 6 bytes/stem. Stems are stored sequentially in stem order:

```
Byte offset within frame:
  0   1   2   3   4   5    6   7   8   9  10  11    12  13  14  15  16  17    18  19  20  21  22  23
 [── stem 0 (6 bytes) ──] [── stem 1 (6 bytes) ──] [── stem 2 (6 bytes) ──] [── stem 3 (6 bytes) ──]
```

## Sample layout within a stem

This is the part that confuses everyone. **Within a single stem's 6 bytes, the bytes for left and right channels are interleaved in a specific non-obvious order.**

From `storagethingies/DiskManager.hpp` lines 65–82 — this is the authoritative decoder:

```cpp
inline void decode_te_frame_payload_i32(const uint8_t* frame_payload,
                                        DecodedStemFrameI32* frame_out) {
    for (size_t stem = 0; stem < kStemsPerFrame; ++stem) {
        const uint8_t* data = frame_payload + stem * kStemFrameSize;

        frame_out->left[stem]  = (static_cast<int32_t>(static_cast<int8_t>(data[1])) << 16) |
                                 (static_cast<uint32_t>(data[0]) << 8) |
                                  static_cast<uint32_t>(data[3]);

        frame_out->right[stem] = (static_cast<int32_t>(static_cast<int8_t>(data[2])) << 16) |
                                 (static_cast<uint32_t>(data[5]) << 8) |
                                  static_cast<uint32_t>(data[4]);
    }
}
```

Reading that backwards: per-stem byte index → which sample bit it carries:

| Byte index (within stem) | Sample | Bits | Notes |
| --- | --- | --- | --- |
| `data[0]` | **Left** | bits 15:8 (mid byte) | Unsigned, becomes the middle byte of left sample |
| `data[1]` | **Left** | bits 23:16 (MSB) | Signed (cast `int8_t` → sign extended into bits 31:16) |
| `data[2]` | **Right** | bits 23:16 (MSB) | Signed |
| `data[3]` | **Left** | bits 7:0 (LSB) | Unsigned |
| `data[4]` | **Right** | bits 7:0 (LSB) | Unsigned |
| `data[5]` | **Right** | bits 15:8 (mid byte) | Unsigned |

In other words, **the 6-byte stem layout is:**

```
Byte:    0           1           2           3           4           5
Sample:  L_mid       L_msb       R_msb       L_lsb       R_lsb       R_mid
         (8-15)      (16-23)     (16-23)     (0-7)       (0-7)       (8-15)
```

**The MSBs of left and right sit next to each other in bytes 1 and 2.** This is not little-endian, not big-endian — it's a custom interleaved layout. Calling it "little-endian 24-bit PCM" is wrong on multiple counts: the byte ordering within a sample is `mid, msb, lsb` for left and `msb, mid, lsb` (kind of) for right, and the left and right samples themselves are interleaved at the byte level.

The signed-ness comes from the cast: bits 23:16 are cast as `int8_t` first (sign-extending into the upper bits of `int32_t`), then shifted left by 16. This gives a 24-bit signed value stored in the lower 24 bits of an `int32_t`. See the **in-memory** PCM representation section below.

## Why this layout?

It's optimized for the on-disk block-skipping trick. Within a frame, the higher-order bytes of both channels (which carry the most audible information) sit next to each other. This isn't directly exploited by the decoder, but the design likely reflects:

1. **Hardware-friendly DMA patterns** — the I²S TX path takes specific bytes in a predictable order
2. **TE-internal tooling** — the audio encoder TE used to produce these files probably wrote samples in this order natively
3. **Possibly historical reasons** — early-stage firmware decisions calcified into the format

There is no public documentation from TE explaining why; the layout was reverse-engineered.

## In-memory PCM representation (after decoding)

Once decoded, each sample becomes an `int32_t` with the 24-bit value **sign-extended into bits 31:24** and packed into the lower 24 bits otherwise. Maximum positive value: 8,388,607 (= 2^23 − 1). Maximum negative: -8,388,608.

For converting **between** float and packed 24-bit values, `audiothingies/PcmPacking.hpp` provides:

```cpp
inline int32_t float_to_pcm_right24_fast(float sample) {
    // clamp to [-1.0, 1.0]
    return static_cast<int32_t>(sample * 8388607.0f);
}
```

[code: `audiothingies/PcmPacking.hpp`]

The function name `_right24_fast` tells you everything: **right**-aligned, **24-bit**, **fast** (no branch on overflow except for NaN-handling). This is the in-memory format the audio engine uses internally and the format that gets sent over I²S to the codecs.

**Important distinction:**

- **On-disk format** = 6 bytes per stem, interleaved order as above
- **In-memory format** = `int32_t` per channel per stem, right-aligned 24-bit
- **I²S wire format** = depends on I²S peripheral configuration (typically 32-bit slots with 24-bit right-aligned samples in the lower bits)

The decoder in `storagethingies` translates from the on-disk format to in-memory at read time. The audio mixer then operates on the in-memory representation; the I²S TX backend translates to the wire format.

## Stem assignment

The four stems are conventionally:

1. **Stem 0** — drums
2. **Stem 1** — bass
3. **Stem 2** — melody / instruments
4. **Stem 3** — vocals

This is convention rather than enforced — there's nothing in the format spec that says "stem 0 must be drums." TE's stock firmware applies effect routing based on this convention (e.g., certain effects may sound better on certain stems). For custom stems, you control which audio goes in which slot when you encode the album image.

The four track LEDs and four faders correspond to these four stems in order. The "Track N" buttons mute/solo stem N − 1.

## Sample rate and clock derivation

48 kHz sample rate × 64 BCLK cycles per LRCK period = **3.072 MHz BCLK**.

The 3.072 MHz clock comes from an external oscillator enabled via P0.13 [`stem_player.dts` lines 391–400]. This oscillator is dedicated to audio timing — it's not shared with the nRF's system clock. The nRF's I²S peripheral acts as bus master, dividing this reference.

Why 64 BCLK cycles per LRCK? Because 32 bits per channel × 2 channels = 64 bits per stereo sample period. Even though only 24 bits of each 32-bit slot carry sample data, the slot size is 32 bits for I²S framing reasons.

## Channel multiplexing on the wire

The nRF transmits all 8 channels (4 stems × 2) on a single SDOUT wire. The codecs each consume a subset:

- **CS42L42** (headphone): consumes stereo pair from the mixed/processed audio
- **TAS2505** (speaker): consumes mono summation (typically L+R or just L)

The selection happens via codec register configuration (`SP_RX_CH_SEL = 0x2501` on CS42L42). Both codecs are driven by the same BCLK/LRCK from the nRF — they synchronize on the frame and pick their slots.

The "8 channels on I²S" claim refers to the TDM-style multiplexing where 8 channels share one wire. The CS42L42 and TAS2505 also support standard 2-channel I²S; the SP-1 uses 8-channel mode to deliver all stems simultaneously to the audio chain (though in practice the mixing happens in nRF firmware, so the codecs see post-mix audio).

## Frame address arithmetic

When the audio engine wants to play frame N (where N counts from 0 at the album's start), it computes:

```
te_sector_index_in_album = N / 340
frame_in_sector          = N % 340
```

Then it maps `te_sector_index_in_album` to the actual eMMC TE-sector index via the album header (which records each song's `start_sector` and `length_sectors`). See `12-audio-engine-internals.md` for the full address resolution in `DiskManager::resolve_album_frame()`.

## What the format does NOT include

A few things developers expect but won't find:

- **No timecode / sample rate marker.** The format implicitly is 48 kHz; the firmware knows. If you mis-clock playback, the audio will sound pitched.
- **No CRC / checksum per sector.** The eMMC provides its own CRC at the protocol level, but there's no application-level integrity check.
- **No compression.** The audio is raw uncompressed PCM. A typical album is ~311 MB for ~30 minutes (4 stems × stereo × 48000 × 24 bits / 8 / 60 ≈ 86 MB/min × 30 min × 0.12 ≈ 311 MB; ballpark matches moecal1947's 311 MB number [Discord #general, 2026-05-09 09:33]).
- **No fade-in/fade-out.** Songs start abruptly at sector boundaries. Stock TE firmware presumably applies short crossfades on song transitions in playback code.
- **No EQ / dynamics metadata.** All processing is in the engine; the audio file is dry source material.

## Album header (first sector)

The first 8192-byte sector of an album is reserved for the **album header** [TKT wiki: Data-Structure, accessed 2026-05-12]. Audio data begins at offset `0x2000` (sector 1).

The album header layout per `TKT wiki: Album-metadata-format` (accessed 2026-05-12):

| Offset | Length | Field | Notes |
| --- | --- | --- | --- |
| 0 | 13 | Magic bytes `ALBUM_PRESENT` | Must match exactly. Required for the album to be considered valid. |
| 13 | 4 | Album length (in 0x2000-byte sectors) | Total album sector count |
| 17 | 1 | Number of songs | Song count |
| 18 | 64 | Album title | Null-terminated, padded with ASCII `X` |
| 82 | N × 136 | Song entries | One per song, layout below |

**Each song entry is 136 bytes:**

| Offset (within entry) | Length | Field | Notes |
| --- | --- | --- | --- |
| 0 | 4 | Song start offset (in 0x2000-byte sectors) | Where the song's audio begins on disk |
| 4 | 4 | Song length (in sectors) | |
| 8 | 64 | Artist name | Null-terminated, padded with `X` |
| 72 | 64 | Song title | Null-terminated, padded with `X` |

**Album validity:** the magic bytes `ALBUM_PRESENT` must appear both at offset 0 **and** at the very last bytes of the album image [TKT wiki: Album-metadata-format]. The trailing magic is a second integrity check.

**Cross-reference with `storagethingies/DiskManager.hpp`:** the C struct declares `artist[65]` and `title[65]` whereas the on-disk fields are 64 bytes. The extra byte in the C struct is for a null terminator in memory; on disk the field is exactly 64 bytes. Do not write 65 bytes per name field.

**Endianness:** integer fields are presumed little-endian to match the nRF52840 (not explicitly stated in the wiki). Verify against a real album image before committing an encoder.

**Caveat:** this layout was extracted via WebFetch summary of the TKT wiki page. Numbers above match a self-consistent interpretation (13 + 4 + 1 + 64 = 82 ✓; 4 + 4 + 64 + 64 = 136 ✓). Verify directly against `github.com/timknapen/SP-1-dev/wiki/Album-metadata-format` before relying on offsets for production code.

## Encoding custom stems

To produce a custom album:

1. **Separate stems** from a song (HT Demucs, Spleeter, or any source-separation tool). emvee1968 uses HT Demucs FT [Discord #firmware, 2026-05-08 23:54].
2. **Resample** each stem to 48 kHz stereo if not already.
3. **Quantize** to 24-bit (right-aligned in 32-bit container).
4. **Frame-by-frame**, write each sample's bytes in the interleaved order documented above.
5. **Pack 340 frames per sector**, write 16 native eMMC blocks of 512 bytes per sector.
6. **Write the album header** (with song offsets) to the first sectors.
7. **Add timing side-data** to each sector — see `10-midi-timing-encoding.md` for the MIDI clock counter that needs to be embedded.
8. **Upload** via the USB protocol (`16-usb-upload-protocol.md`) or write directly to the eMMC if you have hardware access.

Step 7 is non-trivial — getting the timing data right is what makes the **tape FF/RW and looping effects work**. See `11-block-interleaving-tape-fx.md`. Without correct timing data, the device plays the audio but the effects don't sync musically.

**Canonical workflow status:** TimK has not yet published a definitive end-to-end stem-prep tool. emvee1968 and virtualflannel_46386 each have their own pipeline but neither is released. moecal1947 is building the upload side. See `known-unknowns.md` and `references/21-original-firmware-stems.md`.

## Where to go next

- For the eMMC sector / block layout context → `08-emmc-storage.md`
- For the MIDI clock counter encoded in the side-band data → `10-midi-timing-encoding.md`
- For the 0/2/1/3 block interleaving (why frames are laid out the way they are in each sector) → `11-block-interleaving-tape-fx.md`
- For the in-memory mixing and resampling → `12-audio-engine-internals.md`
- For the DSP effects that consume the decoded samples → `13-dsp-effects.md`
- For the USB upload of an album → `16-usb-upload-protocol.md`
- For canonical stem-prep workflow → `21-original-firmware-stems.md`
- For why "24-bit little-endian PCM" is wrong → `corrections.md` and `hallucination-watchlist.md`
