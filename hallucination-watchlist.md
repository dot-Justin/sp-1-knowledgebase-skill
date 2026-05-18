# Hallucination Watchlist

Past AI-generated SP-1 content has produced specific errors that were publicly corrected by **tkt1000 (TimK)**, the lead firmware engineer and de facto technical authority on the device. This file enumerates those errors so Claude does not repeat them, and lists general patterns that produce SP-1 hallucinations.

If a claim Claude is about to make matches anything below, **stop and verify against primary sources** (code or named-author posts).

---

## Explicit past errors (publicly called out by TimK)

These are word-for-word from Discord #general, 2026-05-10 22:51–22:57 UTC, in response to an AI-generated summary of the Lines archive.

### Error 1 — "24-bit PCM little endian"

**The wrong claim** (paraphrase): "The SP-1 stores audio as 24-bit PCM little-endian samples."

**TimK's correction:** *"24 bit PCM little endian … that doesn't make sense."* [Discord #general, tkt1000, 2026-05-10]

**Why this is wrong:** There are two distinct representations, neither of which is "24-bit little-endian PCM":

- **In-memory:** right-aligned 24-bit signed values in `int32_t`, max magnitude ±8,388,607 (= 2^23 − 1). Function: `float_to_pcm_right24_fast`. The 32-bit container's bytes land in nRF52840 native order (little-endian) but that's an architecture property of the int32, not a property of the audio format. [code: `assets/audiothingies-2026-05-09/PcmPacking.hpp`]
- **On-disk (per stem, 6 bytes):** `L_mid, L_msb, R_msb, L_lsb, R_lsb, R_mid` — left and right channels are interleaved at the byte level, with MSBs adjacent, LSBs adjacent, mid bytes at the outsides. This is not little-endian or big-endian in any standard sense. [code: `assets/storagethingies-2026-05-09/DiskManager.hpp` lines 65–82, `decode_te_frame_payload_i32`]

**Correct claim format:** "Audio samples are 24-bit signed. In memory they sit right-aligned in `int32_t` containers. On disk, each stem stores 6 bytes per frame in the interleaved order documented in `references/09-audio-format-spec.md` — neighboring bytes can belong to different channels."

### Error 2 — "Python and Rust tools in tkt1000's repo"

**The wrong claim** (paraphrase): "TimK's `SP-1-dev` repository contains Python and Rust tools for [some task]."

**TimK's correction:** *"the python and rust tools being in my repo … that doesn't make sense"* [Discord #general, tkt1000, 2026-05-10]

**Why this is wrong:** `github.com/timknapen/SP-1-dev` is a documentation repository. As of synthesis date its contents are: `README.md`, `src/stemplayer_pins.h`, `img/`, `icon.png`, `LICENSE`, plus a GitHub Wiki. **There are no Python or Rust tools in this repo.** (The repo is now bundled in this skill at `assets/SP-1-dev-2026-05-13/` — pin header + wiki pages — so this can be verified directly.)

**If you need a tool for SP-1 work:**
- The actual Zephyr BSP for MIDI-controller mode lives at `github.com/ericlewis/sp1-midi` (different person from TimK; also bundled at `assets/sp1-midi-2026-05-13/`).
- The audio engine and storage code lives in `assets/audiothingies-2026-05-09/` and `assets/storagethingies-2026-05-09/` (bundled in this skill). Originally shared as Discord file attachments by ericlewis 2026-05-09; not in any of ericlewis's public GitHub repos.
- The `solderless.engineering` web updater is a separate project.
- The slow Python USB uploader exists in moecal1947's private code as of 2026-05-09; not yet public.

**Correct behavior:** When mentioning a tool, name the exact repo (and file when known). If you cannot point to it, do not claim it exists.

### Error 3 — "Step 4 in the workflow to prepare stems"

**The wrong claim** (paraphrase): A documented workflow for preparing stems for upload that includes "Step 4: [whatever the AI made up]."

**TimK's correction:** *"step 4 in the workflow to prepare stems is nonsense"* [Discord #general, tkt1000, 2026-05-10]

