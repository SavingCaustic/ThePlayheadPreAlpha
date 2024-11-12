#pragma once

#include <cmath>
#include <constants.h>
#include <iostream>

namespace audio::filter {
// This filter isn't worth a penny and needs to be replaced.

enum FilterType {
    LPF, // Low Pass Filter
    BPF, // Band Pass Filter
    HPF  // High Pass Filter
};

class MultiFilter {
  public:
    void setFilterType(FilterType type) {
        filterType = type;
        initFilter();
    }

    void setCutoff(float frequency) {
        if (frequency > 0.0f) {
            cutoffHz = frequency;
            initFilter();
        } else {
            std::cerr << "Invalid cutoff frequency: " << frequency << std::endl;
        }
    }

    void setResonance(float res) {
        resonance = std::clamp(res, 0.0f, 1.0f); // Clamping resonance between 0 and 1
        initFilter();                            // Re-initialize the filter to apply the new resonance
    }

    void initFilter() {
        const float dt = 1.0f / TPH_DSP_SR;
        float RC, RC2;
        std::cout << "Initializing filter with cutoff frequency: " << cutoffHz
                  << " and resonance: " << resonance << std::endl;

        switch (filterType) {
        case LPF:
            RC = 1.0f / (2.0f * M_PI * cutoffHz);
            alpha = dt / (RC + dt);
            alpha *= 1.0f + resonance; // Adjust alpha by resonance factor for emphasis
            break;

        case BPF:
            RC = 1.0f / (2.0f * M_PI * cutoffHz);
            RC2 = 1.0f / (2.0f * M_PI * (cutoffHz * bandwidthFactor));
            alpha = dt / (RC + dt);
            alpha2 = dt / (RC2 + dt);
            alpha *= 1.0f + resonance; // Apply resonance to the low-pass part
            break;

        case HPF:
            RC = 1.0f / (2.0f * M_PI * cutoffHz);
            alpha = RC / (RC + dt);
            alpha *= 1.0f + resonance; // Apply resonance for a sharper high-pass cutoff
            break;
        }
    }

    void applyFilter(float &sample) {
        switch (filterType) {
        case LPF:
            sample = alpha * sample + (1.0f - alpha) * previousSample;
            previousSample = sample;
            break;

        case BPF:
            highPassedSample = alpha2 * (sample - previousSample) + (1.0f - alpha2) * highPassedSample;
            previousSample = sample;
            lowPassedSample = alpha * highPassedSample + (1.0f - alpha) * lowPassedSample;
            sample = lowPassedSample;
            break;

        case HPF:
            highPassedSample = alpha * (sample - previousSample + highPassedSample);
            previousSample = sample;
            sample = highPassedSample;
            break;
        }
    }

  private:
    FilterType filterType = LPF;
    float previousSample = 0.0f; // Renamed from previousLeft to previousSample
    float highPassedSample = 0.0f;
    float lowPassedSample = 0.0f;
    float alpha = 0.0f;
    float alpha2 = 0.0f;
    float cutoffHz = 8000.0f;
    float resonance = 0.0f;       // New resonance property
    float bandwidthFactor = 1.5f; // Adjustable bandwidth factor for BPF
};

} // namespace audio::filter
