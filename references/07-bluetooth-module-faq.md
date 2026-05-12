# FAQ — Bluetooth Module

**Q: What Bluetooth chip is in the SP-1?**
A: **Cypress / Infineon CYBT-353027-02**. Bluetooth 5.0 Classic + LE. UART-attached. Built-in firmware. See `07-bluetooth-module.md`.

**Q: How is it wired?**
A: UART0 at 115200 baud with hardware flow control. TX P1.02, RX P1.04, RTS P1.01, CTS P1.03. Reset on P0.10 (NFC2 reclaimed as GPIO).

**Q: Does any public firmware use it?**
A: **No.** ericlewis's `sp1-midi` BSP defines the pins but doesn't bring up a stack. TimK has not investigated it. Only emvee1968 claims a working implementation, and emvee's code is unreleased. See `known-unknowns.md`.

**Q: emvee1968 said BT was easy — was it?**
A: *"Not too bad, a few hours with Claude Code"* [Discord #firmware, emvee1968, 2026-05-08 23:28]. Take that with a grain of salt — emvee was using Claude Code heavily and the time estimate reflects automation more than the actual complexity. Expect more than a few hours if you're driving it manually.

**Q: How would I bring up Bluetooth myself?**
A: 1) Build an HCI-over-UART driver at 115200 baud with RTS/CTS (Zephyr has `CONFIG_BT_HCI_UART`). 2) Use Zephyr's Bluetooth host stack. 3) Implement the profile you want (A2DP source/sink, HFP, SPP). 4) Sequence the reset on P0.10 properly. See `07-bluetooth-module.md` "Implications".

**Q: Is the BT module audio source or sink?**
A: Up to you in custom firmware. The CYBT supports both directions. Stock TE firmware's role isn't publicly documented but is presumably audio source (stem player → BT headphones / speaker).

**Q: What's P1.05 used for?**
A: TimK's pin header defines it as `PIN_CY_SPI_CSN` — a possible secondary SPI interface to the BT module. Not used by any public firmware. Treat as documented-but-unused. See `03-pcb-and-schematic.md`.

**Q: Does the BT module use I²S for audio?**
A: Unknown. Most likely audio is handled in software on the nRF and sent as A2DP packets over HCI/UART. But the CYBT module also has a PCM audio interface in its datasheet, and it's plausible some pins go there. No public reverse engineering of the module's audio side yet.

**Q: How do I pair from stock firmware?**
A: TE's stock firmware uses some button combo that's not publicly documented at the protocol level. emvee1968 used **vol+ and vol-** simultaneously as a pairing entry in their custom firmware.

**Q: Will the BT module work without firmware reflashing if I don't ask it to?**
A: It's held in reset (P0.10 low) by `sp1-midi`'s default. Your firmware controls whether it ever powers up.

**Q: What does power consumption look like with BT active?**
A: Not measured by the community. The CYBT-353027-02 datasheet has the spec.

**Q: Where to go for help?**
A: Ask emvee1968 in Discord for guidance on their pairing implementation. Or wait for their code release. Or read the Cypress WICED-Bluetooth documentation if you want to handle the module without Zephyr's stack.
