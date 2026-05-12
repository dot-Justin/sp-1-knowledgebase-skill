# Tools and Utilities

**Synthesized through:** Lines #846 (2026-05-06), Discord through 2026-05-11.

The community has produced a handful of tools, libraries, and utilities for SP-1 work. This file is the directory: what exists, what it does, where to find it, and whether it's public.

For the broader custom-firmware ecosystem see `20-custom-firmware-state.md`. For who maintains each tool see `26-community-and-authority.md`.

## Tool directory

### Public repositories

#### `github.com/timknapen/SP-1-dev`

**Owner:** TimK (timknapen)
**Status:** Public, active.
**Contents:**
- `README.md` — introduction and disclaimer
- `src/stemplayer_pins.h` — canonical pin definitions (C header)
- `img/` — device illustrations
- `LICENSE`
- **Wiki** (hosted on GitHub, not local files) — the canonical technical documentation including the **Bootloader** page (`/wiki/Bootloader`).

**Use this for:** Authoritative pin map, bootloader protocol documentation, hardware overview. Pin header is C-style and works for nRF-SDK as well as Zephyr.

#### `github.com/ericlewis/sp1-midi`

**Owner:** ericlewis
**Status:** Public, posted 2026-05-09.
**Contents:** Full Zephyr BSP for the SP-1 (board definition, drivers, app skeleton). Implements a USB MIDI 2.0 controller — **not** a stem player.

**Use this for:** A buildable starting point for custom firmware. Forks of this repo are the practical path to writing your own firmware. See `19-sp1-midi-bsp.md`.

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
- Images and attachments
- Reply threading
- Search functionality
- Agent-friendly directory with summaries, post indexes, and AI instructions

**Use this for:** Reading the original thread without needing a Lines account. Cite specific posts by number (e.g., `Lines #556`). The agent-friendly summaries make targeted searches efficient.

**Web frontend:** `https://sp-1.dotjust.in`

### Web-based tools

#### `solderless.engineering` / `solderless-engineering.pages.dev`

**Maintainer:** TimK + loksi + tunelight + keebstudios
**Status:** Offline for an update as of 2026-05-09 [Discord #news, tkt1000].
**What it does:** Browser-based firmware flasher for the SP-1. Uses Web Serial in Chromium-family browsers to talk the bootloader protocol.

**Use this for:** Easy firmware flashing without any local tooling. Requires holding Track 1 + Track 4 while plugging in USB-C.

**When offline:** Use `theunflappable`'s Python tool or write your own client against the bootloader protocol.

### Private / shared-in-Discord tools

#### `audiothingies.zip`

**Author:** ericlewis
**Status:** Shared as Discord attachment in #firmware on 2026-05-09 00:18:55 UTC.
**Contents:** C++17 header-only reference implementation of the stock audio engine: AudioEngine, StockRuntimeMixer, VarispeedResampler, StemEffectRack, 5 effect nodes, PcmPacking, Zephyr I²S TX backend.

**Use this for:** Understanding stock TE audio behavior at the code level. Reimplementation of the audio engine for custom firmware. Not buildable as-is (requires storage layer and integration).

#### `storagethingies.zip`

**Author:** ericlewis
**Status:** Shared as Discord attachment in #firmware on 2026-05-09 00:21:46 UTC.
**Contents:** C++17 reference implementation of the eMMC driver + DiskManager. Includes `decode_te_frame_payload_i32` — the authoritative audio frame byte-layout decoder.

**Use this for:** Understanding eMMC bring-up and the sector / block layout. Pair with `audiothingies` for a complete audio playback system. The frame decoder is the single source of truth for the on-disk PCM format.

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

- **Public album image construction tool** — given 4 WAV stems, output a correctly formatted `.sp1` album image with sync words and proper interleaving
- **Public album image extractor** — given an `.sp1` file, output 4 WAV stems for inspection
- **Public fast USB uploader** — would close the gap from 4.5 days down to ~minutes
- **Public solderless replacement** — a standalone (non-web) firmware flasher with album-upload support, for use when solderless.engineering is offline
- **Public reference for the album header byte layout** — to confirm encoders produce headers that stock firmware accepts

These are good targets for community contributors. None exist as of synthesis date (2026-05-11).

## Where to go next

- For who builds what → `26-community-and-authority.md`
- For each tool's specific role in a workflow → topic-specific reference files
- For known unknowns that would inform new tool development → `known-unknowns.md`
