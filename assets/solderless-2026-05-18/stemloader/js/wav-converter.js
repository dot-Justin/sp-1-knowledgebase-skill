/**
 * parsing multichannel wav data
 * 
 * Supported format: 24 bit, 48 kHz, 8 channel signed PCM
 * 
 */

const FRAME_SIZE = 24;
const STEM_FRAME_SIZE = 6;
const BLOCK_SIZE = 2048;
const SECTOR_SIZE = 8192;
const FRAMES_PER_SECTOR = 340;
const MAX_SECTORS = 0x76000; // max number of sectors on eMMC (3,959,422,976 bytes)
const BLOCK_ORDER = [0, 2, 1, 3];
const CLOCK_MAX = 49152;
const CLOCK_INCR = 512;
const TICKS_PER_BAR = 96;
const OFFSET_LEDS = 2044;
const OFFSET_TEMPO = 2042;
const OFFSET_CLOCK = 2040;

/**
 * Checks if this is valid WAV data
 */
function wavVerify(buf) {
  const view = new DataView(buf);
  const tag = o => String.fromCharCode(view.getUint8(o), view.getUint8(o + 1), view.getUint8(o + 2), view.getUint8(o + 3));

  if (tag(0) !== 'RIFF') throw new Error('Not a RIFF file');
  if (tag(8) !== 'WAVE') throw new Error('Not a WAVE file');

  let fmt = null, dataOffset = 0, dataSize = 0, offset = 12;

  while (offset + 8 <= buf.byteLength) {
    const id = tag(offset);
    const size = view.getUint32(offset + 4, true);

    if (id === 'fmt ') {
      const af = view.getUint16(offset + 8, true);
      fmt = {
        audioFormat: af,
        subFormat: af === 0xFFFE && size >= 40 ? view.getUint16(offset + 32, true) : null,
        numChannels: view.getUint16(offset + 10, true),
        sampleRate: view.getUint32(offset + 12, true),
        bitsPerSample: view.getUint16(offset + 22, true),
      };
    }

    if (id === 'data') { dataOffset = offset + 8; dataSize = size; }
    offset += 8 + size + (size % 2);
  }

  if (!fmt) throw new Error('No fmt chunk');
  if (!dataOffset) throw new Error('No data chunk');

  const ef = fmt.audioFormat === 0xFFFE ? fmt.subFormat : fmt.audioFormat;
  if (ef !== 1) throw new Error(`Audio format must be PCM (got 0x${ef?.toString(16)})`);
  if (fmt.bitsPerSample !== 24) throw new Error(`Bit depth must be 24 (got ${fmt.bitsPerSample})`);
  if (fmt.sampleRate !== 48000) throw new Error(`Sample rate must be 48000 Hz (got ${fmt.sampleRate})`);
  if (fmt.numChannels == 0) throw new Error(`Must have at least 1 channel (got ${fmt.numChannels})`);

  const totalFrames = Math.floor(dataSize / (fmt.numChannels * 3));

  return { fmt, totalFrames };
}

/**
 * Parse WAV data and return 
 * - format 
 * - channels (audio data)
 * - total number of frames
 */
function parseWAV(buf) {
  const view = new DataView(buf);
  const tag = o => String.fromCharCode(view.getUint8(o), view.getUint8(o + 1), view.getUint8(o + 2), view.getUint8(o + 3));

  if (tag(0) !== 'RIFF') throw new Error('Not a RIFF file');
  if (tag(8) !== 'WAVE') throw new Error('Not a WAVE file');

  let fmt = null, dataOffset = 0, dataSize = 0, offset = 12;

  while (offset + 8 <= buf.byteLength) {
    const id = tag(offset);
    const size = view.getUint32(offset + 4, true);

    if (id === 'fmt ') {
      const af = view.getUint16(offset + 8, true);
      fmt = {
        audioFormat: af,
        subFormat: af === 0xFFFE && size >= 40 ? view.getUint16(offset + 32, true) : null,
        numChannels: view.getUint16(offset + 10, true),
        sampleRate: view.getUint32(offset + 12, true),
        bitsPerSample: view.getUint16(offset + 22, true),
      };
    }

    if (id === 'data') { dataOffset = offset + 8; dataSize = size; }
    offset += 8 + size + (size % 2);
  }

  if (!fmt) throw new Error('No fmt chunk');
  if (!dataOffset) throw new Error('No data chunk');

  const ef = fmt.audioFormat === 0xFFFE ? fmt.subFormat : fmt.audioFormat;
  if (ef !== 1) throw new Error(`Audio format must be PCM (got 0x${ef?.toString(16)})`);
  if (fmt.bitsPerSample !== 24) throw new Error(`Bit depth must be 24 (got ${fmt.bitsPerSample})`);
  if (fmt.sampleRate !== 48000) throw new Error(`Sample rate must be 48000 Hz (got ${fmt.sampleRate})`);
  if (fmt.numChannels == 0) throw new Error(`Must have at least 1 channel (got ${fmt.numChannels})`);

  const numChannels = fmt.numChannels;
  const totalFrames = Math.floor(dataSize / (numChannels * 3));
  const channels = Array.from({ length: 8 }, () => new Int32Array(totalFrames));
  const raw = new Uint8Array(buf, dataOffset, dataSize);

  for (let f = 0; f < totalFrames; f++) {
    const base = f * 3 * numChannels;
    for (let c = 0; c < 8; c++) {
      if (c < numChannels) {
        const b = base + c * 3;
        let s = raw[b] | (raw[b + 1] << 8) | (raw[b + 2] << 16);
        if (s & 0x800000) s |= 0xFF000000;
        channels[c][f] = s;
      } else {
        channels[c][f] = 0;
      }
    }
  }

  return { fmt, channels, totalFrames };
}

