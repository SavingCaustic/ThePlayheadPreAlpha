#include "Rotator.h"
#include <cmath>

Rotator::Rotator() : pulse(0.0f), pulsesPerFrame(0.0f) {
    // Initializing pulse and pulsesPerFrame
}

Rotator::~Rotator() {}

void Rotator::reset() {
    pulse = 0.0f;
}

void Rotator::setTempo(int bpm, bool dotted) {
    float eightsPerSec = (bpm / 60.0f) * 2;
    if (dotted) {
        // Correct calculation of 6/8, 9/8 etc.
        eightsPerSec *= 1.5f;
    }
    setPulsesPerFrame(eightsPerSec);
}

bool Rotator::frameTurn() {
    // Called *after* render actions in playerEngine.
    // Returns true if we're entering a new eighth note.
    pulse = std::round((pulse + pulsesPerFrame) * 1000) / 1000;
    if (pulse >= 12 * TPH_TICKS_PER_CLOCK) {
        pulse -= 12 * TPH_TICKS_PER_CLOCK;
        return true;
    } else {
        return false;
    }
}

void Rotator::setPulsesPerFrame(float eps) {
    pulsesPerFrame = eps * TPH_RACK_RENDER_SIZE / TPH_AUDIO_SR * TPH_TICKS_PER_CLOCK * 12; // 12 not 24 since /8
}
