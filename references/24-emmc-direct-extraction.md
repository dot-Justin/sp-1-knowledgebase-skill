# eMMC Direct Extraction

**Synthesized through:** Lines #846 (2026-05-06), Discord through 2026-05-11. Primary source: fishdog_'s Discord #hardware walkthrough (2026-05-10 and 2026-05-11).

This file covers physically reading or writing the SP-1's eMMC chip (Toshiba THGBMNG5D1LBAIL) **outside** the device, via either test-point access or chip desolder. Use this when you need fast read/write speeds, or when you don't want to depend on a working SP-1 to interact with the data.

For software access via the nRF52840 (read-only, slow), see `08-emmc-storage.md`. For LED bit-bang firmware extraction (slow, non-invasive), see `17-led-bitbang-dump.md`.

## When to use which method

| Method | Open device? | Desolder chip? | Risk | Speed | Use case |
| --- | --- | --- | --- | --- | --- |
| **Test point read** | Yes | No | Low — clip leads only | Medium (eMMC bus speed up to 50 MHz with 4-bit reader) | Verify content, fast dump for inspection, write album from host |
| **Desolder + USB adapter** | Yes | Yes | High — desolder + reflow at 380–420 °C | Fast (USB Mass Storage speed) | Permanent setup for chip-out-of-device work; recoverable but tedious |
| **In-circuit nRF52840** | No (if using bootloader) | No | Very low | Slow (32 MHz 1-bit) | Audio playback during normal operation; everything else use a different method |

## Method 1: Test point access

