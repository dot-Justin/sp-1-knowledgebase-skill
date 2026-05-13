# Gaps Index — Where the SP-1 Skill Falls Short

This file is the comprehensive triage list of **documentation and knowledge gaps** in the skill as of synthesis date 2026-05-13. It is broader than `known-unknowns.md` (which is reserved for *research-blocking unknowns* with a clear "next-move" entry) — this file also catches:

- TBDs left in reference prose ("not yet publicly documented", "exact value unknown")
- Synthesis gaps where bundled primary sources have the answer but the skill hasn't surfaced it
- Cross-reference holes where one reference points at another that doesn't deliver
- Open questions inside the reference files that never made it to the formal known-unknowns

Each entry: **what's missing, what *is* known, where the question surfaces, severity tier, and the next move.**

Severity tiers:

- **T1 — Project-blocking.** A user trying to follow the skill's advice hits a wall.
- **T2 — Use-case-blocking.** A specific path is non-functional without this; other paths work fine.
- **T3 — Detail-level.** Annoying inconsistency or a "would be nice" precision question.

When a gap closes, **move the entry to `corrections.md`** (so the audit trail survives) and update the affected reference. Don't just delete it.

---

## Tier 1 — Project-blocking gaps

### T1-01 SWD pad layout on the rear PCB

**Where it surfaces:** `04-debug-interfaces.md:80`, `23-physical-disassembly.md:70`, `03-pcb-and-schematic.md:128`, `14-approtect-glitch-attack.md:106`.

