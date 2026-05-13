# Sources

This file describes **all primary sources** Claude can consult when the synthesized reference files in `references/` don't have the answer. Sources fall into two categories:

1. **Bundled with the skill** — files in `assets/` distributed with this skill, available to any user
2. **External or corpus-dependent** — files the user might have locally (the Lines thread archive, the Discord scrape, the source code repos), or public URLs anyone can hit

Always cite which category a piece of information came from. If Claude needs to verify a claim and the user has a local corpus, the corpus paths below let Claude grep through primary sources.

---

## Bundled assets (always available)

These files ship with the skill at `~/.claude/skills/sp-1/assets/`:

### `TE-Stem-Player-manual.pdf`
**The full Teenage Engineering Stem Player manual.** Original TE document distributed with units. Covers user-facing button operations, behavior of the stem player, official feature descriptions. ~509 KB.

Source: Lines #69 — uploaded by an early community member. The community has treated it as freely shareable (it's a manual for a device whose units are in the wild).

Use this when:
- User asks about an official user-facing feature ("what does the function button do?")
- Verifying a feature's official name vs. community nickname
- Understanding the device's intended UX before designing a custom UX

### `TE-Stem-User-Guide.pdf`
A shorter user guide, distinct from the full manual. Quick-reference style. ~59 KB.

Source: Lines #464. Similar redistributability assumptions as the manual.

### `TE-Stem-User-Guide.docx`
The same user guide content, but in editable Word format. ~16 KB.

Source: Lines #1 (post by PedalsandChill, the thread originator). Useful if you need to extract or quote text without OCR.

### `solderless-2026-05-12/` and `solderless-2026-05-12.zip`

**Bundled archive of `solderless-engineering.pages.dev` as of 2026-05-12.** Complete static deployment — HTML, CSS, JS, fonts, images. The JS modules are the **canonical reference implementation** for the SP-1 host-side bootloader + album-upload protocol.

**Provenance:** Obtained as a community backup on 2026-05-13. Archive contents reflect the 2026-05-12 deployed state (the deployed JS/HTML is dated 2026-05-09; README inside the archive is dated 2026-05-12).

**Layout** (flattened from the original nested form):

- `assets/solderless-2026-05-12/README.txt` — included run instructions from the archive
- `assets/solderless-2026-05-12/index.html` — main UI (firmware flash + album upload + device info)
- `assets/solderless-2026-05-12/stemloader.html` — TKT's standalone stem-loader UI
- `assets/solderless-2026-05-12/js/protocol.js` — **canonical** protocol implementation: CRC-8 table, COBS encode/decode, packet framing, serial I/O, all album/flash constants
- `assets/solderless-2026-05-12/js/firmware.js` — firmware flash sequence (R/F/E/H opcodes)
- `assets/solderless-2026-05-12/js/storage.js` — album upload sequence (R/0x70/0x50/0x37/0x39/0x51 + metadata/end-marker construction)
- `assets/solderless-2026-05-12/js/wav-parser.js` — WAV → SP-1 encoder (`encodeToSP1`)
- `assets/solderless-2026-05-12/stemloader/js/*.js` — same protocol, TKT's earlier UI
- `assets/solderless-2026-05-12.zip` — original unmodified zip, in case you need the byte-identical archive

**Run it locally:**

```sh
cd assets/solderless-2026-05-12
python3 -m http.server 8788
# Open http://127.0.0.1:8788/ in Chrome or Edge (Safari does not support Web Serial)
```

**Cite as:** `[Source: assets/solderless-2026-05-12/js/storage.js lines X-Y]`. Treat as canonical reference for SP-1 host-side protocol details.

### `SP-1-dev-2026-05-13/` (TimK's repo: canonical pin map + wiki)

**Bundled snapshot of `github.com/timknapen/SP-1-dev` (MIT licensed, Copyright Tim Knapen 2026).** Contains the authoritative pin definitions plus all 14 wiki pages as markdown.

**Layout:**

