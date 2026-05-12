# Debug Interfaces

**Synthesized through:** Lines #846 (2026-05-06), Discord through 2026-05-11.

This file covers every way to talk to a SP-1 for debug, dump, or flash purposes: SWD, USB CDC ACM, the bootloader CDC mode, the eMMC test points, and the LED-bit-bang technique. For the bootloader **protocol** (the byte-level commands once you're in bootloader mode) see `15-bootloader-protocol.md`. For the eMMC **format** see `08-emmc-storage.md`.

## Summary

| Interface | Hardware needed | What you can do |
| --- | --- | --- |
| **Track 1 + Track 4 + USB-C** (bootloader trigger) | A USB-C cable and a working device | Flash a new firmware via the USB CDC bootloader (no soldering); cannot read the existing firmware |
| **`solderless.engineering`** (web updater) | Just a Chromium-family browser supporting Web Serial | Same as above, with a friendly UI |
| **USB CDC ACM console** (`sp1-midi` + `cdc-acm-console` snippet) | Working app firmware that includes CDC | Serial console / Zephyr shell during normal runtime |
| **SWD header** (user-added) | Open the device + solder SWD pads + use a debugger (J-Link, DAPLink, etc.) | Full debug, breakpoints, RAM/flash read/write — only works on a chip that has had APPROTECT cleared via a glitch, or on a chip in factory state |
| **eMMC test points** | Open the device + clip leads or pogo pins | Read or write the eMMC directly, bypassing the nRF52840 |
| **LED bit-bang dumping** | A custom 1-page firmware + the bootloader update path + a camera or photodiode | Read flash byte-by-byte by encoding bytes as LED states (slow but works on a still-locked chip) |
| **eMMC desolder + USB-to-eMMC adapter** | Hot air, an unpopulated USB-to-eMMC PCB | Drop the chip onto an adapter and `dd` it from a host PC |

## 1. Bootloader CDC mode (Track 1 + Track 4 + USB-C)

**The community's most important discovery.** B_E_N established the trigger in Lines #556, late 2025. Pressing **Track 1 + Track 4 + plugging in USB-C** enters the bootloader for **60 seconds**. The device enumerates as a USB CDC ACM device on the host. The bootloader is what `solderless.engineering` talks to and what every custom-firmware flash uses.

Things that are **not** part of the trigger (despite earlier guesses):
- Power button — not required, was removed from an original version of B_E_N's post in an edit
- Anything connected to SWD — the bootloader is unrelated to the SWD pin state

Critical limits of bootloader CDC mode:
- **Cannot dump firmware.** The bootloader does not expose a flash-read command. *"you cannot"* [Discord #firmware, ericlewis, 2026-05-08 17:13, in response to *"are there known commands to read/dump firmware over the serial interface?"*].
- **Can flash app firmware.** Sequence: `0x52` (bootloader version) → `0x70 0x01` (set GPREGRET magic `0x1A96` for parser 2) → `0x50` (reboot) → stream firmware via `0x39` packets → `0x51` (reset). See `references/15-bootloader-protocol.md` for the full opcode list.
- **Can read album content** indirectly via the running firmware after reflashing.
- **Custom firmware risk:** if your firmware doesn't include a working route back to the bootloader, you can brick the device until you either (a) trigger Track 1 + Track 4 + USB again from cold start, or (b) disconnect the battery and re-plug USB to force the chip into a reset state. emvee1968 reported having to do (b) with their early Claude-Code-generated firmware [Discord #firmware, 2026-05-08 23:56].

## 2. `solderless.engineering` web utility

A web-based wrapper around the bootloader CDC mode. Uses **Web Serial** in Chromium-family browsers. Lets a user flash a custom firmware binary onto a real SP-1 with no opening, soldering, or local tooling required.

- Public URL: `https://solderless.engineering/`
- Cloudflare Pages mirror: `https://solderless-engineering.pages.dev/`
- **Status as of 2026-05-09:** offline for an update [Discord #news, tkt1000, 2026-05-09 12:29 — *"PSA: The update utility is temporarily offline, in preparation for a lil' update."*]. tkt1000 has said it will return; the team has not given an ETA.
- Built by **TimK** in collaboration with team members **loksi**, **tunelight**, and **keebstudios** [Discord #general, tkt1000, 2026-05-06 20:30]
- Source code is **not currently public**

When the utility is online, the user flow is:

1. Open the URL in Chrome.
2. Pick a firmware `.bin` (or one of the prebuilt options).
3. Plug the SP-1 into USB-C while holding Track 1 + Track 4.
4. Hit "flash."
5. The browser opens a Web Serial connection, walks the bootloader command sequence, and writes the firmware.

If the utility is offline, the alternatives are:

- Use `theunflappable`'s Python re-implementation: `test_bootloader.py` (shared in Discord #firmware on 2026-05-08 by theunflappable). This implements the bootloader protocol but does **not** add stem upload — it's just the protocol re-host.
- Build your own client against the documented opcode sequence (see `15-bootloader-protocol.md`).

## 3. USB CDC ACM console (runtime serial)

`sp1-midi` enables a CDC ACM serial console via the snippet `cdc-acm-console`:

```sh
cmake -B build -GNinja -DBOARD=stem_player -DSNIPPET=cdc-acm-console
```

[code: `sp1-midi/README.md` Quick Start; `sp1-midi/prj.conf` enables `CONFIG_USBD_CDC_ACM_CLASS=y`, `CONFIG_SHELL=y`, `CONFIG_SHELL_BACKEND_SERIAL=y`]

This gives you a Zephyr shell over a USB CDC ACM endpoint. You see boot logs, can type commands, and probe at runtime. The device shows up as a composite USB device alongside the MIDI 2.0 endpoint.

Notes:
- The CDC ACM console is only available while the **custom application firmware** is running and has booted enough to enumerate USB. If the app crashes before USB is brought up, you see nothing — use SWD for that level of debug.
- The CDC console does **not** give you bootloader access. To reach the bootloader you still need Track 1 + Track 4 + USB-C from a power-on event.

## 4. SWD header (user-added)

There is **no factory SWD header on the SP-1**. TimK added one to develop custom firmware [Discord #hardware, tkt1000, 2026-05-07 17:43 — *"No, I only soldered an SWD header"*]. The community consensus is that **you cannot properly develop firmware for this device without an SWD header** [Discord #hardware, tkt1000, 2026-05-08 05:20 — *"you can't properly develop firmware for this without fully opening the device and adding the SWD header so you can attach a debugger"*].

SWD requirements:
- Access to **SWDIO**, **SWCLK**, **GND**, and **VDD** (or a 3.3 V supply matched to the SP-1's logic level)
- A compatible debugger: J-Link, DAPLink-compatible probes, Black Magic Probe, or the Nordic nRF52 DK (which can be re-flashed as a debug probe)

As of synthesis date, **the exact PCB pad locations for SWDIO and SWCLK are not in the publicly visible documentation**. TimK promised wiki documentation of the test points [Discord #hardware, tkt1000, 2026-05-07 17:43]; check `github.com/timknapen/SP-1-dev/wiki` for an SWD pinout. If it's not there, ask in Discord.

**Important about APPROTECT:** A factory SP-1 has Nordic's APPROTECT bit set. SWD will **not** give you read access to firmware. SWD can still be used for flash erase + reprogram (the chip allows mass-erase via SWD even when APPROTECT is set), so once you've added an SWD header, you can develop your own firmware. The original firmware will be wiped in the process. The only way to **read** the original firmware via SWD is to first clear APPROTECT via a voltage glitch attack — see `references/14-approtect-glitch-attack.md`.

## 5. eMMC test points

Per TimK, the eMMC's **DAT0, DAT1, DAT2, DAT3, CLK, and CMD lines are broken out to test points on the PCB** [Discord #hardware, tkt1000, 2026-05-07 17:41–17:53]:

> There are test points you can connect to. … It's single data line eMMC 5.0. … Actually if I remember correctly DAT0-DAT3 are broken out to test points, so you could read much faster than the MCU!

This means you can dump or write the eMMC by clipping onto these test points without desoldering the chip. The catch: **the exact pad locations have not yet been documented in any of the publicly available material as of synthesis date.** TimK said *"I'll add documentation of the test points to the wiki"* on 2026-05-07; verify whether `github.com/timknapen/SP-1-dev/wiki` has a test-points page before relying on this.

If you do find the test points, an SD-card breakout board or a standalone eMMC reader (sold as "eMMC ISP adapters") will let you read/write the chip from a host PC. Note that the chip stores the SP-1 album in a custom 8 KB-sector layout — see `08-emmc-storage.md` for the on-disk format.

## 6. eMMC desolder + USB-to-eMMC adapter

For a complete physical extraction, `fishdog_` documented the desolder method in Discord #hardware:

- **Desolder temperature:** approximately **380–420 °C** with a hot-air station, on a **175 °C** preheater [Discord #hardware, fishdog_, 2026-05-11 19:08]
- **No reballing required** — fishdog_ built up a layer of solder on the chip pads and on the destination adapter, then aligned and reflowed [Discord #hardware, fishdog_, 2026-05-10 19:25]
- **Destination:** an unpopulated USB-to-eMMC adapter PCB sold cheaply on AliExpress and Amazon. fishdog_ linked to a specific Amazon listing (since unavailable at synthesis date) and a working AliExpress alternative [Discord #hardware, fishdog_, 2026-05-10 19:22; esko8444, 2026-05-11 07:02]

Once soldered to the adapter, the eMMC enumerates as a USB block device. `dd` it directly. fishdog_ reported success without any custom driver or formatting:

> I was just able to solder it on and plug it in and dd. [Discord #hardware, fishdog_, 2026-05-10 18:55]

After dumping, the SP-1 is non-functional until you reverse the procedure (or replace the eMMC).

## 7. LED bit-bang firmware dump

**The cleverest extraction technique.** `zee_33` demonstrated this in Discord #firmware on 2026-05-08:

The setup:
- The SP-1 still has APPROTECT set, so you cannot read its flash via SWD or the bootloader.
- But the bootloader **lets you flash any custom firmware you want** to slot 0.
- Your custom firmware, running on the device, can read the entire flash freely — including the bootloader region (`0x00000000`–`0x00020000`) and any data that wasn't overwritten when slot 0 was reflashed.
- The same custom firmware can blink LEDs.
- ∴ Write the smallest possible firmware that reads a window of flash and modulates the 8 LEDs to encode bytes, then iterate windows.

zee_33's approach [Discord #firmware, zee_33, 2026-05-08 17:33 + 19:04 + 19:08]:

> You can't [read firmware via the bootloader], however thanks to TKT documentation of everything was I was able to hack up a way to use the LED lights as feedback to read certain parts of the firmware.

> essentially that, it was a lot of back and forth of reading the bytes via LED translation then feeding it back. bitbanging through lots of iterations,

> yes started very small, i think the utility described it as 1 page

Key practicalities:
- **Keep the dump firmware tiny** to minimize how much of slot 0 gets overwritten. zee_33 says ~1 page (= 4 KB on the nRF52840). The flash regions you overwrite when you flash the dump firmware are unrecoverable.
- **Read with a camera, not your eyes.** The LEDs can switch fast enough that visual reading is painfully slow; an iPhone camera at high frame rate plus a script to decode the LED patterns is the practical setup.
- **You'll need multiple firmware iterations** — one to dump bootloader region, one to dump everything *outside* the slot you overwrote, and so on. Restore the device between dumps by re-flashing the original-album-loading firmware (or whatever you want to test next).

This technique is unique to devices where you have a flashing path but no read path. The SP-1's bootloader is exactly that case.

## 8. Boot-time reset breadcrumbs (in `sp1-midi`)

When using `sp1-midi`, the BSP records the reason for the last reset in a small persistent area so you can inspect "why did my device boot?" after the fact. Look at `subsys/system/ResetBreadcrumbs.cpp` and `ResetBreadcrumbs.hpp`. This is useful when chasing watchdog timeouts or unexpected reboots during custom-firmware development.

## Where to go next

- For the bootloader protocol (byte-level commands once you're in bootloader mode) → `15-bootloader-protocol.md`
- For the USB upload protocol (sending an album to the device) → `16-usb-upload-protocol.md`
- For the APPROTECT bypass glitch attack → `14-approtect-glitch-attack.md`
- For the LED bit-bang dump in more detail → `17-led-bitbang-dump.md`
- For the eMMC format → `08-emmc-storage.md`
- For physical disassembly to access SWD pads → `23-physical-disassembly.md`
