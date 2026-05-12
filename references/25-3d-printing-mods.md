# 3D Printing and Mods

**Synthesized through:** Lines #846 (2026-05-06), Discord through 2026-05-11.

A handful of 3D-printable accessories have been designed by community members. None are formally curated; they live on Thingiverse / individual member archives / Discord attachments. This file lists what's been built and where to find each.

## Released designs

### nonkillwingman's belt clip

A clip-on attachment that lets the SP-1 hook to a belt or strap. nonkillwingman (Lines handle; **_isaaczz** on Discord) created this around late 2025.

[Lines ~#640 area; Discord #hardware]

Status: STL files distributed via Thingiverse or shared on request in Discord.

### ItDitry's scale model

A non-functional plastic model the same exterior dimensions as the SP-1. Useful for fitting tests, photographing without risking the real device, etc.

[Lines ~#650 area]

### 41mad4's magnetic case variant

A printable enclosure with magnetic attachment points. Specific use case unclear from the synthesized material — likely for vehicle / wall mounting.

[Lines ~#700 area]

### walkerauga's KOII compatibility plate

A printed plate that makes the SP-1 compatible with KOII (a modular synth standard / mounting system from Teenage Engineering, presumably). Released 2026-05-09 in Discord [Discord #hardware, walkerauga, 2026-05-09 01:00]:

> I designed and printed this plate for KOII compatibility. Let me know if yall want the STL. Came out flawless on my end.

Photo attached to the Discord message. STL files shared on request.

### isacvr67_nolife (_isaaczz)'s padded pouch v3

A padded pouch / case for transport. Posted 2026-05-10 [Discord #hardware, isacvr67_nolife, 2026-05-10 20:31]:

> I never mentioned this in my other post on lines but I made a v3 of the pouch with padding

Likely uses TPU or flexible filament for the padded interior.

## In-development mods (not yet released)

### steellll's anodized aluminum case mod

Plans for anodizing the SP-1's aluminum case in custom colors [Discord #hardware, steellll, 2026-05-06 23:01]:

> I have been talking to some different local places that offer anodization to compare and see if would even be worth doing it myself or just having them do the batch of aluminum parts from the tear down once my second unit arrives

This is a destructive mod — requires disassembly + chemical stripping of the existing anodization. Not a 3D-printed part but a physical-modification approach.

### steellll's storage mod (post-firmware)

[Discord #hardware, steellll, 2026-05-06 23:04]:

> in the long run, after firmware and software has been released, I also have plans for storage :)

Specifics unclear — possibly a larger eMMC swap, a microSD adapter, or external storage. Currently theoretical.

## How to design new mods

### Dimensions

The SP-1's exterior dimensions are well-known in the community but not formally cataloged in this skill. Reference: photographs in the Lines thread or the existing 3D-printed designs (use their bounding boxes as proxies).

### FPC respect

If your mod requires opening the device, plan around the FPC connector. Mods that mount on the *outside* of the case (clips, pouches, anodizing) don't touch the FPC. Mods that insert *between* layers (custom case halves) need to either preserve the FPC connection or include their own data routing.

### Material choices

- **PLA / PETG** — for non-thermal-load accessories (pouches, clips, brackets)
- **Nylon** — for hinges, mechanical parts
- **TPU** — for flexible parts (pouch padding, gripper inserts)
- **Carbon-fiber composites** — for rigid, lightweight cases

The SP-1's aluminum is rigid and conductive — avoid metal-impregnated filaments that could short pads on contact.

### Sharing

Common patterns from the existing designs:

- **Discord attachments** — fastest way to get feedback from community members
- **Thingiverse** — broader audience but slow update cycle
- **GitHub** — for STLs + CAD source (FreeCAD / Fusion 360 / etc.). Few community 3D files are in GitHub yet.

## Why mods are interesting

Beyond aesthetics, mods address real limitations:

- **Carrying** — the SP-1 has no built-in clip / strap; pouches and clips fill this gap
- **Mounting** — KOII compatibility lets the device join Eurorack/modular setups
- **Aesthetics** — anodized colors let users customize a uniformly-black device
- **Protection** — pouches protect the aluminum surface from scratches

None of these are gating issues for firmware development, but they help the SP-1 fit into music-production workflows.

## Where to go next

- For physical disassembly (required for some mods) → `23-physical-disassembly.md`
- For PCB layout (if your mod needs to know component positions) → `03-pcb-and-schematic.md`
- For getting in touch with mod designers → `26-community-and-authority.md`
