/**
 * WAV file parsing and SP-1 audio encoding.
 * Supported format: 24 bit, 48 kHz, 8 channel signed PCM.
 * Depends on: protocol.js (for SECTOR_SIZE, FRAMES_PER_SECTOR, etc.)
 */

function wavVerify(buf) {
  const view = new DataView(buf);
  const tag = o => String.fromCharCode(view.getUint8(o), view.getUint8(o+1), view.getUint8(o+2), view.getUint8(o+3));

  if (tag(0) !== 'RIFF') throw new Error('not a RIFF file');
  if (tag(8) !== 'WAVE') throw new Error('not a WAVE file');

  let fmt = null, dataOffset = 0, dataSize = 0, offset = 12;
  while (offset + 8 <= buf.byteLength) {
    const id = tag(offset);
    const size = view.getUint32(offset + 4, true);
    if (id === 'fmt ') {
      const af = view.getUint16(offset + 8, true);
      fmt = {
        audioFormat: af,
        subFormat: af === 0xFFFE && size >= 40 ? view.getUint16(offset + 28, true) : null,
        numChannels: view.getUint16(offset + 10, true),
        sampleRate: view.getUint32(offset + 12, true),
        bitsPerSample: view.getUint16(offset + 22, true),
      };
    }
    if (id === 'data') { dataOffset = offset + 8; dataSize = size; }
    offset += 8 + size + (size % 2);
  }

  if (!fmt) throw new Error('no fmt chunk');
  if (!dataOffset) throw new Error('no data chunk');
  const ef = fmt.audioFormat === 0xFFFE ? fmt.subFormat : fmt.audioFormat;
  if (ef !== 1) throw new Error(`audio format must be PCM (got 0x${ef?.toString(16)})`);
  if (fmt.bitsPerSample !== 24) throw new Error(`bit depth must be 24 (got ${fmt.bitsPerSample})`);
  if (fmt.sampleRate !== 48000) throw new Error(`sample rate must be 48000 Hz (got ${fmt.sampleRate})`);
  if (fmt.numChannels !== 8) throw new Error(`must be 8 channels (got ${fmt.numChannels})`);

  return { fmt, totalFrames: Math.floor(dataSize / (8 * 3)) };
}

function parseWAV(buf) {
  const view = new DataView(buf);
  const tag = o => String.fromCharCode(view.getUint8(o), view.getUint8(o+1), view.getUint8(o+2), view.getUint8(o+3));

  if (tag(0) !== 'RIFF') throw new Error('not a RIFF file');
  if (tag(8) !== 'WAVE') throw new Error('not a WAVE file');

  let fmt = null, dataOffset = 0, dataSize = 0, offset = 12;
  while (offset + 8 <= buf.byteLength) {
    const id = tag(offset);
    const size = view.getUint32(offset + 4, true);
    if (id === 'fmt ') {
      const af = view.getUint16(offset + 8, true);
      fmt = {
        audioFormat: af,
        subFormat: af === 0xFFFE && size >= 40 ? view.getUint16(offset + 28, true) : null,
        numChannels: view.getUint16(offset + 10, true),
        sampleRate: view.getUint32(offset + 12, true),
        bitsPerSample: view.getUint16(offset + 22, true),
      };
    }
    if (id === 'data') { dataOffset = offset + 8; dataSize = size; }
    offset += 8 + size + (size % 2);
  }

  if (!fmt) throw new Error('no fmt chunk');
  if (!dataOffset) throw new Error('no data chunk');
  const ef = fmt.audioFormat === 0xFFFE ? fmt.subFormat : fmt.audioFormat;
  if (ef !== 1) throw new Error(`audio format must be PCM (got 0x${ef?.toString(16)})`);
  if (fmt.bitsPerSample !== 24) throw new Error(`bit depth must be 24 (got ${fmt.bitsPerSample})`);
  if (fmt.sampleRate !== 48000) throw new Error(`sample rate must be 48000 Hz (got ${fmt.sampleRate})`);
  if (fmt.numChannels !== 8) throw new Error(`must be 8 channels (got ${fmt.numChannels})`);

  const totalFrames = Math.floor(dataSize / (8 * 3));
  const channels = Array.from({ length: 8 }, () => new Int32Array(totalFrames));
  const raw = new Uint8Array(buf, dataOffset, dataSize);

  for (let f = 0; f < totalFrames; f++) {
    const base = f * 24;
    for (let c = 0; c < 8; c++) {
      const b = base + c * 3;
      let s = raw[b] | (raw[b + 1] << 8) | (raw[b + 2] << 16);
      if (s & 0x800000) s |= 0xFF000000;
      channels[c][f] = s;
    }
  }

  return { fmt, channels, totalFrames };
}

function encodeToSP1(channels, totalFrames, bpm = 80) {
  const totalSectors = Math.ceil(totalFrames / FRAMES_PER_SECTOR);
  const output = new Uint8Array(totalSectors * SECTOR_SIZE);
  if (bpm === 0) bpm = 1;
  const tempo = (48000 * 60) / (24 * bpm);

  for (let s = 0; s < totalSectors; s++) {
    const sectorBase = s * SECTOR_SIZE;
    const sectorFrameStart = s * FRAMES_PER_SECTOR;
    const envelopes = [0, 0, 0, 0];

    for (let frame = 0; frame < FRAMES_PER_SECTOR; frame++) {
      const globalFrame = sectorFrameStart + frame;
      if (globalFrame >= totalFrames) break;
      const blockId = BLOCK_ORDER[frame % 4];
      const byteOffset = sectorBase + BLOCK_SIZE * blockId + FRAME_SIZE * Math.floor(frame / 4);

      for (let stem = 0; stem < 4; stem++) {
        const L = channels[stem * 2][globalFrame];
        const R = channels[stem * 2 + 1][globalFrame];
        const base = byteOffset + stem * STEM_FRAME_SIZE;
        envelopes[stem] = Math.max(Math.max(envelopes[stem], L), R);
        output[base]     = (L >> 8) & 0xFF;
        output[base + 1] = (L >> 16) & 0xFF;
        output[base + 2] = (R >> 16) & 0xFF;
        output[base + 3] = L & 0xFF;
        output[base + 4] = R & 0xFF;
        output[base + 5] = (R >> 8) & 0xFF;
      }
    }

    for (let i = 0; i < envelopes.length; i++) {
      output[sectorBase + 2044 + i] = 255 * envelopes[i] / 0x800000;
    }
    output[sectorBase + 2042] = tempo & 0xFF;
    output[sectorBase + 2043] = (tempo >> 8) & 0xFF;
  }

  return output;
}
