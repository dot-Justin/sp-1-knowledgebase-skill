# Tools and Utilities

**Synthesized through:** Lines #846 (2026-05-06), Discord through 2026-05-11.

The community has produced a handful of tools, libraries, and utilities for SP-1 work. This file is the directory: what exists, what it does, where to find it, and whether it's public.

For the broader custom-firmware ecosystem see `20-custom-firmware-state.md`. For who maintains each tool see `26-community-and-authority.md`.

## Tool directory

### Public repositories

#### `github.com/timknapen/SP-1-dev`

**Owner:** TimK (timknapen)
**Status:** Public, active. MIT licensed.
**Contents:**
- `README.md` — introduction and disclaimer
- `src/stemplayer_pins.h` — canonical pin definitions (C header)
- `img/` — device illustrations (642 KB of PNGs — not bundled)
- `LICENSE`
- **Wiki** (hosted on GitHub) — 14 technical pages covering hardware, bootloader, peripherals, I²S/I²C/PWM/SAADC, audio format, album metadata, battery charger, Bluetooth module

**Bundled subset in this skill:** `assets/SP-1-dev-2026-05-13/` contains `LICENSE` + `README.md` + `src/stemplayer_pins.h` + `wiki/*.md` (all 14 wiki pages as markdown, ~24 KB total). `img/` is **not** bundled (graphics-heavy; fetch from the source repo if you need device illustrations).

**Use this for:** Authoritative pin map (the `stemplayer_pins.h` header is THE canonical pin reference), bootloader app-integration spec (the `Bootloader.md` wiki page), hardware overview, sector trailer layout per the wiki.

#### `github.com/ericlewis/sp1-midi`

**Owner:** ericlewis
**Status:** Public, posted 2026-05-09. MIT licensed.
**Contents:** Full Zephyr BSP for the SP-1 (board definition, drivers, app skeleton). Implements a USB MIDI 2.0 controller — **not** a stem player. The only buildable public custom firmware.

**Bundled in this skill:** `assets/sp1-midi-2026-05-13/` — full repo snapshot. CMakeLists, Kconfig, prj.conf, app.overlay, boards/, drivers/, subsys/, app/, dts/, include/ — everything you need to read or fork. See `references/19-sp1-midi-bsp.md` for a walkthrough.

**Use this for:** A buildable starting point for custom firmware. Forks of this repo are the practical path to writing your own firmware. See `references/19-sp1-midi-bsp.md`.

#### `github.com/ericlewis/libpo32`

**Owner:** ericlewis
**Status:** Public.
**Contents:** C99 library for the **Teenage Engineering PO-32 Tonic** (a different product). Implements PO-32's acoustic transfer protocol and drum synthesis.

**Use this for:** Reference of ericlewis's C library style. **Not directly relevant to SP-1 development** — the protocols are different. Don't confuse PO-32 acoustic transfers with SP-1 USB CDC bootloader. See `hallucination-watchlist.md`.

#### `github.com/resinbeard/beaglebone-black-glitcher`

**Owner:** murray (or contributors thereof — see the repo)
**Status:** Public.
**Contents:** BeagleBone Black PRU code for nRF52840 APPROTECT voltage glitch attack.

**Use this for:** Reproducing the firmware extraction. Useful as a general nRF52 glitch tool, not SP-1-specific.

#### `github.com/dot-Justin/TE-SP-1-lines-thread-archive`

**Owner:** dotjustin
**Status:** Public.
**Contents:** Mirror of the Lines forum thread. Includes:
- Original posts (markdown)
- Images and attachments (90 MB of binary uploads — images, PDFs, docs)
- Reply threading
- Search functionality
- Agent-friendly directory (`agent/`) with summaries, post indexes, and AI instructions

**Use this for:** Reading the original thread without needing a Lines account. Cite specific posts by number (e.g., `Lines #556`). The agent-friendly summaries make targeted searches efficient.

**Web frontend:** `https://sp-1.dotjust.in`

**Bundled subset in this skill:** `assets/lines-thread-archive/` contains everything in `agent/` plus thread-level metadata (~1.6 MB total). Binary attachments are NOT bundled (90 MB total); use the source repo or web frontend for those.

### Web-based tools

#### `solderless.engineering` / `solderless-engineering.pages.dev`

