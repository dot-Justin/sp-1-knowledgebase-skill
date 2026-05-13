---
name: sp-1
description: This skill should be used when the user asks about the Teenage Engineering SP-1 stem player (or "Stem Player SP-01"), the nRF52840-based Kanye/TE collaboration device, custom firmware development for it, custom stems / album encoding, the bootloader trigger ("Track 1 + Track 4"), `solderless.engineering`, the `ericlewis/sp1-midi` Zephyr BSP, the `audiothingies` / `storagethingies` reference code, APPROTECT glitch attacks on the SP-1, eMMC extraction via test points or desolder, the 0/2/1/3 block interleaving for tape FF/RW, MIDI clock encoding in the audio data, the CS42L42 / TAS2505 / BQ24232 / CYBT-353027-02 / THGBMNG5D1LBAIL chips wired in the SP-1, or any technical question about the device's hardware, firmware, format, or reverse-engineering history.
---

# SP-1 Stem Player Reference Library

This skill provides curated reference material about the **Teenage Engineering SP-1 stem player** — an unreleased Kanye West / TE prototype based on the nRF52840 SoC. The community has reverse-engineered it over 3+ years (2024-04 to 2026-05+). This skill synthesizes that work into accurate, cited reference files so Claude can answer technical questions without making things up.

**The single most important rule for this skill:** the SP-1 community has publicly called out AI hallucinations about this device. Before asserting any technical claim, read `hallucination-watchlist.md` and verify against the cited source.

## Quick facts

- **What it is:** A 4-stem audio player. nRF52840 SoC. 4 GB Toshiba eMMC. Cirrus CS42L42 headphone codec + TI TAS2505 speaker amp. Cypress CYBT-353027-02 Bluetooth module. TI BQ24232 charger. 8 LEDs, 4 faders, 4 track buttons, transport rocker, function button, USB-C.
- **What it does (stock):** Plays 4 stereo stems at 24-bit / 48 kHz with effects (filter, distortion, gate, chorus/delay/echo) and tape FF/RW. Original units shipped with Donda or Jesus Is King album stems.
- **Why it's hackable:** The community bypassed APPROTECT in Jan 2025 (murray, voltage glitch). Track 1 + Track 4 + USB-C triggers a 60-second bootloader. `solderless.engineering` makes flashing easy. Custom stems work on stock firmware if formatted correctly.
- **Current state (synthesis date 2026-05-11):** Lines forum thread closed (copyright/spam); community moved to Discord. `solderless.engineering` offline for an update. Multiple custom firmwares in development (emvee1968, virtualflannel_46386, TimK). Only public buildable BSP: `ericlewis/sp1-midi` (MIDI controller, not stem player).

## Navigation — which file for which question

**Always read first when working on this skill:**

- `synthesis-log.md` — what's covered through what date; freshness rules
- `update-log.md` — every change since 2026-05-13, with what/why/source per intake batch
- `hallucination-watchlist.md` — the 3 explicit tkt1000 callouts + protocol-byte anti-patterns from 2026-05-13
- `corrections.md` — documented reversals (what was once believed wrong)
- `known-unknowns.md` — what's explicitly unresolved as of synthesis date
- `working-confirmed.md` — what is confirmed working with citations

**Then for specific topics:**

