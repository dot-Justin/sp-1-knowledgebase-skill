# Overview and History

**Synthesized through:** Lines #846 (2026-05-06), Discord through 2026-05-11.

## What the SP-1 is

The **Teenage Engineering SP-1** (model name *Stem Player SP-01*) is a small handheld stem player prototype designed by Teenage Engineering, originally intended for distribution alongside Kanye West's *Donda* and *Jesus Is King* albums. It was never released to retail. Devices reached the public through artist gifts, leaks, and secondary-market resale.

Internally it is a four-stem audio playback device built around the Nordic Semiconductor nRF52840 SoC with a custom 4 GB eMMC storing 8-channel, 24-bit, 48 kHz audio. Four faders control per-stem volume. Four "track" buttons mute/solo per stem. A volume rocker, a fast-forward / rewind rocker, a play button, and a function button complete the UI. Bluetooth is wired but not used by stock firmware. Audio is output via TRRS headphone jack (CS42L42 codec) and an internal mono speaker (TAS2505 amplifier).

The device runs a custom Teenage Engineering firmware with sophisticated DSP effects (filter, distortion, gate, echo, tape FF/RW) that exploit the on-disk audio interleaving to do "time stretching" without the CPU having to do real-time DSP at the data rates the chip would otherwise require.

## Lineage