TimK described the test points exist [Discord #hardware, 2026-05-07 17:53]:

> Actually if I remember correctly DAT0-DAT3 are broken out to test points, so you could read much faster than the MCU!

This means the chip's full 4-bit data bus is exposed for external reading. You'd need:

- **An eMMC reader board** — e.g., the "TestPro eMMC Tool" or DIY equivalent. These are sold for phone repair work and accept eMMC chips at their native 4-bit interface.
- **Clip leads or pogo pins** — to connect to the test pads without permanent soldering.
- **VCC + VCCQ supplied externally** — the eMMC needs power; can come from the SP-1 itself if you leave the battery connected, or from your reader's supply.

**The exact pad locations on the PCB are not yet publicly documented** as of synthesis date (2026-05-11). TimK promised to add them to `github.com/timknapen/SP-1-dev/wiki` [Discord #hardware, 2026-05-07 17:43]. Check there first. If the wiki hasn't been updated, ask in Discord.

Procedure (once you know where the pads are):

1. Open the device (`23-physical-disassembly.md`)
2. Connect clip leads to: DAT0, DAT1, DAT2, DAT3, CLK, CMD (6 lines minimum)
3. Connect ground
4. Power the eMMC (typically 3.3 V VCCQ + 1.8 V VCC depending on reader)
5. Issue eMMC init commands (CMD0, CMD1, CMD2, CMD3, etc.) from your reader
6. Read or write blocks as needed
7. Disconnect cleanly before reassembling

Most commercial eMMC readers do the init automatically. You just have to wire the lines correctly.

## Method 2: Desolder + USB adapter (fishdog_'s technique)

For permanent, fast access, desolder the chip and mount it on a **USB-to-eMMC adapter PCB** sold for phone-data-recovery work.

### What fishdog_ did

[Discord #hardware, fishdog_, 2026-05-10 18:55–19:25 and 2026-05-11 19:08]:

- **Desolder the eMMC** from the SP-1 main PCB at **380–420 °C** with a hot-air station, on a **175 °C preheater**.
- **No reballing** required. Just buildup a layer of solder on the chip's existing pads and on the adapter's pads, then reflow into place.
- **Adapter:** an unpopulated USB-to-eMMC PCB from AliExpress / Amazon — the chip soldered onto a board that has a USB connector and the appropriate eMMC controller/passives.

fishdog_'s reported observation [2026-05-10 18:55]:

> I was just able to solder it on and plug it in and dd

i.e., once on the adapter, the eMMC enumerates as a USB block device and `dd` reads it byte-for-byte.

Adapter source — fishdog linked one specifically [Discord #hardware, 2026-05-10 19:22, link unavailable at synthesis date]. esko8444 provided a working AliExpress alternative [Discord #hardware, 2026-05-11 07:02]: `https://a.aliexpress.com/_EG9I9jM` (verify current availability).

### Desolder process

1. **Preheat board** with bottom-heat tool at 175 °C for ~30 seconds to stabilize the PCB temperature
2. **Apply hot air** at 380–420 °C to the eMMC chip from above
3. **Wait** for the solder to liquefy (visible by chip starting to float on its pads)
4. **Lift** the chip with tweezers as soon as it's loose
5. **Clean** the SP-1's PCB pads with desoldering braid + flux

### Reflow onto adapter

1. **Buildup solder** on both the chip's pads and the adapter's pads (this avoids needing a reball station)
2. **Apply flux generously**
3. **Position** the chip on the adapter with the correct orientation (pin 1 markers align)
4. **Heat from above** with hot air at ~340–360 °C (lower temperature for the second reflow, to limit thermal stress on the chip)
5. **Inspect** with magnification — should be clean joints, no shorts

### Test

Plug the adapter into a USB port. It should enumerate as a USB Mass Storage device. `lsblk` (Linux) or Disk Utility (macOS) shows the new block device.

```sh
sudo dd if=/dev/sdX of=sp1_emmc.img bs=4M status=progress
```

The full chip is 4 GB; `dd` reads it in a few minutes over USB 2.0.

### Result

You have a binary copy of the SP-1's eMMC. From here you can:

- Parse the album header (see `08-emmc-storage.md`)
- Decode audio frames (see `09-audio-format-spec.md`)
- Verify that custom album generators produce equivalent bytes
- Modify and write back (`dd if=new_image.img of=/dev/sdX bs=4M`)

### Returning the chip to the SP-1

You can reverse the procedure: desolder from adapter, mount back on SP-1 PCB. The thermal cycling is hard on the chip; expect some risk of failure on the second move. Most fishdog-style users keep the chip on the adapter as their working setup and use the SP-1 only for audio playback via the eMMC.

If you want to use the SP-1 with the chip removed, that won't work — the SP-1's firmware expects the eMMC to be present and will hang or error if it's missing.

## Method 3 (advanced): In-place reader without desolder

Some commercial eMMC readers have "ISP mode" (In System Programming) where they connect to the test points and read the chip in place without removing it from the host PCB. This is essentially the test-point access method but with a more polished tool.

Examples: UFI Box, EasyJTAG, Medusa Pro. These are phone-repair tools that work for any eMMC accessible at the chip's data lines.

The SP-1's test points make this feasible. Pinout-wise, you'd need to map each tool's expected pin assignments to the SP-1's test points.

## Why this is non-destructive

For both Method 1 (test points) and Method 2 (desolder + adapter), the audio content on the eMMC is preserved. You can:

- Read the original album for archival purposes
- Reverse-engineer the byte layout against the audio format spec
- Write a new album and put the chip back

The only loss is **the SP-1 unit itself temporarily becoming a doorstop** during the work. Once the chip is back in place (or replaced with a programmed equivalent), the device works again.

## Why this is important

The SP-1's encrypted-firmware-doesn't-encrypt-storage architecture is a fundamental design choice. APPROTECT only protects the nRF52840's flash — the eMMC is unprotected. **Any user with basic SMD rework skills can read the entire audio dataset.** This is why the Kanye album stems can be extracted (and is the implicit reason the community doesn't distribute them).

For custom firmware authors, this means:

- Don't put secrets on the eMMC; they're trivially readable
- If you implement custom album encryption, do it at the firmware level (decrypt during playback) — but this is a one-way arms race against a physically-capable attacker

## Where to go next

- For physical disassembly to access the eMMC → `23-physical-disassembly.md`
- For the eMMC format on disk → `08-emmc-storage.md`
- For audio frame decoding once you have the bytes → `09-audio-format-spec.md`
- For non-physical firmware extraction → `17-led-bitbang-dump.md`
