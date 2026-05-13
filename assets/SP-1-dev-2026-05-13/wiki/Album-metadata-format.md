## Album metadata structure
```
offset  length  

0       13      Magic bytes 'ALBUM_PRESENT'
13      4       Album length in (0x2000 byte) sectors. 
                In the case of JIK sector 0x38EE4 = byte 0x71DC8000, 
                preceded by the second 'ALBUM_PRESENT'
17      1       Number of songs in this album
18      64      Album Title, null terminated and padded with X's
82      *       List of song metadata, see below for structure

8192    *       Second sector, start of audio data
```

A valid album has to have the magic bytes 'ALBUM_PRESENT' at the start of the album, and as the very last bytes of the album.

## Song metadata structure

```
offset  length  

0       4       Song start offset in (0x2000 byte) sectors
4       4       Song length in sectors
8       64      Artist name, null terminated and padded with X's
72      64      Song Title, null terminated and padded with X's
```