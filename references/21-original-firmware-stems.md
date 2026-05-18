# Original-Firmware Custom Stems

**Synthesized through:** Lines #846 (2026-05-06), Discord through 2026-05-11, TKT wiki Album-metadata-format / Audio-format / Data-Structure pages accessed 2026-05-12, the **solderless source archive** (ingested 2026-05-13), and the **solderless 2026-05-18 re-snapshot** (multi-app launcher rewrite, new official Tim Knapen user FAQ). The encoder pseudocode below was a direct port of solderless's `encodeToSP1()` from the 2026-05-12 source; the **2026-05-18 encoder additionally writes a sync counter at sector offset 2040**, see `references/10-midi-timing-encoding.md` 2026-05-18 update for the 8-byte sector trailer spec. See `update-log.md` 2026-05-18.

## Canonical user workflow (Tim Knapen, 2026-05-18 help.md)

The 2026-05-18 re-snapshot includes the **first official user-facing FAQ** authored by Tim Knapen, at `assets/solderless-2026-05-18/stemloader/help/help.md`. It is the canonical end-user procedure for putting custom audio on a stock SP-1.

**Required WAV format (per the official help):**

> 24 bit, 48 kHz, 8 channel, PCM WAV

Channel-to-stem mapping (verbatim from help.md):

```
CHANNEL   STEM
1         1, left
2         1, right
3         2, left
4         2, right
5         3, left
6         3, right
7         4, left
8         4, right
```

**Update 2026-05-18: 8 channels is no longer required.** The new encoder (`assets/solderless-2026-05-18/stemloader/js/wav-converter.js::parseWAV`) accepts **1 to N** channels. Missing channels play silent stems; channels beyond 8 are ignored. (The 2026-05-12 encoder rejected anything but exactly 8 channels.) The help.md text still says 8 channels because that's the canonical full-stem layout, but the encoder no longer enforces it.

**WAVEFORMATEXTENSIBLE subFormat offset bug fix (2026-05-18):** the 2026-05-12 parser read the `subFormat` field at `offset+28` inside the `fmt ` chunk, which is wrong by the WAV spec (correct offset is `+32`). WAV files emitted by tools using `WAVEFORMATEXTENSIBLE` (audioFormat = 0xFFFE) may have been misclassified by the 2026-05-12 encoder; the 2026-05-18 encoder handles them correctly. [Source: diff between `assets/solderless-2026-05-12/stemloader/js/wav-parser.js` and `assets/solderless-2026-05-18/stemloader/js/wav-converter.js`.]

**Tim Knapen's recovery procedure for interrupted transfers** (verbatim):

> SP-1 can seem unresponsive after a transfer was interrupted by unplugging the USB cable or closing the stem loader tab. This can look worrying but is normal: SP-1 is still in transfer mode and can't play audio. This can simply be resolved by long pressing the function button to restart SP-1. Data can become corrupted when a transfer is interrupted, so make sure to re-transfer any unfinished songs afterwards.

**BPM matters for effects:** Tim notes that BPM is used to encode the MIDI clock and timing of each song, and is needed for the gate effect, looping, and other time-based effects on the SP-1. Set this correctly per song; default is 80 if not specified. The stem loader extracts BPM from filenames matching `/(\d+(?:\.\d+)?)\s*bpm/i`.

