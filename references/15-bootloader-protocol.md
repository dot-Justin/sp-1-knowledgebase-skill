# Bootloader Protocol

**Synthesized through:** Lines #846 (2026-05-06), Discord through 2026-05-11. Reference URL: `github.com/timknapen/SP-1-dev/wiki/Bootloader` (canonical doc — verify current state).

The SP-1 has a small bootloader in the first 128 KB of nRF52840 flash. **Track 1 + Track 4 + USB-C** triggers it; the device then enumerates as a USB CDC ACM device for **60 seconds** and accepts a small set of single-byte opcodes that allow flashing a new application firmware. The bootloader cannot read existing firmware, only write to the app slot.

This file documents the opcode-level protocol so that anyone can talk to the bootloader without needing `solderless.engineering`.

## Trigger

1. **Hold Track 1 + Track 4** (both buttons pressed simultaneously)
2. **Plug in USB-C** (data + power)
3. Release the buttons after enumeration

The device:
- Boots into bootloader mode (not the app)
- Enumerates as a USB CDC ACM device on the host
- Stays in bootloader mode for **60 seconds**
- After 60 seconds with no activity, reboots into the app

The trigger was discovered by **B_E_N** [Lines #556, ~mid-2025]. Original post included a "step 4 — power button" that was removed in an edit [see `corrections.md`].

**Important:** the analog button matrix decodes Track 1 + Track 4 as a specific threshold combination on ADC channel AIN0. If your hardware is damaged in a way that prevents the SP-1 from reading this combo (e.g., FPC disconnected), the bootloader can't be entered through the buttons. SWD is the alternative — see `04-debug-interfaces.md`.

## Opcode set

The protocol uses single-byte opcodes over USB CDC ACM. Opcodes are documented by **ericlewis** in Discord, paraphrased here with TimK's wiki as cross-reference [Discord #general, ericlewis, 2026-05-09 09:45; `github.com/timknapen/SP-1-dev/wiki/Bootloader`].

| Opcode | Name | Purpose |
| --- | --- | --- |
| `0x52` | **Bootloader version check** | Sent first thing to confirm you're talking to the bootloader. Device responds with version info. |
| `0x54` | **Device ID** | Returns the device's unique identifier (presumably from the nRF52840's DEVICEID register). |
| `0x58` | **Current album title** | Returns the title string of the currently-loaded album on the eMMC. Useful for verification (does the device have the album you think it does?). |
| `0x70` | **Set GPREGRET magic** | Followed by a payload byte. `0x70 0x01` sets GPREGRET to `0x1A96`, telling the bootloader to enter "parser 2" mode on next reboot. |
| `0x50` | **Trigger reboot** | Causes the device to reset; the next boot reads GPREGRET and chooses the appropriate parser. |
| `0x37` | **Stop playback / reset sector pointer** | Halts the audio engine (if running) and resets the read pointer. Used before bulk upload. |
| `0x39` | **Stream stem data packet** | Sends one 136-byte data packet to be written to the eMMC: 4-byte header + 4-byte sector address + 128 bytes payload data. Four such packets fill one 512-byte native eMMC block. |
| `0x43` | **Read write counter** | Returns how many bytes have been written so far. Used to verify upload progress. |
| `0x51` | **System reset** | Hard reset, including USB re-enumeration. Used at end of upload to make the device re-read the album from eMMC. |
| `0x66` | **Verify album valid** | Checks that the album image on eMMC is well-formed (correct header, etc.). |

## Two parsers: parser 1 (default) and parser 2 (upload mode)

The bootloader has **two operating modes**, selected via the **GPREGRET** (General Purpose Retained Register) which survives a soft reset:

- **Parser 1** (default, GPREGRET = 0 or any value other than 0x1A96): Standard bootloader. Can flash firmware, verify versions, etc.
- **Parser 2** (GPREGRET = `0x1A96`): Album upload mode. Can write to the eMMC via the `0x39` stem data packet command.

To enter parser 2:
1. Connect via CDC, confirm bootloader (`0x52`)
2. Set GPREGRET magic (`0x70 0x01`)
3. Trigger reboot (`0x50`)
4. Wait for USB re-enumeration
5. Confirm parser 2 active (re-send `0x52`, check response)

## Sequence for flashing a new app firmware

The minimum sequence (without album upload) is:

```
1. Trigger Track1+4+USB-C
2. Wait for CDC enumeration (~1 second)
3. 0x52       — Bootloader version check; confirm we're in bootloader
4. (firmware flash commands — exact protocol details TBD; see below)
5. 0x51       — System reset
6. Device reboots into the new app
```

The exact firmware-flash commands are not fully documented in the public material. Reading `theunflappable`'s Python implementation (`test_bootloader.py` shared in Discord #firmware 2026-05-08) is the practical reference. The Solderless web tool also implements these commands — viewing its source code (if accessible during a build) is another path.

## Sequence for uploading a new album