**Why this is wrong:** As of synthesis date there is **no canonical, documented "prepare stems" workflow** that has been published. People in the community are doing it different ways — HT Demucs separation [Discord #firmware, emvee1968, 2026-05-08], custom format conversion to the eMMC layout, USB upload via the offline solderless utility or moecal1947's slow Python tool. **No "step 4" exists because no canonical numbered workflow has been published.**

**Correct behavior:** Don't fabricate workflow steps. If asked "what's the workflow for getting custom stems on the device?" describe the *components* that need to exist (stem separation, format conversion to eMMC layout, USB upload) and cite who has done each component, rather than presenting a numbered procedure that doesn't exist.

---

## General hallucination patterns to avoid

These are recurring failure modes Claude has either fallen into or is at high risk of falling into when working on SP-1:

### Don't conflate ericlewis with TimK

They are **two different people** who are both active and authoritative.

- **TimK** (Lines), **tkt1000** (Discord), **timknapen** (GitHub) — same person. PCB schematic reversal, eMMC driver implementation, original Zephyr custom firmware (private), bootloader documentation. Owns `github.com/timknapen/SP-1-dev`.
- **ericlewis** (Discord, GitHub) — different person. Published `sp1-midi` BSP (MIDI controller, 2026-05-09), shared `audiothingies`/`storagethingies` (now bundled in this skill at `assets/audiothingies-2026-05-09/` and `assets/storagethingies-2026-05-09/`) as Discord attachments 2026-05-09, wrote `libpo32` for a different TE product, has working tape FF/RW implementation, deep knowledge of effects internals.

Both are active in the Discord. When citing, name the person and the source.

### Don't claim Bluetooth works in the official BSP

**ericlewis/sp1-midi** has Bluetooth UART pins defined in the device tree, but the BSP app is a MIDI controller with no audio playback and no Bluetooth stack. **emvee1968** claims working Bluetooth pairing (via vol+/vol- combo) in unreleased custom firmware [Discord #firmware, 2026-05-08]. There is no publicly available SP-1 firmware with working Bluetooth.

### Don't confuse PO-32 protocol details with SP-1

`libpo32` is for the **Teenage Engineering PO-32 Tonic** — a separate, released product. It uses an acoustic data-transfer protocol over speaker/microphone. **The SP-1 has nothing to do with this.** SP-1 uses USB CDC for data transfer and eMMC for storage. Don't merge concepts.

### Don't quote post numbers without verifying

The Lines thread has 846 posts. Specific post numbers cited in this skill have been verified against the archive. But if Claude is improvising a citation for new content, **do not fabricate a post number**. Either cite by author + approximate date, or open the relevant `agent/posts/NNN.md` (if the corpus is available) and use the verified number.

### Don't assume what's in `sp1-midi`'s app/

The `app/` directory of `ericlewis/sp1-midi` is a **MIDI controller**, not a stem player. It does USB MIDI 2.0 + CDC ACM, reads faders/buttons/encoders, sends CC and MMC messages. It does **not**:
- Play audio
- Apply effects
- Read or write the eMMC stem data (the eMMC driver is there but unused by the app)
- Implement Bluetooth
- Implement original-firmware UX

If asked "how does the SP-1 play audio?", do not look in `assets/sp1-midi-2026-05-13/app/` — look in `assets/audiothingies-2026-05-09/AudioEngine.cpp` and the original firmware analysis.

### Don't claim a feature is "in the firmware" without citing which firmware

There are at least four distinct firmwares relevant to the SP-1:
1. **Original TE firmware** (extracted by murray attempt #8504, 2025-01-25)
2. **TimK's custom firmware** (in development; not public)
3. **ericlewis/sp1-midi** (public, MIDI-controller-only)
4. **emvee1968's custom firmware** (unreleased, basic playback + gate + Bluetooth)
5. **virtualflannel_46386's custom OS** (unreleased, full FX page + mixer)

When discussing a feature, name the firmware.

### Don't claim solderless.engineering is offline (status flipped 2026-05-18)

**As of 2026-05-18 the site is online again** with a multi-app launcher (stem loader, firmware utility, device info, spoom1). Local mirror at `assets/solderless-2026-05-18/`. The site was offline for an update from 2026-05-09 to ~2026-05-17 [Discord #news, tkt1000, 2026-05-09]. If asked about current status, the answer is "online as of 2026-05-18; verify with the user if you need current state past that date."

### Don't fabricate eMMC clock speeds

The chip works reliably at **32 MHz in 1-bit mode** [code: `assets/storagethingies-2026-05-09/EmmcDriver.cpp`]. **High Speed mode is not enabled** and higher clocks are unreliable. TimK ran into this and worked around it. Do not claim higher speeds are possible without citation.

### Don't claim the eMMC is encrypted

It is **not encrypted, just formatted strangely** [Discord #hardware, tkt1000, 2026-05-07]. The audio layout uses 8192-byte logical sectors composed of 16 native 512-byte eMMC blocks, with interleaved frames for tape FF/RW. See `references/08-emmc-storage.md`.

---

## Protocol-byte hallucinations (added 2026-05-13 from solderless archive)

After the solderless source archive was ingested on 2026-05-13, the following speculative claims were ruled out. **Do not assert any of these.**

### Don't say `0x52` is a "bootloader version check"

`0x52` 'R' is a **state query**. The reply `0x53` 'S' returns either a 5-byte state payload or a 4-byte LE32 page counter. The 5-byte payload is **raw integer bytes** (each 0-9), not ASCII characters — bytes `[0, 0, 1, 0, 0]` join into the string `"00100"` (boot mode), bytes `[1, 0, 5, 1, 0]` into `"10510"` (upload mode). Stringify via `Array.from(payload).join('')`, never `String.fromCharCode`. Solderless string-compares this; the bit semantics aren't named in source. There is no "version" anywhere in the response. [Source: solderless archive `js/storage.js` lines 87–125; `gate-fix-testing/src/upload/protocol.mjs::decodeStateReply`.]

### Don't claim state-query bytes are ASCII characters of digits

Earlier wording in this skill called the state-query reply "a 5-byte ASCII string" and the per-character note said "digit ASCII = byte value." That phrasing burned the dumper project on 2026-05-14 — an AI implementation used `String.fromCharCode(b)` (treating each byte as a Unicode code point) and got `    ` instead of `"00100"`, causing the boot-mode check to fail silently against real hardware. **The bytes are not ASCII characters.** They are raw small integers; the digit semantics come from `Number.prototype.toString()` on each byte, joined together. Either match the reference implementations literally (`Array.from(payload).join('')`) or compare the byte array directly (`payload[0]===0 && payload[1]===0 && payload[2]===1 && …`).

### Don't claim firmware-flash requires "flash mode" before `0x46`

The firmware-flash flow does **not** require the device to already be in a particular flash-only mode. Solderless's `firmware.js::flashFirmware` sends `0x52` once, accepts whatever 0x53 reply comes back (4-byte LE32 page counter *or* 5-byte `"00100"` boot mode), and proceeds directly to `0x46` erase. The `0x70`/`0x50` "set upload mode + reboot" dance is only needed for album-upload sessions, NOT for firmware flash. Earlier versions of this skill suggested a more rigid mode-transition flow that didn't match the canonical implementation.

### Don't claim the `0x39` packet has a "4-byte magic / sub-command header"

`0x39` payload is exactly **136 bytes**: `chunk_counter (LE32) | emmc_byte_offset (LE32) | 128 bytes data`. Both header fields are plain little-endian uint32s. No magic, no sub-command byte, no checksum-in-header. [Source: solderless archive `js/storage.js` lines 638–647, identical in `stemloader.js`.]

### Don't claim the second header field is a "sector address"

`emmc_byte_offset` is an **absolute byte offset** in eMMC, not a sector or block address. The host increments it by 128 per chunk. Earlier wording in this skill called it "sector address (32-bit, presumably native 512-byte block addressing)" — that was wrong.

### Don't claim the host sets `GPREGRET = 0x1A96`

The host-side mode switch is plain: `0x70 [1]` ("set mode" with payload byte `0x01`), then `0x50` reboot. The `0x1A96` value was a Discord paraphrase from ericlewis; it doesn't appear in any public host implementation. There may be a magic value inside the firmware that the host can't observe, but the host doesn't need to set or know it.

### Don't claim `0x39` packets are ACK'd one by one

`0x39` is **fire-and-forget at 115200 baud**. The host uses `sendNoReply` for every chunk; no per-chunk ACK or NAK. The only post-upload verification is `0x43` (chunk-counter check) and `0x66` (album validity check). If a chunk is dropped on the wire, the upload silently corrupts. [Source: solderless archive `js/storage.js` line 644 + `stemloader.js` line 834.]

### Don't claim the tempo field is "samples-per-tick" or "Q8.8 BPM"

The encoder writes tempo as `(48000 * 60) / (24 * bpm)`. For 60 BPM = 2000; 80 BPM = 1500; 120 BPM = 1000. Stored as **uint16 LE at bytes 2042..2043** of each sector (end of block 0). The exact firmware-side semantic interpretation isn't documented, but the formula is the canonical encoder-side specification. [Source: solderless archive `js/wav-parser.js::encodeToSP1` lines 96–97, 127–128.]

### Don't claim per-block 8-byte trailers exist in encoder output

The TKT wiki documents a layout of **2 sync + 2 tempo + 4 LED bytes per TE-block × 4 blocks = 32 bytes per sector**. That describes what stock firmware **can read**. The solderless encoder writes only a **per-sector trailer** at the end of block 0 — there are still **no per-block trailers**. The size of that single trailer evolved between encoder versions:

- **2026-05-12 encoder** (`js/wav-parser.js::encodeToSP1`): 6 bytes — `tempo (u16 LE at 2042) + 4× envelope (u8 at 2044)`. **No sync counter, no LED data**, nothing in blocks 1-3's trailer space.
- **2026-05-18 encoder** (`stemloader/js/wav-converter.js::encodeToSP1`): 8 bytes — `clock (u16 LE at 2040) + tempo (u16 LE at 2042) + 4× envelope (u8 at 2044)`. Per-sector clock now written; still no per-block trailers and still no LED data.

When writing a custom encoder, the 2026-05-18 8-byte sector trailer is now the canonical form. Don't fabricate per-block trailers; both layouts above are valid for stock firmware playback. See `references/10-midi-timing-encoding.md` and `corrections.md` 2026-05-18 entry.

### Don't claim there's a separate "sync counter" being written by encoders (NARROWED 2026-05-18)

**The 2026-05-12 encoder did not write a sync counter.** `storage.js::uploadAlbum` and `wav-parser.js::encodeToSP1` wrote zero sync-counter bytes.

**The 2026-05-18 encoder *does* write a sync counter** — once per sector, as `uint16 LE` at sector offset 2040 (end of block 0). It wraps modulo `CLOCK_MAX = 49152` (= 512 bars × 96 ticks/bar; 96 ticks/bar = 4 quarters × 24 PPQN). Empty sectors get sentinel `clock = 0xFFFF` (paired with `tempo = 0x0000`). First tick of a song is hardcoded to `clock = 1`.

**What's still wrong** even after 2026-05-18: claims of a **per-block** sync counter (4 separate counters per sector, one per TE-block). The encoder writes one counter per sector, not four.

**What's still speculative**: the older skill-described pseudo-formula `int(sample_position / (30000 / bpm)) % 24489` predates the encoder source. The 2026-05-18 encoder uses a different accumulator (`clockAcc += CLOCK_INCR + samplesPerTick - elapsedSamples`, modulo 49152) — these may or may not reconcile. Logged as a known-unknown in `known-unknowns.md`.

[Source: `assets/solderless-2026-05-18/stemloader/js/wav-converter.js::encodeToSP1` MIDI clock block — `OFFSET_CLOCK = 2040`, `CLOCK_MAX = 49152`, `CLOCK_INCR = 512`.]

### Don't conflate solderless's iframe/serial-shim with the SP-1's on-device behavior

As of 2026-05-18 the `solderless.engineering` site is built as a parent `index.html` hosting four sandboxed iframes (stem loader, firmware utility, device info, spoom1). The parent owns one `navigator.serial` port. Each iframe loads `js/serial-shim.js`, which intercepts `navigator.serial` calls inside the iframe and proxies them to the parent via `window.postMessage`. Only the **active** iframe can send TX; the parent rejects messages from inactive frames.

**This is a host-side tooling pattern, not anything the SP-1 firmware does.** The SP-1 sees a single CDC ACM stream from the host browser; it has no concept of iframes or postMessage. Don't describe the SP-1 firmware as "supporting multiple apps simultaneously" or "switching contexts between apps" — context-switching happens entirely in the host's browser.

[Source: `assets/solderless-2026-05-18/index.html` lines 130-349; `assets/solderless-2026-05-18/js/serial-shim.js`.]

---

## When in doubt

If a question is going to require Claude to make a factual claim about the SP-1 that is not directly verifiable in this skill, the local corpus, or the public repos, **the correct answer is "I'm not sure — verify with the live Discord or ask TimK / ericlewis."** Hallucinating a specific number, opcode, pin, or behavior is worse than admitting uncertainty.