- `assets/SP-1-dev-2026-05-13/LICENSE` — MIT
- `assets/SP-1-dev-2026-05-13/README.md` — TimK's repo README
- `assets/SP-1-dev-2026-05-13/src/stemplayer_pins.h` — **the canonical SP-1 pin map**, cited dozens of times throughout this skill. C header; works with both nRF SDK and Zephyr.
- `assets/SP-1-dev-2026-05-13/wiki/Home.md` — wiki landing page
- `assets/SP-1-dev-2026-05-13/wiki/Hardware-overview.md` — chip-level overview
- `assets/SP-1-dev-2026-05-13/wiki/Peripherals.md` — peripheral map
- `assets/SP-1-dev-2026-05-13/wiki/Getting-started.md` — dev environment intro
- `assets/SP-1-dev-2026-05-13/wiki/Bootloader.md` — app-integration spec (flash offset 0x20000, max size 0xDEFFF, 5s watchdog, pre-initialized peripherals, RESETREAS cleanup, SYSTEM_OFF requirement)
- `assets/SP-1-dev-2026-05-13/wiki/Data-Structure.md` — sector = 0x2000 (8192) bytes = 16 native 512-byte eMMC blocks; first sector is album metadata
- `assets/SP-1-dev-2026-05-13/wiki/Album-metadata-format.md` — full album header layout (`ALBUM_PRESENT` magic, song entries at offset 82, 136 bytes each)
- `assets/SP-1-dev-2026-05-13/wiki/Audio-format.md` — sector trailer layout (per the wiki: 2 sync + 2 tempo + 4 LED per block; see `corrections.md` for the encoder-side reality)
- `assets/SP-1-dev-2026-05-13/wiki/I2S.md` — nRF I²S slave mode, BCLK from 3.072 MHz external osc, LRCLK from CS42L42, 256-sample / 128-frame buffer
- `assets/SP-1-dev-2026-05-13/wiki/I2C.md` — codec addresses, pull-up values, init sequences, mute behavior
- `assets/SP-1-dev-2026-05-13/wiki/PWM.md` — (stub: PWM drives LEDs, no detail)
- `assets/SP-1-dev-2026-05-13/wiki/SAADC.md` — (stub: heading only)
- `assets/SP-1-dev-2026-05-13/wiki/Battery-charger.md` — BQ24232 pin map (ISET=P1.00), 500 mA USB cap
- `assets/SP-1-dev-2026-05-13/wiki/Bluetooth-Module.md` — CYBT-353027-02 UART pin map + secondary SPI control path

**Cite as:** `[code: assets/SP-1-dev-2026-05-13/src/stemplayer_pins.h]` or `[TKT wiki: Audio-format, assets/SP-1-dev-2026-05-13/wiki/Audio-format.md]`.

**Note:** `img/` and `icon.png` were intentionally excluded (642 KB of device illustrations; not needed for agent reasoning). For the illustrations, fetch from `github.com/timknapen/SP-1-dev` directly.

### `sp1-midi-2026-05-13/` (ericlewis's Zephyr BSP)

**Bundled snapshot of `github.com/ericlewis/sp1-midi` (MIT licensed, Copyright Eric Lewis 2026).** Public, posted 2026-05-09. **The only buildable public custom firmware** for the SP-1.

This BSP implements a USB MIDI 2.0 controller — **not** a stem player. But the board definitions, drivers, and BSP infrastructure are the practical foundation for any custom SP-1 firmware.

**Key paths:**

- `assets/sp1-midi-2026-05-13/LICENSE` — MIT
- `assets/sp1-midi-2026-05-13/README.md` — build instructions, project overview
- `assets/sp1-midi-2026-05-13/CMakeLists.txt`, `Kconfig`, `prj.conf`, `app.overlay` — Zephyr build config
- `assets/sp1-midi-2026-05-13/boards/teenageengineering/stem_player/` — board files: `stem_player.dts` (canonical DTS), `stem_player_defconfig`, pinctrl
- `assets/sp1-midi-2026-05-13/drivers/audio/cs42l42_codec.{c,h}` — Cirrus CS42L42 headphone codec driver (I²C addr 0x48)
- `assets/sp1-midi-2026-05-13/drivers/audio/tas2505_codec.{c,h}` — TI TAS2505 speaker amp driver (I²C addr 0x18)
- `assets/sp1-midi-2026-05-13/drivers/charger/charger_bq24232.c` — BQ24232 GPIO-based charger control
- `assets/sp1-midi-2026-05-13/subsys/storage/emmc/EmmcDriver.cpp` — eMMC read driver (bit-banged CMD + SPIM3 DAT0)
- `assets/sp1-midi-2026-05-13/subsys/power/PowerManager.cpp` — battery/charger/USB monitoring
- `assets/sp1-midi-2026-05-13/subsys/system/{Watchdog,ResetBreadcrumbs}.cpp` — system services
- `assets/sp1-midi-2026-05-13/app/` — MIDI controller app skeleton (faders → CC, buttons → notes, transport → MMC), state machine (IDLE / RUNNING / SHUTTING_DOWN / DEEP_SLEEP), LED PWM

