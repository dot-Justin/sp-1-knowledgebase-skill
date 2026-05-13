
This is the way the audio data is structured on the eMMC flash memory of the SP-1.
There is no filesystem, simply a list of songs with title and artist names and a memory offset and length expressed in sectors.

- 24 bit 48kHz audio
- 8192 byte sectors containing 340 audio frames each
- Album metadata is stored at the start of the first sector and is padded with X's (`0x58`)
- Bytes 2040-2043 of each sector are reserved for timing data
- Bytes 2044-2047 of each sector are reserver for LED data


## Audio data structure

The bytes we read the audio from.

All songs are 24bit, 48 kHz, 8 channel audio (4 stereo tracks) 

---

#### Song
A **song** is split in **sectors**

---

#### Sector
`size: 8192 bytes `

A **sector** contains 
- 340 **audio frames** = 7.083ms of audio at 48 kHz
- 2 bytes of timing/sync data (bytes 2040, 2041)
- 2 bytes encoding tempo (bytes 2042, 2043)
- 4 bytes of LED data (bytes 2044-2047)

A sector is split up in 4 x 2048 byte **blocks**

---

#### Block
`size: 2048 bytes`

A **block** contains 
- 85 **audio frames**

---

#### Audio Frame
`size: 24 bytes`

An **audio frame** contains 
- 4 stereo **stem frames**

Audio frames are not ordered sequentially inside a sector, but split up along the 4 blocks of a sector like so:  
(blocks are ordered 0, 2, 1, 3)

```
Block   Frame
0       0
        4
        8
        ..
        336
2       2
        6
        10
        ..
        338
1       1
        5
        9
        ..
        337
3       3
        7
        11
        ..
        339
```

If we were to iterate throught the 340 frames of a sector it would go something like this:

```C
#define FRAME_SIZE 24           // 8 * 3 bytes for 8 channel 24 bit audio
#define BLOCK_SIZE 2048         // size of a block in bytes
#define FRAMES_PER_SECTOR 340   // number of frames in one 8192 byte sector

int block_order[] = {0, 2, 1, 3};

for(int frame = 0; frame < FRAMES_PER_SECTOR; frame ++){
    int block_id     = block_order[frame % 4]; 
    int block_offset = BLOCK_SIZE * block_id ;
    int frame_offset = FRAME_SIZE * frame / 4;
    int byte_offset  = block_offset + frame_offset;
}
```

Where 
- `frame` is the index of the frame in the 340 samples of a sector.
- `block_id` is the index of the block that contains the frame. 
- `block_offset` is the byte offset of the block in the sector.
- `frame_offset` is the byte offset of the frame in the block.

The **audio frame** samples are structured like this:

```
offset  length  
0       6       stem frame 0
6       6       stem frame 1 
12      6       stem frame 2
18      6       stem frame 3
```

---

#### Stem frame
`size: 6 bytes`

A **stem frame** contains 
- 2 _interleaved_ left and right samples

In a **stem frame** the bytes are interleaved unconventionally: 
(LE 301, 452)

```
offset  sample  byte
0       L       MB
1       L       MSB
2       R       MSB
3       L       LSB
4       R       LSB
5       R       MB
```

So grabbing a stereo sample from the bytes in a frame would look something like this:

```C
int32_t sample_left =  (data[1] << 24) | (data[0] << 16) | (data[3] << 8); // 103
int32_t sample_right = (data[2] << 24) | (data[5] << 16) | (data[4] << 8); // 254
```



    