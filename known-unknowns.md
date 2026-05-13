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
- The module is wired to the nRF52840 via UART (TX P1.02, RX P1.04, RTS P1.01, CTS P1.03, Reset P0.10) [code: `sp1-midi/boards/.../stem_player.dts`]
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
- ericlewis's `sp1-midi` BSP has `PowerManager.cpp` (subsys/power) with battery/charger/USB monitoring [code: `ericlewis/sp1-midi/subsys/power/`]
- ericlewis: *"ALWAYS REMEMBER TO PROGRAM IN YOUR POWER DOWN"* [Discord #firmware, ericlewis, 2026-05-09 01:02]

**Why unresolved:** The PowerManager in `sp1-midi` exists but isn't wired up by the example MIDI controller app to handle the "off" state. Custom firmware authors have to integrate it themselves and verify nRF52840 SYSTEM_OFF behavior with the BQ24232 charger and TAS2505/CS42L42 codec power lines.

**Next move:** Read `ericlewis/sp1-midi/subsys/power/PowerManager.cpp` and study `CONFIG_POWEROFF=y` Zephyr APIs. If still stuck, ask ericlewis.

---

## BQ24232 battery charger driver completeness

**Question:** How complete is the charger driver in the public BSP, and what's needed for production use?

**Mostly resolved as of 2026-05-12** following TKT wiki Battery-charger page incorporation:

- `sp1-midi/drivers/charger/charger_bq24232.c` exists with GPIO-based control:
  - nPGOOD = P0.24 (open-drain, low when USB power good)
  - nCHG = P0.22 (open-drain, low while charging)
  - CE = P0.21 (active-low charge enable)
  - **ISET = P1.00** (corrected from earlier erroneous "P0.19" — see `corrections.md`)
  - BATT_LEVEL = P0.28 / AIN4
  [code: `ericlewis/sp1-midi/boards/teenageengineering/stem_player/stem_player.dts`; `timknapen/SP-1-dev/src/stemplayer_pins.h`]
- **Maximum USB current draw: 500 mA** [TKT wiki: Battery-charger, accessed 2026-05-12]
- ISET voltage reflects actual charging current; can be monitored via a voltage divider on AIN [TKT wiki: Battery-charger, accessed 2026-05-12]

**Still open:** The specific ISET resistor values that determine the fast/slow charge currents are on the schematic but not in code or wiki. For most firmware work the existing driver is "complete enough."

**Next move:** For deep customization (e.g., implementing fast/slow charge mode switching), measure ISET voltage on real hardware while toggling the override pin, or consult TimK's KiCad reversal for resistor values.

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

**Next move:** Wait for virtualflannel_46386 / emvee1968 release. Or read more of `audiothingies/effects/` (BiquadFilter, DistortionNode, ChorusFlangerNode, DelayEchoNode, GateEnvelopeNode, StemEffectRack). Or ask ericlewis.

---

## High Speed eMMC mode

**Question:** Can the eMMC be run above 32 MHz?

**Known:**
- The Toshiba THGBMNG5D1LBAIL is rated for High Speed mode (52 MHz)
- It works reliably at 32 MHz **without** High Speed mode enabled [code: `storagethingies/EmmcDriver.cpp`]
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
- `audiothingies/` reflects ericlewis's reimplementation of this engine in clean C++17 — *equivalent in behavior* but not a byte-for-byte port

**Why unresolved (depending on what "resolved" means):** The disassembled binary is enormous; specific functions for every transport mode, every effect, the LCD/LED state machine, the menu system — have not all been documented in narrative form. ericlewis's `audiothingies` is the closest thing to a written explanation, plus the Discord conversations.

**Next move:** Read `audiothingies/AudioEngine.cpp` for the playback model. For deeper behavior matching, run the original firmware in QEMU or on real hardware and observe.

---

## Solderless.engineering source code

**~~Question:~~ RESOLVED 2026-05-13 via local archive.** A community backup surfaced of a complete static snapshot of `solderless-engineering.pages.dev` dated 2026-05-12. Archive is at `assets/solderless-2026-05-12.zip`. Source is **not officially open-sourced** by the Solderless team, but the archived static deployment is unobfuscated/un-minified JS and serves as the canonical protocol reference. See `update-log.md` 2026-05-13 and `references/27-tools-and-utilities.md`.

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
- `storagethingies/DiskManager.hpp::decode_te_frame_payload_i32` and `audiothingies/PcmPacking.hpp::float_to_pcm_right24_fast` use **right-aligned** representation (bits 23:0 = sample, bits 31:24 = sign extension).
- The **byte assignments agree** in both sources: byte 0 = L_mid, byte 1 = L_msb, byte 2 = R_msb, byte 3 = L_lsb, byte 4 = R_lsb, byte 5 = R_mid.

**Why unresolved:** the two sources express the same data with an 8-bit shift difference. The most likely reconciliation is that the wiki shows an intermediate engine representation while `PcmPacking.hpp` shows the canonical in-memory format used by the mixer — but this hasn't been confirmed. Could also be wiki imprecision.

**Why it matters:** anyone writing a decoder from the wiki formula and feeding it to a mixer expecting right-aligned samples will get audio that's 256× too loud (or, with int saturation, distorted). The functional/byte-level audio data is the same; the only question is the bit shift in the destination word.

**Next move:** read both `storagethingies/DiskManager.hpp::decode_te_frame_payload_i32` and the wiki page directly (not through WebFetch's summarizer) and reconcile. If still ambiguous, ask ericlewis or TimK whether the engine internally uses left- or right-aligned 24-bit values.

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

## How to use this file

Before asserting a definitive technical answer about the SP-1, **check this file for the topic.** If it's listed here, respond with: (1) what is known, (2) what is the unresolved gap, (3) who has the next move.

When new information closes a gap, the relevant entry should be **removed from this file** and reflected in the corresponding `references/NN-topic.md` (and optionally added to `corrections.md` if the change replaces a prior belief).