| Year | Event |
| --- | --- |
| ~2018 | Initial concept work between TE founder Jesper Kouthoofd and Kanye West |
| 2019 | Kano collaboration; CES meeting; *Jesus Is King* released on TE devices |
| Sep 2020 | Very Good Touring imports ~859 units as "Stemplayer speaker with battery" — these go to Ye and his circle [Lines #743, maybe] |
| ~2020 | Kano version released to retail; TE version shelved |
| 2024-04 | First public Lines forum acquisition + teardown thread opens [Lines #1, PedalsandChill] |
| 2024-07 | Glitch attack campaign begins; murray publishes BeagleBone PRU glitcher |
| 2024-12 | JoseJX decodes the basic eMMC audio format: 8 channels, 24-bit, 48 kHz |
| 2025-01-25 | **Firmware extracted** — murray bypasses APPROTECT on the nRF52840, attempt #8,504, voltage glitch attack |
| 2025-02–04 | Ghidra disassembly (Duloz et al.); PCB schematic reversed (TimK) |
| 2025-04 | B_E_N discovers the **Track 1 + Track 4 + USB-C** bootloader trigger [Lines #556] |
| 2025-05 | TimK loads custom stems on real hardware [Lines #499] |
| 2025-06 | **solderless.engineering** launches — web-based firmware updater requiring no soldering |
| 2025-09 | Galapagoose confirms the MIDI-clock encoding: `0xF8` at 31,250 baud, 128 µs pulse, 4 × 24 PPQN [Lines #739] |
| 2026-01 | TimK confirms **custom stems play on stock TE firmware** (no firmware modification required) [Lines #799–805] |
| 2026-05-06 | Lines moderators close the thread over copyright/spam concerns; community moves to Discord (still open) |
| 2026-05-08 | zee_33 demonstrates LED-bit-banged firmware dump via a 1-page custom firmware [Discord #firmware] |
| 2026-05-08 | emvee1968 demos custom firmware built with Claude Code Opus 4.7 — basic playback, gate effect, working Bluetooth pairing [Discord #firmware] |
| 2026-05-09 | ericlewis publishes `sp1-midi` Zephyr BSP on GitHub (MIDI-controller mode); shares C++ audio engine + storage driver reference implementations in Discord |
| 2026-05-09 | `solderless.engineering` taken offline for an update [Discord #news, tkt1000] |
| 2026-05-11 | virtualflannel_46386 demos custom OS with full FX page (beat repeat, bit crusher, send delay, filter, pitch ±12 cents, mixer) — awaiting solderless return to release [Discord #general] |

## Who's who

A small handful of contributors did most of the deep work. Knowing who to trust on which topic saves a lot of time.

| Name | Areas of authority | Where to cite |
| --- | --- | --- |
| **TimK / tkt1000 / timknapen** | Firmware development (Zephyr custom firmware, eMMC driver, I2S audio, USB CDC), PCB schematic reversal, bootloader documentation | Owns `github.com/timknapen/SP-1-dev`; active in Discord |
| **ericlewis** | Audio engine internals (mixer, effects, tape FX), storage system, USB upload protocol, MIDI controller BSP | Owns `github.com/ericlewis/sp1-midi`; active in Discord |
| **murray** | APPROTECT glitch attack, BeagleBone PRU automation | Lines thread; not active in Discord as of synthesis |
| **JoseJX** | Audio format decoding (sectors, chunks, PCM layout), timing metadata interpretation | Lines thread |
| **Galapagoose** | Hardware timing, MIDI protocol, PCB analysis | Lines thread |
| **B_E_N** | Bootloader trigger (Track 1 + Track 4) | Lines #556 |
| **Duloz** | Ghidra disassembly, privilege mode, bootloader region analysis | Lines #205–240 |
| **maybe / sankebergel** | Pre-2024 historical timeline (post #743) | Lines #743 |
| **emvee1968** | Custom firmware built with Claude Code Opus 4.7 (unreleased) | Discord, May 2026 |
| **virtualflannel_46386** | Custom OS with full FX page (unreleased) | Discord, May 2026 |
| **zee_33** | LED-bit-banged firmware dump via minimal custom firmware | Discord, May 2026 |
| **fishdog_** | eMMC chip desolder + USB-to-eMMC adapter method | Discord #hardware, May 2026 |
| **moecal1947** | Python USB uploader (slow but working) | Discord, May 2026 |
| **dotjustin** | Lines thread archive at `sp-1.dotjust.in`; this skill | Discord, May 2026 |
| **loksi, tunelight, keebstudios** | solderless.engineering web updater team | Credit by tkt1000 in Discord #general, 2026-05-06 |

For the canonical identity map across handles (TimK = tkt1000 = timknapen; sankebergel = maybe; etc.), see `synthesis-log.md`.

## Why the Lines thread closed

Per tkt1000 [Discord #general, 2026-05-08]:

> They told me they had too much work cleaning up the thread because of people spamming. A higher volume than what they're used to.

The trigger was a combination of:
- Increased posting volume after the device hit minor mainstream awareness
- Requests for firmware binary / album file dumps (against Lines' ToS due to Kanye-album copyright)
- Concern about AI/scraping (mentioned but apparently not the primary reason)

Lines did not delete the thread. It is still readable. They also added a direct link to the new Discord [tkt1000, Discord #general, 2026-05-08]. The community moved to Discord and momentum has accelerated — the discord saw two custom firmware demos and a sophisticated USB-upload effort all within its first week.

## Why this is interesting work

A few things make the SP-1 a uniquely good reverse-engineering target:

- **Sealed device with consumer-grade hardware** — no debug header exposed, but the APPROTECT bypass is a known glitch attack
- **Plausible cleanroom firmware path** — once the format is documented, custom firmware can be written from scratch without copying TE code
- **Compelling artifact** — the device is well-designed, has a vibrant community, and has cultural cachet from its Kanye/TE association
- **Composability** — `solderless.engineering` (web updater) + ericlewis's BSP + TimK's hardware docs + custom audio formats means a developer can pick which layer to work at without rebuilding the stack

## What this skill assumes

- You are working with a real SP-1 (or planning to)
- You are willing to read C++ and device-tree source
- You will not redistribute TE's binary firmware or Kanye album stems
- You are comfortable with the risk that custom firmware can brick your device

If any of those assumptions doesn't fit, the relevant reference file should say so.

## Where to go next

- For chip-level summary → `02-hardware-overview.md`
- For board layout / PCB / FPC → `03-pcb-and-schematic.md`
- For SWD / USB CDC / test points → `04-debug-interfaces.md`
- For the audio format on the eMMC → `09-audio-format-spec.md`
- For setting up a build environment → `18-zephyr-build-environment.md`
- For the bootloader trigger and protocol → `15-bootloader-protocol.md`
- For "is X actually working?" → `working-confirmed.md` (top level)
- For "is X a known-unknown?" → `known-unknowns.md` (top level)
