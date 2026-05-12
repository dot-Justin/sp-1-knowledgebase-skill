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

- **In-memory:** right-aligned 24-bit signed values in `int32_t`, max magnitude ±8,388,607 (= 2^23 − 1). Function: `float_to_pcm_right24_fast`. The 32-bit container's bytes land in nRF52840 native order (little-endian) but that's an architecture property of the int32, not a property of the audio format. [code: `audiothingies/PcmPacking.hpp`]
- **On-disk (per stem, 6 bytes):** `L_mid, L_msb, R_msb, L_lsb, R_lsb, R_mid` — left and right channels are interleaved at the byte level, with MSBs adjacent, LSBs adjacent, mid bytes at the outsides. This is not little-endian or big-endian in any standard sense. [code: `storagethingies/DiskManager.hpp` lines 65–82, `decode_te_frame_payload_i32`]

**Correct claim format:** "Audio samples are 24-bit signed. In memory they sit right-aligned in `int32_t` containers. On disk, each stem stores 6 bytes per frame in the interleaved order documented in `references/09-audio-format-spec.md` — neighboring bytes can belong to different channels."

### Error 2 — "Python and Rust tools in tkt1000's repo"

**The wrong claim** (paraphrase): "TimK's `SP-1-dev` repository contains Python and Rust tools for [some task]."

**TimK's correction:** *"the python and rust tools being in my repo … that doesn't make sense"* [Discord #general, tkt1000, 2026-05-10]

**Why this is wrong:** `github.com/timknapen/SP-1-dev` is a documentation repository. As of synthesis date its contents are: `README.md`, `src/stemplayer_pins.h`, `img/`, `icon.png`, `LICENSE`, plus a GitHub Wiki. **There are no Python or Rust tools in this repo.**

**If you need a tool for SP-1 work:**
- The actual Zephyr BSP for MIDI-controller mode lives at `github.com/ericlewis/sp1-midi` (different person from TimK).
- The audio engine and storage code lives in `audiothingies.zip` and `storagethingies.zip`, shared privately on Discord 2026-05-09 by ericlewis. **Not currently in any public repo.**
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
- **ericlewis** (Discord, GitHub) — different person. Published `sp1-midi` BSP (MIDI controller, 2026-05-09), shared `audiothingies.zip`/`storagethingies.zip` privately in Discord (2026-05-09), wrote `libpo32` for a different TE product, has working tape FF/RW implementation, deep knowledge of effects internals.

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

If asked "how does the SP-1 play audio?", do not look in `sp1-midi/app/` — look in `audiothingies/AudioEngine.cpp` and the original firmware analysis.

### Don't claim a feature is "in the firmware" without citing which firmware

There are at least four distinct firmwares relevant to the SP-1:
1. **Original TE firmware** (extracted by murray attempt #8504, 2025-01-25)
2. **TimK's custom firmware** (in development; not public)
3. **ericlewis/sp1-midi** (public, MIDI-controller-only)
4. **emvee1968's custom firmware** (unreleased, basic playback + gate + Bluetooth)
5. **virtualflannel_46386's custom OS** (unreleased, full FX page + mixer)

When discussing a feature, name the firmware.

### Don't claim solderless.engineering is online

As of synthesis date (2026-05-09) it is **offline** for an update [Discord #news, tkt1000, 2026-05-09]. If Claude is asked about current status, say "offline as of 2026-05-09 per tkt1000's notice; verify current state by asking the user or visiting the URL." Don't claim it's working.

### Don't fabricate eMMC clock speeds

The chip works reliably at **32 MHz in 1-bit mode** [code: `storagethingies/EmmcDriver.cpp`]. **High Speed mode is not enabled** and higher clocks are unreliable. TimK ran into this and worked around it. Do not claim higher speeds are possible without citation.

### Don't claim the eMMC is encrypted

It is **not encrypted, just formatted strangely** [Discord #hardware, tkt1000, 2026-05-07]. The audio layout uses 8192-byte logical sectors composed of 16 native 512-byte eMMC blocks, with interleaved frames for tape FF/RW. See `references/08-emmc-storage.md`.

---

## When in doubt

If a question is going to require Claude to make a factual claim about the SP-1 that is not directly verifiable in this skill, the local corpus, or the public repos, **the correct answer is "I'm not sure — verify with the live Discord or ask TimK / ericlewis."** Hallucinating a specific number, opcode, pin, or behavior is worse than admitting uncertainty.
