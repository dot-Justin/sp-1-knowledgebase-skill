# FAQ — Physical Disassembly

**Q: Do I need to open the SP-1 to flash custom firmware?**
A: **No.** The Track 1 + Track 4 + USB-C bootloader trigger works without opening. Opening is needed only for SWD development debug, eMMC test point access, or hardware mods.

**Q: How is the SP-1 held together?**
A: Glue and friction. No screws. The white plastic piece around the play/function buttons is glued in one spot; the case halves are friction-fit with adhesive.

**Q: What tool do I need to open it?**
A: **Hairdryer** (not heat gun) + **plastic spudger / pry tool**. That's it. See `23-physical-disassembly.md`.

**Q: Why is the white plastic piece hard to remove?**
A: It's adhesive-bonded. Heat softens the adhesive; gentle prying lifts it. Multiple heat-and-pry cycles is normal. **Don't yank** — TimK reports it was "stressful" even for him.

**Q: What temperature should the hairdryer get to?**
A: Generic hairdryers max around 60-80 °C, which is plenty to soften the adhesive without damaging plastic. Heat guns set above ~120 °C risk melting plastic or discoloring the aluminum.

**Q: What's the FPC connector?**
A: The 19-pin, 0.3 mm pitch flex connector between the main PCB and the user-facing flex (buttons, LEDs, jack). Manufacturer: **Omron XF3B-1945-31A**. Fragile — handle with care.

**Q: My SP-1 stopped working after reassembly — what's wrong?**
A: **Reseat the FPC.** 90% of "I bricked it" cases are an FPC that looks plugged in but isn't fully seated. See `corrections.md` (PedalsandChill's example).

**Q: How do I disconnect the FPC?**
A: 1) Lift the ZIF lever (small dark plastic tab on the connector); 2) Slide the FPC out laterally (don't pull straight up).

**Q: Can I replace the battery?**
A: Yes — generic 3.7 V LiPo with matching connector. Specific TE-spec battery probably not available; salvage from another unit or use a same-size generic.

**Q: Can I replace the speaker?**
A: Probably hard without aftermarket parts. The speaker is small + glued. Headphones via TRRS jack still work if the speaker fails.

**Q: Should I wear an ESD strap?**
A: Recommended for sensitive electronics. Not strictly required for casual work on a non-static-prone surface.

**Q: How long does disassembly take?**
A: 10-30 minutes the first time; 5-10 minutes once you know the technique. The white plastic piece is the time-consuming step.

**Q: What about reassembly?**
A: Reseat the FPC carefully, clean adhesive residue from where the white plastic sat, press the white plastic back firmly. The adhesive often re-sticks without additional glue.

**Q: What if I damage a trace on the PCB?**
A: Repair with conductive ink or fine bond wire is possible but difficult. Most people accept the loss at that point.

**Q: Can I open it while it's powered on?**
A: **Don't.** Always disconnect USB and let the device sleep before opening. Hot-unplugging the FPC under power risks damage.

**Q: Is there a video walkthrough?**
A: Not formally curated. Search the Lines thread (`sp-1.dotjust.in`) for "teardown" and the post numbers 5-100 range. Multiple users have posted teardown photos.