| Topic / question | File |
| --- | --- |
| Overview, history, who's who | `references/01-overview-and-history.md` |
| Chip-level architecture, peripheral map | `references/02-hardware-overview.md` |
| PCB layout, schematic, FPC, pin map | `references/03-pcb-and-schematic.md` |
| SWD, USB CDC, test points, ways to talk to the device | `references/04-debug-interfaces.md` |
| BQ24232 charger, battery, USB power routing | `references/05-power-and-battery.md` |
| CS42L42 + TAS2505 audio codec drivers, I²C config | `references/06-audio-codecs.md` |
| CYBT-353027-02 Bluetooth module | `references/07-bluetooth-module.md` |
| eMMC chip + protocol + sector layout + prefetch model | `references/08-emmc-storage.md` |
| Audio format on disk (the byte layout) | `references/09-audio-format-spec.md` |
| MIDI clock encoding in side-band data | `references/10-midi-timing-encoding.md` |
| The 0/2/1/3 block interleaving and tape FF/RW | `references/11-block-interleaving-tape-fx.md` |
| Audio engine internals (mixer, transport, varispeed) | `references/12-audio-engine-internals.md` |
| DSP effects (filter, distortion, gate, chorus/delay) | `references/13-dsp-effects.md` |
| APPROTECT bypass / glitch attack | `references/14-approtect-glitch-attack.md` |
| Bootloader CDC protocol | `references/15-bootloader-protocol.md` |
| USB album upload protocol | `references/16-usb-upload-protocol.md` |
| LED bit-bang firmware dump technique | `references/17-led-bitbang-dump.md` |
| Zephyr build environment | `references/18-zephyr-build-environment.md` |
| `ericlewis/sp1-midi` BSP walkthrough | `references/19-sp1-midi-bsp.md` |
| Custom firmwares (public + unreleased) | `references/20-custom-firmware-state.md` |
| Stock-firmware custom stems workflow | `references/21-original-firmware-stems.md` |
| Ghidra disassembly findings | `references/22-disassembly-ghidra.md` |
| Physical disassembly / heat-and-pry | `references/23-physical-disassembly.md` |
| eMMC direct extraction (test points / desolder) | `references/24-emmc-direct-extraction.md` |
| 3D-printed accessories and mods | `references/25-3d-printing-mods.md` |
| Who's who, identity map | `references/26-community-and-authority.md` |
| Tools and utilities | `references/27-tools-and-utilities.md` |

Each topic has a paired FAQ file at the same path with `-faq.md` appended. For quick-lookup questions consult the FAQ first; for deep technical questions go to the main reference.

For **primary sources** (Lines posts, code, GitHub repos, manuals) see `sources.md`.

## Top 15 quick-lookup answers

These are the questions that come up most often. For deeper detail follow the citation to the relevant file.

**Q: How do I enter the bootloader?**
A: Hold **Track 1 + Track 4** and plug in USB-C. The device enumerates as USB CDC ACM for 60 seconds. The power button is NOT part of the combo (despite some old posts saying so). → `references/15-bootloader-protocol.md`.

**Q: Can I play my own audio without custom firmware?**
A: Yes. Stock TE firmware plays any correctly-formatted album. → `references/21-original-firmware-stems.md`.

**Q: What's the eMMC clock speed?**
A: **32 MHz** in 1-bit mode. High Speed mode is not enabled. Don't try to go higher — unreliable. → `references/08-emmc-storage.md`.

**Q: Is the audio 24-bit little-endian PCM?**
A: **No.** That's a recurring hallucination. In-memory: right-aligned 24-bit in `int32_t`. On-disk: interleaved as `L_mid, L_msb, R_msb, L_lsb, R_lsb, R_mid` per stem. → `references/09-audio-format-spec.md` and `hallucination-watchlist.md`.

**Q: What's the 0/2/1/3 block thing?**
A: Physical disk order of the 4 TE blocks per sector. Enables tape FF/RW by reading less per sector at high speeds. → `references/11-block-interleaving-tape-fx.md`.

