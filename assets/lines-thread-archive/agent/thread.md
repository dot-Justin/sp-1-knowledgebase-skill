# TE Stem Player — Complete Thread Archive

**846 posts** · **179 participants** · **Apr 2024 – May 2026**

---

---

## Post #1 by PedalsandChill

*Staff note:* this thread has been closed due to continual issues with problematic activity that has been contrary to the [Forum Rules](/guidelines) and [Terms of Service](/tos), as well as that activity's interference with the ability of project maintainers to coordinate development efforts. They have setup an external platform for technical and community discussion, so visit the project's Discord for all updates, inquiries, releases, support, and development: https://discord.gg/y4V6VfHYck

----

Please read this post!
Updates and current status of the project will be added at the bottom and most questions should be answered here.

Recently acquired the Teenage Engineering Stem Player prototype. Unfortunately, it only has Ye’s Jesus Is King album on it. I’m wondering if there’s anyone in the lines forum that might be able/willing to help (whether in providing tips or other possible methods) in trying to modify it to load custom stems.
Thanks!


[grid]

![IMG_6653|375x500, 75%](upload://9D3v0Wnt1iLxY6XNxRXVyZCKhFp.jpeg)

![IMG_6654|375x500, 75%](upload://4q76jlJTYPoBcFHVRTo5KLgOC3x.jpeg)


[/grid]

adding user guide @instantjuggler shared on the OP-Forum


[grid]

![IMG_6668|375x500, 75%](upload://zYQGDabhnCiwQ6WPz7M2kCv1qvf.webp)

![IMG_6669|375x500, 75%](upload://oSMbkUFU4hwAQZBWW1mm4jaFqjn.webp)

![IMG_6670|375x499, 75%](upload://bi2rsd94QRZuQo4PI7wtkFqlEUK.webp)


[/grid]

Hopefully this download link works. But here’s a text copy.
[TE Stem User Guide.docx|attachment](upload://cbFQDhNysx91xaUHBWabFZFzCvV.docx) (15.2 KB)

[TE Stem User Guide.txt|attachment](upload://gCeD5M94r9yrSdsDiHUjhAwM2Uz.txt) (3.3 KB)

![412309503_747177837285286_2992001639406901420_n|690x350, 75%](upload://amWPiAg6KotIXxWHDNOlPcshScj.jpeg)

Other docs found.


[grid]

![IMG_8855_2|321x500, 75%](upload://8C6L89x0vUWpEIiKErMEhvPSxBX.jpeg)

![IMG_6681|500x500, 75%](upload://yxbnVSkAR9YqByEgvtlI5RdBHDC.webp)

![images|236x213](upload://d8FUNFqIHmqzgj1o7KodZY1S9aX.jpeg)

![Screenshot 2026-03-08 at 6.11.17 PM|690x472](upload://i7pL5URrW4K0EHv6S7CERMNQHKE.jpeg)


[/grid]

Back panel is slightly different in this. Listing artist name, album name, audio quality, and run time.


[grid]

![IMG_6682|500x500, 75%](upload://lX1AWCfeNThqPnpGv5MvGC3TqeE.webp)

![6a9d1f15ffbb4605978c4ae220d487bc|500x500, 75%](upload://aVh5qgYmPeHP5blaJWcWY0nY3mK.webp)

![7d1c2bd0c30b4752b9df0751bb3b735b|500x500, 75%](upload://ruqaUTFAo7yPuZ8V41Nw7TmRGoD.webp)


[/grid]

here’s a video of @instantjuggler doing an in-depth video over the TE Stem Player’s functionality.

https://youtu.be/zynYy35AdE0?si=7EjuZ4jMZvGwYKMi

History and timeline of TE Stemplayer gathered and written out by @maybe

https://llllllll.co/t/te-stem-player/66795/743


@B_E_N discovered a button combo that creates a USB Serial Interface for 60 seconds. Hold track 1 and 4 while plugging in a usbc cable.

https://llllllll.co/t/te-stem-player/66795/556?u=pedalsandchill


## Updates

TE Bootloader was cracked and a group made https://solderless.engineering/ as a way to update the TE-SP1 without having to open the device. Meaning you will be able to update it through it’s usb-c port.

https://llllllll.co/t/te-stem-player/66795/709?u=pedalsandchill

Currently there is no firmware to load yet as it is still being written.

## Community Additions

te-sp belt clip and case designed by @nonkillwingman 

https://www.thingiverse.com/thing:7312090

https://www.thingiverse.com/thing:7313465

te-sp scal model by @ItDitry

https://www.thingiverse.com/thing:7312595
---

## Post #2 by marcus_fischer

Paging @instantjuggler 

He is the only other person I know who has one.
---

## Post #3 by PedalsandChill

Oh I've been talking with him a little as well.
---

## Post #4 by _pete

Pretty neat! Good luck with tinkering around!
---

## Post #5 by murray

if i were handed this with the given intention i'd:
- e-mail teenage engineering or find an engineer who worked on the project to help fill in the blanks
- open it up to get a sense of the hardware it's using
- look around for some kind of physical programming interface or persistent storage medium (or sdcard or etc.) or web interface (https://krystalgamer.github.io/stem-player-emulator/)
- investigate if there's an obvious place for program storage--this depends on whether it's running on a microcontroller, fpga, a cpu with embedded os, etc. 

i'd imagine down the road you might run into five options depending on what you find and in order of desirability:
- teenage engineering or engineer willing to work with you (schematics, tips, whatever)
- a dedicated storage medium for audio files that the device simply plays from--likely requiring a specific audio format
- some kind of network interface for simply replacing stems, relatively easy-going having to only having to figure out how to talk to it and write some network calls to upload new stems
- reprogram it ideally having a decompiled binary or bitstream in-hand using a convenient programming method
- replace targeted electronic components to use onboard controls and audio hardware to implement a convenient/open method for loading/playing/replacing stems
---

## Post #6 by PedalsandChill

TE emailed me back confirming they made it. But said since it wasn't released, they couldn't provide support. I did dig through the device manager and found a connection with it under USB Serial Device (COM3). Under that looking at the device property "Bus reported device description" it said "OP-Z Serial Emulator". For whatever that's worth. 🤷‍♂️
If anyone has thoughts, feel free to chime in! 😅
---

## Post #7 by murray

the op-z has different "disk modes". maybe you can try holding down some buttons when restarting the device and possibly force it into usb mass storage mode?:

https://teenage.engineering/guides/op-z/disk-modes
---

## Post #8 by PedalsandChill

I did think of that. Feel like I've tried so many different combos already. But I haven't seen any noticeable changes yet. But I'm still trying!
---

## Post #9 by Tyler

“Serial Emulator” strikes me as maybe a debugging peripheral. Which, given it’s a prototype, I’d say it’s pretty likely there is a debug version of firmware on board. Have you tried connecting to COM3 with a serial terminal like picocom or PuTTY? Worth trying a common baudrate like 115200 and seeing if you get any feedback.

Will be following this thread with interest.
---

## Post #10 by PedalsandChill

Haven't had the chance yet. But I'll be giving it a shot tonight. I think there's been quite a bit of effort on others part to "hack" OPZ. Maybe someone in that realm could offer some insight. 🤞
::update::
Didn't get the chance to try puTTY. But popped the back off again (much easier than tx6). I was going to do a full tear down, but it seems the plastic piece on the side is held in place with adhesive. I did manage to figure out how to loosen the ribbon cable. Also snapped a pic of the guts before putting the back plate on again.
![IMG_6662|375x500](upload://aNlGfGasFRx804zjfq2x4tG7Ew8.jpeg)
---

## Post #11 by robbie

I see a Cirrus [CS42L42](https://www.cirrus.com/products/cs42l42/) audio codec with a TI [TAS2505](https://www.ti.com/product/TAS2505) audio amp, a Toshiba [THGBMNG5](https://www.digikey.com/en/products/detail/kioxia-america-inc/THGBMNG5D1LBAIL/9841782) 4GB eMMC flash, and a Nordic [nRF52840](https://infocenter.nordicsemi.com/index.jsp?topic=%2Fstruct_nrf52%2Fstruct%2Fnrf52840.html) SoC.

Interestingly the nRF52840 provides BLE, so have you seen any sign of Bluetooth from it? While not identical to the OP-Z’s [nRF52832](https://www.nordicsemi.com/Nordic-news/2018/11/Teenage-Engineering-employs-nRF52832-SoC-to-link-device-and-iOS-partner-app), it’s possible the idea might have been to have a similar MIDI-over-Bluetooth iOS/Android partner app.
---

## Post #12 by PedalsandChill

Phone doesn't pick it up through any of the following apps: orthoplay, OPZ, tp7 and midimttr. 
I also tried a few nrf and a some bt sniffing apps. GATT browser, bt inspector, but the name didn't show, or it did but under a different title and I still couldn't connect.
My tv could pick it up and connect, along with my ob4, JBL bt speaker and my windows pc.
A few basic commands in Linux and the [patent](https://patents.justia.com/patent/20230050370) they submitted a while back and the [diagrams](https://image-ppubs.uspto.gov/dirsearch-public/print/downloadPdf/20230050370) makes me believe you need an app to access it. Also leaning towards this theory because of how the field system and KOII operate/update through webmidi.
I did plug a CME WidiMaster into the 3.5m jack for midi/PO beat sync on the player. Then connected my iPad via bt to the WidiMaster and tested with midiwrench and the iPad only received a clock signal.
---

## Post #13 by murray

the next useful step, to me, is to attempt to get the device into a mode/state to acquire a flash dump of the storage connected to the nRF52840 using the nordic development tools over usb. assuming there's no other evidence of an alternative programming interface, my assumption is that's how they program/populate the persistent memory at the factory. it may even be possible to decompile the resulting image and shake out some more answers.

https://devzone.nordicsemi.com/f/nordic-q-a/86416/nrf52840-flash-dump
https://infocenter.nordicsemi.com/index.jsp?topic=%2Fug_nrf52840_dk%2FUG%2Fdk%2Fgetting_started.html

https://www.rapid7.com/blog/post/2019/04/23/extracting-firmware-from-microcontrollers-onboard-flash-memory-part-2-nordic-rf-microcontrollers/

(just in case it's not clear--be really careful with these tools. from a brief scan of the nordicsemi's development resources, it's possible that teenage engineering is using custom written drivers and overwriting anything important on the flash would ~~hose~~ set back this effort)
---

## Post #14 by PedalsandChill

[quote="murray, post:13, topic:66795"]
it’s possible that teenage engineering is using custom written drivers and overwriting anything important on the flash would ~~hose~~ set back this effort)
[/quote]
From all the digging I've done on my PC, mac and RasPi, I definitely think they are using/have a custom driver on it. I have no clue how to even begin bypassing that or finding another way of communicating with it other than attempting a hardwired firmware dump. :grimacing:
---

## Post #15 by djcuvcuv

i think the TE stem player for all intents and purposes is a dead end device that can only play the one song (JIK) which is already factory loaded.

most of what ive heard over the years is that folks hoped to be able to simply load/configure but since it requires custom FW, that will pretty much never happen unless you'd like a new and harrowing hobby :D 

its an amazing relic of gear and anyone would be lucky to be the owner of one, but it can't really be expected to serve any functional purpose in ones arsenal of gear
---

## Post #16 by PedalsandChill

I agree. Definitely like having this little piece of TE history. Truthfully, I doubt I'll ever be able to swap songs on it. For me a big motivating factor isn't just changing the songs. It's also unraveling the mystique around this widely seen, but little known device. The few times I had seen clips and videos of it out in the wild, I would get so excited waiting for it's release. The style and form factor along with TE's incredible spin on functionality/creativity really hyped it up for me. I was pretty saddened when they scrapped the project.
---

## Post #17 by instantjuggler

thanks for adding the user guide- i was almost going to do it, just to have everything here in one place. let me try and dig up another file i have that's related to this and see if you want to add it to your original post!
---

## Post #18 by PedalsandChill

For the purpose of knowledge and education. Tore the stem player almost all the way down. There's quite a bit of adhesive on all the innards. Sorry the pics are kinda crumby. After I got it back together it functioned perfectly fine as before.
I'll probably end up doin the same thing I did with my [TX-6 tear down](https://llllllll.co/t/teenage-engineering-tx-6/54737/486?u=pedalsandchill). After my nerves settle bit, take another crack at it, strip it down all the way and take better pics. 
![IMG_6738|375x500](upload://lCIvrclYawQPIYKXs2zGf7wYA1l.jpeg)
![IMG_6742|666x500](upload://dobwFZeApnLEdzma4ivuLjXLdiO.jpeg)
![IMG_6741|375x500](upload://4XkLe0xln9YWpaFiFUrQAnCFF8i.jpeg)
![IMG_6744|617x500](upload://nd2d1izRiWnFJd6EGB44tJYs4Jb.jpeg)
![IMG_6745|510x500](upload://bYhnOBNnAHM2llHMNtA4hTEIbVb.jpeg)
---

## Post #19 by modularbeat

I just want to say that I love this whole thing and think it has the makings of a classic Internet epic. It’s like “The Safe” post on Reddit, except in reverse. I will not rest easy until Ye has been exorcised from this device.
---

## Post #20 by PedalsandChill

I'm pretty determined and patient with low expectations. Which seems like a good mixture for this endeavor. lol
My plan is to just be as open about every finding/detail I can (*within reason*). If anything maybe this thread can become an archive for info and someone else with more/better knowledge on this type of thing can pick up where I slow down. I'd like to find others who have managed to get one, or dedicated enthusiasts as well, then see if we can't all put our heads together and crack this thing. 
If anyone from TE happens to see this thread and feels like anonymously dropping some tips, hints, or advice I'm all ears!

::edit::
While I'm thinking of it. Does anyone have suggestions on the best place to keep and share docs/pics/files for public viewing with least chance of compromising security for myself? Safe to assume GoogleDrive would be good enough?
---

## Post #21 by instantjuggler

hey just checking in to see if you have any new thoughts- has the project reached its end or are you still pursuing different ideas? i'm pretty surprised that more players from the warehouse discovery last year haven't reached the secondhand market yet. a few are selling here and there but not many at all, prices are really up.
---

## Post #22 by PedalsandChill

Oh this is gonna be a slow going long game type project. I'm still working towards getting loading custom stems on there, I just haven't had near as much time this past month or so to work on it.
---

## Post #23 by instantjuggler

not so relevant now, but for years people have been searching for an image of the back of the paper instructions. i finally found it tonight:

![2rsh215hoe681|500x500](upload://3EvyS0AG5xsrvMuEdxxIwCMWJhq.webp)
![6a9d1f15ffbb4605978c4ae220d487bc|500x500](upload://aVh5qgYmPeHP5blaJWcWY0nY3mK.webp)
![7d1c2bd0c30b4752b9df0751bb3b735b|500x500](upload://ruqaUTFAo7yPuZ8V41Nw7TmRGoD.webp)
---

## Post #24 by xmacex

This took me to some unfamiliar corners of YouTube (Gospel + "if you are a fan of Kanye West, you are a fan of yourself") but it was interesting. In general I am very interested in this idea of deconstructing what a track is, and how it is to received by the music listening audience members, here in the 21st century. I mean, a linear music file is a pretty strange thing to put music into – we had a discussion about it in one of the tracker threads a few years ago, didn't we? Some net labels (Thinner, AutoPlate) also released music as Traktor files.

Anyway, this kind of a stem player would be a lot of fun with 2/4 on-grid drum grooves in the 135-140 range. A pocket schranz machine. Kind of like a four-track *oooooo* script with hands-on controls.

I wanted to ask how the stem player version compares to the normal release of the album? If one were to play the stem player version from beginning to end and not mess with any of the controls, would this effectively be the same as listening through the normal release?
---

## Post #25 by instantjuggler

i made a video document of all the features from the guides collected here in this thread:

https://youtu.be/zynYy35AdE0

and @xmacex in regards to your comments- yeah there's been a bunch of examples of artists releasing music and letting the audience kind of have the final say in how it sounds in the end. to pick a random one for fun, apparatjik distributed stems online and let anyone remix them, continuing this for several rounds until they themselves did a final remix which was once again released as stems. 

you mention something like a four-track oooooo script with hands-on control and that's exactly right- this is once again one of those things where the whole is more than the sum of its parts somehow. it doesn't have all the functionality in the world, but it packs a ton in such a tiny space. the physical controls plus the immediacy of the work flow makes it really compelling to play with. its really hard to communicate just how nice it is to use and how absolutely tiny it is in person. 

and yeah, just letting the album play through is pretty much the exact same thing as listening to the album i'd say. however, because there's just enough granular control and just enough effects built in, i'd say its pretty easy to generate completely foreign soundscape-esque loops. my plan for now is to make a live set around first building a loop on the stem player and then layering over top of it with other machines. because the album features vocals, and the player has reverse and speed controls, plus some limited loop point features, you can get really weird vocal sounds that don't exactly sound like vocals. there's a great feature on the device in that you can set up a framework- your loop length, direction, speed, which stems you want muted, and then unique effects on each stem... and then gently scrub through the song with that framework intact. you get these moments of vocal lines phasing into and out of the loop length. so you can dial in just the start of words, or just the end of a syllable. 

and at the end of the day, some of the drum production on the album isn't bad. you can just mute out the other layers, set a short loop point, and have some nice drums and put effects over them. and yeah i get it, you could anyway have a little drum loop with a million other devices and by splitting the stems out online from the original album (or probably at this point just find the original stems that have leaked). but for me personally i really like this sandboxed approach to creativity at the moment. its like hey here's a little box with an album on it, you can't change that, but here's a few things you can change. so now go to work on that and build from there. its a fun game for me at the moment at least!!
---

## Post #26 by monomefisher

Hello, what power charger do you use?
---

## Post #27 by instantjuggler

I've just been using a regular apple USB wall adapter
---

## Post #28 by monomefisher

OK, thank you, for safety I choose 5v1a
---

## Post #29 by ge.claut

Hello there!
I just stumbled across this conversation while looking for a way to change the files on the TE Stem Player. Is there anything new?
Would love to get some other songs on to this amazing device...
---

## Post #30 by OGProgrammer

There HAS to be a way to put this thing into USB mode or something.

Update:

![IMG_0770 Medium|375x500](upload://8OLlZvlu7gELwE7bA1Z7lz99iXK.jpeg)

My back seems to have diff writing than the video, I think I have F&F one? 

I'm on a quest here to find ways to upload new tracks. Contacted TE - dead end (they said to reach out to kanyewest.com LOL), hit up ppl at TE on linked in - stonewalled

Msged Yeezy support, I'm sure I'll never get a response...

Catch me on X or IG if you have one and want to collaborate on finding a way to load more files.
---

## Post #31 by PedalsandChill

What's your username on IG?
---

## Post #32 by instantjuggler

yeah TE has even publicly denied they ever even made this device lol. guessing the NDA's on that project are beyond reality. i know of one effort going on right now to crack the device. i'll check out your insta handle when you respond here and try to hook you all up with each other...
---

## Post #33 by Andromax

Hey. Learned about this project a few weeks ago. Just wanted to say that you're all doing some incredible work! This product is so fascinating and I hope you find a way to crack it soon or later. Keep it up!
---

## Post #34 by instantjuggler

Someone just posted the internals over on Reddit: https://www.reddit.com/r/teenageengineering/s/SeL9FlhrI2

[grid]
![IMG_9659|361x500](upload://2giV4idyrIP7hvu5EvyTBpPCT3d.jpeg)
![IMG_9658|361x500](upload://xIl830WuThElFZUZUGsbP46ZTg4.jpeg)
![IMG_9656|361x500](upload://7TAz4KNVS08yGQKoG5bk6OMa1Aa.jpeg)
![IMG_9655|361x500](upload://gOt6xIHbsWgq1u9GAWXoD9C8XhW.jpeg)
![IMG_9657|361x500](upload://rIUN19aSGzvliRGivXVdrdI2AOM.jpeg)
[/grid]
[grid]
![IMG_9654|361x500](upload://wUCO4i5i4jun7ZPwzBH5L8kcz5w.jpeg)
![IMG_9653|361x500](upload://pw2lor33QvoE1I4V7LgrftL4IfM.jpeg)
![IMG_9652|361x500](upload://rjSlCbmSXw4zOzExCviG4DK5uCb.jpeg)
![IMG_9651|361x500](upload://3id8RhqcYAJVV7lCKrNpZgcxxvU.jpeg)
![IMG_9650|361x500](upload://nOs7gZAPuNBWPetPyzW0TeoFlyi.jpeg)
[/grid]
![IMG_9649|361x500](upload://lxYBZPtZreQyYuEDNgtgXrV6RsR.jpeg)
![IMG_9648|361x500](upload://6iePOApzyF5GZbJb3iCvPMuz6NN.jpeg)
---

## Post #35 by amandwato

Looks like a batch of the Stem Players is [up for sale here](https://parisaint.com/products/yzy-2019-og-unreleased-stem-player-by-teenage-engineering-1). Seems like a pretty fair price too. Fair warning, I have not purchased it myself and have no plans to since I wouldn't have a use for the item except to try and hack it.
---

## Post #36 by Virtual_Flannel

Wow thanks for the heads up, I bought one, as a TE nerd ive always wanted to check one out, very low risk at that really fair price tbh.
---

## Post #37 by kim

Where do the stem players in the wild come from. Are they prototypes. Did TE send them out to people, testers, and those people passed them on. Or was it something else that happened. If anyone knows.
---

## Post #38 by instantjuggler

there was a friends and family release of perhaps around 200 units that had some different printing on the back and came in little boxes with an instruction sheet. a very small number of the friends and family release were sold on reselling platforms. those units got sold over and over again and eventually became pretty beat up. it makes sense that because it was a friends and family only release, not many of those people would let their units get out in the wild, perhaps to stop from not being considered a friend or family in the future. its rumored that one of the first units leaked out behind the scenes came from ye's pesonal chef who sold his stem player to a fan.

then there was 200 to 400 dead stock units found in a warehouse in L.A. a couple of years ago. those units only had the usb cable with them, no box, and different text on the back. that's what's currently being sold online, mostly through paris saint, and now because paris saint offered them at such a low price, you start to see these units popping up on ebay quite regularly. 

and since its that time of year right now, i should point out that there is a TE guide for the device that hinted at a "christmas edition" of the unit!
---

## Post #39 by KHAGE

[quote="amandwato, post:35, topic:66795, full:true"]
Looks like a batch of the Stem Players is [up for sale here](https://parisaint.com/products/yzy-2019-og-unreleased-stem-player-by-teenage-engineering-1). Seems like a pretty fair price too. Fair warning, I have not purchased it myself and have no plans to since I wouldn’t have a use for the item except to try and hack it.
[/quote]

After a moment's reflection, I finally pulled the trigger.

I'm now keeping my fingers crossed that a hack will soon see the light of day... :crossed_fingers:t4:
---

## Post #40 by Ecoustic

Perhaps against better judgement, I did the same. I love a good story and this one, with its various twists and turns, along with the players involved, is marvelous.
---

## Post #41 by Virtual_Flannel

For me it was like buying a Unicorn, no idea what im going to do with it, but its a freaking unicorn.
---

## Post #42 by beto

I think there's a 20% chance someone smarter than me will crack this, so I got one too!
---

## Post #43 by KHAGE

Sorry for the math but if I say the same thing would it make a 40% chance ? 🤔🤪
---

## Post #44 by Virtual_Flannel

I think it may require a special machine to flash a new firmware with different audio. So youd need access to this type of machine, and the ability to write a new firmware with the new audio baked it. Id suggest anyone who buys one to assume it wont be able to be updated. Id consider it a collectors item for hardcore TE fans.
---

## Post #45 by instantjuggler

i've heard of 5 teams trying to crack this device so far... 2 of the teams have failed. everyone is looking for some sort of button combo to boot it into some sort of disk mode where it would mount as a hard drive on a laptop and then you could drag and drop files at will. but i think this is just pure fantasy because that's what people want to happen. i think your theory about a custom firmware with different audio sounds more likely. but knowing from what i've heard from TE talking about their other firmwares, i don't think there's any way we'll be able to load our own music onto the device. 

i don't know anything about firmwares or interfacing with hardware... would there be any chance to start from zero and write a firmware from scratch that would just take the physical components of the device and maybe even just do something like basic file playback? but even as i write that, it sounds impossible.
---

## Post #46 by OGProgrammer

My IG is @OGProgrammer if anyone wants to collaborate on getting more stems on here. They seem to be popping up on ebay a lot lately https://www.ebay.com/itm/226516484437 - I wonder if some factory is starting to sell them off or something. Custom firmware sounds like the only route but we will keep trying!
---

## Post #47 by murray

[quote="instantjuggler, post:45, topic:66795, full:true"]
i've heard of 5 teams trying to crack this device so far... 2 of the teams have failed. everyone is looking for some sort of button combo to boot it into some sort of disk mode where it would mount as a hard drive on a laptop and then you could drag and drop files at will. but i think this is just pure fantasy because that's what people want to happen. i think your theory about a custom firmware with different audio sounds more likely. but knowing from what i've heard from TE talking about their other firmwares, i don't think there's any way we'll be able to load our own music onto the device. 

i don't know anything about firmwares or interfacing with hardware... would there be any chance to start from zero and write a firmware from scratch that would just take the physical components of the device and maybe even just do something like basic file playback? but even as i write that, it sounds impossible.
[/quote]

you're right--there's likely no button combo to get this device into disk mode. it's even possible that the filepaths for the sound data (which are probably on the 4GB emmc) are hardcoded into the firmware.

there is a chance that uploading custom firmware could be done, but it likely depends on whether teenage engineering bothered to enable APPROTECT on the board--APPROTECT is a feature on some of nordicsemi's soc's (and is available on the stem player's soc) which disables debug/programming of the system. getting around APPROTECT requires hardware fault injection detailed by a talented hacker here:
https://limitedresults.com/2020/06/nrf52-debug-resurrection-approtect-bypass/

given APPROTECT is disabled, it would be both possible to dump the code stored in flash (given a successfully connected hardware debugger, in a link i shared above) potentially reverse engineering the binary or upload custom firmware (involving writing drivers for both the audio codec and the emmc).
---

## Post #48 by Tyler

I was able to order one too, and am planning to attempt some hacking. Curious if the 5 teams mentioned earlier up are documented somewhere online? Maybe we should organize this reverse engineering effort.

Fingers crossed APPROTECT is disabled, because that would allow us to analyze the binary and potentially inject partial custom code. I think even if it is enabled though, building a new firmware from scratch should always be a possibility. There should be a mass_erase type command through OpenOCD or an nRF toolchain. Would be hard to get started on that without first working out a schematic though.
---

## Post #49 by Virtual_Flannel

A heads up for anyone with the TP7 and TX6 you essentially have a deluxe stem player when you link the devices. You can load a multitrack file with separated stems on the TP7 and then control the separate tracks with the faders on the TX6.
---

## Post #51 by opd

This might finally convince me to buy a TP-7 if we can't find a way to mod/flash the stem player...
---

## Post #52 by Blazeauga

We should find a capable software engineer online who would take on the project passionately and then crowd fund him a unit to experiment with. Being able to frontload this with whatever music would be a creative dream. Though I will say that after about two hours of tinkering, I wish the loop function had a little more depth and wasn’t limited to four beats. 

It’s disheartening that none of the manuals for these demos mention anything about data transfer. Makes me wonder if the concept was always “experience a specific album in a new way”. This could’ve expanded to other Kanye albums and maybe other artists. Similar to how TE produces pocket operator variants.
---

## Post #53 by Jonathan_Riley

I would definitely back this if it ended up happening.
---

## Post #54 by Galapagoose

just ordered one, so count me in for the hacking crew! always fun to open a TE device! writing a custom firmware from scratch would def be possible, but the challenge would be how to *load* it onto other devices without requiring hardware mods... we'll see!
---

## Post #55 by Duloz

Kanye said in a video interview at some point that the teenage engineering stem player was meant as a means to spread the gospel, hence Jesus is King.

I wonder if the collab with teenage engineering started with it being a more open device, but then Kanye got the idea to make it about spreading religion. Maybe TE was early in realizing that Kanye was truly crazy, or took issue with being used to evangelize, and severed the relationship after producing a pre-determined number of units based on contract.
---

## Post #56 by xmacex

Should this be so, some people might argue that reverse engineering and rewriting the firmware is kind of a heretic move – and I'm for it!
---

## Post #57 by PedalsandChill

I would think the more probable outcome was kano gave in to more of ye's demands and style. Maybe a better deal financially too?
Where as te has a distinct look and feel on almost everything they get involved with. You can generally always see their stylistic influence.
I definitely think te intended to expand on it. Granted I'm a noob at ble and USB sniffing, but when I was running Wireshark doing some ble sniffing on it, I saw some prompts about a mic. Which would explain the little black holes on the side. When I took it apart I didn't see any mics, but the black holes may have just been place holders considering these are all just prototypes.
---

## Post #58 by instantjuggler

from what i could find, it might also seem likely that TE couldn't get the price down to where ye wanted it for a release? the kano players were around $200 each, and i would guess that the TE stems would have been more maybe? seems crazy they could produce all those custom pieces in the player and have retail be so low when even the pocket operators were coming in more near $100 than the $50 target they initially wanted to hit...

but from what i could gather on the kano discord, alex hinted that he had started his work on the stem player and then somehow TE got involved and "stole" his .... work? designs? it was hard to tell what he meant, and then after taking that side street ye came back to kano for the release. to be honest this totally squares with ye's approach to making music- have a million people take a stab at a track, a verse, a beat, whatever. and then kind of edit it all together or go in one particular direction. i can totally see ye doing this with the stem player, he has been talking about a stem player since yeezus in 2013. virgil was all into TE, i could see ye starting a stem player with kano, then virgil turning him onto TE and since he was hyped on a stem player at the time he gets TE to make a version as well.
---

## Post #59 by xmacex

We are also talking about a one-percenter ultrarich so I am not sure if their pet projects are suspect to a achieving a price point or not or any kind of economics which might matter to mortals. Just a perspective on the speculations.
---

## Post #60 by PedalsandChill

Anyone have the stem player with audio quality and bit rate on the back? Wondering if that's the stem player he is using in this video. Because it originally starts on his song "up from the ashes" before he cycles to another song.
https://www.instagram.com/reel/CalC_jjKCNu/?igsh=MTZmc2x1YWRsaGRsaA==
---

## Post #61 by KHAGE

Just got mine from Paris Saint… currently charging… 🪫 (by the way, the USB-C socket is flimsy… the cable attaches poorly and comes off easily)

Does anyone have a pdf manual to share ?
---

## Post #62 by instantjuggler

all the online documentation about the device functionality is upstream in this thread... you can perhaps download the photos i took of the paper manual and grab out the text, its not too many pages. 

for reference my usb-c connection is just fine on my unit. hope you didn't get a defective one!!
---

## Post #63 by KHAGE

[quote="instantjuggler, post:62, topic:66795"]
for reference my usb-c connection is just fine on my unit. hope you didn’t get a defective one!!
[/quote]

I’m using the provided cable that came with the TE Stem. 

It’s still charging… I will try again with another cable… 😬🤞🏽
---

## Post #64 by Duloz

I've been doing some poking around. Used Zadig to force install various generic drivers with no luck.

I've been bouncing things off of ChatGPT with various snooping utilities, here's that conversation so far:

https://chatgpt.com/share/677f3e5b-148c-800c-829c-0b124c153d2d

note specific asks about the 5x2 contact pad on the backside of the chip.
---

## Post #65 by murray

i managed to open mine up in a way to keep the device powered while the top of the pcb is exposed (carefully prying the battery out of its adhesive, breaking a volume button off its traces in the process [whoops, not whoops]). 

probed around for obvious clock signals with an oscilloscope and tested voltage+continuity with a dmm. the traces in the silkscreened box look like either a chip footprint or, if we're really lucky, some kind of jtag header. from top-left to bottom-right i've got:
[ GND ] [GND] [GND] [GND] [3.3V]
[   ?    ] [  ?   ][   ?   ] [  ?   ][3.3V]
GND is continuity with the pcb ground, not necessarily a confirmed ground.

next step for me (or someone else) is to use a debugger to probe around for SWDIO and SWCLK. i lost my debugger in the move to oakland and my daughter is crying so that's it for me until saturday or something.
---

## Post #66 by Galapagoose

just received mine & am shocked at just how incredibly tiny this thing is. i was expecting TX6 size (which is already tiny) and it's less than half that size?! truly wild.

was looking at the pcb photos of the debug header & guessed similar to your findings. i'm surprised at the last pin having continuity with 3v3 though. was hoping it was a standard arm 10pin interface:
![arm-jtag|437x321](upload://nY5TtwxTFmAACx9kJiBvh2yLk6a.jpeg)

if the header was only setup for SWD, then i think RTCK being grounded, and TDO/TDI being N/C would make sense. but that last pin really needs to be a signal pin for my hypothesis to work out.

still charging on the bench but hope to dig in more soon.
---

## Post #67 by murray

[quote="Galapagoose, post:66, topic:66795, full:true"]
if the header was only setup for SWD, then i think RTCK being grounded, and TDO/TDI being N/C would make sense. but that last pin really needs to be a signal pin for my hypothesis to work out.
[/quote]

right there with you--i was pretty deflated to find 3.3V on that pin. it might be useful to eyeball the traces coming from the nordicsemi chip to get an idea where the debug pins might be exposed, but might be faster to simply go from pin to pin with a debugger guessing and checking combinations (and probably testing with a dmm to avoid frying the debugger). i'm pretty sure the debug pins are on the bottom right of the chip, but haven't confirmed the correct footprint:

![Screenshot 2025-01-08 at 10.13.31 PM|634x500](upload://qiBVbWnnBQRTkiujE45D1sgS4Ah.jpeg)
![Screenshot 2025-01-08 at 10.13.55 PM|690x241](upload://59xWfQYJ0G8A6AvndC0Oh5LFamt.jpeg)
---

## Post #68 by Galapagoose

did a little snooping just with voltage & continuity, and a little layered illustrator. came up with this.

![te-stem-rev1|361x500](upload://btrAF7wG3p874YglExoODq4ArU5.jpeg)

i think it looks like it is the standard 10pin hookup, though i can't trace the `nRESET` signal. it would make sense for these to be the SWD interface with the 3 test points exposed for automated firmware loading. your readings make sense to me, ie. SWDIO rests at +3v3, and SWDCLK rests at 0v (at least in this case). some chips allow you to use these pins as GPIO after boot though - not sure if that's the case with this family.

i noted the DEC1 & DEC4 pins mentioned in the APPROTECT page above in case we go that route. these are *not* confirmed, just guesses from the illustrator mockup. would need to measure voltages before trying any attack to confirm the correct attachments. thankfully the other options are immediately close by, so it should be easy to tell. the USB lines are noted as well for no good reason, other than i was having fun with the datasheet.

PLEASE NOTE: the image is reversed for the header (it's shown from the perspective of the chip-side of the board.

too late for the soldering iron, so debugger connection will have to wait til tomorrow or friday eve if noone beats me to it.
---

## Post #69 by KHAGE

[quote="instantjuggler, post:62, topic:66795"]
all the online documentation about the device functionality is upstream in this thread…
[/quote]

Did it for myself, but that might interest someone else…

[TE Stem Player manual.pdf|attachment](upload://nN9nGpY6V6ju9dHi1su051x40Tu.pdf) (497.0 KB)


[quote="PedalsandChill, post:1, topic:66795"]
![412309503_747177837285286_2992001639406901420_n](upload://amWPiAg6KotIXxWHDNOlPcshScj)
[/quote]

Is there a better quality image or pdf of this one ?
---

## Post #70 by instantjuggler

[quote="KHAGE, post:69, topic:66795"]
Is there a better quality image or pdf of this one ?
[/quote]

Unfortunately not that I have found. Though someone on eBay claims to be selling an original copy of it along with the device at the moment...
---

## Post #72 by Virtual_Flannel

Just got mine and want to see if it will fit inside a case of tictacs.
---

## Post #73 by instantjuggler

You might have to remove half of the tic tacs first?
---

## Post #74 by Sam_Muniz

My Stem Player Just arrived today, stoked to see the development happening here, now to start tinkering!
---

## Post #75 by soysos

It's funny, I have no interest in this TE/Kanye thingy, but I'm following this thread like a murder mystery series. I hope you folks solve the puzzle, I'm rooting for you!
---

## Post #76 by Virtual_Flannel

This thing is so cool, its actually driving me crazy i cant put my own music on it. Hope someone figures it out. Even if I had to send it away and pay someone to flash custom music on it, id do it in a heartbeat.
---

## Post #77 by instantjuggler

i like this hardware so much more than the kano version. and i think the kano version is super cool. but there's something about this form factor, with the hardware controls as opposed to the touch sensitive kano ones that is just a joy to use. and the size is out of this world. i'd use this hardware as the basis for an entire live set if i could load my own stems onto it...
---

## Post #78 by kasselvania

I legit think the Kano version lacked for me due to its size. Not to mention the web tool for stem splitting. I’m legit watching this project in the hopes you guys make progress and I’ll scoop on of these. Haha
---

## Post #79 by wheelersounds

Asking out of interest/product design fantasy -- what functions does it have that appeal to you? Do the faders have enough travel that you could be comfortable performing with it?
It looks like the faders are from [taowave](https://en.taowaves.com/), likely custom? It doesn't look like they offer slide pots by default on their site. I'm a lousy hardware engineer but I imagine designing something similar and maybe only 20% larger would be possible, though maybe not quite scalable...
---

## Post #80 by Virtual_Flannel

It seems the feature set for the Kano and the TE stem player are almost identical. The Kano uses a web based app to connect and send files. Im wondering if theres anything to be gleaned there for those trying to solve this thing. Could be though, that the TE device was never designed to be able to load your own files.
---

## Post #81 by Duloz

As I continue poking this thing with USB probing tools and chatGPT, it keeps coming back to figure out the jtag or find a button combo that puts it into a different mode.

 I can think of one combination that maybe nobody has tried: both rocker buttons and power on.

Since it has to be disassembled to even try, i know i haven't gotten to it.

Edit: since i am a new user and limitted number of replies...

holding the function button down til it turns off, choose a combinaton of buttens, hold them down, then power it on...

I feel like there must be SOMETHING, as i noticed that if you have it plugged in, then hold the function button to power it on, the chime that windows plays when a new device is connected does not occur until you release the function button. this seems promising that it is waiting for some other event when powering on and the button is held down.
---

## Post #82 by Galapagoose

i got stuck on the jtag header because i dont have a real programmer (just stlink’s which are more locked down and less flexible). setting up a raspi as a jlink but that wont happen for a few days.

also my thought that the unknown signal on the jtag header was nRESET seems incorrect (or perhaps is reassigned after boot) as shorting it to 3v3 or gnd did not reset the device.

i looked into the usb side of things but it’s using a custom usb class (likely to include multiple different interfaces) but i dont know how to get more info about how that works. guessing it’s a copy-paste from their other devices so that should help w reversing it, but a little beyond my paygrade.

i actually applied for a job w them years back and they were looking for ppl with Zephyr RTOS experience, so i’m guessing that’s what’s running on here. not sure if that helps decode the usb setup, but thought i’d mention!

how are people “turning on” the device to check for bootloader/mass storage button combos? i’m so used to having a dedicated “reset” button that im unsure how to work without one.
---

## Post #83 by Virtual_Flannel

Has anyone been messing around with MTP? I turned it on with an MTP app on my Macbook and it asked me to turn on MTP on my device. Not sure if this is just a generic message or if it knows the device needs to be put in a certain mode? If thats the case Im kind of guessing that maybe if there is “disk” mode of sorts its accessed, not while turning on the unit, but by a hidden menu accessed while the unit is turned on.
-Another question, does anyone know what the little pin sizes holes on each side of the unit are for?
---

## Post #84 by cole9

Seeing this makes me just want to buy one myself even more. The question is: where did you guys  purchase yours? I saw some listings on parisaint, stockx, grailed and ebay, but all of them are based in the US. I live in the EU myself and also am a student, so my budget isn't that stretchy, and paying duty fees on top the already hefty price isn't the sweetest thing. Does anybody know if there are any EU seller for the stem player?
---

## Post #85 by xmacex

One thing is to try to get Jesus is King off this thing. Another thing is to try to load ones own material on it.

I wonder, did anyone try having fun playing this thing already?
---

## Post #86 by beto

[quote="xmacex, post:85, topic:66795"]
I wonder, did anyone try having fun playing this thing already?
[/quote]

I did a quick jam for https://llllllll.co/t/jamuary-2025/70219/119, running it through the ZOIA to chop one of the songs.
---

## Post #87 by PedalsandChill

Idk, I'm not really convinced there's a button combo for it. After trying a seemingly endless combo to no results, a talking with other owners about this being a prototype. It makes most sense to me now that it is strictly a prototype device not meant to have the functionality of swapping stems and likely has a custom written driver.
I've tried all sorts of sniffing via ble and USB using any tool I could from kali Linux. I've seen hints of things, but no definitive audio files. Granted I'm pretty new and this stuff though.

@Virtual_Flannel the little holes on the side are just small holes painted black. That was one of the first things I checked out when I took mine apart. But when I was digging through stuff I saw some mentions of a mic. But it could have just been default responses to libraries or code in the device. So my guess is live sampling was planned but development never made it that far.
---

## Post #88 by Duloz

I suspect the folks looking at the jtag are going to be the heroes, if there are any heroes to be found.  

Was it you that mentioned seeing references to the op-z somewhere? My op-z broke, but I bet it uses standard drivers.

Anyone here good with java, maybe also start issuing commands like a web app might?

Edit, just for shits: anyone with an op-1 field tried connecting to it via usb and Bluetooth? Wonder if there’s any zombie code for a planned integration
---

## Post #89 by PedalsandChill

That was me. But when I dug further I'm pretty sure it was just leftover history/information from when I'd previously connected my opz to the computer.
---

## Post #90 by Tyler

![Screenshot 2025-01-09 at 7.25.03 PM|646x192](upload://7u0qxe5tiWr4eDd3TafEUFYUb5B.png)
It might be a while before I get to join in on the fun :(

---

[quote="Duloz, post:81, topic:66795"]
I feel like there must be SOMETHING, as i noticed that if you have it plugged in, then hold the function button to power it on, the chime that windows plays when a new device is connected does not occur until you release the function button. this seems promising that it is waiting for some other event when powering on and the button is held down.
[/quote]

I think this is a noteworthy observation. It sounds like holding the function button is changing or preventing the USB mode if it chimes on release. Maybe someone can try this while monitoring the low level USB activity. Does the device still power on when the button is held like this?

It's also possible that if there is a combination, it could be using a GPIO pin not attached to any physical input, but through one of the test points on the PCB.

---

[quote="Galapagoose, post:82, topic:66795"]
how are people “turning on” the device to check for bootloader/mass storage button combos? i’m so used to having a dedicated “reset” button that im unsure how to work without one.
[/quote]

Based on the previous quote, it sounds like it's just through holding the function button for a few seconds to go between power off/on. I think I saw the same mentioned on one of the manual pictures near the top of thread. Maybe it's faster to remove the battery connection and then power on via USB if the circuitry allows.

[quote="Galapagoose, post:82, topic:66795"]
i looked into the usb side of things but it’s using a custom usb class (likely to include multiple different interfaces) but i dont know how to get more info about how that works. guessing it’s a copy-paste from their other devices so that should help w reversing it, but a little beyond my paygrade.

i actually applied for a job w them years back and they were looking for ppl with Zephyr RTOS experience, so i’m guessing that’s what’s running on here. not sure if that helps decode the usb setup, but thought I'd mention!
[/quote]

That's an interesting tidbit, maybe we could dig around the Zephyr docs.

~~[Zephyr 2.7.0](https://github.com/zephyrproject-rtos/zephyr/releases/tag/zephyr-v2.7.0) looks like it was a major release circa 2021-2022, but hard to say for sure what version they were *if* they were using it. There are some bluetooth audio improvements mentioned in the patch notes which might make it enticing for a device like this, but is TE a company that learns toward long term stable or cutting-edge?~~

I think I got the dates wrong. Did this come out in 2019?

---

I'm really enjoying the intrigue surrounding the life, death, and resurrection of this device.
---

## Post #91 by murray

[quote="Tyler, post:90, topic:66795, full:true"]
[quote="Duloz, post:81, topic:66795"]
I feel like there must be SOMETHING, as i noticed that if you have it plugged in, then hold the function button to power it on, the chime that windows plays when a new device is connected does not occur until you release the function button. this seems promising that it is waiting for some other event when powering on and the button is held down.
[/quote]

I think this is a noteworthy observation. It sounds like holding the function button is changing or preventing the USB mode if it chimes on release. Maybe someone can try this while monitoring the low level USB activity. Does the device still power on when the button is held like this?
[/quote]

it waits to power on and issue usb communication until released--i monitored the usb traffic on linux using `udevadm` and all i saw were a `add` and a corresponding `bind` event. forcing several types of usb storage drivers through `udev` yielded nothing for me.

[quote="Tyler, post:90, topic:66795, full:true"]
[quote="Galapagoose, post:82, topic:66795"]
how are people “turning on” the device to check for bootloader/mass storage button combos? i’m so used to having a dedicated “reset” button that im unsure how to work without one.
[/quote]

Based on the previous quote, it sounds like it's just through holding the function button for a few seconds to go between power off/on. I think I saw the same mentioned on one of the manual pictures near the top of thread. Maybe it's faster to remove the battery connection and then power on via USB if the circuitry allows.
[/quote]

this is what i've experienced, shorter press for power-on and long-press for power-off. active power provided over usb has been kind of finicky for me--i have not tried to disconnect the battery and use usb as a constant power source, but may give that a try ~~tomorrow~~ monday evening when my debugger arrives as a bonus activity.
---

## Post #92 by instantjuggler

[quote="wheelersounds, post:79, topic:66795"]
Asking out of interest/product design fantasy – what functions does it have that appeal to you? Do the faders have enough travel that you could be comfortable performing with it?
[/quote]

for me i just like the overall combination of functions- i really prefer the physical press of a button to mute stems, as opposed to the touch based approach on kano. maybe its the physical size of my fingers which are quite large that don't very easily fit into to physical shape of the depression in the kano stem channels... its actually sometimes quite hard for me to mute stems on the kano. i think i'm also just a person who loves the tactile feels of a physical control over a touch screen type experience. so the faders are just really fun to use, but its everything combined with the size, the surprising quality and loudness of the tiny tiny speaker, bluetooth connectivity, i don't know, all of it. and for sure 100% you can achieve the type of functionality that the device has through other means. in fact i'm really looking forward to the tape! kickstarter which just finished. it has 4 tracks and some physical controls as well...

[quote="xmacex, post:85, topic:66795"]
One thing is to try to get Jesus is King off this thing. Another thing is to try to load ones own material on it.
[/quote]

i'm not sure i quite follow other than are you saying this is 2 distinctly different types of tasks? not sure what would be the purpose of taking JIK off the device without loading other content onto it?

[quote="xmacex, post:85, topic:66795"]
I wonder, did anyone try having fun playing this thing already?
[/quote]

yes! i play mine all the time! one thing i did for a series of trips last summer was to take the TE KE (the te term for the stem player) along with a tp-7 on the train- i'd make short loops on the ke that were different variations of isolated stems being looped, reversed, slowed down, or whatever and then record those with the tp-7 creating a bunch of samples from the manipulated material on the ke. 

even though i have a lot of experience of playing mine, i'm not quite there yet with one ambition which is to use it as a layer in a live set where i can mix a loop on it, such as a beat, and then build up other layers on top of it with other gear. i did use it in one live show last autumn and had one small moment with it but i didn't dare to push my luck and perform a whole set with it yet. i need more practice to be able to reliably build up something good live in the moment...
---

## Post #93 by Duloz

I ended up recording a beat i created weeks ago with the corniest vocals on the album, "Closed on Sunday," because they sounded good together when i was testing the midi tempo out
---

## Post #94 by Virtual_Flannel

![IMG_3832|690x487](upload://89RKl8gQLMDYzPmpODOiukQzmVG.jpeg)
Just a wee bit too wide!
---

## Post #95 by Duloz

I made  a pipe out of a tic tac box my freshman year of college about 24 years ago. Yeah, I'm old.

Back to the broader group: I'm weighing buying a dev kit from nordic for the nRF52840 so that I can try to interact with it from the pc side. It's $50, and I have a gift card of some amount or another coming to me from work for producing a holiday song with industry-specific lyrics with contributions from other employees... but I'm not sure how much of a help it would be. I've tried the nRF connect app by nordic on both ios and android, and the app does not see the stem player on either os, despite it being visible in a normal BT scan.
---

## Post #96 by brick

by golly lines folks, you've got me interested. i just ordered one for myself. Count me in on this hacking effort!! 
[quote="Blazeauga, post:52, topic:66795"]
We should find a capable software engineer online who would take on the project passionately and then crowd fund him a unit to experiment with.
[/quote]
I'm a computer engineer willing to take on the project passionately! Capable? That's a different question :joy:
I'm still only a computer engineering undergrad, so who knows what i'll be able to achieve. I am doing a focus on embedded systems though, so this'll be some great personal project experience.

[quote="Galapagoose, post:82, topic:66795"]
i got stuck on the jtag header because i dont have a real programmer (just stlink’s which are more locked down and less flexible). setting up a raspi as a jlink but that wont happen for a few days.
[/quote]
~~actually i think i *do* have a jtag programmer somewhere. got one in a box of assorted DIY euro stuff from a eurorack sales FB group, i may have to dig it out.~~ i think it's an stlink one, having googled "stlink jtag debugger". that said, i'll have access to equipment at my uni when the semester starts back up soon, and there's no way they won't have one. i'll ask a professor.

---
brick's usual idle yapping incoming!
---
---

[quote="Galapagoose, post:68, topic:66795"]
USB lines are noted as well
[/quote]

oh hold on... usb data lines go straight to the SoC, and the information says that the chip has a built in usb 2.0 interface. those pins (AD5 and AD7) are... *[checks document]* well it looks like they're actually AD4/6, which are, big surprise, usb D- and D+ respectively. So the USB jack is routed to the chip's built in usb data interface; it's not just tied to whatever. 

This doesn't really get us closer to anything, but it does confirm that there can be *some* way to get data from the chip to usb, the question is just how.

---

as another thought, since everyone is talking about button combos; given that the usb is tied to the SoC itself, any button combos would need to be using buttons tied to the SoC's pins. someone who has one currently could be well served by following the button traces; any that *don't* lead to the N52840 are guaranteed to not be included for any potential bootloader combo. if there are any that aren't connected to the chip, it should reduce the possibility space drastically.

---

[quote="robbie, post:11, topic:66795"]
a Toshiba [THGBMNG5](https://www.digikey.com/en/products/detail/kioxia-america-inc/THGBMNG5D1LBAIL/9841782) 4GB eMMC flash,
[/quote]

audio media is most definitely being stored on this chip. if we're only looking to put something else on it, a direct modification of the data on this chip would probably be the easiest. it's likely that the SoC is just pulling direct audio files off the chip, no idea what format they'd be in; but it's a good idea to check what is being stored on this chip and how, i'd use a file recovery tool. if someone is willing to desolder this chip and hook it up to a usb interface chip it'd be relatively easy to look into, if requiring a hot air station.
on another note, what traces are carrying the audio signal flow? does the flash chip send a digital signal straight to the audio chip or is it run through the SoC ? this would be a good thing to check.

---

[quote="Duloz, post:95, topic:66795"]
despite it being visible in a normal BT scan.
[/quote]
~~wait, so it _is_ visible in a standard BT scan? i thought it was stated further up in the thread that it *wasn't*.
we should probably start noting our models' hardware revisions, when we note what we're seeing or finding. it's possible there's major firmware revisions in addition to hardware ones between units.~~

edit: ignore anything i've said about BT. looking at the manual further up, shows that the BT functionality is for connecting to speakers or headphones. bluetooth on the device is probably configured as an audio streaming source; so any attempts to retrieve data off of this are probably going to hit a dead end.

--- 
edit 2:
[quote="instantjuggler, post:34, topic:66795"]
Someone just posted the internals over on Reddit:
[/quote]
![image|361x500, 75%](upload://3PUfbTFYcc5cIidurdUw5W4kmop.jpeg)

oh? hold on, that's strange... this is a *second* bluetooth chip, specifically [an audio transciever](https://www.infineon.com/cms/en/product/wireless-connectivity/airoc-bluetooth-le-bluetooth-multiprotocol/airoc-bluetooth-modules/cybt-353027-02/#!?fileId=8ac78c8c7d0d8da4017d0ee5e2336db4).

---
edit 3: 
[quote="Galapagoose, post:66, topic:66795"]
but that last pin really needs to be a signal pin for my hypothesis to work out.
[/quote]
[quote="murray, post:67, topic:66795"]
right there with you–i was pretty deflated to find 3.3V on that pin
[/quote]

i'm assuming you're referring to pad/pin 10 here [RESET]. it's worth noting that these sorts of devices often have their reset pins function as active low; so it would make sense that the pad ties high to 3v3. if you look at the pinout, it also technically notes the pin as "***n***RESET". it's likely tied high through an internal pullup or something, which is why you're reading 3v3.
---

## Post #97 by PedalsandChill

If you can share any basic instructions or commands on how to gather any information that would be helpful to you, I'm more than willing to help in anyway. I have two TE stem players that I could test between as well.
---

## Post #98 by brick

there's not really a ton of stuff i'd recommend doing that I can give good instructions for... I wouldn't recommend desoldering the flash chip, as there's gotta be a better way to interface with it while it's on the board. Sometimes companies and distributors can put data onto chips like that from the factory/warehouse before shipping, and if TE has done that here we're kinda SOL without desoldering the chip. Otherwise, there's gotta be data out lines from the SoC to the flash chip...

Actually, there's one thing that's pretty easy you could do. it may not be flashy, but i always start these kinds of projects by finding datasheets for absolutely every single chip you can see on the board. All it takes is taking photos of all the chips, then typing in whatever's written on those chips plus "datasheet" into Google and seeing what you get. Would be a great way to consolidate all of that information for everyone else wanting to look into this!
---

## Post #99 by PedalsandChill

Awesome! I'll try to take one of them apart again this weekend and get some raw photos of the chips and board with my actual camera rather than just my phone. Since I have two now, I won't be quite as skittish as I was the first time. After that I'll start looking up datasheets and hyper link everything here.
---

## Post #100 by Tyler

Agree with compiling a list of components in a spreadsheet somewhere!
---

## Post #101 by Duloz

Can someone confirm below:
1) Plug Stem Player into windows computer
2) Power it on

Expected Result: The computer should play the alert sound for a device being connected.

Approach 2:
1) Plug Stem Player into windows computer
2) Hold the rocker in the down position (towards the faders) 
3) Hold the function button down
4) Hold the play button down when all four lights between the function and play button turn on to indicate power
5) Release the function button

Expected result: The Stem Player will begin playing music as soon as it powers on, but the windows alert for a new device is delayed by several seconds.

Edit: Updated Instructions, it is very specific.

Edit 2: it also will not show up running lsusb in linux during this delay.

Edit 3: To clarify...

[quote="brick, post:96, topic:66795"]
as another thought, since everyone is talking about button combos; given that the usb is tied to the SoC itself, any button combos would need to be using buttons tied to the SoC’s pins. someone who has one currently could be well served by following the button traces; any that *don’t* lead to the N52840 are guaranteed to not be included for any potential bootloader combo. if there are any that aren’t connected to the chip, it should reduce the possibility space drastically.
[/quote]

I followed the traces visible on the back to the rocker buttons, the volume buttons, and then two lines that run all the way across the top of the device and disappears under the ribbon cable, probably connected to the power cable and play button? The rest of the ribbon connections trace back to the emmc. There is a chip labeled made by TI that I can't identify between the SoC and the usb port, likely handling power in.

Edit 4: google sheets document with all parts that chatGPT and I could read from the back https://docs.google.com/spreadsheets/d/1jUDMUMYHk64kJc5kIgoHwVWj_fNWHdiUgfPFLzF0ucE/edit?usp=sharing
---

## Post #102 by brick

random thought, someone should check if it can send audio over the USB-C port, try a usbc headphone dongle/ usbc headphones.
I wonder if the thing isn’t configured to act as a usb host, and that’s why people are having issues getting usb connection. The fact that later revisions can send midi clock data is interesting.
---

## Post #103 by Galapagoose

[quote="Duloz, post:101, topic:66795"]
There is a chip labeled made by TI that I can’t identify between the SoC and the usb port, likely handling power in.
[/quote]

agreed. looks like a pretty standard battery charger ~~/ switch mode power regulator~~. would handle dynamic switching between battery and usb power & typically connects to the uC with i2c (this is how the device can light side buttons indicating battery charge level).

regarding “2 antennas” above, im not sure what the 2nd is. the unit with linked datasheet would be acting as an external antenna attached directly to the uC.

next to the antenna is the speaker (i assume, never seen one that looks like this) which makes sense with power amp chip on the opposite side of the board.

going to do a little further investigation of the test points this evening. will try and write up my findings.

also, why dont we all post the things we’ve tried as we do them so we’re not repeating work. things that *dont* work are quite useful to know, and of course you can note if you felt like there was more to be explored but you didnt know what to do (a common experience for me).
---

## Post #105 by robbie

[quote="brick, post:98, topic:66795"]
always start these kinds of projects by finding datasheets for absolutely every single chip you can see on the board
[/quote]

My above post links to data sheets found from earlier photos.

https://llllllll.co/t/te-stem-player/66795/11?u=robbie
---

## Post #106 by PedalsandChill

Here's a link to some macro shots I took of some of the chips on the PCB. I took macros of the whole board (both sides) but it will take me a bit to line/match them all and merge into a single image.

https://drive.google.com/drive/folders/1R9qF1h411O-9KnFrncggWZ7xEFAMq_rm
---

## Post #107 by Galapagoose

did a tiny bit more sleuthing this eve. all the test points were at 0V except the 3 as marked (and the 3 next to the 10pin header discussed above). i believe the TP at bottom-left next to usb connector is the VBUS from USB cable (which was unconnected when i was scoping). gives me some more general ideas of what's going on architecture wise.

![IMG_6741|375x500](upload://qbU9Ag9fGPs6WFqg0INGrozhtnG.jpeg)

here's annotated idea of what the different silicon is for. most of this you probably all know by now, but figured it would be a good introduction for new entrants to this thread:

![annotated-top|375x500](upload://humUjx68RWHOUyRtneumVCTSKW9.jpeg)

1. USB-C power connects directly to uC (to power the usb-transceiver, but not the main chip), and to the LiPo charger.
2. The LiPo charger doubles as a power-source switch, to allow the device to run directly from USB power when the battery is full. Probably something like a [BQ24232](https://www.ti.com/lit/ds/symlink/bq24232.pdf?ts=1736637170535&ref_url=https%253A%252F%252Fwww.ti.com%252Fproduct%252FBQ24232).
3. However, the device won't seem to turn on when LiPo is not connected, so think of it as USB -> LiPo -> uC power.
4. nRF52840 can (and does) run directly off the LiPo output (3v ~ 4v2).
5. Audio codec has both +3v3 & +1v8 power derived from the LiPo output. There are test points for both of these sources. The +1v8 is regulated directly from the +3v3 output (likely an LDO for better noise performance).
6. Speaker amplifier has a dedicated power regulator. Not sure if it runs directly off LiPo or something else.
7. Due to limited pin count on the uC, I think they are using resistor-dividers to combine multiple switches into a single ADC channel. Likely using binary weighting [via R2R ladders as seen clearly on faders pcb], so they can detect any combination of keypresses by decoding the ADC readout value.
8. The chip between audio codec & nand memory is likely just a level translator as the codec & uC run on different voltage levels.
9. The chip to the top-right corner of codec (between codec, nand & uC) is connected to the codec's i2c pins. Likely another level translator, though different because the outputs are open-drain. There are hardware pullups on both sides of this translator so i'm guessing it's just there for a simple shifting purpose.

I am perplexed by the 2 tiny components on the FFC next to the MODE button. It's strange to me that they are on both legs of the switch which seems to me like they'd always be series and thus could be replaced with a single component. Perhaps there is something special happening here as this button is powering up/down the system. Whenever I see something where I say "that looks dumb" it's usually because I'm missing something...
---

## Post #108 by PedalsandChill

So I hadn't taken a picture of either one of those because I thought they were blank. Just looked at them again under my camera and notice the smaller one on the right had some lettering on it. It just looks like the letters pH though. I'm not sure how helpful that will be. But here's the photos. One on the left just looks like the number 314. I'll also add these to the Google drive folder.
![IMG_5242|690x459](upload://s4NTDOz8EJXEhHASHbF64yWzouf.jpeg)
![IMG_5246|690x459](upload://rGAe7R6ZiZbC0DJwOcUB9TuHK5A.jpeg)
---

## Post #109 by brick

[quote="robbie, post:105, topic:66795"]
My above post links to data sheets found from earlier photos.
[/quote]
apologies, i must have missed it among all the scrolling up and down in this thread!

aha, found it. page 51 of [the nRF52840's datasheet](https://files.seeedstudio.com/wiki/XIAO-BLE/Nano_BLE_MCU-nRF52840_PS_v1.1.pdf) tells us about the debugger.
![image|690x361, 100%](upload://xBX5L4p7viZjXmro2zGlWiudvuw.png)

i have no idea at what point in the production process these found their way out, but if these are closer to design phase than release, it's possible that APPROTECT wasn't enabled, for debugging purposes.
ah well, not much else i can do other than wait for my unit to show up.
---

## Post #110 by Galapagoose

the i2c level translator looks like it's an onsemi [NLA9306](https://www.onsemi.com/download/data-sheet/pdf/nla9306-d.pdf).

the codec translator is something like a [TXB0304](https://www.ti.com/lit/ds/symlink/txb0304.pdf?HQS=dis-dk-null-digikeymode-dsf-pf-null-wwe&ts=1736656140557). This could be a cheaper option with unidirectional lines, but for our purposes it doesn't matter.

out here waiting on the MVP with a JLink to attack the SWD(hopefully) connector!
---

## Post #111 by Sam_Muniz

I have tried this and was unable to get any sign of usb audio out from the KE
---

## Post #112 by Virtual_Flannel

Was jamming with the PO sync out the other day and realized the audio file must be a 5 track multitrack file with the sync track embedded on the fifth track. I don’t imagine the unit would have the built-in capability to detect tempo of manually added files. If there was ever supposed to be some kind of app this could have been automated.
---

## Post #113 by Duloz

Probably useless, and maybe more bad news than good. Made a script to brute force usb control transfer commands. Results are in the file.

https://drive.google.com/file/d/1vRb7t_11UlLmWAbJlNx3a8j5V9fw41Ws/view?usp=drive_link

i'm driving blind here, though, so i might just be doing this wrong.

Edit: My second unit is arriving tomorrow, so I’m less worried about bricking it at this point, if that is possible. I’m dual booting windows and Linux, so can make noise from a couple of directions. If anyone has anything they want me to throw at it…
---

## Post #114 by brick

Taking a cursory scroll through the file...

Hm, interesting. I'm not really sure what i'm looking at, and i don't know a ton about USB protocol, but there is a good section of commands in there that are successful and return arrays. The vast majority of the commands sent are returning a pipe error, which is probably worth tossing out. A handful of them are returning i/o errors, and a good  section of them are successful and returning.. arrays? not sure how that works. 
 there's a big chunk of commands that work between  
> bmRequestType: 0x80, bRequest: 0x08, wValue: 0x00, wIndex: 0x00,
bmRequestType: 0x80, bRequest: 0x08, wValue: 0xf0, wIndex: 0xf0,

? not sure what this means.
in addition, there's a giant block that returns "Invalid parameter" punctuated by one pipe error and an entity not found between
> bmRequestType: 0x81, bRequest: 0x00, wValue: 0x00, wIndex: 0x00,
bmRequestType: 0x81, bRequest: 0x0f, wValue: 0xf0, wIndex: 0xf0,

Everything past that has a pattern of seven I/O errors, and then one pipe error.
Not entirely sure what to make of this, but there are some interesting patterns in here that probably mean something to someone.

---

[quote="Virtual_Flannel, post:112, topic:66795"]
Was jamming with the PO sync out the other day and realized the audio file must be a 5 track multitrack file with the sync track embedded on the fifth track. I don’t imagine the unit would have the built-in capability to detect tempo of manually added files.
[/quote]
This is a good instinct, but i'm not sure if that's entirely the case. Some of the later revisions' manuals indicate that the sync can actually output MIDI over the TRS. perhaps a custom file format? there's no way that they're building in a conversion between a 5th click track into midi data on the chip, right? when i get mine, i'll see if the midi is compatible with my KO2.
---

## Post #115 by Virtual_Flannel

Good point, I think @instantjuggler has the Kano device, curious if anyone knows what the files look like for that device. It might be all under the hood though, and the app maybe converts the files to a proprietary file type?
---

## Post #116 by brick

Since the device so boldly states, 48khz 24bit, it's hard to imagine the files as anything other than multitrack WAV's though..
---

## Post #117 by Duloz

![Untitled|690x476](upload://nB7wvMgqYLOPSbW5eOkRv0OvUjU.jpeg)

I have failed to connect to it with a computer in this mode consistently when the stemplayer is in pairing mode (volume buttons), and it hasn't been visible to me when not in pairing mode until i switched to parrot os. It shows up whenever it is turned on period.

edit: after turning it off, then turning it back on and reconnecting, the sound is playing through my computer speakers. so that is that.

bluetoothctl returns track, album details, etc.
---

## Post #118 by PedalsandChill

I'm not sure if it's in the wave file, but it definitely sends a midi clock signal for each song. You can select if it sends bpm with po sync or midi when selecting between the 4 'modes'.  But all of that only happens through the trs jack. I've never seen hints or signs of any additional midi signals being sent out. I tested it with midi wrench and checked for sysex commands.
--Also, just bricked one of my stem players after putting it back together. So careful out there to others considering taking there's apart.-- *scratch that I just fixed it guess ribbon cable was slightly bent or something*

Something just occurred to me related to what @Virtual_Flannel said about the tracks carrying their own bpm. I think there may be something to that. 
If you're using the player in basic mode, all you have is the trem/gate effect. Mute all other tracks except one, hold mode then select the unmuted track, release mode but keep the one unmuted track going with the trem/gate effect, cycle between songs. You'll notice each song has a different bpm for the trem as you're cycling through. Sometimes the tracks on either the left or right go faster or slower. So I don't feel it's really possible that the effect is basing off a single beat division. Unless each track uploaded is somehow assigned a division of the main bpm. At that point though, maybe they could just as easily be assigned their own bpm? But idk, thoughts?
---

## Post #119 by Duloz

I should have a nRF52840 dev kit by the weekend.

edit: any tips on removing the white plastic side easily?

Edit 2: second unit has a different colored barcode, wonder if there is any significance? also, the upside down number on the darker one is 10247, last one was 08494. First exposed image on this thread was 04xxx something, i wonder what the lowest number is?
![IMG_0821|666x500](upload://vhqc3V4BVSdOmrDTnN3mCCQodKc.jpeg)

edit 3: got it taken apart. battery connector looks like the same type that is in the op-1, if anyone is desperate for a battery. The unit powers on and operates on usb power alone, but acts the same at the moment.
---

## Post #120 by fishdog

Played around today, the plastic is held on by adhesive on the front side, I was able to just use a plastic spatula to pry it off, but a little heat from a heat gun should do it easily. The main circuit assembly is also one big piece that is held to the front of the case with more adhesive, and there was also some on the battery holding the main board to the sub components. Mine came apart fairly easily
---

## Post #121 by modulpaule

The square on the upper left side looks different on these units. What is this?
---

## Post #122 by fishdog

Probably just a adhesive dispense location.


Edit 1: 
Was also able to hook up a J-link last night and successfully connected to the uC but my first few initial attempts to dump the firmware have been unsuccessful as both dumps resulted in just 0s filling the binaries. Will have to solder on some wires to ensure a good connection and try again
---

## Post #123 by JaggedNZ

That’s consistent with appprotect bit being set.

But all is not lost https://limitedresults.com/2020/06/nrf52-debug-resurrection-approtect-bypass/

As always proceed at your own risk :slight_smile:
---

## Post #124 by murray

[quote="murray, post:47, topic:66795, full:true"]
APPROTECT requires hardware fault injection detailed by a talented hacker here:
https://limitedresults.com/2020/06/nrf52-debug-resurrection-approtect-bypass/

given APPROTECT is disabled, it would be both possible to dump the code stored in flash (given a successfully connected hardware debugger, in a link i shared above) potentially reverse engineering the binary or upload custom firmware (involving writing drivers for both the audio codec and the emmc).
[/quote]

part 2 is a bit more relevant for our use-case, but in either article the fault injection hardware is not disclosed:
https://limitedresults.com/2020/06/nrf52-debug-resurrection-approtect-bypass-part-2/

since we already know that the stem player circuit does not match the reference design nearly as well as the logitech device--next step would be to sketch out a more complete picture of the stem player circuit and determine if the relevant pins are exposed to test points or traces at all.
---

## Post #125 by JaggedNZ

I agree, it would be worth locating the decoupling capacitors. 

I can provide some advice on executing the hardware modifications as I’ve managed similar scale work in the past. Minimum equipment would be: temp controlled iron with a fine tip (pinecil, ts100 or similar), fine tip metal tweezers, 0.6mm rosin core solder, flux, kapton tape, “magnet” wire or very fine gauge Teflon/ptfe wire,  a digital microscope, steady hands and a lot of patience!

His fault injection hardware is documented elsewhere on his site, I think he’s using a beagleboard, but there is also a video on YouTube documenting how to use an esp32 to successfully execute this attack.
---

## Post #126 by murray

[quote="JaggedNZ, post:125, topic:66795, full:true"]
I agree, it would be worth locating the decoupling capacitors. 
[/quote]

as well as the cpu voltage pins for executing the fault. i would normally bow out of this project at this point but given such clear documentation, it's too tempting to bail.

[quote="JaggedNZ, post:125, topic:66795, full:true"]
His fault injection hardware is documented elsewhere on his site, I think he’s using a beagleboard, but there is also a video on YouTube documenting how to use an esp32 to successfully execute this attack.
[/quote]

i may have a spare beagle board laying around.. when i have a second i'll take a look. given 5$ in components i bet we could breadboard the injector pretty easily
---

## Post #127 by Galapagoose

did a little scoping. i won't have my jlink for at least a week, so doing what i can!

marked all the decoupling caps. they're mostly +3v3 (the main supply for the chip), then a few at +1v3 (the on-board LDO outputs) and a single CPU core voltage at +1v14 -- note this looks like they're running the core at a higher voltage than the datasheet suggests probably because they are overclocking the chip to get enough performance.

the DECx caps are marked. i'm very confident of the 1,4 & 6 (which are all that's needed for the APPROTECT hack) and the others are guesses based on the chip pinout, though they shouldn't matter.

![decoupling-map|690x496](upload://3yUA9R4ksxMGFcGbJaHjwqI6pCj.jpeg)
---

## Post #128 by brick

got an email! my stem player should be at my door on friday, can't wait!
also, I finally got around to taking a look at the APPROTECT bypass writeups; I'm not entirely sure what's going on there exactly: i do understand that there seems to be some hardware system to load the APPROTECT status value into the access port peripherals, and that they inject a pulse into a power line during some period of the startup sequence to... i'm not sure what. bypass the value load? force a chip reset?

[quote="murray, post:126, topic:66795"]
i may have a spare beagle board laying around… when i have a second i’ll take a look. given 5$ in components i bet we could breadboard the injector pretty easily
[/quote]

found the link: https://limitedresults.com/2021/03/the-pocketglitcher/
not only does it have some information about using a beaglebone for this (and has an example of using it for this exploit), it also has a link to a github repo!

honestly a good chunk of these writeups are going over my head, and i can't if i just don't understand what he's saying for much of it, or if he's being intentionally quite vague on specific details. for some reason i believe it's the latter. i hope other people can glean more from it all than i did.


more importantly, what i gather is that the chip's firmware can be cleared and flashed with new data *even with APPROTECT on.* so only *one* person needs to perform the bypass and dump the firmware. then once that firmware is shared, the rest of us can clear and reflash the chip with APPROTECT off, using just the jlink debugger and no hardware bypass giving us room to work! we should be able to make considerable headway once the bypass is performed and firmware is dumped.
---

## Post #129 by murray

[quote="brick, post:128, topic:66795, full:true"]
got an email! my stem player should be at my door on friday, can't wait!
also, I finally got around to taking a look at the APPROTECT bypass writeups; I'm not entirely sure what's going on there exactly: i do understand that there seems to be some hardware system to load the APPROTECT status value into the access port peripherals, and that they inject a pulse into a power line during some period of the startup sequence to... i'm not sure what. bypass the value load? force a chip reset?
[/quote]

it's disturbing the cpu at the precise time that it attempts to load the APPROTECT value from flash memory via the memory controller without fully interrupting the boot process. 

[quote="brick, post:128, topic:66795, full:true"]
more importantly, what i gather is that the chip's firmware can be cleared and flashed with new data *even with APPROTECT on.* so only *one* person needs to perform the bypass and dump the firmware. then once that firmware is shared, the rest of us can clear and reflash the chip with APPROTECT off, using just the jlink debugger and no hardware bypass giving us room to work! we should be able to make considerable headway once the bypass is performed and firmware is dumped.
[/quote]

yes, that's right! 
@Tyler mentioned this possibility in an earlier post, too.

[quote="Tyler, post:48, topic:66795"]
Fingers crossed APPROTECT is disabled, because that would allow us to analyze the binary and potentially inject partial custom code. I think even if it is enabled though, building a new firmware from scratch should always be a possibility. There should be a mass_erase type command through OpenOCD or an nRF toolchain. Would be hard to get started on that without first working out a schematic though.
[/quote]

--

double confirming that APPROTECT is enabled.

![20250114_172411|666x500](upload://u4Su8LFYQh3Cvw6vl2UrfByu0Bw.jpeg)

![screenshot|690x283](upload://yefewszmAW77lRTdb1hyPqCXqDk.png)

found my beaglebone black (it has PRU onboard) and going to go ahead and configure it as a fault injector like described in the above links.
---

## Post #130 by brick

[quote="murray, post:129, topic:66795"]
at the precise time that it attempts to load the APPROTECT value
[/quote]
someone asked how he figured out exactly where to inject the fault, and they said 
>" Analysis, methodology and experience (I do that since long time…)". 

mans literally  pulled an "it came to me in a dream"

---

[quote="murray, post:129, topic:66795"]
@Tyler mentioned this possibility in an earlier post, too.

[quote="Tyler, post:48, topic:66795"]
[...] There should be a mass_erase type command through OpenOCD or an nRF toolchain. [...]
[/quote]
[/quote]

browsing through the writeup shows this at the end of part 1: 
![image|690x219, 75%](upload://xUTubjDeAnIIL6myZMoX5xgfec1.png)

which tells us the exact command we'd need!

that said, one of the comments indicates it should be possible to dump, clear, and reflash the UICR specifically while altering the APPROTECT value, while leaving the actual flash progmem intact:
![image|690x400, 75%](upload://2QFWwTkMRiTLQX8q0jmYX1EEq4H.png)

which nicely enough also gives us the other command as well.


i know we still want to dump the flash for firmware reverse engineering purposes, but doing it this way should leave less room for bricking the device, which is always nice especially for people who only have one [me :dotted_line_face:].

---

[quote="murray, post:129, topic:66795"]
found my beaglebone black
[/quote]
nice. i was at microcenter earlier to grab some hdmi cables and  thought i'd go see if they have a beaglebone i could grab while i was there. they do not stock any beaglebones :sob:

---
edit: update. found the exact bit where explains how to clear all with both openOCD and the nRF toolchain:
![image|690x380, 75%](upload://kGRZPLbAZcT2uzq0iv6mVbpARVy.png)
---

## Post #131 by Duloz

[quote="brick, post:128, topic:66795"]
once that firmware is shared, the rest of us can clear and reflash the chip with APPROTECT off, using just the jlink debugger and no hardware bypass giving us room to work! we should be able to make considerable headway once the bypass is performed and firmware is dumped.
[/quote]

I'm hoping we can make some sense of it and find some hints as to how to do things via usb, I can't imagine they didn't have that in mind.
---

## Post #132 by PedalsandChill

If you or any of the other computer engineers bricks theirs while working on this, I'll do my best to send a replacement. I have at least one I can spare for the cause.
---

## Post #133 by Tyler

Mine just arrived! Probably won't have time to set up for the fault injection until this weekend. I didn't really do much prep -- I hope my trusty **esp-prog**  is adequate for the job.

@murray, it looks like you are poised to try the exploit pretty soon, is that right? Best of luck!
---

## Post #134 by murray

[quote="Tyler, post:133, topic:66795, full:true"]
@murray, it looks like you are poised to try the exploit pretty soon, is that right? Best of luck!
[/quote]

if i can find the time to get all the pieces of the system tested this week between work and baby, i should be able to get it off the ground imminently-ish. it would sure feel really cool to upload a registry and firmware dump by the weekend. if i can't, i'll look forward to any updates from you!
---

## Post #135 by brick

it would definitely perhaps not be a bad idea to get multiple people dumping the firmware before we start clearing and reflashing; just to compare data to make sure it all came off properly.
though that said, hope the firmware doesn’t majorly change from revision to revision 💀
---

## Post #136 by kasselvania

You sly devils…. I bit. Just ordered. Praying our combined intelligence can make this thing more than a good looking paperweight emblazoned with the name thou shalt not speak.
---

## Post #137 by claasp

i've been following this thread like it's a murder mystery
---

## Post #138 by Ecoustic

Well, it won’t be my combined intelligence, at least. I’m a writer and I appreciate the mystery of a story that’s only partially told. So I was willing to bet that some canny individual here might break the code and in the process drag me across the finish line. My most fervent hopes for my little TE Stem are with you. 🤠
---

## Post #139 by fishdog

My work is on a pause, no longer powering up... If I can't get it back up this week I could maybe desolder the board to track traces further and find out what rest of the test points are.
---

## Post #140 by OGProgrammer

The most compelling info I've gathered from reading through this is holding the function button while turning it on doesn't seem to let the play button "play" - this gives me hope there is a button sequence but reading above about the USB being connected directly to the battery is the bad news. I'm going to try and follow usb activity on various button combos. Function + any single button doesn't seem to work. Perhaps it's holding another button for some period of time. I'll try to make a sheet of possibilities using ChatGPT and go through them. One note is I have the friends and family version of the device. I'm hoping this came with a way to load more music but it is also possible this was a previous version of the firmware and didn't have a way.
---

## Post #141 by Virtual_Flannel

It seems to me like they originally intended to make the device user accessible. Maybe when the deal with Kanye soured,  he had already ordered a couple thousand devices, so they locked it down before loading the units with the firmware. If thats the case the ability might be in there, might be able to edit the code to enable it.
---

## Post #142 by instantjuggler

in one way i would almost think that the friends and family version would have less of a chance to load other music onto it... since its essentially a promo item for the album to give away for fun as opposed to, say, another step in the direction of a product being released to the general public. but of course that's just 10000% speculation on my part. i'm curious though, since you have the F&F version, can you confirm it came in a silver cardboard box?

[quote="Virtual_Flannel, post:115, topic:66795"]
Good point, I think @instantjuggler has the Kano device, curious if anyone knows what the files look like for that device.
[/quote]

while everyone else here is doing real work, i can say that for this question- on the front end you can edit the bpm on the website when you upload files to the kano device. if you need to though, like when their website doesn't work... which, right now is pretty much all the time, then you can hook up your kano stem player to your computer and put it in disk mode. then there's a specific file structure inside the folder on the device that you need to have which is 4 .wav files and then there is a track.txt file that you can edit to set a specific bpm, along with the color of the led's.
---

## Post #143 by xdimi3x

https://devzone.nordicsemi.com/f/nordic-q-a/47640/jtag-programmer-under-100-not-a-dk

jtag tool for those interested...

A cheaper one might be had on amazon

the OP-Z uses a AD/Blackfin i believe. the Nordic chip has a processor on it and my guess the Nordic chip is the brain.
---

## Post #144 by murray

if non-commercial use is the target, edu mini is probably the best bet for the budget minded:
https://shop-us.segger.com/product/j-link-edu-mini-8-08-91/?srsltid=AfmBOorO1CbljXwGK8sm30QH8S11YeuNZwC3eeyUoj3b7kaR1BJf-1OM
---

## Post #145 by Duloz

The fact that the usb port has traces to the SoC would indicate that to me, i think. they could have just run power tracers to the usb otherwise.
---

## Post #146 by Galapagoose

Yes I think this might be my comment being unclear. The USB signal traces connect directly to the uC, but the USB *power* goes to the battery charger chip, and then onto voltage regulation before powering the chip. This is very typical & doesn't change the centrality of the uC.

Every connection on the whole board is controlled by the uC directly. There is no other logic chip on the board, and the firmware for the uC is the only variable here (well apart from the storage on the memory chip, but this is almost certainly just the playback data. the firmware will be saved in the chip's flash).
---

## Post #147 by Duloz

I don’t know if I’m going to be let down, but the dev kit for the SoC is supposed to come with one, and is a little cheaper
---

## Post #148 by murray

looks like that will work well!

https://docs.nordicsemi.com/bundle/ug_nrf52840_dk/page/UG/dk/hw_debug_out.html
---

## Post #149 by Duloz

https://www.symmetryelectronics.com/products/nordic-semiconductor/nrf52840-dk/

900 in stock here, two day shipping is about $9 in the us
---

## Post #150 by murray

wednesday evening update--i've managed to get some early code running on the beaglebone black's pru (yellow square wave on the oscilloscope) and to solder some thin wire to DEC1 and DEC4 (also being measured on the oscilloscope, thanks @Galapagoose for tracing those out!): stem player is playing as a mini-celebration that i haven't fried anything yet (on mute of course).

edit: capacitors still need to be removed!(?)

![20250115_224216|666x500](upload://1fzg7onYvaL9cmlGR5lFYQZEsP4.jpeg)
![20250115_224222|666x500](upload://pbKMBCwiPnLxSlWtyKvA3kv1jwM.jpeg)

what i didn't think about when doing this is how i'm going to restart the stem player to begin instrumenting the power consumption on cpu startup--since the stem player is always fed by the battery it is essentially always-on without a battery drain or a tedious/dangerous hold on the power button (those wires i soldered near the cpu are flimsy and not insulated). i guess i could wire an inline switch between the battery and the board?

additionally, i'd like to write the glitch trigger code on the beaglebone's pru so that the timing/delay is done in assembly rather than the `__delay_cycles()` method in C to ensure high precision/predictability. also we need to find an appropriate voltage for the glitch "command"--limitedresults doesn't show it on their oscilloscope screenshots but the measurement at the bottom of their screen is 5V. 

once the glitch code is parameterized and synchronized with the power consumption behavior, we should be able to attempt the exploit with a manual power up, an automated glitch injection, and a subsequent manual attempt to access the debug interface.

i'll add some example code and resources once i'm a little further along and feeling more confident in these materials.

(something i am worried about is that these stem players were being circulated sometime in 2022 and this vulnernability was fixed with a hardware revision on November 21st 2021. hopefully teenage engineering was working with the old version of the chip and all of this isn't for naught haha)
---

## Post #151 by kurtgirdle

if you take a close up pic of the nrf chip, it should have a date code and some kind of marking to indicate silicon revision
---

## Post #152 by murray

![chip_designation|356x343](upload://eOhaxMJjXYub0GD9UX657oHK8BY.png)

the one on the nordicsemi website in this footprint is:
QIAAF0
2242ED

so, that's revision F, 2022?


![20250116_001935|375x500](upload://aqq9gL0RdTHJMwjkbaexM4KQ0uq.jpeg)

 going to take a leap here and say the chip on my board is an earlier version?
QIAAD0
1946DH

revision D, 2019?
---

## Post #153 by instantjuggler

i really really want to thank everyone who is working on this!!!! its like a dream come true that all these deep attempts to look into the device are even happening at all.

and i don't want to distract anyone who is actually working on things so maybe someone who has some knowledge but not the device itself might be able to answer- but what might be a couple of outcome scenarios here? 

like in the best case i can plug my KE into my laptop with a usb cable, somehow update the firmware if anything materializes to fascilitate this, and then potentially access some sort of root folder where i could "drag and drop" (or whatever) my own music files onto the device? 

or then what's the smallest amount of success above the conclusion of the journey just being that the device is locked and its impossible to modify anything at all- would it be something like if you open your unit and solder some wires to one of the chips you can... erase JIK from the device... but that's it or something? 

maybe its impossible to tell anything at this point but i don't know anything about electronics and was just curious about some context of what might be happening along the way here...
---

## Post #154 by brick

[quote="murray, post:150, topic:66795"]
edit: capacitors still need to be removed!(?)
[/quote]

hey quick note before you go to the trouble, limitedresults notes somewhere (I think on the second post?) that to only perform the attack, you don’t need to remove the caps. they did that for analysis’ sake

---

edit (i got out of bed and came to my desktop to reply to more posts):

[quote="murray, post:150, topic:66795"]
what i didn’t think about when doing this is how i’m going to restart the stem player to begin instrumenting the power consumption on cpu startup
[/quote]
they also specifically note on the page about using the beaglebone that the nRF chip is very high efficiency, and can probably be powered by the beaglebone itself, through the chip's power lines. I think this is probably the best way to power the chip while doing the exploit, at least.

[quote="murray, post:150, topic:66795"]
these stem players were being circulated sometime in 2022 and this vulnernability was fixed with a hardware revision on November 21st 2021
[/quote]

it's worth noting that the Kano version of the stem player was released in conjunction with *Donda* (Aug 2021), while the TE version seems intended to be released in conjunction with *Jesus is King* (Aug 2019). given the development timeline, I don't think we really need to worry about the hardware revision here.
---

## Post #155 by Nick_Martin

This is amazing! I have one and if I can help out in any way let me know. Unfortunately I’m not a developer but any other task I’m happy to jump on. Testing or finding info etc.
---

## Post #156 by brick

[quote="Galapagoose, post:146, topic:66795"]
(well apart from the storage on the memory chip, but this is almost certainly just the playback data.
[/quote]

[quote="Duloz, post:131, topic:66795"]
I’m hoping we can make some sense of it and find some hints as to how to do things via usb, I can’t imagine they didn’t have that in mind.
[/quote]

there's ***gotta*** be a way to load data onto the flash chip through usb, right? like, there's no way that they're uploading *Jesus is King* to the flash chip, through the JTAG, right? how else would they get the media information on there, or edit bpm etc?

---

[quote="instantjuggler, post:142, topic:66795"]
on the front end you can edit the bpm on the website when you upload files to the kano device. if you need to though, like when their website doesn’t work… which, right now is pretty much all the time, then you can hook up your kano stem player to your computer and put it in disk mode.
[/quote]
i had a similar concern with the KO2 when i got it, the thought that the upload and sample editing utility was only available as a webpage was not entirely comforting. if it's worth anything though, i'd found a way using nativefier to package that into a local executable i could run: https://www.reddit.com/r/teenageengineering/comments/1aylewn/offline_ep133_sample_tool/
although in searching for this post, it seems someone's made a nicer version with some QoL features i'll have to download.
---

## Post #157 by kurtgirdle

yep! 2019 date code

![IMG_6707|690x314, 50%](upload://zXAxIh4cuu7u9WwN39s9XKOdvWB.jpeg)
![IMG_6708|690x445, 50%](upload://nTn8mYtcIBGY8eV8QCRMoAueiLU.jpeg)
---

## Post #158 by kasselvania

If it’s of any use to anybody who is doing the work on this stuff, I have the first version of the Kano stem player and don’t use it/want it. I am happy for it to be an organ donor if someone thinks cracking it open and poking at its internals would be assistive.
---

## Post #159 by xdimi3x

I have not go too hardware deep on this device but:
Thought which comes to mind is reset trace on the nordic go to any of the buttons?
Does the instruction set for the Nordic how a recommended DFU mode trigger and if so are any of the pins connected up in a way that might give a hint on how to put it into a mode change?
---

## Post #160 by Duloz

UPS says my dev board will be delivered to me in Pittsburgh within the next two hours.

The tracking info says it hasn't left Fort Worth.

DOUBT
---

## Post #161 by murray

[quote="brick, post:154, topic:66795, full:true"]
[quote="murray, post:150, topic:66795"]
edit: capacitors still need to be removed!(?)
[/quote]

hey quick note before you go to the trouble, limitedresults notes somewhere (I think on the second post?) that to only perform the attack, you don’t need to remove the caps. they did that for analysis’ sake
[/quote]
---

that's true, but since we don't have the timing parameters from limitedresults (this is one of the 'exercises' they've implicitly left up to the reader), we are in a position of performing analysis. hopefully the capacitors won't smooth out the important features, but if we get stuck they'll have to go.

[quote="brick, post:154, topic:66795, full:true"]
[quote="murray, post:150, topic:66795"]
what i didn’t think about when doing this is how i’m going to restart the stem player to begin instrumenting the power consumption on cpu startup
[/quote]
they also specifically note on the page about using the beaglebone that the nRF chip is very high efficiency, and can probably be powered by the beaglebone itself, through the chip's power lines. I think this is probably the best way to power the chip while doing the exploit, at least.
[/quote]

i read this too--the battery is 3.7V and the beaglebone black is a 3.3V system. definitely the easiest route, but the voltage difference may introduce more variables and opportunities for the sytem to act unpredictably.

[quote="brick, post:154, topic:66795, full:true"]
[quote="murray, post:150, topic:66795"]
these stem players were being circulated sometime in 2022 and this vulnernability was fixed with a hardware revision on November 21st 2021
[/quote]

it's worth noting that the Kano version of the stem player was released in conjunction with *Donda* (Aug 2021), while the TE version seems intended to be released in conjunction with *Jesus is King* (Aug 2019). given the development timeline, I don't think we really need to worry about the hardware revision here.
[/quote]

props to @kurtgirdle for taking all the guessing out of this question!

-- 
[quote="brick, post:156, topic:66795, full:true"]
there's ***gotta*** be a way to load data onto the flash chip through usb, right? like, there's no way that they're uploading *Jesus is King* to the flash chip, through the JTAG, right? how else would they get the media information on there, or edit bpm etc?
[/quote]

they could have used an emmc flash programmer which gets teenage engineering around having to implement a disk storage mode on the stem player. i think it's important to remember that this thing is a prototype--in my professional experience, customer-facing configuration features are typically the last things to get implemented before a product goes into final release testing (if they're even planned at all).
---

## Post #162 by Duloz

there are thousands of them out there at this point, and  all i believe were in Kanye's ownership until fairly recently. They even got listed for sale on his website before being pulled. I don't think all of these can be considered prototypes.
---

## Post #163 by murray

[quote="Duloz, post:162, topic:66795, full:true"]
They even got listed for sale on his website before being pulled. I don't think all of these can be considered prototypes.
[/quote]

the notion that some of them were publicly sold, combined with the lack of mention in any of the user manual sheets, further cements my belief that a usb disk mode is not accessible without modification. i'll avoid any further dumping of cold water on this investigatory angle--maybe y'all will find something!
---

## Post #164 by Duloz

i don't recall TE talking about their web tools within the one pager info sheets in the past, though I don't think I have any lying around. 

Was the sample tool even available when the KO-133 first launched? I don't think it was on the one page insert, at the very least.

not proof of anything, but lack of documentation doesn't necessarily mean anything when it comes to TE.

Edit: the downloadable user manual for the OP-XY wasn't even fully fleshed out for weeks after launch
---

## Post #165 by soysos

Pittsburgh? Me too!
Have we met?
---

## Post #166 by Duloz

Probably not, I've kind of been a shut-in since lock down in 2020. working on it.

Edit: unless you're the guy that a gave the op-z case and line module to.
---

## Post #167 by murray

thursday evening update--i spent most of tonight attempting to profile the cpu power like limitedresults had done. 

i had disconnected the lipo battery and am currently unplugging+plugging the usb brick to act as a rudimentary switch. my trigger on DEC1 and DEC4 had been working okay, but was not seeing the same power line behavior as limitedresults' oscilloscope screenshots.

then switched to my huge 90's 200MHz oscilloscope and was again able to observe and trigger on the edges of the power signals pretty reliably but again, nothing analagous to limitedresults' findings.

so i decided to go ahead and remove the caps DEC1, DEC4, and DEC6 as @Galapagoose had traced out earlier:

 [quote="Galapagoose, post:127, topic:66795, full:true"]
the DECx caps are marked. i'm very confident of the 1,4 & 6 (which are all that's needed for the APPROTECT hack) and the others are guesses based on the chip pinout, though they shouldn't matter.
![decoupling-map|690x496](upload://3yUA9R4ksxMGFcGbJaHjwqI6pCj.jpeg)
[/quote]

i was careful to disconnect power and use a magnifier to ensure i wasn't shorting the leftover pads after resoldering my jumper wires to DEC1 and DEC4.

oh and by the way a **WARNING**:
if you want to continue using your stem player as-is, **DO NOT** remove decoupling capacitor DEC6 from your board. DEC1 and DEC4's removal kept my device functional, but after removing DEC6 my stem player has all but stopped functioning and the only way that i know my cpu is (might be?) still working is the response from `openocd` letting me know that APPROTECT is enabled and that it can't bring up a debugger. this is fine for me because i just want the code, but you have been warned.

anyways, at the moment i am not able to acquire a trigger on the power signals when i plug the usb-c brick in and i'm not sure why and it's driving me crazy (my multimeter reads correct voltage values on DEC1 and DEC4). there's a chance i'm doing something stupid, but in any case in a few minutes i'm going to hang it up for the night.

--

oh and i attempted an assembly version of my code on the beaglebone black's pru and it behaves pretty much exactly the same, both switch a gpio pin on and off every ~5ns. so i'll probably just use the C code to implement the glitch once i have a suitable trigger from the oscilloscope generated by the CPU's power lines.
---

## Post #168 by Duloz

second day air has turned into you'll get it in a week.

edit: it took two days just to travel 37 miles from Fort Worth to Dallas. wtf UPS... maybe i'll get it before i die of old age?

Edit 2: I just broke one of the rocker buttons off of the board on my FAFO unit, so it is now up for sacrifice. Do i need a beagle board alongside the dev kit?
---

## Post #169 by murray

friday evening update--tonight i was working between my oscilloscopes' trigger and range options, trying to get the tek tds 640a (the only oscilloscope i have with a trigger out) to trigger on the nordic chip's power lines. 

last night i was sort of pulling my hair out, but turns out i just wasn't being patient enough. first, my tired brain didn't realize that the capacitors on the usb-c power brick were holding a charge for a couple seconds after disconnection which was preventing the power lines from resetting. and second, with the tools that i have, it's a sort of coin flip that the scope will catch the right part of the signal and not trigger too early. i've been able to mitigate this condition somewhat using the trigger holdoff feature, but it's still not near 100% reliable. it also might explain why limitedresults' script seems to have been run several times before a successful attack (notice the *APPROTECT BYPASS 13*):

![PoC_shellview|690x286](upload://A8Cls5IRe9jez75d9kjw5xuQHgJ.png)

the below photo shows a successful trigger acquisition on DEC4 (top) and DEC1 below it:

![20250117_234037|666x500](upload://43TTlmWmza2UvR1ESuPvYFOQe8z.jpeg)

and then here's the trigger out of the 640a being fed into the 2004c, where the trigger out is driven LOW following the successful trigger acquisition (notice the signal bordering the bottom of the screen):

![20250117_235603|666x500](upload://5dxcckKFwWLX7scHWg0DcZ1n0WW.jpeg)

you'll see that the power behavior looks similar to limitedresults' screenshot (though in mine, DEC1 is bottom and DEC4 is top but in their screenshot, DEC4 is on bottom and DEC1 is top):

![Power-Up_Flash|690x424](upload://ykA0Ui7EyNFxfthhy0zHJsyF7XT.webp)

my next step is going to be sending the 640a trigger out into a designated GPIO of the beaglebone black and triggering a pulse of multiple defined periods back out to the 2004c for visualization and general testing. followed by gathering the courage to begin experimenting with sending pulses back into the nordicsemi chip.
---

## Post #170 by JaggedNZ

My limited understanding of glitch attacks is they are often require many attempts and are often automated. e.g. you can try thousands of attempts as long as you catch one that works.
---

## Post #171 by kasselvania

Wow. For some reason my iPhone photo is too big to post, but, is that a mic by the power indicator LEDs?! 

Also, now that I have this thing in my hand I’m DYING for the community to crack this thing, cause this device is flipping sweet.
---

## Post #172 by robbie

[quote="kasselvania, post:171, topic:66795"]
is that a mic
[/quote]
Unfortunately not. &nbsp;
[quote="PedalsandChill, post:87, topic:66795"]
the little holes on the side are just small holes painted black.
[/quote]
---

## Post #173 by murray

sunday evening update--

tonight i connected the tds640a's trigger output to the beaglebone black and then connected a designated gpio to send a pulse to both oscilloscopes for voltage and timing measurements. it took some perusing documentation, but i ended up with this code which i loaded onto pru0 of the beaglebone:

```
#include <stdint.h>
#include <stdbool.h>
#include <pru_cfg.h>
#include "resource_table_empty.h"
#include "prugpio.h"

volatile register uint32_t __R30;
volatile register uint32_t __R31;

void main(void)
{
  bool reset_pulse = false;
  const unsigned long pulse_delay_cycles = 1000;
  const unsigned long pulse_width_cycles = 1000;
  uint32_t gpio_p9_27_in = P9_27;
  uint32_t gpio_p9_30_out = P9_30;

  /* Clear SYSCFG[STANDBY_INIT] to enable OCP master port */
  CT_CFG.SYSCFG_bit.STANDBY_INIT = 0;

  while(1) {
    if( (__R31 & gpio_p9_27_in) == gpio_p9_27_in ) {
      reset_pulse = true;;
    } else {
      if( reset_pulse ) {
        __delay_cycles(pulse_delay_cycles);
        __R30 |= gpio_p9_30_out;
        __delay_cycles(period_width_cycles);
        __R30 &= ~gpio_p9_30_out;
        reset_pulse = false;
      }
    }
  }
  __halt();
}
```

successfully triggering on the stem player's power lines yielded good results (the beaglebone black is peeking out the side on the right):

![20250120_000455|666x500](upload://g9Tr4B3AQVpwzO9rQZLG0lAqrPb.jpeg)

we have a 5us ~3.3V pulse generated at the time of trigger acquisition:

![20250120_000511|666x500](upload://ynKFPhC3sLD6rPMRzn8hqd26xkm.jpeg)

and an exaggerated scale of the pulse on channel 3 shows that it coincides with the power line trigger acquisition on channel 1:

![20250120_000516|666x500](upload://dcD3BYTKEaH46xZNVlasJ3IPvcx.jpeg)

manually arming the tds640a is a bit tedious and while i'll probably manually trigger the first few glitch campaigns, if i don't make progress then i'll probably be forced to fully automate this procedure like limitedresults had done. i'm lucky that the tds640a has an obsolete gpib interface because gpib<->usb adapters are pretty readily available and Tom Verbeure wrote up a nice article on working with his own tds420a from linux using a native instruments gpib-usb-hs dongle:

https://tomverbeure.github.io/2020/06/27/Tektronix-TDS420A-Remote-Control-over-GPIB.html

the associated oscilloscopes' programmers manual looks like single trigger acquisitions are (probably) possible:

[tektronix_tds_programmers_manual.pdf|attachment](upload://da22mL8xe1DvJwqyFGTX7OQoWxO.pdf) (1.7 MB)

after enabling remote arming of the oscilloscope i'd then modify the beaglebone pru code to grab the values for `pulse_delay_cycles` and `pulse_width_cycles` from shared memory populated by a userspace program. and lastly, look at the feasibility of running the stem player system from the beaglebone black's gpio's (probably using the second pru).

anyways, more immediately i'll use my next pocket of time i find to think and decide on ideal pulse characteristics which i'll use to begin my manual glitch campaign on the nordicsemi chip.
---

## Post #174 by instantjuggler

just wanted to put a little note here to help out in case anyone wants to make consecutive posts instead of editing a previous post with updates on progress since discourse doesn't allow that on lines right now. 

but to not be totally without content, great new video by JAK over on youtube:
https://youtu.be/ekgWgMeqTn4?si=I01J_iPJT8SIUX3A
---

## Post #175 by brick

my stem player has been in my local sorting center since friday :skull: no idea when it'll turn up.
I also got a chance to check, and it seems my programmer is an ST-link v2. do we think that'd be adequate, or should i invest in something else?
---

## Post #176 by murray

a quick google turned up this link and, although unsupported by nordicsemi, looks like it will work with st-link or open tools:
https://devzone.nordicsemi.com/f/nordic-q-a/21415/flashing-nrf52832-chip-with-st-link-v2

--

if anyone would like to follow along--some of the reading i'm doing on crowbar glitches:

https://media.githubusercontent.com/media/jontyrudman/voltage-glitch-nrf52-cc254x-paper/master/report/report.pdf

https://eprint.iacr.org/2016/810.pdf

http://newae.com/sidechannel/cwdocs/tutorialglitchvcc.html
---

## Post #177 by xdimi3x

With where you are, and the stuff listed above is there a for the less gifted to help you? I have some tools you have shown and to get some stuff I don't have... if you are at a place where you are banging the machine and can sort walk through a tool chain more of us could bang the machine...
---

## Post #178 by murray

[quote="xdimi3x, post:177, topic:66795, full:true"]
With where you are, and the stuff listed above is there a for the less gifted to help you? I have some tools you have shown and to get some stuff I don't have... if you are at a place where you are banging the machine and can sort walk through a tool chain more of us could bang the machine...
[/quote]

almost there--i'm a little concerned about folks bricking/killing their stem players on account of me, especially since i don't have a copy of the dumped flash yet. but i don't have a problem with assembling a full set of materials for describing my non-automated setup with parts list and instructions given a heavy disclaimer at the top.

i'm waiting on some parts from mouser for the crowbar glitch (n-channel mosfet, 10/100/1k ohm resistors, should be here on wednesday), which is the very next step from my last update. after those come in and i have an opportunity to give everything a go, i'll start putting together some more coherent documentation. 

and i fully agree that @brick's thought is solid thinking:

[quote="brick, post:135, topic:66795, full:true"]
not be a bad idea to get multiple people dumping the firmware before we start clearing and reflashing; just to compare data to make sure it all came off properly.
[/quote]
---

## Post #179 by fishdog

Has anyone started a Git repo yet for the project?
---

## Post #180 by mmryspace

[quote="xdimi3x, post:177, topic:66795"]
I have some tools you have shown and to get some stuff I don’t have…
[/quote]

I have a [black magic probe](https://www.adafruit.com/product/3839) that I bought a while back with dreams to hack the Korg SQ-1's firmware (still dreaming about that one...) . It appears the MCU on the stem player is a [supported target](https://black-magic.org/supported-targets.html) . I have zero experience with this but eager to learn as well.
---

## Post #181 by Duloz

Hooray, i can reply again!

Anyone have any idea what the hell I did?

I’ve resumed spamming this unit with usb commands to look at results.  

I just picked it up and was surprised that it turned on because it hadn’t right after I reconnected the battery a few days ago, which made me think i just hadn't connected it properly.

Note that the button that is pressed when you press the rocker towards you is broken off, but I don’t think that has had an impact before. Momentary switches close circuits, so it should still be an open circuit if the button is gone, right?

It won’t play until I press play and one of the track buttons. Something is up.

https://youtube.com/shorts/WGh8mVHL0LA?si=r3dZkDKar1DZJWHN

Edit: different face button combinations give different light patterns... 

edit2: oh man this unit is fubar. the light combinations match to which tracks are currently playing, and it doesn't necessarily include the track that you are holding down. 

i don't see why a missing button would completely throw it off, and I'm fairly sure it worked normally right after the button broke off.
---

## Post #182 by murray

[quote="fishdog, post:179, topic:66795, full:true"]
Has anyone started a Git repo yet for the project?
[/quote]

no, but i am intending to do so. i'll be unblocked by a parts delivery tomorrow and should have something up by the end of the weekend.

[quote="Duloz, post:181, topic:66795, full:true"]
i don't see why a missing button would completely throw it off, and I'm fairly sure it worked normally right after the button broke off.
[/quote]

have you opened it up and had a look at neighboring traces under a magnifier? maybe there's a trace that's been shorted? i inadvertently broke off one of the volume buttons, but my device was okay.
---

## Post #183 by PedalsandChill

Might be your ribbon cable. I had a similar issue when I tried to put the back plate  on before the side piece. Took mine back apart, adjusted the ribbon cable, put side piece on first, then the back and it was working normal again.
---

## Post #184 by Duloz

I've reseated the ribbon cable a few times now, the spot where the button broke off appears clean, but it could be hosed there still, looks like the solder pads ripped right off the board. either way,  this unit is toast.

Edit: i supposed i could bombard my other unit with usb commands to confirm it is a hardware issue vs some glitch from being bombarded with commands for two days straight, but i don't want two janky units.
---

## Post #185 by PedalsandChill

Any way to pinpoint which command messed it up?
---

## Post #186 by Duloz

I am continuing to throw stuff at it to see if i can make anything else happen. As i think about it, I'm almost positive it had to be something done via usb, as i'd already broken the button off before i started running stuff, and hadn't taken it back apart. My process has been to turn it on and start it playing before i run the script to keep it from turning off. it wasn't until after i quit that i noticed it wasn't working anymore. 

I could still be wrong, it would not be remotely close to the first time.

edit: unfortunately, there is nothing obvious in the logs.

Edit 2: I assume it was one in the 800 block, since those return successes, and no other ones have. I'm going back through the 800 block, trying every single code, it's going to take a while. 
altogether, i may be wasting a lot of time.

mispoke when i said 800 block. adhd brain. meant 0x80, but no dice there. I'm going to run through a few more this weekend, but at this point it is just a test device, and while i will brag about my soldering skills to an extent, my limit seems to be adding GPIO pins, no smaller, and specifically through-hole. Either i can keep putzing with this board and use it as a test target for firmware if it is pulled, OR i could drop the broke-ass one in the mail to someone who is trying to work on the the aprotect fault injection so you have an already busted board in case it bricks or whatever.

I'm going through 0x00-0x0F now, will do 0x40 and 0xc0 as well because those are generally vendor specific, and wireshark and lsusb both say it is in vendr specific mode, so i'm probably an iduot for not starting there.
---

## Post #187 by murray

~~hey y'all--looks like i fried my stem player tonight experimenting with a crowbar glitch circuit involving a power mosfet. the power rails are all reading way low and the openocd wont connect anymore. i'll try to put up a git repository and a mostly comprehensive set of information for anyone interested in continuing down this road on sunday-ish.~~

jumped the gun on this! we are still in business--openocd is connecting.
---

## Post #188 by Galapagoose

ahh bummer!
im well positioned to jump in and take up these reigns. appreciate any info you can provide & we can hopefully get you a replacement via the team once there’s something more to do with it!
---

## Post #189 by murray

mega-kind of you to express that. will pay those wishes forward when the opportunity arises!
---

## Post #190 by instantjuggler

glad to hear its not dead yet!!! i just checked over on paris saint, and they've dropped the price of the stem player even more now, at least that's what's showing here in sweden...
---

## Post #191 by Duloz

I’m on my last round of usb command runs to determine if i caused the change in behavior via usb. It is probably no, and a hardware issue. With that in mind, does anyone working on a hardware hack want me to drop it in the mail so you have a second unit?  I’ve got two others with a third on the way, so the borked one can probably be more useful elsewhere
---

## Post #192 by murray

i'll take it, it would help to have another unit to compare firmware to. will pm you
---

## Post #193 by brick

my unit has finally arrived! though, at the beginning of what looks to be a very busy week. i've gotten the chance to fiddle with playback and stuff, but i haven't disassembled it yet. we'll see if i can get my stlink setup to work!
---

## Post #194 by Duloz

I'll try to get it in the mail this week.

I'm going to drop out of the conversation at this point, as I really don't have anything else to bring to the table.

Sorry to everyone for all my non-value-add messages so far.

Oh, would the nr52840-dk be of use to any of y'all in this? don't know what the hell  was thinking i would be able to do with it.
---

## Post #195 by murray

take as long as you need to send the b0rked stem player over. i really appreciate you making that available and it’ll be an asset with figuring out how to reprogram this device (plus i may be able to plug my working controls into it and be able to test with it–my device is at the point where it’s nonfunctional except for minimal cpu facilities).

i also wanted to say that i think your input has been positive and even though the usb investigation didn’t yield much in the way of actionable data, it lets everyone reading know what’s been tried and what not to do going forward. knowing what doesn’t work can be just as valuable as knowing what does work, so thanks for your efforts!

you should think about hanging onto that development kit. once we have a copy of the flash file, you’ll be able to use the development kit’s onboard programmer to reflash your unit which gets around the APPROTECT limitation and, if we’re able to figure out a way to get custom stem files loaded, would allow you to upload custom firmware in order to do that.

--

ps. i've been working on adding a few improvements and performing some testing on the beaglebone black glitch code that i've been cobbling together. as soon as my improvements are complete, i'll make it public.
---

## Post #196 by Duloz

Fair. I will hold on to it
---

## Post #197 by murray

i had fewer cycles this week and weekend to dedicate to this, but i spent a fair amount of time performing manual triggers and subsequent glitches. i grew discouraged pretty quickly at the potential number of attempts it would take to successfully bypass APPROTECT. so i've decided to go down the road of an automated glitching system laid out like the below diagram:

![automated_glitching_diagram|690x428](upload://AaKGZlBV9kExxDFJHOJSdcvmW6F.png)

i'm envisioning that the entire system will be managed by the beaglebone black using python, very similar to limitedresults' system. chronologically the flow will look something like:

1. glitch pulse delay and width parameters will be programmatically chosen from a given range
2. those parameters will be set for the glitch pulse
3. triggers are armed on the 640a and 2004c oscilloscopes using gpib
4. the script will supply power to the stem player from a gpio pin
5. when the 640a acquires a trigger during stem player power on, its trigger out will signal to code running on the beaglebone's first pru to emit a pulse with the given parameters
6. the pulse will cause an n-channel mosfet to close and pull the stem player's DEC1 to ground which is known as a "crowbar glitch"
7. an attempt will be made to attach the j-link to the stem player's debugger thru openocd and download the firmware
8. if access to the debugger is successful, quit and wait for a human
9. if access to the debugger is denied, choose new glitch parameters and try again at step #2

(the 2004c oscilloscope exists purely to monitor the characteristics of the glitch pulse output since i am short of 640a-compatible probes and the 640a is my only oscilloscope with a hardware trigger output)

here is the in-progress glitching code:
https://github.com/resinbeard/beaglebone-black-glitcher

it should be noted that currently the pru is hardcoded to respond to ttl low and to use specific gpio pins. also the pulse delay/width "cycles" are about ~5ns each. i'll post some measurements when i'm a little bit further along writing the gpib commands to pull plots off my oscilloscopes.

i would have all of the equipment i need for the hardware setup, but i made the mistake of buying a ~$100 national instruments gpib-usb-hs adapter off ebay for the remote arming of the 640a's trigger from the beaglebone. well not only are 99% of ~$100 national instruments gpib-usb-hs devices on ebay chinese counterfeits, but they won't work with the linux gpib drivers (they will work on windows). lesson learned after diving through sourceforge issues.

so i went ahead, bit the bullet, and bought a [prologix gpib usb controller](https://prologix.biz/product/gpib-usb-controller/) which will arrive in a few days. it's worth it to me since i have some function generators and a spectrum analyzer that also have gpib interfaces.
---

## Post #198 by brick

glad to see you've got this stuff mostly figured out! hopefully automation will take the busy work out of it, freeing up your time for other things. fingers crossed that you find the parameters fairly quickly!

unfortunate that i haven't been able to help much with the hardware exploit end of things, hope i can help more with the firmware once it gets dumped!

edit:
I know you've probably already done this, but just for sake of completeness i'm going to ask; in regards to the timing for the crowbar glitch, have you tried comparing the signals coming from your unit, to the pattern shown by limitedresults in that red box? [i vaguely remember you mentioning something about capacitors obscuring this signal but i can't see it in the thread]
---

## Post #199 by murray

thanks! i hope so too--luckily we're not fully working in the dark and there are some hints i've been using from limitedresults' blog and the papers i've been reading. i'll edit my post to include those once i'm back at my personal computer.

--
edit:

paper on glitching a nRF52832
https://github.com/jontyrudman/voltage-glitch-nrf52-cc254x-paper

![glitch_delay_distribution|690x407](upload://sP9bxljxZLtA98i7bKU3grLrEE0.png)

![glitch_width_distribution|690x393](upload://tgMbP8kYsxVsHwKwsnaPr8J2ETO.png)


limitedresults' blog

referencing my oscilloscope photos above, it looks like i am able to trigger on the same/very-similar cpu behavior as limitedresults:

![image|666x500](upload://8ihkbDYAp8Kkf2XM4vv9e7QVam2.jpeg)

![image|690x424](upload://4WABP9dQiS17scSuqQKmLteotGh.png)

observing the rigol's `D` cursor position indicator, this red box is 16.1us ahead of the trigger:

![image|690x424](upload://7LHsCrttLasKAiJbdpaZYwKw3f.png)

limitedresults' crowbar glitch width appears to be very small--each of the grid squares is 2us of time which is 2000ns. eyeballing it, i would estimate the glitch could be anywhere between ~5-1000ns taking into account the characteristics of the mosfet being used. limitedresults doesn't share their circuit but does say this within their [pocketglitcher writeup](https://limitedresults.com/2021/03/the-pocketglitcher/).
```
Analog Development
The Analog stage is based on level shifter, Mosfet and resistors, designed to produce a powerful negative glitch (also known as crowbar technique) once triggered by the Pulse_Output (P1_36).
It has to be reliable, able to drive a large amount of current with a very fast response Time Ton.
```

![image|690x424](upload://z3vmwzqxMOOQjnm9cg1Ej7spjux.png)

--

and yes, i had identified a likely location of the memory/register loading at cpu startup (which relates to limitedresults' recorded behavior in the screenshot with the red box) which i've been targeting for my manual glitching. unfortunately since my 'incident' (the one above where i was spooked that i fried the stem player), power consumption looks a bit different (DEC1 for example is reading lower than usual). i'll include these with my measurements of the glitch pulse that my pru code is triggering in a follow-up or edit of this reply.

--

can't reply, so i'll update my existing response--

20250130 00:29 pacific time

today i spent some time learning gpib protocol and controlling the tek tds2004c from the beaglebone black over the usb hub. also plotted some measurements of both the pulse generated by my glitch pru code and the resulting voltage drop. you'll notice the tek 640a has acquired a square wave--i'm using single trigger acquisitions on the probe compensation signal to reliably trigger the pru code on the beaglebone black. the crowbar circuit (consisting of a gpio supplying 3.3V, an n-channel mosfet, a pull-down resistor, and pru pulse input) is on the breadboard.

![20250130_002751|666x500](upload://vb6U98spAFr2VWiw2idvqfVJbYp.jpeg)

and here are some waveforms from the tds2004c and their corresponding glitch parameters (in cycles) which will give you an idea of how the glitch width relates to the shape of each crowbar glitch. top signal (red) is the voltage, bottom signal (blue) is the glitch pulse.

```
root@BeagleBone:/opt/source/beaglebone-black-glitcher/glitch# ./glitch-param-set --delay 0 --width 1
Using /dev/mem.
pulse_delay_cycles: 0, pulse_width_cycles: 1
munmap succeeded
root@BeagleBone:/opt/source/beaglebone-black-glitcher/glitch# 
```
50ns / div
![crowbar_1cycle_50ns_window|503x500, 100%](upload://qtEH4EI25eZqgO83zf8QWrGvErs.png)

--

```
root@BeagleBone:/opt/source/beaglebone-black-glitcher/glitch# ./glitch-param-set --delay 0 --width 2
Using /dev/mem.
pulse_delay_cycles: 0, pulse_width_cycles: 2
munmap succeeded
root@BeagleBone:/opt/source/beaglebone-black-glitcher/glitch# 
```
50ns / div
![crowbar_2cycle_50ns_window|503x500](upload://2cJh3o0KIwwtsNSVaz4HgoKTTuN.png)

--

```
root@BeagleBone:/opt/source/beaglebone-black-glitcher/glitch# ./glitch-param-set --delay 0 --width 3
Using /dev/mem.
pulse_delay_cycles: 0, pulse_width_cycles: 3
munmap succeeded
root@BeagleBone:/opt/source/beaglebone-black-glitcher/glitch# 
```
50ns / div
![crowbar_3cycle_50ns_window|503x500](upload://926MeDtKMllJbvTBMldBxxaMWkU.png)

--

```
root@BeagleBone:/opt/source/beaglebone-black-glitcher/glitch# ./glitch-param-set --delay 0 --width 5
Using /dev/mem.
pulse_delay_cycles: 0, pulse_width_cycles: 5
munmap succeeded
root@BeagleBone:/opt/source/beaglebone-black-glitcher/glitch# 
```
50ns / div
![crowbar_5cycle_50ns_window|503x500](upload://brjdmYttZLeyzye1o5e43sVckNF.png)

--

```
root@BeagleBone:/opt/source/beaglebone-black-glitcher/glitch# ./glitch-param-set --delay 0 --width 10
Using /dev/mem.
pulse_delay_cycles: 0, pulse_width_cycles: 10
munmap succeeded
root@BeagleBone:/opt/source/beaglebone-black-glitcher/glitch# 

```
100ns / div
![crowbar_10cycle_100ns_window|503x500](upload://3VAUeH8KBUlbikbFniPkyDIVIhD.png)

--

```
root@BeagleBone:/opt/source/beaglebone-black-glitcher/glitch# ./glitch-param-set --delay 0 --width 20
Using /dev/mem.
pulse_delay_cycles: 0, pulse_width_cycles: 20
munmap succeeded
root@BeagleBone:/opt/source/beaglebone-black-glitcher/glitch# 
```
100ns / div
![crowbar_20cycle_100ns_window|503x500](upload://x8IB5BOEgoiwAlWFFxdI6qOx95X.png)

--

anyways, y'all probably get the idea that those signals straighten out and more perfectly oppose each other the wider we make our glitch pulse width. the characteristics we're seeing are partly due to how fast the beaglebone pru is able to toggle its corresponding gpio and the switching behavior (how fast it reacts to its gate input) of the n-channel mosfet ([this one](https://www.adafruit.com/product/355)). hoping to have this automated glitching system running by this weekend.  when it's successfully running i'll post a schematic of the crowbar glitch (it's stupid simple) and results if i have any. it's possible either the pru code will need optimizing or the mosfet will need to be switched out for a faster one or both.
---

## Post #200 by bchampion96

I have no skin in this game, it's just exciting to observe. You lot are so fucking smart - it's a real treat to see the passion you're putting into this project!
---

## Post #201 by instantjuggler

thanks for taking the time to share all these resources and your results so far!!!
---

## Post #202 by Noisetrees

It’s a treat to follow what you figure out with this stuff. I’m following it like an elaborate murder mystery 🖤
---

## Post #203 by kasselvania

I want to express the same sentiment. 

The device has been sitting on my desk, and I keep thinking “just you wait and see, my friends may actually set you free!”

😂
---

## Post #204 by brick

oh? hold on
out of curiosity, i plugged the stem player into my macbook, to see if i could communicate with it, and without any extra prodding i see this in the device manager.
![image|598x434, 50%](upload://8TKKpNBCYydeO4GTrrvTR3GOn83.png)
the fact that it advertises its name as "*stem player*" on the bus implies that, at a minimum, there is at least *some* implementation of sending data over usb, which gives me hope that data transfer is implemented, as well. i'll work on seeing if i can access data over the usb line.

edit 1:
further snooping indicates it's advertising as both.. uh, a host and a client device?
![image|690x71, 100%](upload://fy2oM0ilgUy9QIN4XevNuDfIJlD.jpeg)
frankly, i'm not really sure what any of this means. if anyone's able to point me in the correct direction regarding documentation and resources on how to snoop on usb devices, that'd be helpful. google is only really giving me basic troubleshooting advice for if your usb thumb drive doesn't work, which isn't really what i need here.
---

## Post #205 by kasselvania

Okay, okay, okay… I had tried this and sorta poked at it but thought it had to be too easy and that someone would’ve DEFINITELY already tried this. Glad to see it could actually bare fruit. 

I wonder if there’s anything we can tell via terminal that would inform us if the stem player functions similarly to a device such as the OP-Z (which was the “newest” device in the TE line with USB transfer).

There’s also a question of the Bluetooth code and antenna they used and if it resembles the Z. If so, there may be methods of accessing the device through that route as well.
---

## Post #206 by brick

[quote="kasselvania, post:205, topic:66795"]
There’s also a question of the Bluetooth code and antenna they used
[/quote]
Good instinct, but afaik the Bluetooth antenna they've used is an audio transmitter unit. That said, the main chip that is powering the unit (that @murray has been working on crowbarring) *also* has a bluetooth transmitter in it. query as to whether it's activated and transmitting. I'm under the impression that it isn't.

If you've tried something, even if you don't know where to go with it, or even if it's a dead end, it's best to share what you've done! someone said this earlier, but knowing what has and hasn't been done is always helpful information, even if it leads to a dead end!

[quote="kasselvania, post:205, topic:66795"]
I wonder if there’s anything we can tell via terminal that would inform us if the stem player functions similarly to a device such as the OP-Z
[/quote]

I was honestly thinking of it like connecting to norns through usb. We'll see what's going on, i'll do some snooping
---

## Post #207 by Duloz

This is what i had been leaning into with my script. If you hit it with my tests. if you hit it with my python script.
see if it is of any use to you.  

you will get successful responses with some commands w in the  neighborhood of bmRequestType = [0x08]. 0x40 and 0xc0 are supposed to be vendor specific request types, so it might be worth looking around there.

adjustments to lines 100 to 105 changes the query range. It will dump results to a CSV. it requires pyusb (pip3 pyusb). If the stem player turns off, it will save the last position to a json so you can start up again. If you have wireshark running and monitoring USB, i don't think it will turn off. 

This is all just brute force stuff though.

https://drive.google.com/file/d/1aokEFIfM1X4u8FJjYMs8v_SGcPJX-Fzb/view?usp=sharing
---

## Post #208 by eigen

~~it seems to register as a class compliant audio device~~

nevermind, might have skewed my analysis. sorry for the false hopes.

EDIT: yeah, this was another device that re-registerd itself.
---

## Post #209 by brick

i remember looking at that earlier, and just having no idea what i was seeing. I think I still have the logs open in a tab somewhere actually… guess I gotta start digging into USB message specifications.

i was trying to run wireshark to log the packets, but I’m running into some issues with my machine. apparently to enable USB logging on i think catalina or later, i have to turn off system integrity protection? currently trying to find a machine that doesn't require me to do that :skull:
---

## Post #211 by elew

Signed up specifically to this forum to recruit any folks to a discord server where we also successfully jailbroke the analogue pocket. DM me or reply or something (not sure how this works). I have 5 units on the way.

Edit: neat I figured it would be more anon than this lmao.

Edit 2: I’m guessing we can exploit the usb stack similar to what we did with AP. It likely requires a bit of specialized equipment. I’m going to have X-rays taken of the devices board too.

Edit 3: we probably will need to use something a bit different than just a PC to do said USB exploit.

Edit 4: sorry, I can’t just reply and my thoughts are fragmented whilst sick. But the plan for one of the devices will be to pull off the eMMC and dump the flash. Similar to what I did with pocket will make swappable too. This should give us hints as to what the device expects sound wise.
---

## Post #212 by Duloz

if you can run linux on a thumb drive, or anything like that, should be an option
---

## Post #213 by elew

FWIW, you can turn it off temporarily. It’s not unusual. You basically boot into safe made and run a command. Once you have captured the logs, reboot and turn it on again. It’s the turn it back on thing that is important.

Edit: since you’re on Mac, use Parallels and install Ubuntu or something.
---

## Post #214 by murray

tuesday night update! -- unfortunately i wasn't quite able to get the fully automated system working by the end of this past weekend. this was due to a couple of unforeseen complications:

1. powering the stem player over a gpio pin is a little trickier than it might seem. i attempted to retrofit the battery connector/plug to receive 3.3V from the beaglebone black, but (like @Galapagoose mentioned a while before) the stem player is deriving all of its voltages from a 4.2V supply. so this was a deadend.
2. i removed all of my wires and jumpers (cpu power lines and debug) and ended up with a short somewhere on the board which i spent a couple hours chasing down. just as i was about to throw in the towel, i decided to spend some time cleaning a capacitor of excess solder and managed to revive it.
3. work got kind of crazy and with a three-month old baby daughter there has been little time.

anyways, in order to give myself a little more confidence, i soldered a [swd header](https://www.adafruit.com/product/752) onto the jtag/swd traces and i'm no longer sweating about breaking off wires and shorting something while working on the board's cpu-side.

![20250204_225232|666x500](upload://snuOHynsd3bU2c9UAOZLHtSDOAr.jpeg)

you'll also notice two small circuits:
1. the crowbar circuit. this is what i was using to generate the plots in my previous reply and involves an n-channel mosfet to create the crowbar. 
2. another n-channel mosfet connected to a beaglebone gpio and a hacked up usb-c cable. the mosfet is wired up in a very similar way to the crowbar and *does* allow me to control the stem player's power from a gpio pin. it's a low side switch, so the ground line between the usb-c cable's two halves is being opened and closed as the gpio state changes.

i would have a schematic for y'all, but unfortunately electronics cad is the only thing i use windows for (other than games) and i don't feel like messing with altium at this time. once i have a minute to fire up kicad or something, i'll lay something down (like i said though--there are thousands of examples of n-channel mosfet low side switches on the internet. the crowbar is basically such a switch, except drain is connected to the target power supply's positive voltage).

also i switched out the previous mosfets in the circuit for bs170's. this has been pretty cool in that the glitch has become much more responsive (rise time has reduced dramatically), so pulse width adjustments can be less coarse. not super happy with the oscillations at the end of the short, but i'm happy with where things are going:

OLD glitch @ 1cycle, 50ns /div
![crowbar_1cycle_50ns_window|503x500](upload://qtEH4EI25eZqgO83zf8QWrGvErs.png)

NEW glitch with bs170 @ 1cycle, 100ns /div
![crowbar_1cycle_100ns_window_bs170_mosfet|503x500](upload://2AFq6GjFqx5WKVoDG0tM4BpGhCb.png)

and i may wait to solder back on the jumper wires to the cpu power lines until my new soldering iron comes in. i've run into a couple scary moments with the board not powering on or connecting to openocd and i think my nine lives are used up. to give you an idea of what i'm working with, this is the metcal iron (missing a faceplate) i was given 18 years ago which has seen some shit and i don't want to buy new cartridges for. the tip is totally corroded/dissolved:

![20250204_225346|666x500](upload://fqM2b09lUmDzctrwaYzRlv72TVt.jpeg)

in summary, i've pretty much got everything together to automate the glitch campaign--just need to solder on two jumpers to the stem player and write a python script which runs the flow i shared last week.

--

[quote="elew, post:211, topic:66795"]
Signed up specifically to this forum to recruit any folks to a discord server where we also successfully jailbroke the analogue pocket. DM me or reply or something (not sure how this works). I have 5 units on the way.
[/quote]

i appreciate the offer, but much of the work in this topic is built on previous investigations and study by folks who've shared their pocesses and learning in an extremely good-natured and thorough manner. in the interest of solidarity, free information, and generally upping the punx, i prefer to do this here and out in the open.  wishing you all the best, though! (and it would be awesome if you remember this topic and share what you've found or been able to do!).
---

## Post #215 by elew

We will share our method. It’s harder to real time comm here.

Edit: not too different from what you’re working on. But we have found prior art that makes this all much simpler. You need only an esp32 and a steady hand ;)

Edit 2: yes, 4v. But you can use multiple gpio outs from your beagle probably.

Edit 3: we are also gonna play with some expensive toys. Don’t worry too much about frying your @murray, I have a bunch of them and will happily send you one.

Edit 4: 2/9/2025 - 1:57AM
Something we *didnt* consider is the eMMC could have write protect enabled according to spec it seems you can permanently enable it. We won’t know till we finish the exploit (hopefully I can get to tomorrow, waiting for a couple parts)
---

## Post #216 by murray

ah yeah i did see the permanent write protect mode mentioned in the datasheet:

https://media.digikey.com/pdf/Data%2520Sheets/Toshiba%2520PDFs/THGBMNG5D1LBAIL.pdf

hopefully it's not set--would make retrofitting devices several magnitudes more difficult! let us know what you find!

i'm stuck on parts too--my new soldering iron tip has been hanging out in new jersey since the 2nd.
---

## Post #217 by elew

I grabbed some soldering tweezer that might make it slightly easier to lift the DEC1, and the DEC4 isn’t necessary for what we are trying. How’d you go about removing (did you remove it?) DEC1? I’m considering my hot air station and a lot of kapton tape and low flow lol.

Edit: tweezers should be here today (or tomorrow depending on other time zones haha)

Edit 2: you can also power the board directly with 3.3v using the test pads near the battery connector and it remains fully functional. The battery voltage is nominally 3.2v. Might simplify your set up a bit more.
---

## Post #218 by murray

ah good call on the tweezers! for DEC1, the capacitor is small enough that applying the iron long enough prompted the whole thing to lift off its pads without the pcb absorbing too much heat (the iron was an automatic metcal type so can't give you a specific temp to aim for). 

also that test pad hint is a great optimization that circumvents the need for using a mosfet and usb cable. i'll give that a try when i'm next at my electronics desk, thank you!

(surprised that hooking into the battery connector didn't work for me if the test pads will do it--must be some intermediate circuitry between the connector and the pads)
---

## Post #219 by elew

There is, I think. I’ll post a pic of which pads to use in a minute.

Edit: sorry for the crap drawing. On my phone lol. Red squiggle is pos, and I’m sure you’re aware what is ground (black squiggle). I at least get lights coming on. But again, it should operate within 3.3v at least. I’ll solder to them to triple check. But it basically shows it at the lowest charge on the LEDs. 

Edit 2: okay, so after some testing 3.3v is enough to power it on but seems to trigger it thinking the battery is dead. Which makes sense. I observe similar behavior using a fresh stem player which doesn’t power on (since the battery is quite dead) but giving it just enough juice (very briefly charging it) shows similar power down behavior. We need slightly more than 3.3v. But you won’t like what I did next. I power it directly with 5v instead and the device boots and appears to operate perfectly fine. I believe this is because they’re running the chip in VDDH. Which, it (this specific chip) allows up to 5.5v. So they seem to be using it roughly in range with what the battery allows for. I observe no difference in behavior or magic smoke. This is *probably* okay. But the less voltage you run the easier the glitch probably is, since brown out should be more likely. Maybe. Ha. Tools for decap come later today. I don’t feel super comfortable trying the “just hold the soldering iron on” without properly knowing the temps. Ironic, I know. 

Edit 3: I’m pretty sure it’s running VDDH. This at least makes our driver for reset only more complicated by another set of FET and it’s already wired now. So, yay?

https://devzone.nordicsemi.com/f/nordic-q-a/76855/nrf52840-high-voltage-mode-specs

![IMG_2180|375x500](upload://rbJ3r2sXuWEVX3FAGoSgy34D9Zw.jpeg)
---

## Post #221 by Tyler

[quote="uhhhhhh, post:220, topic:66795, full:true"]
Very confused as to why no one has followed up on the fact that the device shows up as a serial device. Has anyone checked what is output on the serial port?
[/quote]

Mine does not enumerate as a serial device. I wonder if @PedalsandChill has an earlier version than me.

Not sure if the serial number on the back of the device has a date code built in, but here is what is engraved on mine:

```
YZY0020SP01 Y4KNF1N8
```

And here's the USB info from **System Report...**:
```
stem player:

  Product ID:	0x1701
  Vendor ID:	0x2367
  Version:	1.00
  Serial Number:	FDE35217D1CD
  Speed:	Up to 12 Mb/s
  Manufacturer:	teenage engineering
  Location ID:	0x02130000 / 4
  Current Available (mA):	500
  Current Required (mA):	500
  Extra Operating Current (mA):	0
```
---

## Post #222 by brick

if you crack it open, the board should show a revision code. I don't know mine's off the top of my head but i could easily check when i'm back home.
i intend to take a look at what kinds of commands the port is responding to, but i need to do a lot more reading into the USB protocol to be able to decipher it. there's a bruteforce script to try raw USB commands, and a dump of some of the trials in these two posts from @Duloz:



[quote="Duloz, post:113, topic:66795"]
Probably useless, and maybe more bad news than good. Made a script to brute force usb control transfer commands. Results are in the file.

https://drive.google.com/file/d/1vRb7t_11UlLmWAbJlNx3a8j5V9fw41Ws/view?usp=drive_link
[/quote]

[quote="Duloz, post:207, topic:66795"]
This is what i had been leaning into with my script. If you hit it with my tests. if you hit it with my python script.
see if it is of any use to you.
[/quote]

---

[quote="elew, post:215, topic:66795"]
Something we *didnt* consider is the eMMC could have write protect enabled according to spec it seems you can permanently enable it. We won’t know till we finish the exploit (hopefully I can get to tomorrow, waiting for a couple parts)
[/quote]

While this is a potential concern, I don't see it as being likely. The current released version of the Stem Player has a lot of marketing around it that it's "a new way to listen to music" and that other people could (theoretically) release music to be on the Stem Player. Unless this was a feature that was dreamed up later, I don't see why eMMC would be locked, as that would mean that this would be impossible in the future.
But then again, take anything i say with a grain of salt. Fingers crossed i'm right about it though :sob:
---

## Post #223 by NonPerson_Human

Wait, does the prototype stem only have Yesus is King with no ability to go to the site and upload new stems? lmao
---

## Post #224 by kasselvania

It was a promotional device developed in collaboration between the Ye and TE. This was never meant for more than just the album. Which, frankly, given the events of the last week, is incredibly regrettable. 

But this is why the community coming together to crack this thing open such a gift.
---

## Post #225 by elew

I have multiples and the numbers are all the same except the QR codes which are obviously serial related. 

The only reason it’s a potential concern to me is how easy it is to flip that bit on and that the USB has no obvious entry point and the manual never mentions loading new songs. I also have reason to believe they are locked down very purposely. Whether by Kanye request or TE own doing, I am unsure. 

We will find out soon enough. The rest of my equipment arrived this evening and am pretty prepared to do the same glitch @murray has been working on. 

[quote="Tyler, post:221, topic:66795"]
`YZY0020SP01`
[/quote]

This is the device identifier. All my packaging has this. The second number is the devices unique serial number. Which seems to follow a pattern of Y4KNFXXX. Of the 6 devices none enumerate as serial devices on windows / Mac / Linux.

Someone had one lacking a serial number but I don’t think they opened it?
---

## Post #226 by joedoe56

hey guys, ive got one too now. anything I can do with it that hasn't already been done?
---

## Post #227 by murray

heya! my best advice would be to read the entire topic (i know that it's kind of long, but it's the best way to catch up on what's been considered, done, and planned) and if you feel that there are any gaps jump on in. but other than that and more specifically:

1. if you have some hardware/software skills+tools, it would be beneficial to have a few copies of the firmware binary to compare against each other (and no one to my knowledge has dumped the flash yet, so the more folks working on that the better).
3. setting up an environment for static code analysis and disassembly of the firmware when we have it. i'm not particularly experienced in reverse-engineering ARM machine code (though i like to think i learn quick). there is a proof of concept set of nRF5x disassembly tools for ida pro on github (https://github.com/DigitalSecurity/nrf5x-tools), but they are seven years old and i fully do not expect them to work out of the box. i also don't have a license to ida pro (i'm attempting to obtain a legal one through edu channels).

i will say that if you're not participating in attempting the firmware dump, it's probably not worth opening up and modifying your stem player until we have a viable path towards uploading custom data to the device.
---

## Post #228 by elew

There is some good ghidra resources I’ve used before for ARM reversal too. Though I don’t have any on hand! Great advice overall. 

I will note we should refrain from sharing firmwares but we can at least checksum to make sure our dumps match, and we can definitely talk about stuff related to the firmware. But firmware files should be a strong no due to potential copyright issues.
---

## Post #229 by murray

to anyone reading this--if we confirm a successful firmware dump don't you dare direct message me about getting the firmware because i definitely won't like that or respond to you or even acknowledge that i received your message with a compressed tar file :upside_down_face:
---

## Post #231 by Duloz

I definitely didn't put the janky stem player in a padded envelope and write your address on it, then did not decide that I need to get some address labels and a proper sharpie, since all I could definitely not find was a paintbrush marker that did not make my handwriting ten times worse.

I don not intend to drop it in the mail in the next two days, and I will not be message you with an ETA when I have not done so.
---

## Post #232 by murray

i very, very much appreciate you not doing any of those things. doing any of those things would not only disappoint me, but would make me feel like you disparaged the Kanye West whom i hold in such high esteem and regard
---

## Post #233 by Duloz

I've seen some discussion in the last day or two about power requirements for the device. 

I have been able to operate it without issue while disconnected from the battery and connected to my computer via USB. In fact, this was the state that it was in for the vast majority of my usb testing.

For those who have been looking at my earlier usb script testing, in my last message I mentioned that the device will stay powered on while connected to wireshark, this might be a mixed bag: you can get a little more detail on things, but if wireshark is keeping the device from powering off, any change in state that you might be able to trigger from a USB command might just result in an error message.

I think i am going to take the script back up with a "CLEAN" device that I have not opened up, no wireshark, just back to brute force with more attention paid to the device by me as I go, make sure it is normal after each shut down before i restart the script. If i fubar another one, it will confirm that usb commands can do something. I've got three here ready to go, and another three on the way.
---

## Post #234 by elew

I’m more worried about TE. They wrote the FW. Not Kanye. What we are doing technically falls under DMCA but it only matters if *you are doing it to your device*. What it doesn’t cover is sharing what you take off your device with other people. Just a warning is all. You can most definitely give and even sell modified devices to other folks though. 

My hope is we can do some analysis and share something that is easily usable by other people so there isn’t anything questionable being passed around. Which is one reason we are going the route that many (and potentially all) non-EE people might be able to use. I believe there is a solderless version of this. But it’s too long to write up here and we still need to confirm a dump. That said, my lab is almost cleaned up enough and all equipment has arrived. 

We also take this seriously in our discord, it’s literally rule #1 lol.

**Rule #1**: do not share any copyrighted file on this server, this is not in our policy. Please refer to portion of dumps through address ranges, or provide source code / instructions to extract the mentioned file from another device.



[quote="Duloz, post:233, topic:66795"]
I have been able to operate it without issue while disconnected from the battery and connected to my computer via USB. In fact, this was the state that it was in for the vast majority of my usb testing.
[/quote]

Same hehe. Test pads are so friendly ;)
![IMG_2262|388x500](upload://aKSvM5JjWmAnIwnk5coLE2nRJNb.jpeg)
---

## Post #235 by Duloz

With that caution fairly in mind, i will point out that TE has all but denied that this device exists, and for all intents and purposes, it seems like the owner of the entire TE Stem Player may very well be a wholesaler/logistics firm.

 This is an interesting string of events. Based on the history I've dug up on efforts to hack this thing, TE has directed people to go ask Kanye's website. 

Last spring, this happened, note that 1981 Distribution's headquarters is within spitting distance of where Paris Saint is "headquartered":

" According to documents obtained by The U.S. Sun, Yeezy LLC - the clothing umbrella owned by [Kanye](https://www.the-sun.com/who/kanye-west/) - entered into an agreement with logistics firm 1981 Distribution earlier this month.

The 1981 Distribution firm specializes in distributing and warehouse facilities for [e-commerce](https://www.the-sun.com/topic/ecommerce/) and it appears the firm is handling Yeezy's stock and shipping.

As collateral for this deal, Kanye put up two luxury [motors](https://www.the-sun.com/motors/), his YZY Gap Apparel range, “various storage archive items” and “Stem Player designed by Teenage Engineering," The U.S. Sun can confirm via documents obtained from Uniform Commercial Code [financial](https://www.the-sun.com/money/news-money/) statements."

Anyway around it, i don't really think that any entity is going to chase these efforts down. I  guarantee that the article is not referring to Kanye's own personal stem player, but the entire stock.

Whoever owns this device at the moment, if even aware of these efforts, is probably drooling over the fact that we just keep ordering more of them. And if these efforts do pan out, they're certain to move the entire stock within a day. 

It's abandonware at this point, but a wholesaler got their hands on a whole bunch of them. I think that is basically the extent of the story. 

All that said, yeah, don't go passing around firmware to every random person on the internet, that creepy Uncle Fed could be anywhere.
---

## Post #236 by murray

to be clear--anyone who reads my comment is advised to take it seriously.

more immediately, passing around copyrighted material that does not belong to the distributor is against this forum's terms of service:
https://llllllll.co/tos

--

ps. new soldering iron tip scheduled to arrive by wednesday. will have the python scripted glitch campaign written by then so all i'll need to do is hook things up and hit 'go'! of course will share in a git repo.
---

## Post #237 by elew

Might be adandonware but you never know what parts of the FW got used in later TE products. Anyway, better safe than sorry and some folks in our group do this for a living — so extra caution is always good.
---

## Post #238 by kasselvania

Maybe the community can do something along the lines of a patcher, similar to a Super Mario World ROM hack. Leave all the copywriten material to the user to pull off their device, and an easily distributable patch to update said firmware.

Also, I know that what I just mentioned is beyond “non-trivial” I just wanted an excuse to talk about Mario ROM hacks.
---

## Post #240 by JaggedNZ

Hopefully (best case) with a disassembled rom there will be a path or process found to upload new stems via usb and a web based tool can be built without need for new firmware.

Otherwise it’s time to develop a new clean room firmware implementation. Knowing what’s on the emmc memory could be a boon here I suspect.

FYI just because the physical stem players have been sold does not mean the IP has. Fair chance this is still owned by TE or Ye or parts could even be licensed from 3rd parties!
---

## Post #241 by KHAGE

[While I'm waiting for the day when I can have fun with my own sounds (:crossed_fingers:), I'm playing with those of the infamous Kayne West...](https://llllllll.co/t/the-tiny-studio-corner-thread/10991/3612?u=khage)
---

## Post #242 by Duloz

Just a heads up, if you order from Paris Saint, be careful about ordering more than one, they seem not to know how to count higher than 1, because that's all that showed up.
---

## Post #243 by elew

maybe people are buying too many, cause I got all of mine :P

I have confirmed that all my software side stuff is working with a DK. so, off to solder on the SWD and take the cap off, and add my glitch wire!

edit: SWD soldered... I highly recommend using a header and not soldering to the pads. way hard lolol. good call @murray. I have sanity tested the software portion of things a few times, and am quite sure I am seeing the player. now to decap and add the glitch wire...

Edit 2: device one has died, my glitch wire popped off and I think I damaged the board trying to reattach it. But I learned a bunch of great stuff this go around, including. I witnessed the glitch and sweeping behavior at least for a little while. Next one will be easier… and have to wait until end of next week. I have work trip :( software side is all done, and I’ve verified the attack works on a devkit as well as the SWD etc. plenty of attempts remaining :)
---

## Post #244 by soysos

It's funny how folks here are trying to liberate/unlock this stem player as Ye gets more and more unhinged and shitty.
Perhaps when the final puzzle is solved some sort of cosmic transformation will take place or a demon will be cast out of his body, freeing him.
---

## Post #245 by bobbcorr

I am a firm believer in the redemptive qualities of time and pressure; after all, that's how diamonds happen.

On the other hand, maybe what we've got here is ... failure to communicate. *Some men, you just can't reach*.
---

## Post #246 by jojjjajjr

On this note, “lament configuration” would be a fun name for custom firmware
---

## Post #247 by murray

monday update--
i received my soldering iron tip and was able to setup the complete automated glitching setup on friday by resoldinger the prerequisite jumper wires and finishing the first iteration of the python code:

![20250216_231410|666x500](upload://tYSkAEgLRqqmKKjWtHUhGue5Zuz.jpeg)

![20250216_231440|666x500](upload://q2X2ME1sAspzEQ8nPZVD5iRvpxI.jpeg)

some examples of the glitch can be seen here:

![20250216_233335|666x500](upload://r5ZG644Ya5O1X0Va9gUphcvJMJ7.jpeg)
![20250216_233319|666x500](upload://tBrOZOsUIDDAldgCK3j8uvjjyiq.jpeg)

the system follows the same organization and flow that i outlined in a previous post:

[quote="murray, post:197, topic:66795"]
![automated_glitching_diagram](upload://AaKGZlBV9kExxDFJHOJSdcvmW6F)

i’m envisioning that the entire system will be managed by the beaglebone black using python, very similar to limitedresults’ system. chronologically the flow will look something like:

1. glitch pulse delay and width parameters will be programmatically chosen from a given range
2. those parameters will be set for the glitch pulse
3. triggers are armed on the 640a and 2004c oscilloscopes using gpib
4. the script will supply power to the stem player from a gpio pin
5. when the 640a acquires a trigger during stem player power on, its trigger out will signal to code running on the beaglebone’s first pru to emit a pulse with the given parameters
6. the pulse will cause an n-channel mosfet to close and pull the stem player’s DEC1 to ground which is known as a “crowbar glitch”
7. an attempt will be made to attach the j-link to the stem player’s debugger thru openocd and download the firmware
8. if access to the debugger is successful, quit and wait for a human
9. if access to the debugger is denied, choose new glitch parameters and try again at step #2
[/quote]

it is available here:
https://github.com/resinbeard/hw-vulnerability-testing

it uses the glitcher code that i published here:
https://github.com/resinbeard/beaglebone-black-glitcher

the `run()` routine looks like this:
```
def run(self, delay_begin, delay_incr, delay_stop, width_begin, width_incr, width_stop, try_max=20):
        glitch_count = 0
       
        self._reset_glitch()
        time.sleep(1)
        
        delay_count = delay_begin
        while delay_count < delay_stop:
            width_count = width_begin
            while width_count < width_stop:
                try_count = 0
                while try_count < try_max:
                    print(f'glitch_campaign.py::GlitchCampaign.run(), GLITCH ATTEMPT #{glitch_count}, delay: {delay_count}, width: {width_count}')
                    if self._try_glitch(delay_count, width_count):
                        print('glitch_campaign.py::GlitchCampaign.run(), successful glitch!!, exiting..')
                        exit(0)
                    else:
                        self._reset_glitch()
                    try_count += 1
                    glitch_count += 1
                    time.sleep(1)
                width_count += width_incr
            delay_count += delay_incr

```

the script is fairly custom, but can be pretty easily adapted to whatever equipment is on-hand. the main script is `glitch_campaign.py` and included is a module `scopes.py` to encapsulate the gpib commands i'm using to control each of my oscilloscopes. i have these copied to my beaglebone black which i run using a command like  this:
```
# python3 glitch_campaign.py --width-begin 1 --width-incr 1 --width-stop 9 --delay-begin 102000 --delay-incr 1 --delay-stop 104001 --try-max 5
```
here is the help to explain the args:
```
# python3 glitch_campaign.py -h
usage: glitch_campaign.py [-h] --delay-begin DELAY_BEGIN --delay-incr DELAY_INCR --delay-stop DELAY_STOP --width-begin WIDTH_BEGIN --width-incr WIDTH_INCR --width-stop WIDTH_STOP [--try-max TRY_MAX]

optional arguments:
  -h, --help            show this help message and exit
  --delay-begin DELAY_BEGIN
                        number of cycles to initiate glitch after power-on trigger acquisition
  --delay-incr DELAY_INCR
                        number of cycles to increment glitch delay after an unsuccessful attempt
  --delay-stop DELAY_STOP
                        number of cycles to limit delay
  --width-begin WIDTH_BEGIN
                        starting width of pru pulse in cycles
  --width-incr WIDTH_INCR
                        number of cycles to increment glitch width after an unsuccessful attempt
  --width-stop WIDTH_STOP
                        number of cycles to limit width
  --try-max TRY_MAX     number of times to attempt glitch on each parameter change
(pyenv) root@BeagleBone:/home/debian/te-stem-player-hacking# 

```
--

unfortunately i'm not having much success--i think that this is partly due to how unstable this particular stem player has become. in a previous post i mentioned that the power behavior looked very different after i had a scare thinking i had fried the board. this looks like a pretty dramatic ringing where there used to be a well-defined representation of the dc voltage. you can see this in the above photos. i believe this instability is varying the trigger time thus throwing off the timing of the glitch either before or after the critical memory event occurs and so far hitting this moving target has not been successful. i am excited to test with the stem player @Duloz is sending me and hoping to have a much more predictable/not-fucked-up system to work with. i'm pretty optimistic that i'm on the right track given that the critical memory event is about 2us long and my trigger is ~10ns+ wide.

--

[quote="elew, post:243, topic:66795"]
edit: SWD soldered… I highly recommend using a header and not soldering to the pads. way hard lolol. good call @murray. 
[/quote]
i didn't so much have a problem getting wires soldered onto the traces, but instead kept breaking them off! (and they weren't cold joints). highly recommended!

[quote="elew, post:243, topic:66795"]
Edit 2: device one has died, my glitch wire popped off and I think I damaged the board trying to reattach it. But I learned a bunch of great stuff this go around, including. I witnessed the glitch and sweeping behavior at least for a little while. Next one will be easier… and have to wait until end of next week. I have work trip :frowning: software side is all done, and I’ve verified the attack works on a devkit as well as the SWD etc. plenty of attempts remaining :slight_smile:
[/quote]

nice work! look forward to hearing how you get along with glitch on further attempts. makes me hopeful to hear that you've got your glitch working on a dev board. and damaging the board seems to be easy to do--to be honest i'm surprised i can still try connecting to my own board via the debugger.
---

## Post #248 by elew

It’s funny you say that because I had almost the exact same issue. I’m pretty sure I can still access my “broken” player via SWD too, but didn’t trust it after the glitch started looking funny on the osci so tested it out and it was not working. 

I don’t recall if I tested the player *after* reattaching the glitch wire, but part of me thinks I did and it was fine. I’m not sure. That said, I usually tried to make sure it would play after every hardware modification. 

eMMC is writable btw.
---

## Post #249 by kasselvania

[quote="elew, post:248, topic:66795"]
eMMC is writable btw.
[/quote]

This is certainly not insignificant!
---

## Post #250 by Cementimental

That or he will re-release it with a giant swastika engraved on the side
---

## Post #251 by kasselvania

While I do really like dunking on folks who are objectively the worst, I believe the best way we could collectively punish and infuriate “he whole shall not be named” would be to outright strike his name, his beliefs, and the majority of identifiers from the record. 

If we just keep this thread about the stem player, while hinting at the unironically terrible nature of one of its creators, we will be creating the world i believe we all would want to live in. 


I’m not making any rules here, but just expressing my feeling in that I’d appreciate this being one of the few spaces this tortured and torturous fellow wasn’t also infiltrating.
---

## Post #252 by Ecoustic

Yes. Kind of ignore rather than abhor.
---

## Post #253 by brick

[quote="elew, post:248, topic:66795"]
eMMC is writable btw.
[/quote]

Amazing! Great news.
How did you learn this, did you pop the chip to check on a flasher? May be worth checking the traces to see if write activity pins are connected to the main cpu.
---

## Post #254 by Duloz

I will go to the post office tomorrow. I’ve still got my niece’s Christmas present to send also, so don’t take it personally, my neuros have been super diverged lately.
---

## Post #255 by murray

no worries, we're not under any deadline. just grateful you're willing to send it all! i'm still continuing to attempt to glitch my current board
---

## Post #256 by Surface13666

So, I have a question my unit came in today and I plugged it in to change and the light comes on but, after ten ish mins the light completely turns off and when I unplug it and attempt to power it on it doesn’t work? Am I doing something wrong
---

## Post #257 by murray

we have a successful glitch and firmware is dumped!!

```
gitch_campaign.py::GlitchCampaign.run(), GLITCH ATTEMPT #8504, delay: 101134, width: 77
Open On-Chip Debugger 0.11.0-rc2
Licensed under GNU GPL v2
For bug reports, read
        http://openocd.org/doc/doxygen/bugs.html
swd
Info : J-Link EDU Mini V1 compiled Mar 14 2024 13:18:10
Info : Hardware version: 1.00
Info : VTarget = 3.287 V
Info : clock speed 1000 kHz
Info : SWD DPIDR 0x2ba01477
Info : nrf52.cpu: hardware has 6 breakpoints, 4 watchpoints
Info : starting gdb server for nrf52.cpu on 3333
Info : Listening on port 3333 for gdb connections
dumped 1048576 bytes in 32.025875s (31.974 KiB/s)

Info : Listening on port 6666 for tcl connections
Info : Listening on port 4444 for telnet connections

$ du -h nrf52_dumped.bin 
1.0M    nrf52_dumped.bin

```

more to say when i have some more time!
but feels good to be able to report.
---

## Post #258 by soysos

I have no idea what this means, but I'm excited!
---

## Post #259 by ModulationStation

As someone who is watching this and doesn't own the stem player (and may never), I am very excited by this. A great effort so far to everyone involved. This is wonderful news.
---

## Post #260 by Virtual_Flannel

[quote="murray, post:257, topic:66795"]
we have a successful glitch and firmware is dumped!!
[/quote]

Wow, getting excited!!! Tempted to buy another one or two of these things, im sure the value will sky rocket once its cracked and sold out.
---

## Post #261 by kasselvania

I sorta feel that vibe. The OG stem splitter was a cool idea totally ruined by its crummy implementation. If this device actually gets hacked, it’s worth having a few on hand.
---

## Post #262 by Zeke_B

I’ve wanted one because I loved the idea and now that progress has been made to this point, I ordered one. 

Thanka for sharing the journey here. It has been fascinating to keep up with!
---

## Post #263 by Ian_Lennox

Why is this so riveting??? Should we be making a Netflix doc???
---

## Post #264 by _pete

I'm so, so tempted to order one. I had a rocky past with the Kano team during their iteration of the Stemplayer (and development of subsequent devices) so perhaps I'm just hung up emotionally, but it's verrrrrrry tempting to get a unit in the event that the backend gets cracked open by you lovely folks. I am also just a sucker for little handheld devices, and seeing the photo with the Tic Tac box has me reeling. 

Sending the best to the hacking team - may the silicon chiclet reveal its secrets soon so that we may all reap the benefits of the stockpile of these devices otherwise being abandonware.
---

## Post #265 by PedalsandChill

I sent a message to [chuppl](https://youtu.be/Ls3VkE2B8zM?si=3xaEj8Baj1D9akXv) 😆
---

## Post #266 by digitalguilt

Hi. Don’t know the credibility or if it was brought up already, but I heard that Kano has/had patent on stem splitting and TE were using their code or pcb for it. Kanye later on decided to go with Kano due to them being a first party of that technology

I’m not 100% sure of that, I’ve read this info in different nooks and crannies. I really hope that Stem Splitting in TE version is on device and not in web interface, otherwise we’ll have to develop our own interface for that if (hopefully not if but when) great people of this thread would ever come up with the solution. In this case I’m willing to contribute with designing the webpage and even pirate-host it anonymously since a lot of people are afraid of DMCA (but I’m not, not located in US nor EU) 

Also in [this video](https://youtu.be/XCFylXd7tQg?si=Det5KPPpVP3Cz6J4) it can be seen that the player was used with aux and usb together, assuming that data transfer was done via a cable and not bluetooth. If it would help I may try to find more videos of Kanye using it

Edit: Also I've spotted that OP's Stem Player states that it has Jesus Is King (and duration) which makes me think that stem splitting is not done on device but rather uploaded as stem files. Also there are several [totally legally published JIK stem files](https://docs.google.com/spreadsheets/d/1vW-nFbnR02F9BEnNPe5NBejHRGPt0QEGOYXLSePsC1k/edit?gid=197907807#gid=197907807). For example I've researched "Use This Gospel" files and it has 10(!) .WAV files inside. So looks like Stemming wasn't done on device but rather was uploaded from PC. Just this song's size is  633,9MB and I kinda doubt that bluetooth would be a best way to transfer songs on all devices (*whole album approximate size is around 4GB, like the chip the stem player has soldered on it's PCB*) I wonder if it's possible to expand storage with soldering similar chip with larger size of memory? 

![image|690x286](upload://n8r57OkxCi4qSvt4xh6Us7hQ4pb.png)
---

## Post #267 by instantjuggler

well alex from kano has said a lot of things... lots of things that i would never put any trust in to be honest. i have not researched this but there are countless websites promoting stem splitting, as well its incorporated into many hardware devices these days i think (MPC or something?) so i highly highly doubt that kano has a patent on stem splitting? as well, i don't think TE was using any code for stem splitting? they made a stem player, not a stem splitter?

there is no stem splitting on the TE device. and no interface for stem splitting needs to be developed by anyone? like for me personally i have my own stems of my own music, as i have created the music using stems, that i would like to load onto the device. but yeah, sure, if you have a song that you don't have the stems for then it would need to be stemmed out if you wanted to load it onto the TE player. there are many many many services both paid and free out there which will accomplish this for you already.
---

## Post #268 by robbie

These devices definitely aren’t doing splitting on-board; they’re just playing back the pre-split files. And I’d expect folks here to be much more interested in using their own stems rather than automagically chopping up commercial tracks.

But either way there are plenty of services available to do stem splitting, so it’s not something that would need replicating here. They still take a reasonable chunk of hardware though, so most run as online services (even when they appear not to). As for Kano, based on the timing of the product release, I’m betting they used [Spleeter](https://research.deezer.com/projects/spleeter.html) on their web portal.
---

## Post #269 by PedalsandChill

I find it highly unlikely TE used any kind of code from Kano considering how basic and limited kanos device is in comparison. They really are two totally separate devices.
Kano had also claimed for a long time that they created a special brand new cutting edge stem splitting software when really they were just using spleeter and .mp3 files. The whole thing is a mess. You can read a little about it [here](https://krystalgamer.github.io/stem-player-emulator/). Not sure if you spoke to someone over there, but I wouldn't trust any source coming from Kano. They've proven to be pretty unreliable one the whole.
---

## Post #270 by Duloz

I would add to this that even on a decent daw, stem splitting is kind of hit or miss, and there is generally some bleed-over of instruments when they cross frequency cutoffs in my experience. The stems on this are pretty clean, I almost wonder if they were even split, or output as separate tracks from the master.
---

## Post #271 by digitalguilt

As you can see in my recent comment I have attached a file that may be inside TE Stem Player. Every track was done in ProTools and was exported as separate stems. I don’t really know how internally Stem Player mixes them (no luck with metadata), but it’s different from Kano’s stem player. According to some files Kano’s player has only 4 files per song meanwhile TE could have more than 8. My best suggestion is still usb+webapp

I wish I could’ve helped more but that’s all I know. Currently waiting for a link to a seller that has them for 60$, will let you all know once this info approves. Will order a few of them to mess around
---

## Post #272 by instantjuggler

the TE stem player has 4 stems, the same as the kano player.
---

## Post #273 by Virtual_Flannel

[quote="digitalguilt, post:271, topic:66795"]
Currently waiting for a link to a seller that has them for 60$, will let you all know once this info approves.
[/quote]

Please share that link if you get it!
---

## Post #274 by kasselvania

Just do everyone knows, I’m99% sure that the Kano stem splitter web interface was using Deezer, which is an open source stem splitting algo. I had a stem splitting obsession for a while and did a side by side comparison.


There are a number of algos out there, including another open source one created and maintained by Facebook.

My favorite paid splitter is AIAIA.AI

It’s paid but good. 

If I were to participate in conjecture, there’s a chance that the 4 track playback and speed manipulation of the TE stem splitter could be built off of the back of the OP-1 (OP-1f). Seems plausible. But it’s still just conjecture. Maybe if we crack open the code, we’ll be able to compare, since I know that the OG-1 was broken open, with teams finding an abandoned synth engine and brought it up using a custom firmware. 

Anyway, long story short, no, the stem splitting ability is absolutely not patented.

As for DMCA’s and patent/IP infringement, I really really think there’s more hand wringing than necessary. Worst comes to worst, someone would just DMCA the GitHub. Kinda NBD, all things considered. 

I say we forge ahead. Hella amped to crack this bad boy open.
---

## Post #275 by Duloz

Ghidra definitely recognizes functions and data calls. Making sense of decompiled code is a different story.
---

## Post #276 by elew

It is definitely not on device.

Edit: you can also find the OG stems where you found that. Use the google sheet better ;)

Edit2: the album uses around 1.8gb of the 4. Pretty sure it’s 8 channel audio uncompressed wav. It’s NOT the same stems as the ones Kano uses. They’re higher quality.
---

## Post #278 by digitalguilt

[quote="kasselvania, post:274, topic:66795"]
I’m99% sure that the Kano stem splitter web interface was using Deezer
[/quote]

You are right indeed. The [famous article](https://krystalgamer.github.io/stem-player-emulator/) by KrystalGamer about Kano's Stem Player actually says that "*The splitting process is done off the device. In fact, they use the open-source Deezer's splitter called [Spleeter](https://github.com/deezer/spleeter)*". I don't really know if anyone has managed to contact KrystalGamer, but I have texted him on Discord and he said that he is willing to help us, but he doesn't have the player nor he wants to buy one. If anyone would manage to get him one or would like to introduce more to the project he said "*contact me either through here [discord] or email*" *(both are on his website where the OG article was posted)*

[quote="elew, post:276, topic:66795"]
you can also find the OG stems where you found that
[/quote]

The sheet in Stem section has only 1 complete stem-version of JIK and it was downloaded from Kano's Stem Player (it even has text files with colors for each song lol). The file I was reviewing previously is dated 2019, while Follow God has indeed 4 stem files (like on Kano) on the other hand Use This Gospel has 10 (meanwhile Kano's version still has 4). I don't really know what's up with those Studio Files but if we were lucky to pull songs from TE SP I think we could be able to compare them with other versions

[quote="instantjuggler, post:272, topic:66795, full:true"]
the TE stem player has 4 stems, the same as the kano player.
[/quote]

So the files from TE SP were pulled or not? Just wanted to know where this info comes from so I could look forward and do more researches.
---

## Post #279 by instantjuggler

Maybe I'm missing something here but- the TE stem player has 4 faders that control 4 stems. I don't get it, is there some idea that each fader would control more than 1 stem loaded onto the device? 

Since the TE player has a friends and family release before the Kano one came out, it stands to reason that ye's team supplied JIK in 4 stems per song and then just used those files as well for the public Kano release?
---

## Post #280 by PedalsandChill

TE stem player has 4 tracks per song, but they're all in stereo.
---

## Post #281 by Virtual_Flannel

Those stems linked earlier aren’t from the stem player theyre just the full stems to the songs i think. Those stems would be mixed down into four groups for the player I think.
---

## Post #282 by kasselvania

This read is absolutely bonkers.

I’m also very proud that my ears confirmed the Deezer connection!
---

## Post #283 by _pete

I can confirm this. The Kano player had the same "official", or "studio" stems as the TE player, and therefore, the actual records. The Kano player was then sold on the half-truth that you could essentially do the same with any song you'd like using their browser algorithm, which, as mentioned above, was free and open source from the start. 

Didn't mean to reopen the convo re: Kano though - happy hacking, one and all.
---

## Post #285 by digitalguilt

Hi everyone! Came back with the info where to get kinda cheap TE SP. They are being sold at live auctions at WhatNot (app or website) by a guy called [phantom_boyz](https://www.whatnot.com/user/phantom_boyz/reviews?app=web&sharing_channel=copyLink?app=web&sharing_channel=copyLink03480098680348805/1343879619320746055). Sadly he doesn't have them just for sale (but maybe someone may text him asking about adding them for sale)

Also some screenshots were sent, you can see a proof of TE SP being sold for less than 70$
 ![image|230x500](upload://uqzAsRTtIgcoT0ihiVv7k2KMaBU.jpeg)
---

## Post #286 by CH23

I just signed up to this website after learning this little device is available. ordered one as well.

anyone tried pressing both of the rocker buttons at once to see if that puts it in another state? that's the one combination that's not normally accessible, but I can imagine it being used for testing purposes.

I'll definitely try to contribute once I have the device, would love to load my own stuff on here.

-CH23
---

## Post #287 by bobbcorr

I adore the idea. Wouldn’t that be a hoot if it worked!
---

## Post #288 by murray

[quote="CH23, post:286, topic:66795"]
anyone tried pressing both of the rocker buttons at once to see if that puts it in another state? that’s the one combination that’s not normally accessible, but I can imagine it being used for testing purposes.
[/quote]

this is a cool and novel idea! but :( check the "rocker pendulum" and the switches directly next to it. the rocker is a pendulum that swings one way or the other depending on which side of the "switch" is rocked and its neck sits between two switches/buttons. rock up, it puts pressure on and activates the momentary switch/button directly above it. rock down and it activates the switch directly below. pretty clever mechanism imo, but leaves out the possibility for a "third" press where pressure is applied to both switches at the same time.

[quote="PedalsandChill, post:18, topic:66795"]
![IMG_6742|666x500](upload://dobwFZeApnLEdzma4ivuLjXLdiO)
[/quote]
---

## Post #289 by CH23

yes but also no; it can't be used if the unit's together. Perfect for functionality only deemed needed for testing, and not accessible to end users. That's exactly why I brought it up :)
---

## Post #290 by murray

ah i gotcha--curious if anything shakes out from the exercise! definitely let us know!
---

## Post #291 by bobbcorr

![IMG_7486|245x200](upload://8XjkIi6h4tz5MeGmMxO2Ege7nlI.gif)
---

## Post #292 by digitalguilt

This actually reminds me how Apple does on their devices. Generally speaking - there is a combination for restarting you phone that be like "*hold lock button + click volume up button and then hold volume down*". What I'm saying is that it might have a combination that is not easy to find (since people could accidentally missclick it). But I'm still not sure about this due to how uneasy it would be to load music from a factory? It's done via USB cable for sure, we just don't know how yet (I still don't believe in Bluetooth theory due to file weight) 

I still hope that Murray will get us further with the dumbed firmware he got!

Were we able to offload songs from TE SP?

Also I think there was a discord server for a faster communication process, please add me there @digitalguilt if possible
---

## Post #293 by elew

what is the md5 of your dump, for FW only -- not the user writable parts.

[quote="digitalguilt, post:292, topic:66795, full:true"]
This actually reminds me how Apple does on their devices. Generally speaking - there is a combination for restarting you phone that be like “*hold lock button + click volume up button and then hold volume down*”. What I’m saying is that it might have a combination that is not easy to find (since people could accidentally missclick it). But I’m still not sure about this due to how uneasy it would be to load music from a factory? It’s done via USB cable for sure, we just don’t know how yet (I still don’t believe in Bluetooth theory due to file weight)

I still hope that Murray will get us further with the dumbed firmware he got!

Were we able to offload songs from TE SP?

Also I think there was a discord server for a faster communication process, please add me there @digitalguilt if possible
[/quote]

the files are most definitely not done via BT, the BT chip is super dumb (they don't use the actual nRF). We have been able to offload the songs and we are working on reverse engineering the format. Finally got listenable audio today!
---

## Post #295 by digitalguilt

[quote="elew, post:293, topic:66795"]
Finally got listenable audio today!
[/quote]

Great news! Would you mind dm'ing me in discord? @digitalguilt
---

## Post #296 by Duloz

I attempted it, but i misjudged and broke one of the rocker buttons off entirely.
---

## Post #297 by CH23

I don't know if you use linux, but the command `file` followed by the filename could potentially give you the info if it is a common format.

You could also try running it through spek.cc if it is recognised as audio, to get the exact bit rate, bit depth, and other info.

@Duloz broke off entirely? can't be soldered back together?


I just got a text from fedex; mine should come in next week.
---

## Post #298 by elew

It is in no way a common format.
---

## Post #299 by instantjuggler

i have no technical knowledge, but is there some advantage to not using a common format for the sound files?
---

## Post #300 by digitalguilt

@instantjuggler 

My only suggestion is that TE SP was planned to be sold with only one album on it (Separate TE SP for every new album). Since we have already seen JIK "branded" one. This would make its users not be able to upload/offload songs to/from the player and actually buy more players (security measures). The batch without engraving might have been Yeezy's test batch for further album releases (since all of these TE SP's are ariving without packaging at all). I still wonder what kind of file format it is and I hope Elew is going to post more info on it
---

## Post #301 by jadam

Sent you a Discord friend request (assuming you are `elew` on that platform as well)
---

## Post #302 by Virtual_Flannel

My initial thinking was it would be a multitrack wav.  Containing five stereo tracks, the four stems and then the POsync track
---

## Post #303 by murray

hey folks!

the past week i've been poring over the firmware dump and attempting consecutive glitches in order to get some confidence on the binary that has already been extracted from the device. i received an additional, mostly functional stem player on wednesday (thank you thank you @Duloz !) and plan to perform at least one additional dump on that device.

but first i finally installed kicad and wanted to share the very simple schematic of my automated glitch system in case anyone wants to replicate the effort:

![screenshot|444x500, 100%](upload://t30Kj7CqteEE93EvP0cPqRmJxuN.png)

for the code analysis, i am using ghidra along with an svd loader script and svd device file from nordic:
https://github.com/leveldown-security/SVD-Loader-Ghidra
https://web.archive.org/web/20240102214735/https://leveldown.de/blog/svd-loader/
https://www.keil.arm.com/packs/nrf_devicefamily

the most immediately clear takeaway from the disassembled code are the active peripherals. the binary seems to be stripped of all sdk versioning info, logging, and most debug messages. there are some references to kanye's album metadata which aren't being obviously referenced in the disassembly.

in an effort to discover a possible legacy nrf sdk version (and concretely relate some of the disassembly to human-readable code), the newest non-nrf connect sdk's were downloaded and added to the repositories within this set of tools intended for ida pro:
https://github.com/DigitalSecurity/nrf5x-tools

i couldn't get a signature match to determine sdk version which naively points to the newer zephyr-based nrf connect sdk for the firmware's source.

to help make sense of the stem player's firmware disassembly, i've been playing with the nrf connect sdk by writing multiple simple projects, comparing their compiled binaries in ghidra, and then using those comparisons to inform me about common code entry points and call patterns and ultimately how the stem player code might be using the nrf52840's peripherals. i've been able to idenfity some simple functionality like simple configuration of certain peripherals and critical sections indicated by the disabling and enabling of interrupts.

but something i'm still frustratingly unclear on is how the emmc chip is being operated. there is no hardware functionality on the nrf52840 that is capable of emmc interaction and there doesn't seem to be any onboard hardware emmc drivers on the stem player. many of the nrf52840 peripherals make use of what's called "EasyDMA" which is a kind of interconnect that allow a sort of shared memory between peripherals without any cpu interaction. my working suspicions are either that the emmc is driven through a custom bitbanging driver or that the emmc is somehow connected to the usb peripheral in hardware and managed by the cpu.

and one last question i'm especially interested in is where the dsp is occurring--the logic for mixing to stereo, forward/reverse playback, etc must be somewhere between the emmc and audio codec on the cpu, but not clear where yet.

(normally i'd think that the spi peripherals [which are active] would be driving the emmc, but spi was dropped from the mmc spec years ago around 4.5, we are on 5.0 with this emmc):
https://media.digikey.com/pdf/Data%20Sheets/Toshiba%20PDFs/THGBMNG5D1LBAIL.pdf

additionally, at least two scenarios are possible which muddy the waters:
1. the stem player firmware dump is incomplete/corrupt
2. my disassembly skills are green enough that i'm not seeing where ghidra needs more context to represent a more accurate picture of the firmware dump

i'm getting very close to shelving ghidra for a while and resolving to building an accurate schematic of the stem player hardware and beginning work on fully custom firmware.

-- 

[quote="elew, post:293, topic:66795"]
what is the md5 of your dump, for FW only – not the user writable parts.
[/quote]

*?

![screenshot_memory_map|690x484](upload://bp17xlSp71dnVtMg3xZG7dpzMY9.png)

by user writable parts, i'm assuming you mean the code ram and flash, but not the UICR and FICR? maybe it's easier to speak in offsets--the md5 for my dump of `0x0 0x100000` is `2538e5d28e46398fda879b6c4aecd73b`. what do you get?

--

[quote="elew, post:293, topic:66795"]
We have been able to offload the songs and we are working on reverse engineering the format. Finally got listenable audio today!
[/quote]

this must be some kind of obfuscation--the cirrus cs42l42 expects i2s or tdm which are simple serial formats. do you have any specifics of this "at rest" format on-hand? 24bit? 16bit?
---

## Post #305 by JoseJX

I've been working on the dump. It's not so much obfuscation as it is likely optimizations for mixing/scrubbing through the tracks. It's 24-bit @ 48000KHz PCM, but it's interleaved in an interesting way along with some metadata that I've worked out as well. I'd be happy to share what I've got thus far if it would help. I've also done a fair bit of reverse engineering, so if you'd like some help figuring out what you've got from the NRF, please let me know :slight_smile:
---

## Post #306 by murray

thanks for your insights! curious about the interleaving and how they've optimized scrubbing and mixing, but especially interested in clues around how the nrf talks to the emmc. i'll pm you!
---

## Post #308 by JoseJX

I'm new here, so I'm not sure that I can get PMs, but you can reach me at the same name (JoseJX) at reddit until I have more permissions here for anything you don't want to post publicly yet.

As a broad overview of what's on the flash:

* It appears to use 8K sectors
* The first 8K are used for album information and track metadata (offset, length in 8K chunks)
* The songs themselves appear to be blocked into 2K chunks.
* There's a likely checksum byte at the start of each 2K block (but I don't have the algorithm for this yet)
* There's 7 bytes of metadata at the end of the 2K block, with time keeping information in the first three bytes and then 4 bytes of presumed LED brightness (maybe, it looks kinda right?)
* The track data itself appears to be interleaved by bytes for left and right channels for the track in big endian, but they go through all 4 of the 2K blocks in each sector.
* I still have some data that's out of place, I've got a little bit of information bleed which is distorting the audio, but I haven't figured out where I've made a mistake yet.
---

## Post #309 by Duloz

there are lots of privilege checks going on in the functions throughout the firmware, and very promising hints at an alternative mode. trying to track some down, but am going in some circles.
---

## Post #310 by Galapagoose

[quote="murray, post:303, topic:66795"]
i’m getting very close to shelving ghidra for a while and resolving to building an accurate schematic of the stem player hardware and beginning work on fully custom firmware.
[/quote]

does anyone have a properly broken unit that could be donated to this cause? would be really helpful (or rather, much faster) to create a schematic if we could desolder the eMMC and NRF chips in particular to avoid a whole lot of challenging probing. looks like a 4-layer PCB with ground/power internal layers, so should be pretty straightforward to reverse the schematic if we can remove the components.

i'd do this on my own unit, but i don't have the tools / skills to re-solder the BGA and whatever-the-hell-package the NRF is. as such it'll probably be a sacrificial modification.
---

## Post #311 by elew

There is an nRF8240 UFO board for chip whisperer I plan to have printed up. I doubt the NAND is necessary for full boot.
---

## Post #312 by KHAGE

For someone who, like me, is boldly following this thread with the impression of watching a foreign film without subtitles... could someone in the know tell me what the current chances of success of this hacking project are ? 🤔🤞🏽🙏🏽
---

## Post #313 by CH23

there is good progress towards figuring out the basics. with that i mean that the firmware has been extracted from the device, but is not entirely 'human readable' as far as I understand it. the flash memory appears to be writable, but it's not known how yet as the audio files are in a weird format.
---

## Post #314 by jaseknighter

it has been an absolute delight to watch this thread evolve. i have been inspired to build a time machine so i can start my education from scratch in EE so i can participate in this project.

on an ever so slightly more serious note, it occurred to me this morning that it’d be super neato if the player could be rewritten to have a mode that played music sent to it via usb or bluetooth using its built-in speaker.
---

## Post #315 by Duloz


I’ve gotten some different results in decompiling between Ghidra and just using a command line decompiler. Each one is able to decompile things that the other didn’t. My next focus is closing those gaps if possible.

It seems like there might be three conditions that need to be met for privilege mode if I am on the right track. Two must be non-zero (possibly those magic button combinations), the third expects an 8 from some variable.
---

## Post #316 by JoseJX

@murray I was thinking that perhaps a way to determine how the communication is being done would be to try to snoop P0.08/P0.06 (CMD, CLK) and confirm on the data lines (which do have test points). The protocol itself isn't that complicated, so bit banging it wouldn't be unreasonable, but the overhead would be high. It does appear that the eMMC is directly connected to the processor, so it doesn't look like there's another flash controller or anything. On the firmware itself, I'd say that you probably were on the right track with identifying the areas of code that set up and interacted with the peripherals. I'd probably start by trying to see if there were interrupts set up for handling the eMMC, it seems unlikely to be polled, but you never know!

@Duloz I'm curious as to what kind of checks you're looking at. Are you tracing execution from startup? Or are you just looking at random functions that you're decompiling? Different tools may see different functions, but it all might just be data as well. Are there signs of functions that would change the USB mode?
---

## Post #317 by murray

[quote="JoseJX, post:316, topic:66795"]
@murray I was thinking that perhaps a way to determine how the communication is being done would be to try to snoop P0.08/P0.06 (CMD, CLK) and confirm on the data lines (which do have test points). The protocol itself isn’t that complicated, so bit banging it wouldn’t be unreasonable, but the overhead would be high. It does appear that the eMMC is directly connected to the processor, so it doesn’t look like there’s another flash controller or anything.
[/quote]

thanks for confirming that the emmc is hooked directly up to the processor! and very likely using some kind of bitbanging driver. i'm under a heavy suspicion that my firmware dump isn't complete--on several of the peripherals there appears to be a suspicious lack of register assignments/references (for example, on the I2S peripheral only TASKS_START and TASKS_STOP are assigned which can't be right) and in general i find that the code size is just simply a whole lot smaller than i'd expect. additionally on subsequent successful glitches of my instrumented board, the dumps are very obviously wrong and i'm suspecting a hardware issue.

i've setup the stem player that @Duloz sent to me on my test jig and can confirm much better/predictable power behavior of DEC1 and DEC4 after capacitor removal. but i am running into troubles shorting DEC1 and suspect there is still some capacitance on that power line that i need to figure out, i would really like to avoid running into the same position i ended up in last month and want to skip removing DEC6's capacitor if possible:
[quote="murray, post:167, topic:66795"]
oh and by the way a **WARNING**:
if you want to continue using your stem player as-is, **DO NOT** remove decoupling capacitor DEC6 from your board. DEC1 and DEC4’s removal kept my device functional, but after removing DEC6 my stem player has all but stopped functioning and the only way that i know my cpu is (might be?) still working is the response from `openocd` letting me know that APPROTECT is enabled and that it can’t bring up a debugger.
[/quote]

once i'm confident of my dump, i'll look into emmc-specific handling and instrumenting p0.08 and p0.06--the jumper wires hooked up to my stem player prevents me from hooking it up to the physical controls to trigger playback and thus possible traffic on p0's cmd and clk.

[quote="JoseJX, post:316, topic:66795"]
@Duloz I’m curious as to what kind of checks you’re looking at. Are you tracing execution from startup? Or are you just looking at random functions that you’re decompiling? Different tools may see different functions, but it all might just be data as well. Are there signs of functions that would change the USB mode?
[/quote]

also very curious about this! i was able to see some references to ACL's in my dump, but this appeared to me related to peripheral memory usage. 

if a couple weeks go by and we haven't gleaned pin assignments or more hints from firmware dumps, i'll offer up my original stem player board as sacrifice and start removing bga's to brute-force create a rough schematic.

[quote="jaseknighter, post:314, topic:66795"]
on an ever so slightly more serious note, it occurred to me this morning that it’d be super neato if the player could be rewritten to have a mode that played music sent to it via usb or bluetooth using its built-in speaker.
[/quote]

totally--in general i think it would be a good idea to start brainstorming how folks would want to interact with this thing. sure there's the obvious one where we preserve its current stem player functionality modified with custom user sounds. but there's also the potential to use it and its controls as a usb midi device and a bunch of other possibilities yet to be dreamed up.
---

## Post #318 by Jonathan_Riley

I want to use it as a Walkman with tape speed and reverse features
---

## Post #319 by Okey_Nwachie

Same, with a bit of "let me tweak and freestyle over this instrumental real quick" thrown in. I am also a complete noob based on far how this has progressed but there are supporters watching with baited breath for sure!!
---

## Post #320 by Duloz

[quote="JoseJX, post:316, topic:66795"]
@Duloz I’m curious as to what kind of checks you’re looking at. Are you tracing execution from startup? Or are you just looking at random functions that you’re decompiling? Different tools may see different functions, but it all might just be data as well. Are there signs of functions that would change the USB mode?
[/quote]

In the pseudo C code, there are checks on privilege mode throughout many functions. All point to r3 in terms of the pre pseudo-c decompilation . Tracing all of the sources, they seem to be pointing to the ram sector, so looking for dynamic values.

At the end of the line, r3 is compared to zero, another factor is compared to zero, and a third is compared to eight. This was the last function that i tracked down before going after a different thread. that final function enables irq interrupts.

i then noticed that there were a large number of empty records in the decompiled code followed by many that failed to decompile. I am most curious as to whether this has anything of interest, or if it is just junk
---

## Post #321 by nahsho

Would it be possible for it to be used as a like audio interface where you can hear your music through it and use its effects and possibley looper live And not have to download songs onto it? Like the tx-6
---

## Post #322 by kasselvania

I don’t believe there’s an analog audio in port. Adding class compliant usb audio interface drivers might be hard.
---

## Post #323 by Duloz

I feel like it has come up, but i can't find it... is there a TE model number associated with this thing that has been turned up?
---

## Post #324 by CH23

see https://llllllll.co/t/te-stem-player/66795/221

YZY0020SP01

I speculate that this means 'yeezy' 'hw revision 00.2.0' 'Stem Player 01'
---

## Post #325 by Ian_Lennox

🤣 twenty characters of lol
---

## Post #326 by CH23

@Duloz I just received my unit; before turning it on is there something i can do to help your efforts?

EDIT: too late, i could not stop myself from trying it.

Speakers works, device enumerates on linux:

2367:1701 Teenage Engineering stem player

```
Bus 001 Device 013: ID 2367:1701 Teenage Engineering stem player
Negotiated speed: Full Speed (12Mbps)
Device Descriptor:
  bLength                18
  bDescriptorType         1
  bcdUSB               2.00
  bDeviceClass            0 [unknown]
  bDeviceSubClass         0 [unknown]
  bDeviceProtocol         0 
  bMaxPacketSize0        64
  idVendor           0x2367 Teenage Engineering
  idProduct          0x1701 stem player
  bcdDevice            1.00
  iManufacturer           1 teenage engineering
  iProduct                2 stem player
  iSerial                 3 D139B11901FA
  bNumConfigurations      1
  Configuration Descriptor:
    bLength                 9
    bDescriptorType         2
    wTotalLength       0x0012
    bNumInterfaces          1
    bConfigurationValue     1
    iConfiguration          4 Default configuration
    bmAttributes         0xc0
      Self Powered
    MaxPower              500mA
    Interface Descriptor:
      bLength                 9
      bDescriptorType         4
      bInterfaceNumber        0
      bAlternateSetting       0
      bNumEndpoints           0
      bInterfaceClass       255 Vendor Specific Class
      bInterfaceSubClass      0 [unknown]
      bInterfaceProtocol      0 
      iInterface              0 
Device Status:     0x0001
  Self Powered

```

it does not appear as a serial device.

![stem_player_pcb|682x500](upload://8GfRZyO4OhglSnr1Sw4INj9IXBv.jpeg)

for your entertainment; scans of the PCB at 1200dpi (unless lines reduces the quality, of so, i'll find another way to post them)
---

## Post #327 by Duloz

  Why on earth would it need to do this, unless for album art, but why and how?

Note, this is an AI interpretation of pseudo-c stuff:

```
**Function body**
   
   The function is responsible for reading raw data from a TIFF image file into memory.
```


Edit: think we have a sound format
" While TIFF is primarily known as a format for images, <mark>it can also be used to carry audio data</mark>, particularly within the context of Digital Cinema Distribution Masters (DCDM) for use in DCPs and ProTools sessions."
---

## Post #328 by JoseJX

I think the AI is hallucinating, I have the sound format mostly figured out from the flash dump and it's just raw PCM interleaved in an interesting way :)
---

## Post #329 by Duloz

Absolutely tripping, a second go through seems much more realistic.
---

## Post #330 by robbie

[quote="Duloz, post:327, topic:66795"]
" While TIFF is primarily known as a format for images, <mark>it can also be used to carry audio data</mark>, particularly within the context of Digital Cinema Distribution Masters (DCDM) for use in DCPs and ProTools sessions."
[/quote]

Yeah, that appears to be total nonsense. While you *could* stuff audio (or any random data) into a TIFF, it’d just be using a TIFF as a wrapper around another, separate data format.

[details="Addendum"]
God knows why I’m fact-checking AI slop, but here we are. [SMPTE 428-4](https://pub.smpte.org/pub/rp428-4/rp0428-4-2010.pdf), a part of the standard which defines recommendations for `DCDM`, states:
> The keywords "shall" and "shall not" indicate requirements strictly to be followed in order to conform to the document and from which no deviation is permitted.

and:

> Audio essence shall be stored in single channel Broadcast Wave files, one Broadcast Wave file per audio channel.

There is no mention of TIFF in this document (although TIFF is used elsewhere in the standard for *image* encoding purposes).
[/details]
---

## Post #331 by CH23

going by this pcb pic i just found (not related to the TE stem player!) I wonder if this layout is an industry standard?

Also initially I thought the 'grid' of VIAs on the TE PCB was to function as a heatsink for the chip, but it looks like a programming/debugging interface? - EDIT: it likely is a heatsink indeed.

![image|438x302](upload://jUyE73n6K1FnhH9Nr5EWw8LTNtJ.jpeg)

(pic above shows a detail of the Tangara music player)

Another thing: when you use the function button to turn on the device, but keep holding it, it won't enumerate. no button combination i found will make it leave this state unless you let go of the function button.
---

## Post #332 by Duloz

OK, some notes that might be promising after pushing all the successfully decompiled functions, the following track with my previous expectations of the meaning of the r1 and r3 values (involved in privilege checks):

Maybe related to the fact the the stem player does not enumerate until the power button is released?:
**Holding a specific button** during power-on could set a flag at `r1 = 0x8`. 

Also:
* When **USB is plugged in**, the firmware might check if a special request is sent.
* If `f8c3 2500` is written, it could **activate USB DFU (Device Firmware Upgrade) mode**.

More:
If `r3` at `0x43C` is `0`, the firmware jumps to the bootloader.

Finally:
* If a firmware verification fails:

```
429d       cmp   r5, r3
d174       bne.n 0x5dc  ; Jump to alternative mode
```
  * If an invalid firmware is detected, the bootloader might **automatically activate**.


## ** Conclusion**

✅ **Bootloader detected.**
✅ **Can be activated via register values or button press.**
✅ **Possible USB-based activation (DFU or mass storage mode).**
✅ **Failsafe: Corrupted firmware might force bootloader mode.**

Lots of stuff dumped in a text file. r1, r2,  and r3 are the keys to the kingdom.

https://drive.google.com/file/d/12Cbq8qeSOhFzUAW1AkBQIDG7oKFxK9wR/view?usp=drive_link
---

## Post #333 by kasselvania

How have things been on this? Anybody had further progress?
---

## Post #334 by J_A_C_O

Hi everybody, I'm new to this, I've just received my stem player.
Does anyone have a recap? hahaha
---

## Post #335 by PedalsandChill

@J_A_C_O Check the video here.
[quote="PedalsandChill, post:1, topic:66795"]
here’s a video of @instantjuggler doing an in-depth video over the TE Stem Player’s functionality.

![](https://img.youtube.com/vi/zynYy35AdE0/maxresdefault.jpg "Teenage Engineering Stem Player Deep Dive")

[ Teenage Engineering Stem Player Deep Dive ](https://www.youtube.com/watch?v=zynYy35AdE0)
[/quote]
---

## Post #336 by J_A_C_O

already watched, thanks you.
I've been having issues charging it, am I the only one?
---

## Post #337 by KHAGE

The provided usb-c cable didn’t work for me either.
---

## Post #338 by Surface13666

Mine also isn’t charging only works thru usb
---

## Post #339 by CH23

ouch that's annoying. You should probably remove the battery, since it could expand, explode, and/or catch fire if you keep charging a faulty battery.
---

## Post #340 by Duloz

Speaking of the battery... if all else fails, i am fairly certain that the stem player battery has the same connector as the OG op-1. lower capacity, but still an option in desperate times.
---

## Post #341 by hallmar

[quote="murray, post:257, topic:66795"]
we have a successful glitch and firmware is dumped!!
[/quote]

I gasped at seeing this, after finally reading through my backlog of unread posts in this thread.

Great fucking work!!!
---

## Post #342 by murray

thank you, i appreciate that! things have been a bit quiet from my end due to transitioning to stay-at-home working dad. will be back in a short while once my family's routines establish a bit further
---

## Post #343 by J_A_C_O

you think is that risky? I tried to charge it over night
---

## Post #346 by CH23

if the battery is not working, then it could be.
---

## Post #347 by OGProgrammer

Nothing super technical but I have two sets of instructions here. https://drive.google.com/drive/folders/1aZd8a6TdXeCzW8ydlRsuxSwiuiQjTzCL?usp=sharing
I don't think it be helpful for the hacking part of this discussion but def for collectors. Look at the PDF and Document in here.
---

## Post #348 by Slab

I picked one up that was delivered yesterday. I was actually expecting it to be smaller from what I had seen and read. It looks like I was lucky and the battery is working great. I've only had a few minutes to play around with it, but I definitely prefer using it with the forth fader all the way down. I don't expect much beyond being able to use it as designed, but I'm a fan of TE products and enjoy obscure audio formats/devices. If anything, it will be fun to use this as a source to heavily manipulate for dark ambient jams. Given the album of choice found on it, I find that particularly appealing.
---

## Post #349 by Valerie_Capers

What would be the difficulty level in dumping jt then loading it with a replicated code from the Kano stem players functions? Is it possible to switch the touch values to correspond to the physical switches on the TE one?
---

## Post #350 by CH23

is the hardware and SOC equal enough to do so?
---

## Post #351 by murray

as far as i'm aware each company's devices share zero commonalities with each other (other than being embedded systems that are meant to play kanye stems). it would be a total port of the software to another platform with a completely different set of peripherals and circuit configurations--akin to rewriting the te stem player firmware from scratch using the kano player's functionality as a high-level design document.
---

## Post #352 by rip

Recieved mine yesterday and been playing around with the functions according to the manual, however I noticed it doesn't seem to activate anything other than basic mode when function button is pressed and there are no top LEDS as stated in the manual. Has anyone managed to get it to select individual tracks or fx? I've also tried sync mode with my PO-12 and it did not communicate as described in the manual
![f0gwfilo|375x500](upload://vg7nkwtX8O4VY9CexWwtNmvSEpk.jpeg)
---

## Post #353 by instantjuggler

I can confirm that my unit does all the functions as listed in the manual, including po-sync, etc. 

I have heard reports of several units from the most recent drops that have flaws in functionality from batteries that don't charge to the audio jack not working and everything in between. Hopefully your unit isn't broken- I'm on my phone right now so kind of hard to check but is there a factory reset somehow to try?
---

## Post #354 by rip

Thanks for the replay, my battery and audio jack seem to work fine on my unit but I'm not aware of any method to factory reset unless maybe reflashing firmware?
---

## Post #355 by _pete

20 chars of can confirm.
---

## Post #356 by instantjuggler

seems like people have gotten the firmware off the device so far... but i'm not part of any discord groups... not sure if the conversation has moved there or somewhere else or the efforts have stalled out for now (or forever?). not any updates here in this thread for quite some time, no idea if reflashing could be a thing at some point. would hope so!
---

## Post #357 by Virtual_Flannel

Very curious if anyone is still working on this or if most options have been exhausted. If this is the end, I really appreciate everyone who put in lots of valuable time into this. De-Kanyefying a cool TE device is definitely a noble cause.
---

## Post #358 by edison

i like de-kanye better than new kanye.

sorry i had to.
---

## Post #359 by Duloz

I keep checking in, and will probably do some more testing of things. I'm not sure what else i can contribute myself at this point though, and I have a bunch of personal bullshit dragging me away on the one hand, as well as a literal dream come true dragging me away on the other.
---

## Post #362 by strtlzrd

Thank you all for your hand work on this.

after reading this entire thread i am left with 2 questions that may or may not pertain to this, if not feel free to remove or whatever im not rly sure how this place works. 

Anyway the 2 things are: 

In relation to the released one, the web player seemed to do the work in terms of actual stem separation and whatnot and the player could read them off of that. I have no technical knowledge on this kind of stuff but say for instance the website gives the stem player the stems which is stored in a 4 track per song sort of deal for the stems on the stem player. Instead of loading songs straight to it, they get processed into readable stems for the player. Because this device (the TE one) seems bricked in that it may not be able to get access to the memory, would a reason be because it is only capable of reading JIK's stems and there isn't a way for us to create the same files with the proper language yet of other files? 

A disk mode seems reasonable which someone has mentioned though on the documentation the usb c is for charging only, no mention of reading capabilities which leads me to believe that this thing may literally not be able to receive anymore files than it already has. 

Sorry if this didnt make sense, drunk but i love music and nerdy cool things.
---

## Post #363 by JoseJX

There hasn't really been too much progress unfortunately, we're waiting on a new dump of the hardware using a more reliable approach. Life has gotten in the way of things a bit, but we'll get there eventually!
---

## Post #364 by instantjuggler

Thanks for the update- and for not giving up hope!!!
---

## Post #365 by Duloz

you can get some detail over usb. I don't see much reason for them to have bothered having it identify itself and appear in the device manager as stem player if usb was not meant to do anything.

edit: though it does concern me a bit that the album details seem to be spelled out in plain text in the FW
---

## Post #367 by TheProblemBelief

In a similar vein Linux could show something different. I remember that not so long ago I could access "protected" devices. Granted, not quite at the component level like the Stem Player, but as Linux is quite precise about access buses, this might help too?

I don't have the skill to participate in this venture but I find it fascinating!
---

## Post #370 by TimK

Unfortunately, it's not that simple. I develop firmware for nRF52840, but I've never hacked into other people's work. 

This thread is quite an amazing read with a crazy mix of some very good info and some wild wild speculation (and some classic chatGPT nonsense). I must say that even the wild speculation from people without a firmware or EE background can be valuable for bringing new ideas to the table!

1. Ikea's speaker has a completely different chipset. That means it's irrelevant for this device. We already know from the research done that the firmware is "locked" (APPROTECT enabled), hence the glitching attack. So no, this device is not accessible.

2. OP1repacker is a very cool project, and could definitely be inspiring, but again, made for a very different device. OP-1 runs on an Analog Devices blackfin, which is not even ARM based. (nRF52840 is ARM M4)

I believe our best bet is that @elew and @murray continue the amazing work they've already been doing. (So fun to follow!)
I also have a small child, so I understand the predicament, but I do hope the people here doing such great work can continue someday! I will be here to read it all when they do.
---

## Post #371 by maybe

super interesting thread, great work everyone!

i just ordered one of these of stockx, but it's kind of worrying to see that people are having issues with the battery and audio jack. would it be possible to swap the battery if it is faulty?
---

## Post #372 by KHAGE

[quote="amandwato, post:35, topic:66795, full:true"]
Looks like a batch of the Stem Players is [up for sale here](https://parisaint.com/products/yzy-2019-og-unreleased-stem-player-by-teenage-engineering-1). Seems like a pretty fair price too. Fair warning, I have not purchased it myself and have no plans to since I wouldn’t have a use for the item except to try and hack it.
[/quote]

Just to let you know… now it's half the price I paid for it.
---

## Post #373 by Duskmos

I got mine from the Paris Saint batch about a month ago for $100, works fine - no issues with mine, but didn't come with the paper manual. When I saw these pop up I had to get one as a TE fan, and an ex-Kanye fan. Def an interesting piece of music culture / tech history. Looking forward to maybe seeing the firmware get cracked someday, but even if it doesn't, it's still pretty fun to play with and will always be interesting on a shelf haha.
---

## Post #374 by TimK

Where did you get the flash dump from? Do you also have the firmware dump? I have some experience developing for nRF52840 and would love to take a look at this while waiting for the stem player I ordered to arrive.
---

## Post #375 by maybe

I just received both of my devices and noticed a significant difference in quality. One unit has the serial number Y4KNGAKJ. On this unit, the print on the back is more defined, the buttons are noticeably clickier, and the volume sliders have a better tactile feel. The other unit, with serial number Y4KNGAGX, appears to function properly overall, but the play button gets stuck. So I wonder if the GAGX is a later revision with lower qc?
---

## Post #376 by KHAGE

Mine has serial #Y4KNF9UQ and it’s fine. 
I only had problems with the provided USB cable (flimsy connection).
---

## Post #377 by 41mad4

Hi! Do all units include the "designed and engineered by KW and TE" ?
---

## Post #378 by TimK

What would be the legality of reverse engineering the schematic from the PCB and publishing that work online? I like Teenage and don't want to upset them or sabotage them :face_with_peeking_eye: 

![Screenshot 2025-05-27 at 11.36.39|655x500, 75%](upload://f426xn2SGCAEYC3voILyWirYNIr.png)

I've been working on reverse engineering some of the PCB so we can create a clear pin mapping. It's a little difficult because I'm working only with the photos of the PCB that have been published by others while I'm waiting on the device I've ordered.

Did anyone remove the flash chip and the nRF52840 from the PCB?  It would be great to have some photos of what's going underneath those. The fact that those are BGA (and aQFN) and the 4 layer board makes for a lot of guesswork right now. 
The BLE module and the speaker also obscure a lot of traces.
---

## Post #379 by Ronderswan

Very excited about the progress here! 

I reached out to see if TE would maybe help as well and they gave the same response.
---

## Post #380 by 41mad4

Since my unit didn't come with a case I made my own using some 3d printing, magnets and foam for protection. Went for an earthy minimalist look.  Lmk what you think!

![image|451x500](upload://6m9x2KLLiwOvP7WmfUONNdnnKVY.jpeg)
---

## Post #381 by SUNSHINESHECK

Just ordered my stem player. Hopefully we can crack the code and put other songs/albums
---

## Post #382 by instantjuggler

This looks incredible!!! Does it give enough protection that you would just throw it into a backpack like this and not worry about it getting damaged?
---

## Post #383 by 41mad4

Yes! Neodymium magnets are surprisingly strong and keep the shells together quite strongly.
---

## Post #384 by Localprestige

Would be rad to get a case like this for the TP-7!
---

## Post #386 by SUNSHINESHECK

Whats the latest update with cracking the stem player? or shall i ask where are we stuck? are people still working on it? <3
---

## Post #387 by Giant_Love

Hi everyone, just a thought here... I think the TP-7 has 3 output jacks that work as in and out. Maybe these jacks have dual purpose also?!?!?

Didn't the old NanoPods use a 1/8" to USB data connector?
---

## Post #388 by OGProgrammer

I'm still under the belief that holding the power button and then hitting a button combo will trigger usb mode. This suspicion is based off the fact when you hold the power button to turn the unit on, the lights flash on but hitting play doesn't actually play yet until you let go of the power button.
---

## Post #389 by SUNSHINESHECK

Ive just tried every button combo i could think of and nothing :frowning: but it would make sense to get into some kind of disk mode.
---

## Post #390 by soysos

I'm surprised that after so much time and so many clever folks on the case there's been no solution to cracking this thing ...
---

## Post #391 by instantjuggler

might be that there is no solution.

really appreciate all the efforts everyone put into this!

if its ever a case of resources, for example in terms of money facilitating more time being put into the project, i'm happy to donate and maybe others are too!
---

## Post #392 by PedalsandChill

I don't wanna speak out of turn by saying too much. But I feel safe saying it ain't over yet.
---

## Post #393 by jadam

I feel like the conversation has continued elsewhere. There was some mention of there being a discord, and I wonder if that's where active discussion is taking place. If so, I'd love an invite link as I am interested in helping out!
---

## Post #394 by instantjuggler

this is the best news ever! i understand that nothing is certain, but i really appreciate knowing that there are more steps to take!!!
---

## Post #395 by kasselvania

I had requested access to said discord a few times and wasn’t given direction or access. So, I’ve decided to sit back and wait.
---

## Post #396 by andrew

would encourage folks with the right experience to collaborate privately if things are ongoing + would encourage less experienced folks to practice patience ☆

(perhaps subtle hype/encouragement is appropriate)
---

## Post #397 by soundcloud-synplant

hey everyone!! ive been following this thread for almost a year now in hope that some day there will be a hack, great work everyone involved!! i love the functionality of this thing and it would totally be a daily carry for me if there was a solution!!

anyway, i could have missed it, but i dont believe ive seen this done in the thread yet, so i figured i’d share what ive found, useless or not. when connecting the device to my phone (pixel 9), i get a prompt reading “preparing usb device”. when i go to the device in settings it shows me that the USB is controlled by the stemplayer and not my phone. when i try to switch the usb control over to my phone, it asks for my phone password, but then after a few seconds, says “couldnt switch”. i know this is like a braindead take, but it seems like the control request might be getting rejected by the stemplayer, or maybe just failing in general, but if the device is recognized as a “USB Device”, then just MAYYYBE theres a way to access the storage through android phones. while probably futile, im gonna continue to try button combinations while turning on/off/trying to switch the usb control. again, sorry if this has already been done, im just trying to contribute to this project!!

![Screenshot_20250806-172450|222x500](upload://5GUfKG1iNGME9yc4RcpCFA21AUe.png)

edit: seems like this is the typical android behavior when connecting any device. damn it. gonna keep attempting anything and everything
---

## Post #398 by Harry_Villan

Hi everyone new to the thread, I just received my stem player and was curious if anyone has tried putting the stem player into midi mode I think its 3 and then connecting? Mine is charging at the moment but curious if I just missed it.
---

## Post #399 by instantjuggler

I have done midi sync out and pocket operator sync out. Both worked great for me.
---

## Post #400 by jedgar

I have this urge to recreate the Stem Player as a game on the TE-designed Playdate.

![IMG_7606|375x500](upload://cYkmuGtD1Xqp1CuxcABotr0DGc5.jpeg)
---

## Post #401 by instantjuggler

would be great if you kept it JIK only on the playdate game version! at least until the original is hacked…
---

## Post #402 by jedgar

haha, yes it must maintain parity with hardware.
---

## Post #403 by TimK

Small update for those who still have hope for this device: there is still a little bit of activity on the discord!

The glitching and dumping of the original firmware has mostly stalled. Maybe @elew  will get to it one day… we’ll see when he has time again!

But meanwhile, @JoseJX has done phenomenal work decoding the data that was dumped off the flash chip (with a tiny little bit of help from me). We have 24 bit 48kHz stereo WAV files of all stems now. This means we mostly understand the data structure. Still a little bit of work to be done on the metadata that is present along with the stems, but it’s almost there.

I have reverse engineered most of the PCB with help from some people on the discord and I know all signals except maybe 2 or 3.

![kicad_view_mcu|690x366](upload://8kX4YZtlH0oSL3kAtHlIR61M7vI.jpeg)

I have also been working on a new firmware and have created an eMMC driver for nRF52840 that reads the flash memory at the same speed as the original stem player’s firmware.

I have audio via I2S up and running - I believe in a very similar way as the original - and can play audio over both speaker and headphone out. (I have made my stem player into a weird little synth as a test)

All the buttons and LEDs can be operated.

USB data transfer works too, but there is a bit of an issue: *If* this ever becomes a fully functional new firmware, data transfer will always be limited to USB2.0 full speed (12 Mbits per sec.)  That’s all  nRF52840 can do. Loading new stems will be slow. To give an example, I can dump the full 1.9GB of flash data in about 1h (!) over USB CDC. I do believe this can be improved, but it will always be kind of a slow device for transferring data.

I haven’t worked at all on the interfacing with the battery charging chip and the BT audio module. (this shouldn’t be too hard since I2S is working and we know all the signals)
---

## Post #404 by KingCreateR

Hey, do you have the link to the discord, I recently got interested in the stem player and was attempting to make my own personal one. But I'd love to see the progress you are making in the server
---

## Post #405 by Tyler

Would love to contribute to firmware! Did you end up going with an RTOS?
---

## Post #406 by instantjuggler

woooooaaaahhhh! what a crazy major update for those of us not on the discord!!! THANK YOU for this epic update- such great news all around. really appreciate you sharing your work here and everything you have found out so far.

also should note that at the time of me writing this the TE stem player is available to purchase for only $80-

https://store.yeezymafia.com/
---

## Post #408 by TimK

I don't know who else is working on something, but I am still far from a functioning firmware. The pieces I talked about are only the necessary “proof of concept” tests to fully understand the hardware and its limitations and to create the building blocks for an actual firmware. 

I love to contribute to the hype (I loved reading here about all the glitching done by @murray and @elew !!) but I also don’t want to create the impression there will be a firmware to download next week.
---

## Post #409 by jedgar

I ordered one from New Republic for $80 https://shopnewrepublic.com/products/yzy-2019-og-unreleased-stem-player-by-teenage-engineering
---

## Post #410 by FaremisSound

Thank you for the heads up! Really hoping the wizards of this community will eventually be able to “open” this device up!
---

## Post #411 by jedgar

Update: it arrived and isn’t a box of rocks. Just turned it on and it works great.
---

## Post #412 by lucasp

Wow ordered one as well, even if doesn't work, such a beautiful piece of design history

@TimK Great work! I'm a programmer and would love to help, could you send me an invite to the discord?
---

## Post #413 by TimK

Hi Lucas,

I can’t give people access to the discord since I’m only a guest over there, just like I am here. (where I’m sometimes allowed to post and sometimes I’m not… understandable but annoying !)

I believe JoseJX is an admin at the discord, you could try to contact him.

I am working on documenting all my work. I have compiled a lot of information that could be useful for development. I’d be happy to share it with anyone who wants to develop their own firmware for this thing.

Is there something specific you want to work on?

This is what I haven’t worked on yet:

* interfacing with the BT audio module (CYBT-353027-02)
* interfacing with the battery charger (BQ24232)
* Audio effects
* PO / MIDI sync

If anyone on here is familiar with eMMC programming/ bus timing, I would be interested in picking your brain. My eMMC driver runs at 32 MHz and works well, but when I switch to High Speed mode I’m getting inconsistent data reads. Strangely the THGBMNG5D1LBAIL in stem player allows me to go up to 32MHz without enabling High Speed mode, even though the eMMC standard specifically requires this.

As a little extra for the non devs, here’s a photo of my development unit. :smiling_face_with_sunglasses: 

![IMG_8153|690x459](upload://262qUWyIhIDThhLnwAShoP3ZzXF.jpeg)
---

## Post #414 by marcus_fischer

I finally got a stem player and am super excited about the prospect of loading my own samples onto it someday. 

the form factor is so satisfying!
---

## Post #415 by Tyler

[quote="TimK, post:413, topic:66795"]
Is there something specific you want to work on?

This is what I haven’t worked on yet:

* interfacing with the BT audio module (CYBT-353027-02)
* interfacing with the battery charger (BQ24232)
* Audio effects
* PO / MIDI sync

[/quote]

(Disclaimer – Not Lucas)

I could jump in on the BT and Charger ICs. I’ve written production firmware that interfaces with a handful of different chips from the BQ series.

Have you worked out the schematics for those two chips?
---

## Post #416 by lucasp

I'd be very interested on working on the effects! 
Should be receiving my unit next week, so I'll ping here when I can start working on it.

Beautiful image and setup, what motivated you to do a development unit rehousing? Easy access to the PCB to poke around?
---

## Post #417 by TimK

Hi Tyler and Lucas,

That is great! I have reverse engineered most of the PCB. I have the full pinout and schematic of MCU and all relevant ICs. I think I have everything we need for BQ24232 and the CYBT-353027-02 module.

@lucasp  haha yes this one has a SWD connector soldered to the back. I wanted to have everything fully functional while keeping as much access as possible and also keep the whole thing a little bit protected. (I have done a bit of travelling this summer and I brought this guy along for playing around.) I have done a few tests with a USB boot loader for programming, but I prefer SWD and a programmer for now. 

I will PM you both to share my documentation!
---

## Post #418 by KingCreateR

Hey, sorry if the channel is more meant for development info, but I was wondering if you plan on having it be some sort of firmware that you upload onto the stem player or one built from the ground up to buy? Just wondering if I should bother purchasing now and waiting, or just saving the money.
---

## Post #419 by TimK

I don’t really understand your question, but I’ll try to answer? I can only speak for myself, I don’t really know what anyone else here is doing.

To me, the stem player is a very fancy nRF52840 dev board. This is a fun side project that I can put on pause whenever real work / dad life gets too busy. I definitely am not trying to sell anything here and I certainly don’t want to start producing stem players myself. That would require a lot of time and money and a serious team of engineers and designers. Probably a manager and a business plan too.🥵

At some point I do want to have a functioning firmware I can share, but I’m only doing this for my own entertainment and don’t want to promise anyone anything. I know it is technically perfectly possible to make the firmware I have in mind, but it’s a lot of work to get it right. I also want to share as much of my findings as possible, but I don’t want to illegally publish anything or upset TE.

Right now, I don’t see a way yet of making this easy to share. Once there’s a bootloader on the device it would be easy to update the firmware. But initially you will always need to completely open up the device and connect a programmer to the SWD port on the back of the PCB to load a bootloader (and erase TE’s firmware). If anyone has a better idea, I would love to hear it.

I won’t advise you on buying an object that -right now - can only play songs that I personally can’t wait to get rid of… 😎
---

## Post #420 by Ben_Jabituya

Amazing to see some of the work here. I managed to get hold of one of these last week and never thought there were many around. Will make a youtube video on the hardware comparison vs the Kano at somepoint soon as I’d been meaning to do that just for the Kano one - they’re both interesting. Will share here when I do it.

The choice of the NRF52840 is strange but if you’re programming with Zephyr you can use almost anything you want if it has the memory and speed you need. If they used USB for file transfer, and I can’t see any other way they’d do it, it would have been ultra slow unless they’d send the files over as mp3/compressed and decompress on the stem player maybe. The Kano Stem player had the same issue, this one would have been even slower if they used the same kind of approach because the MCU on the Kano version is way more powerful. Even without looking at the firmware I don’t think they used the radio on the Nordic chip, it uses a version that isn’t shielded and the stem player case doesn’t have any FCC markings other than for the Cyprus chip that’s used for streaming audio over Bluetooth. Maybe they would have done the FCC testing and report closer to shipping these but it looks like they were quite far into the process already and there’s a lot more around than I originally thought.

Rather than make firmware for it as it is I think it would be better to remake the PCB based on the known parts - looks like schematic is mostly reverse engineered by this point. Replace the emmc with an sd card reader so it makes file transfer more simple and then swap the Nordic chip with something more practical - the emmc will be expensive to place anyway if you only make a few of them because it’s BGA, the one on here is obsolete now as well. You don’t even have to know the existing schematic actually because the buttons, LEDs and sliders not on the main PCB are interfaced with a flex cable to the board by the looks of it so if the PCB is roughly the same shape it will work if you know which things those lines are for and use the right ribbon connector. If there’s enough interest and funding I could design a PCB to get made and share the design files unless anyone else has time to do it, I know some people have drafted schematics already so there are starting points that aren’t from scratch that could be used. Someone would have to make the firmware or I could do that as well dependent again on interest. Would need to know where the flex cable lines go ideally otherwise that will take time to work out. The PCB is small so routing isn’t straightforward. Firmware isn’t too hard, the main thing is down-mixing the stems properly and then some of the effects like reverb are tricky but there’s plenty of info on ways to do that. The Cypress/Infineon Bluetooth chip for audio out is easy to use but can use a different chip anyway if needed. People could make their own versions of a stem player as well with it as the supply for these are limited. Just think although this seems long, it’s better than everyone having to use niche glitch hacks to try and reprogram them, and even if you do any firmware with the hardware in here it will probably be limited anyway.

The advanced mode on the TE one is a nightmare to use as well. If you had a second bank of buttons it would be much easier but they were either limited with pin count on the MCU or wanted to keep the hardware simple with less buttons. The spare pin count issue can be worked around though and limiting buttons creates a mess with how you have to toggle everything. The back cover could possibly be replaced with additional buttons or sliders added and exposed to make it easier to use, could use those for the effects maybe rather than having to have separate basic and advanced modes. Would make the device thicker though but it’s ultra tiny as it is anyway.
---

## Post #421 by TimK

Ben, I would love to see your video of the hardware comparison between this stem player and the Kano one! (I don’t know anything about that device)

TE most likely didn’t use Zephyr for this. We found some indications in the incomplete firmware dump that points to the old nRF5 SDK. I believe when they started working on this, zephyr support by Nordic didn’t exist yet or was at least brand new.

I don’t think this was ever meant to have file transfer over USB. The flash stores 24 bit 48 kHz stereo audio (about 1.9 GB). It just takes too long to upload/download that amount of data over USB 2.0 full speed to create a good user experience. (I’m also thinking about compression to speed this up, but still.) How is this with the Kano device? What MCU are they using?

You are right about the radio. The only Bluetooth/BLE in this device is the Cypress module, there is no antenna on the nRF52840.

To me it sounds way more adventurous to create a new PCBA AND write firmware rather than only write firmware! Especially for hobby project on such a niche device.. I already feel a bit crazy for the amount of work I’ve put into this so far. 🫣 (The PCB in the stem player is REV 9 by the way) But I do understand where you’re coming from, the nRF52840 is a bit of a lightweight in certain areas. New hardware could fix some of those limitations. 

The FPC connector is 19pin P0.3mm XF3B-1945-31A by Omron (See https://omronfs.omron.com/en_US/ecb/products/pdf/xf3b.pdf ) and I have the pinout too if you want.

best,
Tim
---

## Post #422 by Ben_Jabituya

Sure on the video, il put a link here once I have it done :) will still be a few weeks once I get time. That’s great you’re able to see they coded it natively in nrf, defo makes things even more confusing why’d they’d pick it! It’s an amazing SoC but just an odd way to implement and not use BLE.

Your schematic and layout is a good starting point for a replacement board. and yes it looks like you and others put some serious time into it but I get how it’s easy to go down rabbit holes once you start lol. I would basically create a copy of the project and delete the mmc chip and swap it for a micro sd reader, the protocol is similar so all the wires are the same although you’d delete 4 of the data wires. I don’t know what sd reader part would work best based but hopefully something can fit. For mcu, something like STM32F413CGU6 would be good if it can support all the gpios needed-a lot of stm32F4s with >1MB flash and 256k sram would work well-this one has a smallish footprint. If you have the pinout from that FPC I don’t mind at least checking a couple of mcu options and showing how I think the peripherals should be wired and setup in Cube IDE.

The Kano version used an stm32H750, I’ve never even opened mine yet but there are teardown videos on YouTube I’ve seen. It seems overkill but I think part of the reason might be USB data transfer-H7 can do USB high speed. Some F4s can do high speed as well I think but you need extra parts. Kano actually just implemented usb badly and it was still slow for them anyway but it’s an option for this; could replace the Nordic chip for a basic spec stm32H7 and keep an emmc on the board as it is now and then do file transfers over usb…the board will be more costly to make though.
---

## Post #423 by TimK

I’ll send you a DM with the info about the PCB and FPC pinout!
---

## Post #425 by FaremisSound

It’s down to $60 now! [link](https://shopnewrepublic.com/products/yzy-2019-og-unreleased-stem-player-by-teenage-engineering) for those who are contemplating
---

## Post #426 by KHAGE

Damn… I'm weak… I ordered a second one. 🫣
---

## Post #428 by TimK

Everyone needs at least one dev unit :smiling_face_with_sunglasses:
---

## Post #429 by nevvkid

Maybe you should have a look at

* https://www.kickstarter.com/projects/andrewjohnmarch/wvr-modern-open-source-embedded-audio
* [https://www.sparkfun.com/wvr-audio-development-board.html](https://www.sparkfun.com/wvr-audio-development-board.html)

Happy to make an intro with Andrew.

If things need to be tiny a ESP32-S3 could probably fit easier…
---

## Post #430 by sam

:cry: $45 shipping to the UK… probably for the best as I don’t need any more projects…
---

## Post #431 by ErickHatesBMC

Hey! Could you possibly share the STL files for that? I got one that didn’t come with the case either and think yours looks great.
---

## Post #432 by rmri

Ordered 2 units. Super interested to program this thing.
I did some reverse engineering before on TE products and found that they tend to choose sensible ways to interact with their devices and not reinvent the wheel.
I would speculate there should be a very easy way to upload new content, just think about how many times Kanye changes his songs even on streaming platforms. But of course they might have provided him with a custom app so who knows.

In any case, looking forward to try my luck with the firmware programming.
---

## Post #433 by TimK

I think this is valid. During development they definitely had a way to change the content. It would be nightmare to debug otherwise. I too work with clients (and developers) who want to be able to make changes up to the last minute…

This device absolutely has that access via the DAT0-DAT3 test points. USB Full Speed is just too annoying for this amount of data. It’s also completely reasonable to “lock” the content after production. I personally don’t believe we’re going to find any USB access via the TE firmware.

rmri, I sent you a DM in case you’re interested in looking at my documentation.

And I would love to hear more about the reverse engineering you did on other TE products!
---

## Post #434 by _pete

Reading these recent comments has my (novice) mind racing thinking about odd button combos / holding down buttons while turning on similar to how other TE products have operated in the past / continue to operate today. maybe holding the mute buttons while turning on power, maybe holding the side buttons (or any of them) for a set amount of time. I'm so tempted to order one just to sit on a Sunday morning and try every permutation and oddball physical input combo with the USB cable plugged in to a computer with hopes of unlocking a disk mode.
---

## Post #435 by Hlp

Is there still the mysterious top secret x files stem player discord or just another urban myth?
---

## Post #436 by _pete

I finally caved and ordered one. Any shot I could have access to the discord server?
---

## Post #437 by ancaja123

I bought one too. Honestly, just can’t wait to try the stem playback combined with sync. Hopefully mine is functioning, nd in general it’s such a cool relic, and fun to watch future development. This will also kind of let me know if I should consider a TP-7/TX-6 for touring internationally. I use my MPC Live as a stem player and multisample module, but it is a little large to travel with, and I’m not willing to shell out that Field cash just yet
---

## Post #438 by Sumusoid

Ordered one…couldn’t resist :confused: any way to get to the discord? People still working on it :)?
---

## Post #439 by _pete

Bumping this thread again….hoping for the best!
---

## Post #440 by marcus_fischer

![Screenshot 2025-12-13 at 5.57.56 PM|340x500](upload://7LWH14isYf8nRNNLpC8zdw4cPWx.jpeg)

hahahahaha… I got served this ad on social media.

how tiny must that hand be to make the stem player look that huge?!

speaking as someone who works in the commercial photography industry, I feel glad that these product photography templates aren’t coming for all our jobs.
---

## Post #441 by rmri

Haha that picture is crazy, I have big hands but still.

![IMG_1019|375x500](upload://5ZPlR2y2BxuQX4axjgXXAWmid90.jpeg)
---

## Post #442 by KHAGE

Proportions of the Stem are not even right. Bad AI Photoshop trick !?
---

## Post #443 by Kaj_West

https://youtu.be/bYQzpZ88jHM

At 0:43 Do they know something we don’t?
---

## Post #444 by TheProblemBelief

I am surprised at that too but I find this thread so fascinating!
---

## Post #445 by instantjuggler

unfortunately no- dude’s just explaining about the general concept of a “stem player” and he’s not putting his own tracks on there. there is an advanced mode outlined very well in this thread and in my mega review video. but you can’t “make beats” or whatever that he’s saying. i mean, you can in the sense that you can remix the music to the point where its radically changed and sounds like a new beat. but he’s just generalizing and glossing over the details which could imply something else other than what is acutally possible/happening.
---

## Post #446 by loooops

Whoa just checked this link and they are on sale for $40 [here now](https://shopnewrepublic.com/products/yzy-2019-og-unreleased-stem-player-by-teenage-engineering). Glad yours showed up functional! Might take the risk on a couple at this price.
---

## Post #447 by ancaja123

Damn, he’s being hella misleading in general then. But yea, I think you’re absolutely right. Got mine about a week ago, your videos helped a lot. I think it’s an insanely cool and inspiring device. MIDI over TRS is weird, you almost have to beat match no matter what, using the scrub knob, but once it locks in it works. I’m still shocked at how well bluetooth works, and that they didn’t implement that into the TP-7. It would honestly make it an instant buy for me.
---

## Post #448 by B_E_N

Picked one of these up myself as well. Anyone have a link to discord?
---

## Post #449 by rmri

[quote="loooops, post:446, topic:66795"]
Might take the risk on a couple at this p

[/quote]

Got mines from New Republic, both worked fine.
---

## Post #450 by B_E_N

I got my stem player a few days ago. The first time I connected it to my Mac, I got macOS security system prompt  asking “Allow accessory to connect? Do you want to connect the USB accessory to this Mac?”. While I was trying various button combos and presses, trying to see if there’s a disk mode to activate, I got this prompt again momentarily. It popped up for a second and before I could reach for my trackpad, it went away. If there is a disk mode, wouldn’t I get this security prompt the first time, for that mode?

I tried to replicate but couldn’t. I don’t recall the exact combo I did but was something with the power/fn button and the play button while plugging and unplugging USB. 

I was trying different combos somewhat quickly so maybe it was a fluke. What do you guys think?
---

## Post #451 by opd

Dope, just bought another 2. Now I feel a lot better about taking apart my current one!
---

## Post #452 by theflwrboy

I just got a couple of the $40 New Republic units. They work great! I was able to get the “Allow accessory to connect? Do you want to connect the USB accessory to this Mac?” message to appear by holding the PLAY and FUNCTION buttons. It happened twice. The second time, I selected Allow and now the message no longer shows up.
---

## Post #453 by Virtual_Flannel

I think thats just to allow charging.
---

## Post #454 by theflwrboy

By using these instructions,

“To find a hidden USB device on your Mac, first **<mark>check **[System Information](https://www.google.com/search?q=System+Information&rlz=1C5CHFA_enUS902US902&oq=how+to+find+usb+device+that+is+hidden+&gs_lcrp=EgZjaHJvbWUqBwgBECEYoAEyCQgAEEUYORifBTIHCAEQIRigATIHCAIQIRigATIHCAMQIRirAjIHCAQQIRirAjIHCAUQIRifBTIHCAYQIRifBTIHCAcQIRifBdIBCTE3NjczajBqN6gCALACAA&sourceid=chrome&ie=UTF-8&mstk=AUtExfAoFPE7Y7Ke-1wDn8bQWkxYUqYU31524oMGK3Ct90VDK47o588TVG6zpGM6FTsiJP8DPxTkgR4V3-W-eKTm6dHAIzRrf_fbrCZzd8YmttJc8k99l-BhQFh-qK1SAM_WmlBlcyMKvYby35pdlC3vx0ly835tW15Y8pbJTr0BKSWmYk4&csui=3&ved=2ahUKEwjH49Xc78yRAxXwl2oFHRIzJ38QgK4QegQIARAC)** (Hold Option + Apple Menu > System Report > Hardware > USB) to see if the hardware is detected</mark>**. If it’s detected but not visible, check **[Finder Settings](https://www.google.com/search?q=Finder+Settings&rlz=1C5CHFA_enUS902US902&oq=how+to+find+usb+device+that+is+hidden+&gs_lcrp=EgZjaHJvbWUqBwgBECEYoAEyCQgAEEUYORifBTIHCAEQIRigATIHCAIQIRigATIHCAMQIRirAjIHCAQQIRirAjIHCAUQIRifBTIHCAYQIRifBTIHCAcQIRifBdIBCTE3NjczajBqN6gCALACAA&sourceid=chrome&ie=UTF-8&mstk=AUtExfAoFPE7Y7Ke-1wDn8bQWkxYUqYU31524oMGK3Ct90VDK47o588TVG6zpGM6FTsiJP8DPxTkgR4V3-W-eKTm6dHAIzRrf_fbrCZzd8YmttJc8k99l-BhQFh-qK1SAM_WmlBlcyMKvYby35pdlC3vx0ly835tW15Y8pbJTr0BKSWmYk4&csui=3&ved=2ahUKEwjH49Xc78yRAxXwl2oFHRIzJ38QgK4QegQIARAD)** (Finder > Settings > General/Sidebar) to show external disks, use **Disk Utility** to mount it, or use Terminal commands like `diskutil list` to force mount it.”

I was able to find it hidden in this menu.

I wasn’t able to find it in **Disk Utility,** and I know nothing about coding so I’m not really sure where to go from here, but maybe this is helpful for someone who does??

![Screenshot 2025-12-20 at 11.15.36 AM|690x461](upload://y5Ce8bjr133HSGWl7iVPrD9FDGK.png)
---

## Post #455 by vanklomk

How quickly did they ship? I haven’t gotten any shipping email for the two I bought a few days ago. Was hoping to gift one to a friend for Christmas. Not a big deal, just curious.
---

## Post #456 by Sumusoid

I bought 16 days ago. It took 5 days for me to get ‘item shipped’ email. Now DHL tracking shows ‘Label created’ which would mean the item has not even left the country yet. I just hope the tracking sucks and the shipping has gone already forward.
---

## Post #457 by PedalsandChill

It’s a long shot. But I'm guessing people have already tried this on a windows computer?

https://teenage.engineering/downloads/usb-asio
---

## Post #458 by opd

Yeah it doesn’t work, unfortunately.
---

## Post #459 by TimK

I’m sorry everyone, but the *“Allow accessory to connect?”* prompt on MacOS should happen for any device that enumerates on USB (depending on your system’s privacy and security settings). It’s unfortunately not an indication of any kind of mass storage functionality, just the most basic USB implementation. I have created many prototypes on nRF52840 that have no MSC USB functionality but still will make this prompt pop up.

The entry in System Information is also perfectly normal, no secrets there at all.

This is what I see for example for a custom firmware I’ve loaded onto my stem player. I know there’s absolutely no mass storage or “disk mode” functionality on mine, because I didn’t implement it in this firmware. (I did however implement CDC/ACM)

![Screenshot 2025-12-25 at 22.21.37|535x500](upload://jpb9DKzcv1R31qi7V3RDfsP6HQq.jpeg)

![Screenshot 2025-12-25 at 22.23.10|606x500](upload://40cnRKt8lG2xTpPFJJeAxuhqCqd.jpeg)

Sorry if I’m destroying dreams here. 🫣

On the other hand: I’m planning to continue working on firmware for the stem player this week. I hope to have some news for you all soon.

Happy holidays everyone.
---

## Post #460 by PapaDragon

Just got one as a gift, didn't even know it existed. But excited for when we can use it's full potential. Also I keep hearing there's a discord server?

![PXL_20251226_005807231.MP|281x500](upload://h8JAAe1ygvAvsMavm0poaZnETQB.jpeg)
---

## Post #461 by _pete

This is the best news I’ve heard all week. Thank you for all of your hard work, Tim!
---

## Post #462 by SlightlySid

I just ordered mine, I’m going to attempt to build a custom firmware for this device. I’ve read through this thread a couple times to try to get up to speed. Would you be open to sharing the documentation on the progress you’ve made?
---

## Post #463 by TimK

I have a lot of documentation to help you get started. I’ll DM you soon
---

## Post #464 by KHAGE

Here is a good start…

[TE Stem User Guide.pdf|attachment](upload://mcarkBtLzNyjQ9TYEG983f7mDWy.pdf) (57.4 KB)
---

## Post #465 by SlightlySid

Very much appreciated @TimK looking forward to it!
---

## Post #466 by _pete

stock update: new republic stem players are apparently sold out. i find it hard to believe, given last time i checked the website, it seemed as though there were 50-60 in stock. maybe they were unlisted because of increased interest so that they could be relisted at their original price. 

either way, we’re the lucky few.
---

## Post #467 by matej

Same on Paris Saint. About a week ago they had around 60 units in stock, and New Republic showed 80 (checked by adding the maximum quantity to the cart). Makes you wonder if someone decided these shouldn’t be sold anymore.
---

## Post #468 by TimK

Thanks Khage! This document is very useful to me, since I’m trying to recreate most of the original functionality. Do you know the origin of this PDF or who made it?
---

## Post #469 by KHAGE

I’m not sure… but since I only discussed (and discovered) TE Stem here… I guess I found it here… much higher up in the thread.
---

## Post #470 by TimK

@instantjuggler did you create this PDF or know the author?
---

## Post #471 by instantjuggler

i didn’t creat the .pdf

its a pretty crazy story, though i have my suspicions…

i had just moved to a new apartment in stockholm. only my parents had the address. one day a large envelope shows up in the mail. inside are those instructions. but, like, not just as a random .pdf printed out. it was somehow, not typed per se, but on a different kind of paper, and with some color on various parts of the document.

someone inside TE who knew that i had a prototype had sent it to me undercover. the reason i think this is because its not quite true that only my parents had the new address… because i had actually placed an order with TE right upon moving and had the order shipped to my new address!!!

i know TE is under NDA about the entire stem player thing. i can’t get a single one of them to even acknowledge that the device exists. but i have a feeling someone there wanted to help out. i ended up photographing the papers and sent the originals to stemplayerarchive on instagram.

as far as i can tell, people are now just printing out low quality photos of the papers and actually selling them now, for example as part of this package: https://www.north-american.shop/products/yzy-2019-og-unreleased-stem-player-by-teenage-engineering-copy

before TE (allegedly) sent me those papers, we still didn’t know about all the functions because there was only a front scan of the cartoon instruction paper circulating online. everyone thought that for sure if we could only see the back of that cartoon paper we would find out how to load our own stems!!
---

## Post #472 by Zeke_B

That is such a cool story and is SO intriguing!! Super fun and wild component to the story of this entire thing.
---

## Post #473 by TimK

Haha that’s incredible!! Love the stem player lore! Does that mean we don’t know the origin of the PDF itself? Now I’m going to have to try to figure out who posted it first and where! :detective: :smiling_face_with_sunglasses:
---

## Post #474 by instantjuggler

you can see the original photos of the document i received in the first post in this thread. in terms of the actual .pdf that khage linked, i had seen in around before, but always thought it was just someone typing up the information of the papers i received and put online. thinking about it now, of course that could be totally wrong. i was just lining up events with the timeline of when i came across the information… though i was pretty thorough about looking online for anything i could find as events unfolded. the .pdf could be from anywhere i guess!
---

## Post #475 by amandwato

It seems that all the previously listed vendors are now out of stock, is anyone aware of any shops still carrying the Stem Player for purchase?
---

## Post #476 by TimK

I’m so hyped! I got a rough version of the tape speed effect to work! (I thought this was going to be my biggest challenge) 🥳 ⏭️  It’s soo fun!!

Here’s a little demo of my current firmware. All original hardware (in a different enclosure) and still original audio data on stem player memory, but all new firmware. Still a lot of work to do, but we’re getting there. I hope next demo will be with new audio too.

https://youtu.be/D4gLpbTTsPo

Happy 2026 everybody!
---

## Post #477 by ModulationStation

This is super awesome. I love it.
---

## Post #478 by _pete

wow wow wow!! happy new year indeed!!
---

## Post #479 by instantjuggler

this is so amazing! thank you for sharing this and also for sticking with it so far.
---

## Post #480 by vanklomk

Awesome! I have two out for delivery right now. Excited to gift one to a buddy and wait for custom firmware
---

## Post #481 by dddstudio

I don't have one, but this is awesome. Congrats guys! :) I can't wait to see what cool things you come up with.
---

## Post #482 by Sumusoid

I ordered one from New republic. It’s been 28 days or something. Still tracking says only: label created. They are not very responsive :confused:

Great progress on the firmware!
---

## Post #483 by marcus_fischer

woah. amazing.

have you had any luck loading your own sounds onto it?
---

## Post #484 by jaseknighter

@TimK, on behalf of stem players the world over, whether sitting silently packaged and clustered together in large brown cardboard boxes in a coat closet somewhere waiting to be sold, or sitting alone on a bookshelf largely forgotten and collecting dust, feeling sad and lonely, all having given up hope of ever in a thousand years achieving the freedom of expression possible given the design of their hardware and ui controls, yet cruelly shackled by the limitations of firmware restrictions and legal obligations originating from those whose hearts contain contents i could not even begin to describe, i would like to express the utmost astonishment at and gratitude for what you have achieved. thank you.
---

## Post #485 by TimK

[quote="marcus_fischer, post:483, topic:66795"]
have you had any luck loading your own sounds onto it?

[/quote]

I haven’t really tried yet, but we’ve fully decoded the audio and LED data format, as you can see in my demo. (Again, many thanks to @JoseJX for his work and advice!!)

The audio data format is a bit overly complicated for my taste, with very strange byte interleaving AND sample interleaving. Probably to obfuscate the original audio from Ye and stop us from ripping the original stems. We did it anyway.

I don’t foresee any problems loading my own stems (except speed). This is going to be my next challenge. I haven’t decided yet if I’ll change the data format or write software to convert PCM files to the weird stem player format. There is still plenty of space (2GB) on the flash memory to mess around with other stems.

[quote="vanklomk, post:480, topic:66795"]
Excited to gift one to a buddy and wait for custom firmware

[/quote]

Thank you everybody for the encouragement, but please be patient. I’ve already had to pause this for a few months because of work deadlines. This might happen again pretty soon, but I’ll try to keep at it!
---

## Post #486 by instantjuggler

its pretty crazy to be anywhere near loading up your own music- but now the real question becomes… what Ye album are you going to try and load!??!?!?
---

## Post #487 by unfound_accident

Hey, thanks for your great work! I am planning on doing what Ben described and just make the pcb from scratch as a hobby project (not planning to add any high end audio fx features for now, just turning the thing into a usable portable music player first). Could you also DM me the pinout/PCB knowledge you have got so far? would save me some time to focus on PCB design!
---

## Post #488 by damnjeeves

Sad to have missed out on grabbing one of these. Love the design.

I am confused about the “stems” regarding this unit and wonder if that’s a reason it fell apart w TE

With the other Kanye stem player you had to upload whatever song you were trying to put on the device to a site, then it would be ready to put on the player and think that was separating the “stems”
---

## Post #489 by Rubyodingus

I cannot find any internal photos of the EP-2350 (ting), This might be dumb, but I wonder if it shares anything with the stem player.
---

## Post #490 by TimK

[quote="Rubyodingus, post:489, topic:66795"]
I cannot find any internal photos of the EP-2350 (ting), This might be dumb, but I wonder if it shares anything with the stem player.

[/quote]

Look for a teardown of EP-133, I’m expecting to see very similar hardware in EP-2350 if not identical. EP-133 shares nothing with the stem player as far as I can tell. The main MCU of the stem player would be way underpowered for that type of device. *EDIT: oops I misunderstood and thought you were talking about EP-40!*

[quote="unfound_accident, post:487, topic:66795"]
Hey, thanks for your great work! I am planning on doing what Ben described and just make the pcb from scratch as a hobby project (not planning to add any high end audio fx features for now, just turning the thing into a usable portable music player first). Could you also DM me the pinout/PCB knowledge you have got so far? would save me some time to focus on PCB design!

[/quote]

Absolutely! I’ll dm you. But don’t expect too much. All the information I have collected relates to building new firmware for this specific hardware. For example, I didn’t bother at all with documenting (passive) component values, understanding trace impedance, etc… I didn’t even draw a full schematic, only the bare minimum needed for working on the firmware.

Secondly, this design is pretty space constrained. If you want to keep all functionality and mechanical hardware this is going to be a decent project. I would looove to know how much work Teenage Engineering actually put into this abandoned project. To me, starting the hardware AND firmware from scratch for an unpaid project like this is just way too much work.

The current hardware is perfectly capable of being a portable music player. For regular audio, you could probably just do 2 channels /16 bit / 44.1 kHz instead of 8 channels / 24bit / 48 kHz. That would make transfer times over USB not great, but much more reasonable. (6.5x faster)

best,
Tim
---

## Post #491 by katagiri37

I acquired a few units the other day and played around understanding the internals on my microscope. @TimK Would you also be able to share with me any knowledge you have on the device, no matter how little? I also have interest in contributing to a custom firmware if I can wrap my head around it
---

## Post #492 by marcus_fischer

https://www.youtube.com/watch?v=bYQzpZ88jHM&t=43s

I don’t know if it’s an oversimplified explanation of it or not, but this Nothing designer sure makes it sound like he is able to put his own music onto the stem player.  given the crossover between TE and Nothing, perhaps he has access to a tool beyond what is public.

(starts 43 seconds in)
---

## Post #493 by PedalsandChill

I saw this shared somewhere before. Honestly, the creator of it just doesn’t know what they’re talking about.
---

## Post #494 by burnt

i thought i read somewhere that the ting is based around the rp2350 hence the name ep2350
---

## Post #495 by TimK

[quote="burnt, post:494, topic:66795"]
i thought i read somewhere that the ting is based around the rp2350 hence the name ep2350

[/quote]

Oh I was confused I thought @Rubyodingus was talking about EP-40 Riddim!

[quote="Rubyodingus, post:489, topic:66795"]
I cannot find any internal photos of the EP-2350 (ting)

[/quote]

So no, I wouldn’t expect it to be a copy of EP-133 🤦🏻‍♂️
---

## Post #496 by Rubyodingus

EP-133 would be way too big, I am referring to the hand held mic, EP-2350. It probably has no relevance, but considering it's size and functionality, I thought it may have similar internals. The ting is held by 4 Phillips, im surprised I cannot find one internal photo. Also, this is probably another stupid suggestion, but I wonder if the tool to upload samples to the ting, could work for the stem player. 

![images (3)|398x500](upload://lcfWwraKOQO0BoEAZrfugAIb2Cu.jpeg)
---

## Post #497 by instantjuggler

no i’m sorry, these are 2 totally different things. even though they were both made by TE (the stem player and the mic) they have nothing to do with each other.

i guess NDA’s are forever… wish there was a time period after which they expired. this whole stem player story is such a mystery, no? i don’t know anything about hardware, but seems that there is at least 2000+ hardware units of the stem player that were produced. and its all custom hardware, not trivial to produce. just the expense of making all those units is already incomprehensible to me.

there was some interview with jesper (TE CEO) not too long ago where he talked about some recent product, i can’t remember which one, and said that if the product didn’t work out then the company would be bankrupt. maybe it was the tp-7 or something, or the EP series. but anyway that kind of also signals how the company manages its money or whatnot? like that they would just go ahead and produce thousands of units of the stem player hardware, without the release side of things following through.

people in this thread keep talking about how there must be some sort of hidden interface to load on new music. but you have to remember that originally the TE stem player was a friends and family only release to promote JIK. either there never was any intention of selling the player publicly, or if there was then maybe anyway it could have been locked to that one album only. just because we want it to have different music on it doesn’t mean TE ever developed that side of the project!
---

## Post #498 by kasselvania

Besides some NDAs being forever, there are sometimes orgs that simply engender a lifelong importance and loyalty from employees. As an example; early people of Nintendo of America are sometimes even reluctant to speak about specifics from nearly 40 years ago. Not for fear of retribution, but simply to not color the legacy, feeling the orgs themselves are the true orbiters of their stories. 

It’s an odd one, but like, try and get ANY deets from a Samsung employee in S. Korea! If you can, a lot of corporate espionage companies would pay you handsomely.
---

## Post #499 by TimK

[quote="marcus_fischer, post:483, topic:66795"]
have you had any luck loading your own sounds onto it?

[/quote]

I have now… :upside_down_face:
---

## Post #500 by KHAGE

[quote="TimK, post:499, topic:66795"]
I have now… :upside_down_face:

[/quote]

Hohoho… :raising_hands: :raising_hands: :raising_hands: 

The dream becomes reality…
---

## Post #502 by TimK

[quote="burnt, post:494, topic:66795"]
i thought i read somewhere that the ting is based around the rp2350 hence the name ep2350

[/quote]

@Rubyodingus

Here it is: https://op-forums.com/t/ep-2350-ting-customizable-samples-and-effects-via-config-json/30479

Interesting, I see they’re using UF2 too!
---

## Post #503 by Stoff9

Too good to be true, that was the point for me to register :smiley: …just to thank you an all for your work.

I ordered 3 devices a few weeks ago, I would love to get my own stems on this device. 

I don’t really want to play other songs on this device but how cool would it be to make a song on the OP-1 and get my own stems on this thing. :heart_eyes: 

Again thank you all for getting to this point!
---

## Post #504 by glia

i’m in amazement :saluting_face: 
just assumed this would *never* be possible or i would’ve bought a unit

if you ever make a file converter as well i’ll release some songs in stemplayer format for people to mess around with
---

## Post #505 by bobbcorr

*Aprés ça le deluge.*

Seems like this would be a good time to plunk for one of these monsters, but I fear I have snoozed and therefore ... loosed? 

I'll just go have a lovely cup of tea to dispel the GAS. May the odds be in your favor.
---

## Post #506 by ndnr

Is there actually any place? where they are still available?
---

## Post #507 by amandwato

I haven’t been able to find a shop
---

## Post #508 by TimK

@Virtual_Flannel pointed me a little in the right direction. I think I’ll just use the multichannel WAV format and modify the firmware I have to be able to play that instead of the weird stemplayer format.

We still have to interleave the LED data (envelopes) and looping cue points but that’s nothing. So I think I’ll go 24 bit, 48kHz 8 channel WAV (little endian). That should be easy to export, edit, etc. from regular audio software like Audacity, right? 

Does anyone know of good software to edit cues/ loop points? 

Some feedback from my current firmware:

![Screenshot 2026-01-10 at 08.54.32|516x500](upload://4lkSBXpN5C5MLO9GIMkwHOHxLNK.png)

💾👷🏻‍♂️
---

## Post #509 by FaremisSound

Fantastic news and so thankful for your work in making this all possible! I’d suggest Reaper. It’s very inexpensive (also has a fully featured trial that never expires). Is this what you mean by cues/loop points functionality? [https://youtu.be/TbiKhJ_9CxM?si=Age_SgfH51GKp9jU&t=126](https://youtu.be/TbiKhJ_9CxM?si=Age_SgfH51GKp9jU&t=126)
---

## Post #510 by q_ben

I’d take a look at the Morphagene file setup and conversation about loop point control as it uses a very similar format.
---

## Post #511 by glia

[quote="TimK, post:508, topic:66795"]
Does anyone know of good software to edit cues/ loop points?

[/quote]

pretty [easy to add cues with this](https://www.ecstasyjones.com/tp-7-wav-editor) but i have no clue whether export format is tp specific

![waveditr|690x384](upload://zCquZbLzPKLb3CRqOamRobAzVK4.jpeg)
---

## Post #512 by _pete

Ahhh this is huge, Tim! Congrats! Can’t wait to see what comes next.
---

## Post #513 by earlyplastic

I’m wondering too. I found one on ebay

EDIT: also I found mine by just searching “yzy stem player”, it was cheaper than any that come up when searching “teenage engineering stem player”, so it may be worth trying that in the future, though none are coming up at the moment.

Curious if any will pop up in any other shops!
---

## Post #514 by Cementimental

[quote="TimK, post:508, topic:66795"]
Does anyone know of good software to edit cues/ loop points?

[/quote]

[Endless Wav](https://www.kvraudio.com/product/endless-wav-by-bjoern-bojahr) maybe?
---

## Post #515 by Jonathan_Riley

[quote="TimK, post:508, topic:66795"]
Does anyone know of good software to edit cues/ loop points?

[/quote]

For a simple user experience, it might make sense to build a standalone app for the entire process, which could including dumping old firmware, installing new firmware, editing files on the device and adding cue points.

How much onboard storage does the stem player have? Is it limited to roughly Jesus is King?
---

## Post #516 by Okey_Nwachie

So amazing bro! Thanks for all your work I had basically given up. I am not rich, but drop your venmo or something I know some of us won’t mind contributing. I just want my own music on this thing and cant believe it might actually be a possibility
---

## Post #518 by TimK

JIK takes about 1.9GB, there’s another 2GB completely empty. My new “song X” is in that space right now.

A standalone app that does everything sounds great, but also a lot of work… The more I can take advantage of existing tools, the better.

@glia  That TP-7 WAV editor looks great! Maybe we could convince the developer to make a version for our little stem player?

@Okey_Nwachie @Nwilton  Thank you for wanting to contribute, that’s very kind! I’ll think about this once I have something that is actually ready to share!
---

## Post #519 by shellfritsch

could the stem player use the same file format as tp-7? then folks lucky enough to own both could xfer tp-7 recordings to the stem player?

i don’t have a stem player - and i’m kicking myself for not picking one up when they were available.
---

## Post #521 by matej

I see they’re back in stock at New Republic, cheaper than ever :smiley: 

https://shopnewrepublic.com/products/yzy-2019-og-unreleased-stem-player-by-teenage-engineering
---

## Post #522 by Stoff9

Thank you! Next order is placed.
---

## Post #523 by _pete

ohhhh!! what a lovely treat!
---

## Post #525 by Durdee

Yea! Thanks for the heads up, at this price it was impossible to resist.
---

## Post #527 by attowatt

already had two in the shopping cart, then realized that shipping is more than the unit itself (45$) ... 🫠
---

## Post #528 by TimK

[quote="shellfritsch, post:519, topic:66795"]
could the stem player use the same file format as tp-7? then folks lucky enough to own both could xfer tp-7 recordings to the stem player?

[/quote]

Highly unlikely because the stem player doesn’t have a filesystem or even really files.. There are no traditionally headers etc. like in a standard WAV file on the stem player flash memory. 

I would assume the TP-7 has a file system and can play multiple file types, but I know nothing about TP-7 so I’m really just guessing here. But man that is one beautiful device!!
---

## Post #529 by fyoosh

[quote="TimK, post:508, topic:66795"]
Does anyone know of good software to edit cues/ loop points?
[/quote]

Following this project with great interest and enjoying reading your updates Tim. 

Just thinking out loud here so I might be way off the mark. But are cue and loop points similar from a programming perspective to how the Make Noise Morphagene’s WAV files use splice markers?

I remember someone made an open source web app for adding those splice markers to WAV files for the morphagene. 

Here’s the link: https://knandersen.github.io/morphaweb/

And a blog post about it too: https://projects.kevinandersen.dk/week-79-morphagene-reel-splicer/

If there is a similarity here, maybe that open source project could be useful for this stem player project.
---

## Post #531 by TimK

Thank you! This looks very interesting!

I was already thinking about a web app for converting multichannel WAV to stemplayer data. That would make it much more more portable than writing everything in C/C++ like I’m used to.

I’ll definitely look at this as a starting point for a conversion/ uploading tool.

[quote="Sumusoid, post:482, topic:66795"]
I ordered one from New republic. It’s been 28 days or something. Still tracking says only: label created. They are not very responsive :confused:

[/quote]

How’s shipping etc. going for those who ordered from new republic? I’m thinking about getting a spare device in case I mess up my dev unit.
---

## Post #532 by instantjuggler

If you ever need another unit just let me know and I can send you one!
---

## Post #533 by Oktatracker

Just chiming in here to echo what others are saying. Fantastic work - would be so cool to be able to add other songs and stems to this little machine. Let me know where I can support your work.
---

## Post #534 by TimK

[quote="instantjuggler, post:532, topic:66795"]
If you ever need another unit just let me know and I can send you one!

[/quote]

Thank you! That’s so generous, I’ll keep it in mind! Did you order a pallet of them? 😂

@Oktatracker thank you, I will figure something out when I have something to share!
---

## Post #535 by jedgar

They're now down to $35 at New Republic https://shopnewrepublic.com/products/yzy-2019-og-unreleased-stem-player-by-teenage-engineering
---

## Post #536 by tunelight

Still no news or update on your order?
---

## Post #537 by vanklomk

I ordered mine on 12/17, they shipped on 12/23 and it arrived on 1/2.
---

## Post #538 by Sumusoid

Few days ago I canceled the order since nothing had happened except the “label created”
---

## Post #539 by tunelight

Thanks for the reply; I might have to do the same, I am not seeing great reviews.
---

## Post #540 by bobbcorr

1/16: order placed 
1/21: order shipped
1/24-25: Snowmageddon in DC (est.)
---

## Post #541 by ParliamentLite

I had a weird experience with New Republic, but I received my Stem Player without incident.

I ordered a shirt and a pair of pants (I was too tempted by the Palace X Gap shirt), tracking indicated the shirt had shipped with the stem player. Got the box and there was no shirt inside! Turns out they were sold out and wound up sending me a refund… but it was strange that the tracking said they both came out together. Anyway I would love to listen to anything else on this thing, but it’s been fun so far!
---

## Post #542 by ATTRKTR

Perhaps they prioritise domestic orders over international, because I also placed my order on the 16th (shipping to Tasmania) and my tracking hasn’t updated past “Label Created”.
---

## Post #543 by Voltmeister

After following this thread since mid 2024, I decided to order one too. I hope one day this can hold stems of my own work for processing live.
---

## Post #544 by KHAGE

Not sure… I ordered two more units two days ago (Monday, January the 19th) and I already have an UPS tracking #…

It’s my third order with New Republic. No problem so far.
---

## Post #545 by deezy7

Started following in August of 2024, just ordered two Stem Players from New Republic for $35 each on Monday, January the 19th and got a UPS tracking number this morning, says estimated arrival for the 28th.
---

## Post #546 by Galapagoose

Perhaps it’s already widely known, but unless I’m mistaken, you’re going to need an SWD programmer to change the firmware on these things, and perhaps do some soldering to attach it. Just wanted to avoid anyone buying in on the assumption that the firmware swap would be simple.

I kind of hope this isn’t all the way true, so please correct me if wrong!
---

## Post #547 by crucFX

ordered on the 16th, label created almost immediately, and it was shipped via fedex on the 20th, currently in transit. (live in WI)

\* Arrived today (the 26th)
---

## Post #548 by kasselvania

Claude, remove these cursed audio files and replace them with my field recordings run through a torso S-4 without need for soldering or deep firmware rewrites. Make no mistakes.
---

## Post #549 by TimK

[quote="Galapagoose, post:546, topic:66795"]
Perhaps it’s already widely known, but unless I’m mistaken, you’re going to need an SWD programmer to change the firmware on these things, and perhaps do some soldering to attach it.

[/quote]

I too hope this isn’t all the way true. Soldering a header makes it impossible to put the device back together. (As is the case with my current dev unit)

![TE-SP0_with_SWD_header_web|666x500](upload://igGrd94NxDAn2x4NXc7p98v4Tzh.jpeg)

This right here doesn’t fit in the tiny stem player enclosure!

To avoid soldering, we can program a bootloader using a pogo pin connector or even just 3 jumper wires and steady hands. I have done this before when I forgot the right connector and urgently needed to update firmware on a prototype. 😵‍💫 It’s not fun, but it’s possible and it would only need to work once.

You do need a debugger for this (I use the nRF52840-DK) and you need to completely open up the device since the only access to SWD is at the back of the main PCB.

After a bootloader has been installed (and the TE firmware has been wiped), the device can be closed up again and firmware updates can happen via USB.
---

## Post #550 by Voltmeister

This sounds very hopeful and workable. Great!
---

## Post #551 by bobbcorr

Yeah! This sounds (wipes forehead) very doable especially (fingers shake) for someone like me with (drops phone, kicks it across the room) such steady hands and (pours a drink) calm nerves. :)
---

## Post #552 by Rubyodingus

I ordered mine on the 16th, still just confirmed. I emailed their support and they said they switched warehouses and they're getting caught up
---

## Post #554 by TimK

Do you mean the enclosure for my dev unit? That’s only to hold the PCBs and keep the thing functional while connected to a debugger. So only useful when developing/testing firmware with this device.

There was a pretty nice 3D printed cover posted here before, just scroll up a few hundred messages! 😎
---

## Post #555 by katagiri37

I’ve been hospitalised for the past two weeks with the flu going around but I think I might be brought back to life with these updates. Great work @TimK
---

## Post #556 by B_E_N

This button combo appears to create a USB serial interface for 60 seconds. I haven’t been able to successfully connect to it. Maybe this interface is meant to upload firmware?

1. Make sure device is unplugged and powered off
2. Press and hold track buttons 1 and 4 (continue holding)
3. Plug in USB
4. P̶r̶e̶s̶s̶ ̶a̶n̶d̶ ̶h̶o̶l̶d̶ ̶p̶o̶w̶e̶r̶ ̶b̶u̶t̶t̶o̶n̶
5. After \~5 seconds, LED 1 will turn on and you can release all 3̶  2 buttons.

Edit: It appears that step 4 is not needed.
---

## Post #557 by kasselvania

I have a sneaking suspicion that this is a TE psy Op and doing this combo will cause these unsanctioned devices to explode like a tape player in Get Smart
---

## Post #558 by KHAGE

How do you say “*je prends mon mal en patience*” in English ?

![IMG_8051|375x500](upload://ncf2qHXP8lWirV9T62E9oBhTjFk.jpeg)
---

## Post #559 by _pete

I had a feeling there was a button combo lurking in the device protocol somewhere. Gonna try this now.
---

## Post #560 by TimK

Amazing discovery! What happens after the 60 seconds? 

(I have triggered very similar behaviour in a different way: Track 1 LED lights up, the device is unresponsive but enumerates as a serial port, after a short while it just turned off I believe. I couldn’t figure out how to communicate with it yet)

@KHAGE 😳 that is an impressive setup. They feel very related to each other. 
Is TP-7 as fun as it looks? I’m having a lot fun playing with the tape speeds on SP01, must be so cool on TP-7!
---

## Post #561 by _pete

After trying this, my computer asks permission to connect, and after hitting allow, the track LED remains on, and I think after 60 seconds (I'm guessing it’s waiting for a specific signal) without inputs, my device just goes into the normal boot sequence and turns on as normal. 

I’ll try doing this a few more times to see if anything different happens. Tried opening Disk Utility (on a Mac), Teensy flasher, and Balena etcher just to see if the device would show up as a drive or destination - no such luck.
---

## Post #562 by TimK

If you’re on Mac, do `ls /dev/cu*` when the Track 1 LED is on. I think you should see a serial port.
---

## Post #563 by _pete

Hmmmm… no luck yet on my end. I’ll troubleshoot with a USB hub and edit this post if I get anything juicy.
---

## Post #564 by vanklomk

I noticed in device manager that this button combo makes it show up as a COM5 device and after 60 seconds that disappears and it just starts showing as “stemplayer” with no drivers. I’m not smart enough to understand the logs at all.
---

## Post #565 by bobbcorr

[quote="KHAGE, post:558, topic:66795"]
How do you say “*je prends mon mal en patience*” in English ?
[/quote]

"I can be patient" is an approximate translation, but the literal translation is more fun, and doesn't translate as readily.

(and because I can't add two consecutive replies, I present my second thought below)

I'm guessing TE is a Mac shop ... it is possible they're using the DFU (device firmware update) protocol, which an AI buddy described as " <mark>a protocol and mode allowing electronic devices to update or restore their firmware without specialized JTAG hardware, usually via USB</mark>."

If there's a way of using DFU mode to interrogate the device, that might be ... useful?

My apologies if I've just explained water vapor to a cloud.
---

## Post #566 by B_E_N

Output from a couple of commands on my mac. Going to continue poking around. 

```
10:31:50 ~ $ ls /dev/cu.* /dev/tty.*

/dev/cu.usbmodemD00AD2B9505D1
/dev/tty.usbmodemD00AD2B9505D1
```

```
10:32:00 ~ $ nrfutil device list

D00AD2B9505D
Product         stem player
Ports           /dev/tty.usbmodemD00AD2B9505D1
Traits          serialPorts, usb

Supported devices found: 1
```

```
============================================================
USB DEVICE INFORMATION
============================================================
Vendor ID:  0x2367
Product ID: 0x1701
Device:     stem player by teenage engineering
Serial:     D00AD2B9505D
Bus:        0, Address: 1
Class:      0
SubClass:   0
Protocol:   0
USB Ver:    2.0

============================================================
CONFIGURATIONS
============================================================

Configuration 1:
  Interfaces: 2
  Attributes: 0xc0
  Max Power:  500mA

  Interface 0:
    Alt Setting: 0
    Class:       2 (CDC (Communications))
    SubClass:    2
    Protocol:    1
    Endpoints:   1
      Endpoint 0x82 (IN):
        Type:        Interrupt
        Max Packet:  64
        Interval:    16

  Interface 1:
    Alt Setting: 0
    Class:       10 (CDC-Data)
    SubClass:    0
    Protocol:    0
    Endpoints:   2
      Endpoint 0x81 (IN):
        Type:        Bulk
        Max Packet:  64
        Interval:    0
      Endpoint 0x01 (OUT):
        Type:        Bulk
        Max Packet:  64
        Interval:    0

============================================================
RAW DESCRIPTOR DATA
============================================================
Device Descriptor: 12 01 00 02 00 00 00 40 67 23 01 17 00 01 01 02 03 01
Config Descriptor: 09 02 4b 00 02 01 04 c0 fa 08 0b 00 02 02 02 01 00 09 04 00 00 01 02 02 01 00 05 24 00 10 01 05 24 01 03 01 04 24 02 02 05 24 06 00 01 07 05 82 03 40 00 10 09 04 01 00 02 0a 00 00 00 07 05 81 02 40 00 00 07 05 01 02 40 00 00
```

[quote="vanklomk, post:564, topic:66795"]
I noticed in device manager that this button combo makes it show up as a COM5 device and after 60 seconds that disappears and it just starts showing as “stemplayer” with no drivers.

[/quote]

This is the same behavior on my Windows PC as well. It shows up as COM5 and 60 seconds later it shows “stemplayer” with no drivers. This was my first time plugging it into my Windows PC. Does anyone know if the no driver thing something that has always happened for folks on Windows?

[quote="TimK, post:560, topic:66795"]
(I have triggered very similar behaviour in a different way: Track 1 LED lights up, the device is unresponsive but enumerates as a serial port, after a short while it just turned off I believe. I couldn’t figure out how to communicate with it yet)

[/quote]

Do you recall how you were able to trigger it?  Perhaps triggering it in a different way allows for other capabilities.. I’ve been trying various nrfutil commands to poke around but none of them seem supported.

[quote="kasselvania, post:557, topic:66795"]
I have a sneaking suspicion that this is a TE psy Op and doing this combo will cause these unsanctioned devices to explode like a tape player in Get Smart

[/quote]

I think we have stepped into “this may brick your device” territory.
---

## Post #567 by KHAGE

[quote="TimK, post:560, topic:66795"]
@KHAGE :flushed_face: that is an impressive setup. They feel very related to each other.
Is TP-7 as fun as it looks? I’m having a lot fun playing with the tape speeds on SP01, must be so cool on TP-7!

[/quote]

The TP-7 is a little gem that brings back fond memories of the days when I used to enjoy tinkering with my recordings on my ReVox reel-to-reel tape recorder. 👌🏽🙌🏽
---

## Post #568 by TimK

[quote="B_E_N, post:566, topic:66795"]
I think we have stepped into “this may brick your device” territory.

[/quote]

I agree, I think the right command in those 60 seconds could potentially erase a flash page or two. Proceed with caution and backup your data 😎
---

## Post #569 by Rubyodingus

For anyone else who ordered from new republic, I ordered the 16th, and it just shipped (26th)
---

## Post #570 by tunelight

Thanks everyone who confirmed shipping notifications, I just received mine to the UK, order placed on the 19th : )
---

## Post #571 by zzzach

Same here, ordered on the 18th and got a shipping email today. UPS doesn’t have the package yet.

From the email:

> *We’re happy to let you know that your order is being processed to ship out soon. During holidays or promotional periods when there’s higher order volume, we may experience delays in shipping out your order. Please contact our Customer Service team if you do not see your tracking update in the next 5-7 business days.*
---

## Post #572 by DGoods

Ordered 2 Units on 21st and it should arrive today in Germany.

Could someone send me a link to the discord please.
---

## Post #573 by tunelight

Oh is there a dedicated discord? Yes please do share : )
---

## Post #575 by earlyplastic

The one I ordered from ebay showed up today. Shocked by how small it is!
---

## Post #576 by nuk

any one else had problems with shopnewrepublic? i placed a order on the 6th december and still haven’t gotten it, they keep saying they are moving warehouse etc, and now they have stopped replying to my emails all together.

![New Project|666x500](upload://n5xH6Exsh4x68g2DGOEHqvU4AaL.png)

EDIT: Ended up getting a refund. with the statement

”We do sincerely apologize but we are having some issues shipping the Stem players internationally. At this time it would be the best to issue you a full refund. Please allow 5-7 business days for the refund to be fully credited back. We apologize for the inconvenience. ”
---

## Post #577 by ATTRKTR

Yes, I’ve had the same problem with international shipping via DHL since Jan 23rd, saying:

“PACKAGE SCREENING EXCEPTION - CANNOT BE SHIPPED INTERNATIONAL”.

I was told New Republic was talking to DHL about the issue, but haven’t had an update so far. 

I ordered a second one via eBay which hasn’t had this issue and has already arrived in the destination country.

Hopefully something happens soon…ish.
---

## Post #578 by evnander

Dang, sorry to hear that a few of y’all are having issues. Mine just arrived here in NYC today, after having placed an order on January 18.

And yeah, wow, these are *shockingly tiny.* For comparison, here is one next to an airpods pro case.

![IMG_2444|375x500](upload://9VxZfIuEVb3s8AAa7rkvsFqAREW.jpeg)
---

## Post #579 by fyoosh

I ordered on 16/01 and it was shipped with UPS on the 21/01. It’s been stuck in the UPS facility in Ontario until now though. I’m assuming because of the storm. Hopefully it’ll get moving soon. 

@evnander I knew they were small but wow it is **small**!
---

## Post #580 by earlyplastic

FYI I also ordered some from newrepublic, it took them a short while to ship, but they arrived today
---

## Post #581 by tim_g

Hey everyone! I have been reading here for the past month and it’s been a real pleasure seeing all of your testing and results!

I just received one of the two I ordered from them too, and the other one is having the same issue as you all with delays and whatnot, but they just updated a new shipment so I guess we’ll see. They seem to respond quick at least.
---

## Post #582 by tunelight

Mine is finally moving too, now in Ontario (UPS). It will be delivered later than stated when shipping label was created but at least it is now on the way.
---

## Post #583 by Kaj_West

I ordered 9/12 shipped 13/01 got stuck 16/01 newrebulic no longer answering emails. I don’t know what to think at this point.
---

## Post #584 by okyeron

(FWIW - ordered from NewRepublic on Jan 17 - received today in US)

Is there a website or something with all of the collected hacking knowledge so far?  (pinouts, etc.) There’s multiple mentions of a discord, but I’ve DM’d a few people from this thread and not received any information about that.

I’d be happy to host a GitHub repo or personal website page to collect whatever public knowledge there is on this thing.
---

## Post #585 by TimK

I haven’t shown any progress in a while. That doesn’t mean there’s no progress. I have just been a little busy.

![Screenshot 2026-01-22 at 14.36.12|690x456](upload://5Dk6aA1cEV5qe9hx8Cg5iez65VI.jpeg)

I hope more soon.

I also did order a spare from new republic but never received it. UPS says it never left the US (I'm in EU)  ¯\\\_(ツ)\_/¯
---

## Post #586 by Voltmeister

I am in NL and had that too. UPS shipping started January 26 and “stalled” after a week or two. Today I saw it was scanned/located in a warehouse in Cologne, Germany. Quite unexpectedly, but hopeful. Hope your delivery will soon be on the move too.
---

## Post #587 by TimK

For work I’m regularly having components shipped with UPS from Digikey in the US. I’ve never had an issue with them. So this is pretty strange to me. I think this is the first time I’ve ever even had to contact UPS customer support.

So you're still waiting too? Cologne is not far.
---

## Post #588 by Voltmeister

Yes, I am still waiting. Delivery date was Feb 3 at first. Since it arrived in Cologne it says “The delivery date will be provided as soon as possible”.
---

## Post #589 by DGoods

Ordered on Jan 21 and it stuck since 26th somewhere. I called the support and payed the import charges online. Now it says the delivery date will be provided as soon as possible.
---

## Post #590 by instantjuggler

just wanted to touch base here and give a report (of sorts) on a couple of things-

i’ve been following the TE stem player since it first leaked online, with snippets of video here and there of ye using it in the studio and at a few performances. as soon as i saw it i was really intrigued by the design and tried all i could to learn more. 

i stalked online forums and social media posts day and night for years, looking for information. of course now most recently the extra pallets of units have been found and sold at relatively very low prices. i paid $650 for my first unit back in the early days. so yeah i’m pretty invested in this little piece of hardware.

just wanted to say that i continue to look around online and try to hear what’s going on regarding the TE stem player. when the few thousands of more units were found in a warehouse and sold through various platforms there were tons of people saying they were going to hack the device and add their own music. of course i totally understand the enthusiasm and desire to make that happen. at one point i counted 7 different, how to say, self-organizing groups of people, who vowed to crack the device. 

of course i have also heard the rumors about some mythical discord channel where all the “serious” hackers are working on this project behind the scenes. i’m also not a member of that discord, and i don’t personally know of anyone who is. so of course i have no idea if its actually a thing, and if it is then how far along anyone has gotten over there (if at all), and/or if its still active to this day. then, disregarding that potential group of people, i have to say that @TimK is the only person i know of right now who is actively working on this project… and furthermore is the only person i’ve heard of who has made any progress!

that’s not to say there might be people out there doing things that i don’t know about. but just to give my perspective, with as much effort as i’ve put into keeping up with this device, this thread here on lines is the only active discussion that i know of at this time. all the other chatter elsewhere has died down significantly in the past few months. 

with that being said, its even more amazing what @TimK has managed to achieve so far!!
---

## Post #591 by pdrgf

Hi guys, I ordered a couple of these and they’ll hopefully be arriving this week. Last year I done my first ever hardware project reverse engineering a discontinued STEM toy and it was a ton of fun ([link](https://github.com/padraigfl/twsu-arcade-coder-esp32)). I previously done another software solution to making a discontinued Android NFC based card game somewhat future proof that wasn’t quite as fun  but I enjoyed diving into all the stuff.

Am in no way a hardware developer and have no experience with the nRF52840 so if I was to try and get something running I’d want to mostly focus on some easy wins. With that in mind I have two questions that spring to mind right now:

1. it looks like the thing that’s blocking easy interfacing with the serial pins would be the battery; could we possibly either use a smaller battery to free up the room to access them  (I know we’re talking miniscule sizes, might be only 100mah) or just rely on USB power?
2. how feasible would it be to modify the existing firmware to swap the tracks out? I imagine there’s a high chance that even if it is possible you’d be limited to the exact file sizes of the current tracks embedded onto the device which wouldn’t be ideal considering the length of JIK and its individual tracks. This seems like something people have probably already attempted to do but just checking. \[edit: just had a look into the emmc chip and looks like it’d be a big challenge to interface directly with?\]

Should have it by next weekend but I imagine it’ll take me at least a few weeks to figure out a dev setup. People far smarter than me have already attempted working on this so I imagine the main way I’d be able to assist would be debugging stuff and providing a sanity check for someone else; will message again when I’ve my stuff set up as I’d be happy to help whoever debug what they’re working on

Ordered: Jan 25th
Current status: On the Way (Departed from Facility; Philadelphia, PA, United States)
Updated status: Feb 2nd On the Way (Stansted, United Kingdom)
Another update: Feb 3rd On the Way - Arrived at Facility (Somewhere near me)

Hopefully arriving tomorrow :crossed_fingers:
---

## Post #593 by DGoods

Update: After contacting UPS again it turns out that my package has not arrived in germany so far.

They say its stuck somewhere between Canada and USA because difficulties in clearance.
---

## Post #594 by KHAGE

Two more to come, here (GVA, Switzerland)… also stuck somewhere in the States. 

Meanwhile…

https://www.instagram.com/p/DUQGkJvCLbQ/
---

## Post #595 by ancaja123

On the topic of the original use case of the SP: Does anyone find the MIDI out to be usable? I’ve tested it fairly extensively, and it doesn’t appear that it sends a Start/Stop command, just clock. So it’s fairly hard to sync up, especially considering the audio has to “wind up” for a ms or so. It’s almost like you have to try and press play on another device at the perfect time to get usable sync.
---

## Post #596 by Okey_Nwachie

Ok so this might be dumb but. Does anyone have a Teenage Engineering OB-4? Can that connect to a pc? It seems like that speaker might already do like 50% of what we’d want this device to do (loop, playback speed, scratch etc.) Would there be any way to start from there and build up or maybe that could give insight as to how the stems are stored translated? Lol or im living in cyberpunk 2077 and just ignore me but its an idea I had LOL
---

## Post #598 by _pete

this looks so, so great, Tim! Can’t wait!
---

## Post #599 by megageox

I have an OB-4 and it DOES have a built-in drive mode but no way to upload to it.
---

## Post #600 by TimK

Thanks Pete me neither! I’m getting a bit impatient myself as I’m not advancing as fast as I would like to.

[quote="Okey_Nwachie, post:596, topic:66795"]
Would there be any way to start from there and build up or maybe that could give insight as to how the stems are stored translated?

[/quote]

We already know exactly how the stems are stored in memory on the SP01. The only thing that’s still a little bit mysterious is the sync/midi clock data but I’m very close to figuring that out too.

And of course working on the transfer as we speak.

💾👷🏻‍♂️
---

## Post #601 by BenjyBuilds

Hey, been following this thread for a while, can i ask if you actually believe you can get into this thing without any hardware mods? I want to buy one of the stem players, but i’m not sure if i should if it only plays JIK forever
---

## Post #602 by PedalsandChill

I wouldn’t buy it for the hope of what it might do. Only buy it for what it is. Not to say it isn’t possible. There’s still a lot of work being done.
---

## Post #603 by glia

does ob4 have a stem mode? that would be the main draw for most folks hovering around the topic
---

## Post #604 by instantjuggler

Just to remind people that if you are only interested in stem playback in general, and not specifically the TE device- Kano still sells their stem player which allows you to load your own music onto it and has effects, speed control, loops, etc.
---

## Post #605 by TimK

Yes. I very much agree with this.

It also depends what you understand by hardware mods. I don’t consider anything I'm doing hardware mods. I simply added a header for convenience during development, but this is absolutely not required, and could be removed later.

On the other hand, I've said this many many times already: I think I know this device well enough by now: there is no way to change the firmware (and thus the content) without fully opening up the device for access to the SWD lines on the back of the main PCB.

And once again: any stem/audio uploads over USB will be super slow because of the limitations of the main chip. Downloading the JIK stems from the device takes about 1h !!!

Don’t buy this expecting a firmware update please. 

(I think I'm going to stop responding to the same questions over and over, people can read the thread, it’s a fun read!)
---

## Post #606 by Voltmeister

![IMG_0443|375x500](upload://sSFKGec8mUheyeipGQOSW4GzzsB.jpeg)

@TimK it arrived today with UPS. Went from Cologne to a local facility in NL within a day and home-delivered the next day. Tracking now shows several steps I could not see yesterday. All it showed was “in Cologne”, when actually this thing was on the move/staying elsewhere.

Hope yours too will surface soon!
---

## Post #607 by radioedit

[quote="TimK, post:605, topic:66795"]
(I think I’m going to stop responding to the same questions over and over, people can read the thread, it’s a fun read!)

[/quote]

I’ve gone ahead and made the top post in this thread a wiki so that it’s editable by other users. Perhaps someone can add a short FAQ to the end of that?

Edit: actually, while I’m here, a quick mod note. This thread has for a long time been sailing somewhat close to breaking the [rule](https://llllllll.co/faq) stating: “You may not post descriptions of, links to, or methods for stealing someone’s intellectual property (software, video, audio, images), or for breaking any other law.”. We’ve had to delete a few posts that have crossed that line. We don’t do this because we hate fun and hardware hacking, we do it so the community doesn’t get shut down due to a legal threat. Please be aware of this, and if you’re not sure whether what you’re about to post is going to break a rule then it’s always okay to check with mods first before posting. Thanks everyone! 💚
---

## Post #608 by TimK

Thank you for this! And thank you for the mod note.

I am definitely trying to be careful of staying away from stealing intellectual property. Which is somewhat difficult when you’re trying to figure out undocumented and abandoned hardware and I realise this can be a thin line sometimes. I do hope I haven’t crossed it yet and this is still a harmless and fun adventure. I too like fun and hardware hacking, and have no appetite whatsoever for legal troubles.

(I have in fact tried to ask on here how far I can go with for example sharing my reverse engineering efforts)

Please do let know if/when I’m sailing too close to the edge, I’ll adjust course right away.

Let’s keep this fun!

@Voltmeister Let’s go!!! on my side UPS has admitted to losing the package already and I’ve asked new republic to open a lost package claim with UPS.. I have very little hope anything good will come out of this. Sounds very much like domestic (UPS) shipping from new republic is going well and international shipments are having trouble.
---

## Post #609 by Voltmeister

Oh man, that’s not good. Sorry to hear. Are you able to continue your project? Or do you need another device for that?
---

## Post #610 by pdrgf

Mine arrived today, very surprised to not have any customs charges; did I just get lucky or will I be charged afterwards?

Have 3 and I love the form factor, had read so much about how small it was that I was expecting it to be even smaller but it’s a pretty optimal size for what it does. Looks like all of them were totally flat batteries on arrival, can’t imagine that bodes well for how long the batteries are gonna last seeing as they’re already 6+ years old.

Okay so I guess my next steps are:

1. buy a dev board to familiarise myself with
2. open up one of these and figure out how I’m going to interface with it
3. figure out next steps (mostly just offering to help others I imagine)

Does anyone have an estimate for how many of these were actually made? the old estimates I’m seeing look like they’re massively off; unless literally the only people buying it are posting in here
---

## Post #611 by BenjyBuilds

Ngl, JIK is an alright album, and i might just buy it for fun. Is it a fun device on it’s own? Can someone tell me the functions other than stems?
---

## Post #612 by _pete

The stem player on its own is definitely fun, and the album loaded on it isn’t…..entirely without merit. I bought mine when I reconciled with the idea of using it as an exercise in sampling/audio recycling/repurposing - I figured if I can get any good tonal material out of the (current) only optional audio on the device, I should. I’ve used moments from songs on it in live settings here and there.

The functions themselves are outlined pretty extensively in the thread. Definitely recommend reading a little more from the existing material out there before deciding to commit to owning one, but whether or not you find any of the functions fun or useful is up to you. It has limited looping, some effects, a couple of different playback modes to accommodate the stripped-back controls having influence over each mode. 

As Tim said (wisely), don’t buy one because of what it could be in the future. Buy one because of what it currently is (or don’t) - it’s unfair to expect anything more than what’s already been underway without understanding that there are a lot of fine lines being tread here.
---

## Post #613 by glia

[quote="_pete, post:612, topic:66795"]
The stem player on its own is definitely fun, and the album loaded on it isn’t…..entirely without merit. I bought mine when I reconciled with the idea of using it as an exercise in sampling/audio recycling/repurposing - I figured if I can get any good tonal material out of the (current) only optional audio on the device, I should. I’ve used moments from songs on it in live settings here and there.

[/quote]

this is basically what i plan to do until anything more might be possible…i hadn’t truly been interested until tim seems to have made a leap in progress

that new found interest led me to finding the lowest price of entry to this madness that i’ve ever seen and i’m comfortable with having one regardless what else may occur

since i’d done so little research beyond the core concept of stemming ye music i had no idea this has fx, a rocker (precursor to tp7 mechanism?), and had forgotten (or never knew) that there’s an entire album stored on it

[quote="TimK, post:605, topic:66795"]
there is no way to change the firmware (and thus the content) without fully opening up the device for access to the SWD lines on the back of the main PCB.

And once again: any stem/audio uploads over USB will be super slow because of the limitations of the main chip. Downloading the JIK stems from the device takes about 1h !!!

[/quote]

i’m fine with slow & just need to look into what kind of swd programmer will be best to try with SP 

not in any hurry :)
---

## Post #614 by ancaja123

No, it has a “Disk Mode” that is where your loops (made on the device) are stored, and various other interesting features. Noise machine, metronome, ambient mode, and what they call karma loops. The karma loops are absolutely the most gimmicky thing in the device and I find them to be an ear sore. It is a similar thing, though, where they are seemingly baked in there, and you can’t replace them. 

The only thing that’s similar to the SP, is the speed change and looping, however, it’s much better on the OB-4, in my opinion. Full control over speed, and the looping is designed extremely well. Has some sort of beat detection algorithm that just works amazingly well, gives you perfect loops wherever and however you want, as opposed to the SP which has clear cue points for loops, that never change. That being said, the OB-4 struggles with triplets, and will usually get a loop wrong on a song with triplet style swing, if that makes sense.

There’s also no USB or anything in the OB-4, just if anyone is wondering.
---

## Post #615 by tim_g

If it can help, I was able to connect to it via bluetooth when connected via usb, but it disconnects as soon as it connects. It’s now showing as a audio output device though. I’m wondering if the mac fails the exchange for connection because it needs a specific app to connect (like the pokeball plus controller with BLE for example), but not sure. I know that storage wouldn’t be accessible too via bluetooth, but would have been cool if we could transmit a “verification" key or something.

![Screenshot 2026-02-05 at 00.03.42|690x398](upload://rbYIpZx8mlRdPFneMwtGVyQxmWz.jpeg)
---

## Post #616 by frankchannel

Ordered from New Republic Jan 16th, arrived in Ontario, Canada on Feb 4th (no taxes or duties!) after being stuck in Ontario, California (who knew that was a place?) for several days.

![PXL_20260205_192355400|376x500](upload://eoLERfr5vsJJpEZLh0tiKaQDY5S.jpeg)
---

## Post #617 by Simone

Same story here! 

This thing is actually pretty fun, the button combos and functions really make it a cool novelty. Shame about the Kanye album lol, this would be an actual decent quirky sample mangling machine if you could load your own stuff in. 

Having 35$ (+35$ shipping) worth of fun already but dang the potential of this thing is kinda frustrating haha. It almost makes music as is, though midi sync is sloppy at best.
---

## Post #618 by zzzach

My shipping label was created 11 days ago but UPS still doesn’t have the package. Reaching out to customer service about it now.
---

## Post #619 by tunelight

It took a while from shipping to UPS being able to collect but it should happen eventually. No harm contacting customer service.

My experience has been: 
Ordered 19 Jan, with UPS 29 Jan, received 4 Feb (UK)

@TimK dm if you need a spare device to test with : )
---

## Post #620 by zzzach

I hit them up and they responded in just over an hour. Not bad!

Evidently, UPS tracking got messed up but UPS handed it off to USPS anyway. I got a proper USPS tracking number, and can see that it’s currently in transit. Phew.
---

## Post #621 by TimK

[quote="glia, post:613, topic:66795"]
i’m fine with slow & just need to look into what kind of swd programmer will be best to try with SP

[/quote]

My daily driver is [nRF52840-DK](https://www.digikey.com/en/products/detail/nordic-semiconductor-asa/NRF52840-DK/8593726), which I find great and is not too expensive. Work well with the nordic tools. But it is currently out of stock at Digikey! Must be all the people trying to hack their stem players :slight_smile:
---

## Post #622 by ParanormalPatroler

How about you buy one for me as well and we split on shipping? I stopped at the $45 as well
---

## Post #623 by fyoosh

Just a heads up for anyone else who used New Republic and isn’t in the US. I ordered on 16th Jan, they shipped with UPS, then it got delayed and never left Ontario CA. New Republic support just let me know that

> It seems this item is not being accepted by most countries and they are looking into this to try and find a resolution

Slightly strange because I’ve seen the posts from others receiving theirs in Europe and the UK, where I am, and I think that one person even ordered the same week. Anyway, they refunded me after sending this email and customer support was all good.

 If they put them back up for sale maybe I’ll get a US friend to grab me one instead 🤞
---

## Post #624 by louwrens

Yeah, I ordered mine around the same time, it has made its way into my country down in Africa but has been stuck at customs for inspection for a while now. It was however not shipped via UPS but by a different service. It seems there is something strange going on with how it is classified that might be causing issues.
---

## Post #625 by Rubyodingus

I finally got mine today; I order the 16th of January, the label got made the 26th, then no updates on tracking until today (11th of Feb) where it was just randomly delivered.
---

## Post #626 by chapelierfou

I’m in France and received the 2 I ordered like 2 weeks ago, paid 22€ taxes.
---

## Post #627 by pdrgf

[quote="TimK, post:621, topic:66795"]
[nRF52840-DK](https://www.digikey.com/en/products/detail/nordic-semiconductor-asa/NRF52840-DK/8593726)
[/quote]
would a basic nRF52840 board do the job for getting a grip on things before putting down more money? I've only ever worked with arduino and dirt cheap esp8266/esp32 boards in the past.


Reading up on these boards they sound pretty cool; surely bags of potential for cool interactive experiences at a pretty low cost point with their wireless communication options?

Looks like the entirety of TE's recent suite of products use chips from the same range; I'm guessing that's something to do with the communication capabilities, makes me wonder if this device was ever meant to be able to wirelessly communicate with TP-7/TX-6/etc...
---

## Post #628 by zzzach

Mine arrived today, in the US via USPS. Ordered January 18.
Charging and looking for a guide now.

It’s a shame it just arrived in a padded envelope… I would’ve been super curious to see official TE packaging for such a tiny device. Not even sure if they got that far along before bailing out?
---

## Post #629 by PedalsandChill

Check the top post of this thread for manual and other docs/photos that have been gathered.
---

## Post #630 by instantjuggler

The friends and family release had packaging. You can see a photo of the packaging in the background of that big Jesper interview last year that's on YouTube… It was a silver cardboard box.
---

## Post #631 by vanveluwen

@TimK \~ word on the street is there is a discord for contributing work on this. I’ve done a decent amount of hardware stuff and have a few SWD programmers about. My stem player showed up this week so let me know where we are coordinating :slight_smile:
---

## Post #632 by TimK

I have a very similar experience. My package has not moved since January 23 when UPS last scanned it in Ontario, CA. (I’m in Belgium.) I had already paid my customs/taxes to UPS.
I did hear from others in Europe being able to receive their shipment, but this could be just a matter of being lucky with customs?

I’ve been on the phone with UPS multiple times, they can’t do anything. New Republic’s customer service person has been politely answering my emails, but also says she can’t access their own UPS account, so doesn’t seem to actually do anything. It’s a bit infuriating because nobody can help or even say what is going wrong. If you’re not in the US, I would not recommend ordering from them. ☹️
Meanwhile, all my DigiKey orders shipped via UPS just keep arriving as usual.

[quote="pdrgf, post:627, topic:66795"]
would a basic nRF52840 board do the job for getting a grip on things before putting down more money? I’ve only ever worked with arduino and dirt cheap esp8266/esp32 boards in the past.

[/quote]

Yes and no. You can’t program an nRF52840 without either loading a bootloader first or using a programmer (J-Link). The nRF52840-DK has a J-Link programmer *and* an nRF52840 on board! It is also well documented and fully supported by Nordic. If you want to develop for the stem player hardware, I don’t see the point in getting other dev boards. (For other projects, yes nRF52840 is a great chip you can do cool stuff with, any decent dev board will do!)
(If you’re an experienced embedded developer, this doesn’t apply because you could turn any MCU into your own SWD programmer, but then again you probably own a J-link already  ¯\\\_(ツ)\_/¯ )

[quote="pdrgf, post:627, topic:66795"]
Looks like the entirety of TE’s recent suite of products use chips from the same range; I’m guessing that’s something to do with the communication capabilities, makes me wonder if this device was ever meant to be able to wirelessly communicate with TP-7/TX-6/etc…

[/quote]

Not at all, the stem player only has bluetooth audio through a separate BT module, the BLE stack of the nRF52840 is not used or even accessible at all. I haven’t heard of any other TE products using nRF52840 except for the Choir puppets: https://github.com/jetztgradnet/Choirama

@vanveluwen I will DM you, I have some stuff to get you started!
---

## Post #633 by Stoff9

@TimK If you’re not able to get another device, hit me up, I’ll send you a device from germany if shipping is not that much.

Thanks again for your work.

Did anyone tried this? [free-stem-separation-and-automatic-loop-export-app-for-op-xy-op1-field](https://op-forums.com/t/free-stem-separation-and-automatic-loop-export-app-for-op-xy-op1-field/31343)
---

## Post #634 by pdrgf

Looks like they’ve relisted them at $92 =/
New URL is [https://shopnewrepublic.com/products/yzy-2019-og-unreleased-stem-player-by-teenage-engineering-by-phantom-marketplace](https://shopnewrepublic.com/products/yzy-2019-og-unreleased-stem-player-by-teenage-engineering-by-phantom-marketplace)

edit: 2026-02-14 they’ve changed it again: [https://shopnewrepublic.com/products/yzy-2019-og-unreleased-stem-player-by-teenage-engineering-1](https://shopnewrepublic.com/products/yzy-2019-og-unreleased-stem-player-by-teenage-engineering-1)

---

Cool, will find an nRF52840-DK! kinda keen to try out some of the communication options it offers (did I read I can get them to fairly reliable detect the distance between each other? that could have tons of fun possibilities), plus it looks like it’d hold it’s value okay if I wind up not using it :crossed_fingers: Unlike random microcontrollers off aliexpress (which I’ve already ordered…)

From a very brief look the ESP32_nRF52_SWD process doesn’t look too difficult but probably not worth the risk on the TE board!
---

## Post #635 by br4n_d0n

Wasn’t the price $35 before? Seems like they got greedy :expressionless_face:
---

## Post #636 by pdrgf

yeah, it’d be interesting to know what was the last order date to successfully deliver. Mine was January 25th, looks like people ordered way earlier than that and haven’t gotten them yet though?
---

## Post #637 by corpusjonsey

I ordered on 1/21 and got it yesterday (2/12). There was a lot of radio silence in the beginning, then it went from CA to OH then to CO to me.
---

## Post #638 by zzzach

I’m having a blast slowing down and mangling the onboard tracks. It’s like a game to see how unrecognizable I can get the loops to be.

https://on.soundcloud.com/YYh5y21dnN94JbW7Pd

The more I interact with it, the more I wish for a field recorder (like a TP-7, sans wheel) or even just an MP3 player from TE in this form factor. I’m still blown away by the inclusion of bluetooth audio here given its absence in TE’s flagship products.
---

## Post #639 by Nick_Martin

Mine was shipped 21st of Jan. Haven’t gotten an update since the 26th of Jan. Just says delayed New republic have refunded me now. No idea what has happened to the units
---

## Post #640 by lzma

I received my unit the other day and I’ve been having a blast with it. For the price I paid I’m totally satisfied with it, even just to look at it.

Reading through this thread has been pretty amazing, so many people from different disciplines working on cracking this device. My background is in cybersecurity, particularly with firmware and software reverse engineering, I was wondering if there’s someone currently working on those efforts? If I can help out in any way, let me know. I’d want to focus on reverse engineering the unknown protocol the device uses over serial. The info we need should be in the firmware, if the device is designed to accept commands over serial.

Hardware hacking is not my forte and the glitching attack to bypass APPROTECT was incredible. I might try and reproduce that with my unit so I can have a dump to begin reverse engineering.
---

## Post #641 by pdrgf

did any of the people who ordered before me (Jan 25th) buy *just* a stem player? I was pretty tempted by some of the unbranded clothes (Balenciaga quality clothing for $20?!) but figured that massively increased the odds of some issues at customs

Feels like base level professionalism to ship to people in order of when they actually buy stuff. Although I guess liquidators mightn’t be the most professional of groups.

@lzma keen to hear how getting a dump goes if you do attempt it! I’d imagine people being able to easily restore the stock firmware alone would massively help with people working on it. (this is assuming a dump can be relatively easily reflashed)
---

## Post #642 by instantjuggler

this sounds great! i also use mine a lot for just random sample fodder. super easy and fun to dial in something that doesn’t sound at all like the source material…
---

## Post #643 by beto

Mine was ordered Jan 18th, shipped on the 26th, arrived yesterday (Feb 12th). I’m in Florida.
---

## Post #644 by zzzach

Your deep dive on YouTube showed me the way, so thanks!
---

## Post #645 by instantjuggler

oh that’s so awesome that it helped out! i don’t know what it is about the design of this design, i find the button combos fiddly, sometimes actually physically difficult to perform with my big hands… but regardless i find the workflow super fun and somehow intuitive in a way that i don’t with other devices that have equally random architecture. like the tp-7, i can’t wrap my head around it. but the TE stem player speaks to me somehow!
---

## Post #646 by TimK

[quote="pdrgf, post:641, topic:66795"]
@lzma keen to hear how getting a dump goes if you do attempt it! I’d imagine people being able to easily restore the stock firmware alone would massively help with people working on it. (this is assuming a dump can be relatively easily reflashed)

[/quote]

A clean dump should be easy to flash. Once APPROTECT is unset, flashing any binary is a piece of cake. Setting APPROTECT again after flashing is very easy too.

As far as I know, nobody has been able to get an uncorrupted dump by doing the Limited Results voltage glitch on the original stem player PCB. I have done the voltage glitch successfully many, many times on other hardware, but was never successful on the actual stem player PCB.

I don’t think anyone has tried to voltage glitch with a transplanted chip yet, meaning putting it on eg. the nRF52840 dongle, which was super easy to glitch in my experience. If you do want to get to the original firmware, I suggest going this route. Transplanting will not be easy because this chip is semi BGA. (I didn’t have the balls to do it with mine 🫣)

And finally yes, being able to restore the original firmware would be a huge deal!

[quote="lzma, post:640, topic:66795"]
Hardware hacking is not my forte and the glitching attack to bypass APPROTECT was incredible. I might try and reproduce that with my unit so I can have a dump to begin reverse engineering.

[/quote]

If you’re successful I would not mind a copy of that binary. I could verify it on my dev unit in 2 seconds. 😎
---

## Post #647 by lzma

Thanks for the info! My soldering skills are not too shabby but I doubt I’d be able to pull off that transplant. I have been working with a research lab which has some very expensive machines that help to automate this process though with other chips for embedded devices. Next time I head over there I’ll try and see what their hardware is capable of.

As far as the existing dumps go, how corrupt are they? Do we know what’s intact and what is not?
---

## Post #648 by pdrgf

if we had a dump of the firmware, how likely is it that one could:
- flash custom firmware which has the ability to read/write to the eMMC
- replace/supplement the existing tracks with new music in the same format
- re-flash the original dump
- get the full TE firmware experience with custom tracks

I can think of ways they'd ensure this wouldn't work. I modded a game a while ago called Dropmix (very basic but worth a look for anyone interested in stems btw) that done some hash verifications on boot for each audio file (fortunately the solution there was to just keep it offline)
---

## Post #650 by TimK

[quote="pdrgf, post:648, topic:66795"]
if we had a dump of the firmware, how likely is it that one could:

* flash custom firmware which has the ability to read/write to the eMMC

[/quote]

No dump needed for this, I’ve done this a long time ago, see above in this very thread.

[quote="pdrgf, post:648, topic:66795"]
* replace/supplement the existing tracks with new music in the same format

[/quote]

Same, this has been done. I did decide to change the audio format a little to make it more intuitive.

[quote="pdrgf, post:648, topic:66795"]
* re-flash the original dump

[/quote]

If there’s a clean dump, this should be trivial. I can’t see why the original binary + UICR couldn’t be loaded onto a device again and restored the original firmware.

[quote="pdrgf, post:648, topic:66795"]
* get the full TE firmware experience with custom tracks

[/quote]

That’s a much bigger question! That means either recreating the full TE experience plus adding stem upload (which I’m doing) or modifying a clean dump so it can upload new data. (We have seen in an old corrupted fw dump that the song titles are present in the firmware, so this could be tricky.)

I have fully reverse engineered their data format (except for MIDI, PO sync data, still working on that), so I know exactly how audio data should be written to work with their firmware. I have already written my own audio data to memory and played, as well as played their audio data on my own firmware and extracted all original, individual stems as WAV files.This is not news, I have written about this before in this thread. I haven’t played my own audio data on their fw because we don’t have a clean dump (yet?)

[quote="lzma, post:647, topic:66795"]
Thanks for the info! My soldering skills are not too shabby but I doubt I’d be able to pull off that transplant. I have been working with a research lab which has some very expensive machines that help to automate this process though with other chips for embedded devices. Next time I head over there I’ll try and see what their hardware is capable of.

[/quote]

That would be so cool! I honestly don’t think the transplant should be that crazy if you have the right tools. The only reason I haven’t done it is because I don’t have the spares to be able to fail. (And maybe I lost interest in the whole glitching a bit when I started working on my FW)
---

## Post #651 by chapelierfou

[quote="TimK, post:650, topic:66795"]
The only reason I haven’t done it is because I don’t have the spares to be able to fail.

[/quote]

I’d be happy to financially help if you need to order some units.
---

## Post #652 by Stoff9

As i said, hook me up with your adress and Ill send you one for free to belgium.
---

## Post #653 by TimK

@Stoff9 @chapelierfou  (and others)

I’m so impressed with the generosity and helpfulness I keep seeing in this thread. Maybe I’m disconnected but that seems rare online. ❤️

I will try to not disappoint with the work I’m doing on this!
---

## Post #654 by chapelierfou

[quote="TimK, post:653, topic:66795"]
Maybe I’m disconnected but that seems rare online. :heart:

[/quote]

This place is not like the rest of the internet !
---

## Post #655 by BenjyBuilds

Alright, might buy one, but £68 seems a little pricey, do you think i should wait out a better price or do you think that’s fair
---

## Post #656 by glia

please don’t take this the wrong way but only you can decide that, honestly

and as stated a few times in messages above…your “worth it” valuation should be based on current state and functions of TE SP not what it *might* eventually become
---

## Post #657 by Rubyodingus

Its a gamble, but if you like it for what it currently is, go for it. Next week they could have the price back down, but I think the increase came from their issues with fulfilling this last batch, and the newer interest in it. Hindsight is 20/20

The new supply circulating will find its way on ebay, and unless the firmware hack is released soon, someone may sell it for cheaper.
---

## Post #658 by ItDitry

Hey everyone, I’ve been snooping around this thread for a while and most recently I’ve came around to actually owning one of these fellas and opening it up, due to needing to replace the LiPo battery. Since this is technically an upgrade, this had me wondering if future hardware updates would be possible, specifically to the storage of the device since the flash storage is roughly 2gbs. I know that so far firmware has yet to be \*fully understood but it’s still fun to think about the future of this little neat machine, food for thought :\]

![IMG_4158|602x500](upload://jByBEd5KdTvTtpjlsaQDPNRBZwO.webp)
---

## Post #659 by TimK

SP01 has 4GB of internal FLASH memory, but JIK only uses 1.9GB of that, the rest is zeroed out. 

There’s no space in the enclosure to fit anything like this (unless you remove the battery or other stuff) but more importantly, no interface to connect to. I very much doubt it would be possible to add any hardware mods in this enclosure. 

Also the system is too slow to add lots of memory. 2GB transfer is roughly an hour of waiting. It's pointless to add more memory unless the system is completely redesigned.
---

## Post #660 by gg11704

Looks like it’s back down to $35 USD at New Republic, fyi!
---

## Post #662 by TimK

@chrispmonkey i’ll DM you to get you started!

For my own testing purposes, I made an app that can upload new songs to the stem player over USB (currently at a sad 280 kBps), but it’s very rough: written in C++, compiled for my Mac and not very portable. Just a quick proof of concept/ dev tool.

I think i wrote about this here before, but i think a web app using eg. web serial could be a good idea. (I’ve never used web serial for data transfer so I don’t know the pitfalls.) I heard that @tunelight has already started working on something.

I would love some help on the software side so i can focus on finishing the alpha release of the firmware. (I’m almost there)

I’ve been getting a lot of requests about development. Maybe it is time to setup our own development discord for the new software and firmware?

💾👷🏻‍♂️
---

## Post #663 by dotJustin

Hey Tim, brand new to the forum but I’ve really enjoyed reading through this thread. I have one SP on hand, and another on the way, and would love to help out with this effort in any way that I can. I think a dedicated discord would be great, as well as a Github Org. Would be happy to set either/both of those up as well, if you’re interested!
---

## Post #664 by jadam

I’d also be interested in a separate discord. I can help out with the sample tool / web app and firmware beta testing!
---

## Post #665 by salahmason

I grabbed a few of them from the New Republic sale. Happy to donate one if an extra piece of hardware is useful to this effort!
---

## Post #666 by dk3k

im in no way shape or form able to code but i have been following this thread for a few months now and the development is exciting! if a discord is created, i would highly appreciate to be added. seeing progress and everyone working together is awesome.
---

## Post #667 by KHAGE

Same here… I know absolutely nothing about programming, but I love you mates !

EDIT :

[quote="KHAGE, post:544, topic:66795, full:true"]
Not sure… I ordered two more units two days ago (Monday, January the 19th) and I already have an UPS tracking #…

It’s my third order with New Republic. No problem so far.
[/quote]

UPS has finally admitted that the package is lost.
I need to contact New Republic for a refund.
---

## Post #668 by TimK

As I’m working towards an *alpha* (still far from beta!) release of the new firmware, I would like to ask you all what you think is important in a new stem player firmware, so I created this poll:

https://forms.gle/SohTv9rCu52SWL287

I’m also interested in what everybody’s technical skills are and who wants to help out.

The features I work on before the first alpha release will depend on the results of this poll.
---

## Post #669 by _pete

I gasped aloud when I saw this! Scrambling to fill it out now!
---

## Post #670 by instantjuggler

so amazing that the poll leaves room for any original developers to chime in!
---

## Post #671 by jaseknighter

I was just thinking…perhaps folks who want to contribute but forgot to mail their application to engineering school on time when they were young would like to work on a zine or comic book telling the story of this hurcalian effort.
---

## Post #672 by TimK

one can hope  ¯\\\_(ツ)\_/¯

@jaseknighter yeah the lore of this thing is worth telling too!
---

## Post #673 by pdrgf

Tried to order a couple more from the website but it's not allowing my address, have they blocked international shipments in some crude manner that isn't apparent until you try to pay?

RE: web serial, assuming there isn't anything especially bespoke in the C++ code it should be easy enough to adapt to Rust for a more portable WASM-friendly solution?
---

## Post #674 by robbie

[quote="pdrgf, post:673, topic:66795"]
it’s not allowing my address
[/quote]

Yes, they’ve not been accepting non-US addresses since the players were relisted, probably related to the shipping problems detailed previously in the topic.
---

## Post #675 by br4n_d0n

This is an embedded device and I question the overhead that Rust would add to it for no real gain
---

## Post #676 by tunelight

I am no expert by any means but the web app would simply allow to convert WAV and pass it on to the connected SP device, it would not to be installed on the device as such?

I am looking forward to help where I can : )
---

## Post #677 by pdrgf

Do you mean for the firmware on the device itself? I'm not super deep into how well it works on embedded systems (I thought quite well asides from lacking libraries?) but I meant in terms of working towards a desktop or web tool that transfers files onto the device; it'd potentially work around the issues of portability for the C++ based solution and possibly be a bit easier to adapt onto a web based solution later without immediately going down those lines?
Sorry, that was meant to be in response to Tim [here](https://llllllll.co/t/te-stem-player/66795/662?u=pdrgf)
---

## Post #678 by br4n_d0n

@pdgrf I thought you were referring to the firmware being written in Rust, which I didn’t see a real benefit to, but if it’s for something else then it really doesn’t matter.
---

## Post #679 by ancaja123

[quote="TimK, post:668, topic:66795"]
https://docs.google.com/forms/d/e/1FAIpQLSe6DpW9aS3fgIlX3hDT6XiZyl1zNuhw4gxzdVflPcSxcgM5Hg/viewform?usp=send_form

[/quote]

Off topic, and just food for thought, but I really wonder if the OB-4 Karma Loops are loaded on in the same fashion as the SP. I wouldn’t be surprised, as they’re baked in and most likely developed around the same time (similar features too, obviously)
---

## Post #680 by Hugene98

Hey everyone, I’m new to the forum and really excited about the amazing work you’re doing. I can’t wait to upload new songs to the firmware and enjoy this device even more! :drooling_face: 

![WhatsApp Image 2026-02-19 at 17.04.23|375x500](upload://6CpZqlNUXFYUsoBRkUr7f0GgP8d.jpeg)
---

## Post #682 by UnderTak3r

Welcome!
Quick question, that case looks awesome, where did you get it?
---

## Post #683 by dotJustin

Looks like this one off cgtrader: https://www.cgtrader.com/3d-print-models/hobby-diy/electronics/te-stemplayer-case
---

## Post #684 by ItDitry

I could absolutely remake that case, just need some dimensions..
---

## Post #685 by baris

Hey, first post here! I got a Stem Player, but unfortunately the play button is not working… Is this a known issue that has any fix? Or should I just return it?
---

## Post #686 by instantjuggler

i would return it. some of the units have been reported to arrive with various defects. the play button not working is not a common thing i have heard about, nor a fix for that.
---

## Post #687 by ItDitry

unrelated to software, but I finally got a replacement battery from DigiKey to replace the original one which wouldn’t charge, and it works perfectly now with roughly 66% more battery life. One small defect addressed! 

![IMG_4288|666x500](upload://uNTdGcZCF2hkGdK7Ao4TqDJCeDO.jpeg)
---

## Post #688 by nuk

any advice for taking off the side panel without destroying the flex pcb?
---

## Post #689 by ItDitry

Using heat will be your best friend, there’s a lot of adhesive on the inside that’ll cause the trouble. Don’t try bending the plastic cause it can cause some permanent damage (you could probably flex it back with heat but I’ve already done enough)
---

## Post #690 by TimK

How did you manage to close it back up and somehow attach the tiny header to your battery?? Very impressive!

[quote="nuk, post:688, topic:66795"]
any advice for taking off the side panel without destroying the flex pcb?

[/quote]

Do not touch the flex pcb: no tools near it!

The plastic side piece is only glued in one spot, on the other side of the device:

![IMG_1656|450x500](upload://j1LpKePHx3qLnxLWgavpIrENjjB.jpeg)

Other news: I heard back from New Republic. They stopped shipping them internationally because there are too many problems!
---

## Post #691 by KHAGE

[quote="TimK, post:690, topic:66795"]
Other news: I heard back from New Republic. They stopped shipping them internationally because there are too many problems!
[/quote]

Yep. Since UPS lost the two Stem Players I ordered in mid-January, I have just received (finally) confirmation of a refund from New Republic. 

Too bad. :-/
---

## Post #692 by DGoods

Same here, UPS lost my order too and i got a refund.

But now its not possible to order from Europe anymore :(
---

## Post #693 by KHAGE

I already have two, so I’m good… but, yeah, that’s a pity.
---

## Post #695 by TimK

Hi everyone, first of all, thanks to all of you who answered the questions in the poll! We’ve received 120 responses so far!!

I’m trying to read everything and will take your requests into account for where my firmware development will go. There have already been some amazingly creative and funny answers in the request for new features. Thank you all so much for taking the time, I'‘m having a blast going through them!

Some quick stats:
- Almost everyone (97%) wants to load new stems, almost everyone wants audio (70%) and tape speed (81%) effects. 76% want to get rid of Kanye on this 🫣
- About 50% of people are interested in some form of development (that is great!)
- 12 people have claimed to be Kanye (He does have big personality, so I hear)
- NOBODY from TE responded!! (unbelievable! 😭🙃)

I’ve been quite busy again, but there has been some major progress. I’m also setting up a discord and a GitHub repo. They will go live I hope in the next week as I find the time and I will start sending invites when they’re ready! Please bear with me, I will keep you guys posted.

![icon-256|256x256](upload://v1tp78jNLf61PTCbrhRWMXFr2H9.jpeg)

More good stuff very soon!

💾👷🏻‍♂️
---

## Post #696 by TSG

![Stem Player](upload://AeuiuXc2zDwuLgAWlWEyF6X90VA.jpeg)
Got mine yesterday to UK- I ordered it in January but sent it via a relative in california. Even the domestic shipping took about 3 weeks.
It is actually a lot more fun to use than I expected, been quite enjoying looping the Bruce Haack sample and pretending the 'other' artist connection never existed.
---

## Post #697 by PedalsandChill

Since so many people have recently bought a player, I thought I would provide a .txt link for the manual. There’s one at the beginning of this thread, but it’s a word doc and not everyone has that.

[TE Stem User Guide.txt|attachment](upload://gCeD5M94r9yrSdsDiHUjhAwM2Uz.txt) (3.3 KB)

\*side note: if yours is dead on arrival. make sure to use a non-rapid charger for it. the wattage is too high and it won’t take.
---

## Post #698 by Kelly_Green

Does anyone know when the New Republic will have another sale? I see that the Stem player is currently being sold on the NEW REPUBLIC site for $45. Should I wait, or is this the lowest price it will be at? Any help is appreciated!
---

## Post #699 by PaulFe

Per their website, $45 is a sale price of the typical $120.
---

## Post #700 by dotJustin

I looked into new republic’s actual price history via the wayback machine when I bought my first SP. If I recall correctly, $35 is the lowest it has been since the listing has been up, and it does occasionally change price. Personally, I would buy now as there's a decent chance that they'll up the price randomly.

I bought my first one at $35, and once I received it and confirmed it was legit, my friends went to buy some and it had gone up in price, then a few days later it came back down, so it's really unpredictable.
---

## Post #701 by robbie

Is the Stem Player even available on New Republic at the moment? It's shown in their listings, but when clicked the site returns a 404 page. Or are they doing some clumsy geoblocking?
---

## Post #702 by PedalsandChill

It's only available to US buyers at the moment.
---

## Post #703 by calm.cat

You don’t need to buy this, the hardware looks nice though. Get a 20 year old thinkpad and it will be able to do much more interesting software things.

Sorry to be old man yells at cloud or whatever, but we can program computers (old ones too, especially so maybe?) to do things like this…
---

## Post #704 by robbie

[quote="PedalsandChill, post:702, topic:66795"]
only available to US buyers
[/quote]

Yes, I mentioned that up thread, but it *was* still available to order. Now the page doesn’t show at all (for me), and I’m wondering if they have been unlisted completely again. Is anyone currently able to add it to their cart?

[quote="calm.cat, post:703, topic:66795"]
20 year old thinkpad
[/quote]

I haven’t yet found a Thinkpad that quite fits in my pocket.
---

## Post #705 by PedalsandChill

Sorry, bud. I missed your comment mentioning that. Weird that it’s not on the page. I just bought another a couple days ago.

::edit:: 

shows up [here](https://shopnewrepublic.com/products/yzy-2019-og-unreleased-stem-player-by-teenage-engineering?srsltid=AfmBOooNTV_-RwptwCD_5EyWukOk6b8wZr_HrTKu2uL-e9RXlIz8SR3r) for me. Does it show for anyone else?
---

## Post #706 by instantjuggler

i’m in sweden and get a 404 code over here when i follow that link…
---

## Post #707 by pdrgf

RE international orders, I imagine it's still possible to use proxy delivery services? Given how much new republic's international postage costs were it surely can't be that much more expensive.

I used some to order stuff from Japan in the past but have no knowledge of which US ones would be good so if someone has some suggestions it could be useful to someone here. The one major caveat I would add is that it would probably need to be one that has previously sent battery powered devices for you as I imagine the lithium batteries could be a bit of an issue with some services.
---

## Post #708 by nuk

i couldn’t order from New Republic so i just ordered on stockx and both my units came within 5 days to EU, more expensive but at least no problems
---

## Post #709 by TimK

Hi all, I’m back with a “small” update.

The discord (that may or may not exist) woke up again recently and worked their magic… they cracked the TE bootloader and created an online update utility that can update the firmware of the stem player without ever opening up the device!

This means that all I wrote here about needing to open up the device to first load a bootloader is no longer true. I’ll also have to change my firmware a bit because I now have to play by the rules of the TE bootloader, but that’s not going to be a problem.

Here you go:

https://solderless.engineering

Big thanks to the work of @JoseJX , @fishdog and SKFU! (I helped a tiny bit too)
And thanks to @B_E_N for finding the button combo that started it all!

Meanwhile, I’ve also been slowly working on getting the SP-1 developer repo ready on GitHub so anyone will be able to create their own firmware or software for this thing. More on that soon!

Everyone, have a great start of your weekend!

💾👷🏻‍♂️

*PS: small warning: if you use the updater to load a bad binary you WILL brick your stem player!*
---

## Post #710 by taylorsizem_re

The link worked for me. Seeing the enthusiasm here along with the developments above, combined with the $45 sale price was too much to pass up.
---

## Post #711 by Justmat

excitement overload! guess i should make sure mine still charges and powers up lol
---

## Post #712 by Virtual_Flannel

Wow this is incredible. Are there currently any firmwares to test out? Its wild this exists before an actual firmware is available to test. Either way cant wait to give it a try!
---

## Post #713 by kasselvania

uhhh. yeah, I’m following @Justmat and whipping out a TE USB-C cable just to be cheeky.
---

## Post #714 by TimK

I’ve tested the updater a few times with different fw builds. But I have nothing to release yet because my firmwares don’t do reset to bootloader yet. With a device that has a battery that means it would be stuck in that fw. (No going back to bootloader)

I’m working on documenting the requirements for compiling for the bootloader so anyone could build any kind of crazy firmware they can think of. I still want to release a public fw that can do the basic stuff the original does + stem uploads. It’s a lot of work, but I’m getting there!
---

## Post #715 by dotJustin

So excited!! Will we be getting a public discord server?
---

## Post #716 by TimK

Yes! But again… I have so much to do and a kid that wants to play ALL THE TIME. 

So I still need to finish setting it up.

I hope that once the discord is up and running and I've documented the stem upload protocol, some of you can take over a little bit on the software side!
---

## Post #717 by dotJustin

I understand, lots to do! I've set up multiple servers in the past, and even created one that ended up with several thousand members. That being said, if you need help with channel/role/general setup, I'd be happy to lend a hand.
---

## Post #718 by pdrgf

Wow this is all amazing to hear! Hoping there's an article somewhere about the process of discovering the button combo and figuring out how to communicate with the device to allow flashing.


One thought I've had recently is whether the midi sync port could be used for i2c modules? They'd need an external power source but assuming they're extremely low power that mightn't be an issue; it'd give the device a (very tiny) amount of extensibility that could go a long way.
---

## Post #719 by louwrens

Not sure if I am being slow but why would there be a utility to update the firmware if no firmware is available from whoever made the utility in the first place. Or is this something only shared on the discord that does or doesn’t exist? Or is the thought currently that it is for people who can write their own firmware?
---

## Post #720 by instantjuggler

you can write your own firmware, or if someone else releases a firmware you want to upload… well now you can!
---

## Post #721 by TimK

I have been working on new firmware, and I know others are at least interested to do the same. A bunch of people here are interested in having firmware that allows them to upload new stems. (98% of the people who answered the poll)

This update utility is a huge step in that direction because with it people who aren’t developers, who have no access to a debugger, or simply don’t want to open up their devices will be able to load a new firmware when it’s done.

I’ve been telling everyone here that this wouldn’t be possible because we didn’t know about the bootloader that was already present on the device. Now we know about it AND we’re able to use the bootloader! (If there had been a new firmware release first, I’m afraid people who didn’t want to would’ve been opening up their devices to flash my bootloader!)

At this point I think there’s no question we will be able to remove Kanye and play other stems (I have done this already) it’s just a matter of when. I’m racing towards a build that is ready for a public release, but I wouldn’t mind if someone else beats me to it! That’s why I’m setting up a GitHub repo and a discord: so other people could work towards the same goal.
---

## Post #722 by Shepherd_Howe

Well I guess I got mine in just in time!
---

## Post #723 by zzzach

Probably a stupid question, but would midi out (beyond clock sync) be possible through firmware?

In my dreams I want to use the buttons and faders to mute and mix tracks on the TP-7 😜.
---

## Post #725 by _pete

20 chars of this is super rad. Can’t wait for the alt firmwares to release!
---

## Post #726 by crucFX

20 char of fantastic work!
---

## Post #727 by vehka

So cool that there was a button combo after all. I wonder how many hours of collective puzzle-solving it took to find it… Great work everybody who’s involved in reverse engineering this and developing new stuff for it!
---

## Post #728 by TimK

I need to read up on the MIDI protocol, but if we can send a MIDI clock out, I would assume the rest of the protocol is possible too. I haven’t really looked at the MIDI at all, does anyone here have a good reference to get into the protocol and the electrical spec?

MIDI is quite low on my todo list, but I like this idea and I’ll have to dive into MIDI a bit more soon!
---

## Post #729 by br4n_d0n

https://midi.org/specs
---

## Post #730 by TimK

Thank you! Sorry I was too lazy to do this myself 🫣

For anyone who’s interested too:
- Tip ring sleeve connector for MIDI spec: https://midi.org/specification-for-use-of-trs-connectors-with-midi-devices
- Electrical standard: MMA/AMEI CA-033: https://mitxela.com/other/ca33.pdf
- MIDI 1.0 spec: https://midi.org/midi-1-0-detailed-specification

I just scoped signal while in MIDI mode. I should see 31250 BAUD data but I see no packets at all! (only a 128us pulse in sync with the music) Could it be this device doesn’t send out actual MIDI at all but rather a simple pulse? Has anyone else looked at the signal or used the MIDI mode?
---

## Post #731 by marcus_fischer

Yeah I thought it was to sync PO units. 

I don’t remember seeing anything about midi sync
---

## Post #732 by TimK

According to the PDF manual I’ve seen here mode 3 is PO sync and mode 4 has MIDI clock output.
---

## Post #733 by tomw

I’m getting midi clock out here - switch to mode 4 and I see it while playing

![image|343x500](upload://dE9pIrcsWcPSYRCiMWeb3BJI3Va.png)
---

## Post #734 by PedalsandChill

Midi-wise, the clock is all I've ever been able to detect from it as well.
---

## Post #735 by instantjuggler

to help celebrate the public release of the usb hack, and the website interface launch, i dug up a couple of old pictures of the original packaging from the friends and family release back in the day-

![Screenshot 2026-03-08 at 6.01.46 PM|690x459](upload://k4EfwdcG5kvV98aPVVvj1IiTw9B.jpeg)

![images|236x213](upload://d8FUNFqIHmqzgj1o7KodZY1S9aX.jpeg)

![Screenshot 2026-03-08 at 6.11.17 PM|690x472](upload://i7pL5URrW4K0EHv6S7CERMNQHKE.jpeg)
---

## Post #736 by TimK

Thanks @tomw! I only see a 128us pulse while in mode 4. That’s not even long enough to send 10bit at 31250 BAUD, so I don’t know what’s going on. 

@instantjuggler that last photo! Another prototype??
---

## Post #737 by instantjuggler

i think its just the model we have with different packaging? the photo is taken from a slide show background at a TE lecture… the slideshow had different renders of the stem player. i did at one point see a photo of someone who had the device in a physical silver box such as pictured above. but i couldn’t find that photo again now. its been years ago though that i saw it. at the time i didn’t know what it was exactly. if i had to guess, the silver box was somehow internal TE packaging, and then you can see the final friends and family packaging that was sent out. which, by the way, is the same packaging style/format that kano ended up using on the commercial release of their player…
---

## Post #738 by TimK

Your last photo shows 2 screws and the play icon, so that’s the front of the device. I don’t think I’ve seen that before?
---

## Post #739 by Galapagoose

re: MIDI clock. just checked on hardware, and a midi clock message is just a 128uS pulse! i tried to reason it out but failed, but it kind of makes sense because midi clock is `0xf8` which is just `11111000` in binary.

so theoretically, the pin is hooked up to a UART that is sending `0xf8` but it could also just be a GPIO with a fixed pulse width. generic midi out should be possible if the pin hooked up to the port is able to be a uart TX pin!
---

## Post #740 by br4n_d0n

Damn, I need to get you guys on the TX-6
---

## Post #741 by TimK

That makes sense! I feel so stupid now! 4 bit (includes the start bit) at 31250 BAUD is exactly 128us. case closed! Thanks @Galapagoose !

And yes, on nRF52840, this pin can be UART at 31250 BAUD, so real MIDI is still on the table.

I guess I might share it here already even though it’s still WIP. This is where I’m keeping the SP-1 developer documentation:

[https://github.com/timknapen/SP-1-dev](https://github.com/timknapen/SP-1-dev)

The full MCU pinout is there too.
---

## Post #742 by nonkillwingman

![tesp clip|577x500](upload://5CEQAfHvLRxC01YsZevMno1AueH.jpeg)

Hi, I’ve made this simple clip for the player, using 6.3cm *x 4.5cm x* 1cm as stem player’ dimensions. it might be slightly tight as the file was designed without margin. feel free to try it out.

[the thingiverse page of this clipper](https://www.thingiverse.com/thing:7312090)

UPDATE: now it should be good to go with some margin left
---

## Post #743 by maybe

Tried to summarize the whole (very confusing) timeline w public info and stuff from this thread

### Pre 2019

From what I’ve been able to gather, TE have never publicly commented on the device, and information about the product’s development is non-existent. TE’s founder Jesper has once mentioned that he met Kanye on his farm while they were touring the US. This might’ve been during 2018 but it is hard to confirm, the stem player collaboration could’ve started somewhere around here. https://scandinavianmind.com/human-touch-interview-jesper-kouthoofd-teenage-engineering/

### January 2019

Kanye and Alex Klein meet at CES. Development of the *Kano* stem player starts soon thereafter. https://www.complex.com/music/a/shawn-setaro/kanye-west-jesus-is-king-tech-ceo-writer-kano-alex-klein

### Early to fall 2019(?)

Allegedly early Kano prototypes. Source claims that these are from *before* TE got involved, I find this hard to believe. Some of these look similar to the TE version but without the Dieter Rams-esque design aesthetic.  https://www.instagram.com/p/CkZv4unMZMV/?img_index=1

### 23 October 2019

At a listening party for *Jesus is King* at The Forum, Kanye switches between a laptop and the stem player. The stem player briefly plays unreleased tracks as he skips through to find the right song in a similar fashion to when the device was previewed to BigBoy. This could suggest that it was flashed with an early album version in a similar manner to those that have made it onto the market. https://www.tiktok.com/@momosett/video/6992434771234229509?embed_source=121374463%2C121468991%2C121439635%2C121433650%2C121404358%2C121497414%2C121477481%2C121351166%2C121487028%2C73347567%2C121331973%2C120811592%2C120810756%2C121503376%3Bnull%3Bembed_pause_share&refer=embed&referer_url=cdn.embedly.com%2Fwidgets%2Fmedia.html%3Fsrc%3Dhttps%253A%252F%252Fwww.tiktok.com%252Fembed%252Fv2%252F6992434771234229509%26display_name%3Dtiktok%26url%3Dhttps%253A%252F%252Fwww.tiktok.com%252F%2540momosett%252Fvideo%252F6992434771234229509%26image%3Dhttps%253A%252F%252Fp16-sign-va.tiktokcdn.com%252Fobj%252Ftos-maliva-p-0068%252F208fa395b4454529b74e31fa9e604e39_1628053093%253Flk3s%253Db59d6b55%2526x-expires%253D1728338400%2526x-signature%253DngKLyF9sDVciyX4xBO629M29jlg%25253D%2526shp%253Db59d6b55%2526shcp%253D-%26key%3D2aa3c4d5f3de4f5b9120b660ad850dc9%26type%3Dtext%252Fhtml%26schema%3Dtiktok&referer_video_id=6992434771234229509

### 25 October 2019

In an Apple Music interview with Zane Lowe, Kanye references the TE stem player: “This portable stem player that we designed with Teenage Engineering for this album and the albums before it… is to spread the gospel.” On the same day, an interview with BigBoy is released, including footage of Kanye previewing the stem player and briefly playing an unreleased track before skipping to the correct track. *Jesus is King*, the album loaded onto the TE device, is released.

### 12 December 2019

The “Teenage Engineering KE Guide” dated 2019.12.12 is labeled as ‘REV.02’. Notably, the top left includes references to “stem player prototype 2”, “christmas edition”, and “REV01”. Furthermore, it suggests that internal name Teenage Engineering had for the device was the KE

### 17 September 2020

A shipment record shows that Very Good Touring (a Kanye associated company) imported 859 units of the “Stemplayer speaker with battery” to West Brands LLC in Calabasas, CA. Model number is specified as TE023AS001, indicating that it is the Teenage Engineering version. https://www.importgenius.com/importers/very-good-touring

### 23 March 2021

In a now deleted tweet, a known Kanye collaborator tweets about the TE stem player. https://www.reddit.com/r/WestSubEver/comments/mbikj9/cons_tweet_about_stem_player/?share_id=r1svR-DwFrCx-SeM_Hkuh&utm_medium=ios_app&utm_name=ioscss&utm_source=share&utm_term=1 This was also posted as an instagram story https://www.reddit.com/r/WestSubEver/comments/mbij5c/cons_has_stem_player/

### 11 May 2021

Instagram story featuring the TE stem player and what seems like more developed packaging than the previous image. https://www.reddit.com/r/WestSubEver/comments/na8d1c/don_c_with_a_yzy_stem_player/
![bild|242x500](upload://iFzrnTVWKOuzv4t3omEBAeWbqtM.jpeg)

### 23 June 2021

Instagram story showing the inside of the packaging shown in the previous image https://www.reddit.com/r/WestSubEver/comments/o6k51t/kristen_crawley_with_the_yzy_stem_player/
![bild|231x500](upload://f1jvyxNgjIB313CK22pl0NVh95V.jpeg)

### 5 August 2021

The TE stem player is found loaded onto Kanyes Shopify store, however, it was not possible to purchase. https://www.reddit.com/r/WestSubEver/comments/oybela/yzy_stem_player_for_sale_thank_you_tolozen_say/

This can be further verified on Internet Archive, by uncommenting some hidden html that is still present on this date
![bild|327x500](upload://4WGpGkwQKcSRsmRzGiXJG0UtOfe.jpeg)

### 25 August 2021

“Donda Stem Player” by *Kano* officially launches.

### 16 April 2024

A UCC filing shows that Yeezy LLC pledged an unknown number of “Stem Player designed by Teenage Engineering” as collateral, amongst other items, in a deal with 1981 Distribution Inc. These *could* be the ones that are now being sold.
---

## Post #744 by TimK

Somebody needs to make a Walkman case with a clip like this 😂

[quote="maybe, post:743, topic:66795"]
Furthermore, it suggests that internal name Teenage Engineering had for the device was the TE

[/quote]

Is there something missing here?

Amazing lore drop otherwise! ❤️
---

## Post #745 by Moofy

Well I just bought one via a friend in the states. This looks like it’ll be fun to play with and a lot cheaper than buying one of their mixers…
---

## Post #746 by _pete

So in order for the SP to connect to the website, we need to initiate the button combo, correct? I’m not sure if I haven’t properly configured webserial or if I’m doing something wrong, but I’m not seeing my stemplayer pop up - anyone have an example of what the website is supposed to look like when things are up and working properly? I might be having difficulty because I’m using a USB hub - I’ll try directly via USB-C on my laptop shortly (I also don’t have any alt firmwares to upload - I just can’t help myself / my curiosity is brimming).
---

## Post #747 by TimK

haha love this 😂 
So after you’ve selected a VALID binary, it will display the instructions to connect and upload to a stem player.
The instructions are basically: hold track 1 + track 4 buttons while you plug in the USB cable. keep holding until track 1 LED lights up.

**But do not try this with a dummy / bad binary! It could very well brick your device!**
---

## Post #748 by _pete

Sweet. Thank you for clarifying, Tim! Will heed your warning until something real crystallizes!
---

## Post #749 by instantjuggler

[quote="TimK, post:744, topic:66795"]
Is there something missing here?

[/quote]

I think its a typo- internal docs name the device as the KE
---

## Post #750 by maybe

yes, edited the post! :)
---

## Post #751 by corpusjonsey

[quote="maybe, post:743, topic:66795"]
“Donda Stem Player” by *Kano* officially launches.

[/quote]

What does this have to do with the TE stem player?
---

## Post #752 by Virtual_Flannel

That is what Kanye released instead of the Teenage Engineering device after some fallout.
---

## Post #753 by corpusjonsey

Oh ok I see. Thanks! I should have just looked it up.
---

## Post #754 by nonkillwingman

As someone may wanna design their own gadgets for tesp01, here’s a sketch stl file that you can use it as reference. The size and buttons, led and speaker isn’t 100% correct but should be enough to design a case or something else. Remember to use millimeters in your software. I can also share other formats if someone need it.

Here’s the link.

[TESP mock sketch](https://www.thingiverse.com/thing:7312371)

UPDATE: there’s some geometry issues with convex version, the current one should be good to go

#2UPDATE, now the layout should be 99% of the actual design,
---

## Post #755 by ItDitry

I also made my own model, including the individual stem sliders and buttons, it’s not the prettiest thing to 3d print but it’s as accurate as I can get without making an assembly, I’ll drop the stl for it later but here’s a drawing I made for fun 😊
![image|647x485](upload://r3Ty59QQwfjUXtBlya9JWg62VsS.jpeg)

Edit - as promised here’s a download to it! https://www.thingiverse.com/thing:7312595
---

## Post #756 by k_o

reading thru this thread since mine arrived today

i spent the last month working on just this, it can add and edit cue/loop points and read them from aif/m4a/wav (little endian)/mp3 and a few others. its mostly js

if it’d be of use i can fork a version for this project![image|690x174](upload://vfaZUCYM9M2i3GZr6nI658MD83I.png)
---

## Post #757 by nonkillwingman


![20260311_161830|666x500](upload://kxTAcm2Sg9G7MioE1yhFiB7nANp.jpeg)

It is tight, will update file later.
---

## Post #758 by instantjuggler

thanks for the summary! really great collection of information there, some real deep cuts that are appreciated.

i couldn’t find it right now, but i recall that not too long ago, maybe within the past few months, didn’t jesper post a photo (most likely on instagram, maybe as a story) of him at ye’s wyoming ranch along with a handful of other TE members, during the wyoming era? 

i don’t have the timeline in front of me right now of the full wyoming era of the production studios out there on his ranch, but i wonder how that timeline lines up with the first public mention of either the TE or kano player.

as well, i have always suspected that virgil introduced ye to TE somehow… that could also be a timeline to chase up, to see when virgil first started to hang out with TE to start to triangulate some more details on the timeline. i’m sure if i was better at speaking swedish we could find out when ye was in stockholm, visiting and working with TE along with virgil, it was all over the news in stockholm back when that happened. 

i just wish alex klein would support his claims that TE “stole” the stem player (concept?) from kano with actual facts and dates and evidence. there are several sources of ye talking about making a stem player going back as far as 2013. would be so fun to untangle what the actual truth is…
---

## Post #759 by maybe

thank you! the only similar thing i've seen from wyoming is a meeting with alex klein, james turrell and kanye. but would be really useful to add that to the timeline if TE was at the ranch!

my theory is that work on both the kano and TE version happened in parallel, but if i were to guess it seems like TE was "first" considering that we know when alex klein and kanye met in 2019. the TE version was "done" during the same year while i don't think we even saw mockups of the Kano version until 2020/2021(?)

but yeah unsubstantiated claims everywhere about this so i hope that TE will release some product history book or something that will give insight to scrapped products and ideas they've worked on. (or if there is a lurking TE member that knows something :wink:)
---

## Post #760 by PedalsandChill

There’s some awful irony here. The one person we’ve all been trying to cleanse the device of; is also likely the one person who would blurt out the answer to who the progenitor of the stem player is. NDA’s be damned. If anyone ever has the misfortune of meeting him in person, please ask so we can know the answer. lol
---

## Post #761 by Paul_Roth

Are there any public current firmwares to try flashing onto the TE Stem Player, or are most being built from scratch? Sorry if this sounds like a stupid question.
---

## Post #762 by PedalsandChill

From my understanding not yet, the bones of everything are still being built.
---

## Post #763 by TimK

For my part, I have built a lot of proof of concept parts as fast as my free time allows me and I went up to uploading and playing my own stems on the stemplayer.

Recently the update utility was released which made all my work on a bootloader useless but also provides a much easier update route for everyone.

I already changed my firmware to work with the original TE bootloader. Now I'm busy cleaning up and testing so I don’t ship a brick. 

Next to this I'm trying to setup a documentation repo on github so other people could develop in parallel. (Which is a decent job in itself)

I understand people might be getting impatient, but this a hobby for me. I’ve been saying for weeks I'm close to an alpha release (for the brave) and that's where I still am. It’ll happen soonish, but I have some other deadlines to make right now. I wish I could go faster.
---

## Post #764 by nonkillwingman

silly move here.

DO NOT DUMP ANY FILE YET IF YOU DONT HAVE THE CORRECT FIRMWARE :sob:

I was so excited after seeing the web tool published. Was trying to play around the Nordic tools, as the web tool somehow cracked the bootloader, I was being silly to use a test bin and tries to block the flash while see what’s website is doing.

I did somehow find some codes that allows sp01 can at least send some thing, at this stage the player is still alive and good to go.

While I tried to do deeper, seems like either my code to stop the flash is failed or the player is under some protection mode as it is locked into a weird condition while I can’t turn off the device and the bootloader mode is actively on, the track 1 led is on for 60 seconds and function button will only make the device into that mode again. But at least i can still connect via usb at stage.

The i tried some button combo, as far as I remembered should be track 1 and 4 with some function clicking. The lights is off and still not on again.

The device is now bricked.:joy:

Assuming at this stage open it up with a nrf528040dk is the only method turn it back?

I still have 3 spares to play with, or at least I can dump the original firmware from…

@TimK TimK Would you mind if you could share me the og iron soldering documentation that I could possibly try on my devices?

EDIT: After some moves, the device is able to back to bootloader mode some how, here is what i tried(not in a order), don’t actually know which method worked. 

take the battery out for like 1-2 hours; disconnect the battery and short the power while press function button; press the function button for a while and then try 1+4 method again(after a fully disassemble for like 2 hour). 

So good news is this device is still able in bootloader mode. I’ll not suggest play around with the web and device without a working firmware, but at least this wasn’t ended that bad.
---

## Post #765 by TimK

If you still have the bootloader running you didn’t brick it. You can still load another firmware! But the original TE firmware is gone (for now) I can send you a medikit binary. I’ll contact you.
---

## Post #766 by nonkillwingman

Since my first unit is in its bricked condition for now, here is a case designed inspired by @41mad4 ‘s design. 

[TE stem player case](https://www.thingiverse.com/thing:7313465)

![case upper|690x493](upload://sX4ffDcKx5DopzoNdTBIBQys9Q5.jpeg)

![case btm|651x500](upload://i7xfPCjnT815Jl2qYuDOhvGzEnZ.jpeg)

![case overview|690x495](upload://6rhuMHQgl2qphFOnMAoj4pBfuQ5.jpeg)
---

## Post #767 by PedalsandChill

We’re all gonna end up recreating the whole experience from scratch ourselves. 😆
---

## Post #768 by TimK

This is a very strange product design masterclass by ye and TE 😂
---

## Post #769 by Paul_Roth

Oh no worries, THANK YOU for like everything you've done to push this project forward.  I think me and everyone else are giving standing ovations for the work you've done to advance and finally start cracking the SP-01
---

## Post #770 by taylorsizem_re

I just got my Stem Player in the mail today and im kind of shocked how fun it is on its own.

Is there a cheatsheet that details the advanced modes and their known features?
---

## Post #771 by nuk

first post, the video helps a bit more than the guide that’s also there
---

## Post #772 by Kysrox

Hello, I appreciate all the contributions made by people much smarter than me.

I was wondering, has anyone tried to sync two of the Stem Players together? I’ve been playing with the Stem Player + PO32, and it works amazing.
---

## Post #773 by PedalsandChill

I’ve tried. But it can’t receive signal. Only send it. Same as its midi functionality.
---

## Post #774 by Kysrox

RIP

I was envisioning being able to chain two of them together and getting more stem separation/mixing two songs stems together on the fly.
---

## Post #775 by PedalsandChill

Same. Maybe with the new fw?? 🤞
---

## Post #776 by dmon

I think, if we should be able to get USB midi running we could use something like we could use something like [the tubbutec usb bridge A](https://tubbutec.de/usb-bridge-a/) to allow midi sync (and other messages) in both directions.
---

## Post #777 by PedalsandChill

Kinda random. But I’m sure someone at TE is probably aware of this thread at this point. I can’t help but wonder what their thoughts are on the te-sp being picked up and continued by the community.
---

## Post #778 by lorem_xyaipsum

Heya! New here talking in the forum, I’ve been following this for a long time and today finally got myself a SP after a few weeks (I’m from Mexico so it takes a bit longer than expected lol), I’m so excited for what’s coming up for this little fella.

![20260314_083502|375x500](upload://1b7poif20kkB4mFGz2CrYmlgsuC.jpeg)

(ittle space boingy boof with it)

Also I’ve been working for a companion app with all the commands discovered at the moment (excluding the one for loading the binary file :P) in a animated format for a more easy understanding to new (and old) users.

[Little gallery because lines doesn’t allow me to put more pictures lol](https://imgur.com/a/kh9sRXU)

At this moment it is in a very rough beta in Android, which I want to expand it with even an iOS version once the firmware files and new findings for the SP are being released, also if someone has some suggestion it will be so well received!
---

## Post #779 by nonkillwingman

![20260315_151038|375x500](upload://ou90m8HUCFbebGQjd5mmubZfHyE.jpeg)

another forgotten design from TE
---

## Post #780 by pdrgf

Oh wow is that one of the Baidu speakers? There's so little info about those online; how'd you find one?
---

## Post #781 by nonkillwingman

Yes! the Raven H speaker, really hard to find new one recently, but can try on goofish and ask if any friend can bring it. Hear it was like 10-15 usd years ago, I paid around $45 for this one.

It requires old Android phone and old baidu home app to pair the controller and the speaker, tbh not much can do, but it should be running Android like other smart speaker, the issue is it only have one usbc which is power, and hard to find any tear down or any info about the chip it used…

But it definitely looks good.
---

## Post #782 by pdrgf

[quote="nonkillwingman, post:781, topic:66795"]
goofish
[/quote]
oh wow, I kind of thought they were vaporware tbh. You should definitely do a video running through what it can do. Can it at least be a basic bluetooth speaker
Did the R speaker ever get released?

I see one disassembled on goofish now and it doesn't look like it'd be very easy to put back together!
very very tempted though...
---

## Post #783 by PedalsandChill

So jealous of that raven H speaker! 😭😭
---

## Post #784 by nonkillwingman

It is a Bluetooth speaker,  just not so good. Dont think the R had a public release.

At least it can be used with stem player, really fun to play with,
---

## Post #785 by Maksim

Hi guys, I’m new here, found this thread today. I just got mine TE Stem player, sorry for the silly question, but did you found a firmware somewhere to upload on the https://solderless.engineering? Because when I open the site with the device plugged, I don’t see anything to connect it or am I being stupid? sorry for the dumb question
---

## Post #786 by tunelight

No firmware yet, and 20 chars
---

## Post #787 by Maksim

okay, got it, thank you!
---

## Post #788 by CH23

That’s really awesome, i’m glad the code was cracked! 

Do you think this could also make it possible to backup the current firmware, so it would be easier to exchange the stems?
---

## Post #789 by Virtual_Flannel

Not possible, I know its a lot, but please try and read through as much of the thread as possible as almost all questions have been answered. People are working on custom firmware so you can load your own stems, but it is not possible to unpack the current firmware which is why it need to be rewritten from scratch.
---

## Post #790 by CH23

From what I understood was that previous backups made seemed incomplete, but that was before the serial connection was discovered. If it’s possible to flash the device, then it might be possible to have it execute software to make it dump its own flash. I have been reading this thread for months at this point.
---

## Post #791 by Virtual_Flannel

from what i understand the official firmware is encoded in a way that makes it impossible to unpack, thus the need for a custom firmware.
---

## Post #792 by CH23

firmware decoding can take time, it’s not *impossible* per se. 

Technically decoding may not even be required, as it seems that the audio can be accessed/exported from the dump. I know @TimK  previously added a track to the existing set, and found the tracknames in the firmware.  This could also mean that we can find which parts of the firmware hold the audio and replace that.
---

## Post #793 by Winterrr

![image|375x500](upload://9XxdLPo9Z65B3t6bcxZ118pr8Xp.jpeg)

Such a fascinating device can’t wait for more to come and to help create stems!
---

## Post #794 by lynatic

Afaik firmware ≠ storage. There’s a small firmware storage chip inside the SoC and a larger chip that stores the audio files. The SoC has the APPROTECT bit set, meaning the firmware can’t be ready out (basically it has copy-protection) but it can still be overwritten. There is an attack to bypass that protection but it requires an elaborate hardware setup. Some people tried it and bricked their stem player in the process, then gave up.

This is where we are now. Flashing a new firmware doesn’t change the audio files since they’re on a separate chip. Timk is now developing a custom firmware from scratch (would be less of an IP-nightmare to publish anyway).
---

## Post #795 by Ronderswan

This is a dumb question, and I hate to ask, but would there be any credence to the idea of ‘vibe coding’ a solution?
---

## Post #796 by radioedit

I’d recommend perusing our 460-post (at the time of writing) thread on the subject :) 

https://llllllll.co/t/ai-content-within-lines-should-we-have-a-policy/70728
---

## Post #797 by _pete

You might brick the device. What a vibe kill that would be.
---

## Post #799 by TimK

Weekend update. I didn’t have much time to work on the new firmware lately, mostly because of work and dad life, so there hasn’t been too much progress on that. But there has been a lot of progress on another side that I can’t talk much about yet… Let me just leave this here.

https://youtu.be/liCm3V7YdOs

Happy weekend everybody!

💾👷🏻‍♂️
---

## Post #800 by instantjuggler

its hilarious that the te stem player is so small, that your dev unit is twice as big as the normal te stem player… and yet your dev unit is still smaller than the pocket operators, tx-6, tp-7, or cm-15… which are currently te’s smallest products!
---

## Post #801 by CH23

you just made my weekend a bit happier, this is very exciting!
---

## Post #802 by prettyflocko

oh my god……….. i sign in this thread to say thanks to you

im not programmar so i just watching this project for a few month

but finally thank u Tim K and other engineer bros
---

## Post #803 by Voltmeister

Wow Tim! Awesome!! You lit up the weekend! Thank you so far.
---

## Post #804 by KHAGE

We love you @TimK ! 👏🏽🙌🏽👌🏽👍🏽❤️
---

## Post #805 by Simone

Amazing, bravo @TimK !
---

## Post #806 by _pete

20 chars of *yippeeeeeee*!!!
---

## Post #807 by TimK

@instantjuggler  My dev unit is simply the original electronics folded open like a book, so yeah about 2x the size!

@_pete I’ve seen those 20 chars mentioned here before, what does it mean?
---

## Post #808 by _pete

20 characters is the minimum length for a post! So people usually add it to shorter replies to meet the limit.

I must also say, the selection of sounds in this demo really lend themselves to letting the character of each effect translate very well. Really great demo, Tim!! This gives us a lot of hope!

Also, how does the BPM analysis work with the triplet song, the last one in the demo? I have a thousand questions!
---

## Post #809 by cityz3n

Okayyyy, after following this thread for a year and half I finally caved in after seeing compatriot Tim’s video today and bought one. I kept myself from buying one because I funded the tape! kickstarter, which is a bit in the same size and category. But one can never have two many small 4 track contraptions. ;-)
---

## Post #810 by TimK

Thanks for the explainer Pete! And thanks for the compliment on the sounds. I didn’t really think too much about this, just exported some stems from old stuff on my OP-Z that I thought could work on the SP-1.

I don’t do BPM analysis on the songs. Right now the timing metadata is completely left blank. I half figured out how they do it, but not completely. I did find there are no cue points in the metadata. In the TE audio format there’s a 16bit counter that is continuously running up to `24489` in 96 steps (4x24!) and then starting over. I believe this encodes the MIDI clock and thus the BPM, but I haven’t implemented it yet in my own stem converter, so there simply is no timing for the TE firmware to work with in my songs. This makes for example the gate effect not work on my songs yet. Does that make sense?
---

## Post #812 by _pete

Makes total sense, and thank you for explaining. I think 24PPQN (pulses per quarter note? I believe?) is standard resolution for MIDI clock/time code. I am not a coder, but I know that I’ve dealt with similar clock multiplications when converting MIDI signals to sync with eurorack modules. Would need someone with more experience in the nuts and bolts to confirm this.

I can only speak for myself when I say that synced looping of audio (and I guess effects by proxy if everything is synced) is pretty high priority for being able to play back my own material on this device. Sounds like MIDI sync out would come along with understanding that internal process.
---

## Post #814 by TimK

I fully agree and you’re right about the MIDI clock. The embedded timing data in the audio format is essential for a lot of the timing based effects like looping, gate, delay(stutter?) and 2 of the LPF presets. All of those don’t properly work with my stems and looping even mutes my songs now!

I have analyzed the timing bytes only partially, because I didn’t have the time yet. But I’m convinced this will not be a problem. I already have calculated what I think is (part of) the BPM of each song on JIK from these bytes, but didn’t have time yet to measure the BPM of the actual tracks to verify. (Does someone have those maybe?)
---

## Post #815 by maybe

wow. i had totally given up hope these things would be able to play new songs for a long while. amazing work!
---

## Post #816 by Squezyx

If I understand correctly, you are already on the way to using TE stem player as a donda stem player?
---

## Post #817 by Justmat

it's more that we can use the TE stem player without having kanye on it at all lol 

but sure, you could put more/ different kanye on there if you wanted
---

## Post #818 by Squezyx

I mean, any song can be separated into stems on te stem player now, right?
---

## Post #819 by _pete

No, the TE Stemplayer only plays back studio-quality, pre-separated stems.

The Kano Stemplayer has stem separation technology, but it’s locked in a web client (the device itself just manages playback) and the stem separation tech is based on free, open-source stem separation algorithms that aren’t going to be any better than current software offerings in 2026 (the Stemplayer was released in early 2022 and spectral algorithmic stem separation has since improved to the point of becoming commonplace, see Serato and Ableton and Akai’s offerings), and with the Kano Stemplayer, unless there’s a major paradigm shift at the company (unlikely), you probably won’t ever be able to add your own studio-quality stems. Right now they’re trying to turn it into a stem-centric streaming service with only certain material by certain artists being released.

TLDR: TE Stemplayer is on track to be able to receive your original music, pre-separated into stems (thanks to the lovely folks here!!). The Kano Stemplayer can split stems from any song (at negligible quality), but can’t receive pre-separated stems, even in dev mode.

The TE Stemplayer also never released officially - the units you see here are not supported by any company or entity despite TE making them and Kanye’s name being on the device. They’re also less than $50 if you know where to find them. Kano / Stem (as they’re now known) offer a certain level of support for the devices if things go wrong. The Kano unit is $200.
---

## Post #820 by trakigt

a question, do we know how much storage the TE SP has? since it was made only for one album im guessing it could have even less than the Kano/Stem 8gb
---

## Post #821 by Squezyx

So, if I have song files divided into stems, I can play them stem by stem on a TE stem player?
---

## Post #822 by pdrgf

4GB of space, from what I can recall the write times are quite slow, not sure if this has been resolved with recent discoveries but I'd guess not?

[quote="robbie, post:11, topic:66795"]
a Toshiba [THGBMNG5](https://www.digikey.com/en/products/detail/kioxia-america-inc/THGBMNG5D1LBAIL/9841782) 4GB eMMC flash
[/quote]
---

## Post #823 by Virtual_Flannel

Eventually when someone makes a firmware capable, which Tim is working on.
---

## Post #824 by PedalsandChill

On the way of using TE stem player as a TE stem player.
---

## Post #825 by instantjuggler

just for completeness- you can load your own stems onto the kano/stem stem 1 and stem 2. i have a stem 1 full of my own tracks. there is also a published way to load your own tracks onto the stem 2.
both devices need you to boot them into a special mode, basically a disk mode where you then drag and drop your specially and precisely prepared folder (and sub folder structure) containing your stems. its a pain in the neck to say the least. but you used to be able to put whatever track you want onto stem 1 using the old website, your track or any other track. and that UI was pretty easy i have to say.
i had an interview with stem a couple of months ago. they wanted to ask about my “customer user experience.” during that talk i learned that they don’t have a clear internal path of how they will support stem 1 in the future. they claimed they couldn’t pay the bills to keep stem splitting for uploading to the device for free. and in general i think its a major problem for their PR… the notion that their hardware is doing any sort of stem splitting itself. its all tied to a website which is a whole different thing in and of itself. the hardware is in many ways just an accessory for where the company is trying to go now (some AI powered streaming platform to compete with spotify). its just that kano doesn’t realize that yet!
---

## Post #826 by _pete

I worked with Kano briefly in 2022 shortly after the Donda 2 release in Miami. I synchronized the heartbeat for playback leading up to the event and made the heartbeat sound from scratch. After that event, Alex Klein reached out and I worked with them in London doing sound design for UX/UI on the as-yet-unreleased projector, stemplayer, and a couple of other unreleased devices that I was prototyping. Admittedly, my ideas were pretty psychedelic: I wanted devices to sing (like birds) to each other when paired with each other in different combinations, I wanted to incorporate webAPI data of weather data to incorporate synthesized wind that reflects local weather patterns - the team seemed pretty receptive creatively.

The whole contract fell apart in October following the Fashion Week event and Kanye’s subsequent political meltdown. I suspect they cut all of their contractors, and I may never know if my work will be included on any of these projects. They tried making me sign an NDA, but the one they sent had nothing to do with my work - it was clearly copy and pasted from the Donda 2 event. So I won’t get in trouble/stand to lose nothing by discussing this publicly.

I was the only full-time musician they had hired (albeit as a contractor). I brought a Norns, Grid, and a small eurorack setup to the office to set up at my little temporary desk (as well as a few books about industrial design and interfaces as related to all five senses). Interest was limited - they were so preoccupied working around Kanye during arguably his most manic period. I’m still in touch with some of the engineers and designers, but Alex refused to acknowledge cutting me loose aside from stopping payment and taking me off of their slack channel.

Back then, there was no way to upload stems, and ostensibly there would never be. Dev mode didn’t include the ability to load pre-separated studio stems at all, so this is news to me. I also gave away all three of my Kano units (they gave me a few and I bought one out of curiosity at the event).

All of this is to say that I’m not remotely shocked by your recent interview. I hope I don’t come off as overly critical when I describe my experience as not having seen proper project management firsthand back then, and very loose, nebulous goals. Also, Alex is a kind, but perhaps not very considerate person. Or maybe the other way around. I only got to know him so well.

Edit: found a photo of my desk, heavily cropped for the sake of avoiding spoilers for any fans. I was really determined to help turn the Stemplayer into more of an instrument. 

![IMG_3246|384x500](upload://fLuLr5tWwKIsh8ih2JvRpk8TA22.jpeg)
---

## Post #827 by instantjuggler

thank you for this detailed inside look at your experience with the company! basically kano told me the file structure of the folder and how to format all the information and audio files in order to put them on stem 1, with stem 1 in a disk mode, just connected to a computer via a usb cable. and then they said there’s the same way to do that with the new stem 2, but i’m too lazy to go through all of that again. its kind of stupid i think, because they don’t really support stem 1 in a sustainable way right now. i guess there’s some sort of stem 1 support on their stem.fm platform but that’s not public right now so… seems like they would at least make a user interface on a web page to upload your own pre-separated stems onto stem 1. but yeah, let’s get excited about the TE stem player firmware that will allow us to not have to deal with kano anymore!!!
---

## Post #828 by _pete

100000% - I find it interesting how odd energetic circles open and close, like @TimK’s video drop yesterday, the same day as Kanye’s new record, this old device being given new life, and the current (perceived) winners of the deal in the moment having such difficulty articulating a path forward without the very reason for their initial success (and subsequent downfall) in this field in the first place.

It’s also taught me a lot about the power of good design and community care: the fact that this little TE stem player is so *impossible to ignore* (to the point that we’ve arrived at DIY community solutions here and now) and the fact that the company committed (ish) to making stem players is not supporting of a community enough to let people clearly import their own songs really speaks volumes on all fronts. Teenage Engineering may have been right, in hindsight, to make something impactful and get out before things got messy. What an odd world.

The whole experience taught me a lot, but ultimately gave me insight into the creative vortex that I didn’t have access to before, as well as the confidence that my ideas deserve to exist, and that I’m articulate enough to create complicated networks that lead to execution of weird ideas. And since then, music has brought me to beautiful places.
---

## Post #829 by Moofy

I’m curious, what were the books on interfaces and industrial design?
---

## Post #830 by _pete

[Michael Haverkamp’s “Synesthetic Design”](https://birkhauser.com/en/book/9783034607155), [Hamish Meikle’s “Catalog of Strategies”](https://www.abebooks.com/9781584230991/Catalog-Strategies-Meikle-Hamish-1584230991/plp), to name a couple. I will also add to this list [Osamu Sato’s “The Art of Computer Designing”](https://www.colpapress.com/products/the-art-of-computer-designing-osamu-sato), although I got my hard copy after 2022.
---

## Post #831 by Moofy

For those interested here’s a copy of the 1993 version of Sato’s book [The Art of Computer Designing](https://archive.org/details/satoArtOfComputerDesigning/mode/2up).

The updated one looks beautiful.
---

## Post #832 by _pete

sorry for getting off-base. My point is that it’s nice to be able to focus on the TE player having a new future, unbound.
---

## Post #833 by TimK

Took a look at the timing data encoded in the audio and figured it out! All bytes are now accounted for. Next demo will include ALL effects. 

💾👷🏻‍♂️
---

## Post #834 by _pete

Now that’s so rad. Tempo-synced effects are huge.
---

## Post #835 by Okey_Nwachie

Omfg TIM K!!! You absolute legend. I had lost faith entirely and now cant wait for that video drop.
---

## Post #836 by TimK

A little off topic, but for some reason, I find myself needing to record more stems off my OP-Z or OP-XY… it’s a pain in the neck! Does anyone have a good workflow for recording 4 stems off one of these devices?

For those who can’t wait to start developing for SP-1: I’ve already documented most of the audio format in the [Github wiki](https://github.com/timknapen/SP-1-dev/wiki/Audio-format), and I’ll add the information about the timing there when I’ve properly verified my findings. I think I have figured out how to generate the correct timing data and I hope to do a test with my own songs on the TE firmware soon.. unfortunately I’m super busy with work until at least end of next week 😭

Meanwhile, someone has been trying to break into my website. Please, I think I know who you are, just contact me. There’s really no need to spam my server 🙄
---

## Post #837 by wayk

> A little off topic, but for some reason, I find myself needing to record more stems off my OP-Z or OP-XY… it’s a pain in the neck! Does anyone have a good workflow for recording 4 stems off one of these devices?

I have had similar frustrations trying to record tracks from OP-Z. There is the [underbridge project](https://github.com/BKLronin/underbridge/tree/qt) that is aimed at doing this. I remember trying it a while back and finding it a little fiddly, but its definitely worth a try.
---

## Post #838 by baris

[quote="TimK, post:836, topic:66795"]
Meanwhile, someone has been trying to break into my website.

[/quote]

Swedish TE ninjas trying to break in?
---

## Post #839 by Winterrr

I have two devices willing to possibly brick one to try out the new firmware. I can also upload my own stems and send you footage to see how the progress is on my device! Do reach out if you need stems as well! Don’t know if they need to be specifically from the OP-Z tho
---

## Post #840 by radioedit

[quote="TimK, post:836, topic:66795"]
but for some reason, I find myself needing to record more stems off my OP-Z or OP-XY… it’s a pain in the neck! Does anyone have a good workflow for recording 4 stems off one of these devices?
[/quote]

We have a thread for the [OP-Z](https://llllllll.co/t/teenage-engineering-op-z/2326) and one for the [OP-XY](https://llllllll.co/t/teenage-engineering-op-xy/69435) where you could ask :slight_smile:
---

## Post #841 by TimK

I’ll check with the experts over there! Thanks!

[quote="baris, post:838, topic:66795"]
Swedish TE ninjas trying to break in?

[/quote]

Haha no i don’t think TE cares about what we’re doing! And all traffic came from a town in Texas.
---

## Post #842 by louwrens

Very cool and exciting developments! Could you perhaps explain what changed that now allows you to use your own stems while still using the TE firmware? (I am aware of the SE firmware uploading tool). I was somehow under the impression that it wasn’t possible at first and that the only way to get custom stems on there was to write custom firmware. If this has changed is there still a need to develop custom firmware?

The dev you are talking about here is to make something that will encode the stems into the correct format according to the documentation you compiled and shared?
---

## Post #843 by CH23

TExas huh….coincidence? i think not!
---

## Post #844 by TimK

Yeah, I took a little break from working on my own firmware because I just have too much work right now and I found a way to play my own stems on the original TE firmware, so no custom FW needed for that! (I’ve still been using my own custom firmware and software to load the stems onto the stem player)

To play new audio on the original firmware, we have to fully understand the SP-1 audio format, including all the meta data. I had figured out the audio part months ago and the album meta data is pretty straightforward. All this is documented in the Github repo.
Just this week, I fully reversed the last missing piece: the timing data bytes that allow for all the looping and other time based effects. I’ll test my findings ASAP to make sure I got it right before I publish bad info.

[quote="louwrens, post:842, topic:66795"]
The dev you are talking about here is to make something that will encode the stems into the correct format according to the documentation you compiled and shared?

[/quote]

Yes! It seems to me that most people would be perfectly happy to keep using the original TE firmware and just play their own music. Development would be needed to create a tool that can convert 8 channel WAV files (or 4 stereo files) into the SP-1 audio format, add all the metadata and transfer to the player. There is a way to do that and I’ll work on that next when I have time again. Of course, once the data format and the transfer protocol are fully documented, anyone could build a tool to add new stems!

I still think custom FW is pretty cool, and I hope my developer documentation would be helpful for people interested in this. I have also half set up a Discord server for discussion on the software and firmware development, but I haven’t gotten around to finishing it because again, not enough time.. (and I have no clue what I’m doing with that stuff! 🫣)
I’m truly sorry for anyone who is eagerly waiting to start development, I promise I’m going as fast as I can!
And sorry for this wall of text!

[quote="CH23, post:843, topic:66795"]
TExas

[/quote]

😂 IT ALL MAKES SENSE NOW!
---

## Post #845 by PedalsandChill

Don’t apologize for taking the time you need to develop something for free. I started this thread roughly 2 years ago just hoping the right eyes and minds would see it. Many people didn’t think where we are now would even be possible. All that to say, personally I’m just grateful for all the work yourself and the solderless team have already done. I look forward to the open source nature of a custom fw.
---

## Post #846 by _pete

I think (given how people have longed for this device to have new life for a long time) people would love to use the stock TE firmware over nothing any day of the week. However, given that you’ve already been bearing the brunt of the hard work taking the more interesting, perhaps sustainable route, that gives this device a lot more than a second lease on life. Let me explain:

I’ve owned the (sadly discontinued) [Critter & Guitari 5 Moons](https://www.critterandguitari.com/5-moons) for a long time now, and it’s been my de facto “stem player” for a while. It also has the benefit of being able to record in, and I managed to battery mod mine (not a terribly tough process, even for a novice), making it more of a looper than a straightforward stemplayer, but that’s beside the point. The 5 Moons has been open-source from the start, meaning swapping firmware has been as easy as mashing all of the buttons while connected to a computer and swapping a single file and folder. Although the device didn’t sell well (and sadly C&G aren’t known for maintaining support for a lot of their old instruments / sometimes only make short runs of niche devices), people have uploaded a lot of custom .pd patches and made a lot of use out of a few faders and buttons. New effects like varispeed, reverb, delays, all on top of the looper - things the maker could have never conceived of.

Another thing I will say is that the TE stemplayer has that *funny little rocker* on the side. I’m a late-to-the-game Octatrack owner, and I bought it because although there are many samplers like it, none of them have that *funny little crossfader*. Mapping it to macros has been a joy. Seeing the rocker on the SP-1 also reminds me of the way TE implemented the rocker on the [TP-7](https://llllllll.co/t/teenage-engineering-tp-7/63256). It also reminds me of the switch on my beloved [Mannequins W/](https://llllllll.co/t/mannequins-w-2-0/34091) modules - one switch combined with a couple of buttons opens up many worlds of control. I wonder how the rocker on the SP-1 can be implemented as a macro controller / perhaps decoupled from playback.

Paving the way for custom firmware will pay off, if not for this small community, for many folks beyond who may have gotten their hands on this funny little device. We also all have the [TX-6](https://llllllll.co/t/teenage-engineering-tx-6/54737)’s many modes as inspiration for where this little thing might be able to go. The work done here could be really great.
---

## Post #847 by Virtual_Flannel

[quote="TimK, post:844, topic:66795"]
found a way to play my own stems on the original TE firmware

[/quote]

How do you do it? This is the holy grail!
---

## Post #848 by axismecca

[quote="TimK, post:844, topic:66795"]
custom

[/quote]

Thank you for all of your work, I’ve been following this project for years. Never thought we would actually see this day. TE stem player has to be my favorite product design ever. 


Question, sorry if this is obvious, I really just don’t know.

https://solderless.engineering/. what does that link do?

it uploads your custom firmware? What would be the point of doing that? 

What are the differences in using the TE firmware or custom for playing tracks?

How exactly are owners of the TE stem going to put our own music on it? 

is it going to be drag and drop? a dev mode like the kano stem player? 

thank you for your work and if there is a discord I would love to join.
---

## Post #849 by Virtual_Flannel

Hey there, answers to all your questions are in the thread, give it a read, its a good time! To summarize, the website will allow you to upload a custom firmware so you can upload your own stems. Such a firmware doesn’t exist yet. People are working on it.
---

## Post #850 by axismecca

thank you for answering.

so if im not mistaken, im able to use the stem player for my own music now?

or did they achieve the breakthrough of allowing songs to be uploaded to the player?

Im just not sure what the firmware is exactly and what it does. as far as i know, people still haven’t been able to put their own music on the device?
---

## Post #851 by Simone

two things are going on. The website will eventually allow custom firmware, but a recent development is that @TimK and presumably others have been able to put their own stems on the TE firmware, which is super exciting. *You* (and I) cannot do that yet without lots of effort but it should come eventually.

Personally I am still hoping custom firmware efforts will happen because I feel like as fun as it is, the TE firmware is a bit undercooked, there is more that could be done with this interface, especially RE: the effects. But just getting Kanye out of there would be such a massive improvement.
---

## Post #853 by axismecca


thank you, this is groundbreaking. thank you tim
---

## Post #855 by eeluminaughty

Just ordered my second device to tinker with, has anyone found a fitting alternative battery for the examples that don’t work anymore? I got lucky with the first one but was just thinking about how to retrofit a replacement should the time come. Also -  thought about if anything else had to be done other than simply soldering the wires to the corresponding terminals. A lot of the examples that I was finding included the pcm board so I thought it wouldn’t be much else to it. 

As for battery fitment I went off the model name printed on the battery assuming it was 3.5mm thick 29mm long and 31mm tall I’m looking at one advertised [here](https://ebay.us/m/haEY71) as 3x30x35 as in some internal [pics](https://www.reddit.com/r/teenageengineering/comments/1hlbi25/kanye_stem_player_internals/) I’ve seen the length might have some wiggle room to spare. I’d love to hear if anyone has any input or found workarounds to keep this neat device truly wireless if the user wants hehe
---

## Post #856 by PedalsandChill

I believe someone further up the thread managed to change/update their battery. But I'm not 💯 sure.
---

## Post #857 by instantjuggler

@PedalsandChill @TimK Leveling up the collection- I am ready!

![IMG_5812|375x500](upload://5APrEfPrO2twUn8uLkgTmw0JcOY.jpeg)

![IMG_5819|375x500](upload://ct9BCwagGFAIbk0N6CvglPfJEFo.jpeg)
---

## Post #858 by TimK

Floating stem players in space! Love that photo. One could imagine these being juggled 😎

Can’t wait to get started on this again. Just a little bit more real work to finish and I’ll be back spamming this thread again! 😂
---

## Post #859 by Okey_Nwachie

Take your time for suree. Now have i been checking every single day waiting on an update? No comment 😂
---

## Post #861 by N0T-1SAC

Are there any updates yet? I've been super invested in this thread and read almost all of it, and I'm super excited to be able to put my own music onto this in the future!
---

## Post #862 by wheelersounds

I believe @TimK has said that he’s eager to post updates when he has the time to do more work – I’m glad you’re excited! I am too. Let’s wait for Tim to post an update, he’s been very generous with his time and also updates here very soon after making progress!
---

## Post #863 by KHAGE

I finally managed to get my hands on two more units, but unfortunately one of the Stem Players arrived with a broken fader near the control knob.

![Broken Stem fader|374x500](upload://vkOCBBWjGMyJ5AUFb6Q8oK8LQnA.jpeg)

😱😥😭

This actually makes me want to be extremely careful with my other Stem Players, as this plastic part seems particularly delicate and fragile to me.

Do you think there’s any way to replace this fader?
I could try gluing the knob back on, but it’s so thin that I’m afraid the glue might run off to the side and jam the linear movement.

Since I know I can’t request a replacement for this unit, what would you do in my place?
Is this third fader beyond repair?… Or do you think there might be a solution?

Thank you very much for your time and your valuable advice. 🙏🏽
---

## Post #864 by CH23

the part is a b47k slide potentiometer from taotao (also known as taowaves) but I can’t find it online.

what you could do is put a tiny amount of glue on the broken off part, then put that on the slider. that way you shouldn’t have spills. use a toothpick to apply the glue to it.
---

## Post #865 by KHAGE

I'll try to be extremely careful when applying the superglue… and I hope it holds, since the fader will be handled frequently. 🤞🏽
---

## Post #866 by murray

in addition to putting glue on the part, you could find some adhesive-style strip and secure the glued part to the fader while the stem player is mounted upside-down. this way, any extra glue runoff will flow down the part and not into the well of the potentiometer itself.
---

## Post #867 by russ

Would love to know if there is any way the community could help pick up some of the work or fill in any gaps here. I know @TimK has said for a while that he’s been busy with other stuff, so i’d love to help in any way if there is room for that. whether that’s testing, documentation or even helping around the public facing side of things for the community to actually use.

it feels like there are enough interested people here that some of the load could maybe be shared. if there are known pitfalls or areas where extra hands would actually help, i think a lot of people here would be interested in contributing in any way! if there’s already a discord or another place where some of this is being discussed, I would love to join the discussion there there too.
---

## Post #868 by TimK

The Taowave slide potentiometers are hard to find through the regular suppliers, but I think i’ve read about people on here breaking off volume buttons or destroying their devices in other ways. Maybe you can contact one of them for a donor? If you’re comfortable with SMD soldering, it looks pretty doable to replace one of those faders.
---

## Post #869 by disc0p

Picked mine up at the border yesterday, excited to tinker!
---

## Post #870 by JaggedNZ

There are some SMD potentiometers on aliexpress. They are not the original Taowaves ones but you might be able to find one thats close enough. SMD potentiometers or "direct sliding potentiometer" seem to be key search terms.
---

## Post #871 by KHAGE

Unfortunately, since the contact point is extremely thin, the glue didn't hold up after few gentle adjustments to the fader.
Since I don't have the right equipment—let alone the skills—for SMD soldering… I think I'll just have to make do with three tracks on this STEM… 😢 …or use a toothpick to move the broken fader.
---

## Post #872 by burnt

unless the pins are hidden underneath
smd doesn’t really require any special tools
just a good plan and some patience
which are helpful things for any type of soldering

things like jacks and faders are usually amongst the easier things to desolder / solder w a standard iron, a sucker or wick, and maybe some flux
again all things that are common for any type of soldering

only saying bc i see ppl say things like that all the time and just want to help dispel any myths that might deter ppl from trying

u can do it! (if u choose to)
---

## Post #873 by KHAGE

![image|375x500](upload://zJdkpYs6ZZw4YKIUVvVKCjKePTF.jpeg)

*(original image posted right here, much earlier in this thread)*

Between the micro-soldering and the delicate disassembly process, I have to admit I’m feeling uneasy about this challenge…
---

## Post #874 by charbot

If you can find the replacement slider,  you can likely get the work done at a cellphone repair place.   Ultimately, tho...it may be more than the cost of a new device
---

## Post #875 by CH23

You could also try to find a local hackerspace, ask them to help you with desoldering and soldering in a new one.
---

## Post #876 by KHAGE

[quote="TimK, post:868, topic:66795, full:true"]
The Taowave slide potentiometers are hard to find through the regular suppliers, but I think i’ve read about people on here breaking off volume buttons or destroying their devices in other ways. Maybe you can contact one of them for a donor? If you’re comfortable with SMD soldering, it looks pretty doable to replace one of those faders.
[/quote]

Is there a generous organ donor reading this ? ;-)
---

## Post #877 by br4n_d0n

I find it interesting that the sliders are like that when I would've it would be like the TX-6

![fd57b6d1001c078f794f08f6906ddc051c8b7b08\\_2\\_562x750|374x500](upload://fzja8BiaOzB1tKuNBmaMOIPJTDt.jpeg)

Credit: [PedalsandChill](https://llllllll.co/t/teenage-engineering-tx-6/54737/519)
---

## Post #878 by TimK

Is this inside TX-6? Fascinating board! What are the chips between the pots? ADCs? And also look at the routing to the switches on the right! 🤔
---

## Post #879 by br4n_d0n

Yes, if you look at PedalsandChill's 2 posts: https://llllllll.co/t/teenage-engineering-tx-6/54737/486 , https://llllllll.co/t/teenage-engineering-tx-6/54737/519 , you'll see the TX-6 torn down. It contains multiple PCB's stacked on top of each other.
---

## Post #880 by BenjyBuilds

Btw, have you considered using airfix (or similar) plastic bonding cement? It would probably work for plastic contacts that small if you use a needle
---

## Post #881 by KHAGE


Is that the kind of glue used in model making?

Oh… that's a pretty good suggestion. 👍🏽

Thanks for the tip !

PS. Something like this [Tamiya Extra Thin Cement](https://www.tamiya.com/english/products/87038/index.html), maybe ?
---

## Post #882 by N0T-1SAC

This is like very random but I made a little sack bag thing to hold my sp-01 because it would get all scuffed up in my pocket. its took me a bit the first version was wayy to long and I didn't sew it together correctly, but I think I did a good job
![Untitled872\_20260429210444|375x500](upload://57zPXcB9yFjWpqQinctU7TDum31.jpeg)
---

## Post #883 by BenjyBuilds

Try use humbrol poly cement, i don't know that one
---

## Post #884 by opd

Likely just cheaper/quicker to do it the way the Stem Player did vs the TX-6; the SP uses more off-the-shelf components and was only a prototype product produced in a presumably small batch, while the TX-6 is fully custom and in production at higher quantities.
---

## Post #885 by russ

Has there been any mention of recent updates from the solderless team? Haven't seen anything in a while, would be great to hear if there are any roadblocks where help might be useful.
---

## Post #886 by PedalsandChill

If you don't see any updates at the top of the thread, (like has been mentioned several times already) then there are no updates. People need to stop spamming this post and actually read.
---

## Post #890 by JosueArias

can anyone tell me the aprox weight for the package if you bought from  newrepublic? Thanks!
---

## Post #891 by PaulFe

Hello, I was reading a post (posted in the past couple days if i remember correctly) in this thread by someone (I didn't memorize their username), and it seemed like they had already achieved loading their own files to the TE STEM Player and had some of the original firmware rebuilt or being used.  It was quite an in depth post regarding how they got to where they were at, and even had a screen shot.

I'm just wondering (maybe being a bit nosey) why it got deleted.
---

## Post #892 by _pete

20 chars of scroll up! pretty sure it's there.
---

## Post #893 by system

*Staff note:* this thread has been closed due to continual issues with problematic activity that has been contrary to the [Forum Rules](/guidelines) and [Terms of Service](/tos), as well as that activity's interference with the ability of project maintainers to coordinate development efforts. They have setup an external platform for technical and community discussion, so visit the project's Discord for all updates, inquiries, releases, support, and development: https://discord.gg/y4V6VfHYck
