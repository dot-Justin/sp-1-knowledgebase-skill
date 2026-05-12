# APPROTECT Glitch Attack

**Synthesized through:** Lines #846 (2026-05-06), Discord through 2026-05-11.

The Nordic nRF52840 has a hardware **APPROTECT** (Access Port Protection) bit that, when set, blocks SWD-based read access to firmware. Production SP-1 units ship with APPROTECT enabled. The community bypassed this in **January 2025** via a voltage glitch attack — and the technique is now well-documented public knowledge. This file describes the attack at the level needed to understand the threat model and the contribution made by **murray**, who led the campaign and succeeded on attempt #8,504.

**Important:** This is **educational / historical context**. Do not use this against devices you don't own, and don't attempt it without understanding the risk of bricking the device.

## What APPROTECT does on nRF52840

The nRF52840 has multiple revisions; older silicon revisions (rev A, rev B early) have a known weak APPROTECT implementation. The bit is set in the chip's UICR (User Information Configuration Registers) and is checked by the chip on every boot.

When set:
- SWD debug connections are refused (no read/write/run-control)
- The chip won't expose any registers via the access port
- Mass-erase is still allowed (the chip can be wiped + reprogrammed via SWD), but the existing firmware can't be read out

When cleared (or bypassed by a glitch):
- SWD becomes fully functional: firmware can be dumped, RAM inspected, breakpoints set

This is Nordic's documented security feature. The known weakness is that the **bit check happens at a known timing during boot** and can be disrupted by a precisely-timed voltage transient (glitch). If you glitch the chip's voltage rail at exactly the right moment, the APPROTECT check fails open — SWD is enabled even though APPROTECT is still nominally set in UICR.

This is known as a **"crowbar glitch"** — short the supply rail briefly to drop voltage, then release. Done at the right time, it corrupts a specific load instruction in the boot ROM.

The vulnerability is documented in **limitedresults**' public writeup ("nRF52 Debug Resurrection / Access Port Protection Bypass") from 2020. The SP-1 community applied this known technique to a real target.

## Why the SP-1 was a glitchable target

The SP-1's nRF52840 is rev B (or earlier — exact rev not publicly cataloged). It's susceptible to the limitedresults technique. Hardware access points are accessible:

- **DEC1 capacitor:** the decoupling cap on the nRF's VDD pin. Shorting this briefly drops the voltage rail.
- **DEC4 capacitor:** the decoupling cap on the chip's internal regulator output. Alternative crowbar target.

