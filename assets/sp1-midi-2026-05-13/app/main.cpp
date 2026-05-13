#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/adc.h>
#include <zephyr/drivers/pwm.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/input/input.h>
#include <zephyr/logging/log.h>
#include <zephyr/usb/usbd.h>
#include <zephyr/usb/class/usbd_midi2.h>
#include <zephyr/usb/bos.h>
#include <hal/nrf_power.h>

extern "C" {
#include <ump_stream_responder.h>
}

#include "AppStateMachine.hpp"
#include "Leds.hpp"
#include "MidiController.hpp"
#include "power/PowerManager.hpp"
#include "system/Watchdog.hpp"

LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

// ── USB composite device (CDC ACM + MIDI) ───────────────────────

#define SP_USB_VID  0x1209
#define SP_USB_PID  0x0001

USBD_DEVICE_DEFINE(sp_usbd,
                   DEVICE_DT_GET(DT_NODELABEL(zephyr_udc0)),
                   SP_USB_VID, SP_USB_PID);

USBD_DESC_LANG_DEFINE(sp_lang);
USBD_DESC_MANUFACTURER_DEFINE(sp_mfr, "Teenage Engineering");
USBD_DESC_PRODUCT_DEFINE(sp_product, "SP-1 MIDI Controller");
USBD_DESC_CONFIG_DEFINE(sp_fs_cfg, "FS Configuration");

USBD_CONFIGURATION_DEFINE(sp_fs_config, 0, 125, &sp_fs_cfg);

static struct usbd_context *init_usb(void) {
    int err;

    printk("USB: adding descriptors\r\n");
    err = usbd_add_descriptor(&sp_usbd, &sp_lang);
    if (err) { printk("USB lang desc: %d\r\n", err); return nullptr; }

    err = usbd_add_descriptor(&sp_usbd, &sp_mfr);
    if (err) { printk("USB mfr desc: %d\r\n", err); return nullptr; }

    err = usbd_add_descriptor(&sp_usbd, &sp_product);
    if (err) { printk("USB product desc: %d\r\n", err); return nullptr; }

    printk("USB: adding FS configuration\r\n");
    err = usbd_add_configuration(&sp_usbd, USBD_SPEED_FS, &sp_fs_config);
    if (err) { printk("USB config: %d\r\n", err); return nullptr; }

    printk("USB: registering all classes\r\n");
    err = usbd_register_all_classes(&sp_usbd, USBD_SPEED_FS, 1, nullptr);
    if (err) { printk("USB register classes: %d\r\n", err); return nullptr; }

    // Composite device: use IAD triple (Misc/0x02/0x01)
    usbd_device_set_code_triple(&sp_usbd, USBD_SPEED_FS,
                                USB_BCC_MISCELLANEOUS, 0x02, 0x01);

    printk("USB: init\r\n");
    err = usbd_init(&sp_usbd);
    if (err) { printk("USB init: %d\r\n", err); return nullptr; }

    printk("USB: enable\r\n");
    err = usbd_enable(&sp_usbd);
    if (err) { printk("USB enable: %d\r\n", err); return nullptr; }

    printk("USB: composite device ready\r\n");
    return &sp_usbd;
}

// ── USB MIDI device from device tree ────────────────────────────

#define USB_MIDI_DT_NODE DT_NODELABEL(usb_midi)
static const struct device *const midi_dev = DEVICE_DT_GET(USB_MIDI_DT_NODE);

// ── Fader ADC channels ──────────────────────────────────────────

static const struct adc_dt_spec fader1_adc = ADC_DT_SPEC_GET_BY_NAME(DT_PATH(zephyr_user), fader1);
static const struct adc_dt_spec fader2_adc = ADC_DT_SPEC_GET_BY_NAME(DT_PATH(zephyr_user), fader2);
static const struct adc_dt_spec fader3_adc = ADC_DT_SPEC_GET_BY_NAME(DT_PATH(zephyr_user), fader3);
static const struct adc_dt_spec fader4_adc = ADC_DT_SPEC_GET_BY_NAME(DT_PATH(zephyr_user), fader4);

// ── Globals ─────────────────────────────────────────────────────

static midi::MidiController g_midi;
static leds::Leds g_leds;
static power::PowerManager g_power;

// ── UMP Stream Responder ────────────────────────────────────────

static const struct ump_endpoint_dt_spec ump_ep_dt =
    UMP_ENDPOINT_DT_SPEC_GET(USB_MIDI_DT_NODE);

static const struct ump_stream_responder_cfg responder_cfg =
    UMP_STREAM_RESPONDER(midi_dev, usbd_midi_send, &ump_ep_dt);

// ── USB MIDI callbacks ──────────────────────────────────────────

static void on_midi_rx(const struct device*, const struct midi_ump ump) {
    if (UMP_MT(ump) == UMP_MT_UMP_STREAM) {
        ump_stream_respond(&responder_cfg, ump);
        return;
    }
    g_midi.on_rx(ump);
}

static void on_midi_ready(const struct device*, const bool ready) {
    LOG_INF("USB MIDI %s", ready ? "ready" : "disconnected");
    g_midi.ready.store(ready, std::memory_order_relaxed);

    // Status LED: play LED 1
    g_leds.set_play(0, ready ? 128 : 0);
}

static const struct usbd_midi_ops midi_ops = {
    .rx_packet_cb = on_midi_rx,
    .ready_cb = on_midi_ready,
};

// ── Input subsystem callback (buttons → MIDI) ───────────────────

