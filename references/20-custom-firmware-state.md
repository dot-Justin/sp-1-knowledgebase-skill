# Custom Firmware State

**Synthesized through:** Lines #846 (2026-05-06), Discord through 2026-05-11.

This file enumerates the **custom firmwares** known to exist for the SP-1, their status (released / unreleased), what they do, and who's building them. Last updated 2026-05-11; the Discord is active and this list may need refreshing.

## tl;dr

| Firmware | What it does | Public? | Authoritative source |
| --- | --- | --- | --- |
| **Stock TE firmware** | Plays original Donda / JIK stems with full effects | Private (not redistributed) | Reverse-engineered from murray's 2025-01-25 dump |
| **`ericlewis/sp1-midi`** | USB MIDI 2.0 controller (faders/buttons → MIDI) | **Public on GitHub** | `github.com/ericlewis/sp1-midi` (2026-05-09) |
| **TimK's private firmware** | "Fully functional" — details not public | Private; tested with update utility | tkt1000, Discord #hardware, 2026-05-08 |
| **emvee1968's firmware** | Basic playback + gate effect + Bluetooth pairing | Unreleased (awaiting 2 more units) | emvee1968, Discord #firmware, 2026-05-08 |
| **virtualflannel_46386's custom OS** | Full FX page: beat repeat, bit crusher, delay, filter, pitch ±12 cents, mixer | Unreleased (awaiting solderless return) | virtualflannel_46386, Discord #general, 2026-05-11 |
| **`audiothingies` / `storagethingies`** | Reference implementations of stock audio engine + storage | **Bundled in skill** at `assets/audiothingies-2026-05-09/` and `assets/storagethingies-2026-05-09/` (originally Discord attachments) | ericlewis, Discord #firmware, 2026-05-09 |
| **`theunflappable`'s `test_bootloader.py`** | Bootloader protocol implementation (host-side) | Privately shared in Discord | theunflappable, Discord #firmware, 2026-05-08 |
| **moecal1947's Python uploader** | Slow but working USB album uploader | Privately shared / in development | moecal1947, Discord #general, 2026-05-09 |
| **zee_33's LED-bit-bang dump firmware** | Minimal firmware that exfiltrates flash bytes via LEDs | Privately developed; not yet published | zee_33, Discord #firmware, 2026-05-08 |

## Stock TE firmware

The original firmware that ships with every SP-1. Extracted by murray on 2025-01-25 via voltage glitch attack (`14-approtect-glitch-attack.md`). **Not publicly redistributed** — community treats it as TE's IP. Reverse-engineering produced public artifacts (`assets/audiothingies-2026-05-09/`, `assets/storagethingies-2026-05-09/`, the bootloader documentation) but the binary itself stays in private hands.

