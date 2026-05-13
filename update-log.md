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