**What's known:**
- A standard 10-pin SWD/JTAG header footprint exists on the **back of the main PCB** [Lines #068 (cross-referenced in `assets/lines-thread-archive/agent/posts/068.md`); Lines #243; Lines #417, TimK — *"this one has a SWD connector soldered to the back"*; Lines #605/613, TimK — *"there is no way to change the firmware (and thus the content) without fully opening up the device for access to the SWD lines on the back of the main PCB"*].
- 3 test points are adjacent to the header [Lines #068].
- Adafruit-style header (`adafruit.com/product/752`) fits [Lines #214].

**What's missing:** exact PCB silkscreen labels, pad coordinates, and which of the 10 pins maps to which signal (SWDIO / SWCLK / GND / VDD / nRESET / etc.). TimK said *"I'll add documentation of the test points to the wiki"* on 2026-05-07 17:43; the bundled `SP-1-dev-2026-05-13/wiki/` does **not yet** include this page.

**Why T1:** the skill repeatedly tells users "add an SWD header" as the path to serious firmware development; without the pad map this advice is followable only by people who can probe traces themselves.

**Next move:** ask TimK directly in Discord for a board-photo annotation, or wait for the wiki update, or trace the SWD lines on a real PCB with a multimeter (SWDIO rests at +3v3, SWCLK at 0v per Lines #068).

---

### T1-02 eMMC test point coordinates on the rear PCB

**Where it surfaces:** `04-debug-interfaces.md:90`, `24-emmc-direct-extraction.md:29`, `08-emmc-storage.md`, `23-physical-disassembly.md:78`, `03-pcb-and-schematic.md:126`.

**What's known:**
- All four data lines (DAT0–DAT3) plus CLK and CMD are broken out to test points [Discord #hardware, tkt1000, 2026-05-07 17:53; Lines #316; Lines #433 — *"DAT0-DAT3 test points"*].
- Faster than the on-chip 1-bit read because external readers can use the full 4-bit bus.

**What's missing:** PCB coordinates / silkscreen labels for each of the 6 pads. Methods 1 (test-point read) and 3 (in-place ISP read) of `24-emmc-direct-extraction.md` cannot be executed without these.

**Why T1:** the skill describes a fast non-destructive read path that no one can follow without re-probing the board.

**Next move:** same as T1-01 — TimK wiki update, or scope each test point on real hardware, or annotate a high-res PCB photo from the Lines archive.

---

### T1-03 KiCad schematic / PCB CAD source

**Where it surfaces:** `03-pcb-and-schematic.md:33`.

**What's known:**
- TimK reverse-engineered enough of the PCB to extract the full nRF52840 pinout, BQ24232 wiring, and CYBT-353027-02 wiring [Lines, TimK].
- TimK quoted in Lines: *"I have reverse engineered most of the PCB. I have the full pinout and schematic of MCU and all relevant ICs."*
- But also: *"I didn't even draw a full schematic, only the bare minimum needed for working on the firmware. I didn't bother at all with documenting (passive) component values, understanding trace impedance, etc."*

**What's missing:** the partial KiCad files have not been published. A *full* schematic does not exist anywhere.

**Why T1:** this single artifact would resolve at least six other gaps (T1-04, T1-05 indirectly, T2-02 ISET values, T2-07 second-jack pins, several Tier-3 component IDs). It is the highest-leverage closed door in the whole skill.

**Next move:** ask TimK whether he'll publish the partial KiCad, or fund/coordinate a complete PCB reversal effort (Lines threads include offers from community members but no public output).

---

### T1-04 Power-tree regulator IDs and topology

**Where it surfaces:** `02-hardware-overview.md:48`, `05-power-and-battery.md:25`, `03-pcb-and-schematic.md:137`.

**What's known:**
- USB → BQ24232 charger → LiPo + system rail [`assets/SP-1-dev-2026-05-13/wiki/Battery-charger.md`].
- 3.3 V and 1.8 V rails exist; both are derived from the LiPo output [Lines, anonymous — *"Audio codec has both +3v3 & +1v8 power derived from the LiPo output. There are test points for both of these sources. The +1v8 is regulated directly from the +3v3 output (likely an LDO for better noise performance)"*].
- 3.3 V test point and 1.8 V test point both exist on the PCB [same Lines post].

**What's missing:** the regulator IC part numbers (manufacturer, datasheet). Whether there's a separate audio analog supply. Max current per rail. Thermal envelope.

**Why T1:** power budgeting for any custom firmware (especially audio + Bluetooth simultaneously) is guesswork without these. Replacing failed components is impossible.

**Next move:** photograph the regulator ICs on a real PCB and identify by markings; or wait for KiCad publication (T1-03).

---

### T1-05 nRF52840 silicon revision cataloging

**Where it surfaces:** `14-approtect-glitch-attack.md:29`, `known-unknowns.md::Hardware variants` (touches but doesn't focus on silicon revision).

**What's known:**
- The Nordic APPROTECT vulnerability that murray exploited was patched in newer silicon revisions. The fix shipped around **2021-11-21** [Lines, multiple].
- SP-1 units were circulating in 2022 [Lines].
- But the TE version was intended for *Jesus is King* (Aug 2019); the device's silicon likely predates the fix.
- The vulnerability is described in **limitedresults**' 2020 public writeup ("nRF52 Debug Resurrection / Access Port Protection Bypass") for revs A and early B.
- Lines community noted [Lines, paraphrased]: *"if you take a close up pic of the nrf chip, it should have a date code and some kind of marking to indicate silicon revision."* Specific date codes mentioned in Lines include "revision F, 2022" and "revision D, 2019" (interpretations vary).
- murray's glitch succeeded on attempt #8504 on 2025-01-25, confirming **at least his unit** has vulnerable silicon.

**What's missing:**
- A cross-unit catalog of nRF52840 date codes (which production batches of SP-1 use which silicon).
- Confirmation of whether any SP-1 batch shipped with post-fix silicon (which would make the glitch attack impossible for those units).
- A reliable way for an owner to read the date code without opening the device.

**Why T1:** users following the APPROTECT-bypass path may waste weeks on a glitch campaign that can't possibly succeed because their unit's silicon is patched. The skill currently doesn't warn them to check.

**Next move:** crowd-source date code photos from Discord owners; cross-reference with serial-number patterns (Lines #375 noted differences between `Y4KNGAKJ` and `Y4KNGAGX` units in build quality — may also correlate with silicon revision).

---

### T1-06 Stock-firmware UI / menu state machine

**Where it surfaces:** `22-disassembly-ghidra.md:104`, `13-dsp-effects.md` (effects activation UX not described), `19-sp1-midi-bsp.md:208` (BSP doesn't include it).

**What's known:**
- The stock firmware has a UI state machine: button event handling, LED animations, effect activation (effect type + variation selection), volume display, transport mode visualization, power on/off sequence.
- `audiothingies/` (bundled at `assets/audiothingies-2026-05-09/`) is explicitly the **engine** only — no UI layer.
- emvee1968's custom firmware reimplemented some of this (gate effect, vol+/vol- BT pairing combo) but didn't implement power-off [Discord #firmware, 2026-05-08].
- ericlewis is building a "proper BSP" with audio that presumably includes a UI layer, but it isn't published.

**What's missing:** a structured description (or disassembly walkthrough) of how the stock firmware translates button events into engine state changes, specifically:
- Effect type / variation selection (which buttons cycle which axis)
- Solo vs mute behavior
- Tape-FX rate stepping
- LED animations during transport changes
- Power on / power off sequence including which subsystems are torn down in which order

**Why T1:** custom firmware authors aiming for stock-feel UX must reimplement this from scratch, with no reference. The DSP / storage / mixer layers are well-documented; the glue layer that makes the device feel like an SP-1 isn't.

**Next move:** Ghidra-dive into the stock firmware app region (`0x20000`+) and document the input handler dispatch table; or ask ericlewis for an outline; or wait for his "proper BSP" release.

---

## Tier 2 — Use-case-blocking gaps

### T2-01 TAS2505 DSP coefficient table interpretation

**Where it surfaces:** `06-audio-codecs.md:243`, `22-disassembly-ghidra.md:118`, `assets/SP-1-dev-2026-05-13/wiki/I2C.md` — wiki explicitly says *"Todo: setup DAC coefficients see 6.1.4 fro DAC programmable coefficients"*.

**What's known:** the table at `sp1-midi/drivers/audio/tas2505_codec.c:27` loads `{register, byte0, byte1, byte2}` triples to specific DSP pages on the TAS2505 (page 44+, per the bundled driver source). Presumed purpose: speaker EQ + bass limiter to protect the small internal driver.

**What's missing:** what filter shapes do these coefficients implement (low-shelf cut? bass limiter? brick-wall HPF?). Which page+register pairs are speaker EQ vs limiter vs DSP routing.

**Why T2:** any custom firmware that wants stock-equivalent speaker tonality has to either reuse this exact table (currently does, blindly) or recompute it from the TAS2505 application reference guide section 5.5–5.6 and 6.1.4.

**Next move:** decode the coefficient bytes against the TAS2505 datasheet's biquad coefficient encoding; or ask TimK directly.

---

### T2-02 BQ24232 ISET resistor values (fast vs slow charge current)

**Where it surfaces:** `known-unknowns.md::BQ24232 battery charger driver completeness`, `05-power-and-battery.md:124`, `assets/SP-1-dev-2026-05-13/wiki/Battery-charger.md`.

**What's known:** USB current cap is 500 mA (set by ISET network). The ISET override pin (P1.00) switches between two charge-current setpoints, but the actual resistor values (and therefore the two current levels) aren't published.

**What's missing:** the resistor values on the schematic.

**Why T2:** fast/slow charge mode switching, fuel-gauge inference from ISET voltage, and accurate thermal modeling all need this.

**Next move:** measure on real hardware (toggle the override pin, scope ISET voltage), or wait for T1-03.

---

### T2-03 Battery voltage divider ratio (for SAADC scaling)

**Where it surfaces:** `05-power-and-battery.md:71`, `assets/SP-1-dev-2026-05-13/wiki/Battery-charger.md` (says "over a voltage divider" without giving the ratio).

**What's known:** P0.28 / AIN4 reads battery via resistor divider. `sp1-midi/subsys/power/PowerManager.cpp` (bundled at `assets/sp1-midi-2026-05-13/subsys/power/`) implements the scaling but the ratio is encoded in code, not surfaced as a documented constant.

**What's missing:** the exact divider ratio, so any future driver / fuel-gauge implementation can independently verify.

**Why T2:** accurate battery-percentage reporting depends on this. Off-by-one-V errors are easy to make.

**Next move:** read the `PowerManager.cpp` source in the bundled `sp1-midi-2026-05-13/` and surface the ratio as a documented value.

---

### T2-04 LED bit-bang extraction protocol details

**Where it surfaces:** `17-led-bitbang-dump.md:117`.

**What's known:** zee_33 demonstrated the technique in May 2026: flash a 1-page (~4 KB) custom firmware that reads flash and modulates 8 LEDs; capture with a phone camera; decode.

**What's missing (all four sub-questions):**
- Exact LED encoding scheme (binary vs grayscale vs error-correcting).
- Address ranges actually dumped (bootloader region, UICR, etc.).
- Verification methodology (CRC against murray's known-good dump?).
- Whether the dump firmware is published anywhere.

**Why T2:** this is the *only* firmware-extraction path that works on an APPROTECT-locked unit without opening the device. Reproducibility hinges on the missing details.

**Next move:** ask zee_33 in Discord for the source / encoding spec.

---

### T2-05 Firmware-internal GPREGRET semantics

**Where it surfaces:** `corrections.md:206`, `hallucination-watchlist.md:131`, `known-unknowns.md::Alternative bootloader modes`.

**What's known:**
- The host doesn't need `GPREGRET = 0x1A96` — that value is firmware-internal at most.
- Host mode-switch is `0x70 [1]` + `0x50` reboot.
- The firmware presumably uses *something* to persist "boot to upload parser" across the reboot, since GPREGRET retains across `sys_reboot()`.

**What's missing:** the actual firmware-side logic: what value(s) are stored in GPREGRET (or another retention register), how the bootloader reads them on next boot, and whether `0x1A96` was real or a Discord paraphrase artifact.

**Why T2:** custom firmware authors implementing a "reboot to bootloader" feature need to replicate this exact mechanism; doing it wrong produces a soft-brick where the device boots back into the broken app instead of the bootloader.

**Next move:** Ghidra-dive into the bootloader region (`0x00000000`–`0x00020000`) and find the GPREGRET / retention-register read on reset, or ask TimK.

---

### T2-06 Second 3.5 mm jack (MIDI / PO-sync) pin assignment

**Where it surfaces:** `known-unknowns.md::Second 3.5mm jack`, `02-hardware-overview.md:23`, `assets/SP-1-dev-2026-05-13/wiki/Peripherals.md` (says *"Todo: MIDI / PO sync"*).

**Status:** already tracked in `known-unknowns.md`. Cross-referenced here so it's not lost from a triage perspective.

---

### T2-07 Stock-firmware song-boundary crossfade

**Where it surfaces:** `09-audio-format-spec.md:172`, `12-audio-engine-internals.md:305`.

**What's known:** both references *presume* stock firmware applies a short crossfade on song transitions ("Stock TE firmware **presumably** applies short crossfades on song transitions in playback code"). The audiothingies reference engine doesn't implement crossfades.

**What's missing:** whether crossfades actually happen, and if so, the length / shape. Verification by listening to a real stock-firmware playback at song boundaries, or by reading the stock-firmware disassembly.

**Why T2:** custom firmware that omits this will have noticeably abrupt song transitions vs stock.

**Next move:** record a stock SP-1 playing past a song boundary and inspect the audio; or scan the disassembly for fade-related code.

---

### T2-08 Effect-state UX layer (button combos → DSP effect state)

**Where it surfaces:** `13-dsp-effects.md` (DSP described; UX layer absent).

**What's known:** the four effect types and four variations exist; only one stem can have an active effect at a time; transition fades exist.

**What's missing:** which button combos select which effect type / variation in stock firmware. How activation state changes when a Track button is held vs tapped. Whether long-presses do anything. The mapping from `INPUT_KEY_*` events to `EffectState` field changes.

**Why T2:** UX-layer gap; sibling of T1-06. Custom firmware author must invent their own combo language or replicate from observation of a stock unit.

**Next move:** characterize on a real stock SP-1 (record button presses + audio response) or Ghidra-dive the app region.

---

### T2-09 Per-block LED data semantics in stock-firmware albums

**Where it surfaces:** `10-midi-timing-encoding.md` (full file), `corrections.md` (per-block 8-byte trailer correction), `working-confirmed.md` (encoder-side spec).

**What's known:**
- TKT wiki documents a per-TE-block trailer with 4 bytes of LED data (8-byte total trailer = 2 sync + 2 tempo + 4 LED).
- The solderless encoder writes *none* of this. Custom albums play correctly without it.
- Stock TE albums presumably contain LED data and produce richer LED animations during playback.

**What's missing:** what stock LED animations does the data actually drive, and what's the encoding (raw 0–255 per stem? RGB-ish quad? Frame-by-frame animation?). The `audiothingies` `current_track_led_word()` accessor exposes a 32-bit word; we know its shape but not its semantic.

**Why T2:** custom albums lose the LED animation richness of stock content. Reproducing it requires understanding the encoding.

**Next move:** dump a stock album's LED bytes (after firmware extraction), correlate to observed LED behavior during playback.

---

### T2-10 nRF52840 SYSTEM_OFF wake-source configuration

**Where it surfaces:** `05-power-and-battery.md:116`, `known-unknowns.md::Power-off in custom firmware` (closely related but not the same).

**What's known:** stock firmware wakes from SYSTEM_OFF on USB-attach and on function-button press. `sp1-midi/subsys/power/PowerManager.cpp` exists (bundled) but as of synthesis date doesn't demonstrate a complete wake configuration.

**What's missing:** which `NRF_POWER->SYSTEMOFF` configuration produces the desired wake sources, with the SP-1's specific pinout (function button on P0.27 direct GPIO, USB attach detection via `nPGOOD` from BQ24232).

**Why T2:** custom firmware that powers off correctly but can't wake is functionally bricked until battery removal.

**Next move:** read `PowerManager.cpp` from bundled `assets/sp1-midi-2026-05-13/`, extract the wake-source code path, and document.

---

## Tier 3 — Detail-level gaps

| ID | Gap | Where it surfaces |
| --- | --- | --- |
| T3-01 | TAS2505 page-by-page register usage beyond pages 0 / 1 / 44 | `06-audio-codecs.md:221`; bundled `wiki/I2C.md` shows P0/P1 only |
| T3-02 | nRF I²S buffer count rationale (`CONFIG_I2S_NRFX_TX_BLOCK_COUNT=8` — why 8 not 4?) | `12-audio-engine-internals.md:294` |
| T3-03 | TimK's FF rate measurement (2.0× in his code vs 2.5× / 4× / 8× / 16× in `audiothingies::kFastForwardRates`) | `11-block-interleaving-tape-fx.md:173`, `corrections.md:152` |
| T3-04 | Stock-firmware Bluetooth HCI command stream at boot | `22-disassembly-ghidra.md:108`, `07-bluetooth-module.md:71` |
| T3-05 | `ResetBreadcrumbs` enum values (canonical reset reasons) | `04-debug-interfaces.md:134`, `19-sp1-midi-bsp.md:124` |
| T3-06 | PWM peripheral period used by stock firmware (vs `sp1-midi`'s 1024 µs) | `02-hardware-overview.md:186`, bundled `wiki/PWM.md` is a stub |
| T3-07 | Envelope-byte semantics in stock-firmware albums (solderless encoder writes `(255 * peak) / 0x800000` per-stem — does stock use the same scaling?) | `working-confirmed.md`, `21-original-firmware-stems.md` |
| T3-08 | Wiki SAADC and PWM pages still stub-status in bundled `wiki/` | `assets/SP-1-dev-2026-05-13/wiki/SAADC.md`, `PWM.md` |
| T3-09 | Whether stock firmware reads back a per-block sync counter at all (solderless writes none and stock-formatted custom albums still drive musically-synced effects) | `10-midi-timing-encoding.md:95` |
| T3-10 | nRF radio peripheral usable as a high-precision timer | `02-hardware-overview.md:13` (mentioned only theoretically) |
| T3-11 | TimK's "minimum schematic" content scope | `03-pcb-and-schematic.md:33` (TimK says it's "the bare minimum needed for working on the firmware") |
| T3-12 | P1.05 PIN_CY_SPI_CSN actual purpose (CYBT secondary control vs unused) | `07-bluetooth-module.md:18`, `03-pcb-and-schematic.md:163` |
| T3-13 | CS42L42 mic-input data path — wiki says *"ASP_SDOUT is unconnected"* meaning mic data only reaches nRF via I²C status registers, but the audio engine doesn't expose a mic-input route | `06-audio-codecs.md:34`, bundled `wiki/I2C.md` |
| T3-14 | Cross-stack divergence: TimK's wiki examples use nRF5 SDK v17.0.2 (deprecated, unsupported by Nordic); ericlewis's BSP uses Zephyr. Skill doesn't surface this | bundled `wiki/Peripherals.md` ("the old (and unsupported) nRF5 SDK v17.0.2, because that's what I've been using") |
| T3-15 | TAS2505 NFC-pin GPIO repurposing flag requirement — wiki says specifically `NFCT_ENABLED 0` + `CONFIG_NFCT_PINS_AS_GPIOS 1` required, currently only briefly mentioned in `02-hardware-overview.md:201` | bundled `wiki/I2C.md:209` |

---

## Structural gaps in the skill

These are about the skill's own internal consistency, not about the SP-1 itself.

### S-01 FAQ companions never propagated for the 2026-05-13 batch

The 2026-05-13 batch (`update-log.md`) explicitly deferred FAQ updates for `15-bootloader-protocol-faq.md`, `16-usb-upload-protocol-faq.md`, `21-original-firmware-stems-faq.md`, `09-audio-format-spec-faq.md`. These may now contain stale Q&A wording.

**Fix:** small consistency pass on those four FAQ files.

### S-02 `cc-skill/questions-for-timk.md` is referenced but not in the skill

`SKILL.md` mentions the file in the "Protocol Claude must follow" section, but it lives in the user's project directory (`~/Projects/sp-1/cc-skill/`), not in the skill. From the skill's perspective the reference is a dead link.

**Fix:** either mirror a sanitized copy into the skill or remove the reference from `SKILL.md`.

### S-03 `known-unknowns.md` mixes "research-blocking unknowns" with general TODOs

The file has 14 entries today, some of which are "ask in Discord and you'll get an answer" (e.g., second-jack pin assignment) and others which are open-ended research projects (e.g., full effects implementations). Splitting "askable" vs "research-grade" would aid triage.

**Fix:** consider re-organizing `known-unknowns.md` by what kind of work would close each entry.

### S-04 No formalized index of "what would be closed by reading the bundled material"

Many gaps in this file are partly resolved by content in `assets/SP-1-dev-2026-05-13/wiki/`, `assets/sp1-midi-2026-05-13/`, `assets/audiothingies-2026-05-09/`, `assets/storagethingies-2026-05-09/`, or `assets/lines-thread-archive/`. The skill cites these inline but doesn't have a single "what's in the bundled corpus and which gaps it closes" map. `sources.md` is close but doesn't focus on gap-closure.

**Fix:** consider adding a short "bundled corpus map" section to `sources.md` or a new top-level file.

---

## How to use this file

When a question doesn't have a clear answer in the references:

1. **Check `known-unknowns.md` first** — formally-tracked research unknowns.
2. **Check this file** — broader documentation / TBD gaps.
3. **Check `corrections.md`** — closed gaps preserved as audit trail.
4. **Check the bundled `assets/`** — the most recent source material may have the answer, especially the SP-1-dev wiki and the Lines archive.

When a gap closes:
1. Move the entry to `corrections.md` with the standard Believed/Actual/Source structure.
2. Update the affected references.
3. Note the closure in `update-log.md` under the relevant batch.

Don't leave stale gap entries here — drift between this file and reality is the failure mode this file exists to prevent.
