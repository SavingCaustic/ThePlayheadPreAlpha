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
    sampleID = (midiNote - 60) % 12;
    leftAtt = 0.0f;
    rightAtt = 0.0f;
    currSamplePos = 0;
    noteVelocity = velocity;
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

    currSamplePos = modelRef.samples[sampleID].getSamplesToBuffer(stereoBuffer, bufferSize, currSamplePos, 1.0f);
    if (currSamplePos == 0.0f) {
        // reached end of sample, stop playing!
        vcaARslope.state = audio::envelope::ADSFRState::OFF;
    }

    // Process stereo samples
    for (std::size_t j = 0; j < bufferSize; j = j + 2) {
        // Attenuate and assign stereo channels
        modelRef.addToSample(j, stereoBuffer[j] * (1 - leftAtt));          // Left channel
        modelRef.addToSample(j + 1, stereoBuffer[j + 1] * (1 - rightAtt)); // Right channel
    }

    return true;
}

} // namespace Synth::Beatnik