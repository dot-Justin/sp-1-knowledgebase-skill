#pragma once

#include <cstdint>
#include <cstddef>

namespace audio {

/**
 * Base interface for any node in the audio processing graph.
 * Uses a pull-based model where downstream nodes request samples from upstream.
 */
class AudioNode {
public:
    virtual ~AudioNode() = default;

    /**
     * Process and fill the output buffer with the requested number of samples.
     * 
     * @param out_buffer Pointer to the float array to be filled (interleaved stereo or mono).
     * @param num_samples Number of samples to generate.
     */
    virtual void process(float* out_buffer, size_t num_samples) = 0;
};

} // namespace audio
