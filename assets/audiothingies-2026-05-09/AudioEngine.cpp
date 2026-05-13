#include "AudioEngine.hpp"

#include <algorithm>
#include <cmath>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(audio_engine, LOG_LEVEL_INF);

namespace audio {

namespace {

// OG-style master volume should provide the audible level shaping. Keep max at
// unity and use a cubic response across the discrete steps.
constexpr std::array<float, 9> kOutputGainSteps = {
    0.0f,
    0.001953125f,
    0.015625f,
    0.052734375f,
    0.125f,
    0.244140625f,
    0.421875f,
    0.669921875f,
    1.0f,
};
constexpr float kPlaybackMixGain = 1.0f;

constexpr float kSlowPlaybackRate = 0.5f;
constexpr float kPauseSettledSpeed = 0.001f;
constexpr float kUnityRelockSpeedError = 0.001f;
constexpr size_t kScanEntrySeedSlots = 4U;

constexpr TransportRenderMode to_transport_render_mode(AudioEngine::TransportMode mode) {
    switch (mode) {
    case AudioEngine::TransportMode::Play:
        return TransportRenderMode::Play;
    case AudioEngine::TransportMode::Slow:
        return TransportRenderMode::Slow;
    case AudioEngine::TransportMode::FastForward:
        return TransportRenderMode::FastForward;
    case AudioEngine::TransportMode::Rewind:
        return TransportRenderMode::Rewind;
    }
    return TransportRenderMode::Play;
}
}  // namespace

bool AudioEngine::init(storage::DiskManager* disk_manager) {
    if (disk_manager == nullptr) {
        return false;
    }

    disk_manager_ = disk_manager;
    stock_mixer_.set_disk_manager(disk_manager_);
    stock_mixer_.set_dual_mono(true);
    observed_speed_.store(1.0f, std::memory_order_relaxed);
    return true;
}

AudioEngine::PerfStats AudioEngine::snapshot_perf_stats() const {
    const k_spinlock_key_t key = k_spin_lock(&perf_lock_);
    const PerfStats snapshot = perf_stats_;
    k_spin_unlock(&perf_lock_, key);
    return snapshot;
}

StockRuntimeMixer::PerfStats AudioEngine::snapshot_mixer_perf_stats() const {
    return stock_mixer_.snapshot_perf_stats();
}

void AudioEngine::reset_perf_stats() {
    const k_spinlock_key_t key = k_spin_lock(&perf_lock_);
    perf_stats_ = PerfStats{};
    k_spin_unlock(&perf_lock_, key);
    stock_mixer_.reset_perf_stats();
}

void AudioEngine::start_playback(uint32_t start_frame) {
    if (start_frame == 0U && disk_manager_ != nullptr) {
        const uint8_t selected_song = disk_manager_->current_song();
        if (selected_song > 0U) {
            start_frame = disk_manager_->song_start_frame(selected_song);
        }
    }
    transport_mode_.store(TransportMode::Play, std::memory_order_relaxed);
    paused_.store(false, std::memory_order_relaxed);
    pending_unity_relock_.store(false, std::memory_order_relaxed);
    mixer_reset_transport_frame_.store(start_frame, std::memory_order_relaxed);
    mixer_reset_transport_pending_.store(true, std::memory_order_release);
    target_immediate_pending_.store(true, std::memory_order_release);
    last_applied_mix_gain_ = -1.0f;
    set_all_stem_weights_q12(0);
    update_transport_target(true);
    produced_frame_.store(start_frame, std::memory_order_relaxed);
    consumed_frame_.store(start_frame, std::memory_order_relaxed);
    observed_speed_.store(1.0f, std::memory_order_relaxed);
    sync_word_.store(0U, std::memory_order_relaxed);
    led_word_.store(0U, std::memory_order_relaxed);
    if (disk_manager_ != nullptr) {
        const bool scan_active = false;
        disk_manager_->start_streaming(start_frame,
                                       transport_reversing(),
                                       scan_active,
                                       storage::kEmmcBlocksPerSector);
        disk_manager_->seed_playback_window_sync(start_frame, 5U);
    }
    playing_.store(true, std::memory_order_release);
}

void AudioEngine::stop_playback() {
    playing_.store(false, std::memory_order_release);
    paused_.store(false, std::memory_order_relaxed);
    pending_unity_relock_.store(false, std::memory_order_relaxed);
    mixer_reset_transport_pending_.store(true, std::memory_order_release);
    observed_speed_.store(0.0f, std::memory_order_relaxed);
    sync_word_.store(0U, std::memory_order_relaxed);
    led_word_.store(0U, std::memory_order_relaxed);
    if (disk_manager_ != nullptr) {
        disk_manager_->stop_streaming();
    }
}

bool AudioEngine::playing() const {
    return playing_.load(std::memory_order_acquire);
}

void AudioEngine::pause_playback() {
    if (!playing()) {
        return;
    }
    paused_.store(true, std::memory_order_relaxed);
    pending_unity_relock_.store(true, std::memory_order_relaxed);
}

void AudioEngine::resume_playback() {
    if (!playing()) {
        return;
    }
    paused_.store(false, std::memory_order_relaxed);
}

bool AudioEngine::paused() const {
    return paused_.load(std::memory_order_relaxed);
}

bool AudioEngine::transport_reversing() const {
    return transport_mode() == TransportMode::Rewind;
}

void AudioEngine::rebase_transport(uint32_t frame_index, bool reseed_cache) {
    mixer_rebase_frame_.store(frame_index, std::memory_order_relaxed);
    mixer_rebase_pending_.store(true, std::memory_order_release);
    produced_frame_.store(frame_index, std::memory_order_relaxed);
    consumed_frame_.store(frame_index, std::memory_order_relaxed);
    if (reseed_cache && disk_manager_ != nullptr) {
        const bool is_paused = paused_.load(std::memory_order_relaxed);
        const auto mode = transport_mode_.load(std::memory_order_relaxed);
        const bool paused_holds_transport =
            is_paused && (mode == TransportMode::Play || mode == TransportMode::Slow);
        const float target_speed =
            paused_holds_transport ? 0.0f : target_speed_for_mode(mode);
        const bool scan_active =
            mode == TransportMode::FastForward || mode == TransportMode::Rewind;
        disk_manager_->rebase_streaming(frame_index,
                                        target_speed < 0.0f,
                                        scan_active,
                                        read_blocks_for_transport(mode, target_speed));
    }
}

void AudioEngine::apply_pending_mixer_state() {
    if (mixer_reset_transport_pending_.exchange(false, std::memory_order_acq_rel)) {
        const uint32_t frame = mixer_reset_transport_frame_.load(std::memory_order_relaxed);
        stock_mixer_.reset_transport(frame);
        return;
    }

    if (mixer_rebase_pending_.exchange(false, std::memory_order_acq_rel)) {
        const uint32_t frame = mixer_rebase_frame_.load(std::memory_order_relaxed);
        stock_mixer_.rebase_transport(frame);
    }
}

void AudioEngine::update_transport_target(bool immediate) {
    if (immediate) {
        target_immediate_pending_.store(true, std::memory_order_release);
    }
}

void AudioEngine::set_transport_mode(TransportMode mode, bool immediate) {
    const auto previous_mode = transport_mode_.load(std::memory_order_relaxed);
    transport_mode_.store(mode, std::memory_order_relaxed);
    update_transport_target(immediate);
    if (disk_manager_ == nullptr || !playing()) {
        return;
    }

    const uint32_t frame_index = current_consumed_frame();
    const bool paused_holds_transport =
        paused() && (mode == TransportMode::Play || mode == TransportMode::Slow);
    const float target_speed =
        paused_holds_transport ? 0.0f : target_speed_for_mode(mode);
    const bool reverse = target_speed < 0.0f;
    const bool scan_active =
        mode == TransportMode::FastForward || mode == TransportMode::Rewind;
    const uint32_t read_blocks = read_blocks_for_transport(mode, target_speed);
    disk_manager_->update_streaming_hint(frame_index, reverse, scan_active, read_blocks);

    const bool entering_scan =
        (mode == TransportMode::FastForward || mode == TransportMode::Rewind) &&
        mode != previous_mode;
    if (entering_scan) {
        LOG_INF("transport_mode enter mode=%u frame=%u consumed=%u target_speed=%.3f immediate=%d read_blocks=%u",
                static_cast<unsigned>(mode),
                current_produced_frame(),
                current_consumed_frame(),
                static_cast<double>(target_speed),
                immediate ? 1 : 0,
                read_blocks);
        disk_manager_->seed_playback_window_sync(
            frame_index,
            std::min<size_t>(kScanEntrySeedSlots, storage::kAudioPrefetchSlots));
    } else if ((previous_mode == TransportMode::FastForward ||
                previous_mode == TransportMode::Rewind) &&
               mode != previous_mode) {
        LOG_INF("transport_mode exit mode=%u frame=%u consumed=%u target_speed=%.3f immediate=%d read_blocks=%u",
                static_cast<unsigned>(mode),
                current_produced_frame(),
                current_consumed_frame(),
                static_cast<double>(target_speed),
                immediate ? 1 : 0,
                read_blocks);
    }
}

AudioEngine::TransportMode AudioEngine::transport_mode() const {
    return transport_mode_.load(std::memory_order_relaxed);
}

bool AudioEngine::step_transport_rate(int delta) {
    std::atomic<uint8_t>* rate_index = nullptr;
    switch (transport_mode()) {
    case TransportMode::FastForward:
        rate_index = &ff_rate_index_;
        break;
    case TransportMode::Rewind:
        rate_index = &rw_rate_index_;
        break;
    case TransportMode::Play:
    case TransportMode::Slow:
        return false;
    }

    int current = rate_index->load(std::memory_order_relaxed);
    const int min_step = 0;
    const int max_step = static_cast<int>(kFastForwardRates.size() - 1U);
    const int next = std::clamp(current + delta, min_step, max_step);
    if (next == current) {
        return false;
    }
    rate_index->store(static_cast<uint8_t>(next), std::memory_order_relaxed);
    update_transport_target(false);
    return true;
}

uint8_t AudioEngine::transport_rate_index() const {
    switch (transport_mode()) {
    case TransportMode::FastForward:
        return ff_rate_index_.load(std::memory_order_relaxed);
    case TransportMode::Rewind:
        return rw_rate_index_.load(std::memory_order_relaxed);
    case TransportMode::Play:
    case TransportMode::Slow:
        return 0U;
    }
    return 0U;
}

uint32_t AudioEngine::current_transport_frame() const {
    if (disk_manager_ == nullptr) {
        return 0U;
    }
    return stock_mixer_.current_frame_index_wrapped(disk_manager_->total_frames());
}

float AudioEngine::current_speed() const {
    return observed_speed_.load(std::memory_order_relaxed);
}

float AudioEngine::target_speed_for_mode(TransportMode mode) const {
    switch (mode) {
    case TransportMode::Play:
        return 1.0f;
    case TransportMode::Slow:
        return kSlowPlaybackRate;
    case TransportMode::FastForward: {
        const uint8_t rate_index = std::min<uint8_t>(
            ff_rate_index_.load(std::memory_order_relaxed),
            static_cast<uint8_t>(kFastForwardRates.size() - 1U));
        return kFastForwardRates[rate_index];
    }
    case TransportMode::Rewind: {
        const uint8_t rate_index = std::min<uint8_t>(
            rw_rate_index_.load(std::memory_order_relaxed),
            static_cast<uint8_t>(kRewindRates.size() - 1U));
        return -kRewindRates[rate_index];
    }
    }

    return 1.0f;
}

uint32_t AudioEngine::read_blocks_for_target_speed(float speed) const {
    return read_blocks_for_transport(transport_mode_.load(std::memory_order_relaxed), speed);
}

uint32_t AudioEngine::read_blocks_for_transport(TransportMode mode, float speed) const {
    const float speed_abs = std::fabs(speed);
    if (mode == TransportMode::FastForward || mode == TransportMode::Rewind) {
        if (speed_abs <= 1.1f) {
            return storage::kEmmcBlocksPerSector;
        }
        if (speed_abs <= 2.5f) {
            return 8U;
        }
        return 4U;
    }
    return StockRuntimeMixer::read_blocks_for_speed(speed);
}

void AudioEngine::set_stem_mix_weights_q12(
    const std::array<uint16_t, 4>& weights_q12) {
    for (size_t i = 0; i < weights_q12.size(); ++i) {
        stem_weights_q12_[i].store(weights_q12[i], std::memory_order_relaxed);
    }
}

void AudioEngine::set_all_stem_weights_q12(uint16_t weight_q12) {
    for (auto& weight : stem_weights_q12_) {
        weight.store(weight_q12, std::memory_order_relaxed);
    }
}

void AudioEngine::set_solo_mask(uint8_t solo_mask) {
    solo_mask_.store(static_cast<uint8_t>(solo_mask & 0x0FU), std::memory_order_relaxed);
}

bool AudioEngine::step_output_gain(int delta) {
    int current = output_gain_step_.load(std::memory_order_relaxed);
    const int min_step = 0;
    const int max_step = static_cast<int>(kOutputGainSteps.size() - 1U);
    const int next = std::clamp(current + delta, min_step, max_step);
    if (next == current) {
        return false;
    }
    output_gain_step_.store(static_cast<uint8_t>(next), std::memory_order_relaxed);
    return true;
}

float AudioEngine::current_output_gain() const {
    const size_t step = std::min<uint8_t>(
        output_gain_step_.load(std::memory_order_relaxed),
        static_cast<uint8_t>(kOutputGainSteps.size() - 1U));
    return kOutputGainSteps[step];
}

uint8_t AudioEngine::current_output_volume_level() const {
    const float gain = current_output_gain();
    const float max_gain = kOutputGainSteps.back();
    const float normalized =
        max_gain > 0.0f ? std::clamp(gain / max_gain, 0.0f, 1.0f) : 0.0f;
    return static_cast<uint8_t>(std::clamp(
        static_cast<int>(std::lround(normalized * 3.0f)), 0, 3));
}

int AudioEngine::current_output_gain_percent() const {
    const float gain = current_output_gain();
    const float max_gain = kOutputGainSteps.back();
    const float normalized =
        max_gain > 0.0f ? std::clamp(gain / max_gain, 0.0f, 1.0f) : 0.0f;
    return std::clamp(static_cast<int>(std::lround(normalized * 100.0f)), 0, 100);
}

std::array<uint16_t, 4> AudioEngine::snapshot_stem_weights_q12() const {
    std::array<uint16_t, 4> weights{};
    for (size_t i = 0; i < weights.size(); ++i) {
        weights[i] = stem_weights_q12_[i].load(std::memory_order_relaxed);
    }
    const uint8_t solo_mask = solo_mask_.load(std::memory_order_relaxed);
    if (solo_mask != 0U) {
        for (size_t i = 0; i < weights.size(); ++i) {
            if ((solo_mask & (1U << i)) == 0U) {
                weights[i] = 0U;
            }
        }
    }
    return weights;
}

void AudioEngine::render_block(int32_t* output,
                               size_t word_count) {
    if (output == nullptr) {
        return;
    }

    const uint32_t render_start = k_cycle_get_32();
    storage::DiskManager* const disk = disk_manager_;
    const bool is_playing = playing_.load(std::memory_order_acquire);
    const bool is_paused = paused_.load(std::memory_order_relaxed);
    const bool pending_unity_relock =
        pending_unity_relock_.load(std::memory_order_relaxed);
    const auto mode = transport_mode_.load(std::memory_order_relaxed);
    const float output_gain = current_output_gain();
    const auto stem_weights = snapshot_stem_weights_q12();

    if (!is_playing || disk == nullptr) {
        std::fill_n(output, word_count, 0);
        return;
    }

    apply_pending_mixer_state();
    const float current_speed = stock_mixer_.current_speed();
    const bool paused_holds_transport =
        is_paused &&
        (mode == TransportMode::Play || mode == TransportMode::Slow);
    if (paused_holds_transport && std::fabs(current_speed) <= kPauseSettledSpeed) {
        stock_mixer_.set_target_speed(0.0f, true);
        observed_speed_.store(0.0f, std::memory_order_relaxed);
        std::fill_n(output, word_count, 0);
        return;
    }

    const float desired_mix_gain = kPlaybackMixGain * output_gain;
    if (desired_mix_gain != last_applied_mix_gain_) {
        stock_mixer_.set_mix_gain(desired_mix_gain);
        last_applied_mix_gain_ = desired_mix_gain;
    }

    const float target_speed =
        paused_holds_transport ? 0.0f : target_speed_for_mode(mode);
    const bool immediate_target =
        target_immediate_pending_.exchange(false, std::memory_order_acq_rel);
    stock_mixer_.set_target_speed(target_speed, immediate_target);

    auto render_result =
        stock_mixer_.render_block(output,
                                  word_count,
                                  to_transport_render_mode(mode),
                                  stem_weights);

    const float new_speed = stock_mixer_.current_speed();
    render_result.read_blocks = read_blocks_for_transport(mode, new_speed);

    disk->update_streaming_hint(render_result.wrapped_end_frame,
                                render_result.reverse,
                                mode == TransportMode::FastForward ||
                                    mode == TransportMode::Rewind,
                                render_result.read_blocks);
    produced_frame_.store(render_result.wrapped_end_frame, std::memory_order_relaxed);
    observed_speed_.store(new_speed, std::memory_order_relaxed);

    if (!is_paused &&
        (mode == TransportMode::FastForward || mode == TransportMode::Rewind)) {
        const uint32_t now_ms = k_uptime_get_32();
        if (now_ms - last_scan_debug_log_ms_ >= 500U) {
            last_scan_debug_log_ms_ = now_ms;
            LOG_INF("transport_scan mode=%u target=%.3f current=%.3f produced=%u consumed=%u read_blocks=%u",
                    static_cast<unsigned>(mode),
                    static_cast<double>(target_speed),
                    static_cast<double>(new_speed),
                    render_result.wrapped_end_frame,
                    current_consumed_frame(),
                    render_result.read_blocks);
        }
    }

    if (pending_unity_relock &&
        !is_paused &&
        mode == TransportMode::Play &&
        std::fabs(new_speed - 1.0f) <= kUnityRelockSpeedError) {
        stock_mixer_.rebase_transport(render_result.wrapped_end_frame);
        observed_speed_.store(stock_mixer_.current_speed(), std::memory_order_relaxed);
        pending_unity_relock_.store(false, std::memory_order_relaxed);
    }
    const uint32_t render_cycles = k_cycle_get_32() - render_start;
    const k_spinlock_key_t key = k_spin_lock(&perf_lock_);
    ++perf_stats_.render_count;
    perf_stats_.render_total_cycles += render_cycles;
    perf_stats_.render_max_cycles = std::max(perf_stats_.render_max_cycles, render_cycles);
    k_spin_unlock(&perf_lock_, key);
}

uint32_t AudioEngine::current_produced_frame() const {
    return produced_frame_.load(std::memory_order_relaxed);
}

void AudioEngine::on_buffer_consumed(uint32_t consumed_frame) {
    consumed_frame_.store(consumed_frame, std::memory_order_relaxed);
    if (disk_manager_ != nullptr) {
        uint32_t sync_word = 0U;
        uint32_t led_word = 0U;
        if (disk_manager_->try_get_block_side_data_cached(consumed_frame,
                                                          &side_data_cached_sector_,
                                                          &side_data_cached_slot_index_,
                                                          &sync_word,
                                                          &led_word)) {
            sync_word_.store(sync_word, std::memory_order_relaxed);
            led_word_.store(led_word, std::memory_order_relaxed);
        }
    }
}

uint32_t AudioEngine::current_consumed_frame() const {
    return consumed_frame_.load(std::memory_order_relaxed);
}

uint32_t AudioEngine::current_track_led_word() const {
    return led_word_.load(std::memory_order_relaxed);
}

uint32_t AudioEngine::current_sync_word() const {
    return sync_word_.load(std::memory_order_relaxed);
}

}  // namespace audio
