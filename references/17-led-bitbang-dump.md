# LED Bit-Bang Firmware Dump

**Synthesized through:** Lines #846 (2026-05-06), Discord through 2026-05-11. Primary source: zee_33's description in Discord #firmware 2026-05-08.

**zee_33** demonstrated a clever technique on **2026-05-08** for extracting firmware from a locked SP-1 **without** the voltage glitch attack and **without** opening the device: write a tiny custom firmware that uses the LEDs to encode flash bytes as visual patterns, capture with a camera, decode. This file documents the technique so other researchers can reproduce or build on it.

The technique is particularly satisfying because it composes two pre-existing capabilities (the Track1+4 flashing path and the LED PWM driver) into a firmware-extraction tool. **No exotic hardware needed.**

## The premise

A factory SP-1 has APPROTECT enabled, so SWD can't read firmware. But:

1. The Track 1 + Track 4 bootloader trigger **lets you flash any firmware you want into slot 0**.
2. Your custom firmware, running on the device, can read the **bootloader region** (`0x00000000`–`0x00020000`) and **anything in flash you haven't overwritten yet**.
3. The same custom firmware can drive the 8 LEDs via PWM.

Therefore: write a small firmware that reads chunks of flash and encodes the bytes as LED patterns. Capture with a phone camera. Decode the patterns back into bytes. Repeat for each chunk.

zee_33's account [Discord #firmware, 2026-05-08]:

> You can't [read firmware via the bootloader], however thanks to TKT documentation of everything was I was able to hack up a way to use the LED lights as feedback to read certain parts of the firmware

> essentially that, it was a lot of back and forth of reading the bytes via LED translation then feeding it back. bitbangging through lots of iterations

> yes started very small, i think the utility described it as 1 page

The "TKT documentation" is `github.com/timknapen/SP-1-dev/wiki` — having the pin map and basic Zephyr setup made bootstrapping a minimal LED-blinking firmware fast.

## The mechanics

### Step 1: Write the smallest possible "dump-via-LED" firmware

You need a firmware that:

- **Is small** (ideally 1 page = 4 KB) so it overwrites as little of slot 0 as possible
- **Initializes the 8 LEDs** (4 Track LEDs on PWM2 + 4 Play LEDs on PWM3 — see `02-hardware-overview.md`)
- **Reads a chunk of flash** at a parameterized address
- **Modulates the LEDs** to encode the chunk's bytes
- **Allows reconfiguration via something** (e.g., specific button combos selecting which address range to dump)

8 LEDs = 8 bits at a time, so one LED-state can encode one byte (each LED on/off representing one bit). Reading 4 KB at 100 bytes/second visual frame rate = 40 seconds per chunk. A 128 KB bootloader dump at this rate is ~22 minutes per chunk × 32 chunks = ~12 hours.

Faster: use grayscale levels (PWM duty cycle) — each LED can encode 2-3 bits via brightness levels. 8 LEDs × 3 bits = 24 bits per frame. Camera frame rate at 60 fps = 90 KB/s. Now 128 KB takes ~1.5 seconds visual + many minutes of camera + decoding overhead.

In practice, zee_33 likely used something between these (probably on/off binary for reliability) and iterated many times to dump targeted chunks.

### Step 2: Flash this firmware via the bootloader

Standard Track 1 + Track 4 + USB-C trigger + `solderless.engineering` (or your own tool against the bootloader protocol — see `15-bootloader-protocol.md`). The custom firmware overwrites slot 0 (`0x00020000`+); the **bootloader region** (`0x00000000`–`0x00020000`) is preserved because the Track1+4 bootloader doesn't write there.

### Step 3: Capture with a camera

Point a phone camera at the SP-1's LED face. iPhone at 60 fps (or 240 fps slow-motion) is sufficient. Slow-motion is preferred — gives more frames per LED transition to handle decode errors.

You'll need:

- A fixed mount so the camera doesn't move
- Consistent lighting
- A way to start the firmware sequence in sync with the camera (e.g., a specific button press to begin)

### Step 4: Decode the video

Write a Python script that:

1. Iterates frames of the video
2. For each frame, samples the brightness of each LED's pixel region
3. Thresholds (or quantizes if using grayscale) to recover the encoded bits
4. Assembles bytes from the LED bit patterns
5. Writes to an output file

OpenCV is sufficient for this. The hardest part is reliably finding the LED positions in the frame (since they move slightly with handheld setups even on a mount).

### Step 5: Iterate

A 4 KB dump firmware can read maybe 16 KB of flash at a time (the firmware itself + RAM working area + LED driver consume some space). For the full ~1 MB flash:

