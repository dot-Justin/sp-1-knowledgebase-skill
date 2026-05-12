# Original-Firmware Custom Stems

**Synthesized through:** Lines #846 (2026-05-06), Discord through 2026-05-11, TKT wiki Album-metadata-format / Audio-format / Data-Structure pages accessed 2026-05-12.

**The most useful single finding for end users:** you do **not need custom firmware** to play your own audio on the SP-1. Stock TE firmware plays any correctly-formatted album image. TimK confirmed this in early 2026 [Lines #799–805]:

> Custom stems work with original TE firmware unmodified.

This file describes the path from "I have a song" to "the song plays on my unmodified SP-1." It synthesizes the available format spec, the upload procedure, and the gaps in the canonical workflow.

## Status: canonical workflow not yet published

As of synthesis date, **no single end-to-end tool exists** that takes a song and produces an album playable on stock TE firmware. The pieces exist:

- **Stem separation:** HT Demucs, Spleeter, MusicNN, beat_this etc. (well-established external tools)
- **Format spec:** documented in this skill (`09-audio-format-spec.md`, `11-block-interleaving-tape-fx.md`)
- **Album image construction:** emvee1968 and virtualflannel_46386 each have internal scripts; not yet released
- **Upload protocol:** documented as opcode sequence (`16-usb-upload-protocol.md`); a fast public client doesn't exist yet
- **Web tool:** `solderless.engineering` (when online) accepts album images via the upload protocol

What you can do *today* with publicly available pieces is build your own pipeline. What you can't do (yet) is `pip install sp1-stems` and have it work.

## The high-level pipeline

```
Source song (mp3, wav, anything)
   ↓ (stem separation)
4 stems: drums, bass, melody, vocals (each stereo, 44.1 / 48 / 96 kHz)
   ↓ (resample if needed)
4 stems at 48 kHz, stereo, 24-bit
   ↓ (frame-pack into TE byte order)
Frames in the L_mid / L_msb / R_msb / L_lsb / R_lsb / R_mid interleaved layout per stem
   ↓ (sector-pack with 0/2/1/3 block interleaving)
8192-byte sectors with 340 frames each in the FF-friendly layout
   ↓ (generate sync words for each TE-block from BPM)
Sectors with timing metadata
   ↓ (assemble album header with song offsets and titles)
Complete album image
   ↓ (USB upload to SP-1)
Album written to eMMC
   ↓ (boot SP-1 normally)
Album plays with full effects
```

Each step has documented details in this skill. The challenge is **gluing them into a single tool**.

## Per-step status

### 1. Stem separation

External tools, well-supported:
- **HT Demucs FT** (emvee1968 uses this) — high-quality 4-stem separation
- **Demucs** (Facebook AI Research) — older but still solid
- **Spleeter** (Deezer) — fast, 4-stem version available
- **Open-source notebooks** (Colab, Hugging Face Spaces) — for users without GPU

Output: 4 WAV files at the original song's sample rate.

### 2. Resampling

Use `ffmpeg`, `sox`, or any audio library. Target: 48 kHz, stereo, 24-bit signed.

```sh
ffmpeg -i drums.wav -ar 48000 -ac 2 -sample_fmt s32 drums_48k.wav
```

(Note: ffmpeg's `s32` is 32-bit signed; the 24-bit data sits in bits 23:0. This matches the SP-1's right-aligned in-memory format. Conversion to the on-disk byte layout happens later.)

### 3. Frame packing — the SP-1's interleaved byte order

For each frame (24 bytes total = 4 stems × 6 bytes), the bytes per stem are:

```
Byte offset:   0       1       2       3       4       5
Carries:       L_mid   L_msb   R_msb   L_lsb   R_lsb   R_mid
```

[code: `storagethingies/DiskManager.hpp::decode_te_frame_payload_i32`]

In pseudocode for one stem:

```python
def pack_stem_frame(left_i32, right_i32):
    # left_i32 and right_i32 are 24-bit signed values, sign-extended into int32
    # Convert to 3 unsigned bytes each, then interleave
    L_lsb = left_i32 & 0xFF
    L_mid = (left_i32 >> 8) & 0xFF
    L_msb = (left_i32 >> 16) & 0xFF
    R_lsb = right_i32 & 0xFF
    R_mid = (right_i32 >> 8) & 0xFF
    R_msb = (right_i32 >> 16) & 0xFF
    return bytes([L_mid, L_msb, R_msb, L_lsb, R_lsb, R_mid])
```

You write 4 stems sequentially in stem order (0 = drums, 1 = bass, 2 = melody, 3 = vocals) to produce 24 bytes per frame.

### 4. Sector packing — 0/2/1/3 block interleaving

Each 8192-byte sector has 4 TE-blocks of 2048 bytes (85 frames each). Frames are distributed:

```
Block 0 → frames where frame_index_within_sector % 4 == 0   (0, 4, 8, ...)
Block 1 → frames where frame_index_within_sector % 4 == 2   (2, 6, 10, ...)
Block 2 → frames where frame_index_within_sector % 4 == 1   (1, 5, 9, ...)
Block 3 → frames where frame_index_within_sector % 4 == 3   (3, 7, 11, ...)
```

The physical disk order of these blocks is `[Block 0, Block 1, Block 2, Block 3]`. See `11-block-interleaving-tape-fx.md` for details.

**Don't skip this step.** Sectors stored in straight frame order play correctly at 1.0x but the FF/RW will be broken (audio sounds like discontinuous chunks instead of fast-forward).

### 5. Per-block side data (sync + tempo + LED)

Each TE-block has an **8-byte trailer** appended after its 2040 bytes of audio [TKT wiki: Audio-format, accessed 2026-05-12]:

```
Offset within block's 8-byte trailer:
  0–1   sync counter   (16-bit, presumed little-endian)
  2–3   tempo          (16-bit; encoding not publicly documented)
  4–7   LED data       (32-bit)
```

× 4 TE-blocks per sector = 32 bytes of side data per sector. Audio occupies 8160 bytes (340 × 24); side data occupies the remaining 32.

For correct tempo-synced effects (`10-midi-timing-encoding.md`):

```python
def sync_counter_for_block(sample_position_in_song, song_bpm):
    # tick rate: 96 ticks per bar at song_bpm BPM
    # samples per tick: 30000 / song_bpm
    return int(sample_position_in_song / (30000 / song_bpm)) % 24489
```

Generate 4 such counters per sector (one per TE-block; sample positions advance by 85 frames between blocks). Write each as bytes 0–1 of the block's trailer.

**Tempo encoding is not publicly documented.** First-pass options:
- Leave tempo bytes zero and see whether stock firmware drives the synced effects from the sync counter alone
- Reverse-engineer by dumping a known-BPM song's tempo bytes from a real album

For a first-pass encoder, you can leave sync counter zero too — audio will play but effects won't be synchronized.

### 6. LED words

Same treatment as sync words. The 4 bytes of each LED word presumably encode brightness levels for the 4 Track LEDs at that block's playback time. Leaving zero gives you "no LED animation"; the LEDs will sit at whatever brightness the firmware defaults to.

For a v1 encoder, leaving LED words zero is acceptable.

### 7. Album header

The first 8192-byte sector of the album is reserved for the album header [TKT wiki: Data-Structure, accessed 2026-05-12]. Audio starts at offset `0x2000` (sector 1).

On-disk layout per `TKT wiki: Album-metadata-format` (accessed 2026-05-12):

```
Offset  Length      Field
0       13          Magic bytes 'ALBUM_PRESENT' (ASCII, no terminator)
13      4           Album length in 0x2000-byte sectors
17      1           Number of songs
18      64          Album title (null-terminated, padded with 'X')
82      N × 136     Song entries

Each song entry (136 bytes):
0       4           Song start offset in 0x2000-byte sectors
4       4           Song length in sectors
8       64          Artist (null-terminated, padded with 'X')
72      64          Title  (null-terminated, padded with 'X')
```

**Validity check:** the magic `ALBUM_PRESENT` must appear at offset 0 **and** as the very last bytes of the album image. The trailing magic is required for the album to be considered valid.

This supersedes the earlier reference to the in-memory `AlbumInfo` C struct from `storagethingies/DiskManager.hpp`. The struct's `[65]` name fields include a C null terminator; the on-disk fields are 64 bytes. Match the wiki layout, not the C struct.

**Endianness:** integer fields are presumed little-endian (nRF52840 native). Not explicitly confirmed in the wiki — verify by inspecting a real album.

### 8. Upload

Once you have an album image, push it via the USB upload protocol — see `16-usb-upload-protocol.md`. Path 1: wait for solderless.engineering. Path 2: write your own client (the bootloader protocol opcodes are documented; the per-packet ACK format is not yet public). Path 3: use moecal1947's slow Python uploader (~4.5 days for a 311 MB album).

## What works on stock firmware vs custom firmware

| Behavior | Stock TE firmware | Custom firmware |
| --- | --- | --- |
| Plays 4 stems at 48 kHz | Yes | Depends on the firmware — see `20-custom-firmware-state.md` |
| Per-stem fader mixing | Yes | Stock-equivalent only via `audiothingies/StockRuntimeMixer` |
| Track button mute/solo | Yes | Yes |
| FF/RW at 2.5x, 4x, 8x, 16x | 2.5x verified; higher rates documented in code | Custom firmwares typically max at 2.0x without block-skipping |
| Tape FX (smooth pause/play bend) | Yes | Yes in `audiothingies/VarispeedResampler`-based firmware |
| Filter, distortion, gate, echo effects | Yes, all 4 with 4 variations | Varies — emvee has gate; virtualflannel has different custom effects |
| Tempo-synced effect modulation | Yes (uses sync words) | Not implemented in any public firmware yet |
| Bluetooth audio output | Yes (presumed, not formally verified) | emvee has pairing working; full audio over BT not yet |
| LED animations synced to song | Yes (uses LED words on disk) | Likely not in custom firmwares |

**The path of least resistance:** prepare a custom album image and play it on stock firmware. You get all the effects, the proper tape FX, tempo sync, and LED sync — without writing any firmware. The only thing you give up is the ability to load *your own* effect algorithms.

**The path of maximum control:** write custom firmware. You can do anything but you have to implement everything from scratch (or use `audiothingies/storagethingies` as the foundation).

## The "step 4 nonsense"

[Context: tkt1000 called out a fabricated "step 4" in a stem-prep workflow as nonsense [Discord #general, 2026-05-10 22:56]]

As of synthesis date there is no canonical numbered workflow that has been blessed by TimK or any other authority figure. Different community members have different pipelines. **Do not present a canonical numbered workflow that has implementation details you can't cite.** The high-level pipeline (separation → resample → encode → upload) is real; specific tool choices and step ordering depend on the pipeline being built.

When the canonical workflow is published (presumably alongside ericlewis's "proper BSP" release), this file should be updated to point at it.

## Tools to watch for

The following are **expected to be public soon** (as of synthesis date):

- **virtualflannel_46386's custom OS** — has working stem-prep; awaiting solderless return
- **emvee1968's HT Demucs pipeline** — awaiting two more units
- **ericlewis's "proper BSP"** — in development, no public ETA
- **An updated solderless.engineering** — offline for an update as of 2026-05-09
- **moecal1947's upload tool** — published or shared in some form, awaiting faster packet framing

## Where to go next

- For the on-disk format byte-by-byte → `09-audio-format-spec.md`
- For block interleaving → `11-block-interleaving-tape-fx.md`
- For sync word / timing details → `10-midi-timing-encoding.md`
- For the upload protocol → `16-usb-upload-protocol.md`
- For unreleased custom firmwares with relevant pipelines → `20-custom-firmware-state.md`
- For open questions (header magic value, side-data layout) → `known-unknowns.md`
