# SP-1 stem loader

Welcome to the SP-1 stem loader *BETA* !

This tool helps you load new stems onto your unreleased Teenage Engineering SP-1 stem player running stock firmware.

## Getting started

### 1. Prepare your stem files 
Before using this tool, combine stems that make up song in one single WAV file.

Stem loader currently supports one data format: *24 bit, 48 kHz, 8 channel, PCM WAV*.
Wav files can have any number of audio channels, but only 8 of them will be used. 

### 2. Connect your SP-1
- Make sure your SP-1 is turned off, then connect it to your computer via USB while holding the track 1 and track 4 buttons.  
- Keep holding those buttons down until the LED of track 1 lights up, then release. SP-1 is now in boot mode.  
- Click *Connect* and select stem player from the drop down that appears. 

### 3. Load your wav files  
Simply drag and drop WAV files onto the stem loader window. 

You can change the order of the tracks by dragging a song by its track number.
You can edit the album title and the titles, artist names and BPM of each song. 
BPM will affect the way the MIDI clock, gate effect, looping and other time based effects work on SP-1.

### 4. Transfer
Click *Transfer Album* and get ready to wait for a looong time.

The track 1 LED on SP-1 keeps blinking while data is being transferred.
(1 minute of audio takes about 10 minutes to transfer!)

>**Note 1**  
Stem loader needs to stay open until the transfer has finished. If you close the tab or if your computer goes into sleep mode during a transfer, the transfer will fail, but it can be restarted later. 

>**Note 2**  
SP-1 can seem unresponsive after a transfer was interrupted by unplugging the USB cable or closing the stem loader tab. This can look worrying but is normal: SP-1 is still in transfer mode and can't play audio.  
This can simply be resolved by long pressing the function button to restart SP-1. Data can become corrupted when a transfer is interrupted, so make sure to re-transfer any unfinished songs afterwards.

## FAQ

### How do I prepare WAV files?

Use your preferred audio software (eg. Audacity) to create 24-bit, 48 kHz, 8-channel PCM WAV files of your songs.

When a song is converted for SP-1, its channels will be mapped to the SP-1 stems like this:

```
CHANNEL   STEM
1         1, left
2         1, right

3         2, left
4         2, right

5         3, left
6         3, right

7         4, left
8         4, right
```

If a file contains less than 8 channels, the corresponding stems will be empty. If it contains more than 8 channels, those channels will be ignored.

### Missing files after reloading the page

For security reasons, web applications can't keep track of local file references between page reloads. When you reload the page or close the tab, the stem loader will remember the album index, but will forget where your files are. File missing icons will appear in front of each track. 

To re-link missing files, simply drag them again onto the window and the stem loader will try to link them to the correct tracks in the album.

### Song BPM

By default, the stem loader will set BPM to 80. You can change it to any value between 30 - 300 to match the BPM of the song.
The BPM will be used to encode the MIDI clock and timing of each song. It is needed for the gate effect, looping and other time based effects on SP-1.

### Pausing a transfer

A transfer can be stopped at any time by clicking *Abort transfer*. You can restart the transfer of the album later and stem loader will skip the songs that were already transferred successfully. The song that was currently being transferred will be left unfinished and will need to be transferred again. 

### Re-transferring songs

To re-transfer a song that was finished successfully, click the track number and *Mark new* to included it in the next album transfer, or click *Transfer song* to only transfer this particular song.
To skip a song during a transfer, click *Mark done* and it will be skipped during the next album transfer.

If you change the order of tracks in an album, parts of the album index will be invalidated (because of the way storage is managed on the SP-1) and certain songs will need to be transferred.

## Questions and feedback

Feedback and questions about the SP-1 stem loader are very welcome on the [TE SP-1 DEV discord](https://discord.gg/y4V6VfHYck)

## Credits

This tool was developed by Tim Knapen and <a href="https://solderless.engineering">solderless</a>.