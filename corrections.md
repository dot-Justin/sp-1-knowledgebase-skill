# Corrections — Things That Were Believed True, Then Weren't

Three years of community work means knowledge evolved. Sometimes an early claim was wrong, sometimes a measurement was refined, sometimes a workaround turned out to be unnecessary. **This file captures those reversals so Claude reads recent ones and avoids quoting old ones.**

Whenever a reference file in this skill describes "the current state of knowledge," it represents the post-correction view. The pre-correction view is preserved here so Claude can recognize when an external source (e.g., an old forum post quoted by the user) is operating on outdated information.

Format per entry: **What was believed → What turned out to be true → Why → Citations.**

---

## Audio format and PCM representation

### Believed: "24-bit PCM, little-endian"
**Actual:** There are **two distinct representations** to talk about: in-memory and on-disk. Neither matches "24-bit little-endian PCM."

**In-memory:** samples are 24-bit signed values stored **right-aligned inside `int32_t` containers**, with maximum magnitude 8,388,607 (= 2^23 − 1). The conversion function is named `float_to_pcm_right24_fast` and returns `int32_t` [code: `audiothingies/PcmPacking.hpp`].

**On-disk (per stem, 6 bytes):**

```
Byte offset:   0       1       2       3       4       5
Carries:       L_mid   L_msb   R_msb   L_lsb   R_lsb   R_mid
```

Left and right samples are **interleaved at the byte level**. The MSBs of both channels sit next to each other in bytes 1 and 2; the LSBs in bytes 3 and 4; the mid bytes in 0 and 5. The decoder is:

```c
left  = ((int8_t)data[1] << 16) | (data[0] << 8) | data[3];
right = ((int8_t)data[2] << 16) | (data[5] << 8) | data[4];
```

[code: `storagethingies/DiskManager.hpp` lines 65–82, `decode_te_frame_payload_i32`]

**Why the wrong claim circulates:** AI summaries of the Lines thread inferred a "standard PCM" wire format from snippets without reading the decode code. The actual layout is more interleaved than any standard format.

