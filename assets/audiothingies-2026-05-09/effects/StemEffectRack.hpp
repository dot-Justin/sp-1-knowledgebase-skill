#pragma once

#include "../../ui/UIState.hpp"
#include "BiquadFilter.hpp"
#include "ChorusFlangerNode.hpp"
#include "DelayEchoNode.hpp"
#include "DistortionNode.hpp"
#include "GateEnvelopeNode.hpp"
#include <zephyr/kernel.h>
#include <algorithm>

namespace audio {
namespace effects {

class StemEffectRack : public EffectNode {
private:
    enum class RoutePhase {
        Steady,
        FadeIn,
        FadeOut,
    };

    struct k_spinlock state_lock_;
    int stem_index_ = 0;
    ui::EffectState active_state_{false, false, ui::EffectType::FILTER, 0};
    ui::EffectState previous_state_ = active_state_;
    RoutePhase route_phase_ = RoutePhase::Steady;

    BiquadFilter biquad_;
    ChorusFlangerNode chorus_flanger_;
    DistortionNode distortion_;
    GateEnvelopeNode gate_envelope_;
    DelayEchoNode delay_echo_;

    static ui::EffectState sanitize(ui::EffectState state) {
        state.variation = std::clamp(state.variation, 0, 3);
        return state;
    }

    static bool same_effect(const ui::EffectState& lhs, const ui::EffectState& rhs) {
        return lhs.active == rhs.active &&
               lhs.type == rhs.type &&
               lhs.variation == rhs.variation;
    }

    static detail::TransitionPhase to_transition(RoutePhase phase) {
        switch (phase) {
        case RoutePhase::FadeIn:
            return detail::TransitionPhase::FadeIn;
        case RoutePhase::FadeOut:
            return detail::TransitionPhase::FadeOut;
        default:
            return detail::TransitionPhase::Steady;
        }
    }

    void apply_state(const ui::EffectState& state,
                     float* out_buffer,
                     size_t num_samples,
                     detail::TransitionPhase transition) {
        if (!state.active) {
            return;
        }

        switch (state.type) {
        case ui::EffectType::FILTER:
            biquad_.set_variation(state.variation);
            biquad_.apply_in_place(out_buffer, num_samples, transition);
            delay_echo_.apply_in_place(out_buffer, num_samples, 0, false, transition);
            break;
        case ui::EffectType::CHORUS_DELAY:
            if (state.variation < 2) {
                chorus_flanger_.set_variation(state.variation);
                chorus_flanger_.apply_in_place(out_buffer, num_samples, transition);
                delay_echo_.apply_in_place(out_buffer, num_samples, 0, false, transition);
            } else {
                delay_echo_.apply_in_place(
                    out_buffer, num_samples, state.variation, true, transition);
            }
            break;
        case ui::EffectType::DISTORTION:
            distortion_.set_variation(state.variation);
            distortion_.apply_in_place(out_buffer, num_samples, transition);
            delay_echo_.apply_in_place(out_buffer, num_samples, 0, false, transition);
            break;
        case ui::EffectType::GATE:
            gate_envelope_.set_variation(state.variation);
            gate_envelope_.apply_in_place(out_buffer, num_samples, transition);
            delay_echo_.apply_in_place(out_buffer, num_samples, 0, false, transition);
            break;
        }
    }

public:
    StemEffectRack() {
        set_sample_rate(detail::kDefaultSampleRate);
    }

    void set_sample_rate(float sample_rate) {
        biquad_.set_sample_rate(sample_rate);
        chorus_flanger_.set_sample_rate(sample_rate);
        distortion_.set_sample_rate(sample_rate);
    }

    void set_stem_index(int stem_index) {
        stem_index_ = stem_index;
        (void)stem_index_;
    }

    void set_effect_state(const ui::EffectState& effect_state) {
        const ui::EffectState sanitized = sanitize(effect_state);

        k_spinlock_key_t key = k_spin_lock(&state_lock_);
        if (same_effect(sanitized, active_state_)) {
            k_spin_unlock(&state_lock_, key);
            return;
        }
        previous_state_ = active_state_;
        active_state_ = sanitized;
        route_phase_ = previous_state_.active ? RoutePhase::FadeOut : RoutePhase::FadeIn;
        k_spin_unlock(&state_lock_, key);
    }

    const ui::EffectState& current_effect_state() const {
        return active_state_;
    }

    void process(float* out_buffer, size_t num_samples) override {
        pull_source_or_silence(out_buffer, num_samples);
        apply_effects_to_buffer(out_buffer, num_samples);
    }

    /// Process effects in-place on a pre-filled buffer (no pull from source).
    /// Use this when stem data has already been converted to float and written
    /// into the buffer (e.g., from StockRuntimeMixer's int32→float path).
    void process_in_place(float* buffer, size_t num_samples) {
        apply_effects_to_buffer(buffer, num_samples);
    }

private:
    void apply_effects_to_buffer(float* out_buffer, size_t num_samples) {
        // Snapshot effect state under spinlock to avoid tearing
        k_spinlock_key_t key = k_spin_lock(&state_lock_);
        ui::EffectState snap_active = active_state_;
        ui::EffectState snap_previous = previous_state_;
        RoutePhase snap_phase = route_phase_;
        // Advance phase under lock so set_effect_state sees consistent state
        switch (snap_phase) {
        case RoutePhase::FadeOut:
            route_phase_ = snap_active.active ? RoutePhase::FadeIn : RoutePhase::Steady;
            break;
        case RoutePhase::FadeIn:
            route_phase_ = RoutePhase::Steady;
            break;
        default:
            break;
        }
        k_spin_unlock(&state_lock_, key);

        // Apply effects outside lock (DSP is expensive)
        switch (snap_phase) {
        case RoutePhase::FadeOut:
            apply_state(snap_previous, out_buffer, num_samples,
                        detail::TransitionPhase::FadeOut);
            break;
        case RoutePhase::FadeIn:
            apply_state(snap_active, out_buffer, num_samples,
                        detail::TransitionPhase::FadeIn);
            break;
        case RoutePhase::Steady:
            apply_state(snap_active, out_buffer, num_samples,
                        detail::TransitionPhase::Steady);
            break;
        }
    }
};

}  // namespace effects
}  // namespace audio
