#include "Synth/Subreal/SubrealModel.h"
#include "core/audio/envelope/ADSFR.h"
#include <cmath>
#include <iostream>
#include <vector>

namespace Synth::Subreal {
constexpr int VOICE_COUNT = 8;

// Constructor to accept buffer and size
Model::Model(float *audioBuffer, std::size_t bufferSize)
    : buffer(audioBuffer), bufferSize(bufferSize) {
    setupParams(); // creates the array with attributes and lambdas for parameters - NOT INTERFACE
    SynthInterface::initializeParameters();
    SynthInterface::setupCCmapping("Subreal");
    reset();           // setup luts. Must come before voice allocation.
    voices.reserve(8); // Preallocate memory for 8 voices
    for (int i = 0; i < VOICE_COUNT; ++i) {
        voices.emplace_back(*this); // Pass reference to Model
    }
}

void Model::setupParams() {
    if (SynthInterface::paramDefs.empty()) {
        SynthInterface::paramDefs = {
            {"osc1_fmsens", {0.0f, 0, false, 0, 1, [this](float v) {
                                 fmSens = v;
                             }}},
            {"osc1_senstrack", {0.5f, 0, false, -1, 1, [this](float v) {
                                    senseTracking = v;
                                }}},
            {"osc2_semi", {0.5f, 13, false, -6, 6, [this](float v) {
                               semitone = round(v);
                           }}},
            {"osc2_oct", {0.5f, 7, false, -3, 3, [this](float v) {
                              osc2octave = round(v);
                          }}},
            {"osc_mix", {0.5f, 0, false, 0, 1, [this](float v) {
                             // easer should be at model. skipping easer for now
                             oscMix = v;
                         }}},
            {"vca_attack", {0.1f, 0, true, 2, 11, [this](float v) { // 8192 max
                                vcaAR.setTime(audio::envelope::ATTACK, v);
                                std::cout << "setting attack to " << v << " ms" << std::endl;
                            }}},
            {"vca_decay", {0.5f, 0, true, 5, 7, [this](float v) { // 8192 max
                               vcaAR.setTime(audio::envelope::DECAY, v);
                               std::cout << "setting decay to " << v << " ms" << std::endl;
                           }}},
            {"vca_sustain", {0.0f, 0, false, 0, 1, [this](float v) {
                                 vcaAR.setLevel(audio::envelope::SUSTAIN, v);
                                 std::cout << "setting sustain to " << v << std::endl;
                             }}},
            {"vca_fade", {0.0f, 0, false, 0, 1, [this](float v) {
                              vcaAR.setLeak(audio::envelope::FADE, v);
                              std::cout << "setting fade to " << v << std::endl;
                          }}},
            {"vca_release", {0.3f, 0, true, 10, 8, [this](float v) {
                                 vcaAR.setTime(audio::envelope::RELEASE, v);
                                 std::cout << "setting release to " << v << " ms" << std::endl;
                             }}},
            {"vcf_type", {0.0f, 3, false, 0, 2, [this](float v) {
                              // not totally safe but compact:
                              filter.setFilterType(static_cast<audio::filter::FilterType>(static_cast<int>(v)));
                              filter.initFilter();
                          }}},
            {"vcf_cutoff", {0.5f, 0, true, 50, 8, [this](float v) {
                                filter.setCutoff(v);
                                filter.initFilter();
                            }}},
            {"vcf_resonance", {0.0f, 0, false, 0, 1, [this](float v) {
                                   filter.setResonance(v);
                                   filter.initFilter();
                               }}},
            {"lfo1_depth", {0.0f, 0, false, 0, 1, [this](float v) {
                                lfo1Depth = v;
                            }}},
            {"lfo1_vca", {0.0f, 0, false, 0, 1, [this](float v) {
                              lfo1vca = v;
                          }}},
            {"lfo1_speed", {0.0f, 0, true, 100, 9, [this](float v) {
                                lfo1.setSpeed(v); // in mHz.
                            }}}};
    }
}

void Model::reset() {
    lut1.applySine(1, 0.5);
    /*lut1.applySine(2, 0.4);
    lut1.applySine(3, 0.3);
    lut1.applySine(4, 0.2);
    lut1.applySine(5, 0.2);
    lut1.applySine(6, 0.1);*/
    lut1.normalize();
    //
    lut2.applySine(1, 0.6);
    /*lut2.applySine(3, 0.3); */
    lut2.normalize();
}

void Model::parseMidi(uint8_t cmd, uint8_t param1, uint8_t param2) {
    uint8_t messageType = cmd & 0xf0;
    float fParam2 = static_cast<float>(param2) * (1.0f / 127.0f);
    switch (messageType) {
    case 0x90:
        // special case, if vel < 64 and note = notePlaying => reOn
        if (false) {
            // if (param1 == notePlaying && param2 < 64) { - not working very well..
            // vcaAR.triggerSlope(vcaARslope, audio::envelope::NOTE_REON);
        } else {
            int8_t voiceIdx = findVoiceToAllocate(param1);
            // ok now start that note..
            if (voiceIdx >= 0) {
                voices[voiceIdx].noteOn(param1, (fParam2 + 0.1));
            }
        }
        break;
    case 0x80:
        // scan all voices for matching note, if no match, ignore
        for (Voice &myVoice : voices) {
            if (myVoice.notePlaying == param1) {
                // well really, we should set the ARstate to release
                myVoice.noteOff();
            }
        }
        break;
    case 0xb0:
        SynthInterface::handleMidiCC(param1, fParam2);
        break;
    case 0xe0: {
        int pitchBendValue = (static_cast<int>(param2) << 7) | static_cast<int>(param1);
        bendCents = ((pitchBendValue - 8192) / 8192.0f) * 200.0f;
        break;
    }
    }
}

int8_t Model::findVoiceToAllocate(uint8_t note) {
    /* search for:
  1) Same note - re-use voice
  2) Idle voice
  3) Released voice - find most silent.
  4) Give up - return -1
*/
    int8_t targetVoice = -1;
    int8_t sameVoice = -1;
    int8_t idleVoice = -1;
    int8_t releasedVoice = -1;
    int8_t releasedVoiceAmp = 1;
    // 8 shouldn't be hardcoded..
    for (int i = 0; i < VOICE_COUNT; i++) {
        Voice &myVoice = voices[i];
        if (myVoice.notePlaying == note) {
            // re-use (what about lfo-ramp here..)
            sameVoice = i;
            break;
        }
        if (idleVoice == -1) {
            // not found yet so keep looking
            if (myVoice.getVCAstate() == audio::envelope::ADSFRState::OFF) {
                idleVoice = i;
            }
            // ok try to overtake..
            if (myVoice.getVCAstate() == audio::envelope::ADSFRState::RELEASE) {
                // candidate, see if amp lower than current.
                float temp = myVoice.getVCALevel();
                if (temp < releasedVoiceAmp) {
                    // candidate!
                    releasedVoice = i;
                    releasedVoiceAmp = temp;
                }
            }
        }
    }
    targetVoice = (sameVoice != -1) ? sameVoice : ((idleVoice != -1) ? idleVoice : ((releasedVoice != -1) ? releasedVoice : (-1)));
    return targetVoice;
}

bool Model::renderNextBlock() {
    // make stuff not done inside chunk.
    // LFO1
    //$se = $this->settings;
    //$lfoHz = $this->getNum('LFO1_RATE');
    //$this->lfo1Sample = $this->lfo1->getNextSample($blockSize * $lfoHz);
    // iterate over all voices and create a summed output.
    for (uint8_t i = 0; i < bufferSize; i++) {
        synthBuffer[i] = 0;
    }
    for (uint8_t i = 0; i < VOICE_COUNT; i++) {
        if (voices[i].checkVoiceActive()) {
            voices[i].renderNextVoiceBlock(bufferSize);
        }
    }
    float dist;
    for (uint8_t i = 0; i < bufferSize; i++) {
        // we could add some sweet dist here..
        dist = synthBuffer[i];
        dist = dist * dist; // skip polarity..
        buffer[i] = synthBuffer[i] * (5 - dist) * 0.1f;
    }
    // not always stereo for now..
    return false;
}

float Model::getOscMix() {
    return oscMix;
}

void Model::addToSample(std::size_t sampleIdx, float val) {
    // use local buffer for spead. Possibly double..
    this->synthBuffer[sampleIdx] += val;
}

const audio::osc::LUT &Model::getLUT1() const {
    return lut1; // Return a const reference to lut1
}

const audio::osc::LUT &Model::getLUT2() const {
    return lut2; // Return a const reference to lut2
}
} // namespace Synth::Subreal