**Cite as:** `[code: assets/sp1-midi-2026-05-13/boards/teenageengineering/stem_player/stem_player.dts]` or similar.

**Build:** see `references/18-zephyr-build-environment.md` and `references/19-sp1-midi-bsp.md` for the full bring-up. Requires Zephyr SDK + west.

### `audiothingies-2026-05-09/` and `audiothingies-2026-05-09.zip`

**Bundled archive of ericlewis's C++17 reference implementation of the stock audio engine.** Originally shared as a Discord file attachment in #firmware on 2026-05-09 00:18:55 UTC. The flat extracted tree + the original zip are both present in `assets/`.

**Layout:**

- `assets/audiothingies-2026-05-09/AudioEngine.{cpp,hpp}` — playback state machine (Play / Slow / FF / RW transports), smoothed speed transitions, frame-accurate sync word and LED word generation
- `assets/audiothingies-2026-05-09/StockRuntimeMixer.{cpp,hpp}` — Q12 fixed-point fader weights, 9-step cubic output gain curve, block-skipping for FF, solo mask
- `assets/audiothingies-2026-05-09/VarispeedResampler.hpp` — sub-sample interpolation, Q24 fixed-point phase increment
- `assets/audiothingies-2026-05-09/PcmPacking.hpp` — `float_to_pcm_right24_fast` (right-aligned 24-bit-in-int32 representation)
- `assets/audiothingies-2026-05-09/AudioNode.hpp` — base audio-node interface
- `assets/audiothingies-2026-05-09/effects/` — `BiquadFilter`, `ChorusFlangerNode`, `DelayEchoNode`, `DistortionNode`, `GateEnvelopeNode`, `StemEffectRack`, `DspTables`, `DspUtils`, `EffectNode`
- `assets/audiothingies-2026-05-09/backends/zephyr_i2s_tx.{cpp,hpp}` — Zephyr I²S TX backend

**Cite as:** `[Source: assets/audiothingies-2026-05-09/AudioEngine.cpp lines X-Y]` or `[code: assets/audiothingies-2026-05-09/effects/BiquadFilter.hpp]`.

### `lines-thread-archive/`

**Bundled subset of the SP-1 Lines forum thread archive.** Includes all 846 posts as agent-friendly markdown, narrative summaries, JSON indexes, and thread metadata. Excludes 90 MB of binary attachments (images, PDFs, docs from posts) which remain in the source repo only.

