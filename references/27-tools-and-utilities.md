# Tools and Utilities

**Synthesized through:** Lines #846 (2026-05-06), Discord through 2026-05-11, and the **solderless 2026-05-18 re-snapshot** (multi-app launcher rewrite).

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
**Status:** **Online as of 2026-05-18** as a multi-app launcher (rewrite ships 4 apps in sandboxed iframes). Was offline 2026-05-09 to ~2026-05-17 for the rewrite [Discord #news, tkt1000]. Both a pre-rewrite snapshot and the current rewrite are bundled locally — see "Bundled in skill" below.
**What it does:** Multi-app launcher hosting four sandboxed-iframe apps that all share one Web Serial port via a postMessage proxy:

1. **stem loader** (evolved from the original single-page upload tool): drag-reorder via Sortable.js, per-song state badges (missing/todo/done/new), in-app Markdown help via `marked.umd.js`, BPM extracted from filename, `localStorage` album persistence, accepts 1-N channel WAV.
2. **firmware utility** (new in 2026-05-18): browser-based custom-firmware flasher. Workflow: T1+T4-boot the device, drag .bin, flash. Two-phase H[lastPage]→H[counter] commit per `references/15-bootloader-protocol.md`. 892 KB max payload (the app slot).
3. **device info** (new in 2026-05-18): live readout at 10 Hz polling of every queryable runtime field (faders, buttons, ladders, charge state, temperature, mode, device ID, album title, write counter). SVG mirror of the device updates in real time. Excellent reference for what each transfer-mode opcode returns and a useful demo that WebSerial round-trips at ~10 Hz are smooth.
4. **spoom1** (new in 2026-05-18): Doom in the browser, controlled by the SP-1 as a USB-HID-like controller. Polls SP-1 at ~16 Hz. Input mapping: T1=walk forward/back, T2=weapon select (7 bands), T3=strafe, T4=turn; btn 2 (rewind)=FIRE, btn 6 (T2)=USE, btn 7 (T1)=RUN, btn 8 (play)=ESC/menu, btn 9 (function)=ENTER/confirm. Useful empirical proof that the SP-1 can drive interactive software at usable polling rates over Web Serial.

**Host-side architecture (2026-05-18):** the parent `index.html` owns a single `navigator.serial` port (Web Serial requires a user gesture per page). Each app loads inside its own `<iframe>` and uses `js/serial-shim.js`, a postMessage-backed `navigator.serial` proxy. The shim makes `navigator.serial.requestPort()`, `port.open()`, `reader.read()`, and `writer.write()` work transparently inside iframes; the parent routes serial RX to the *active* iframe only and rejects TX from inactive iframes. Disconnect events propagate to all iframes. 60-second connection timeout in the shim. This is a **reusable pattern for any future SP-1 host tool** that wants multi-app composition over a single Web Serial port. [Source: `assets/solderless-2026-05-18/index.html` lines 130-349; `assets/solderless-2026-05-18/js/serial-shim.js`.] Note: this is a **host-side** pattern; the SP-1 firmware sees a single CDC ACM stream and has no concept of iframes — see `hallucination-watchlist.md`.

**First official user FAQ:** Tim Knapen authored `stemloader/help/help.md`, the first canonical user documentation for stem upload. Quotable items in `references/16-usb-upload-protocol.md` and `references/21-original-firmware-stems.md`.

**Use this for:** Easy firmware flashing, easy album upload, live device debugging, and (apparently) playing Doom with your SP-1. Requires holding Track 1 + Track 4 while plugging in USB-C.

**When offline:** Run a local archive (see below); or use `theunflappable`'s Python tool; or write your own client against the bootloader protocol.

#### **Bundled in skill** — two solderless snapshots

Two bundles are kept locally for separate reasons:

**`assets/solderless-2026-05-12/` and `assets/solderless-2026-05-12.zip`** (earlier static snapshot, preserved for citation stability)
- **Provenance:** Obtained as a community backup on 2026-05-13. Contents dated 2026-05-12 (the README) and 2026-05-09 (the deployed JS/HTML — last upstream `pages.dev` edit before the announced "offline for an update" period).
- **Contents:** Complete static dump of the single-page-tool deployment — `index.html`, `stemloader.html`, the four JS modules (`protocol.js`, `firmware.js`, `wav-parser.js`, `storage.js`), the TKT stemloader's own copies, plus CSS / fonts / SVGs.
- **Use:** **For new claims, prefer the 2026-05-18 bundle below.** This snapshot is kept so that earlier references citing specific line numbers in `js/storage.js`, `js/firmware.js`, `js/wav-parser.js`, etc. continue to resolve. The byte-level protocol is unchanged between the two snapshots; only the file layout, the file names, and the encoder's sector-trailer size differ. See `references/15-bootloader-protocol.md`, `references/16-usb-upload-protocol.md`, `references/21-original-firmware-stems.md`, and `references/10-midi-timing-encoding.md` for the cross-references.

**`assets/solderless-2026-05-18/` and `assets/solderless-2026-05-18.zip`** (current canonical mirror)
- **Provenance:** Fresh `wget --mirror` scrape of the live `solderless.engineering` on 2026-05-18. 44 files including a skill-added README; ~601 KB extracted.
- **Contents:** Complete static dump of the multi-app launcher: parent `index.html` + `js/serial-shim.js` + per-app subdirectories (`stemloader/`, `utility/`, `deviceinfo/`, `doom/`), plus CSS / fonts / SVGs. The `stemloader/help/help.md` (Tim Knapen's official user FAQ) is included.
- **Use:** **The canonical reference for new claims** about the SP-1 host protocol, the encoder, the firmware-flash flow, and the live-query protocol. The `utility/js/firmware.js` is the cleanest reference for the firmware-flash flow (the 2026-05-12 version mixed flash + album upload in one file). The `stemloader/js/wav-converter.js::encodeToSP1` is the canonical encoder, including the new 8-byte sector trailer (clock + tempo + envelopes).
- **Caveats:** The doom app references `./wasm/doom.wasm` which was not present at scrape time — the page likely fetches it on demand from a CDN. The doom app's input-mapping logic is fully captured in `doom/js/doom.js` even without the WASM binary.

**How to run either bundle:**

```sh
cd assets/solderless-2026-05-18   # or solderless-2026-05-12
python3 -m http.server 8788
# Then open http://127.0.0.1:8788/ in Chrome or Edge (Safari does not support Web Serial)
```

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
