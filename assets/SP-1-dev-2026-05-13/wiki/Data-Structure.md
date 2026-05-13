The flash memory of the SP-1 is divided up into 8192 (`0x2000`) byte sectors. In eMMC flash memory terms, those sectors are 16 (512 byte) blocks. The device has 4GB of storage.

The first sector is reserved for the [album meta data](album-metadata-format).

Starting from the second sector (offset `0x2000`) we can store audio data, which is document under [SP-1 Audio data format](audio-format).