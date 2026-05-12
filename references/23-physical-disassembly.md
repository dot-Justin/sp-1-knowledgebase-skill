# Physical Disassembly

**Synthesized through:** Lines #846 (2026-05-06), Discord through 2026-05-11.

Opening the SP-1 is not strictly required for firmware development (the Track 1 + Track 4 + USB-C bootloader trigger handles flashing). But you'll need to disassemble for: adding an SWD header, accessing eMMC test points, eMMC desolder extraction, hardware mods, FPC inspection, or any physical investigation.

This file covers the disassembly procedure, the common failure modes, and how to put it back together. **Read all of it before starting.** The FPC is fragile and rework opportunities are limited.

## What you need

- **Hairdryer or low-power heat gun** (NOT a high-temperature heat gun for desoldering — too hot will melt plastic). Hairdryer is preferred.
- **Spudger / plastic pry tool** — to coax glued components apart without scratching aluminum.
- **Soft work surface** — to avoid scratching the device.
- **Patience.** Heat-and-pry cycles, not brute force.

You do **not** need a screwdriver — the SP-1's case is glued together, not screwed.

## Step-by-step

### Step 1: Remove the white plastic piece (function / play button area)

This is the only difficult step. The white plastic piece around the play and function buttons is **glued in one spot** [Discord #hardware, steellll, 2026-05-06 23:02].

Procedure (synthesized from Discord #hardware 2026-05-06):

1. **Apply heat** with a hairdryer for ~30 seconds to soften the adhesive. Hairdryer (not heat gun) — *"those shouldn't get hot enough to damage the plastic"* [tkt1000, 2026-05-06 23:13].
2. **Apply gentle pressure** with a plastic pry tool to lift the piece. Do NOT yank — *"be sure not to yank the crap out of it, I bent my first unit's plastic out of shape while trying to brute force it"* [itditry, 2026-05-06 23:54].
3. **Repeat heat-and-pry cycles** as needed. Multiple cycles is normal — TimK reports several cycles were needed: *"I pulled it off with my hands after a bunch of heating/ cooling cycles but it was a stressful experience"* [tkt1000, 2026-05-06 23:11].

Once the white piece is off, you can see the rest of the construction.

### Step 2: Subsequent disassembly

After the white plastic piece is removed, *"the teardown was pretty easy"* [tkt1000, 2026-05-06 23:12]. Detailed photos from the Lines thread show:

- The remaining case halves split with light heat + spudger
- The main PCB lifts out once the case is open
- The user-facing flex (with buttons + LEDs + headphone jack) connects to the main PCB via the FPC

[Detailed step-by-step photos exist in the Lines thread; the agent-friendly archive can be searched for "teardown" or specific post numbers in the 5–100 range.]

## The FPC connector — handle with care

The FPC (Omron XF3B-1945-31A, 19-pin, 0.3 mm pitch) is the most fragile part. Disassembly often requires disconnecting and reconnecting it.

**To disconnect:**
1. Identify the ZIF lever on the connector (small black or brown plastic tab)
2. **Gently** lift the lever to the open position
3. **Slide** the FPC ribbon out — don't pull straight up

**To reconnect:**
1. Make sure the lever is in the open position
2. **Slide** the FPC fully into the connector (it should sit flush)
3. **Lower** the lever to clamp the FPC

**Common failure mode:** the FPC looks plugged in but isn't fully seated. Symptoms: device powers on but no buttons work, or no display response. **Reseat the FPC before assuming you have a hardware failure.**

PedalsandChill's classic example [Lines #118, ~April 2024]:

> Also, just bricked one of my stem players after putting it back together
>
> *scratch that I just fixed it guess ribbon cable was slightly bent or something*

This kind of "I bricked it" → "I just had to reseat the FPC" pattern is the most common false alarm in SP-1 disassembly. Always reseat the FPC before despairing.

## What you can do once inside

### Solder an SWD header

The SWD pads (SWDIO, SWCLK, GND, VDD) are accessible on the main PCB. **Exact locations not yet publicly documented** as of synthesis date — TimK promised to add them to `github.com/timknapen/SP-1-dev/wiki` [Discord #hardware, 2026-05-07 17:43]. Check the wiki or ask in Discord.

Solder a 4-pin header to the pads. Use a low-profile right-angle header so the case can still close.

### Access eMMC test points

DAT0, DAT1, DAT2, DAT3, CLK, and CMD lines are broken out to test points [Discord #hardware, tkt1000, 2026-05-07 17:53]. Connect clip leads or pogo pins for in-circuit eMMC read/write.

Again, **exact pad locations are pending wiki update** as of synthesis date.

### Desolder the eMMC chip

If you want to drop the eMMC onto a USB adapter for fast read/write, see `24-emmc-direct-extraction.md`.

### Inspect / replace components

Common reasons to inspect: failed eMMC, damaged FPC connector, dead battery. Replacement parts:

- **Battery:** Generic 3.7 V LiPo with the appropriate connector. Specific TE-spec battery is unlikely to be commercially available; salvage from another SP-1 or use a same-size LiPo from any vendor.
- **FPC:** Custom; not commercially available. If the FPC is damaged, the realistic recovery path is a donor device.
- **Speaker:** Tiny, glued; replacing is hard without aftermarket parts. Headphones still work via TRRS jack if speaker fails.

### Hardware mods

steellll's plans [Discord #hardware, 2026-05-06 23:01]:

> We have actually already talked about the full plan of the first modification. We talked over via my personal email because I had questions :) I have been talking to some different local places that offer anodization to compare and see if would even be worth doing it myself or just having them do the batch of aluminum parts from the tear down once my second unit arrives

So anodized-color aluminum case mods are in the community plans. Other obvious mods: 3D printed enclosures, expansion via header-attached daughter boards, etc.

### Add I²C / SPI / GPIO daughter boards

Solder header pins to unused nRF52840 GPIOs (the device has many unused pins; see `03-pcb-and-schematic.md`) and run a flex cable out to your daughter board. Risks: trace damage when soldering, EMI from external wiring, mechanical strain on the case-internal joint.

## Reassembly

1. **Reseat the FPC** (double-check it's fully inserted and the lever is down)
2. **Clean adhesive residue** from where the white plastic piece sits if it's flaky
3. **Reapply the white plastic piece** — the adhesive often re-sticks without additional glue: *"the adhesive did dissipate a bit but im very sure that that would've stuck back on even without additional adhesive"* [Discord #hardware, itditry, 2026-05-06 23:53]
4. **Press firmly** to bond
5. **Test** — power on, verify buttons + LEDs work

If the white plastic doesn't restick, a small amount of contact adhesive or 3M VHB tape works. Don't use superglue — it'll be obvious and will fog the plastic surface.

## What to avoid

- **Brute force** — heating cycles work; pulling hard breaks plastic
- **Heat gun on high** — melts plastic and discolors the aluminum
- **Soldering near the FPC connector** — heat damages the plastic body of the connector
- **Connecting / disconnecting FPC under power** — always power down first
- **Working without a soft surface** — scratched aluminum can't be reanodized cleanly without specialist tools

## Tools that help

- **Magnifying glass or USB microscope** — useful for inspecting the FPC at 0.3 mm pitch
- **ESD wrist strap** — overkill but good practice with sensitive electronics
- **Anti-static mat** — for the same reason
- **Clean alcohol wipes** — for removing adhesive residue and oils

## Recovery: device won't power on after reassembly

In order:

1. **Reseat the FPC** (90% of "bricked" cases)
2. **Check battery connection** (the battery's small connector can come loose)
3. **Plug in USB-C** and check for charging — if `nPGOOD` indicator works (the SP-1's stock firmware likely shows a charge indicator), this confirms USB power path
4. **Try Track 1 + Track 4 + USB-C** to enter bootloader — if it enumerates as CDC, the chip is alive
5. **If still dead:** open it again and inspect for damage to traces near the FPC

If you've damaged a trace, repair is difficult but possible with conductive ink or fine bond wire — search reverse-engineering forums for techniques. Most people stop here and accept the loss.

## Where to go next

- For eMMC extraction methods → `24-emmc-direct-extraction.md`
- For SWD header location (pending wiki update) → `04-debug-interfaces.md`
- For the PCB layout → `03-pcb-and-schematic.md`
- For 3D printed cases / mods → `25-3d-printing-mods.md`
- For why FPC fragility matters → `corrections.md` (the "bricked it" → "ribbon cable" entry)
