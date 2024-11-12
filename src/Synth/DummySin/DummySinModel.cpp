#include "./DummySinModel.h"
#include "core/audio/envelope/ADSFR.h"
#include "core/audio/filter/MultiFilter.h"
#include <cmath>
#include <iostream>
#include <vector>

namespace Synth::DummySin {

// Constructor to accept buffer and size
Model::Model(float *audioBuffer, std::size_t bufferSize)
    : buffer(audioBuffer), bufferSize(bufferSize), osc1(lut1), osc2(lut2) {
    setupParams(); // creates the array with attributes and lambdas for parameters - NOT INTERFACE
    SynthInterface::initializeParameters();
    SynthInterface::setupCCmapping("DummySin");
    reset();
}

void Model::setupParams() {
    if (SynthInterface::paramDefs.empty()) {
        SynthInterface::paramDefs = {
            {"osc1_fmsens", {0.0f, 0, false, 0, 1, [this](float v) {
                                 fmSens = v;
                             }}},
            {"osc1_senstrack", {0.5f, 0, false, -1, 1, [this](float v) {
                                    std::cout << "setting sense-tracking to " << v << std::endl;
                                    senseTracking = v;
                                }}},
            {"osc2_semi", {0.5f, 13, false, -6, 6, [this](float v) {
                               semitone = round(v);
                           }}},
            {"osc2_oct", {0.5f, 7, false, -3, 3, [this](float v) {
                              osc2octave = round(v);
                          }}},
            {"osc_mix", {0.5f, 0, false, 0, 1, [this](float v) {
                             oscMixEaser.setTarget(v);
                             std::cout << "setting osc-mix to " << v << std::endl;
                         }}},
            {"vca_attack", {0.1f, 0, true, 2, 11, [this](float v) { // 8192 max
                                vcaAR.setTime(audio::envelope::ATTACK, v);
                                std::cout << "setting attack to " << v << " ms" << std::endl;
                            }}},
            {"vca_decay", {0.5f, 0, true, 5, 7, [this](float v) { // 8192 max
                               vcaAR.setTime(audio::envelope::DECAY, v);
                               std::cout << "setting decay to " << v << " ms" << std::endl;
                           }}},
            {"vca_sustain", {0.8f, 0, false, 0, 1, [this](float v) {
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
            vcaAR.triggerSlope(vcaARslope, audio::envelope::NOTE_REON);
        } else {
            notePlaying = param1;
            noteVelocity = fParam2 + 0.1f;
            vcaAR.triggerSlope(vcaARslope, audio::envelope::NOTE_ON);
            // osc1.reset(); //start at angle0
            // osc1.reset();
            // osc2.reset();
            //
            // float notePan = 1.0f - (64 - param1) / 64;
            // gainLeft = AudioMath::ccos(notePan * 0.25f);  // Left gain decreases as panVal goes to 1
            // gainRight = AudioMath::csin(notePan * 0.25f); // Right gain increases as panVal goes to 1
        }
        break;
    case 0x80:
        if (param1 == notePlaying)
            vcaAR.triggerSlope(vcaARslope, audio::envelope::NOTE_OFF);
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

bool Model::renderNextBlock() {
    float osc1hz, osc2hz;
    constexpr int chunkSize = 16;
    float osc1hzOld;

    vcaAR.updateDelta(vcaARslope);
    if (vcaARslope.state != audio::envelope::OFF) {
        osc2hz = AudioMath::noteToHz(notePlaying + semitone + osc2octave * 12, bendCents);
        osc2.setAngle(osc2hz);
        vcaEaser.setTarget(vcaARslope.currVal + vcaARslope.gap); // + lfo1.getLFOval());
        if (debugCount % 1024 == 0) {
            std::cout << "setting easer to:" << vcaARslope.currVal << std::endl;
        }
        for (std::size_t i = 0; i < bufferSize; i++) {
            float y2 = osc2.getNextSample(0);
            if (i % chunkSize == 0) {
                osc1hz = AudioMath::noteToHz(notePlaying, bendCents);
                osc1hzOld += (osc1hz - osc1hzOld) / 100;
                osc1.setAngle(osc1hz);
            }
            float tracking = fmax(0, (1.0f + senseTracking * AudioMath::noteToFloat(notePlaying) * 10));
            float y1 = osc1.getNextSample(y2 * tracking * fmSens * 2.0f * noteVelocity);

            velocityLast += (noteVelocity - velocityLast) / 10.0f;
            vcaEaserVal = vcaEaser.getValue(); // * lfo1vca;
            float oscMix = oscMixEaser.getValue();
            buffer[i] = ((y1 * (1 - oscMix) + y2 * oscMix)) * vcaEaserVal * velocityLast * 2.0f;
        }
        vcaAR.commit(vcaARslope);
        debugCount++;
        if (debugCount % 1024 == 0) {
            std::cout << "Level: " << vcaARslope.currVal << ", Delta:" << vcaARslope.gap << std::endl;
        }
    } else {
        notePlaying = 0;
        for (std::size_t i = 0; i < bufferSize; i++) {
            buffer[i] = 0;
        }
    }

    motherboardActions();
    return false; // mono
}

void Model::motherboardActions() {
    lfo1.updatePhase();
    lfo2.updatePhase();
}

} // namespace Synth::DummySin