(DEC6 also gets mentioned in early hardware analysis posts but didn't end up being the productive target.)

The attack tooling needed:
1. A way to short the cap precisely (typically a MOSFET driven by a fast pulse generator)
2. A way to clock the glitch timing precisely (nanosecond-scale precision)
3. A way to monitor for success (a SWD probe trying to connect immediately after each glitch attempt)
4. A way to retry many thousands of times

The success rate is low — depending on glitch parameters, anywhere from 1-in-1000 to 1-in-100000. So you need to automate it and run for hours or days.

## murray's BeagleBone PRU rig

murray built an automated glitcher using a **BeagleBone Black**, leveraging its **PRU (Programmable Real-time Units)** to generate the precise voltage pulse timing the attack requires. The code is public:

`https://github.com/resinbeard/beaglebone-black-glitcher`

[Lines #~105–200, murray, mid-2024 through early 2025; referenced in thread summary]

The setup:

1. **BeagleBone Black** with PRU code that generates pulse signals at GHz-class accuracy (PRU runs at 200 MHz, so 5 ns granularity)
2. **MOSFET shorting circuit** wired to DEC1 (or DEC4)
3. **SWD probe** (J-Link or similar) attempting to connect after each glitch attempt
4. **Logging** of every attempt parameter and outcome for tuning

The PRU is the key innovation. Software-timed glitches from a Linux process or even a microcontroller running C don't have enough timing precision; the PRU lets murray sweep across nanosecond-scale offsets and pulse durations to find the small window where the glitch is reliable.

## elew's parallel scale-up

Beyond a certain point, **murray was the only one running the rig**. elew joined the effort with **5 additional SP-1 units** and ran parallel glitch attempts. This was essential — running 5x more attempts per unit-time means hitting #8,504 in days instead of weeks.

[Lines summary; ~late 2024 through January 2025]

## Attempt #8,504 — the dump

On **2025-01-25**, murray's rig succeeded on attempt **#8,504**. The SWD probe connected, APPROTECT-related registers were accessible, and the full nRF52840 firmware (bootloader + app) was dumped.

The dump was approximately 256 KB:
- Bootloader region: 0x00000000–0x00020000 (128 KB)
- App region: 0x00020000–0x000ff000 (~892 KB)

Validation: subsequent successful glitches on murray's instrumented board produced "obviously wrong" dumps [Lines #317, murray], suggesting the success rate even after one good run remained low. The successful dump was preserved and cross-validated; it is the basis for all subsequent analysis (Ghidra disassembly by Duloz et al., TimK's custom firmware reference work, ericlewis's `audiothingies` reimplementation).

## What was learned from the dump

Once dumped, the firmware was loaded into **Ghidra** with the nRF52840 SVD register file as overlay. Key findings:

- **Privilege mode checks** — the firmware uses Cortex-M unprivileged execution for some paths [Lines #~205–240, Duloz]
- **Bootloader logic** — including the Track 1 + Track 4 + 60-second-USB-enumeration trigger, which TimK and B_E_N later turned into the solderless flashing pathway
- **Audio engine structure** — handed off to TimK/JoseJX for stem-decoding work
- **Minimal I²S setup** — initially appeared suspicious but turned out to be accurate (TE firmware really does use minimal I²S register assignments) [Lines #317]
- **eMMC driver** — TimK ported a clean-room equivalent into Zephyr

The disassembly work fueled both the bootloader exploitation (leading to `solderless.engineering`) and the audio format reverse engineering (leading to `audiothingies`/`storagethingies`).

## Did the dump leak Kanye stems?

The original eMMC content (Ye's *Donda* and *Jesus Is King* stems) is **separate from the nRF52840 firmware**. The firmware extraction tells you how the device works; it does **not** include audio data. Audio data is on the eMMC, which is directly readable without any APPROTECT bypass (just connect to test points or desolder the chip) — see `04-debug-interfaces.md`.

The community **does not distribute** the original Kanye album stems for copyright reasons. The firmware itself (which is TE's intellectual property) is also not redistributed in binary form publicly.

## What this means for users today

If you have a stock SP-1 and want to develop custom firmware, you have two paths:

### Option 1 (recommended): Don't worry about APPROTECT

Use the **Track 1 + Track 4 + USB-C** bootloader trigger to flash a custom firmware via `solderless.engineering` (when online) or via your own client against the bootloader protocol (`15-bootloader-protocol.md`). The custom firmware overwrites the app slot; APPROTECT is irrelevant for this workflow.

You **lose the original TE firmware** when you do this (the slot is overwritten). To restore stock behavior, you'd need to flash a stock firmware binary back — which is not publicly distributed but may circulate among community members.

### Option 2 (advanced): Add an SWD header and develop with debug access

Open the device, solder SWD pads (`04-debug-interfaces.md`), and develop with full debug. APPROTECT still blocks you from reading the original firmware, but it does **not** block mass-erase + reprogram. You can use SWD just for development of *your own* firmware.

### Option 3 (research only): Perform the glitch yourself

If you want to dump the original firmware (e.g., to verify or extend the community's reverse-engineering), reproduce murray's setup. This is a significant project — expect to spend weeks on the rig, and have a separate "research" SP-1 unit you're willing to risk.

## What this enables for attackers (threat model)

The APPROTECT bypass means:

- **Anyone with physical access** to an SP-1 (or any nRF52840-based device with this revision) can extract firmware. Hardware-level secrets (keys, intellectual property) in flash are not protected against a determined attacker with a few days and a BeagleBone.
- **Firmware-level secrets are unsafe.** If TE had embedded an AES key in firmware to decrypt the album, that key would now be known.
- **Custom firmware can be flashed** without the user's knowledge if an attacker has brief physical access (Track 1 + Track 4 + USB takes 60 seconds and doesn't require disassembly).

For SP-1 in particular, this isn't a concern — the device doesn't hold user secrets, and the community is using the access constructively. For other nRF52840-based devices, this technique is a published risk that designers should plan for in newer hardware.

## Newer nRF52 silicon

Nordic addressed the timing-glitch weakness in newer silicon revisions. Devices manufactured after a certain point use a more robust APPROTECT implementation. The SP-1's silicon date is from approximately 2019–2020 (matching its development timeline) which is well within the affected window.

## Where to go next

- For the bootloader protocol (which is the main reason most users care about APPROTECT) → `15-bootloader-protocol.md`
- For the firmware analysis (Ghidra, disassembly) → `22-disassembly-ghidra.md`
- For the eMMC dump methods (separate from firmware glitch) → `04-debug-interfaces.md`
- For the LED bit-bang trick (a clever firmware extraction that bypasses glitch entirely, by leveraging the bootloader's flashing capability) → `17-led-bitbang-dump.md`
- For ericlewis's clean-room engine (what the dump enabled) → `12-audio-engine-internals.md`
