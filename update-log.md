# SP-1 Skill Update Log

Persistent change history for the `sp-1` skill, more granular than git commits. Each entry records what changed, why, and where the new claim is sourced. Organized by intake batch (the chronological unit of synthesis work — usually one new source ingested and propagated).

This log is for future-Claude. When asked "why does the skill say X today, and what would have to change for it to no longer be true?", read this file before answering. Earlier history (pre-2026-05-13) lives in `synthesis-log.md` and `corrections.md`; this log is additive.

Format per batch:

- **Source ingested** — what was added to the local corpus, when, where, who provided it
- **Why it matters** — what gaps it closes / corrections it forces
- **Change plan** — the file-by-file inventory of what to update
- **Changes landed** — checkmarked list with one-line "what landed" per file
- **Verification snapshot** — sanity checks performed at end of batch
- **Deferred** — items intentionally not done this batch, tracked here so they don't get lost

---

## 2026-05-18 — Intake batch #1: solderless.engineering re-snapshot + skill corrections

### Source ingested

**Files:** `assets/solderless-2026-05-18/` + `assets/solderless-2026-05-18.zip`
**Provenance:** Fresh scrape of the live `solderless.engineering` site on 2026-05-18 via `wget --mirror`. Site is online again (HTTP 200) after going offline 2026-05-09 for an update. 44 files including this skill-added README; ~601 KB extracted.
**Contents:** A complete architectural rewrite. The 2026-05-12 site was a single-page upload tool; the 2026-05-18 site is a **multi-app launcher** with 4 sandboxed iframes:

- `index.html` — parent launcher; owns one Web Serial port; tab switching + welcome screen + connect button
- `js/serial-shim.js` — postMessage Web Serial proxy injected into iframes (active-iframe-only TX gating, 60-second connect timeout)
- `stemloader/` — evolved stem loader (drag-reorder via `Sortable.js`, per-song state badges, in-app help via `marked.umd.js`, BPM-from-filename, localStorage persistence). The encoder was renamed `wav-parser.js` → `wav-converter.js` and the WAV parser was merged with the SP-1 encoder. **Now writes an 8-byte sector trailer** (clock at offset 2040, tempo at 2042, envelopes at 2044) where the 2026-05-12 encoder wrote only 6 bytes (no clock).
- `utility/` — **new** browser-based custom-firmware flasher. Same R/F/E/H byte-level protocol as already documented; cleaner reference (flash-only, no album mixed in)
- `deviceinfo/` — **new** live runtime-state viewer. Polls faders/buttons/ladders every 100 ms; queries device ID, album title, write counter, battery state, mode on connect. SVG mirror of the device updates in real time.
- `doom/` — **new** "spoom1": Doom in the browser, controlled by the SP-1 as a USB-HID-like controller. Polls SP-1 at ~16 Hz.
- `stemloader/help/help.md` — **first official Tim Knapen user FAQ.** Quotable: 1 min audio = ~10 min transfer, channel mapping (1L,1R → stem 1), "long-press function button to unstick after interrupted transfer".

### Why this matters

Beyond the architectural rewrite, the re-snapshot **corrects one specific factual claim** in the 2026-05-13 synthesis:

- the 2026-05-13 synthesis said the solderless encoder "writes no sync counter and no LED data" and that the sector trailer is "only 6 bytes per sector, at the end of block 0: 2 bytes tempo + 4 bytes per-stem envelope."
- the 2026-05-18 encoder **writes 8 bytes** at the end of block 0: 2 bytes clock (LE16) + 2 bytes tempo (LE16) + 4 bytes envelopes. Empty sectors get sentinel `clock=0xFFFF / tempo=0x0000`. First tick is hardcoded to `clock=1`.

This invalidates a literal statement in `references/10-midi-timing-encoding.md` and the warning in `hallucination-watchlist.md` entry #22 ("don't claim encoder writes per-block sync counter"). The per-block claim is *still wrong* (the encoder writes per-sector at block-0 offset 2040, not per-block); but the *encoder-writes-no-sync-counter* claim is now historical and applies only to the 2026-05-12 bundle.

The new bundle also makes several previously partial pieces of the host protocol more concretely cite-able:

- the iframe + serial-shim postMessage pattern (host-side architecture, not on-device — added to watchlist)
- the live polling rates (10 Hz for `deviceinfo`, ~16 Hz for `doom`) — empirical evidence of WebSerial throughput
- a public, dedicated firmware-flash app (`utility/`) — anyone can now flash a custom .bin from a browser
- the WAV parser **bug fix** (`subFormat` field offset corrected from +28 to +32 inside `WAVEFORMATEXTENSIBLE`) and **channel flexibility** (accepts 1-N channels; missing stems are silent — was: required exactly 8)
- a Tim-Knapen-authored user FAQ document — first canonical recovery procedure for an interrupted transfer ("long-press function button")

### Changes landed

- ✅ `assets/solderless-2026-05-18/` — full 44-file mirror added
- ✅ `assets/solderless-2026-05-18.zip` — byte-fidelity zip created (348 KB compressed)
- ✅ `assets/solderless-2026-05-18/README.txt` — skill-added run instructions, in the style of the 2026-05-12 README
- ✅ `SKILL.md` — synthesis date bumped to 2026-05-18; assets inventory updated; Q6 ("is solderless online?") flipped to yes; Q12 (upload speed) co-cite added; assets bullet list updated; freshness section refreshed
- ✅ `synthesis-log.md` — master synthesis date bumped to 2026-05-18; new 2026-05-18 batch section; freshness rule 2 updated; freshness rule 4 updated (`0x39` framing is now in `references/16-usb-upload-protocol.md`, not "still unknown")
- ✅ `sources.md` — `solderless-2026-05-12` entry retitled "earlier snapshot, preserved for citation stability"; new `solderless-2026-05-18` entry added with full layout + cite-as guidance; `https://solderless.engineering/` URL note updated
- ✅ `corrections.md` — two new entries: encoder-now-writes-clock; solderless-online-with-multi-app
- ✅ `hallucination-watchlist.md` — entry #21 annotated (8-byte sector trailer, not 4× 8-byte block trailers); entry #22 annotated (per-sector yes, per-block still no); new entry added about iframe/serial-shim being host-pattern not on-device
- ✅ `working-confirmed.md` — added safe-claims for 8-byte trailer, sentinel values, first-tick=1, MAX_SECTORS, CLOCK_MAX, bootloader-flash re-confirm, device-info opcodes, WAV channel flexibility
- ✅ `known-unknowns.md` — resolved encoder-writes-clock and solderless-online; added new entries for ladder semantics, PING semantics, FREQ_SWEEP params, 24489↔49152 reconciliation
- ✅ `references/10-midi-timing-encoding.md` — added 2026-05-18 update paragraph documenting 8-byte trailer + sentinels + new constants
- ✅ `references/15-bootloader-protocol.md` — synthesized-through header updated; standalone-clients section updated to "online again with multi-app launcher"; utility app cited
- ✅ `references/16-usb-upload-protocol.md` — synthesized-through header updated; Tim Knapen help.md paragraph added (10:1 ratio, function-button recovery, channel mapping); `wav-parser.js → wav-converter.js` rename noted
- ✅ `references/20-custom-firmware-state.md` — web flasher availability noted; 892 KB ceiling note added
- ✅ `references/21-original-firmware-stems.md` — Tim Knapen canonical-workflow subsection added; WAV channel flexibility + subFormat bug fix noted
- ✅ `references/27-tools-and-utilities.md` — status flip; 4-app inventory; iframe+serial-shim pattern documented; bundle inventory updated
- ✅ FAQ companions to refs 10/15/16/20/21/27 — added new Q&As per plan

