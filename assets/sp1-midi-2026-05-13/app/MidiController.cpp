#include "MidiController.hpp"

#include <zephyr/logging/log.h>
#include <algorithm>
#include <cmath>

LOG_MODULE_REGISTER(midi_ctrl, LOG_LEVEL_INF);

namespace midi {

void MidiController::send_cc(uint8_t cc, uint8_t value) {
    if (!ready.load(std::memory_order_relaxed) || midi_dev == nullptr) {
        return;
    }
    const struct midi_ump ump = UMP_MIDI1_CHANNEL_VOICE(
        kMidiGroup, UMP_MIDI_CONTROL_CHANGE, kMidiChannel, cc, value);
    usbd_midi_send(midi_dev, ump);
}

void MidiController::send_note(uint8_t note, bool on, uint8_t velocity) {
    if (!ready.load(std::memory_order_relaxed) || midi_dev == nullptr) {
        return;
    }
    const uint8_t cmd = on ? UMP_MIDI_NOTE_ON : UMP_MIDI_NOTE_OFF;
    const struct midi_ump ump = UMP_MIDI1_CHANNEL_VOICE(
        kMidiGroup, cmd, kMidiChannel, note, on ? velocity : 0);
    usbd_midi_send(midi_dev, ump);
}

void MidiController::send_mmc(uint8_t command) {
    if (!ready.load(std::memory_order_relaxed) || midi_dev == nullptr) {
        return;
    }

    // MMC SysEx: F0 7F 7F 06 <cmd> F7
    // In UMP MT=3 (Data Message 64-bit):
    //   Word 0: [MT=3][Group][Status=0(complete)][NumBytes=4][0x7F][0x7F]
    //   Word 1: [0x06][cmd][0x00][0x00]
    const struct midi_ump ump = {.data = {
        (UMP_MT_DATA_64 << 28)
        | (static_cast<uint32_t>(kMidiGroup) << 24)
        | (0x0U << 20)           // status = complete SysEx
        | (0x4U << 16)           // 4 payload bytes
        | (0x7FU << 8)           // byte 0: universal real-time
        | 0x7FU,                 // byte 1: all devices
        (0x06U << 24)            // byte 2: MMC command
        | (static_cast<uint32_t>(command) << 16) // byte 3: sub-command
    }};
    usbd_midi_send(midi_dev, ump);
}

void MidiController::poll_faders() {
    for (int i = 0; i < kNumFaders; ++i) {
        int16_t sample = 0;
        struct adc_sequence seq = {};
        (void)adc_sequence_init_dt(&fader_adc[i], &seq);
        seq.buffer = &sample;
        seq.buffer_size = sizeof(sample);

        const int ret = adc_read_dt(&fader_adc[i], &seq);
        if (ret < 0) {
            if (poll_count_ < 3) {
                LOG_WRN("Fader %d ADC read failed: %d", i, ret);
            }
            continue;
        }

        sample = static_cast<int16_t>(std::clamp<int>(sample, 0, 0x0FF0));

        if (poll_count_ < 3) {
            LOG_INF("Fader %d: raw=%d cc=%d", i, sample, sample >> 5);
        }

        if (std::abs(sample - last_fader_raw[i]) < kFaderDeadband) {
            continue;
        }

        last_fader_raw[i] = sample;

        const uint8_t cc_val = static_cast<uint8_t>(
            std::clamp<int>(sample >> 5, 0, 127));

        if (cc_val != last_fader_cc[i]) {
            last_fader_cc[i] = cc_val;
            send_cc(kCcFader1 + static_cast<uint8_t>(i), cc_val);
        }
    }
    ++poll_count_;
}

void MidiController::set_track_led(int index, uint8_t brightness) {
    if (track_led_pwm == nullptr || index < 0 || index >= kNumTrackLeds) {
        return;
    }
    const struct pwm_dt_spec* spec = &track_led_pwm[index];
    const uint32_t pulse = (spec->period * brightness) / 127U;
    pwm_set_pulse_dt(spec, pulse);
}

void MidiController::on_rx(const struct midi_ump& ump) {
    if (UMP_MT(ump) != UMP_MT_MIDI1_CHANNEL_VOICE) {
        return;
    }

    const uint8_t cmd = UMP_MIDI_COMMAND(ump);
    const uint8_t note = UMP_MIDI1_P1(ump);
    const uint8_t vel = UMP_MIDI1_P2(ump);

    if (note >= kNoteTrack1 && note < kNoteTrack1 + kNumTrackLeds) {
        const int led = note - kNoteTrack1;
        if (cmd == UMP_MIDI_NOTE_ON) {
            set_track_led(led, vel);
        } else if (cmd == UMP_MIDI_NOTE_OFF) {
            set_track_led(led, 0);
        }
    }
}

}  // namespace midi
