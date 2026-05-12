# Synthesis Log — SP-1 Skill

This skill is a frozen snapshot of community knowledge as of a specific date. The Discord is still active. Knowledge that was true at synthesis date may have been corrected, expanded, or contradicted by subsequent posts.

## Master synthesis date

**2026-05-12.** Last source incorporated through this date.

### Updates on 2026-05-12 phase 2 (TKT wiki round-2 + `stemplayer_pins.h` + ericlewis DTS cross-check)

After the initial TKT wiki incorporation, the remaining 10 wiki pages were fetched, the `SP-1-dev` repo's README and `src/stemplayer_pins.h` were read, and ericlewis's `stem_player.dts` was cross-referenced. Findings committed individually so each shows in commit history.

Per-commit summary:

- **`Fix I2S master/slave...`** — corrects working-confirmed and 06-audio-codecs to say nRF I²S is slave (BCLK from 3.072 MHz osc, LRCLK from CS42L42). Adds buffer size (256 samples / 128 frames).
- **`Add I2C bring-up details...`** — 4.7 kΩ pull-ups, 2.5 ms post-reset wait, HP_CTL mute behavior (`0x0D` mute, `0x01` unmute headphones), headphone-detect bit 7 of `0x1B77` flagged as potentially distinct from `0x130F`.
- **`Document BQ24232 USB current cap...`** — 500 mA from USB confirmed; replaces "actual charge current is not publicly documented."
- **`Document second 3.5mm jack and unused nRF radio`** — SP-1 has two jacks (TRRS for headphones + MIDI/PO sync), nRF radio explicitly unused (no antenna on PCB).
- **`Resolve BQ24232 known-unknown...`** — BQ24232 driver completeness mostly closed (ISET=P1.00, not P0.19; the README in sp1-midi was the source of the wrong claim). New corrections entry. New known-unknown for second-jack pin mapping.
- **`(this commit) Update synthesis log...`** — bump date, log round-2 incorporation; enumerate wiki pages with their topical content.

**Still open after round 2:**
- 0x39 packet framing for bootloader upload (not in any wiki page)
- Specific BQ24232 ISET resistor values (on schematic, not in code or wiki)
- Second-jack MIDI/PO sync pin mapping (no public code labels these)
- Tempo field encoding (raw BPM? Q8.8?)
- Album header endianness (presumed LE, not confirmed)
- Sample bit-alignment discrepancy (wiki shows left-aligned formula vs. audiothingies right-aligned)
- PWM and SAADC wiki pages are stubs

### Updates on 2026-05-12 phase 1 (TKT wiki initial incorporation)

Fetched and incorporated content from `github.com/timknapen/SP-1-dev/wiki` pages: `Bootloader`, `Data-Structure`, `Album-metadata-format`, `Audio-format`. Specific changes:

- **`references/09-audio-format-spec.md`** — added the album header byte layout (`ALBUM_PRESENT` magic, song entries at offset 82, 136 bytes each), refined sector trailer description to per-block (2 sync + 2 tempo + 4 LED).
- **`references/10-midi-timing-encoding.md`** — split the previously-named "32-bit sync word" into a 16-bit sync counter + 16-bit tempo field per block, per the wiki. Tempo field encoding is a new known-unknown.
- **`references/21-original-firmware-stems.md`** — resolved two inline "Open question" markers (album magic value, side-data layout) with wiki-confirmed answers; updated step 5 to use sync_counter_for_block instead of sync_word_for_block.
- **`sources.md`** — enumerated all 14 wiki pages with descriptions; clarified that the Bootloader page covers app integration but NOT the `0x39` packet framing.
- **`corrections.md`** — added three new entries documenting the sync/tempo split, the `ALBUM_PRESENT` magic value resolution, and the per-block trailer placement.
- **`known-unknowns.md`** — added three new entries: sample bit-alignment discrepancy (wiki shows left-aligned formula, `audiothingies` is right-aligned), album header endianness, and tempo field encoding.

**Confirmed unchanged:** The `0x39` packet header format and ACK protocol for the bootloader upload path is NOT in TKT's wiki (`Bootloader` page is about app-integration, not the wire format). This open question remains in `known-unknowns.md`.

