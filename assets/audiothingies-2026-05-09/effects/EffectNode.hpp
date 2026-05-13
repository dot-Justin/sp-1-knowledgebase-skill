#pragma once

#include "../AudioNode.hpp"
#include <cstring>

namespace audio {

/**
 * Base class for DSP effects (Filters, Delays, etc.)
 * Effects sit between a source node and the destination, modifying the buffer in place.
 */
class EffectNode : public AudioNode {
protected:
    AudioNode* source = nullptr;

    void pull_source_or_silence(float* out_buffer, size_t num_samples) {
        if (source) {
            source->process(out_buffer, num_samples);
            return;
        }

        std::memset(out_buffer, 0, num_samples * sizeof(float));
    }

public:
    void set_source(AudioNode* src) {
        source = src;
    }

    // Subclasses must implement process()
    // Example:
    // void process(float* out_buffer, size_t num_samples) override {
    //     if (source) source->process(out_buffer, num_samples);
    //     // ... apply DSP to out_buffer ...
    // }
};

} // namespace audio
