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
    // noteVelocity = velocity * velocity; // use x2 for now instead of log20.
    // 1.2f may cause distortion on pan! maybe not use "real" panning.
    noteVelocity = 1.2f * velocity;
    float angle = (this->pan + 1.0f) * 0.125f;
    leftGain = AudioMath::ccos(angle) * noteVelocity * this->volume;
    rightGain = AudioMath::csin(angle) * noteVelocity * this->volume;

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
    // currently sampleID <=> voiceID - could be a split..
    // move this tempBuffer to model later. No point in having one for each voice.
    if (modelRef.samples[sampleID].length == 0) {
        std::cout << "sample has not been setup - leaving.";
        vcaARslope.state = audio::envelope::ADSFRState::OFF;
        return false;
    }

    float speed = this->pitch;
    currSamplePos = modelRef.samples[sampleID].getSamplesToBuffer(modelRef.voiceBufferLeft, modelRef.voiceBufferRight, bufferSize, currSamplePos, speed);
    if (currSamplePos == 0.0f) {
        // reached end of sample, stop playing!
        vcaARslope.state = audio::envelope::ADSFRState::OFF;
        std::cout << "played voice:" << sampleID << std::endl;
    }

    for (std::size_t j = 0; j < bufferSize; j++) {
        // mono here..
        modelRef.addToLeftSample(j, modelRef.voiceBufferLeft[j] * leftGain);   // Left channel
        modelRef.addToRightSample(j, modelRef.voiceBufferLeft[j] * rightGain); // Right channel
    }

    return true;
}

} // namespace Synth::Beatnik