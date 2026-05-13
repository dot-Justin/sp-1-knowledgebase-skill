#pragma once

#include <algorithm>
#include <cstdint>

namespace audio {

#if defined(__GNUC__) || defined(__clang__)
#define AUDIO_FORCE_INLINE inline __attribute__((always_inline))
#else
#define AUDIO_FORCE_INLINE inline
#endif

AUDIO_FORCE_INLINE int32_t float_to_pcm_right24_fast(float sample) {
    if (!(sample <= 1.0f)) {
        sample = (sample < 0.0f) ? -1.0f : 1.0f;
    } else if (sample < -1.0f) {
        sample = -1.0f;
    }
    return static_cast<int32_t>(sample * 8388607.0f);
}

}  // namespace audio
