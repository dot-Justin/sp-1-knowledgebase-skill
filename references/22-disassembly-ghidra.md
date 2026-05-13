# Disassembly and Ghidra

**Synthesized through:** Lines #846 (2026-05-06), Discord through 2026-05-11.

After murray dumped the firmware on 2025-01-25, the community loaded it into **Ghidra** with the **nRF52840 SVD register file** as an overlay. **Duloz** led the disassembly work alongside several others. This file documents methodology, key findings, and how to set up the same environment if you want to verify or extend the analysis.

## Tools

| Tool | Purpose |
| --- | --- |
| **Ghidra** | NSA-published reverse engineering framework. Free, open source, supports ARM Cortex-M out of the box. |
| **nRF52840 SVD file** | Cortex Microcontroller Software Interface Standard description of the chip's peripheral registers. Loaded as Ghidra overlay so register reads/writes show up as named symbols. |
| **SVD-Loader-Ghidra** (plugin) | Imports the SVD into Ghidra. Available on GitHub. |
| **The firmware binary** | murray's January 2025 dump. Not publicly redistributed but circulates in private community. |

You'll also want **Nordic's `nrf52840_svd.svd`** (downloadable from Nordic's website) and a Cortex-M4 / Thumb-2 mindset.

## Setup procedure

1. Install Ghidra (10.x or later)
2. Install `SVD-Loader-Ghidra` plugin (or import the SVD manually via Memory Map → Add Block + a custom data type for each peripheral)
3. Create a new Ghidra project; import the firmware binary as raw bytes
4. Set the language to **ARM:LE:32:Cortex-M** (32-bit little-endian Cortex-M)
5. Set the image base to **0x00000000** (the nRF52840 reset vector lives here)
6. Run auto-analysis with "Decompiler Parameter ID" enabled
7. Load the SVD overlay (`File → Import → svdloader.py` if using the plugin)
8. Watch named peripheral references appear in the decompilation (e.g., `NVMC.CONFIG` instead of `*(0x4001E504)`)

This setup is standard for nRF52 reverse engineering and is well-documented in the nRF52840 RE community (independent of SP-1 work).

## Key findings from the dump

### Privilege mode checks

Duloz noted [Lines #220–240, Duloz, ~Feb 2025]: the firmware **uses Cortex-M unprivileged execution** for some code paths. This is significant because it suggests:

- TE made some attempt at compartmentalization between trusted bootloader and less-trusted app
- Specific operations (like writing UICR) are restricted to privileged context
- The bootloader region (`0x00000000`–`0x00020000`) executes in privileged mode; the app slot (`0x00020000`+) may not

This insight informed the bootloader region documentation and made it clear that custom firmware in slot 0 cannot trivially modify the bootloader (you'd need to escalate to privileged mode first).

### Bootloader structure

Disassembly confirmed:

- Bootloader at `0x00000000`–`0x00020000` (128 KB)
- App at `0x00020000`–`0x000ff000` (~892 KB)
- Last 4 KB of flash used for storage / settings
- Reset vector points to the bootloader, which then jumps to the app after its own init

Inside the bootloader:
- USB CDC ACM initialization
- Track 1 + Track 4 button detection logic (ADC poll at boot)
- 60-second timeout if no command received
- GPREGRET-based parser selection (parser 1 = firmware flash, parser 2 = album upload)
- Opcode dispatch table for the documented commands

The bootloader is small but sophisticated. Its presence as a fully-functional flash-update mechanism is what enabled `solderless.engineering`.

### Minimal I²S register usage

murray initially flagged a concern [Lines #317]:

> "the I2S peripheral has only TASKS_START and TASKS_STOP assigned, which can't be right"

This *looked* like an incomplete dump — surely the firmware does more with I²S than start/stop? But subsequent analysis confirmed the dump was complete and the stock firmware really does use a minimal I²S setup. Most I²S configuration happens through DMA descriptors set up once at boot; the firmware only needs to start/stop the peripheral.

This taught the community two things:
1. The dump was reliable (no need to re-glitch)
2. Custom firmware doesn't need to fully exercise every nRF peripheral; minimal use is fine

### Audio engine architecture

The firmware contains a layered audio engine:

- **Storage layer** (eMMC bit-bang + SPIM3 + DMA) → moved into `storagethingies`
- **Frame decoder** (raw bytes → int32 samples) → moved into `storagethingies::decode_te_frame_payload_i32`
- **Mixer + transport** (Q12 weights + VarispeedResampler) → moved into `audiothingies::StockRuntimeMixer`
- **Effects** (filter/distortion/gate/chorus/delay) → moved into `audiothingies::effects::*`
- **I²S TX** (DMA buffer management) → moved into `audiothingies::backends::zephyr_i2s_tx`

ericlewis's `assets/audiothingies-2026-05-09/storagethingies` reflects this analysis, with the layers cleanly separated in clean-room C++17 code.

### eMMC driver structure

The stock firmware's eMMC driver uses the same hybrid GPIO bit-bang + SPIM3 approach that `assets/storagethingies-2026-05-09/EmmcDriver` implements. The 11-state init sequence (POWER_ON → CMD0 → CMD1_POLL → CMD2 → CMD3 → CMD9 → CMD7 → CMD8 → CMD16 → VERIFY → SETUP_ASYNC) was extracted from disassembly and reimplemented.

### Effects implementation details

The DSP node descriptions in `13-dsp-effects.md` come from cross-referencing the disassembly with `assets/audiothingies-2026-05-09/effects/*`. Notable findings:

- **Pre-gain 16.0** for distortion → identified by the multiply-by-constant in disassembly
- **Polynomial waveshaper `x − 0.5·x³`** → identified by the FMUL/FSUB sequence in the inner loop
- **Smoothing coefficient 0.02f = 0x3CA3D70A** → identified by the specific 32-bit constant load in the speed-update path
- **8192-sample delay buffer per stem** → identified by the buffer size constant + 4-stem array

These are the "deterministic constants" that make the reverse engineering credible — each is grounded in a specific instruction sequence and cross-referenced against the audio behavior.

## What hasn't been fully disassembled

A few areas where public documentation thins out:

### UI / menu state machine

The firmware presumably has a UI state machine handling button presses, screen-equivalent state (LED animations), and menu navigation. This is not in the public reverse-engineering output as a structured state diagram. ericlewis's `audiothingies` doesn't include the UI layer (that's app-level, not engine-level).

### Bluetooth interaction

The firmware's interface with the CYBT BT module hasn't been publicly disassembled in detail. Presumably it's HCI commands over UART at 115200 baud, with some firmware-side state machine for pairing / streaming / disconnection. emvee1968's working Bluetooth implementation is the closest public reference but it's separate code, not based on the stock firmware's approach.

### Power management

The firmware's power-down sequence, USB attach/detach handling, and battery management are presumably implemented but not publicly documented at the disassembly level. `sp1-midi`'s `PowerManager` is a working alternative but doesn't necessarily replicate the stock behavior.

### Specific DSP coefficient values

The TAS2505 DSP coefficient table loaded at init (the `{ reg, byte0, byte1, byte2 }` triples in `assets/sp1-midi-2026-05-13/drivers/audio/tas2505_codec.c`) — what does each coefficient implement? It's likely a speaker EQ + bass limiter to protect the small internal speaker. The exact filter shapes aren't documented. (This is one of the open questions for TimK in `cc-skill/questions-for-timk.md`.)

## How to verify findings against the dump

If you have access to a copy of the firmware:

1. Open it in Ghidra with the SVD overlay
2. Search for specific constants mentioned in this skill (e.g., `0x3CA3D70A`, `8192`, `kGateLevels`)
3. Cross-reference the disassembly with `assets/audiothingies-2026-05-09/` source
4. Verify the constants match in both places

If you find a discrepancy, that's a real research finding. Report it to the Discord — TimK and ericlewis would want to know if `audiothingies` diverges from stock behavior.

## Why disassembly matters going forward

The community has extracted the design intent of the stock firmware well enough that custom firmwares can replicate the behavior. But there are still gaps:

- The **side-data byte layout** in sectors (32 bytes per sector for sync + LED + trailer) isn't fully documented at the byte level
- The **upload protocol packet framing** for `0x39` isn't public
- The **album header** format details (magic value, version, padding) aren't published

Closing each of these requires either:
- A direct answer from someone who knows (TimK, ericlewis)
- More disassembly work to find the answer in the firmware

The Ghidra setup remains a useful research tool for filling these gaps.

## Where to go next

- For the APPROTECT bypass that enabled the dump → `14-approtect-glitch-attack.md`
- For ericlewis's clean-room engine that came out of the analysis → `12-audio-engine-internals.md`
- For the on-disk format documented from disassembly → `09-audio-format-spec.md`
- For the bootloader region structure → `15-bootloader-protocol.md`
- For open questions that disassembly could answer → `known-unknowns.md`