**Caveat:** wiki content was read via WebFetch's summarizer, which has known imprecision. Field offsets in the album header are self-consistent (13 + 4 + 1 + 64 = 82; 4 + 4 + 64 + 64 = 136) but should be verified against the live wiki before committing to production code.

## Source coverage at synthesis

| Source | Coverage | Notes |
| --- | --- | --- |
| Lines forum thread | Posts 1–846 (2024-04-09 to 2026-05-06) | Thread closed by moderators 2026-05-06 over copyright/spam concerns. Still readable. |
| Discord — #general | 2026-05-06 through 2026-05-11 | Server launched 2026-05-06 |
| Discord — #firmware | 2026-05-06 through 2026-05-11 | Includes ericlewis tape-FX explanation (2026-05-09) and shared `audiothingies.zip` / `storagethingies.zip` |
| Discord — #hardware | 2026-05-06 through 2026-05-11 | Includes fishdog_'s eMMC desolder method |
| Discord — #news | 2026-05-04 through 2026-05-11 | Includes update-utility offline notice (2026-05-09) |
| Discord — #introductions | Not technically substantive — skipped except for handle/handle disambiguation |
| `github.com/timknapen/SP-1-dev` | Through 2026-05; wiki pages re-fetched 2026-05-12 | All 14 wiki pages enumerated; `Bootloader`, `Data-Structure`, `Album-metadata-format`, `Audio-format` contents incorporated |
| `github.com/ericlewis/sp1-midi` | Posted 2026-05-09 by ericlewis | Zephyr BSP for MIDI-controller mode; not a stem-player firmware |
| `audiothingies.zip` (private — shared Discord 2026-05-09 by ericlewis) | C++17 audio engine, mixer, DSP nodes | Authoritative for audio engine internals |
| `storagethingies.zip` (private — shared Discord 2026-05-09 by ericlewis) | C++17 eMMC driver, DiskManager | Authoritative for storage internals |
| `github.com/ericlewis/libpo32` | C99 library for a different TE product (PO-32) | Referenced only for context |

## Freshness rules

1. When the Discord has been updated since the synthesis date, prefer the user's live observation over the synthesized claim.
2. Solderless.engineering was offline as of 2026-05-09 for an update. Status may have changed.
3. Two custom firmwares were announced but unreleased as of synthesis date:
   - **emvee1968** (Claude Code Opus 4.7, basic playback + gate effect + working Bluetooth pairing) — awaiting 2 more units before release
   - **virtualflannel_46386** (custom OS with FX page: beat repeat, bit crusher, send delay, filter, pitch ±12 cents, mixer with mutes/solo) — awaiting solderless utility return
   - If these have been released since synthesis, references to them in this skill are stale.
4. The USB upload protocol opcodes (see `references/16-usb-upload-protocol.md`) were partially documented by ericlewis on 2026-05-09 but the exact CDC packet framing for opcode `0x39` was never publicly shared. Either get it from the live Discord, ask ericlewis, or read the working updater code.

## Identity map

Some handles map across platforms. Synthesized references use a single canonical name per person.

| Lines handle | Discord handle | GitHub | Canonical name used in this skill |
| --- | --- | --- | --- |
| TimK | tkt1000 | timknapen | **TimK** (or tkt1000 when citing Discord directly) |
| maybe | sankebergel | — | **maybe** (Lines) / **sankebergel** (Discord) |
| nonkillwingman | _isaaczz | — | **nonkillwingman** (Lines) / **_isaaczz** (Discord) |
| — | ericlewis | ericlewis | **ericlewis** |

The `ericlewis` Discord handle and the GitHub `ericlewis` who published `sp1-midi` and `libpo32` are the same person. Active in Discord from 2026-05-08 onward. Not the same person as TimK; do not conflate the two repositories — `timknapen/SP-1-dev` is documentation and `ericlewis/sp1-midi` is a Zephyr BSP for MIDI-controller mode.

## Update procedure

When new Discord content lands and this skill needs to absorb it:

1. Re-read the affected `references/NN-topic.md` file
2. Add new findings with their citation
3. If a new finding contradicts an existing claim, **move the old claim to `corrections.md`** and write the new claim with citations to both old and new
4. Update this file's master synthesis date
5. Update the affected reference file's per-file synthesis date stamp

Do not delete corrected content. Move it to `corrections.md` so Claude can reason about why a claim that "feels right from older notes" is now known to be wrong.
