# Known Unknowns — What The Community Hasn't Resolved

For each unresolved area, this file describes:
- **What the question is**
- **What's known** (the partial answers)
- **Why it's unresolved** (technical barrier, lack of effort, awaiting hardware, etc.)
- **Who would have the next move** (if Claude needs to suggest where to ask)

**Claude must consult this file before asserting a definitive answer about any topic listed below.** When asked about something here, the correct response is to describe what *is* known, name the gap, and suggest the next step (often: "ask in the Discord" or "check if X has published yet").

---

## Bluetooth audio module integration

**Question:** How do you talk to the CYBT-353027-02 Bluetooth module from custom firmware?

**Known:**
- The module is wired to the nRF52840 via UART (TX P1.02, RX P1.04, RTS P1.01, CTS P1.03, Reset P0.10) [code: `assets/sp1-midi-2026-05-13/boards/.../stem_player.dts`]
- TimK's public BSP defines the pins but does not implement a stack [code: `ericlewis/sp1-midi`]
- **emvee1968 claims a working Bluetooth pairing implementation** using the vol+/vol- combo, developed "in a few hours with Claude Code" — but the code is **not yet released** [Discord #firmware, emvee1968, 2026-05-08 23:26 + 23:42]
- TimK has not personally implemented BT: *"I never even properly looked into that"* [Discord #firmware, tkt1000, 2026-05-08 23:27]
- The CYBT-353027-02 is a Cypress/Infineon proprietary module; protocol details are not in the public domain in a form integrated with this device

**Why unresolved:** No one with public-release intent has done it yet. emvee1968 may release the code "next week" (i.e., week of 2026-05-12) when more units arrive [Discord #firmware, emvee1968, 2026-05-08 23:56].

**Next move:** Check `github.com/emvee1968` (if user knows the GitHub handle) or ask in Discord for the status of emvee1968's release. If proceeding without their code, plan on writing a UART driver for CYBT and a HCI host stack — significant work.

---

## eMMC write path

**~~Question:~~ RESOLVED 2026-05-13.** The full byte-level CDC packet protocol for album upload is now documented in `references/15-bootloader-protocol.md` and `references/16-usb-upload-protocol.md` based on the solderless source archive (ingested 2026-05-13). Key findings:
- `0x39` payload is 136 bytes: `chunk_counter (LE32) | emmc_byte_offset (LE32) | 128 bytes data`. Not a sector address — a byte offset.
- Packets are fire-and-forget (no per-chunk ACK). Throughput limited by 115200-baud CDC ACM ≈ 10 KB/s.
- Mode switch via `0x70 [1]` + `0x50` reboot. No host-visible GPREGRET magic — the `0x1A96` claim was a Discord paraphrase that doesn't appear in the host code.
- moecal's 0.75 KB/s comes from USB control transfers (high per-transfer overhead). Solderless's ~10 KB/s comes from CDC bulk-like streaming. ericlewis's "~4 MB/s" requires a hypothetical USB MSC / bulk-endpoint implementation with CMD25 multi-block eMMC writes that has not been built publicly.

See `update-log.md` 2026-05-13 and `working-confirmed.md` for the full resolution.

---

## Power-off in custom firmware

**Question:** How do you cleanly power down the SP-1 from custom firmware?

**Known:**
- The function button is **direct GPIO** (P0.27), not on a ladder like the other buttons [Discord #firmware, ericlewis, 2026-05-09 00:59]
- The stock firmware uses this to enter a deep sleep / power-off state
- emvee1968's firmware *does not currently power off properly* — he has to disconnect the battery after flashing to get into the bootloader [Discord #firmware, emvee1968, 2026-05-08 23:56]
- ericlewis's `sp1-midi` BSP has `PowerManager.cpp` (subsys/power) with battery/charger/USB monitoring [code: `assets/sp1-midi-2026-05-13/subsys/power/`]
- ericlewis: *"ALWAYS REMEMBER TO PROGRAM IN YOUR POWER DOWN"* [Discord #firmware, ericlewis, 2026-05-09 01:02]

**Why unresolved:** The PowerManager in `sp1-midi` exists but isn't wired up by the example MIDI controller app to handle the "off" state. Custom firmware authors have to integrate it themselves and verify nRF52840 SYSTEM_OFF behavior with the BQ24232 charger and TAS2505/CS42L42 codec power lines.

**Next move:** Read `assets/sp1-midi-2026-05-13/subsys/power/PowerManager.cpp` and study `CONFIG_POWEROFF=y` Zephyr APIs. If still stuck, ask ericlewis.

---

## BQ24232 battery charger driver completeness

**Question:** How complete is the charger driver in the public BSP, and what's needed for production use?

**Mostly resolved as of 2026-05-12** following TKT wiki Battery-charger page incorporation:

- `assets/sp1-midi-2026-05-13/drivers/charger/charger_bq24232.c` exists with GPIO-based control:
  - nPGOOD = P0.24 (open-drain, low when USB power good)
  - nCHG = P0.22 (open-drain, low while charging)
  - CE = P0.21 (active-low charge enable)
  - **ISET = P1.00** (corrected from earlier erroneous "P0.19" — see `corrections.md`)
  - BATT_LEVEL = P0.28 / AIN4
  [code: `assets/sp1-midi-2026-05-13/boards/teenageengineering/stem_player/stem_player.dts`; `assets/SP-1-dev-2026-05-13/src/stemplayer_pins.h`]
- **Maximum USB current draw: 500 mA** [TKT wiki: Battery-charger, accessed 2026-05-12]
- ISET voltage reflects actual charging current; can be monitored via a voltage divider on AIN [TKT wiki: Battery-charger, accessed 2026-05-12]

**Still open:** The specific ISET resistor values that determine the fast/slow charge currents are on the schematic but not in code or wiki. For most firmware work the existing driver is "complete enough."

**Next move:** For deep customization (e.g., implementing fast/slow charge mode switching), measure ISET voltage on real hardware while toggling the override pin, or consult TimK's KiCad reversal for resistor values.

---

## SWD pad layout on the rear PCB

**Question:** Where exactly is the SWD header footprint on the SP-1's main PCB, and which physical pad is which signal?

**Known:**
- A standard 10-pin SWD/JTAG header footprint exists on the **back of the main PCB** [Lines #068; Lines #243; Lines #417 (TimK) — *"this one has a SWD connector soldered to the back"*; Lines #605/613 (TimK) — *"there is no way to change the firmware … without fully opening up the device for access to the SWD lines on the back of the main PCB"*]
- 3 test points are exposed adjacent to the header [Lines #068]
- The Adafruit 752-style 10-pin header fits the footprint [Lines #214]
- Multiple community members have soldered headers; TimK said *"I'll add documentation of the test points to the wiki"* on 2026-05-07 17:43

**Why unresolved:** the promised TimK wiki page hasn't shipped — bundled `assets/SP-1-dev-2026-05-13/wiki/` does not contain a dedicated SWD or test-point page as of synthesis date. No annotated PCB photograph circulates publicly with pad-by-pad labels.

**Why it matters:** the skill repeatedly recommends "add an SWD header" as the path to serious firmware development. Without the pad map, this is followable only by people who can probe the PCB themselves (multimeter on SWDIO = +3.3 V rest state, SWCLK = 0 V rest state per Lines #068).

**Next move:** ask TimK in Discord for a board-photo annotation, or wait for the wiki update. As a fallback, the bundled Lines archive (`assets/lines-thread-archive/`) contains photographs and discussions in the post-200 range that can guide manual identification.

---

## eMMC test point coordinates on the rear PCB

**Question:** Where exactly are the DAT0–DAT3 / CLK / CMD test points on the main PCB?

**Known:**
- All four eMMC data lines (DAT0–DAT3) plus CLK and CMD are routed to test points [Discord #hardware, tkt1000, 2026-05-07 17:53 — *"DAT0-DAT3 are broken out to test points, so you could read much faster than the MCU!"*; Lines #316; Lines #433 — *"DAT0-DAT3 test points"*]
- Faster than the on-chip 1-bit read because external readers can drive the full 4-bit bus
- TimK promised wiki documentation on 2026-05-07 17:43

**Why unresolved:** same as SWD pads — promised but not shipped. Bundled `assets/SP-1-dev-2026-05-13/wiki/` has no eMMC test-point page; the Peripherals wiki page lists eMMC interfacing itself as a TODO.

**Why it matters:** `references/24-emmc-direct-extraction.md` Methods 1 (test-point read) and 3 (in-place ISP) cannot be executed without the pad coordinates. The non-destructive eMMC dump path is documented as available but is in practice non-functional.

**Next move:** same as the SWD-pad entry. Alternatively, since CLK and CMD are P0.06 / P0.08 nRF pins, the test points can be located by continuity probing from those nRF pins on a working board.

---

## KiCad schematic source files

**Question:** Will TimK's reverse-engineered KiCad schematic be published?

**Known:**
- TimK has reverse-engineered the nRF52840 pinout, BQ24232 wiring, and CYBT-353027-02 wiring [Lines, TimK]
- TimK: *"I have reverse engineered most of the PCB. I have the full pinout and schematic of MCU and all relevant ICs."* [Lines]
- But also: *"I didn't even draw a full schematic, only the bare minimum needed for working on the firmware. I didn't bother at all with documenting (passive) component values, understanding trace impedance, etc."* [Lines]
- The KiCad files are private; bundled `assets/SP-1-dev-2026-05-13/` does not contain them

**Why unresolved:** TimK has held them privately. There's no public statement of intent to release.

**Why it matters:** this single artifact would resolve at least six other entries — BQ24232 ISET resistor values, 3.3 V / 1.8 V regulator part numbers, second-jack pin assignment, all "unused" GPIO routing, FPC pin-by-pin map, and the CYBT secondary-control-path (P1.05) question. Highest-leverage closed door in the skill.

**Next move:** ask TimK whether he'll publish the partial KiCad. As a deeper alternative, coordinate a community-funded full PCB reversal — Lines posts in the #305–330 range and later include offers from community members for collaborative reversal work.

---

## nRF52840 silicon revision across SP-1 production batches

**Question:** Do all SP-1 units use APPROTECT-vulnerable nRF52840 silicon, or did some batches ship with the post-fix revision?

**Known:**
- The Nordic APPROTECT timing-glitch vulnerability that murray exploited was patched in newer silicon (fix shipped around **2021-11-21**) [Lines, multiple references]
- The vulnerability is documented in `limitedresults`' 2020 writeup for rev A and early rev B silicon
- TE SP-1 development started ~2018 and units shipped to Ye in September 2020 [Lines #743] — well before the silicon fix
- But some SP-1 units were circulating in 2022 [Lines, multiple]
- murray's glitch succeeded on attempt #8504 on 2025-01-25, confirming **at least his unit** has vulnerable silicon
- Lines community discussed reading the nRF chip date code (markings include date and revision letter) — interpretations vary: "revision F, 2022" or "revision D, 2019" appear in posts
- Lines #375 noted significant build-quality differences between units with serial numbers `Y4KNGAKJ` and `Y4KNGAGX`, suggesting different production batches

**Why unresolved:** no organized catalog cross-references unit serial numbers, production-batch markers, and nRF52840 date codes.

**Why it matters:** a user following the APPROTECT-bypass path (or someone trying to set up murray's BeagleBone PRU rig) may waste weeks on a glitch campaign that can't possibly succeed because their unit's silicon is patched. The skill's `14-approtect-glitch-attack.md` currently doesn't warn users to check.

**Next move:** crowd-source nRF52840 date code photographs from Discord owners; cross-reference with serial numbers; build a "your unit is/isn't glitchable" lookup. Until then, the safe advisory is "open the device, photograph the nRF chip date code, check against Nordic's vulnerability matrix before starting a glitch campaign."

---

## Stock-firmware UI / menu state machine

**Question:** How does the stock TE firmware translate button events into engine state changes (effect selection, transport mode, power on/off, LED animations)?

**Known:**
- The stock firmware has a UI layer; `audiothingies/` (bundled at `assets/audiothingies-2026-05-09/`) is explicitly the engine only, no UI
- emvee1968's unreleased custom firmware reimplements some of this (gate effect activation, vol+/vol- combo for BT pairing) but explicitly does not implement power-off [Discord #firmware, 2026-05-08]
- ericlewis stated his "proper BSP" is in development and will presumably include a UI layer; not published
- `references/19-sp1-midi-bsp.md` describes the public BSP's button-to-MIDI mapping but that is a MIDI controller, not a replication of stock UX

**Why unresolved:** no public reverse-engineering of the stock app region (`0x20000`+) at the UI-state-machine level. ericlewis's audiothingies is the engine; the glue / UX layer is missing from public artifacts.

**Why it matters:** custom firmware authors aiming for stock-feel UX must reimplement this from scratch with no reference. Specific sub-questions:
- Which button combos select which effect type and variation
- Solo vs mute behavior (Track button hold vs tap)
- Tape-FX rate stepping behavior
- LED animations during transport changes
- Power on / power off subsystem teardown order

**Next move:** Ghidra-dive the stock app region focused on the input event dispatch; or ask ericlewis for a UX-layer outline; or wait for the "proper BSP" release. Empirically, custom firmware authors can also characterize behavior by recording button presses + audio response on a stock unit.

---

## Second 3.5mm jack (MIDI / PO sync) pin assignment

**Question:** Which nRF GPIO pins drive the MIDI / Pocket Operator sync jack?

**Known:**
- The SP-1 has two 3.5 mm jacks: a TRRS headphone jack on CS42L42 and a second jack supporting MIDI or Pocket Operator synchronization [TKT wiki: Hardware-overview, accessed 2026-05-12]
- `stemplayer_pins.h` does not have a `PIN_MIDI_*` or `PIN_PO_SYNC_*` define
- `stem_player.dts` does not have a labeled MIDI output node
- The MIDI output would need a UART TX (at 31250 baud for standard MIDI) or GPIO bit-bang; Pocket Operator sync uses audio-rate clock pulses on a TRS sleeve channel

**Why unresolved:** No public code targets the second jack. Either TKT hasn't documented the pin mapping yet, or the stock TE firmware uses it via a path that hasn't been called out in the disassembly summaries the community has shared.

**Next move:** Inspect the PCB photo / KiCad reversal to identify which traces leave the FPC and head to the second jack. Or boot stock firmware, probe the second jack with a scope while playing a track, and see which nRF GPIOs change state in sync with audio playback. Or ask TimK in Discord.

---

---

## Effects implementations beyond the basics

**Question:** How do the full tape FX, looping, gate, and LPF presets work in stock firmware?

**Known:**
- Most major effects have been documented at the DSP level [Discord #firmware, ericlewis, 2026-05-09 00:32 + audiothingies code]:
  - Filter: cascaded biquad IIR with LFO modulation (`math_sinf`) + wet/dry crossfade
  - Distortion: pre-gain 16.0 → hard-clip → polynomial waveshaper `x - 0.5·x³` → post-biquad → blend
  - Gate / volume ramp: linear envelope to 0.0 or 1.0 over a block (smooth mute/unmute, not threshold-based)
  - Reverb/Echo: 8192-sample circular delay buffer per stem; always-feeding ("always-listening") design where the buffer records continuously and activation just toggles read-mixing
- TimK noted [Discord #firmware ~2026, paraphrased from cross-referenced Lines posts]: "the embedded timing data is essential for looping, gate, delay, and 2 of LPF presets"
- emvee1968 has gate implemented; virtualflannel_46386 has beat repeat, bit crusher, send delay, filter, pitch shift — but neither has released code yet

**Why unresolved:** The DSP descriptions are at a high level. Exact coefficients, exact tempo synchronization, exact "send delay with performance feedback" behavior, exact bit-crusher quantization details — these are not in the publicly available material. They are inferred from stock firmware behavior.

**Next move:** Wait for virtualflannel_46386 / emvee1968 release. Or read more of `assets/audiothingies-2026-05-09/effects/` (BiquadFilter, DistortionNode, ChorusFlangerNode, DelayEchoNode, GateEnvelopeNode, StemEffectRack). Or ask ericlewis.

---

## High Speed eMMC mode

**Question:** Can the eMMC be run above 32 MHz?

**Known:**
- The Toshiba THGBMNG5D1LBAIL is rated for High Speed mode (52 MHz)
- It works reliably at 32 MHz **without** High Speed mode enabled [code: `assets/storagethingies-2026-05-09/EmmcDriver.cpp`]
- Above 32 MHz: unreliable in TimK's testing
- This is unusual — the eMMC 5.0 spec requires High Speed mode for >26 MHz

**Why unresolved:** Either the chip has a quirk that allows 26-32 MHz without HS mode (and breaks at higher speeds), or TimK's HS mode implementation has a bug. No one has done a full HS-mode bring-up to determine which.

**Next move:** For most use cases, 32 MHz is plenty — the bottleneck is elsewhere. For someone trying to push throughput (e.g., faster uploads or more complex audio FX), this is an open area.

---

## Alternative bootloader modes

**Question:** Are there bootloader entry paths besides Track 1 + Track 4 + USB?

**Known:**
- The Track 1 + Track 4 + USB-C plug-in triggers the bootloader for **60 seconds** [Lines #556, B_E_N]
- The bootloader uses USB CDC for command exchange
- Theoretically the device may support USB DFU as well, and theoretically GPREGRET register settings can force bootloader entry from running firmware (parser 2 boot uses `0x1A96` GPREGRET magic [Discord #general, ericlewis, 2026-05-09 09:45])

**Why unresolved:** No one has fully enumerated bootloader entry conditions. Disassembly work focused on the discovered method.

**Next move:** Ghidra-dive into the bootloader region (read TimK's wiki: `github.com/timknapen/SP-1-dev/wiki/Bootloader`). If exhaustive enumeration matters, ask Duloz (Lines disassembly contributor) or ericlewis.

---

## Hardware variants

**Question:** Are all SP-1 units identical inside, or did TE produce hardware revisions / regional variants?

**Known:**
- A "Paris Saint" batch with alternate artwork exists [Lines, post numbers vary]
- Approximately **859 units** were imported to Ye in September 2020 per Very Good Touring records [Lines #743, maybe (sankebergel in Discord)]
- Total community estimate: 1000–2000 units in the wild
- No systematic firmware-version comparison has been done across multiple variants
- All units examined to date have matched the documented hardware (nRF52840, CS42L42, TAS2505, THGBMNG5D1LBAIL, BQ24232, CYBT-353027-02)

**Why unresolved:** Variants are rare and dispersed; no organized effort to catalog firmware-level differences. tkt1000 commented on 2026-05-10: *"It's incredible that we don't know the full story after all this time"* [Discord #general].

**Next move:** Discord users with non-standard variants should compare firmware dumps. For specific user questions, ask them which variant they have first.

---

## Full original-firmware audio playback engine

**Question:** What exactly does the stock firmware do for audio playback, in code?

**Known:**
- The firmware was dumped by murray (attempt #8504, 2025-01-25, voltage glitch attack on APPROTECT)
- Disassembled in Ghidra by Duloz and others
- High-level behavior is understood: load stems from eMMC, mix with fader weights, apply per-stem effects, send to I2S
- `assets/audiothingies-2026-05-09/` reflects ericlewis's reimplementation of this engine in clean C++17 — *equivalent in behavior* but not a byte-for-byte port

**Why unresolved (depending on what "resolved" means):** The disassembled binary is enormous; specific functions for every transport mode, every effect, the LCD/LED state machine, the menu system — have not all been documented in narrative form. ericlewis's `audiothingies` is the closest thing to a written explanation, plus the Discord conversations.

**Next move:** Read `assets/audiothingies-2026-05-09/AudioEngine.cpp` for the playback model. For deeper behavior matching, run the original firmware in QEMU or on real hardware and observe.

---

## Solderless.engineering source code

**~~Question:~~ RESOLVED 2026-05-13, RE-CONFIRMED 2026-05-18 via re-snapshot.** A community backup surfaced of a complete static snapshot of `solderless-engineering.pages.dev` dated 2026-05-12 and was bundled at `assets/solderless-2026-05-12.zip`. On 2026-05-18 a fresh scrape of the live site (now a multi-app launcher) was bundled at `assets/solderless-2026-05-18.zip`. Source is **still not officially open-sourced** by the Solderless team, but the archived static deployments are unobfuscated/un-minified JS and serve as the canonical protocol reference. See `update-log.md` 2026-05-18 and `references/27-tools-and-utilities.md`.

---

## Sample bit-alignment in the wiki vs `audiothingies`

**Question:** Are decoded samples left-aligned or right-aligned in `int32_t`?

**Known:**
- TKT wiki [Audio-format, accessed 2026-05-12] gives the sample extraction formula:
  ```
  left  = (data[1]<<24) | (data[0]<<16) | (data[3]<<8)
  right = (data[2]<<24) | (data[5]<<16) | (data[4]<<8)
  ```
  This puts the 24-bit value **left-aligned** in `int32_t` (bits 31:8 = sample, bits 7:0 = 0).
- `assets/storagethingies-2026-05-09/DiskManager.hpp::decode_te_frame_payload_i32` and `assets/audiothingies-2026-05-09/PcmPacking.hpp::float_to_pcm_right24_fast` use **right-aligned** representation (bits 23:0 = sample, bits 31:24 = sign extension).
- The **byte assignments agree** in both sources: byte 0 = L_mid, byte 1 = L_msb, byte 2 = R_msb, byte 3 = L_lsb, byte 4 = R_lsb, byte 5 = R_mid.

**Why unresolved:** the two sources express the same data with an 8-bit shift difference. The most likely reconciliation is that the wiki shows an intermediate engine representation while `PcmPacking.hpp` shows the canonical in-memory format used by the mixer — but this hasn't been confirmed. Could also be wiki imprecision.

**Why it matters:** anyone writing a decoder from the wiki formula and feeding it to a mixer expecting right-aligned samples will get audio that's 256× too loud (or, with int saturation, distorted). The functional/byte-level audio data is the same; the only question is the bit shift in the destination word.

**Next move:** read both `assets/storagethingies-2026-05-09/DiskManager.hpp::decode_te_frame_payload_i32` and the wiki page directly (not through WebFetch's summarizer) and reconcile. If still ambiguous, ask ericlewis or TimK whether the engine internally uses left- or right-aligned 24-bit values.

---

## Full byte ordering of the album header

**~~Question:~~ RESOLVED 2026-05-13 via solderless source.** All integer fields are **little-endian**. The encoder writes them via `DataView.setUint32(offset, value, true)` (the `true` flag selects LE in the Web API). Confirmed for: album length (offset 13), song offset (per-entry +0), song length (per-entry +4), all `0x39` chunk header fields (chunk_counter at payload[0..4], emmc_byte_offset at payload[4..8]), tempo (bytes 2042..2043 of each sector). See `working-confirmed.md` 2026-05-13.

---

## Exact encoding of the 16-bit tempo field

**~~Question:~~ PARTIALLY RESOLVED 2026-05-13.** The solderless encoder writes the tempo field as **`tempo = (48000 * 60) / (24 * bpm)`** — a uint16 LE value at bytes 2042..2043 of each sector (end of block 0). For 60 BPM = 2000; 80 BPM = 1500; 120 BPM = 1000. This is a samples-per-something derived count.

The encoder writes only **one tempo value per sector** (at the end of block 0), not per-TE-block as the TKT wiki suggests. Per-block tempo (allowing intra-sector tempo variation) isn't used by the encoder.

**Still open:** the firmware-side semantic interpretation. The value is "samples per X" for some X derived from `24 × bpm` — possibly samples-per-beat-subdivision-at-24-PPQN, or some related musical-time tick. The encoder produces working playback, so the formula is correct as a *write* spec. The *read* spec (how the firmware uses the value internally) is not documented.

**Next move:** the encoder formula is sufficient for producing playable albums. Firmware-side semantics can be deferred unless a custom firmware author needs to consume this field.

---

## "Ladder" ADC values (`t` opcode response)

**Question:** What do the two uint16 values returned by `0x74 't'` represent?

**Known:**
- The opcode returns 4 bytes interpreted as 2× `uint16` LE [code: `assets/solderless-2026-05-18/deviceinfo/js/deviceinfo.js::queryAll` "Ladders" block]
- The deviceinfo app labels them `L1` and `L2` and displays them as live values; it does **not** decode them into named buttons [same file, `pollLive` block]
- The SP-1 hardware uses two "button ladders" — resistor-divider networks where multiple buttons connect through different-value resistors to a single ADC input. The ladder voltage tells the firmware which button is pressed (or which combination, depending on the network topology) [TKT wiki context, plus SP-1's general button-matrix design referenced in `stem_player.dts` `io-channels`]
- `ericlewis/sp1-midi`'s ADC config (`app.overlay`) reads ADC channels 5 and 6 for the two ladders, separate from channels 1–4 used for the faders

**Why unresolved:** the public host code uses the raw ADC values directly and never decodes "which button is pressed by this voltage." The decoding table (voltage threshold → button identity) lives in the stock firmware's input event dispatcher, which has not been disassembled/documented in public artifacts.

**Why it matters (mild):** for a host tool that wants to display "which button is pressed" instead of "which ADC voltage was read," the raw ladder readings aren't directly meaningful. The `0x5C '\\'` button-state opcode already returns decoded button state and is the right opcode for "which buttons are pressed."

**Next move:** for most host tooling, prefer the `\\` opcode and ignore the ladders. For someone reverse-engineering the input layer for a custom firmware, either Ghidra-dive the stock firmware's input dispatcher or measure ladder voltages on real hardware while pressing each button individually. Or ask TimK / ericlewis directly.

---

## Response semantics of `0x62 'b'` (PING)

**Question:** What does the `b` opcode do and what does it return?

**Known:**
- The opcode is defined in the `CMD` enum [code: `assets/solderless-2026-05-18/stemloader/js/stemloader.js` `CMD.PING = 0x62`]
- It is **never called** by any solderless app code; no response handler is registered for it
- Following the `request_char + 1 = response_char` pattern (R→S, T→U, X→Y…), the expected response would be `0x63 'c'` — but this is unverified

**Why unresolved:** the opcode exists in the host's command catalog but is unused, so no observed behavior is documented. The firmware-side handler may or may not exist.

**Next move:** if a use case for PING comes up, send the opcode to a real device in transfer mode and see what comes back. Otherwise this opcode can be ignored.

---

## Response semantics of `0x56 'V'` (FREQ_SWEEP)

**Question:** What payload format does `V` accept, and what is the audio behavior of the sweep?

**Known:**
- The opcode is defined as `CMD.FREQ_SWEEP = 0x56` [code: `assets/solderless-2026-05-18/stemloader/js/stemloader.js`]
- The reply handler treats response cmd `0x57 'W'` as "test tone generated" (string-only log message); no other observable side effect [same file, switch case `'W'`]
- The payload format is not in the JS — solderless does not actually call FREQ_SWEEP with a meaningful payload in any code path
- The opcode's name suggests it triggers an internal frequency sweep through the audio output (presumably for hardware self-test or codec verification)

**Why unresolved:** the host code declares the opcode but never invokes it with parameters. Firmware-side semantics — sweep range, duration, output stem(s), payload format — are not documented in public artifacts.

**Next move:** if a need arises (e.g., a custom-firmware author wants to use this for self-test), reverse-engineer the firmware's handler in Ghidra, or measure audio output while sending the opcode with various payloads.

---

## Reconciliation: Galapagoose's `0-24,489` vs encoder `CLOCK_MAX = 49152`

**Question:** What is the *true* range of the per-sector clock counter?

**Known:**
- **Galapagoose** [Lines #739] described the on-disk timing counter range as "0 to 24,489."
- **The 2026-05-18 solderless encoder** uses `CLOCK_MAX = 49152` as the modulo for its clock accumulator: `clock = (clock + increment) % CLOCK_MAX` [code: `assets/solderless-2026-05-18/stemloader/js/wav-converter.js::encodeToSP1`]
- `49152 = 512 × 96 = 512 bars × 96 ticks/bar`. `24,489 ≈ 24,576 = 256 × 96 = 256 bars × 96 ticks/bar` — but the numbers don't match exactly; `24,489` is **not** half of `49152`.
- Both numbers might be describing different fields entirely: Galapagoose may have measured a stock-firmware **read-side** state variable (one of the 4 per-block sync words in the trailer the wiki documents), while the encoder writes a single **per-sector** clock value at offset 2040.

**Why unresolved:** Galapagoose's source for `24,489` was a Lines post analyzing a real album image; the encoder source is the only known *write* spec. The two haven't been reconciled by anyone publicly. It's possible:
- the stock firmware uses a different counter format for its 4 per-block sync words than solderless writes at the per-sector slot, OR
- Galapagoose mis-counted by a few ticks, OR
- the encoder's `% 49152` modulo represents some other invariant (e.g., a beats-per-album rollover) and the actually-cycled counter is `% 24489` somewhere downstream

**Why it matters (mild):** the encoder produces playable albums on stock firmware, so the formula is correct as a *write* spec regardless of how Galapagoose's reading reconciles. For custom-firmware authors implementing the *read* side (e.g., to drive musically-synced effects), the actual cycled range is what matters.

**Next move:** dump the per-sector clock bytes from a known album image at multiple sectors and chart their values; or ask TimK / Galapagoose directly; or read more of the audio engine in `assets/audiothingies-2026-05-09/AudioEngine.hpp::current_sync_word()` to see what range stock firmware expects.

---

## How to use this file

Before asserting a definitive technical answer about the SP-1, **check this file for the topic.** If it's listed here, respond with: (1) what is known, (2) what is the unresolved gap, (3) who has the next move.

When new information closes a gap, the relevant entry should be **removed from this file** and reflected in the corresponding `references/NN-topic.md` (and optionally added to `corrections.md` if the change replaces a prior belief).
