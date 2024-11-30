#include "Synth/Sketch/SketchModel.h"
#include "core/audio/envelope/ADSFR.h"
#include <cmath>
#include <iostream>
#include <vector>

namespace Synth::Sketch {

Voice::Voice(Model &model)
    : modelRef(model),
      osc1(model.getLUT1()), // Initialize osc1 with LUT from model
      osc2(model.getLUT2())  // Initialize osc2 with LUT from model
{
    reset();
}

void Voice::reset() {
    // Called on setup
    notePlaying = 255; // unsigned byte. best like this..
}

void Voice::noteOn(uint8_t midiNote, float velocity) {
    // requested from voiceAllocate. maybe refactor..
    notePlaying = midiNote;
    noteVelocity = velocity;
    tracking = fmax(0, (2.0f + modelRef.senseTracking * AudioMath::noteToFloat(notePlaying) * 7));
    modelRef.vcaAR.triggerSlope(vcaARslope, audio::envelope::NOTE_ON);
}

void Voice::noteOff() {
    // enter release state in all envelopes.
    modelRef.vcaAR.triggerSlope(vcaARslope, audio::envelope::NOTE_OFF);
}

float Voice::getVCALevel() {
    return vcaARslope.currVal;
}

bool Voice::checkVoiceActive() {
    return vcaARslope.state != audio::envelope::ADSFRState::OFF;
}

bool Voice::renderNextVoiceBlock(std::size_t bufferSize) {
    float osc1hz, osc2hz;
    constexpr int chunkSize = 16; // Could be adapted to SIMD-capability..
    float oscMix = modelRef.getOscMix();
    modelRef.vcaAR.updateDelta(vcaARslope);
    float fmAmp = tracking * modelRef.fmSens * noteVelocity;
    float mixAmplitude = noteVelocity * 0.6f;
    if (vcaARslope.state != audio::envelope::OFF) {
        osc2hz = AudioMath::noteToHz(notePlaying + modelRef.semitone + modelRef.osc2octave * 12, modelRef.bendCents);
        osc2.setAngle(osc2hz);
        osc1hz = AudioMath::noteToHz(notePlaying, modelRef.bendCents);
        osc1.setAngle(osc1hz);
        vcaEaser.setTarget(vcaARslope.currVal + vcaARslope.gap);
        AudioMath::easeLog2(oscMix, oscMixEaseOut);
        AudioMath::easeLog2(fmAmp, fmAmpEaseOut);
        for (std::size_t i = 0; i < bufferSize; i += chunkSize) {
            for (std::size_t j = 0; j < chunkSize; j++) {
                float y2 = osc2.getNextSample(0);
                vcaEaserVal = vcaEaser.getValue();
                float y1 = osc1.getNextSample(y2 * fmAmpEaseOut * (vcaEaserVal + 0.3f));
                modelRef.addToSample(i + j, ((y1 * (1 - oscMixEaseOut) + y2 * oscMixEaseOut)) * vcaEaserVal * mixAmplitude);
            }
        }
        modelRef.vcaAR.commit(vcaARslope);
    } else {
        // Remove voice from playing
        // maybe use return-type
    }
    return true;
}
} // namespace Synth::Sketch