[Source: ericlewis, Discord #general, 2026-05-09 09:45]

```
1. Trigger Track1+4+USB-C
2. Wait for CDC enumeration
3. 0x52       — Bootloader version check
4. 0x70 0x01  — Set GPREGRET magic 0x1A96
5. 0x50       — Trigger reboot
6. Wait for USB re-enumeration into parser 2
7. 0x52       — Confirm parser 2 active
8. 0x54       — Device ID
9. 0x58       — Current album title (verification)
10. 0x37      — Stop playback, reset sector read pointer
11. 0x39 × N  — Stream stem data: N packets, each 136 bytes
                (4-byte header + 4-byte sector addr + 128 bytes data,
                 four packets per 512-byte native eMMC block)
12. 0x43      — Read write counter, verify against expected total
13. 0x51      — System reset; device reboots, re-reads album,
                comes back in parser 2
14. 0x58      — Verify album title matches new album
15. 0x66      — Verify album valid
```

A full 311 MB album = 37,993 logical sectors × 16 native blocks per sector × 4 packets per native block = **2,431,552 packets of 136 bytes each**.

At a hypothetical 100 µs per packet (USB CDC roundtrip time), that's ~4 minutes total upload. moecal1947's current Python tool [Discord #general, 2026-05-09 09:33] achieves ~0.75 KB/s = 5.5 packets/second — far below USB's theoretical capacity. The bottleneck is USB control-transfer overhead, not the eMMC.

## What's known about packet `0x39`

136-byte frame:

```
Byte 0–3:    Header (purpose unknown — likely a magic / opcode marker / command sub-code)
Byte 4–7:    Sector address (32-bit, target location)
Byte 8–135:  128 bytes of payload data
```

[Discord #general, ericlewis, 2026-05-09 09:45]

**Critical gap:** the exact format of the 4-byte header and the **ACK / response format** for `0x39` packets is **not publicly documented**. moecal1947 asked explicitly for the byte-level framing [Discord #general, 2026-05-09 09:47]; ericlewis acknowledged he probably has it (*"Prob."*) but did not share it. See `known-unknowns.md`.

Inferred plausible header structure (subject to verification):

```
Byte 0:    Magic / opcode marker (probably 0x39 echoed, or a sub-command byte)
Byte 1:    Sub-command / flags
Byte 2-3:  Length or checksum
```

ACK protocol is presumably "send packet, wait for single ACK byte, retry on NAK." But the exact convention isn't published.

## Why packets are 136 bytes

136 = 8 (header + sector addr) + 128 (data). With 4 packets per 512-byte native eMMC block, this gives:

```
4 × 128 = 512 bytes of data per native block
4 × 8   =  32 bytes of overhead per native block
```

The 32-byte overhead per 512-byte data is ~6% — reasonable. The choice of 128-byte payload likely reflects USB Full Speed packet sizes (64 bytes per packet × 2 = 128) for efficient transport.

## Why GPREGRET-based parser selection

GPREGRET is a hardware register on the nRF52840 that survives a soft reset (preserved through `NVIC_SystemReset()`). The bootloader reads it at boot to choose the parser mode. This gives a way for **the bootloader to call back into itself in a different mode** without needing a dedicated USB command or relying on the host to time a window perfectly.

Sequence: host sets GPREGRET via `0x70 0x01`, triggers reset via `0x50`, the chip resets, reads GPREGRET, sees `0x1A96`, enters parser 2. The host then re-enumerates and continues with album upload.

`0x1A96` is presumably just an arbitrary magic value; nothing in particular about the number except that it's unlikely to occur naturally.

## What the bootloader cannot do

- **Read firmware.** No flash-read opcode. *"you cannot"* [Discord #firmware, ericlewis, 2026-05-08 17:13].
- **Read the eMMC.** Reading is the app firmware's job. Parser 2 writes; the running app reads back to verify.
- **Run user code.** It's a strict bootloader; only its own command set, no arbitrary code execution exposed.
- **Bypass APPROTECT.** APPROTECT still applies on next boot if your custom firmware doesn't clear it. (For most custom firmware development you neither care about nor need to touch APPROTECT.)

## Safety considerations

**A bad firmware can brick the device.** If your firmware:

- Doesn't bring up USB,
- Doesn't respond to the function-button longpress for shutdown,
- Disables the audio system entirely without providing another visible state,

then the only recovery is to re-enter Track 1 + Track 4 + USB-C from a cold start — which works because the bootloader is still in slot 0 unchanged.

If your bad firmware **also corrupts the bootloader** (e.g., your firmware extends into the bootloader region), recovery requires SWD. So **don't overwrite the bootloader region** (`0x00000000`–`0x00020000`) — keep your firmware in slot 0 (`0x00020000`+).

tkt1000's reminder: *"If you load a bad firmware you can brick your device!"* [Discord #general, 2026-05-09 12:15].

## solderless.engineering and standalone tools

`https://solderless.engineering/` and `https://solderless-engineering.pages.dev/` use Web Serial in the browser to talk this protocol. As of 2026-05-09 the tool is **offline for an update** [Discord #news, tkt1000].

Alternative tools:

- **`theunflappable`'s `test_bootloader.py`** — Python implementation of the bootloader protocol [Discord #firmware, 2026-05-08]. Does not include album upload. Use this to write your own client.
- **TimK's wiki page** at `github.com/timknapen/SP-1-dev/wiki/Bootloader` is the canonical reference. Verify it's current; ericlewis trusts it: *"I bet $10k @TKT made a better version of my notes."* [Discord #general, ericlewis, 2026-05-09 09:45]

Standalone client implementations would be useful in addition to the web tool. virtualflannel_46386 asked if anyone saved the source [Discord #firmware, 2026-05-10 18:40] indicating no public archive of solderless's client code exists yet.

## Where to go next

- For the actual album-upload data path → `16-usb-upload-protocol.md`
- For physical bootloader-trigger details → `04-debug-interfaces.md`
- For the eMMC sector layout that uploaded data targets → `08-emmc-storage.md`
- For the on-disk format of stems → `09-audio-format-spec.md`
- For known unknowns (packet framing for 0x39) → `known-unknowns.md`
- For the broader debug interface picture → `04-debug-interfaces.md`