static void on_input_event(struct input_event *evt, void*) {
    if (evt == nullptr || evt->type != INPUT_EV_KEY) {
        return;
    }

    // Only act on press edges (not release) for toggle controls
    const bool pressed = (evt->value != 0);

    switch (evt->code) {
    // ── Track buttons → MIDI notes (offset by current octave) ────
    case INPUT_KEY_1:
    case INPUT_KEY_2:
    case INPUT_KEY_3:
    case INPUT_KEY_4: {
        const int base = midi::kNoteTrack1 + (evt->code - INPUT_KEY_1);
        const int note = base + (g_midi.octave_offset * 12);
        if (note >= 0 && note <= 127) {
            g_midi.send_note(static_cast<uint8_t>(note), pressed);
        }
        const int led_idx = evt->code - INPUT_KEY_1;
        if (led_idx < leds::kNumTrack) {
            g_leds.set_track(led_idx, pressed ? 200 : 0);
        }
        break;
    }

    // ── Play → MMC Play/Stop toggle ─────────────────────────────
    case INPUT_KEY_PLAY:
        if (pressed) {
            g_midi.playing = !g_midi.playing;
            g_midi.send_mmc(g_midi.playing ? midi::kMmcPlay : midi::kMmcStop);
        }
        break;

    // ── Rockers → octave shift ──────────────────────────────────
    case INPUT_KEY_FASTFORWARD:
        if (pressed && g_midi.octave_offset < 5) {
            g_midi.octave_offset++;
            LOG_INF("Octave: %+d (base C%d)",
                    g_midi.octave_offset,
                    (midi::kNoteTrack1 / 12) + g_midi.octave_offset - 1);
        }
        break;
    case INPUT_KEY_BACK:
        if (pressed && g_midi.octave_offset > -5) {
            g_midi.octave_offset--;
            LOG_INF("Octave: %+d (base C%d)",
                    g_midi.octave_offset,
                    (midi::kNoteTrack1 / 12) + g_midi.octave_offset - 1);
        }
        break;

    // ── Volume up/down → CC 7 step ──────────────────────────────
    case INPUT_KEY_VOLUMEUP:
        if (pressed && g_midi.volume_level < 127) {
            g_midi.volume_level = static_cast<uint8_t>(
                std::min<int>(g_midi.volume_level + 10, 127));
            g_midi.send_cc(midi::kCcVolume, g_midi.volume_level);
        }
        break;
    case INPUT_KEY_VOLUMEDOWN:
        if (pressed && g_midi.volume_level > 0) {
            g_midi.volume_level = static_cast<uint8_t>(
                std::max<int>(g_midi.volume_level - 10, 0));
            g_midi.send_cc(midi::kCcVolume, g_midi.volume_level);
        }
        break;

    // ── Function → MMC Record on/off toggle ─────────────────────
    case INPUT_KEY_MENU:
        if (pressed) {
            g_midi.recording = !g_midi.recording;
            g_midi.send_mmc(g_midi.recording
                ? midi::kMmcRecordOn : midi::kMmcRecordOff);
        }
        break;

    default:
        break;
    }
}
INPUT_CALLBACK_DEFINE(NULL, on_input_event, NULL);

// ── main ────────────────────────────────────────────────────────

int main(void) {
    core::watchdog::start();
    const int main_wdt = core::watchdog::register_channel("main", 5000);
    core::watchdog::feed(main_wdt);

    LOG_INF("Stem Player MIDI Controller Booting...");

    // ── USB composite device (CDC ACM console + MIDI) ───────────
    if (init_usb() == nullptr) {
        LOG_ERR("USB init failed");
    } else {
        LOG_INF("USB composite device ready (CDC ACM + MIDI)");
    }
    core::watchdog::feed(main_wdt);

    // ── LEDs ────────────────────────────────────────────────────
    g_leds.track[0] = PWM_DT_SPEC_GET(DT_NODELABEL(led_track_1));
    g_leds.track[1] = PWM_DT_SPEC_GET(DT_NODELABEL(led_track_2));
    g_leds.track[2] = PWM_DT_SPEC_GET(DT_NODELABEL(led_track_3));
    g_leds.track[3] = PWM_DT_SPEC_GET(DT_NODELABEL(led_track_4));
    g_leds.play[0] = PWM_DT_SPEC_GET(DT_NODELABEL(led_play_1));
    g_leds.play[1] = PWM_DT_SPEC_GET(DT_NODELABEL(led_play_2));
    g_leds.play[2] = PWM_DT_SPEC_GET(DT_NODELABEL(led_play_3));
    g_leds.play[3] = PWM_DT_SPEC_GET(DT_NODELABEL(led_play_4));
    g_leds.all_off();
    core::watchdog::feed(main_wdt);

    // ── Power monitor ───────────────────────────────────────────
    g_power.init();
    core::watchdog::feed(main_wdt);

    // ── Fader ADC ───────────────────────────────────────────────
    for (const auto* spec : {&fader1_adc, &fader2_adc, &fader3_adc, &fader4_adc}) {
        if (adc_is_ready_dt(spec)) {
            adc_channel_setup_dt(spec);
        }
    }
    core::watchdog::feed(main_wdt);

    // ── MIDI controller ─────────────────────────────────────────
    g_midi.midi_dev = midi_dev;
    g_midi.track_led_pwm = g_leds.track.data();
    g_midi.fader_adc = {fader1_adc, fader2_adc, fader3_adc, fader4_adc};

    if (device_is_ready(midi_dev)) {
        usbd_midi_set_ops(midi_dev, &midi_ops);
        LOG_INF("USB MIDI configured");
    } else {
        LOG_ERR("MIDI device not ready");
    }
    core::watchdog::feed(main_wdt);

    // ── Start HSM ───────────────────────────────────────────────
    app::AppMachine machine{};
    machine.midi = &g_midi;
    machine.leds = &g_leds;
    machine.power = &g_power;
    machine.main_watchdog_channel = main_wdt;

    LOG_INF("Boot complete, entering HSM");
    app::run(machine);

    return 0;
}