### Verification snapshot

- `grep -rn "/Projects/" ~/.claude/skills/sp-1/` → no hits (public-skill assertion)
- `grep -rn "/home/justin" ~/.claude/skills/sp-1/` → no hits
- `grep -rn "solderless-research" ~/.claude/skills/sp-1/` → no hits
- `grep -n "writes no sync counter" ~/.claude/skills/sp-1/references/10-midi-timing-encoding.md` → only qualified appearances (e.g. "the 2026-05-12 encoder…")
- `grep -n "0xFFFF" ~/.claude/skills/sp-1/references/10-midi-timing-encoding.md` → empty-sector sentinel documented
- `grep -n "firmware utility" ~/.claude/skills/sp-1/references/27-tools-and-utilities.md` → new app described
- existing 2026-05-12 citations still resolve to existing files (no mass-rewrite happened)

### Deferred

- **Reconciliation of `24489` (Galapagoose, Lines #739) vs `CLOCK_MAX = 49152` (2026-05-18 encoder).** These may be different fields, or the same field measured two different ways. Logged as a still-open unknown in `known-unknowns.md`. Resolution requires either dumping a real album's bytes from a working device or asking TimK / Galapagoose directly.
- **Meaning of the `t` opcode's "ladder" uint16 values** (labeled L1/L2 in deviceinfo). Probably button-matrix R-divider ADC voltages, but not confirmed. Logged in `known-unknowns.md`.
- **Response semantics of `b` (PING)** and parameter format of `V` (FREQ_SWEEP)** — solderless sends them but defines no response handlers. Logged in `known-unknowns.md`.
- **The wasm-doom binary** (`doom/wasm/doom.wasm`) was not present at the scrape's URL space — the doom app may load it from a CDN at runtime. Not bundled; the doom.js source is sufficient for documenting the input-mapping aspect.
- **Bulk-rewriting existing 2026-05-12 citations** in references to point at 2026-05-18 — intentionally skipped. Old citations remain line-stable and the byte-level protocol is unchanged; rewriting them would be churn with no information gain.

---

## 2026-05-13 — Intake batch #1: solderless source archive

### Source ingested

**File:** `assets/solderless-2026-05-12.zip`
**Provenance:** Obtained as a community backup on 2026-05-13. Archive contents dated 2026-05-09 (last upstream edit of the `pages.dev` deployment). README dated 2026-05-12.
**Contents:** Static dump of `solderless-engineering.pages.dev` including:

- `index.html` — main utility (FW flash + album upload + device info)
- `stemloader.html` — TKT's standalone stem loader (linked from main as "TKT stem loader")
- `js/protocol.js` (10.6 KB) — CRC-8 table, COBS, packet framing, serial I/O, state machine helpers
- `js/firmware.js` (6.4 KB) — firmware flash logic (R/F/E/H sequence)
- `js/storage.js` (24.6 KB) — album upload logic + device-info querying
- `js/wav-parser.js` (5.3 KB) — WAV → SP-1 encoder (main utility)
- `stemloader/js/stemloader.js` (27.1 KB) — TKT stemloader UI + upload
- `stemloader/js/sp-1_protocol.js` (6.6 KB) — protocol constants + helpers (TKT version)
- `stemloader/js/wav-parser.js` (6.0 KB) — WAV encoder (TKT version)
- CSS/fonts/images (assets only, no protocol relevance)

**Both JS implementations are byte-for-byte compatible** at the protocol level. The main utility is the polished UX (firmware flashing + album upload + device info in one page); the stemloader is the older TKT-only standalone uploader. Constants like `BLOCK_ORDER = [0, 2, 1, 3]`, `FRAME_SIZE = 24`, `SECTOR_SIZE = 8192`, `ALBUM_PRESENT` magic are identical.

**Status of this source:** This is the **canonical reference implementation** of the SP-1 bootloader + album-upload protocol. Anything contradicting it should be treated as wrong unless backed by a stronger primary source (the original firmware binary, which the community does not have public access to).

### Why this is a big deal

Before this archive:
- The skill said the `0x39` packet had a "4-byte magic/sub-command header + 4-byte sector address" with **speculation** about what the header bytes were and **no public source** for the ACK/retry behavior.
- The skill claimed `GPREGRET = 0x1A96` was the magic value for parser-2 mode based on Discord paraphrase.
- The skill described an "8-byte trailer per TE-block" with sync/tempo/LED fields based on a WebFetch summary of the TKT wiki.
- The skill said `solderless.engineering` source was not public.
- Multiple `known-unknowns.md` entries depended on speculation that this archive resolves definitively.

After this archive:
- The 136-byte `0x39` payload is precisely `chunk_counter (LE32) | emmc_byte_offset (LE32) | 128 bytes data`. No magic header, no checksum-in-header.
- The CRC-8 table is in the outer command framing (COBS-wrapped Q-magic + seq + cmd + plen + payload + crc8). Confirmed CRC-8 table given verbatim.
- Mode switch is plain `0x70 [1]` + `0x50` reboot, observed in source. No `0x1A96` value appears anywhere in the JS. That magic number was a Discord paraphrase that was never independently verified — likely real on the firmware side but the host doesn't need to know it.
- Per-sector trailer (per solderless `encodeToSP1`) is only **6 bytes at end of block 0**: tempo (uint16 LE at bytes 2042-2043) + envelopes (4× uint8 at bytes 2044-2047). Blocks 1, 2, 3 are pure audio plus 8 bytes of unused tail space.
- Tempo formula is `(48000 * 60) / (24 * bpm)`, not a "sync counter" or `Q8.8`. For 80 BPM = 1500. The skill's prior `int(sample_pos / (30000/bpm)) % 24489` formula appears to have been speculative — solderless does not write a sync counter at all.

These changes resolve five `known-unknowns.md` entries and add several new entries to `hallucination-watchlist.md` and `corrections.md`.

---

## Change plan

Each section below describes one file's update: what was wrong/missing pre-batch, what it should say post-batch, and the source. Exhaustive — every fact mined from the archive that has a home in the skill is listed.

### Batch 1A — Protocol files (highest-impact corrections)

#### `references/15-bootloader-protocol.md`

**Current state issues:**
1. Calls `0x52` a "bootloader version check" — actually a generic state query that returns a 5-byte state string ("10510" = upload mode, "00100" = boot mode).
2. Speculates `0x39` packet has 4-byte "magic/sub-command/flags/checksum" header — actually `chunk_counter + emmc_byte_offset`.
3. Asserts `GPREGRET = 0x1A96` parser-2 magic without citation in source. Solderless source uses `0x70 [1]` + `0x50` only.
4. Mentions "ACK protocol presumably one byte per packet" — solderless source is fire-and-forget (no-reply) for `0x39`.
5. Missing: COBS framing, CRC-8 table, baud rate (115200), exact packet body format `0x51 magic | seq | cmd | plen | payload | crc8`.
6. Missing: complete command palette including LEDs (`Z`), faders (`d`), buttons (`\\`), ladders (`t`), battery (`z`).

**Action:**
- Replace opcode table with definitive list (source: `protocol.js`, `stemloader.js` debug panel, `index.html` instructions).
- Add full COBS+CRC-8+packet framing section with the verbatim CRC-8 table and packet-body layout.
- Add "state query response" subsection explaining `"10510"`/`"00100"` strings.
- Replace `0x39` packet layout with definitive `chunk_counter|emmc_offset|data` structure.
- Remove `0x1A96` magic claim (or relegate to a footnote: "value as paraphrased by ericlewis in Discord; not observed in any public host implementation").
- Replace the inferred ACK speculation with the actual no-reply semantics.
- Add the full transaction sequences (firmware flash, album upload) as observed in `firmware.js` and `storage.js`.

#### `references/16-usb-upload-protocol.md`

**Current state issues:**
1. Says byte 4–7 is "sector address (32-bit, presumably native 512-byte block addressing)" — actually absolute eMMC **byte offset** (LE32), and chunks are 128 bytes.
2. Speculates about ACK byte values, retries, NAK handling — none of which exist.
3. Discusses "USB control transfers vs bulk endpoints" / "4 MB/s theoretical" framing — solderless uses Web Serial (CDC ACM) at 115200 baud, achieving ~10 KB/s practical. Both the "moecal slow" and "ericlewis 4 MB/s" framings get nuance: the slow tool is bottlenecked by control transfers, the 4 MB/s number assumes a hypothetical USB MSC implementation, and the actual public solderless tool achieves ~10 KB/s with no per-chunk handshake at 115200.
4. Missing: the `0x37` "reset write counter" command before upload begins. Missing the `0x51` "end-of-upload / reboot" command at the end.

**Action:**
- Rewrite "the bulk packet" section with definitive 136-byte structure: chunk_counter (LE32) | emmc_byte_offset (LE32) | 128 bytes data.
- Add: payload addresses are byte offsets, not block addresses. Chunks are written consecutively (`emmcOffset += 128`).
- Document the full upload sequence verbatim from `storage.js::uploadAlbum`:
  1. `0x37` reset write counter (start of session)
  2. `0x39` × N (no-reply): 64 chunks for metadata sector (8192 bytes), then `song.length * 64` chunks per song, then 64 chunks for end-marker sector
  3. `0x51` no-reply: end-of-upload signal
- Document the metadata-sector / end-marker-sector framing.
- Replace 4 MB/s speculation with: solderless achieves ~10 KB/s at 115200 with no per-chunk handshake; moecal's 0.75 KB/s figure used USB control transfers (~10× slower than bulk).
- Add: WAV requirements are enforced client-side (8 channel, 24-bit, 48 kHz, signed PCM).

#### `references/21-original-firmware-stems.md`

**Current state issues:**
1. Describes "8-byte trailer per TE-block: 2B sync counter, 2B tempo, 4B LED data, × 4 blocks per sector". Solderless `encodeToSP1` only writes 6 bytes at the end of **block 0**: tempo (2 bytes) + envelopes (4 bytes). Blocks 1, 2, 3 have no side-data writes.
2. Tempo encoding speculation (`int(sample_pos / (30000/bpm)) % 24489`) — actual formula is `(48000 * 60) / (24 * bpm)`.
3. Album header layout was sourced from TKT wiki via WebFetch summary — now confirmed byte-for-byte from solderless source. Add validation limits from `sp-1_protocol.js` comments.
4. Encoder pseudocode shows generic frame packing — replace with the definitive nested loop from `encodeToSP1`.
5. "Step 4 nonsense" warning is still valid as a guard rail, but now we have a real numbered workflow from a verified reference implementation. Mark it: "This *is* the canonical encoder, sourced from solderless's `encodeToSP1`."

**Action:**
- Rewrite "Per-block side data" section: only block 0 has metadata, contents are tempo + envelopes, not sync/LED.
- Replace sync-counter pseudocode with the definitive tempo formula.
- Add definitive encoder pseudocode (port of `encodeToSP1`).
- Add validation limits: max album sectors `0x7FFFF`, max songs 60, max title 63 chars, max artist/song-title 62 bytes, max song length 999999 sectors. (Source: `sp-1_protocol.js` ALBUM_ERR comments.)
- Add: metadata sector is filled with `0x58` ('X'), magic at offset 0, fields written over the X-fill.
- Add: end-marker sector is filled with `0x00` and magic appears at the **last** 13 bytes (`SECTOR_SIZE - 13`).
- Add: stem ordering in WAV input: channels[0,1] = stem 0 L/R; channels[2,3] = stem 1 L/R; etc.

### Batch 1B — Audio format files (confirmations + corrections)

#### `references/09-audio-format-spec.md`

**Current state issues:**
1. The 6-byte stem layout `L_mid, L_msb, R_msb, L_lsb, R_lsb, R_mid` is correct and now **doubly confirmed** by the solderless encoder writing bytes in exactly this order.
2. The "8-byte side-data per TE-block × 4 = 32 bytes" claim contradicts the solderless encoder. Fix as above.
3. Album header section already documented correctly per TKT wiki; cross-reference with solderless source (matches).

**Action:**
- Strengthen the byte-layout section: add a "Confirmed by solderless source 2026-05-13" note with file path.
- Rewrite the "non-audio bytes per sector = 32, split as 8 per block" line: actual layout is 6 bytes of trailer (tempo + envelopes) at end of block 0; other blocks have 8 bytes unused tail.
- Add confirmation that integer fields in album header are LE (the JS uses `DataView.setUint32(... true)` for LE everywhere).

#### `references/11-block-interleaving-tape-fx.md`

**Current state issues:** None known. The `[0, 2, 1, 3]` ordering and the `frame % 4` → block_id mapping match solderless source verbatim.

**Action:**
- Add citation note: "Encoder reference implementation: solderless `encodeToSP1`, archived 2026-05-13."
- Otherwise leave as is.

#### `references/10-midi-timing-encoding.md`

**Current state issues (suspected, not yet read):** likely speculates about MIDI clock encoding in the per-block trailer. Per solderless source, the only timing-related write is the per-sector tempo (uint16 LE) at byte 2042–2043 in block 0. No "MIDI clock counter" is written by the encoder. The stock firmware may compute and apply something at playback, but the encoder doesn't lay anything down.

**Action:**
- Read the file, audit for tempo-encoding speculation, replace with definitive formula or move to known-unknowns.

### Batch 1C — Tool / source files

#### `references/27-tools-and-utilities.md`

**Current state issues:**
1. Lists `solderless.engineering` as "offline as of 2026-05-09, source not public."
2. Doesn't reference the local archive.

**Action:**
- Mark solderless source as **locally archived** at `assets/solderless-2026-05-12/`, dated 2026-05-12.
- Note the archive surfaced via community backup on 2026-05-13.
- Note there are two implementations in the archive: main utility (FW + storage + info) and `stemloader/` (TKT's standalone). Both use the same protocol.
- Caveat: archive is from before the announced "offline for update" — if/when solderless comes back online, it may differ.

#### `sources.md`

**Action:**
- Add an entry for the solderless archive: location, provenance, archive date, key files, what they document.
- Add file-level pointers from individual protocol facts in `references/15-*` and `references/16-*` to the specific JS files they're sourced from.

### Batch 1D — Accuracy files

#### `known-unknowns.md`

**Entries to CLOSE (move to working-confirmed / add corrections):**
1. **eMMC write path / 0x39 packet framing** — definitive byte layout now known. Move to `working-confirmed.md`.
2. **Solderless.engineering source code** — source is now locally archived. Note in `working-confirmed.md`.
3. **Full byte ordering of the album header** — LE confirmed by JS using `setUint32(..., true)`. Note in `working-confirmed.md`.
4. **Exact encoding of the 16-bit tempo field** — formula `(48000 * 60) / (24 * bpm)` from solderless. Move to `working-confirmed.md`.
5. **Alternative bootloader modes / 0x1A96 magic** — the `0x1A96` magic value is not present in any public host implementation. Mode switch via `0x70 [1]` + `0x50` reboot is the documented path. Reframe as: "the GPREGRET magic value (if any) is firmware-internal; the host-side procedure is the two-command sequence."

**Entries to keep open:**
- Sample bit-alignment (left vs right aligned) — solderless decodes to right-aligned int32 with sign extension (`if (s & 0x800000) s |= 0xFF000000;`), consistent with `assets/audiothingies-2026-05-09/PcmPacking.hpp`. The TKT wiki's left-aligned formula is still anomalous. Add a note that solderless agrees with audiothingies.
- High Speed eMMC mode — not addressed by solderless (host doesn't talk to eMMC directly).
- BT module integration — not addressed.
- Power-off in custom firmware — not addressed.
- ISET resistor values — not addressed.
- Second 3.5mm jack pin assignment — not addressed.
- Hardware variants — not addressed.
- Effects implementations beyond the basics — not addressed.

#### `working-confirmed.md`

**Entries to ADD:**
- The full SP-1 host-side protocol byte spec is documented, with citations to specific JS files in the local archive.
- The album metadata sector layout is confirmed byte-for-byte from solderless source.
- The encoder pseudocode (`encodeToSP1`) is documented as the canonical reference.
- The tempo encoding `(48000 * 60) / (24 * bpm)` is confirmed.
- The CRC-8 lookup table is documented verbatim.
- COBS framing is documented.

#### `hallucination-watchlist.md`

**Entries to ADD:**
- **`0x52` is a "bootloader version check"** → wrong; it's a state query returning a 5-byte state string. Always cite by behavior, not "version check."
- **`0x39` packet has a 4-byte magic/sub-command header** → wrong; it's `chunk_counter (LE32) | emmc_byte_offset (LE32) | 128 bytes`.
- **The host needs to know about `GPREGRET = 0x1A96`** → wrong; the host just sends `0x70 [1]` + `0x50`. The magic is firmware-internal and the host can't observe it.
- **Per-block 8-byte trailer (sync counter + tempo + LED)** → wrong for the encoder side. Solderless writes only 6 bytes (tempo uint16 + 4× envelope uint8) at the end of block 0; blocks 1–3 have no side-data writes.
- **Tempo is encoded as samples-per-tick / Q8.8 BPM / sync counter** → wrong; formula is `(48000 * 60) / (24 * bpm)`, stored as uint16 LE.
- **`0x39` packets are ACK'd one at a time** → wrong; they are fire-and-forget at 115200 baud (no per-chunk reply).
- **MIDI clock counter is embedded in audio side-data** → solderless does not write one. If stock firmware uses one, it derives from the tempo value or is implicit in playback time.

#### `corrections.md`

**Entries to ADD** (with full Why / What changed / Source structure):
- Bootloader opcode behaviors and packet structure (was speculative, now definitive).
- GPREGRET `0x1A96` claim (was paraphrased from Discord, now relegated to firmware-internal trivia).
- Per-block side-data layout (was sourced from TKT wiki, contradicted by solderless encoder).
- Tempo encoding (was speculative, now definitive).
- Sync counter formula (was speculative; solderless doesn't write one).

#### `synthesis-log.md`

**Action:**
- Bump synthesis date from 2026-05-11 → 2026-05-13.
- Add an entry under "Material covered": solderless source archive (2026-05-12 contents, ingested 2026-05-13).
- Update freshness rules: the protocol byte spec is now considered definitive; questions about it can be answered without "verify with the live Discord" caveats.

### Batch 1E — FAQ files

The FAQ files paired with `15-`, `16-`, `21-`, `09-` need quick consistency passes to reflect the corrections:

- `15-bootloader-protocol-faq.md`
- `16-usb-upload-protocol-faq.md`
- `21-original-firmware-stems-faq.md`
- `09-audio-format-spec-faq.md`

Last in the implementation order — small surgical edits.

### Batch 1F — SKILL.md top-of-skill updates

The Top 15 quick-lookup answers in `SKILL.md` include:

> **Q: What's the album upload speed?**
> A: Currently **~0.75 KB/s** (4.5 days per 311 MB album) via moecal1947's Python tool. **Theoretical max ~4 MB/s** per ericlewis. Proper bulk-endpoint implementation doesn't exist publicly yet.

This needs an update: the solderless web tool achieves ~10 KB/s practical (calculated from 115200 baud + 136-byte chunks no-handshake), which is the **already-public** middle ground between moecal's 0.75 KB/s and ericlewis's hypothetical 4 MB/s. Update the answer.

Other Q&A entries that may need touching:
- "How do I cleanly power down in custom firmware?" — unchanged.
- "Is `solderless.engineering` online?" — update to: offline since 2026-05-09; source code now locally archived from a 2026-05-12 snapshot.

### Out of scope for this batch

- Anything related to firmware reverse-engineering, Ghidra, glitch attacks (the archive is pure host-side).
- BT module integration.
- Bluetooth pairing, deep sleep, charger driver details.
- Custom firmware state (emvee/virtualflannel/TimK).

These remain as documented; no archive evidence touches them.

---

## 2026-05-13 — Changes landed

- ✅ `references/15-bootloader-protocol.md` — full rewrite. New: COBS framing section, CRC-8 table verbatim, complete command palette (state query / mode switch / firmware flash / album upload / device info / LED set), 5-byte state-string semantics ("10510"/"00100"), definitive 0x39 chunk payload (`chunk_counter | emmc_byte_offset | data`), corrected ACK semantics (fire-and-forget), corrected mode-switch (no GPREGRET magic host-visible), local-archive citation.
- ✅ `references/16-usb-upload-protocol.md` — full rewrite. New: definitive 136-byte chunk layout, emmc_byte_offset (not sector address), `0x37` start session, `0x51` end-of-upload, no-ACK semantics, ~10 KB/s practical throughput at 115200, full sequence pseudocode mirroring `storage.js::uploadAlbum`, common-errors checklist.
- ✅ `references/21-original-firmware-stems.md` — full rewrite. New: canonical encoder pseudocode (port of `encodeToSP1`), corrected per-sector trailer (6 bytes at end of block 0 only, not 32 bytes spread across blocks), tempo formula `(48000*60)/(24*bpm)`, exact metadata-sector layout with X-fill, end-marker sector with magic at last 13 bytes, validation limits from ALBUM_ERR enum, retired "step 4 nonsense" caveat (canonical workflow now exists).
- ✅ `references/09-audio-format-spec.md` — small edits. Added 2026-05-13 confirmation note for byte layout, replaced "32 bytes side data per sector (8 per block)" with clarified write-spec vs read-spec distinction pointing at `references/21` and `corrections.md`.
- ✅ `references/11-block-interleaving-tape-fx.md` — single citation note added (encoder confirms `[0, 2, 1, 3]`).
- ✅ `references/10-midi-timing-encoding.md` — clarifying preamble distinguishing what stock firmware **reads** (TKT wiki layout, full per-block side data) vs what solderless encoder **writes** (6-byte subset, end of block 0 only). Replaced "tempo encoding unknown" subsection with the encoder formula + remaining firmware-side-semantic open question.
- ✅ `references/27-tools-and-utilities.md` — added "Local archive" subsection with location, provenance, contents list, run command. Updated "What's missing" list to reflect encoder pseudocode now exists.
- ✅ `sources.md` — added entry for the solderless archive with location, provenance, contents breakdown, citation format.
- ✅ `known-unknowns.md` — closed/revised 4 entries: eMMC write path (resolved), solderless source code (resolved via archive), album header byte ordering (resolved as LE), tempo field encoding (partially resolved with encoder formula).
- ✅ `working-confirmed.md` — added two large new sections: full SP-1 host protocol byte-level table (baud, framing, CRC, every opcode with source-file line refs) + album image format encoder reference table.
- ✅ `hallucination-watchlist.md` — added 8 new entries: 0x52 isn't a version check, 0x39 has no magic header, byte 4–7 is byte offset not sector address, no host-visible GPREGRET 0x1A96, no per-chunk ACK, tempo formula, no per-block 8-byte encoder trailer, no encoder-written sync counter.
- ✅ `corrections.md` — added 7 new entries with "Believed → Actual → Why → Source" structure: 0x52, 0x39 packet, GPREGRET, ACK, tempo, per-block trailer, encoder sync counter, plus archive-public update for solderless source.
- ✅ `synthesis-log.md` — bumped master synthesis date to 2026-05-13, added 2026-05-13 batch summary block.
- ✅ `SKILL.md` — updated "Is solderless online?" Q&A, "What's the album upload speed?" Q&A with three concrete data points, synthesis date bumped to 2026-05-13, added `update-log.md` to "Always read first" list.

### Deferred (not done this batch)

- FAQ companions (`15-bootloader-protocol-faq.md`, `16-usb-upload-protocol-faq.md`, `21-original-firmware-stems-faq.md`, `09-audio-format-spec-faq.md`) — small consistency passes to reflect the corrections. The FAQ files are short Q&A bites; updates can happen when a user-facing question reveals stale wording.
- A new dedicated `references/NN-solderless-archive-walkthrough.md` — guided tour of the four JS files for anyone reading the source directly. Useful but not blocking.
- `known-unknowns.md::Alternative bootloader modes` still mentions GPREGRET 0x1A96. The entry has good additional content (60-second window, USB DFU theoretical) so it stayed; needs a small follow-up edit to drop the 0x1A96 reference.

### Verification snapshot

Sanity checks performed post-edit:

- All file paths in citations match the bundled archive layout (`assets/solderless-2026-05-12/js/*.js`). After 2026-05-13 second-pass, the archive was moved into the repo at `assets/solderless-2026-05-12/` (flattened) and `assets/solderless-2026-05-12.zip`; the prior reference to `~/Projects/sp-1/cc-skill/resources/solderless-extracted/...` is obsolete.
- Cross-file references between `15-bootloader-protocol.md`, `16-usb-upload-protocol.md`, `21-original-firmware-stems.md`, `09-audio-format-spec.md`, `10-midi-timing-encoding.md`, `11-block-interleaving-tape-fx.md`, `27-tools-and-utilities.md`, `corrections.md`, `working-confirmed.md`, `hallucination-watchlist.md`, `sources.md`, `synthesis-log.md`, and `update-log.md` are mutually consistent.
- No file claims the host writes GPREGRET 0x1A96 directly.
- No file claims `0x39` has a magic-byte header or per-chunk ACK.
- No file claims the encoder writes 32 bytes of side data per sector.
- Tempo formula `(48000 * 60) / (24 * bpm)` appears in `21-original-firmware-stems.md`, `working-confirmed.md`, `corrections.md`, `hallucination-watchlist.md` consistently.
- "10510" and "00100" state strings appear in `15-bootloader-protocol.md`, `working-confirmed.md`, `hallucination-watchlist.md`, `corrections.md` consistently.

---

## 2026-05-13 — Intake batch #2: audiothingies + storagethingies bundled

### Source ingested

**Files:**
- `~/Projects/sp-1/cc-skill/resources/audiothingies.zip` (59 KB)
- `~/Projects/sp-1/cc-skill/resources/storagethingies.zip` (37 KB)

**Provenance:** ericlewis's C++17 header-only reference implementations of the stock audio engine and storage layer. Originally shared as Discord file attachments in #firmware on 2026-05-09 (audiothingies at 00:18:55 UTC, storagethingies at 00:21:46 UTC) to help moecal1947 understand how the stock SP-1 audio playback worked. Both were previously cited throughout the skill as `[code: audiothingies/...]` / `[code: storagethingies/...]` but treated as community-private "ask ericlewis in Discord" material.

### Why bundle now (and the social-risk tradeoff)

The skill had been describing these as "community-private — not in a public repo." That meant every citation pointed at files Claude couldn't read in a fresh clone. Practical pain: every "how does the audio engine work?" question forced Claude to either (a) hallucinate plausible details or (b) say "ask ericlewis." Both bad.

The risk in bundling: ericlewis didn't explicitly say "redistribute these freely." He shared them to a Discord channel, not to a public repo. Bundling them in a public GitHub repo (`dot-Justin/sp-1-knowledgebase-skill`) crosses from "semi-private community share" to "fully public." Per unverified community hearsay (logged in the user's auto-memory), there's a faction in the SP-1 dev group that's reportedly cautious about sharing — so this is exactly the kind of thing that could attract pushback.

User decision (logged 2026-05-13): bundle them anyway. Attribution is preserved everywhere ("ericlewis's C++17 reference implementation, originally Discord attachments 2026-05-09"). If pushback materializes, the plan is to add a repo link / accommodate the request in a future iteration.

### Layout chosen

- `assets/audiothingies-2026-05-09.zip` — original zip, byte-identical to the Discord attachment
- `assets/audiothingies-2026-05-09/` — flat extracted form (MacOS metadata stripped), citations use this path. Subdirs preserved: `effects/`, `backends/`.
- `assets/storagethingies-2026-05-09.zip` — original zip
- `assets/storagethingies-2026-05-09/` — extracted, but **flattened further**: the inner `storage/{,emmc/,format/}` directory hierarchy from the zip was collapsed because existing skill citations use the form `storagethingies/EmmcDriver.cpp` (no `storage/emmc/` prefix). All files now live at the top level.

### Changes landed

- ✅ `assets/audiothingies-2026-05-09/` — 19 files extracted, mac metadata stripped
- ✅ `assets/audiothingies-2026-05-09.zip` — original
- ✅ `assets/storagethingies-2026-05-09/` — 8 files extracted + flattened to top level
- ✅ `assets/storagethingies-2026-05-09.zip` — original
- ✅ All `` `audiothingies/...` `` and `` `storagethingies/...` `` markdown citations rewritten to `` `assets/audiothingies-2026-05-09/...` `` / `` `assets/storagethingies-2026-05-09/...` `` via sed
- ✅ `SKILL.md` item 8 — updated redistribution policy. Bundled corpus now: TE manuals, solderless snapshot, audiothingies, storagethingies. Not-bundled: stock TE firmware binary, Kanye stems.
- ✅ `sources.md` — added "Bundled assets" entries for both archives with full per-file layout + citation guidance. Moved old "private" entries to point at the new bundled descriptions.
- ✅ `synthesis-log.md` — updated source-coverage table rows for audiothingies/storagethingies from "private" → "bundled in skill"
- ✅ `working-confirmed.md` — updated section header from "(private — shared 2026-05-09 by ericlewis)" → "(bundled — ericlewis Discord shares, 2026-05-09)"
- ✅ `hallucination-watchlist.md` — updated the "ericlewis vs TimK" disambiguation block to say the code is now bundled in this skill
- ✅ `references/20-custom-firmware-state.md` — updated the firmware-state table entry from "Privately shared (not in a public repo)" → "Bundled in skill" with paths
- ✅ `references/20-custom-firmware-state-faq.md` — updated FAQs "What's in audiothingies/storagethingies?" and "Can I get them?"
- ✅ `references/27-tools-and-utilities.md` — promoted both archives from the "Private / shared-in-Discord tools" section into a new "Bundled-in-skill source archives" section with full descriptions
- ✅ `README.md` — updated the redistribution disclosure paragraph

### PII scan of bundled files

Performed before commit:

- Email/URL/credentials pattern scan — clean (no matches)
- Author / copyright / personal-identifier scan — clean (one false-positive on "lewis" matching `numeric_limits<int32_t>`)
- Filesystem-path scan (`/home/`, `/Users/`, `C:\Users\`) — clean
- TODO/FIXME with names — one match for "1-bit SPIM/PWM/PPI hack" in `EMMC.hpp`, which describes the technical SPIM single-bit-line workaround, not a person

Nothing identifying. Safe to push public.

### Citation form note

Both archives keep their original zip alongside the flat extracted form. Inside the skill, citations should use the extracted form (`assets/audiothingies-2026-05-09/AudioEngine.cpp`); the zip is preserved for byte-identical reference if someone needs to verify the archive wasn't modified post-extraction.

---

## 2026-05-13 — Intake batch #3: Lines thread archive (agent subset) bundled

### Source ingested

**Files:** Selected subset of `~/Projects/sp-1/cc-skill/resources/TE-SP-1-lines-thread-archive/`
**Provenance:** dotjustin's own public mirror of the Lines forum thread at `github.com/dot-Justin/TE-SP-1-lines-thread-archive`. Web frontend at `sp-1.dotjust.in`. Original thread: `llllllll.co/t/te-stem-player/66795` (846 posts, 2024-04-09 through 2026-05-06, closed by moderators).

### What was bundled vs skipped

The full archive on disk is ~213 MB across 4 top-level directories. Bundling the entire thing in a skill repo would be excessive (and most of that 213 MB is the rendered website source, which is irrelevant for agent use).

Bundled (~1.6 MB, 866 files):
- `assets/lines-thread-archive/agent/AGENT-GUIDE.md` (7 KB)
- `assets/lines-thread-archive/agent/thread.md` (4 KB — top-level narrative)
- `assets/lines-thread-archive/agent/summaries/chunk-NNN-MMM.md` (per-100-posts summaries, 9 files, 54 KB)
- `assets/lines-thread-archive/agent/indexes/*.json` (post-index, topic-index, reply-chain-index, participant-index, attachment-index — 200 KB total)
- `assets/lines-thread-archive/agent/posts/NNN.md` (all 846 posts as individual markdown with YAML frontmatter — 1.4 MB)
- `assets/lines-thread-archive/metadata/{participants,stats,upload_manifest}.json` (210 KB)
- `assets/lines-thread-archive/README.md` (skill-specific orientation + attribution, written 2026-05-13)

Skipped:
- `raw/api/posts/NNN.json` (4.4 MB) — original Discourse API responses, redundant with `agent/posts/`
- `raw/posts/` (3.8 MB) — alternate format of the same content
- `raw/assets/uploads/` (90 MB) — 813 binary attachments (images, PDFs, docs). Agents can resolve upload tokens via `metadata/upload_manifest.json` and fetch from the source repo or `sp-1.dotjust.in` when binary content is needed.
- `site-src/` (106 MB) — static web frontend source. Irrelevant for skill use.
- `assets/banner.png` — repo banner image.

### Why this layout

The `agent/` directory was purpose-built by dotjustin for AI consumption — pre-built indexes, narrative summaries, and per-post markdown files. It's the right path for skill use. The `raw/` content is for verification and full-fidelity access, which an agent can fetch on demand from the source repo if needed.

The bundled `assets/lines-thread-archive/README.md` orients agents to the bundled subset, explains what's NOT included, and points at the source repo for binary attachments.

### Why no social risk this batch

This is dotjustin's own publicly-licensed mirror. Bundling their own work in their own skill repo is fully self-attributed (`Source: github.com/dot-Justin/TE-SP-1-lines-thread-archive` callouts throughout).

The original Lines posts were public on `llllllll.co` since 2024-04-09. The participant handles (e.g., `charbot@gmail`) are how those users self-identified on the forum — already public.

### Changes landed

- ✅ `assets/lines-thread-archive/agent/` (4 dirs, 859 files, 1.4 MB)
- ✅ `assets/lines-thread-archive/metadata/` (3 files, 210 KB)
- ✅ `assets/lines-thread-archive/README.md` (skill-specific orientation + attribution)
- ✅ `sources.md` — added "Bundled assets > lines-thread-archive/" entry with full layout, citation form, attribution. Updated existing GitHub-repo entry to point at the bundled subset as primary, source repo as fallback. Updated "Local corpus" entry to defer to bundled subset.
- ✅ `references/27-tools-and-utilities.md` — added "Bundled subset" note to the `TE-SP-1-lines-thread-archive` repo entry.

### PII scan

- Email/credentials pattern: only `charbot@gmail` (a Lines forum handle — public, self-chosen).
- No private filesystem paths.
- No author/copyright tags beyond what's already public in the forum content.

The forum posts contain user opinions, real names where the users chose to share them, and email-like handles (some users picked email-style usernames). All of this is content the users themselves posted publicly to `llllllll.co` — not a privacy issue to mirror.

---

## 2026-05-13 — Intake batch #4: SP-1-dev + sp1-midi + TKT wiki bundled

### Source ingested

Three additions in one batch, all from public MIT-licensed GitHub repos plus a wiki:

1. **`github.com/timknapen/SP-1-dev`** (TimK's documentation repo): `LICENSE` + `README.md` + `src/stemplayer_pins.h`. Bundled at `assets/SP-1-dev-2026-05-13/`. Skipped: `img/` (642 KB of device illustrations — graphics-heavy, agents don't need; fetch from source if needed) and `icon.png`.
2. **TKT wiki pages** (14 pages on `github.com/timknapen/SP-1-dev/wiki`): Home, Hardware-overview, Peripherals, Getting-started, Bootloader, Data-Structure, Album-metadata-format, Audio-format, I2S, I2C, PWM, SAADC, Battery-charger, Bluetooth-Module. Fetched via `raw.githubusercontent.com/wiki/timknapen/SP-1-dev/<Page>.md`. Bundled at `assets/SP-1-dev-2026-05-13/wiki/` (24 KB total).
3. **`github.com/ericlewis/sp1-midi`** (ericlewis's Zephyr BSP): full repo snapshot. Bundled at `assets/sp1-midi-2026-05-13/` (305 KB, 50 files: CMakeLists, Kconfig, app.overlay, boards/, drivers/, subsys/, app/, dts/, include/, prj.conf, README, LICENSE).

### Why bundle now

The skill cites the TKT wiki 50+ times and `stemplayer_pins.h` / `stem_player.dts` dozens of times. Every reference was previously a fetch-on-demand network round-trip away. Bundling makes the skill self-contained and offline-usable for the most-cited authoritative sources.

Both repos are MIT-licensed (`Copyright (c) 2026 Tim Knapen` / `Copyright (c) 2026 Eric Lewis`); attribution preserved. No social risk this batch — both are public open-source repos with explicit redistribution permission via MIT.

### Naming convention

Used `<repo>-2026-05-13` to mark the snapshot date. Both repos may evolve upstream; the bundled copies are pinned to today's state. Future re-snapshots would land at `<repo>-YYYY-MM-DD` for clarity.

The TKT wiki lives at `assets/SP-1-dev-2026-05-13/wiki/` (combined with TimK's repo rather than as a separate `assets/SP-1-dev-wiki-2026-05-13/`) because the wiki is the documentation companion to the repo and they're conceptually one unit.

### Changes landed

- ✅ `assets/SP-1-dev-2026-05-13/{LICENSE,README.md,src/stemplayer_pins.h,wiki/*.md}` (17 files, 86 KB)
- ✅ `assets/sp1-midi-2026-05-13/` (50 files across boards/, drivers/, subsys/, app/, dts/, include/, plus toplevel build files; 305 KB)
- ✅ Sed-swapped citations across the skill:
  - `` `sp1-midi/X` `` and `` `ericlewis/sp1-midi/X` `` → `` `assets/sp1-midi-2026-05-13/X` ``
  - `` `SP-1-dev/src/X` `` and `` `timknapen/SP-1-dev/src/X` `` → `` `assets/SP-1-dev-2026-05-13/src/X` ``
  - `` `SP-1-dev/README.md` `` → `` `assets/SP-1-dev-2026-05-13/README.md` ``
  - Backtick-anchored to avoid matching the GitHub URL `github.com/timknapen/SP-1-dev` which should remain as a public repo URL
- ✅ `sources.md` — added two new "Bundled assets" entries (SP-1-dev-2026-05-13 with full wiki listing; sp1-midi-2026-05-13 with key-path listing) plus citation-form guidance for both
- ✅ `references/27-tools-and-utilities.md` — updated `github.com/timknapen/SP-1-dev` and `github.com/ericlewis/sp1-midi` entries to point at the bundled paths as the primary access route
- ✅ `SKILL.md` item 8 — added both new bundles to the redistributable-corpus list (with explicit MIT-license note implicit in the descriptions)
- ✅ `hallucination-watchlist.md` — updated the SP-1-dev "no Python/Rust tools" entry to note the repo is now bundled (so the claim can be self-verified by reading the bundled subset)

### PII scan

- Email/credentials pattern: clean
- Filesystem paths: clean
- Author/copyright: only MIT LICENSE headers (Copyright 2026 Tim Knapen / Eric Lewis — required for attribution) and the `@author Tim Knapen` + `@copyright Copyright (c) 2025` annotations in `stemplayer_pins.h` (standard MIT-licensed-file attribution with the author's public personal site URL `timknapen.be`). All public, all part of how the authors chose to attribute themselves on their MIT-licensed code.

### Things still NOT bundled

- `libpo32/` — different product (PO-32 Tonic). Hallucination-risk to bundle since the skill repeatedly warns against conflating PO-32 with SP-1.
- `TE-SP-1-Dev-discord-archive/` — private scrape; the skill explicitly says "Do not link to this."
- Vendor datasheets (nRF52840, CS42L42, TAS2505, BQ24232, CYBT-353027-02) — all public, all large, fetchable on demand.
- Stock TE firmware binary — community-private, never bundled.
- Kanye stems — copyrighted, never bundled.
- `theunflappable/test_bootloader.py` — not on local disk in `resources/`; would need to fetch from Discord. Same Discord-attachment provenance as audiothingies. Could be added in a future batch if obtained.

### Current `assets/` corpus after batch #4

- TE-Stem-Player-manual.pdf, TE-Stem-User-Guide.pdf, TE-Stem-User-Guide.docx (TE official manuals)
- solderless-2026-05-12/ + .zip (public web tool snapshot)
- audiothingies-2026-05-09/ + .zip (ericlewis audio engine ref impl)
- storagethingies-2026-05-09/ + .zip (ericlewis eMMC ref impl)
- lines-thread-archive/ (846-post forum mirror, agent subset)
- SP-1-dev-2026-05-13/ (TimK's repo + wiki)
- sp1-midi-2026-05-13/ (ericlewis's Zephyr BSP)

---

## 2026-05-14 — Empirical batch #1: Unit A bootloader observed

### Source ingested

**Type:** Live USB observation, not a file. Captured against a factory Unit A in T1+T4+USB-C bootloader mode on host `dotj-ct-dev-01` (Debian 12) and the user's laptop (Arch), 2026-05-14.

**Specific evidence collected:**

- `lsusb`: `Bus 003 Device 013: ID 2367:1701 Teenage Engineering stem player`
- `/dev/serial/by-id/usb-teenage_engineering_stem_player_<SERIAL>-if00 -> /dev/ttyACM1`
- `serialport.list()` JSON: `{ vendorId: "2367", productId: "1701", manufacturer: "teenage engineering", serialNumber: "<12-hex>" }`
- `0x52` reply payload over CDC: **5 raw integer bytes `[0x00, 0x00, 0x01, 0x00, 0x00]`** (NOT ASCII `"00100"` = `[0x30, 0x30, 0x31, 0x30, 0x30]`)

### Why it matters

Two skill gaps that had been silently wrong:

1. **USB VID/PID for the factory bootloader was undocumented.** The skill talked about ericlewis's `1209/0001` (PID.codes test pair) but never the factory's actual IDs. Anyone writing host code that filters by USB VID/PID would either not find the factory device or have to "use the chooser" UX. Now the IDs are first-class in `references/15-bootloader-protocol.md`.

2. **State-query payload was described as "5-byte ASCII string."** Misleading enough that an AI implementation (the dumper project) coded `String.fromCharCode(b)` and got `    ` instead of `"00100"`, causing a silent boot-mode-check failure against real hardware. The bytes are raw integers, stringification is `Array.from(b).join('')`. The reference implementations (`gate-fix-testing/src/upload/protocol.mjs::decodeStateReply`, `sp-1-stem-tool/src/sp1/protocol.ts`) both do it correctly with the comment "Bootloader emits 5 BINARY bytes, one digit per byte. Join via Array.join('')."

### Changes landed

- ✅ `references/15-bootloader-protocol.md` — added **USB device identifiers** section with VID `2367` / PID `1701`, sample discovery filter array showing both factory and ericlewis-custom IDs. Rewrote the "What state-query returns" section to explicitly say "5 raw integer bytes, NOT 5 ASCII characters" with the literal byte values. Added a "Common AI bug" callout about `String.fromCharCode` and dropped the misleading "5-byte ASCII string" framing. Clarified that boot mode is firmware-flash-ready (no mode transition needed before `0x46`).
- ✅ `hallucination-watchlist.md` — updated the existing "Don't say `0x52` is a bootloader version check" entry with the raw-bytes detail; added **"Don't claim state-query bytes are ASCII characters of digits"** entry pinning the AI bug with citation to the dumper project's 2026-05-14 incident; added **"Don't claim firmware-flash requires flash mode before `0x46`"** entry.
- (Implicit) The dumper project's `host/src/cdc/bootloader.mjs::queryFlashState` now uses the correct `Array.from(payload).join('')` and a unit test (`tests/unit/queryFlashState.test.mjs`) captures the regression so it can't re-occur.

### Verification snapshot

- Re-ran HW-01 + HW-02 against Unit A: HW-01 PASS (270ms), HW-02 PASS (boot mode recognized).
- Diff'd `15-bootloader-protocol.md` against the dumper project's `discover.mjs` filter arrays — same VIDs/PIDs.
- 77 + 4 = 81 host unit tests in the dumper project still pass.

### Deferred

- The exact PID for ericlewis/sp1-midi was `0x0001` based on `app/main.cpp`, but I have no evidence the factory firmware uses anything else than `2367/1701`. If a future user reports a different PID (a different SP-1 unit cohort, or a recent firmware revision), append it to `SP1_BOOTLOADER_FILTERS`.
- The `2367:1701` was observed on Unit A which has the user's gate-trials custom album written to eMMC. The PID for the factory bootloader is the *firmware* PID, not affected by what's on the eMMC. So the same IDs should be valid for Unit B (sealed, factory state).
