# Community and Authority Map

**Synthesized through:** Lines #846 (2026-05-06), Discord through 2026-05-11.

A small handful of contributors did most of the deep technical work on the SP-1. Knowing **who is the authority on which topic** saves time and prevents conflating findings. This file maps people to their areas of expertise.

For the broader history of the project, see `01-overview-and-history.md`. For specific findings and citations, see the topic-specific reference files.

## The identity map

Some people use different handles on different platforms. The skill uses the canonical name listed below; when citing a specific source the source's handle is used.

| Canonical | Lines | Discord | GitHub | Notes |
| --- | --- | --- | --- | --- |
| **TimK** | TimK | tkt1000 | timknapen | Firmware lead; PCB schematic; bootloader doc |
| **ericlewis** | (not active on Lines) | ericlewis | ericlewis | Audio engine + storage RE; sp1-midi BSP; libpo32 |
| **murray** | murray | (not active in Discord as of 2026-05-11) | — | APPROTECT glitch attack |
| **JoseJX** | JoseJX | (not active in Discord as of 2026-05-11) | — | eMMC audio format decode |
| **Galapagoose** | Galapagoose | (not active in Discord as of 2026-05-11) | — | Hardware timing / MIDI clock |
| **B_E_N** | B_E_N | (not active in Discord as of 2026-05-11) | — | Bootloader trigger discovery |
| **Duloz** | Duloz | (not active in Discord as of 2026-05-11) | — | Ghidra disassembly |
| **maybe** | maybe | sankebergel | — | Pre-2024 historical timeline (Lines #743) |
| **PedalsandChill** | PedalsandChill | (not active in Discord as of 2026-05-11) | — | OP; thread continuity |
| **nonkillwingman** | nonkillwingman | _isaaczz | — | 3D printed belt clip / accessories |
| **emvee1968** | (banned from Lines) | emvee1968 | (not yet released) | Custom firmware (Claude Code); HT Demucs pipeline |
| **virtualflannel_46386** | (not active on Lines) | virtualflannel_46386 | (not yet released) | Custom OS with extended effects |
| **zee_33** | (banned from Lines) | zee_33 | — | LED-bit-bang firmware dump; Discord booster |
| **fishdog_** | (not active on Lines) | fishdog_ | — | eMMC desolder + USB adapter method |
| **moecal1947** | (not active on Lines) | moecal1947 | (not yet released) | Slow USB album uploader |
| **theunflappable** | (not active on Lines) | theunflappable | — | `test_bootloader.py` (Python bootloader client) |
| **dotjustin** | (banned from Lines as "mcballs") | dotjustin | dot-Justin | Lines thread archive at `sp-1.dotjust.in`; this skill |
| **loksi** | (not active on Lines) | loksiminko | — | `solderless.engineering` team |
| **tunelight** | (not active on Lines) | tunelight | — | `solderless.engineering` team |
| **keebstudios** | (not active on Lines) | keebstudios | — | `solderless.engineering` team |
| **instantjuggler** | instantjuggler | (not in Discord as of 2026-05-11) | — | F&F device origins; early ownership lore |
| **brick** | brick | (not in Discord as of 2026-05-11) | — | Embedded student; firmware analysis contributions |

## Domain authority

Use this when deciding **who to ask** for which question.

### Firmware development

- **TimK** — Zephyr custom firmware, eMMC driver implementation, I²S audio bring-up, USB CDC, bootloader behavior. Owns `github.com/timknapen/SP-1-dev` (docs + wiki).
- **ericlewis** — audio engine internals (mixer, transport, varispeed, effects), storage system architecture, USB upload protocol. Owns `github.com/ericlewis/sp1-midi` (BSP). Has private full-firmware replacement in development.

### Hardware

- **TimK** — PCB schematic reversal (KiCad), test point locations, FPC connector identification (Omron XF3B-1945-31A). Best person to ask about pinouts, board layout, hardware mods.
- **Galapagoose** — debug header mapping, power delivery analysis, RF/antenna considerations.

### Reverse engineering

- **murray** — APPROTECT bypass methodology, BeagleBone PRU automation, glitch attack debugging.
- **Duloz** — Ghidra workflow, privilege mode analysis, bootloader region disassembly.
- **elew** — parallel test infrastructure (5 units, glitch attempt scaling).

### Audio format

- **JoseJX** — sector / chunk layout, PCM frame decoding, timing metadata interpretation (early work).
- **Galapagoose** — MIDI clock encoding (0xF8, 31250 baud, 128 µs pulse) finalization.
- **ericlewis** — modern authoritative reference via `assets/audiothingies-2026-05-09/` code.

### Bootloader

- **B_E_N** — discovered the Track 1 + Track 4 + USB-C trigger.
- **TimK** — documentation: `github.com/timknapen/SP-1-dev/wiki/Bootloader`.
- **ericlewis** — opcode-level command details (shared in Discord #general 2026-05-09).

### Bluetooth

- **emvee1968** — only public claim of working BT pairing (vol+/vol- combo, in unreleased firmware).
- TimK has not investigated BT. ericlewis hasn't either.

### Physical modifications

- **steellll** — anodized case mod (planned, in progress).
- **fishdog_** — eMMC desolder + USB adapter procedure (380–420 °C, 175 °C preheater).
- **nonkillwingman / _isaaczz** — 3D printed accessories.
- **walkerauga** — KOII compatibility plate.

### Historical context

- **maybe** (sankebergel on Discord) — definitive pre-2024 timeline in Lines #743. Authoritative on F&F origins, 2018-2020 development, Ye / Kano collaboration.
- **instantjuggler** — early SP-1 owner; first-hand testimony on 2019-2020 device experience.
- **_pete** — Kano / TE insider perspective; industry context on why the device was shelved.

### Tools & ecosystem

- **solderless.engineering** team (TimK + loksi + tunelight + keebstudios) — web-based updater.
- **dotjustin (this user)** — Lines thread archive (`sp-1.dotjust.in`); maintains this skill.
- **theunflappable** — `test_bootloader.py`.
- **moecal1947** — Python USB uploader (slow, working).

### Music / audio production

- **emvee1968** — HT Demucs FT stem separation pipeline for custom album content.

## Specific person-to-topic lookup

When you need to know who said what:

- *"How does the eMMC driver work?"* → ericlewis (`assets/storagethingies-2026-05-09/EmmcDriver`) or TimK (origin of the implementation)
- *"Where are the SWD pads?"* → TimK (his wiki, pending update)
- *"What does the 32-bit sync word encode beyond the counter?"* → ericlewis or Galapagoose
- *"How do I dump my SP-1's firmware?"* → murray (glitch attack) or zee_33 (LED bit-bang)
- *"How do I solder onto the eMMC chip?"* → fishdog_
- *"Where's the source for the slow uploader?"* → moecal1947
- *"How do I trigger the bootloader?"* → B_E_N's Lines #556 + ericlewis's Discord opcode dump
- *"How do I implement Bluetooth?"* → emvee1968 (unreleased) or your own work from scratch
- *"What custom firmware should I use?"* → ericlewis's `sp1-midi` (the only public BSP currently)
- *"What's the history of the device?"* → maybe (sankebergel) Lines #743

## Why this map matters

When asking questions in the Discord, addressing the right person saves time and gets better answers. TimK is generally happy to help but is also busy; ericlewis has been very generous with technical details but is "quite busy" too [Discord #firmware, 2026-05-09 00:20]. Direct, specific questions to the right person work better than general "can someone help me with X?" messages.

When *citing* findings in code or documentation, citing the right person credits the actual work. Don't credit TimK for ericlewis's audio engine, and don't credit murray for Duloz's Ghidra analysis. The community has worked hard on this; correct attribution matters.

## Where to go next

- For the project's overall history → `01-overview-and-history.md`
- For specific findings by topic → topic-specific reference files
- For where each named tool lives → `27-tools-and-utilities.md`
- For asking TimK about things that aren't yet documented → `cc-skill/questions-for-timk.md` (skill-development workflow)
