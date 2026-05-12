# Bluetooth Module (CYBT-353027-02)

**Synthesized through:** Lines #846 (2026-05-06), Discord through 2026-05-11.

The SP-1 ships with a **Cypress / Infineon CYBT-353027-02** Bluetooth module wired to the nRF52840 via UART. It is **not used by the public Zephyr BSP** (`ericlewis/sp1-midi` defines the pins but doesn't instantiate a stack). It is **not used by `ericlewis`'s public custom-firmware reference code**. The stock TE firmware presumably uses it for audio output to BT headphones, but the protocol it uses with TE firmware has not been publicly reverse-engineered.

This is the thinnest reference file in this skill. Most of what you want to know is *unknown*. The `known-unknowns.md` Bluetooth section has the full status.

## What is wired

| Signal | nRF52840 pin | DTS / pin-header reference |
| --- | --- | --- |
| **UART TX** (nRF → BT RX) | P1.02 | `pinctrl-0 = <&uart0_default>` → `NRF_PSEL(UART_TX, 1, 2)` |
| **UART RX** (BT TX → nRF) | P1.04 | `NRF_PSEL(UART_RX, 1, 4)` |
| **UART RTS** (nRF → BT) | P1.01 | `NRF_PSEL(UART_RTS, 1, 1)` |
| **UART CTS** (BT → nRF) | P1.03 | `NRF_PSEL(UART_CTS, 1, 3)` |
| **Module reset** | P0.10 (NFC2 reclaimed as GPIO) | `bt_transport_reset` GPIO, active low |
| **Possible SPI CS** | P1.05 | Defined in `SP-1-dev/src/stemplayer_pins.h` as `PIN_CY_SPI_CSN` but unused in `sp1-midi` — likely a secondary control interface, not yet exercised by public code |

UART configuration in `sp1-midi`:

```
current-speed = <115200>;
hw-flow-control;
```

[code: `sp1-midi/boards/.../stem_player.dts` lines 295–302]

So baud is **115200** with full hardware flow control. The CYBT module presumably uses HCI over UART, which is the standard Bluetooth host-controller-interface transport.

## What the chip can do (per datasheet)

The CYBT-353027-02 is a complete Bluetooth Classic + LE module:

- Bluetooth 5.0 (Classic + LE)
- A2DP / HFP / SPP / HID profiles
- Built-in firmware; communicates with the host MCU via HCI over UART (or alternative SPI/PCM/I²S transports the module supports)
- Cypress's typical embedded WICED-Bluetooth firmware

Read the public Infineon datasheet for full capabilities: search "CYBT-353027-02 datasheet."

## What works on the SP-1 today

Nothing is public. The status:

- The pins are wired (DTS confirms).
- ericlewis's public `sp1-midi` BSP **does not bring up a Bluetooth stack** [code: `app/main.cpp` does not initialize BT].
- TimK acknowledged he has never properly investigated the BT module on the SP-1: *"I never even properly looked into that"* [Discord #firmware, tkt1000, 2026-05-08 23:27].
- **emvee1968 claims** working Bluetooth pairing in unreleased custom firmware, developed with Claude Code over a few hours [Discord #firmware, emvee1968, 2026-05-08 23:26 + 23:42]:
  > Hardest part was getting the vol - + button combo pairing working.. everything else seemed pretty straight forward
- emvee1968's code is **not released** as of synthesis date; awaiting two more units before publishing [Discord #firmware, 2026-05-08 23:56].
- No public Bluetooth firmware exists that targets the SP-1 with stems-via-BT-audio.

## Implications for custom firmware

If you want to bring up Bluetooth on the SP-1 yourself, you need to:

1. **Build a UART HCI driver** — at 115200 baud with RTS/CTS. Zephyr has an HCI-UART driver (`CONFIG_BT_HCI_UART=y`) that should work after deasserting the BT reset and waiting for the module to come up.
2. **Choose a Bluetooth host stack** — Zephyr's built-in Bluetooth stack works against an HCI controller and supports A2DP source / sink for audio.
3. **Decide which profile you want** — if you want the SP-1 to act as an audio source (sending stems to BT speakers/headphones), implement A2DP source. If you want it as a sink (receiving audio from a phone), A2DP sink.
4. **Reset / boot sequence** — assert P0.10 low for ~10 ms, release, then start the HCI driver. The module's HCI reset sequence is module-specific; consult the CYBT-353027-02 reference manual.
5. **Integrate the function-button / vol button combos** — emvee1968 used vol+/vol- combo for pairing; the SP-1's analog button matrix is what surfaces these events.

The work is bounded but non-trivial. Anyone willing to do it should expect to spend more than "a few hours" — emvee1968's timeline reflects Claude Code automation more than the underlying complexity.

## Open questions about the module

See `known-unknowns.md` "Bluetooth audio module integration" for the full enumeration. Briefly:

- What firmware revision is on the CYBT module from the factory? Different revisions support different profiles.
- Does the stock TE firmware send a custom configuration command to the module at boot? If so, your custom firmware may need to replicate this or the module may behave differently than the datasheet suggests.
- Is the module's audio transport over PCM/I²S (separate from the UART HCI control), or is audio bridged through the nRF? The pins for an audio data channel haven't been clearly identified — possibly some of the unused nRF GPIOs are wired here. **P1.05 (`PIN_CY_SPI_CSN`)** suggests at least one secondary control path.
- Does the BT module have its own power-supply control that needs to be sequenced with reset?

These are answerable but not currently answered. The next move is to either (a) wait for emvee1968's release, (b) ask in Discord, or (c) read the module's UART traffic with a logic analyzer at boot when running stock firmware.

## Where to go next

- For known-unknowns and the next-move suggestions → `known-unknowns.md`
- For the UART pin definitions and configuration → `02-hardware-overview.md`
- For the analog button matrix used for vol+/vol- pairing combos → `02-hardware-overview.md` (button matrices section)
- For how the function button (NOT on the BT module — but related to power-down sequencing) works → `05-power-and-battery.md`
