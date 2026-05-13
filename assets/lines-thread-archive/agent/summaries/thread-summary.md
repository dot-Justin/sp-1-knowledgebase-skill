# TE Stem Player - Thread Summary

**Thread:** "TE Stem Player" on llllllll.co (Lines forum)  
**Posts:** 846 | **Participants:** 179 | **Views:** 78,451 | **Likes:** 4,967  
**Date Range:** 2024-04-09 to 2026-05-06  
**Status:** Thread closed by moderators (IP concerns); community moved to Discord

---

## What This Thread Is

A community reverse-engineering effort for the **Teenage Engineering Stem Player prototype** (TE-SP-1) - a limited device released in collaboration with Ye (Kanye West) for the *Donda* and *Jesus Is King* albums. The thread spans from the first public acquisition of a unit through full firmware extraction, audio format decode, custom firmware, and a web-based bootloader updater - without ever opening the device.

---

## Hardware

| Component | Part |
|-----------|------|
| SoC | Nordic Semiconductor nRF52840 |
| Audio codec | Cirrus Logic CS42L42 |
| Amp | Texas Instruments TAS2505 |
| eMMC | Toshiba THGBMNG5D1LBAIL (4GB) |
| FPC connector | Omron XF3B-1945-31A (19-pin, 0.3mm pitch) |

The nRF52840 has **APPROTECT** enabled - a hardware read-protection bit that blocks SWD debug access and firmware extraction. Bypassing it requires a voltage/crowbar glitch attack during boot.

---

## Timeline of Breakthroughs

### April–June 2024 - Hardware Identification
- PedalsandChill acquires unit, posts teardowns; community identifies all ICs
- APPROTECT confirmed; first glitch attempts begin (limitedresults technique)
- "Paris Saint" batch discovered - alternate artwork/units revealed

### July–December 2024 - Glitch Campaign
- murray builds automated BeagleBone Black PRU glitcher ([resinbeard/beaglebone-black-glitcher](https://github.com/resinbeard/beaglebone-black-glitcher))
- elew joins with 5 units running parallel attempts
- JoseJX begins decoding eMMC audio format: 8K sectors, 2K chunks, 24-bit 48kHz PCM, 8 interleaved channels (4 stereo stems)

### January 25, 2025 - Firmware Dumped
- murray bypasses APPROTECT on attempt **#8,504** and extracts full nRF52840 firmware
- Firmware analyzed in Ghidra with SVD overlay; audio decoded as listenable files

### February–April 2025 - Firmware Analysis & PCB Reverse Engineering
- Duloz finds privilege mode checks and bootloader hints in disassembly
- TimK joins; reverse-engineers PCB to full KiCad schematic
- I2S audio, all buttons, LEDs, and USB CDC confirmed working in custom build
- B_E_N discovers: hold Track 1 + Track 4 while plugging USB → CDC serial bootloader mode for 60 seconds

### May 2025 - Custom Stems Loaded (Post 499)
- TimK loads his own audio onto the device - custom stems play on real hardware
- eMMC driver stable at 32MHz; 8-channel WAV format chosen for new firmware

### June–August 2025 - Bootloader Cracked / solderless.engineering
- **solderless.engineering** launches: web-based updater using the Track1+4 bootloader exploit
- No device opening required - flash new firmware via USB-C
- SP-1-dev GitHub repo published: [timknapen/SP-1-dev](https://github.com/timknapen/SP-1-dev)
- Community poll: 120 responses, 97% want custom stem support

### September–December 2025 - Audio Format Fully Documented
- Timing metadata decoded: 16-bit counter, values 0–24,489, steps of 96
- 96 steps = 4 × 24 PPQN = MIDI clock encoding
- Galapagoose confirms: 0xF8 MIDI clock = 128μs pulse at 31,250 baud
- Complete audio format documented in GitHub wiki

### January–May 2026 - Custom Stems on Original Firmware
- TimK confirms: custom stems playable on **unmodified TE firmware** (no custom FW needed)
- Full audio format spec published; timing format fully understood
- Thread closed by Lines moderators; community redirected to Discord

---

## Audio Format (eMMC)

- **Sector size:** 8,192 bytes
- **Chunk size:** 2,048 bytes  
- **Encoding:** 24-bit PCM, 48kHz
- **Channels:** 8 interleaved (4 stereo stems: drums, bass, melody, vocals)
- **Timing metadata:** 16-bit counter embedded, 96 steps per cycle, 4×24 PPQN MIDI encoding
- **MIDI clock:** 0xF8 pulse at 31,250 baud, 128μs interval

---

## Key Participants

| Handle | Role |
|--------|------|
| PedalsandChill | OP; acquired 2 units; teardowns; thread maintenance |
| murray | Led firmware glitch campaign; BeagleBone PRU automation |
| TimK | Firmware dev; PCB schematic; SP-1-dev GitHub; custom stems |
| Galapagoose | PCB analysis; MIDI clock decode |
| JoseJX | Audio format decode (sectors, chunks, PCM spec) |
| elew | 5 units; parallel glitch attempts |
| B_E_N | Discovered Track1+4 CDC bootloader combo |
| Duloz | Ghidra analysis; bootloader hints; privilege mode |
| instantjuggler | Long-time owner; videos; lore; user guide |
| brick | Embedded student; analysis contributions |
| maybe | Wrote comprehensive historical timeline (post 743) |
| nonkillwingman | 3D-printed belt clip and case |
| ItDitry | 3D-printed scale model |
| dotJustin | This archive's creator |

---

## Current Status (as of thread close, May 2026)

- Custom stems work on original TE firmware (no hack required)
- solderless.engineering updater live for bootloader flashing
- SP-1-dev GitHub repo active for custom firmware development
- Thread closed by Lines moderators; all activity moved to Discord

---

## Key Links

| Resource | URL |
|----------|-----|
| Discord | discord.gg/y4V6VfHYck |
| Web updater | solderless.engineering |
| Custom firmware repo | github.com/timknapen/SP-1-dev |
| BeagleBone glitcher | github.com/resinbeard/beaglebone-black-glitcher |
| Historical timeline | Post 743 (by maybe) |
| Belt clip (3D print) | thingiverse.com/thing:7312090 |
| Scale model (3D print) | thingiverse.com/thing:7312595 |
| Magnetic case (3D print) | thingiverse.com/thing:7313465 |
| Track1+4 bootloader | Post 556 (by B_E_N) |
| Firmware dump (post) | Post ~200 area (murray, Jan 25 2025, attempt #8504) |
| Custom stems success | Post 499 (TimK) |