**Maintainer:** TimK + loksi + tunelight + keebstudios
**Status:** Offline for an update as of 2026-05-09 [Discord #news, tkt1000]. **A 2026-05-12 source snapshot is archived locally** — see "Local archive" below.
**What it does:** Browser-based firmware flasher + album uploader for the SP-1. Uses Web Serial in Chromium-family browsers to talk the bootloader protocol. The main page also embeds a "TKT stem loader" subpage (TimK's earlier standalone uploader UX) — both implement the same protocol.

**Use this for:** Easy firmware flashing without any local tooling. Easy album upload. Requires holding Track 1 + Track 4 while plugging in USB-C.

**When offline:** Run the local archive (see below); or use `theunflappable`'s Python tool; or write your own client against the bootloader protocol.

#### **Bundled in skill** — `assets/solderless-2026-05-12/` and `assets/solderless-2026-05-12.zip`

**Location:** Inside this skill's `assets/` directory (bundled with the repo). Both the extracted flat tree and the original zip are present.
**Provenance:** Obtained as a community backup on 2026-05-13. Contents dated 2026-05-12 (the README) and 2026-05-09 (the deployed JS/HTML — last upstream `pages.dev` edit before the announced "offline for an update" period).
**Contents:** Complete static dump of `solderless-engineering.pages.dev` — `index.html`, `stemloader.html`, the four JS modules (`protocol.js`, `firmware.js`, `wav-parser.js`, `storage.js`), the TKT stemloader's own copies, plus CSS / fonts / SVGs.
**How to run:**

```sh
cd assets/solderless-2026-05-12
python3 -m http.server 8788
# Then open http://127.0.0.1:8788/ in Chrome or Edge (Safari does not support Web Serial)
```

**Use this for:** Running solderless locally when the public site is down. Reading the JS as the canonical SP-1 protocol reference. The skill cites this archive throughout `references/15-bootloader-protocol.md`, `references/16-usb-upload-protocol.md`, `references/21-original-firmware-stems.md`, and elsewhere.

**Caveats:** Archive is from before the announced "offline for update." When the live site returns, behavior may differ; re-archive if needed.

### Bundled-in-skill source archives (originally Discord attachments)

#### `assets/audiothingies-2026-05-09/` (and `.zip`)

**Author:** ericlewis
**Origin:** Shared as Discord attachment in #firmware on 2026-05-09 00:18:55 UTC.
**Status:** Bundled in this skill repo. Flat extracted tree + original zip both present.
**Contents:** C++17 header-only reference implementation of the stock audio engine: AudioEngine, StockRuntimeMixer, VarispeedResampler, StemEffectRack, 5 effect nodes (BiquadFilter, ChorusFlangerNode, DelayEchoNode, DistortionNode, GateEnvelopeNode), DspTables, DspUtils, PcmPacking, AudioNode, plus a Zephyr I²S TX backend.

**Use this for:** Understanding stock TE audio behavior at the code level. Reimplementation of the audio engine for custom firmware. Not buildable as-is (requires storage layer and Zephyr integration).

**Cite as:** `[Source: assets/audiothingies-2026-05-09/AudioEngine.cpp lines X-Y]` or similar.

#### `assets/storagethingies-2026-05-09/` (and `.zip`)

**Author:** ericlewis
**Origin:** Shared as Discord attachment in #firmware on 2026-05-09 00:21:46 UTC.
**Status:** Bundled in this skill repo. Flat extracted tree + original zip both present.
**Contents:** C++17 reference implementation of the eMMC driver + DiskManager: `DiskManager.{hpp,cpp}`, `EmmcDriver.{hpp,cpp}`, `StemEmmcDevice.{hpp,cpp}`, `EMMC.hpp`, `DiskFormat.hpp`. Includes `decode_te_frame_payload_i32` in `DiskManager.hpp` — the authoritative audio frame byte-layout decoder.

**Use this for:** Understanding eMMC bring-up (CMD0/CMD1/CMD2/CMD3/CMD7/CMD8/CMD9/CMD16 init sequence) and the sector / block layout. Pair with `audiothingies` for a complete audio playback system. The frame decoder is the single source of truth for the on-disk PCM format.

**Cite as:** `[Source: assets/storagethingies-2026-05-09/DiskManager.hpp lines X-Y]` or similar.

### Private / shared-in-Discord tools (not bundled)

#### `test_bootloader.py`

**Author:** theunflappable
**Status:** Shared as Discord attachment in #firmware on 2026-05-08 00:49:57 UTC.
**Contents:** Python implementation of the SP-1 bootloader CDC protocol. Implements the basic flash workflow.

**Use this for:** A standalone Python tool to flash custom firmware when solderless.engineering is offline. Does not include album upload (only firmware flash).

#### moecal1947's USB album uploader

**Author:** moecal1947
**Status:** Privately developed; not yet a release.
**Contents:** Python script that pushes album images to the SP-1 over USB. Works but very slow (~0.75 KB/s, 4.5 days per 311 MB album). Safe + resumable + per-block checksum verified.

**Use this for:** Actually uploading custom albums today. Patience required. Ask moecal1947 in Discord for current state and code access.

#### zee_33's LED-bit-bang dump firmware

**Author:** zee_33
**Status:** Privately developed; not yet a release.
**Contents:** Minimal (~1 page = 4 KB) custom firmware that reads flash and modulates LEDs to encode bytes for camera-based extraction.

**Use this for:** Dumping the bootloader region without a glitch attack or device opening. Ask zee_33 in Discord for code access if attempting to reproduce.

### External tools the community recommends

#### **HT Demucs FT** (audio stem separation)

What emvee1968 uses for separating songs into 4 stems [Discord #firmware, 2026-05-08 23:54]. Open-source. High-quality 4-stem separation suitable for the SP-1's 4-stem format.

Find via: Hugging Face, GitHub repos with PyTorch Demucs implementations.

#### **Ghidra** (firmware reverse engineering)

NSA-published RE framework. Standard tool for nRF52840 disassembly. See `22-disassembly-ghidra.md`.

#### **nRF52840 SVD file** (register definitions for Ghidra overlay)

Downloadable from Nordic's website. Standard companion for nRF52840 disassembly.

#### **SVD-Loader-Ghidra** (plugin)

Ghidra plugin that imports SVD register definitions and shows named symbols instead of raw addresses in the decompilation.

#### **Zephyr RTOS** (firmware framework)

Required for building `sp1-midi`. See `18-zephyr-build-environment.md`.

#### **`west`** (Zephyr project management)

Zephyr's tool for fetching dependencies and orchestrating builds. Recommended over invoking cmake directly.

#### **`nrfjprog`** (Nordic SWD flasher)

Used by `ninja -C build flash` to push binaries via SWD. From Nordic Command Line Tools.

#### **iPhone with slow-motion video** (for LED bit-bang capture)

240 fps slow-motion is sufficient for decoding LED-encoded bytes. Phone-based capture is fine; no specialty hardware needed.

#### **Unpopulated USB-to-eMMC adapter PCBs**

For fishdog_'s desolder method. Sold cheaply on AliExpress and (when in stock) Amazon. Search for "eMMC USB adapter" or "eMMC to USB unpopulated PCB."

#### **Hot air rework station + preheater**

For desoldering the eMMC chip. fishdog_'s configuration: 380–420 °C hot air, 175 °C preheater.

#### **Hairdryer**

For removing the white plastic piece on the SP-1's face. Low enough heat that plastic isn't damaged. See `23-physical-disassembly.md`.

## What's missing

A handful of tools that would be valuable but don't exist yet:

- **Public CLI / library port of `encodeToSP1`** — a non-web, non-JavaScript tool that takes 4 WAV stems + album metadata and emits a `.sp1` album image. The encoder is fully documented in `references/21-original-firmware-stems.md`; what's missing is a published CLI/Python/Rust port.
- **Public album image extractor** — given an `.sp1` file, output 4 WAV stems for inspection.
- **Public fast USB uploader** — would close the gap from 9 hours (solderless's ~10 KB/s) or 4.5 days (moecal's tool) down to ~minutes. Requires CMD25 multi-block eMMC write support and USB bulk endpoints on the firmware side.
- **Public solderless replacement (non-web)** — a standalone CLI firmware flasher + album uploader, for use when solderless.engineering is offline and the user doesn't want to run a local web server. The protocol is fully documented; the port has not been written.

These are good targets for community contributors. The protocol byte spec is no longer a blocker — see `update-log.md` 2026-05-13.

## Where to go next

- For who builds what → `26-community-and-authority.md`
- For each tool's specific role in a workflow → topic-specific reference files
- For known unknowns that would inform new tool development → `known-unknowns.md`