What it does:
- Plays the bundled album (Donda or Jesus Is King depending on the unit's batch)
- Full DSP effects: filter, distortion, gate, chorus/delay, echo
- Tape FF/RW up to **2.5x speed** measured (per tkt1000, Discord #firmware, 2026-05-09 07:04)
- Bluetooth audio output (presumed; not formally verified by the community)
- LED animations synced to embedded sync words

If you have an unmodified SP-1, this is what you have. Flashing any custom firmware overwrites it; you lose the stock experience unless you have a copy of the original binary.

## `ericlewis/sp1-midi`

**The only currently public custom firmware.** Published 2026-05-09 by ericlewis. Zephyr-based, builds with the standard Zephyr SDK. See `19-sp1-midi-bsp.md` for the walkthrough.

What it does:
- USB MIDI 2.0 (UMP) controller
- Faders → CC 1-4
- Track buttons → notes 60-63 (with rocker octave shift)
- Volume rocker → CC 7
- Play/Function buttons → MMC commands
- CDC ACM serial console for debugging
- Power management + watchdog

What it doesn't do:
- Play audio
- Use Bluetooth
- Implement effects

It's the **BSP** — the foundation other custom firmwares should build on. Currently the only buildable starting point for new firmware projects.

## TimK's private custom firmware

tkt1000 has multiple custom firmwares he's tested with the update utility [Discord #hardware, 2026-05-08 14:49]:

> I've tested a bunch of different firmwares with the update utility, but haven't made it public yet

> It's fully functional. But I'm working on something more important now that we want to release first. Close to being ready now

The "something more important" is unclear from the synthesis-date material but is presumably either:
- An updated `solderless.engineering` web tool (consistent with the 2026-05-09 offline notice)
- A more complete Zephyr BSP that includes audio playback
- A bootloader update / patch

When TimK does publish, expect a clean, well-documented codebase similar in style to `SP-1-dev`. He confirmed his custom firmware can do **up to 2.0x FF** [Discord #firmware, tkt1000, 2026-05-09 07:04 — *"I got FFWD only up to 2X after that you need to jump eMMC blocks"*]. Full FF/RW with block-skipping appears to be aspirational.

## emvee1968's firmware

Built with **Claude Code (Opus 4.7)** over about a week [Discord #firmware, emvee1968, 2026-05-08 23:24]:

> I was able to develop custom firmware that replicates the basic functionality of the stock firmware… to be fully transparent, utilizing Claude Code Opus 4.7 after many iterations and about a weeks worth of time

Demonstrated on YouTube (link: `https://youtube.com/shorts/aQGvrUvXvR4`).

Features (as of 2026-05-08):
- Basic playback of custom stems
- **Gate effect** (only one of the stock effects implemented so far)
- **Working Bluetooth pairing** via vol+/vol- button combo [Discord #firmware, emvee1968, 2026-05-08 23:42]
- Custom stems via **HT Demucs FT** stem separation [Discord #firmware, 2026-05-08 23:54]
- Tape half-speed effect via function + play button combo

What's broken / incomplete:
- **No power-off function** — requires disconnecting the battery to re-enter the bootloader [Discord #firmware, emvee1968, 2026-05-08 23:56]
- **No FF/RW** — emvee1968 spent hours trying and gave up; ericlewis later pointed at the block-skipping technique [Discord #firmware, 2026-05-09 00:04]
- **Other effects (filter, distortion, chorus/delay) not yet implemented**

Release plan: emvee1968 is awaiting **two more SP-1 units** to dev safely against, then plans to release the code [Discord #firmware, 2026-05-08 23:56]:

> I am awaiting 2 more sp-1s to dev with which I should get next week.. then I plan on releasing code/repo.. right now I have no functionality to turn off the device.

Expected release: week of 2026-05-12 or later. Not certain.

## virtualflannel_46386's custom OS

Most ambitious unreleased firmware. Demonstrated 2026-05-11 in Discord #general via YouTube link `Tp_XwUd4rPs`:

Features [Discord #general, virtualflannel_46386, 2026-05-11 17:56]:

> there is an fx page with beat repeat, bit crusher, send delay with performance feedback, and filter with performance modulation. You can pitch songs up and down 12cents. Mixer has mutes and solo.

Detailed feature list (as described):
- **Beat repeat** (sample loop / stutter effect)
- **Bit crusher** (bit-depth + sample-rate reduction)
- **Send delay** with "performance feedback" — likely modulated feedback gain via fader or button
- **Filter** with "performance modulation" — interactive filter sweep
- **Pitch shift ±12 cents** (microtonal pitch adjustment)
- **Mixer with mutes and solo** per stem

This extends beyond the stock effect set — beat repeat and bit crusher are not in the original TE firmware effect rack as documented in `audiothingies`. virtualflannel has built new effect modules.

Release plan [Discord #general, virtualflannel_46386, 2026-05-11 17:56]:

> Thanks everyone, once the update tool is back online I will share the firmware and full documentation.

Waiting for `solderless.engineering` to return online. ETA depends on tkt1000's update.

## `assets/audiothingies-2026-05-09.zip` and `assets/storagethingies-2026-05-09.zip`

Shared by ericlewis in Discord #firmware on 2026-05-09 (00:18:55 and 00:21:46 UTC) as private reference code. Headers and sources for:

- `audiothingies`: AudioEngine, StockRuntimeMixer, VarispeedResampler, StemEffectRack, all 5 effect nodes, PcmPacking, Zephyr I²S TX backend
- `storagethingies`: EmmcDriver, DiskManager, StemEmmcDevice, audio frame decoder

These are **clean-room reimplementations** of the stock TE audio engine and storage stack. ericlewis describes them as:

> this MIGHT be helpful [Discord #firmware, 2026-05-09 00:18]

> im in the middle of a proper bsp for zephyr for everyone but quite busy 🙁 [Discord #firmware, 2026-05-09 00:20]

So a **public Zephyr BSP with full audio engine** is in development by ericlewis. The `sp1-midi` MIDI-controller BSP is presumably a precursor; the "proper bsp" with audio is the eventual deliverable.

When the proper BSP ships, the `audiothingies` and `storagethingies` code will likely be incorporated (or replaced by a refined version). The current zips are useful as reference documentation right now.

## moecal1947's Python USB uploader

Working but slow (~0.75 KB/s) album uploader [Discord #general, moecal1947, 2026-05-09 09:33]. Architecture:

- Custom firmware on the SP-1 that exposes a USB vendor interface
- Python script on the host that sends one 512-byte eMMC block at a time
- Each block is staged in tiny USB control transfers
- Per-block checksum verification + range-based resume

Awaiting CDC packet framing details from ericlewis to switch to the proper bulk-endpoint architecture for ~4 MB/s speed. See `16-usb-upload-protocol.md`.

## `theunflappable`'s `test_bootloader.py`

A Python implementation of the bootloader protocol [Discord #firmware, theunflappable, 2026-05-08 00:49]:

> Wanted to share a little Python script based on the web firmware updater that implements the bootloader protocol.

Useful as a starting point for writing your own bootloader client when `solderless.engineering` is offline. Does **not** include album upload. Just firmware flashing via the bootloader CDC.

## zee_33's LED-bit-bang dump firmware

Custom firmware (~1 page = 4 KB) that exfiltrates flash bytes via the LEDs. Used to dump the bootloader region without a glitch attack. See `17-led-bitbang-dump.md`. Not publicly published as of synthesis date.

## What's still missing

| Feature | Status |
| --- | --- |
| Public BSP with audio playback | In development by ericlewis (the "proper bsp") |
| Public BSP with Bluetooth | emvee1968 has it working but unreleased |
| Public BSP with all stock effects | virtualflannel has 4 custom effects; nobody has full stock parity yet |
| Public album-upload tool with reasonable speed | Awaiting solderless.engineering return + packet-framing publication |
| Stem-prep workflow tool (WAV → album.sp1) | emvee1968 has internal code; not released |
| Standalone solderless replacement | theunflappable has bootloader portion; album upload portion in development |

## Watch for Updates

The Discord is moving fast. emvee1968, virtualflannel_46386, ericlewis, and moecal1947 are all actively developing. Expect significant releases in May-June 2026. This file should be re-synthesized periodically.

## Where to go next

- For the public BSP (today's starting point) → `19-sp1-midi-bsp.md`
- For the build environment → `18-zephyr-build-environment.md`
- For the audio engine code (in private zips for now) → `12-audio-engine-internals.md`
- For the upload protocol → `16-usb-upload-protocol.md`
- For canonical stem prep status → `21-original-firmware-stems.md`
- For known-unknowns including release timing → `known-unknowns.md`