**Q: Is `solderless.engineering` online?**
A: Live site **offline** as of 2026-05-09 for an update [tkt1000, Discord #news]. **A 2026-05-12 static snapshot is archived locally** at `assets/solderless-2026-05-12.zip` (community backup, obtained 2026-05-13); runnable via local web server. The archive's JS is the canonical SP-1 host protocol reference. → `references/27-tools-and-utilities.md`.

**Q: Who's TimK / tkt1000 / timknapen?**
A: Same person. Lead firmware engineer. Owns `github.com/timknapen/SP-1-dev`. **Not the same person as ericlewis.** → `synthesis-log.md` identity map.

**Q: What's in `assets/audiothingies-2026-05-09.zip`?**
A: ericlewis's C++17 reference implementation of the stock audio engine. Shared in Discord 2026-05-09. Not in a public repo. → `references/12-audio-engine-internals.md`.

**Q: How do I dump firmware on my own SP-1?**
A: Three options: (1) **LED bit-bang** trick — flash a tiny custom firmware that exfiltrates via LEDs; (2) **APPROTECT voltage glitch** — murray's BeagleBone PRU setup; (3) **SWD with mass-erase + reflash** — read your own code only, not original firmware. → `references/14-approtect-glitch-attack.md` and `references/17-led-bitbang-dump.md`.

**Q: Does the SP-1 have a factory SWD header?**
A: **No.** You solder your own pads. → `references/04-debug-interfaces.md`.

**Q: Can the bootloader read existing firmware?**
A: **No.** Write-only. *"you cannot"* [ericlewis, Discord 2026-05-08]. → `references/15-bootloader-protocol.md`.

**Q: What's the album upload speed?**
A: Three data points: **moecal1947's Python tool** = ~0.75 KB/s (4.5 days per 311 MB album, USB control transfers); **solderless web tool** = ~10 KB/s (~9 hours per 311 MB album, CDC ACM fire-and-forget at 115200 baud — this is the public best); **ericlewis's theoretical max** = ~4 MB/s (minutes per album, requires unbuilt USB bulk + CMD25 firmware). → `references/16-usb-upload-protocol.md`.

**Q: Are there custom firmwares I can flash today?**
A: Only `ericlewis/sp1-midi` (USB MIDI controller; does NOT play stems). emvee1968 and virtualflannel_46386 have unreleased ones with audio. TimK has private ones. → `references/20-custom-firmware-state.md`.

**Q: My device looks bricked after disassembly — what now?**
A: **Reseat the FPC ribbon** before assuming hardware failure. 90% of "bricked" cases are an FPC that looks plugged in but isn't fully seated. → `references/23-physical-disassembly.md`.

**Q: How do I cleanly power down in custom firmware?**
A: Mute codecs, reset audio chips, disable BT module, stop PWMs, disable eMMC supply (P0.14 low), disable button-common rail (P1.10 low), disable 3.072 MHz osc (P0.13 low), call `sys_poweroff()`. Function button longpress is the typical trigger. → `references/05-power-and-battery.md`.

## Protocol Claude must follow

When working on SP-1 topics:

1. **Read accuracy files first.** `hallucination-watchlist.md`, `corrections.md`, `known-unknowns.md`, `working-confirmed.md`. These prevent specific past mistakes.

2. **Cite every factual claim.** Use `[Lines #NNN, author, YYYY-MM-DD]` or `[Discord #channel, author, YYYY-MM-DD]` or `[code: path]` or `[GitHub: repo/file]`. No unsourced claims.

3. **Respect the synthesis date.** Material is current through **2026-05-13**. For "current state" questions (is `solderless.engineering` live? did emvee1968 release?), check `synthesis-log.md` + `update-log.md` and offer to verify with the user.

4. **Don't conflate ericlewis with TimK.** Two different people. Different repos. Different areas of expertise.

5. **Don't conflate PO-32 with SP-1.** `libpo32` is for a different product.

6. **Default to honest uncertainty.** If a question isn't covered in this skill and isn't in the local corpus (see `sources.md`), say "I don't know — verify with the live Discord or ask TimK / ericlewis." Hallucinating a specific number, opcode, pin, or behavior is worse than admitting uncertainty.

7. **The user (dotjustin) maintains this skill.** Questions that come up during work get added to `cc-skill/questions-for-timk.md` (in the user's project directory, not in the skill) for later async resolution. Don't add to the skill itself without verifying claims.

8. **The bundled material in `assets/` is the redistributable corpus.** Currently: TE manuals (PDF + DOCX), the `solderless-2026-05-12/` archive (snapshot of the public web tool), and the `audiothingies-2026-05-09/` + `storagethingies-2026-05-09/` archives (ericlewis's C++17 reference implementations of the stock audio engine and storage layer, originally shared as Discord attachments 2026-05-09; bundled as authoritative reference for the audio engine and eMMC driver). Material **not** bundled and not redistributable: stock TE firmware binary, Kanye stems, anything from non-public Discord channels.

## Synthesis date and freshness

**This skill is a frozen snapshot synthesized through 2026-05-13.** The Discord is active and things may have changed:

- `solderless.engineering` may be back online (or still offline)
- emvee1968's custom firmware may have been released
- virtualflannel_46386's custom OS may have been released
- ericlewis's "proper BSP" may have been released
- New corrections may have superseded current claims

For "what's happening now" questions, ask the user to summarize the current state from the live Discord rather than trusting this skill's snapshot.

## Where to start (by intent)

- **"I want to put my own music on my SP-1"** → `references/21-original-firmware-stems.md`
- **"I want to write custom firmware"** → `references/18-zephyr-build-environment.md` then `references/19-sp1-midi-bsp.md`
- **"I want to add Bluetooth"** → `references/07-bluetooth-module.md` (mostly known-unknowns)
- **"I want to add an SWD header"** → `references/04-debug-interfaces.md` + `references/23-physical-disassembly.md`
- **"I want to dump my SP-1's firmware"** → `references/17-led-bitbang-dump.md` (no glitch attack required)
- **"I want to read the eMMC directly"** → `references/24-emmc-direct-extraction.md`
- **"I want to understand the audio format"** → `references/09-audio-format-spec.md` + `references/11-block-interleaving-tape-fx.md`
- **"I just want to know what the SP-1 is"** → `references/01-overview-and-history.md`

## File inventory

```
sp-1/
├── SKILL.md                              ← this file
├── synthesis-log.md                      ← freshness metadata + identity map
├── hallucination-watchlist.md            ← past AI errors to avoid
├── corrections.md                        ← documented reversals
├── known-unknowns.md                     ← what's still open
├── working-confirmed.md                  ← safe-claims list
├── sources.md                            ← where to find primary sources
├── references/
│   ├── 01-overview-and-history.md        (+ -faq.md)
│   ├── 02-hardware-overview.md           (+ -faq.md)
│   ├── 03-pcb-and-schematic.md           (+ -faq.md)
│   ├── 04-debug-interfaces.md            (+ -faq.md)
│   ├── 05-power-and-battery.md           (+ -faq.md)
│   ├── 06-audio-codecs.md                (+ -faq.md)
│   ├── 07-bluetooth-module.md            (+ -faq.md)
│   ├── 08-emmc-storage.md                (+ -faq.md)
│   ├── 09-audio-format-spec.md           (+ -faq.md)
│   ├── 10-midi-timing-encoding.md        (+ -faq.md)
│   ├── 11-block-interleaving-tape-fx.md  (+ -faq.md)
│   ├── 12-audio-engine-internals.md      (+ -faq.md)
│   ├── 13-dsp-effects.md                 (+ -faq.md)
│   ├── 14-approtect-glitch-attack.md     (+ -faq.md)
│   ├── 15-bootloader-protocol.md         (+ -faq.md)
│   ├── 16-usb-upload-protocol.md         (+ -faq.md)
│   ├── 17-led-bitbang-dump.md            (+ -faq.md)
│   ├── 18-zephyr-build-environment.md    (+ -faq.md)
│   ├── 19-sp1-midi-bsp.md                (+ -faq.md)
│   ├── 20-custom-firmware-state.md       (+ -faq.md)
│   ├── 21-original-firmware-stems.md     (+ -faq.md)
│   ├── 22-disassembly-ghidra.md          (+ -faq.md)
│   ├── 23-physical-disassembly.md        (+ -faq.md)
│   ├── 24-emmc-direct-extraction.md      (+ -faq.md)
│   ├── 25-3d-printing-mods.md            (+ -faq.md)
│   ├── 26-community-and-authority.md     (+ -faq.md)
│   └── 27-tools-and-utilities.md         (+ -faq.md)
└── assets/
    ├── TE-Stem-User-Guide.docx           ← TE official, 16 KB
    ├── TE-Stem-User-Guide.pdf            ← TE official, 59 KB
    └── TE-Stem-Player-manual.pdf         ← TE official, 509 KB
```

**6 top-level accuracy files + 27 deep references + 27 FAQ companions + 3 bundled manuals + this SKILL.md = 64 files.**
