#include "MultiFilter.h"

#include "constants.h"
#include <algorithm>
#include <cassert>
#include <math.h>

namespace audio::filter {

MultiFilter::MultiFilter() {
    // DSP_SR not needed here - what is?
}

void MultiFilter::reset() {
    d1 = d2 = d3 = d4 = 0;
}

void MultiFilter::setFilterType(FilterType flt) {
    filterType = flt;
}

void MultiFilter::setCutoff(float inCutoff) {
    inCutoff = std::fmin(inCutoff, TPH_DSP_SR * 0.5f * 0.99f); // filter is unstable at PI
    inCutoff = std::fmax(inCutoff, 20.0f);
    filterCutoff = inCutoff;
}

void MultiFilter::setResonance(float inRes) {
    filterResonance = inRes;
}

void MultiFilter::setPoles(FilterPoles inPoles) {
    filterPoles = inPoles;
}

void MultiFilter::initFilter() {
    // all dependent on cutoff, half of resonance..
    const double w = (filterCutoff / TPH_DSP_SR);                    // cutoff freq [ 0 <= w <= 0.5 ]
    const double r = std::max(0.001, 2.0 * (1.0 - filterResonance)); // r is 1/Q (sqrt(2) for a butterworth response)

    const double k = tan(w * M_PI);
    const double k2 = k * k;
    const double rk = r * k;
    const double bh = 1.0 + rk + k2;

    switch (filterType) {
    case FilterType::lowPass:
        //
        // Bilinear transformation of H(s) = 1 / (s^2 + s/Q + 1)
        // See "Digital Audio Signal Processing" by Udo Zölzer
        //
        a0 = k2 / bh;
        a1 = a0 * 2.0;
        a2 = a0;
        b1 = (2.0 * (k2 - 1.0)) / bh;
        b2 = (1.0 - rk + k2) / bh;
        break;

    case FilterType::highPass:
        //
        // Bilinear transformation of H(s) = s^2 / (s^2 + s/Q + 1)
        // See "Digital Audio Signal Processing" by Udo Zölzer
        //
        a0 = 1.0 / bh;
        a1 = -2.0 / bh;
        a2 = a0;
        b1 = (2.0 * (k2 - 1.0)) / bh;
        b2 = (1.0 - rk + k2) / bh;
        break;

    case FilterType::bandPass:
        //
        // Bilinear transformation of H(s) = (s/Q) / (s^2 + s/Q + 1)
        // See "Digital Audio Signal Processing" by Udo Zölzer
        //
        a0 = rk / bh;
        a1 = 0.0;
        a2 = -rk / bh;
        b1 = (2.0 * (k2 - 1.0)) / bh;
        b2 = (1.0 - rk + k2) / bh;
        break;

    case FilterType::bandStop:
        //
        // "Digital Audio Signal Processing" by Udo Zölzer does not provide z-transform
        // coefficients for the bandstop filter, so these were derived by studying
        // http://www.earlevel.com/main/2012/11/26/biquad-c-source-code/
        //
        a0 = (1.0 + k2) / bh;
        a1 = (2.0 * (k2 - 1.0)) / bh;
        a2 = a0;
        b1 = a1;
        b2 = (1.0 - rk + k2) / bh;
        break;

    case FilterType::bypass:
        return;

    default:
        assert(nullptr == "invalid FilterType");
        return;
    }
}

// we need to know if this is stereo or mono.. always momo for now..
void MultiFilter::processBlock(float *buffer, int numSamples) {
    if (filterType == FilterType::bypass) {
        return;
    }
    double x, y;

    switch (filterPoles) {
    case FilterPoles::p2:
        for (int i = 0; i < numSamples; i++) {
            x = buffer[i];
            y = (a0 * x) + d1;
            d1 = d2 + (a1 * x) - (b1 * y);
            d2 = (a2 * x) - (b2 * y);
            buffer[i] = (float)y;
        }
        break;

    case FilterPoles::p4:
        for (int i = 0; i < numSamples; i++) {
            x = buffer[i];
            y = (a0 * x) + d1;
            d1 = d2 + (a1 * x) - (b1 * y);
            d2 = (a2 * x) - (b2 * y);
            x = y;
            y = (a0 * x) + d3;
            d3 = d4 + (a1 * x) - (b1 * y);
            d4 = (a2 * x) - (b2 * y);
            buffer[i] = (float)y;
        }
        break;

    default:
        assert(nullptr == "invalid FilterSlope");
        break;
    }
};

float MultiFilter::processSample(float sample, FilterPoles poles) {
    // yeah - we could add arguments later..
    if (filterType == FilterType::bypass) {
        return sample;
    }

    double x, y;
    switch (poles) {
    case FilterPoles::p2:
        x = sample;
        y = (a0 * x) + d1;
        d1 = d2 + (a1 * x) - (b1 * y);
        d2 = (a2 * x) - (b2 * y);
        return float(y);
        break;
    case FilterPoles::p4:
        x = sample;
        y = (a0 * x) + d1;
        d1 = d2 + (a1 * x) - (b1 * y);
        d2 = (a2 * x) - (b2 * y);
        x = y;
        y = (a0 * x) + d3;
        d3 = d4 + (a1 * x) - (b1 * y);
        d4 = (a2 * x) - (b2 * y);
        return float(y);
        break;
    default:
        assert(nullptr == "invalid FilterSlope");
        break;
    }
};

} // namespace audio::filter