- **Dump 1:** read 0x00000000 – 0x00020000 (the bootloader, 128 KB)
- **Dump 2:** read 0x00020000 – the firmware's own start address (the rest of the old slot 0, up to but excluding where your dump firmware lives)
- **Dump 3:** if you want even the bytes your dump firmware overwrote, flash a *different* dump firmware to a different physical address, read the original location

The trick is to **never overwrite the bytes you haven't yet dumped**. Plan your dump firmware addresses carefully.

## Why this works on APPROTECT-enabled devices

APPROTECT blocks **external** read access via SWD. It doesn't block **internal** read access — the running code on the chip can freely read flash. Your custom firmware is running code; it has full access to all flash regions including the bootloader.

The bootloader region remains the original TE bootloader (unchanged by your custom firmware flash). Reading it via your firmware lets you extract the bootloader's binary without ever bypassing APPROTECT.

The app region (slot 0) contained the original TE app firmware **until you flashed your custom dump firmware over it.** The original app firmware is now lost from this particular SP-1 — but if you have the dump of someone else's device (like murray's January 2025 dump from the glitch attack), you have the original app binary already.

So the LED bit-bang technique is most useful for:

- **Bootloader extraction** (the bootloader region is untouched by flashing slot 0)
- **Reading any data the bootloader stored outside the app slot** — including UICR, factory info, possibly device-specific keys if any
- **Confirming firmware integrity on a specific device** (does this SP-1's bootloader match the canonical dump from murray?)
- **Live RAM inspection** (extending the technique to dump SRAM, peripheral register state, etc., would just need a firmware that reads RAM addresses instead of flash)

## What this technique doesn't give you

- **The original app firmware** if it was already overwritten by your dump tool. Murray's glitch dump (Jan 2025) is the canonical extracted app firmware; LED bit-bang doesn't reproduce that work, it complements it.
- **Speed.** Compared to a proper SWD dump (which is hundreds of KB/s), this is orders of magnitude slower. But it's vastly faster than running 8,504 glitch attempts.
- **Real-time debugging.** You can't set breakpoints or read live RAM in a useful interactive way. The technique is for batch extraction, not live debug.

## Comparison to other extraction methods

| Method | Requires opening device? | APPROTECT bypass? | Speed | Risk to device |
| --- | --- | --- | --- | --- |
| **Track1+4 + LED bit-bang** | No | No (works against APPROTECT) | Slow (KB/s effective rate) | Low — overwrites slot 0 but bootloader preserved |
| **SWD with glitch attack** | Yes (open + solder header) | Yes — voltage glitch | Fast once glitched | Medium-high (long campaign of glitches, risk of hardware damage) |
| **SWD without glitch** | Yes (open + solder header) | No — mass-erase only | N/A for reads | Medium (open device + lose original firmware) |
| **eMMC test point read** | Yes (open device) | N/A (eMMC has no APPROTECT) | Fast (eMMC reader) | Low — non-invasive |
| **eMMC desolder** | Yes (heavy work) | N/A | Fast | High — desoldering risks heat damage |

LED bit-bang fills a unique niche: **firmware extraction without opening the device and without glitching**. It's the slowest reading path but the lowest-skill / lowest-tool-requirement option.

## Things to verify

The technique is documented at a high level but specific implementation details are not public:

- zee_33's exact LED encoding scheme (binary? grayscale? error correction?)
- The address ranges actually dumped
- Verification methodology (CRC of dumped bytes vs known-good murray-dump bytes)
- Whether the dump firmware itself is published anywhere

These would be good follow-up questions to zee_33 in Discord if you're planning to reproduce.

## Why this matters for security model thinking

The LED bit-bang technique demonstrates that **APPROTECT alone is not a meaningful barrier when the device has a writable firmware slot**. Once a user can flash *any* firmware (even unprivileged), they can write a firmware that reads everything else and exfiltrates via any available channel — LEDs, USB CDC, Bluetooth, radio, audio output.

This isn't a vulnerability in APPROTECT per se — APPROTECT was designed to prevent **external** reads, not arbitrary internal code execution. But it's a reminder that "firmware is read-protected" and "the device runs trustworthy code only" are different security claims, and APPROTECT only addresses the first.

For SP-1 in particular, this enables non-destructive research on individual units, which is a positive. For devices that need stronger protection, the design lesson is "don't let arbitrary firmware run, or assume any firmware can dump everything."

## Where to go next

- For the bootloader protocol that enables flashing the dump firmware → `15-bootloader-protocol.md`
- For the APPROTECT background → `14-approtect-glitch-attack.md`
- For the LED PWM driver pins → `02-hardware-overview.md`
- For the Zephyr build environment to compile the dump firmware → `18-zephyr-build-environment.md`
- For other physical extraction methods → `24-emmc-direct-extraction.md`