/**
 * Convert parsed WAV data to SP1 data.
 * expects 8 channel 24 bit 48 kHz audio
 * returns a Uint8Array of formatted data
 */
function encodeToSP1(channels, totalFrames, bpm = 80) {
  const totalSectors = Math.ceil(totalFrames / FRAMES_PER_SECTOR);
  const output = new Uint8Array(totalSectors * SECTOR_SIZE);

  if (isNaN(bpm)) {
    bpm = 80;
  }
  bpm = Math.max(20, Math.min(300, bpm));


  // samples per MIDI clock tick (24 PPQN)
  const samplesPerTick = (48000 * 60) / (24 * bpm);
  const tempo = Math.round(samplesPerTick);

  let clock = 0;
  let clockAcc = 0;
  let lastTickSector = 0;
  let nextTickSample = 0;
  let firstTick = true;

  for (let s = 0; s < totalSectors; s++) {
    const sectorBase = s * SECTOR_SIZE;
    const sectorFrameStart = s * FRAMES_PER_SECTOR;
    const sectorFrameEnd = (s + 1) * FRAMES_PER_SECTOR;

    let envelopes = [0, 0, 0, 0];

    for (let frame = 0; frame < FRAMES_PER_SECTOR; frame++) {
      const globalFrame = sectorFrameStart + frame;


      if (globalFrame >= totalFrames) break;

      const blockId = BLOCK_ORDER[frame % 4];
      const byteOffset = sectorBase + BLOCK_SIZE * blockId + FRAME_SIZE * Math.floor(frame / 4);

      for (let stem = 0; stem < 4; stem++) {
        const L = channels[stem * 2][globalFrame];
        const R = channels[stem * 2 + 1][globalFrame];
        const base = byteOffset + stem * STEM_FRAME_SIZE;

        // add envelopes
        envelopes[stem] = Math.max(Math.max(envelopes[stem], Math.abs(L)), Math.abs(R));

        // SP-1 stem byte order: [L_MB, L_MSB, R_MSB, L_LSB, R_LSB, R_MB]
        output[base] = (L >> 8) & 0xFF;
        output[base + 1] = (L >> 16) & 0xFF;
        output[base + 2] = (R >> 16) & 0xFF;
        output[base + 3] = L & 0xFF;
        output[base + 4] = R & 0xFF;
        output[base + 5] = (R >> 8) & 0xFF;
      }
    }

    // LEDS
    // normalize 24 bit envelopes and write to sector
    for (let i = 0; i < 4; i++) {
      output[sectorBase + OFFSET_LEDS + i] = 255 * envelopes[i] / 0x800000;
    }

    // CLOCK
    let clockTick = false;

    // Does the next tick fall inside this sector?
    while (nextTickSample < sectorFrameEnd) {
      clockTick = true;
      if (firstTick) {
        clock = 1; // hardcoded initial clock value
        firstTick = false;
      } else {
        // how many sectors elapsed since previous tick?
        const elapsedSamples = (s - lastTickSector) * FRAMES_PER_SECTOR;
        clockAcc += (CLOCK_INCR + samplesPerTick - elapsedSamples);
        const increment = Math.floor(clockAcc);
        clockAcc -= increment;
        clock = (clock + increment) % CLOCK_MAX;
      }
      lastTickSector = s;
      nextTickSample += samplesPerTick;
    }

    if (clockTick) {
      // MIDI clock 
      output[sectorBase + OFFSET_CLOCK] = clock & 0xFF; // low byte
      output[sectorBase + OFFSET_CLOCK + 1] = (clock >>> 8) & 0xFF;// high byte
      // tempo
      output[sectorBase + OFFSET_TEMPO] = tempo & 0xFF; // low byte
      output[sectorBase + OFFSET_TEMPO + 1] = (tempo >>> 8) & 0xFF;// high byte
    } else {
      // no clock tick  = 0xFFFF0000
      output[sectorBase + OFFSET_CLOCK] = 0xFF;
      output[sectorBase + OFFSET_CLOCK + 1] = 0xFF;
      output[sectorBase + OFFSET_TEMPO] = 0x00;
      output[sectorBase + OFFSET_TEMPO + 1] = 0x00;
    }
  }



  return output;
}
