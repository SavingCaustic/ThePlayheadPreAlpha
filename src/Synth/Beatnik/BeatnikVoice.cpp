#include "BeatnikVoice.h"
#include "BeatnikModel.h"

namespace Synth::Beatnik {

Beatnik::Voice::Voice(Model &model)
    : modelRef(model) {
    reset();
}

void Beatnik::Voice::reset() {
    // Called on setup
    notePlaying = 255; // unsigned byte. best like this..
    currSamplePos = 0;
}

void Beatnik::Voice::noteOn(uint8_t midiNote, float velocity) {
    notePlaying = midiNote;
    sampleID = (midiNote) % 12;
    currSamplePos = 0;
    noteVelocity = velocity * velocity; // use x2 for now instead of log20.
    float pan = 0.0f + (60 - midiNote) * 0.2f;
    float angle = (pan + 1.0f) * 0.125;
    leftGain = AudioMath::ccos(angle) * noteVelocity * 0.5f;
    rightGain = AudioMath::csin(angle) * noteVelocity * 0.5f;

    // it's all in the voice. no AR in model.
    vcaAR.triggerSlope(vcaARslope, audio::envelope::ADSFRCmd::NOTE_ON);
}

void Beatnik::Voice::noteOff() {
    // enter release state in all envelopes.
    vcaAR.triggerSlope(vcaARslope, audio::envelope::ADSFRCmd::NOTE_OFF);
}

audio::envelope::ADSFRState Beatnik::Voice::getVCAstate() {
    return vcaARslope.state;
}

float Beatnik::Voice::getVCAlevel() {
    return vcaARslope.currVal;
}

bool Beatnik::Voice::checkVoiceActive() {
    return vcaARslope.state != audio::envelope::ADSFRState::OFF;
}

bool Beatnik::Voice::renderNextVoiceBlock(std::size_t bufferSize) {
    float stereoBuffer[bufferSize]; // Stereo buffer for both L and R channels
    if (modelRef.samples[sampleID].length == 0) {
        std::cout << "sample has not been setup - leaving.";
        vcaARslope.state = audio::envelope::ADSFRState::OFF;
        return false;
    }

    currSamplePos = modelRef.samples[sampleID].getSamplesToBuffer(stereoBuffer, bufferSize, currSamplePos, 1.0f);
    if (currSamplePos == 0.0f) {
        // reached end of sample, stop playing!
        vcaARslope.state = audio::envelope::ADSFRState::OFF;
    }

    // Process stereo samples
    for (std::size_t j = 0; j < bufferSize; j = j + 2) {
        // Attenuate and assign stereo channels
        modelRef.addToSample(j, stereoBuffer[j] * leftGain);          // Left channel
        modelRef.addToSample(j + 1, stereoBuffer[j + 1] * rightGain); // Right channel
    }

    return true;
}

} // namespace Synth::Beatnik