**Source:** [`github.com/dot-Justin/TE-SP-1-lines-thread-archive`](https://github.com/dot-Justin/TE-SP-1-lines-thread-archive) (dotjustin, public). Web frontend: [`sp-1.dotjust.in`](https://sp-1.dotjust.in/).

**Original thread:** [`llllllll.co/t/te-stem-player/66795`](https://llllllll.co/t/te-stem-player/66795) — posts 1–846, 2024-04-09 through 2026-05-06, closed by moderators.

**Layout:**

- `assets/lines-thread-archive/README.md` — what's included, what's not, citation guidance
- `assets/lines-thread-archive/agent/AGENT-GUIDE.md` — folder navigation + search strategies
- `assets/lines-thread-archive/agent/thread.md` — top-level narrative summary
- `assets/lines-thread-archive/agent/summaries/chunk-NNN-MMM.md` — per-100-posts narrative summaries (9 chunks)
- `assets/lines-thread-archive/agent/indexes/post-index.json` — all 846 posts with metadata + excerpt
- `assets/lines-thread-archive/agent/indexes/topic-index.json` — posts grouped by topic (hardware, audio, bootloader, ...)
- `assets/lines-thread-archive/agent/indexes/reply-chain-index.json` — parent → replies mapping
- `assets/lines-thread-archive/agent/indexes/participant-index.json` — participant metadata
- `assets/lines-thread-archive/agent/indexes/attachment-index.json` — upload tokens with original filenames
- `assets/lines-thread-archive/agent/posts/NNN.md` — individual posts (001 through 846)
- `assets/lines-thread-archive/metadata/{participants,stats,upload_manifest}.json` — thread-level metadata

**Cite as:** `[Lines #NNN, <author>, <YYYY-MM-DD>]` (same form as elsewhere in the skill — the post number resolves against `post-index.json`).

**For attachments** (images, PDFs, docs from forum posts): not bundled here due to size. Use `upload_manifest.json` to map a Discourse upload token to its original filename, then fetch from the source repo's `raw/assets/uploads/` or browse the web frontend.

### `storagethingies-2026-05-09/` and `storagethingies-2026-05-09.zip`

**Bundled archive of ericlewis's C++17 reference implementation of the eMMC driver and DiskManager.** Originally shared as a Discord file attachment in #firmware on 2026-05-09 00:21:46 UTC. The flat extracted tree + the original zip are both present in `assets/`.

**Layout** (the original `storage/{,emmc/,format/}` nesting has been flattened):

- `assets/storagethingies-2026-05-09/DiskManager.{cpp,hpp}` — 10-slot prefetch buffer, album metadata, frame → sector mapping, `decode_te_frame_payload_i32` (authoritative audio frame byte-layout decoder)
- `assets/storagethingies-2026-05-09/EmmcDriver.{cpp,hpp}` — 11-state SMF init (CMD0 → CMD16), async DMA-style reads via SPIM3 + PWM
- `assets/storagethingies-2026-05-09/StemEmmcDevice.{cpp,hpp}` — higher-level wrapper around the driver
- `assets/storagethingies-2026-05-09/EMMC.hpp` — command opcodes / constants
- `assets/storagethingies-2026-05-09/DiskFormat.hpp` — sector/block constants

**Cite as:** `[Source: assets/storagethingies-2026-05-09/DiskManager.hpp lines X-Y]` or `[code: assets/storagethingies-2026-05-09/EmmcDriver.cpp]`.

---

## Public GitHub repos (anyone can access)

These are URLs anyone can visit. The skill cites these inline; Claude can also fetch them as web pages if needed.

### `github.com/timknapen/SP-1-dev`
**Owner:** TimK
**Contents:** Canonical hardware documentation. `README.md`, `src/stemplayer_pins.h` (the C-style pin definitions), images, LICENSE. Plus a GitHub Wiki with 14 technical pages.

**Wiki pages** (verified 2026-05-12 via WebFetch):

1. **Home** — `/wiki`
2. **Hardware overview** — `/wiki/Hardware-overview`
3. **Peripherals** — `/wiki/Peripherals`
4. **Getting started** — `/wiki/Getting-started`
5. **Bootloader** — `/wiki/Bootloader` — covers app integration (flash offset 0x20000, max size 0xDEFFF, 5s watchdog, pre-initialized peripherals, `RESETREAS` cleanup, `SYSTEM_OFF` requirement). **Does not cover the upload protocol wire format or `0x39` packet framing.**
6. **Data Structure** — `/wiki/Data-Structure` — sector = 0x2000 (8192) bytes = 16 native 512-byte eMMC blocks; first sector is album metadata, audio starts at `0x2000`.
7. **Album metadata format** — `/wiki/Album-metadata-format` — full album header layout (`ALBUM_PRESENT` magic at offset 0 and at end, song entries at offset 82, 136 bytes each). **The authoritative source for album header structure** — supersedes the in-memory C struct in `assets/storagethingies-2026-05-09/DiskManager.hpp`.
8. **Audio format** — `/wiki/Audio-format` — 4 blocks × 2048 bytes = 8192-byte sector, blocks ordered {0,2,1,3}, **per-block trailer of 2 sync + 2 tempo + 4 LED bytes**, sample extraction formula. **The authoritative source for the sector trailer layout.**
9. **I2S** — `/wiki/I2S` — confirms nRF I²S is in **slave** mode (BCLK from 3.072 MHz external osc, LRCLK from CS42L42 PLL). Buffer size 256 samples / 128 frames. 24-bit left-aligned.
10. **I2C** — `/wiki/I2C` — addresses (CS42L42=0x48, TAS2505=0x18), 4.7 kΩ pull-ups, 400 kHz max, 2.5 ms post-reset wait, PLL register addresses, mute register behavior, headphone detect bit. (`SAADC`, `PWM` pages currently lack technical detail.)
11. **PWM** — `/wiki/PWM` — stub: confirms PWM drives track + play LEDs; no detail yet.
12. **SAADC** — `/wiki/SAADC` — stub: heading only.
13. **Battery charger** — `/wiki/Battery-charger` — BQ24232 pin map (ISET=P1.00 authoritative, supersedes earlier P0.19 claim), 500 mA USB cap, ISET voltage reflects charge current.
14. **Bluetooth Module** — `/wiki/Bluetooth-Module` — UART pin map + `PIN_CY_SPI_CSN` (P1.05) noted as a secondary control path. TKT explicitly hasn't worked on BT.

**Wiki content stability:** as of 2026-05-12, several pages are stubs ("It'll be done when it's done"). Re-check pages periodically; if a stub page has new content it may close known-unknowns.

Raw page markdown can be fetched at `https://raw.githubusercontent.com/wiki/timknapen/SP-1-dev/<Page-Name>.md`.

**Use this for:** Pin assignments (cross-reference against ericlewis/sp1-midi's DTS), bootloader app-integration requirements, **album header layout, sector trailer layout, sample byte assignments**, general hardware overview.

**Does NOT have:** the USB upload protocol byte-level framing for the `0x39` packet header + ACK. That's still only known to ericlewis (and presumably the offline solderless.engineering JS).

### `github.com/ericlewis/sp1-midi`
**Owner:** ericlewis
**Contents:** Public Zephyr BSP. Full board definition, drivers, app skeleton. **The only buildable public custom firmware.** Implements USB MIDI 2.0 controller (not a stem player).

**Use this for:** Board pin definitions in DTS, codec driver register addresses, build environment configuration, BSP code patterns.

### `github.com/ericlewis/libpo32`
**Owner:** ericlewis
**Contents:** C99 library for the PO-32 (a **different** TE product). Not SP-1 related.

**Use this for:** Reference of ericlewis's code style. **Don't conflate PO-32 with SP-1.**

### `github.com/resinbeard/beaglebone-black-glitcher`
**Contents:** murray's BeagleBone PRU code for nRF52840 APPROTECT voltage glitch attack.

**Use this for:** Reproducing the firmware extraction (research-only).

### `github.com/dot-Justin/TE-SP-1-lines-thread-archive`
**Owner:** dotjustin (the skill's author)
**Contents:** Public mirror of the Lines forum thread. Posts in markdown, images, attachments, threaded replies. Includes an `agent/` directory with summaries and indexes designed for AI use.

**Public web frontend:** `https://sp-1.dotjust.in`

**Use this for:** When a user references a specific Lines post by number (e.g., "what did Galapagoose say in post 68?"), the **bundled subset is the first stop** (`assets/lines-thread-archive/agent/posts/068.md` — see the "Bundled assets" section above). For binary attachments not in the bundled subset, fetch from this repo or `sp-1.dotjust.in/posts/68`.

---

## Local corpus (if the user has it)

If the user is running this skill from their own SP-1 project directory, they may have the following at `~/Projects/sp-1/cc-skill/resources/`. **Don't assume these paths exist** — only use them if the user mentions the corpus or if the files are findable.

### `TE-SP-1-lines-thread-archive/`

Local clone of the Lines archive (same content as `github.com/dot-Justin/TE-SP-1-lines-thread-archive`). **A bundled subset is now shipped inside the skill at `assets/lines-thread-archive/`** — see the "Bundled assets" section above. Use the bundled subset by default; consult the local full clone only when you need the 90 MB of binary attachments (`raw/assets/uploads/`) or the raw Discourse API JSON (`raw/api/`).

**Key sub-paths if the user has the full local clone:**
- `agent/AGENT-GUIDE.md` — instructions for navigating the archive
- `agent/summaries/thread-summary.md` — narrative summary
- `agent/summaries/chunk-001-100.md` through `chunk-801-846.md` — per-chunk summaries
- `agent/indexes/post-index.json` — searchable post metadata
- `agent/indexes/topic-index.json` — posts grouped by topic
- `agent/indexes/attachment-index.json` — uploads with original filenames
- `agent/indexes/reply-chain-index.json` — reply threading
- `agent/posts/NNN.md` — individual posts (846 total)
- `raw/api/posts/NNN.json` — raw Discourse API exports
- `raw/assets/uploads/` — 813 binary attachments (images, PDFs, docs)
- `raw/metadata/upload_manifest.json` — token → local-path mapping

### `assets/sp1-midi-2026-05-13/` (local copy of `ericlewis/sp1-midi`)

Same as the GitHub repo. Reading local files is faster than fetching.

### `SP-1-dev/` (local copy of `timknapen/SP-1-dev`)

Same as the GitHub repo.

### `libpo32/` (local copy of `ericlewis/libpo32`)

Same as the GitHub repo. Not directly relevant to SP-1 work.

### `audiothingies-2026-05-09/` and `storagethingies-2026-05-09/`

**Now bundled in this skill** — see the "Bundled assets" section above for full description. Originally Discord file attachments from ericlewis on 2026-05-09; bundled in `assets/` so citations like `assets/audiothingies-2026-05-09/PcmPacking.hpp` resolve to real files.

These contain the authoritative answers to byte-layout, sector-layout, frame-decode, and effect-implementation questions. The synthesized reference files (`08-emmc-storage.md`, `09-audio-format-spec.md`, `11-block-interleaving-tape-fx.md`, `12-audio-engine-internals.md`, `13-dsp-effects.md`) all cite them as `[code: assets/audiothingies-2026-05-09/...]` or `[code: assets/storagethingies-2026-05-09/...]`.

### Discord scrape

The user may have a local archive of the SP-1 Discord at `~/Projects/sp-1/cc-skill/resources/TE-SP-1-Dev-discord-archive/`. **Do not link to this from the skill** — it's a private scrape. The synthesized references in this skill already incorporate Discord content; Claude should cite by username + date, not by file path.

If the user references something said in Discord that isn't in this skill's references, the user can provide the relevant excerpt and Claude can incorporate it.

---

## Web fetches (anyone can do)

Some authoritative resources live at URLs:

### `https://solderless.engineering/`

Browser-based firmware flasher. **As of 2026-05-09: offline for an update** [Discord #news, tkt1000]. Claude should not assume this is online when answering — verify with the user or note the synthesis-date status.

### `https://solderless-engineering.pages.dev/`

Cloudflare Pages mirror of the above. Same status.

### `https://sp-1.dotjust.in`

Public Lines thread archive frontend. Can be browsed or its agent-friendly JSON indexes can be hit programmatically.

### Nordic nRF52840 documentation

Nordic's product pages have the official datasheet, the SVD file, and the product specification. Public.

### Cirrus CS42L42 datasheet

Available from Cirrus Logic's website. Public.

### TI TAS2505 datasheet

Available from TI's website. Public.

### TI BQ24232 datasheet

Available from TI's website. Public.

### Infineon CYBT-353027-02 datasheet

Available from Infineon's website. Public.

---

## How Claude should use this file

1. **Default to the synthesized references first.** They're curated for accuracy.
2. **When a reference points back to a primary source** (e.g., "see `assets/audiothingies-2026-05-09/PcmPacking.hpp`"), this file tells Claude where that source lives.
3. **When a user asks for "the original post" or "the original code,"** look here for the canonical location.
4. **When in doubt, ask the user** which sources they have access to (bundled-only? plus the GitHub repos? plus the local corpus?). The answer determines what Claude can offer.
5. **Citations in the skill are by handle + date + source channel** (e.g., `[Discord #firmware, ericlewis, 2026-05-09]`). These citations should let the user trace back to the primary source through this file.

---

## What's NOT here

A few categories deliberately omitted:

- **Stock TE firmware binary** — community-private; not redistributed.
- **Kanye album stems** — copyrighted; not redistributed.
- **The Discord invite link** — kept off the skill so casual users can't find the server without an introduction.
- **Specific community members' personal info** — usernames are public; personal details aren't.
- **Anything that would let an attacker compromise unsuspecting users** — there isn't really anything in that category for SP-1, but be cautious if you're tempted to add tooling around the device's hardware.

---

## Where to go next

- For the synthesized reference index → `SKILL.md` (this skill's entry point)
- For the accuracy anchors → `corrections.md`, `known-unknowns.md`, `working-confirmed.md`, `hallucination-watchlist.md`
- For up-to-date community info beyond the synthesis date → ask the user to summarize what's current in the Discord