**The most useful single finding for end users:** you do **not need custom firmware** to play your own audio on the SP-1. Stock TE firmware plays any correctly-formatted album image. TimK confirmed this [Lines #799–805]:

> Custom stems work with original TE firmware unmodified.

This file is the pipeline: how to go from "I have a song" to "the song plays on my unmodified SP-1," with byte-level encoder reference.

## Status: canonical encoder exists publicly (in solderless source)

As of synthesis date, the pieces are:

- **Stem separation:** HT Demucs, Spleeter, MusicNN, beat_this etc. — external tools, well-established.
- **WAV → SP-1 bytes encoder:** solderless's `encodeToSP1()` is the canonical reference. **Verified working** — solderless produces albums that stock firmware accepts. See "Encoder reference implementation" below.
- **Album header layout:** documented byte-by-byte from solderless's `uploadAlbum()` metadata-sector construction. **Verified working**.
- **Upload protocol:** documented opcode-by-opcode in `references/15-bootloader-protocol.md` and `references/16-usb-upload-protocol.md`. The full protocol byte spec is in the local solderless archive.

What does **not** yet exist publicly:

- A `pip install`-able tool that does the whole pipeline end-to-end. Solderless's encoder is JavaScript embedded in a web tool; nobody has ported it to a CLI/library yet.
- A faster uploader (solderless does ~10 KB/s, see `references/16-usb-upload-protocol.md`).
- An album-image extractor (given a `.sp1` blob, recover the original 4 stems as WAVs).

## The high-level pipeline

```
Source song (mp3, wav, anything)
   ↓ (stem separation)
4 stems: drums, bass, melody, vocals (each stereo)
   ↓ (resample if needed)
4 stems at 48 kHz, stereo, 24-bit signed PCM
   ↓ (combine to 8-channel WAV)
8-channel WAV (channels 0,1 = stem 0 L/R; 2,3 = stem 1; 4,5 = stem 2; 6,7 = stem 3)
   ↓ (encode via SP-1 encoder)
SP-1 audio bytes (8192-byte sectors, 340 frames each, 0/2/1/3 interleaving, per-sector tempo+envelope trailer)
   ↓ (prepend album metadata sector, append end-marker sector)
Complete album image
   ↓ (USB upload — references/16-usb-upload-protocol.md)
Album written to eMMC
   ↓ (boot SP-1 normally — power cycle by unplug or "long-press function button" depending on firmware state)
Album plays with full effects
```

## Step-by-step

### 1. Stem separation

Use any open-source 4-stem separator. Working choices:

- **HT Demucs FT** (emvee1968 uses this) — high-quality, slow.
- **Demucs** — older, also good.
- **Spleeter** — fast, lower quality.

Output: 4 WAV files at any sample rate / bit depth.

### 2. Resample + combine

Each stem must be **48 kHz, stereo, 24-bit signed PCM**. Combine into one 8-channel WAV in stem order: drums, bass, melody, vocals.

```sh
# Resample each stem
ffmpeg -i drums.wav   -ar 48000 -ac 2 -sample_fmt s32 drums_48k.wav
ffmpeg -i bass.wav    -ar 48000 -ac 2 -sample_fmt s32 bass_48k.wav
ffmpeg -i melody.wav  -ar 48000 -ac 2 -sample_fmt s32 melody_48k.wav
ffmpeg -i vocals.wav  -ar 48000 -ac 2 -sample_fmt s32 vocals_48k.wav

# Combine to 8-channel
ffmpeg -i drums_48k.wav -i bass_48k.wav -i melody_48k.wav -i vocals_48k.wav \
  -filter_complex "[0:a][1:a][2:a][3:a]amerge=inputs=4" \
  -ac 8 -sample_fmt s32 -bits_per_raw_sample 24 \
  combined.wav
```

(Note: `s32` is ffmpeg's 32-bit signed; 24-bit signed PCM goes into the lower 24 bits with sign-extension. The encoder reads from `int32`-aligned channel arrays.)

The solderless tools validate the WAV strictly:

- RIFF/WAVE container with `fmt ` and `data` chunks.
- Audio format = 1 (PCM) or 0xFFFE (WAVE_FORMAT_EXTENSIBLE) with subFormat = 1.
- `bitsPerSample` = 24.
- `sampleRate` = 48000.
- `numChannels` = 8.

[Source: `wav-parser.js` lines 7–41 in both copies.]

### 3. Encoder reference implementation (`encodeToSP1`)

This is the canonical SP-1 audio encoder. Port to your language of choice — it's stateless and deterministic. Source: `wav-parser.js::encodeToSP1` lines 93–132 (main) and 125–177 (TKT stemloader version). Both implementations are byte-identical for normal audio; the TKT version uses `Math.abs()` for envelopes (more correct) and the main version omits the abs (treats negative peaks as below zero) — both produce playable output.

```python
FRAME_SIZE        = 24       # 4 stems × 6 bytes/stem
STEM_FRAME_SIZE   = 6        # per stem per frame
BLOCK_SIZE        = 2048
SECTOR_SIZE       = 8192
FRAMES_PER_SECTOR = 340
BLOCK_ORDER       = [0, 2, 1, 3]  # frame % 4 -> block_id

def encode_to_sp1(channels, total_frames, bpm=80):
    """
    channels: list of 8 int32 arrays (per-frame samples in 24-bit-in-int32 representation,
              sign-extended; channels[0] = stem0 L, [1] = stem0 R, [2] = stem1 L, etc.)
    total_frames: number of frames in the source audio
    bpm: tempo for this song (used to populate per-sector tempo field)
    Returns: bytes; length is ceil(total_frames / 340) * 8192
    """
    total_sectors = (total_frames + FRAMES_PER_SECTOR - 1) // FRAMES_PER_SECTOR
    output = bytearray(total_sectors * SECTOR_SIZE)

    if bpm == 0:
        bpm = 80
    tempo = (48000 * 60) // (24 * bpm)  # uint16 value stored in sector trailer

    for s in range(total_sectors):
        sector_base = s * SECTOR_SIZE
        sector_frame_start = s * FRAMES_PER_SECTOR
        envelopes = [0, 0, 0, 0]

        for frame in range(FRAMES_PER_SECTOR):
            global_frame = sector_frame_start + frame
            if global_frame >= total_frames:
                break

            block_id    = BLOCK_ORDER[frame % 4]
            byte_offset = sector_base + BLOCK_SIZE * block_id + FRAME_SIZE * (frame // 4)

            for stem in range(4):
                L = channels[stem * 2][global_frame]      # int32 with 24-bit value sign-extended
                R = channels[stem * 2 + 1][global_frame]
                base = byte_offset + stem * STEM_FRAME_SIZE

                # Envelope = peak absolute value seen this sector, per stem.
                # (TKT version uses abs(); main solderless uses raw max — TKT is more correct.)
                envelopes[stem] = max(envelopes[stem], abs(L), abs(R))

                # SP-1 6-byte stem layout: L_mid, L_msb, R_msb, L_lsb, R_lsb, R_mid
                output[base    ] = (L >> 8 ) & 0xFF   # L_mid
                output[base + 1] = (L >> 16) & 0xFF   # L_msb
                output[base + 2] = (R >> 16) & 0xFF   # R_msb
                output[base + 3] =  L        & 0xFF   # L_lsb
                output[base + 4] =  R        & 0xFF   # R_lsb
                output[base + 5] = (R >> 8 ) & 0xFF   # R_mid

        # Per-sector trailer: tempo (uint16 LE) + 4× envelope (uint8) at end of block 0.
        # Layout: byte 2042..2043 = tempo LE, byte 2044..2047 = envelope[0..3]
        output[sector_base + 2042] = tempo & 0xFF
        output[sector_base + 2043] = (tempo >> 8) & 0xFF
        for i in range(4):
            output[sector_base + 2044 + i] = (255 * envelopes[i]) // 0x800000

    return bytes(output)
```

### 4. The 0/2/1/3 block interleaving — what it means

Each 8192-byte sector splits into 4 TE-blocks of 2048 bytes. Frames in the sector are distributed across blocks via `block_id = BLOCK_ORDER[frame_in_sector % 4]`:

```
frame % 4 == 0 → block 0
frame % 4 == 1 → block 2
frame % 4 == 2 → block 1
frame % 4 == 3 → block 3
```

So **physical block 0** (at sector offset 0..2047) holds every-4th frame (0, 4, 8, ...). Physical block 1 (sector offset 2048..4095) holds frame indices 2, 6, 10, ... Block 2 (offset 4096..6143) holds 1, 5, 9, ... Block 3 (offset 6144..8191) holds 3, 7, 11, ...

Each block contains all 4 stems for its frames: 85 frames × 24 bytes = 2040 bytes audio. The remaining 8 bytes per block (block-tail) are unused by the solderless encoder, **except for block 0**, which uses its last 6 bytes for tempo + envelope metadata.

Why this layout? Tape FF/RW with no extra CPU work. See `references/11-block-interleaving-tape-fx.md`.

### 5. Per-sector tempo + envelope trailer (correction vs. earlier wiki-based synthesis)

**What solderless writes:** only 6 bytes per sector, at the **end of physical block 0**:

- Bytes 2042..2043 — tempo (uint16 LE) = `(48000 * 60) / (24 * bpm)`. For 80 BPM = 1500. For 120 BPM = 1000. For 60 BPM = 2000.
- Bytes 2044..2047 — per-stem peak envelope, one byte each, normalized: `(255 * peak_abs_value) / 0x800000`. Roughly 0–255 representing the loudest sample in this sector for that stem (used by the firmware's level meter / LED display, presumably).

**Bytes 2040..2041 are unused.** Bytes 2048+ (blocks 1, 2, 3) have no trailer written by solderless — those blocks are 100% audio for the frames they hold.

Earlier versions of this skill (and the TKT wiki) described **8 bytes of trailer per block × 4 = 32 bytes per sector**, split as 2 sync + 2 tempo + 4 LED per block. That layout describes what the **stock firmware can read** if data is present (per the wiki) — but the solderless encoder demonstrates that **only the 6 bytes at end of block 0 are required** to produce playable albums. The "sync counter" field is not written by the encoder; per-block tempo and LED data are not written either.

Practical implication: when **producing** an album for stock firmware, write only the 6-byte trailer at end of block 0. When **reading** an existing album, you may encounter additional per-block side data (in TE-produced albums), but a custom encoder doesn't need to generate it. See `corrections.md` 2026-05-13.

### 6. Album metadata sector (sector 0)

The very first 8192-byte sector of the album is the **metadata sector**. Solderless builds it like this [Source: `storage.js::uploadAlbum` lines 599–618 + `stemloader.js::createBinary` lines 749–776]:

1. Allocate 8192 bytes filled with `0x58` ('X').
2. Write magic string `"ALBUM_PRESENT"` (13 ASCII bytes) at offset 0.
3. Write `albumLength` as uint32 LE at offset 13. **Value:** sum of all song lengths in sectors + 1 (the +1 is the end-marker sector).
4. Write `numSongs` as uint8 at offset 17.
5. Write the album title at offset 18, null-terminated. Max 63 chars + null = 64 bytes. Excess bytes (up to offset 81) remain as 'X'.
6. For each song, write a 136-byte entry starting at offset 82, then 218, then 354, ...:
   - Bytes 0..3: song offset (uint32 LE, in sectors). First song starts at sector 1 (right after metadata).
   - Bytes 4..7: song length (uint32 LE, in sectors).
   - Bytes 8..71: artist name, null-terminated, padded with 'X', max 62 chars + null = 63 bytes (the encoder leaves byte 72 as 'X' because it writes title there).
   - Bytes 72..135: song title, same encoding as artist.

Max songs that fit in a metadata sector: `(8192 - 82) / 136 ≈ 59`. The firmware caps at 60 songs and rejects more (`ALBUM_ERR.TOO_MANY_SONGS`) — see validation limits below.

```
Offset  Length  Field
0       13      Magic 'ALBUM_PRESENT' (13 ASCII bytes)
13      4       Album length in sectors (uint32 LE) — includes metadata + audio + end marker
17      1       Number of songs (uint8)
18      64      Album title (null-term, X-padded up to 64 bytes total)
82      N×136   Song entries

Per song entry (136 bytes):
  0..3    Song offset in sectors (uint32 LE)
  4..7    Song length in sectors (uint32 LE)
  8..71   Artist (null-term, X-padded, 64 bytes)
  72..135 Title  (null-term, X-padded, 64 bytes)
```

### 7. End-marker sector

After the last song's audio sectors, the album must end with a special **end-marker sector** of 8192 bytes:

- Filled with `0x00`
- The 13-byte ASCII magic `"ALBUM_PRESENT"` placed at the **last** 13 bytes (offset `SECTOR_SIZE - 13` = 8179..8191).

Without this trailing magic, the firmware's verifier returns `ALBUM_ERR.MAGIC_NOT_FOUND_AT_END` (`0x66` verify-command response payload[1] == 15).

[Source: `storage.js::uploadAlbum` lines 690–703.]

### 8. Validation limits

The firmware enforces these caps. The solderless source documents them in the `ALBUM_ERR` enum comments [Source: `sp-1_protocol.js` lines 27–44, also seen in `protocol.js`]:

| Limit | Value | Error code if exceeded |
| --- | --- | --- |
| Album length | `< 2` sectors → `ALBUM_TOO_SHORT` (2); `> 0x7FFFF` (524287) sectors → `ALBUM_TOO_LONG` (3) | 2 / 3 |
| Number of songs | > 60 | `TOO_MANY_SONGS` (4) |
| Album title length | > 63 bytes (before null) | `ALBUM_TITLE_TOO_LONG` (5) |
| Song offset | invalid / out-of-order | `SONG_OFFSET_INVALID` (8) |
| Song length | > 999,999 sectors | `SONG_LENGTH_TOO_LARGE` (9) |
| Song extends past album | sum overflows declared `albumLength` | `SONG_EXCEEDS_ALBUM` (10) |
| Artist name length | > 62 bytes (before null) | `ARTIST_NAME_TOO_LONG` (11) |
| Song title length | > 62 bytes (before null) | `SONG_TITLE_TOO_LONG` (13) |
| Magic at end | absent or wrong | `MAGIC_NOT_FOUND_AT_END` (15) |

The solderless UI defaults titles to "untitled" and artists to "unknown" when empty. Names are truncated to 63 characters (= `STRING_LENGTH - 1`) before null termination.

### 9. Upload

Once the album image is built (one big byte buffer: metadata sector + song audio sectors in order + end-marker sector), push it via the USB protocol. See `references/16-usb-upload-protocol.md` for the full procedure. Path A: use the locally archived solderless utility (when you can run a local web server). Path B: write your own client against the documented protocol. Path C: use moecal1947's slow Python uploader.

## Empirical observations from a real eMMC dump (added 2026-05-16)

First public hands-on inspection of a stock SP-1 eMMC dump, via the dot-Justin / `sp-1-emmc-dumper` project's v0.1 firmware extracting Unit B's contents (Jesus is King album). Partial dump only (~35%) but enough to verify the format and surface two findings.

**Confirmed:** The header + song-table structure documented above (Sections 6 and 8) parses real data exactly as specified. Magic, total_sectors (u32 LE @ offset 13), song_count (u8 @ 17), 64-byte X-padded title (@ 18), and 136-byte song records starting at offset 82 all match. Each duration computed from `song_length_sectors × 7.11 ms/sector` (per audio format Section in `09-audio-format-spec.md`) matches the canonical streaming-service track times within 5 seconds for all 9 valid entries.

**Finding 1 — SP-1 JIK album has 9 valid tracks, not 11 like the streaming release.** The header declares `song_count = 11` but only the first 9 song records contain real values; records 10 and 11 have garbage u32 fields and empty artist/title strings. The 9 tracks present are Every Hour through Hands On (matching the JIK release). Missing: "Use This Gospel" and "Jesus Is Lord". Three unresolved hypotheses for why:

- TE shipped only 9 stems for SP-1 JIK (licensing / stem-availability gap)
- The encoder always allocates `song_count + 2` slots and leaves the trailing ones uninitialized (would suggest reading until you find an obviously-corrupt entry)
- The header's `total_sectors` (233188) accounts for ~37k sectors more than the 9 valid songs sum to (~196k), so the missing audio data MAY actually be in the eMMC but unindexed by the header

Needs a second JIK unit dump to disambiguate.

**Finding 2 — sector layout has inter-song gaps that aren't constant.** The encoder spec (Section 6) implies songs are packed contiguously: song N+1 starts at `songs[N].offset + songs[N].length`. Real data has gaps:

| Transition | Gap (sectors) |
|---|---|
| 1→2  | 0 (contiguous) |
| 2→3  | 70 |
| 3→4  | 0 |
| 4→5  | 211 |
| 5→6  | 0 |
| 6→7  | 310 |
| 7→8  | 0 |
| 8→9  | 0 |

Pattern alternates. Hypothesis (unverified): the 0/2/1/3 block interleaving means each "song sector window" extends to the longest of its 4 stems, and shorter stems get tail-padded — so the gap reflects per-stem length variance at song boundaries. Anyone re-implementing the encoder should be aware that real albums don't always have contiguous song offsets even though the encoder spec generates them.

Inspection tool: [`scripts/inspect-partial-dump.mjs`](https://github.com/dot-Justin/sp-1-emmc-dumper) in the `sp-1-emmc-dumper` repo. Run on any dumped sector-0+ data to parse header and song table.

**Implementation note for tooling authors:** Several existing parsers (including the one in `sp-1-emmc-dumper`'s firmware `app/AlbumHeader.cpp` and host `host/src/verify/albumInfo.mjs` as of 2026-05-16) check for `0x78` ('x' lowercase) when stripping title padding. The actual padding byte is `0x58` ('X' uppercase) — match the spec in Section 6 above. Easy to confuse since both display visually as X-ish characters in hex viewers.

## What works on stock firmware vs custom firmware

| Behavior | Stock TE firmware | Custom firmware |
| --- | --- | --- |
| Plays 4 stems at 48 kHz | Yes | Depends — see `references/20-custom-firmware-state.md` |
| Per-stem fader mixing | Yes | Stock-equivalent only via `assets/audiothingies-2026-05-09/StockRuntimeMixer` |
| Track button mute/solo | Yes | Yes |
| FF/RW at 2.5x, 4x, 8x, 16x | 2.5x verified; higher rates documented in code | Custom firmwares typically max at 2.0x without block-skipping |
| Tape FX (smooth pause/play bend) | Yes | Yes in `assets/audiothingies-2026-05-09/VarispeedResampler`-based firmware |
| Filter, distortion, gate, echo effects | Yes, all 4 with 4 variations | Varies — emvee has gate; virtualflannel has different custom effects |
| Tempo-synced effect modulation | Yes (uses sync words and/or tempo field) | Not implemented in any public firmware yet |
| Bluetooth audio output | Yes (presumed, not formally verified) | emvee has pairing working; full audio over BT not yet |
| LED animations synced to song | Yes (uses LED words on disk if present) | Likely not in custom firmwares |

**Path of least resistance:** prepare a custom album image and play it on stock firmware. You get all the effects, the proper tape FX, tempo sync, and LED sync — without writing any firmware.

## On "step 4 nonsense"

[Context: tkt1000 called out a fabricated "step 4" in an AI-generated stem-prep workflow as nonsense [Discord #general, 2026-05-10 22:56]]

The pipeline above is sourced verbatim from solderless's working implementation. **Steps 1–7 are not fabricated** — they map 1:1 to functions in `wav-parser.js::encodeToSP1` and `storage.js::uploadAlbum`. The skill's prior conservative wording ("there is no canonical numbered workflow") is now superseded by the solderless source.

If a user asks "is there a canonical workflow?", the answer is: yes, solderless's `encodeToSP1` + `uploadAlbum` define it. The skill's encoder pseudocode above is a faithful port.

## Tools to watch for

The following are still **expected to be public soon** (as of synthesis date):

- **virtualflannel_46386's custom OS** — has working stem-prep; awaiting solderless return
- **emvee1968's HT Demucs pipeline** — awaiting two more units
- **ericlewis's "proper BSP"** — in development, no public ETA
- **An updated solderless.engineering** — offline for an update as of 2026-05-09; replaced for now by the locally archived 2026-05-12 snapshot
- **moecal1947's upload tool** — published or shared in some form, awaiting faster packet framing

## Where to go next

- For the on-disk frame byte order → `references/09-audio-format-spec.md`
- For block interleaving → `references/11-block-interleaving-tape-fx.md`
- For the (now mostly historical) wiki view of per-block side data → `references/10-midi-timing-encoding.md`
- For the upload protocol → `references/16-usb-upload-protocol.md`
- For unreleased custom firmwares with relevant pipelines → `references/20-custom-firmware-state.md`
- For corrections vs. earlier synthesis → `corrections.md`
- For the local archive of the solderless source code → `references/27-tools-and-utilities.md` and `sources.md`