**Citations:**
- Correction: [Discord #general, tkt1000, 2026-05-10 22:53 — *"24 bit PCM little endian … that doesn't make sense"*]
- Truth (in-memory): [code: `audiothingies/PcmPacking.hpp`]
- Truth (on-disk): [code: `storagethingies/DiskManager.hpp`]
- Full detail: `references/09-audio-format-spec.md`

### Believed: "Frame layout is 2048-byte chunks"
**Actual:** A logical sector is **8192 bytes**, composed of **16 native 512-byte eMMC blocks**. Each sector holds **340 audio frames** at **24 bytes per frame** (4 stems × 6 bytes per stem = 24 bytes; each stem is stereo 24-bit, so 2 channels × 3 bytes per channel). The leftover space in the sector holds metadata.

The earlier "2048-byte chunk" framing came from coarser intermediate understanding — it described one of the *four* 2048-byte sub-units of a sector (which corresponds to the 0/2/1/3 interleaving units used for tape FF/RW). It is not the unit Claude should use when describing the format.

**Citations:**
- Truth (frames/sector): [code: `storagethingies/DiskManager.hpp`, `audiothingies/StockRuntimeMixer.cpp`]
- Truth (eMMC blocks per sector + interleaving): [Discord #firmware, ericlewis, 2026-05-09 00:22 — *"Because of the 0, 2, 1, 3 interleaving: Block 0 contains frames: 0, 4, 8, 12 …"*]

---

### Believed: "Each TE-block carries a single 32-bit sync word with a 16-bit counter and unknown upper 16 bits"
**Actual:** Per TKT wiki [Audio-format, accessed 2026-05-12], the 4-byte timing payload per TE-block is split into **two distinct 16-bit fields**: a sync counter (bytes 0–1) and a tempo value (bytes 2–3). The "unknown upper 16 bits" are the tempo field.

**Why the wrong claim circulates:** the `storagethingies/DiskManager.hpp` `block_sync_words[4]` array uses `uint32_t`, which packs the two 16-bit fields together. Earlier synthesis treated this packed `uint32_t` as a single semantic "sync word." The wiki page distinguishes them.

**Still unknown:** the exact encoding of the 16-bit tempo field (raw BPM? Q8.8 fixed-point? sample period?) and byte ordering on disk (presumed little-endian).

**Citations:**
- Earlier (now superseded): `references/10-midi-timing-encoding.md` versions before 2026-05-12, `references/21-original-firmware-stems.md` sync_word_for_block() pseudocode
- Truth: [TKT wiki: Audio-format, accessed 2026-05-12]

### Believed: "Album header magic value is not publicly documented"
**Actual:** Per TKT wiki [Album-metadata-format, accessed 2026-05-12], the album header begins with the 13-byte ASCII magic `ALBUM_PRESENT`, and the same 13-byte magic must also appear at the very last bytes of the album image for the album to be considered valid.

The full header layout is:

```
0    'ALBUM_PRESENT'        13 bytes
13   album length sectors   4 bytes (0x2000-byte sectors)
17   song count             1 byte
18   album title            64 bytes (null-term, X-padded)
82   song entries           N × 136 bytes (4 + 4 + 64 + 64)
```

**Note on C struct vs on-disk:** `storagethingies/DiskManager.hpp`'s `AlbumInfo` and `SongInfo` declare name fields as `[65]` — the 65th byte is a C null terminator. On disk the corresponding fields are exactly 64 bytes.

**Citations:**
- Earlier (now superseded): `references/21-original-firmware-stems.md` "Open question: does the album header use a specific magic value or version field?"
- Truth: [TKT wiki: Album-metadata-format, accessed 2026-05-12]

### Believed: "Side-data words sit in a 32-byte trailer at the end of each sector"
**Actual:** Per TKT wiki [Audio-format, accessed 2026-05-12], side data is **per-TE-block**, not consolidated per-sector. Each 2048-byte TE-block is structured as **2040 bytes of audio + 8 bytes of trailer** (2 sync + 2 tempo + 4 LED). × 4 blocks per sector = 32 bytes of side data total, which matches the previously known total but refines its placement.

**Why this matters:** an encoder writing all 32 bytes of side data at sector offset 8160 will not produce a valid album image. The 8-byte trailers must be embedded after each block's 2040 bytes of audio.

**Citations:**
- Earlier (less precise): `references/09-audio-format-spec.md` versions before 2026-05-12, `references/21-original-firmware-stems.md` "Open question: the exact location of the side-data words within the 32-byte trailer"
- Truth: [TKT wiki: Audio-format, accessed 2026-05-12]

---

### Believed: "BQ24232 ISET pin is at P0.19"
**Actual:** **P1.00.** [TKT wiki: Battery-charger, accessed 2026-05-12; code: `SP-1-dev/src/stemplayer_pins.h`; code: `sp1-midi/boards/teenageengineering/stem_player/stem_player.dts` — `iset-override-gpios = <&gpio1 0 ...>`]

**Why the wrong claim circulated:** an earlier version of `sp1-midi/README.md` listed P0.19 for ISET. Both TimK's authoritative pin header AND the DTS in the same `sp1-midi` repo say P1.00. The README appears to be the error.

**Citations:**
- Earlier (wrong): `references/05-power-and-battery.md` and `known-unknowns.md` versions before 2026-05-12 cited `sp1-midi/README.md`'s P0.19
- Truth: [TKT wiki: Battery-charger; pin header; DTS — all agree on P1.00]

---

## eMMC access and protection

### Believed: "The eMMC content is encrypted"
**Actual:** The eMMC is **not encrypted**; the data is *"just formatted strangely"* — meaning it uses a custom 8192-byte sector layout with 0/2/1/3 frame interleaving for tape FF/RW support.

**Citation:** [Discord #hardware, tkt1000, 2026-05-07 17:40 — *"Not encrypted just formatted strangely"*]

### Believed: "You must desolder the eMMC to dump it"
**Actual:** There are **test points exposing DAT0–DAT3 + CLK + CMD** that let you read the eMMC in place. You only need to connect 3 lines minimum (or all 4 data lines for parallel read). DAT0-DAT3 are broken out to test points and can be read faster than the MCU does internally.

Desoldering is still a valid approach (fishdog_ documented it: 380–420 °C, 175 °C preheater, no reballing needed, then solder onto an unpopulated USB-to-eMMC adapter), but it's not required.

**Citations:**
- Test points exist: [Discord #hardware, tkt1000, 2026-05-07 17:41–17:53]
- Desolder method: [Discord #hardware, fishdog_, 2026-05-10 18:55–19:25 / 2026-05-11 19:08]

### Believed: "Custom firmware is required to load custom audio"
**Actual:** Custom stems play on the **stock TE firmware** if the audio is formatted correctly. Stock firmware doesn't validate content; just write a correctly formatted eMMC image and it plays. This was a major late-stage finding.

**Citations:**
- Original belief: implicit in early Lines posts about needing custom firmware
- Correction: [Lines #799–805, TimK, 2026-?]

---

## Bootloader and update path

### Believed: "Track 1 + Track 4 + Power Button triggers the bootloader"
**Actual:** Only **Track 1 + Track 4 + plug in USB-C** triggers the bootloader. Power button is not part of the combo. The original B_E_N post included a "step 4 — press and hold power" that was later removed in an edit.

**Citation:**
- Original wrong claim: [Lines #556, B_E_N, original posting, 2025-?]
- Correction: [Lines #556, B_E_N, edited posting, 2025-?]

### Believed: "You need to open the device and add an SWD header to flash firmware"
**Actual:** Once the bootloader trigger (Track 1 + Track 4 + USB) was discovered, **solderless firmware flashing became possible via USB CDC**. The `solderless.engineering` web tool uses Web Serial to push firmware without opening the device.

**Note:** Developing *new* firmware is much easier with an SWD header for debugging — but end-user *flashing* of an existing firmware does not require it.

**Citation:**
- Original belief: [Discord #hardware, tkt1000, 2026-05-08 05:20 — *"you can't properly develop firmware for this without fully opening the device and adding the SWD header"*]
- Correction (for flashing only): bootloader-trigger discovery [Lines #556, B_E_N] + solderless.engineering launch

---

## Fast-forward / rewind speed

### Believed (TimK working version, May 2026): "Maximum FF speed is 2.0x in custom firmware; original goes up to 2.5x"
**Actual (per ericlewis, with audiothingies code as reference):** The original firmware supports rates of **2.5x, 4x, 8x, 16x for FF** and **1x, 2.5x, 4x, 8x for RW** [code: `audiothingies/AudioEngine.hpp` — `kFastForwardRates`, `kRewindRates`]. The trick is **block-skipping**: above 2.5x the firmware switches from full-sector reads to half-sector reads (just blocks 0 and 1 of the four), and at 4x it switches to quarter-sector reads (just block 0), then duplicates frames to fill the gap. The 0/2/1/3 interleaving makes this work without any extra CPU math.

This is not strictly a correction of TimK's claim — TimK's "I got FFWD only up to 2x" is accurate for his implementation as of 2026-05-09; he just hadn't implemented the block-skipping trick yet. But the **theoretical max throughput is ~4 MB/s** per ericlewis, far above what 2x requires, and the original firmware achieves up to 2.5x using the interleaving trick.

**Citations:**
- TimK's implementation status: [Discord #firmware, tkt1000, 2026-05-09 07:04 — *"I got FFWD only up to 2X after that you need to jump eMMC blocks. I think I measured the original fw only going up to 2.5x"*]
- ericlewis's mechanism explanation + theoretical max: [Discord #firmware, ericlewis, 2026-05-09 00:09–00:34]

---

## Firmware dump completeness

### Believed: "The dump must be incomplete — the I2S peripheral only has TASKS_START and TASKS_STOP assigned"
**Actual:** The dump was complete. TE firmware uses a **minimal I2S register setup**. Sparse register assignments looked suspicious in Ghidra but reflect TE's actual minimal design.

**Citation:**
- Original suspicion: [Lines #317, murray, 2025-?]
- Resolution: confirmed working in later TimK custom firmware [Lines ~#345, TimK, 2025-?]

---

## Disassembly hazards

### Believed: "I bricked my SP-1 by reassembling it"
**Actual:** The unit wasn't bricked — the **FPC (flexible printed circuit) ribbon cable was slightly bent**. After reseating it, the device worked.

This is a recurring failure mode: the FPC connector (Omron XF3B-1945-31A, 19-pin, 0.3 mm pitch) is fragile. Devices that appear bricked after reassembly should first be checked for FPC seating issues.

**Citations:**
- Original wrong belief: [Lines #118, PedalsandChill, 2024-? — *"Also, just bricked one of my stem players after putting it back together"*]
- Correction (same post): *"scratch that I just fixed it guess ribbon cable was slightly bent or something"*

### Believed: "The white plastic piece around play/effect buttons unsnaps mechanically"
**Actual:** It is **glued in place** (in one spot, per steellll). Removal requires **heat** (hairdryer is enough — does not damage plastic if temperature is reasonable). Pulling without heat risks bending the plastic permanently.

**Citations:**
- Belief that it was clip-on or easily removed: implied in early disassembly attempts
- Correction: [Discord #hardware, tkt1000, 2026-05-06 23:11 + itditry, 2026-05-06 23:54]

---

## How to use this file

When Claude is about to make a factual claim about the SP-1 and isn't sure if the claim is from old material or current material:

1. Search this file for keywords related to the claim
2. If the claim matches a "Believed" entry, switch to the "Actual" version
3. If the claim contradicts an "Actual" entry, **don't quote it** without flagging the contradiction to the user
4. When the user shows Claude an old Lines post that contradicts a current reference file, point at this file to explain why

New corrections are added to this file (not deleted from reference files) whenever later sources override earlier sources. Reference files synthesize the **post-correction** view; this file preserves the **pre-correction** view alongside the resolution.
