#pragma once

#include <zephyr/device.h>
#include <zephyr/drivers/adc.h>
#include <zephyr/drivers/pwm.h>
#include <zephyr/usb/class/usbd_midi2.h>
#include <zephyr/audio/midi.h>

#include <array>
#include <atomic>
#include <cstdint>

namespace midi {

constexpr int kNumFaders = 4;
constexpr int kNumTrackLeds = 4;
constexpr int kFaderDeadband = 8;      // ~0.2% of 12-bit range
constexpr uint8_t kMidiChannel = 0;
constexpr uint8_t kMidiGroup = 0;

// CC assignments — all on channel 1
constexpr uint8_t kCcFader1 = 1;       // Faders → CC 1–4
constexpr uint8_t kCcVolume = 7;       // Volume up/down → CC 7 step

// Note assignments (track buttons still send notes for triggering)
constexpr uint8_t kNoteTrack1 = 60;    // Track buttons → notes 60–63

// MMC (MIDI Machine Control) command bytes
// Full SysEx: F0 7F 7F 06 <cmd> F7
constexpr uint8_t kMmcStop = 0x01;
constexpr uint8_t kMmcPlay = 0x02;
constexpr uint8_t kMmcFastForward = 0x04;
constexpr uint8_t kMmcRewind = 0x05;
constexpr uint8_t kMmcRecordOn = 0x06;
constexpr uint8_t kMmcRecordOff = 0x07;
constexpr uint8_t kMmcPause = 0x09;

struct MidiController {
    const struct device* midi_dev = nullptr;

    // Track LED PWM specs (set by main)
    const struct pwm_dt_spec* track_led_pwm = nullptr;

    // Fader state
    std::array<int16_t, kNumFaders> last_fader_raw{};
    std::array<uint8_t, kNumFaders> last_fader_cc{};

    // Transport toggle state
    bool playing = false;
    bool recording = false;
    uint8_t volume_level = 100;

    // Octave offset for track button notes (rocker shifts ±1 octave)
    int8_t octave_offset = 0;  // -5 to +5 → note range 0–127

    // USB MIDI ready flag
    std::atomic<bool> ready{false};
    uint32_t poll_count_ = 0;

    // ADC specs for the 4 faders
    std::array<adc_dt_spec, kNumFaders> fader_adc{};

    void send_cc(uint8_t cc, uint8_t value);
    void send_note(uint8_t note, bool on, uint8_t velocity = 127);
    void send_mmc(uint8_t command);
    void poll_faders();
    void set_track_led(int index, uint8_t brightness);
    void on_rx(const struct midi_ump& ump);
};

}  // namespace midi
