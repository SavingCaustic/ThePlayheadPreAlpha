#include "Synth/Subreal/SubrealModel.h"
#include "core/audio/envelope/ADSFR.h"
#include <cmath>
#include <core/utils/FNV.h>
#include <iostream>
#include <vector>

namespace Synth::Subreal {
constexpr int VOICE_COUNT = 16;

Model::Model() {
    setupParams(UP::up_count);
    SynthBase::initParams();
    SynthInterface::setupCCmapping("Subreal");
    reset(); // setup luts. Must come before voice allocation.
    voices.reserve(VOICE_COUNT);
    for (int i = 0; i < VOICE_COUNT; ++i) {
        voices.emplace_back(*this); // Pass reference to Model
    }
}

Model::~Model() {
    FORMAT_LOG_MESSAGE(logTemp, LOG_INFO, "destructing luts %s", "...");
    sendAudioLog();
    delete lut1;
    delete lut2;
}

void Model::updateVoiceLUT(const audio::osc::LUT &lut, int no) {
    for (int i = 0; i < VOICE_COUNT; i++) {
        Voice &myVoice = voices[i];
        myVoice.setLUTs(lut, no);
    }
}

void Model::updateSetting(const std::string &type, void *object, uint32_t size, bool isStereo, Destructor::Record &recordDelete) {
    // Hash the key
    uint32_t keyFNV = Utils::Hash::fnv1a(type);
    // Handle settings based on hashed keys
    switch (keyFNV) {
    case Utils::Hash::fnv1a_hash("lut1_overtones"):
        if (object && size == sizeof(audio::osc::LUT)) {
            // Safely cast the object to the expected type
            auto *lut = reinterpret_cast<audio::osc::LUT *>(object);
            // Push the current LUT to the destructor queue, or delete it directly
            if (lut1) {
                FORMAT_LOG_MESSAGE(logTemp, LOG_INFO, "destructing lut %d", 1);
                sendAudioLog();
                recordDelete.ptr = lut1;
                recordDelete.deleter = [](void *ptr) { delete static_cast<audio::osc::LUT *>(ptr); }; // Create deleter for LUT
                // moved to ObjectManager: destructorQueue.push(lut1, sizeof(audio::osc::LUT), false, "LUT");
            }
            // Assign the new LUT
            // std::cout << "setting new lut-1 wave" << std::endl;
            lut1 = lut;
            updateVoiceLUT(*lut1, 1);
        } else {
            // Handle error: Unexpected object type or size
            throw std::runtime_error("Invalid object for lut1_overtones");
        }
        break;
    case Utils::Hash::fnv1a_hash("lut2_overtones"):
        if (object && size == sizeof(audio::osc::LUT)) {
            // Safely cast the object to the expected type
            auto *lut = reinterpret_cast<audio::osc::LUT *>(object);
            // Push the current LUT to the destructor queue, or delete it directly
            if (lut2) {
                recordDelete.ptr = lut2;
                recordDelete.deleter = [](void *ptr) { delete static_cast<audio::osc::LUT *>(ptr); }; // Create deleter for LUT
                // moved to ObjectManager: destructorQueue.push(lut1, sizeof(audio::osc::LUT), false, "LUT");
            }
            // Assign the new LUT
            FORMAT_LOG_MESSAGE(logTemp, LOG_INFO, "setting new wave for lut %d", 2);
            sendAudioLog();
            lut2 = lut;
            updateVoiceLUT(*lut2, 2);
        } else {
            // Handle error: Unexpected object type or size
            throw std::runtime_error("Invalid object for lut1_overtones");
        }
        break;
    default:
        // Handle keys that don't match
        throw std::invalid_argument("Unknown key for updateSetting: " + type);
    }
}

void Model::reset() {
}

void Model::bindBuffers(float *audioBufferLeft, float *audioBufferRight, std::size_t bufferSize) {
    this->bufferLeft = audioBufferLeft;
    this->bufferRight = audioBufferRight;
    this->bufferSize = bufferSize;
}

void Model::setupParams(int upCount) {
    if (SynthBase::paramDefs.empty()) {
        // after declaration, indexation requested, see below..
        SynthBase::paramDefs = {
            {UP::modwheel, {"modwheel", 0.0f, 0, false, 0, 1, [this](float v) {
                                modwheel = v;
                                if (lfo1_mw_control == MW::LFOcontrol::speed) {
                                    Model::setLFO1speed();
                                }
                                if (lfo2_mw_control == MW::LFOcontrol::speed) {
                                    Model::setLFO2speed();
                                }
                            }}},

            {UP::osc_mix, {"osc_mix", 0.5f, 0, false, 0, 1, [this](float v) {
                               osc_mix = v;
                           }}},
            {UP::osc1_fmsens, {"osc1_fmsens", 0.3f, 0, false, 0, 1, [this](float v) {
                                   osc1_fmsens = v;
                               }}},
            {UP::osc1_detune, {"osc1_detune", 0.5f, 0, false, -25, 25, [this](float v) {
                                   osc1_detune = v;
                               }}},
            {UP::osc2_oct, {"osc2_oct", 0.5f, 7, false, -3, 3, [this](float v) {
                                osc2_oct = round(v);
                            }}},
            {UP::osc2_semi, {"osc2_semi", 0.5f, 13, false, -6, 6, [this](float v) {
                                 osc2_semi = round(v);
                             }}},
            {UP::osc2_detune, {"osc2_detune", 0.5f, 0, false, -25, 25, [this](float v) {
                                   osc2_detune = v;
                               }}},

            {UP::vcf_cutoff, {"vcf_cutoff", 0.5f, 0, true, 100, 8, [this](float v) {
                                  vcf_cutoff = v;
                              }}},
            {UP::vcf_resonance, {"vcf_resonance", 0.0f, 0, false, 0, 1, [this](float v) {
                                     vcf_resonance = v;
                                 }}},
            {UP::vcf_type, {"vcf_type", 0.0f, 4, false, 0, 3, [this](float v) {
                                // Bypass, LFP, HPF, BPF
                                filterType = static_cast<audio::filter::FilterType>(static_cast<int>(v));
                                // FORMAT_LOG_MESSAGE(logTemp, LOG_INFO, "filterType set to val %d", static_cast<int>(v));
                                // sendAudioLog();
                            }}},
            {UP::vcf_shape, {"vcf_shape", 0.0f, 4, false, 0, 3, [this](float v) {
                                 int i = round(v);
                                 filterPoles = static_cast<audio::filter::FilterPoles>(i % 2);
                                 vcfInverse = (i > 1);
                             }}},
            {UP::vcf_cutoff_kt, {"vcf_cutoff_kt", 0.5f, 0, false, -1, 1, [this](float v) {
                                     vcf_cutoff_kt = v;
                                 }}},
            {UP::vcf_cutoff_vt, {"vcf_cutoff_vt", 0.5f, 0, false, -1, 1, [this](float v) {
                                     vcf_cutoff_vt = v;
                                 }}},

            {UP::peg_atime, {"peg_atime", 0.5f, 0, true, 10, 8, [this](float v) { // 8192 max
                                 pegAR.setTime(audio::envelope::ASRState::ATTACK, v);
                             }}},
            {UP::peg_rtime, {"peg_rtime", 0.5f, 0, true, 10, 8, [this](float v) { // 8192 max
                                 pegAR.setTime(audio::envelope::ASRState::RELEASE, v);
                             }}},
            {UP::pb_range, {"pb_range", 0.0f, 11, false, 2, 12, [this](float v) {
                                pb_range = round(v) * 100;
                            }}},
            {UP::peg_asemis, {"peg_asemis", 0.5f, 11, false, -5, 5, [this](float v) { // +/- 1,2,5,7,12 semis
                                  peg_asemis = pegDepth2Semis[round(v) + 5];
                              }}},
            {UP::peg_rsemis, {"peg_rsemis", 0.5f, 11, false, -5, 5, [this](float v) { // +/- 12 semis
                                  peg_rsemis = pegDepth2Semis[round(v) + 5];
                              }}},
            {UP::noise, {"noise", 0.5f, 0, false, -1, 1, [this](float v) { // +/- 12 semis
                             if (v > 0) {
                                 noise_vcf = v * v; // ha, v*v solves both amplitude and polarity
                                 noise_o2 = 0;
                             } else {
                                 noise_o2 = v * v;
                                 noise_vcf = 0;
                             }
                         }}},
            // WOW WE HAVE ONE OVER HERE!

            {UP::lfo1_speed, {"lfo1_speed", 0.5f, 0, true, 100, 9, [this](float v) {
                                  lfo1_speed = v;
                                  setLFO1speed();
                              }}},
            {UP::lfo1_depth, {"lfo1_depth", 0.0f, 0, true, 2, 10, [this](float v) {
                                  // of what really..
                                  lfo1_depth = (v - 2) * 0.01f;
                                  FORMAT_LOG_MESSAGE(logTemp, LOG_INFO, "setting lfo1-depth to %f", lfo1_depth);
                                  sendAudioLog();
                              }}},
            {UP::lfo1_shape, {"lfo1_shape", 0.0f, audio::lfo::LFOShape::_count, false, 0, audio::lfo::LFOShape::_count - 1, [this](float v) {
                                  lfo1.setShape(static_cast<audio::lfo::LFOShape>(static_cast<int>(v)));
                                  FORMAT_LOG_MESSAGE(logTemp, LOG_INFO, "setting lfo1-shape to %f", v);
                                  sendAudioLog();
                              }}},
            {UP::lfo1_routing, {"lfo1_routing", 0.0f, 4, false, 0, 3, [this](float v) {
                                    lfo1_routing = static_cast<LFO1::Routing>(static_cast<int>(v));
                                    FORMAT_LOG_MESSAGE(logTemp, LOG_INFO, "setting lfo1-routing to %f", v);
                                    sendAudioLog();
                                }}},
            {UP::lfo1_ramp, {"lfo1_ramp", 0.0f, 0, true, 2, 10, [this](float v) {
                                 lfo1_ramp = v * 0.0004; // lo val = ramp
                             }}},
            {UP::lfo1_mw_control, {"lfo1_mw_control", 0.0f, MW::LFOcontrol::_count, false, 0, MW::LFOcontrol::_count - 1, [this](float v) {
                                       lfo1_mw_control = static_cast<MW::LFOcontrol>(static_cast<int>(v));
                                       setLFO1speed();
                                   }}},

            /* next bank */
            {UP::osc_mix_kt, {"osc_mix_kt", 0.5f, 0, false, -1, 1, [this](float v) {
                                  osc_mix_kt = v;
                              }}},
            {UP::osc1_fmsens_kt, {"osc1_fmsens_kt", 0.5f, 0, false, -1, 1, [this](float v) {
                                      osc1_fmsens_kt = v;
                                  }}},
            {UP::vca_pan_kt, {"vca_pan_kt", 0.2f, 0, false, 0, 1, [this](float v) {
                                  vca_pan_kt = v;
                              }}},
            {UP::osc_mix_vt, {"osc_mix_vt", 0.5f, 0, false, -1, 1, [this](float v) {
                                  osc_mix_vt = v;
                              }}},
            {UP::osc1_fmsens_vt, {"osc1_fmsens_vt", 0.5f, 0, false, -1, 1, [this](float v) {
                                      osc1_fmsens_vt = v;
                                  }}},
            {UP::osc2_freq_kt, {"osc2_freq_kt", 0.5f, 5, false, 0, 4, [this](float v) {
                                    // When using steps *NO DECIMALS!*
                                    osc2_freq_kt = v * 0.5f;
                                    FORMAT_LOG_MESSAGE(logTemp, LOG_INFO, "setting osc2-freq_kt to %f", osc2_freq_kt);
                                    sendAudioLog();
                                }}},

            {UP::vcf_attack, {"vcf_attack", 0.0f, 0, true, 2, 11, [this](float v) { // 8192 max
                                  vcfAR.setTime(audio::envelope::ADSFRState::ATTACK, v - 1.5f);
                              }}},
            {UP::vcf_decay, {"vcf_decay", 0.8f, 0, true, 5, 7, [this](float v) { // 8192 max
                                 vcfAR.setTime(audio::envelope::ADSFRState::DECAY, v);
                             }}},
            {UP::vcf_sustain, {"vcf_sustain", 0.8f, 0, false, 0, 1, [this](float v) {
                                   vcfAR.setLevel(audio::envelope::ADSFRState::SUSTAIN, v);
                               }}},
            {UP::vcf_release, {"vcf_release", 0.2f, 0, true, 10, 8, [this](float v) {
                                   vcfAR.setTime(audio::envelope::ADSFRState::RELEASE, v);
                               }}},
            {UP::vcf_fade, {"vcf_fade", 0.3f, 0, false, 0, 1, [this](float v) {
                                vcfAR.setLeak(audio::envelope::ADSFRState::FADE, v);
                            }}},
            {UP::vcf_rate_kt, {"vcf_rate_kt", 0.0f, 0, false, 0, 5, [this](float v) {
                                   vcf_rate_kt = v;
                               }}},
            {UP::vca_attack, {"vca_attack", 0.0f, 0, true, 2, 11, [this](float v) { // 8192 max
                                  vcaAR.setTime(audio::envelope::ADSFRState::ATTACK, v - 1.5f);
                              }}},
            {UP::vca_decay, {"vca_decay", 0.5f, 0, true, 5, 7, [this](float v) { // 8192 max
                                 vcaAR.setTime(audio::envelope::ADSFRState::DECAY, v);
                             }}},
            {UP::vca_sustain, {"vca_sustain", 0.8f, 0, false, 0, 1, [this](float v) {
                                   vcaAR.setLevel(audio::envelope::ADSFRState::SUSTAIN, v);
                               }}},
            {UP::vca_release, {"vca_release", 0.7f, 0, true, 10, 8, [this](float v) { //!!
                                   vcaAR.setTime(audio::envelope::ADSFRState::RELEASE, v);
                               }}},
            {UP::vca_fade, {"vca_fade", 0.3f, 0, false, 0, 1, [this](float v) {
                                vcaAR.setLeak(audio::envelope::ADSFRState::FADE, v);
                            }}},
            {UP::vca_rate_kt, {"vca_rate_kt", 0.0f, 0, false, 0, 5, [this](float v) {
                                   vca_rate_kt = v;
                               }}},

            {UP::lfo2_speed, {"lfo2_speed", 0.5f, 0, true, 100, 9, [this](float v) {
                                  lfo2_speed = v;
                                  setLFO2speed();
                              }}},
            {UP::lfo2_depth, {"lfo2_depth", 0.0f, 0, false, 0, 1, [this](float v) {
                                  lfo2_depth = v;
                              }}},
            {UP::lfo2_shape, {"lfo2_shape", 0.0f, audio::lfo::LFOShape::_count, false, 0, audio::lfo::LFOShape::_count - 1, [this](float v) {
                                  lfo2.setShape(static_cast<audio::lfo::LFOShape>(static_cast<int>(v)));
                              }}},
            {UP::lfo2_routing, {"lfo2_routing", 0.0, LFO1::Routing::_count, false, 0, LFO1::Routing::_count - 1, [this](float v) {
                                    lfo2_routing = static_cast<LFO2::Routing>(static_cast<int>(v));
                                }}},
            {UP::not_used_1, {"not_used_1", 0.0f, 0, false, 0, 7, [this](float v) {
                                  // dummy
                                  lfo2_depth = lfo2_depth;
                              }}},
            {UP::lfo2_mw_control, {"lfo2_mw_control", 0.0f, MW::LFOcontrol::_count, false, 0, MW::LFOcontrol::_count - 1, [this](float v) {
                                       // of what really..
                                       lfo2_mw_control = static_cast<MW::LFOcontrol>(static_cast<int>(v));
                                       setLFO2speed();
                                   }}}

        };
        // now reqeuest interface to reverse index.
        SynthBase::indexParams(upCount);
    }
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
            // FORMAT_LOG_MESSAGE(logTemp, LOG_CRITICAL, "note on in audio thread.. %s", "..awsome");
            // sendAudioLog();
            if (voiceIdx >= 0) {
                voices[voiceIdx].noteOn(param1, (fParam2)); // +0.1??
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
        bendCents = ((pitchBendValue - 8192) / 8192.0f) * pb_range;
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
                float temp = myVoice.getVCAlevel();
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
    // we are using synth-buffer to do dist-calculation, before sending to rack.
    // synth-buffer should be doubled - stereo.
    for (uint8_t i = 0; i < TPH_RACK_RENDER_SIZE; i++) {
        synthBufferLeft[i] = 0;
        synthBufferRight[i] = 0;
    }

    // optimized in params..
    // setLFO1speed();
    // setLFO2speed();

    lfo1.updatePhase();
    lfo2.updatePhase();

    //
    for (uint8_t i = 0; i < VOICE_COUNT; i++) {
        if (voices[i].checkVoiceActive()) {
            voices[i].renderNextVoiceBlock(bufferSize);
        }
    }
    // LEFT & RIGHT..
    float dist;
    for (uint8_t i = 0; i < TPH_RACK_RENDER_SIZE; i++) {
        // we could add some sweet dist here..
        dist = synthBufferLeft[i];
        dist = fmin(4.0f, dist * dist);
        bufferLeft[i] = synthBufferLeft[i] * (5 - dist) * 0.2f;
    }
    for (uint8_t i = 0; i < TPH_RACK_RENDER_SIZE; i++) {
        dist = synthBufferRight[i];
        dist = fmin(4.0f, dist * dist);
        bufferRight[i] = synthBufferRight[i] * (5 - dist) * 0.2f;
    }

    // debugging
    if (false) {
        for (std::size_t i = 0; i < TPH_RACK_RENDER_SIZE; i++) {
            bufferLeft[i] += AudioMath::noise() * 0.01f - 0.005f;
            bufferRight[i] += AudioMath::noise() * 0.01f - 0.005f;
        }
    }

    // not always stereo for now..
    return true;
}

void Model::addToLeftSample(std::size_t sampleIdx, float val) {
    // use local buffer for spead. Possibly double..
    this->synthBufferLeft[sampleIdx] += val;
}

void Model::addToRightSample(std::size_t sampleIdx, float val) {
    // use local buffer for spead. Possibly double..
    this->synthBufferRight[sampleIdx] += val;
}

void Model::setLFO1speed() {
    if (lfo1_mw_control == MW::LFOcontrol::speed) {
        lfo1.setSpeed(lfo1_speed * modwheel);
    } else {
        lfo1.setSpeed(lfo1_speed);
    }
}

void Model::setLFO2speed() {
    if (lfo2_mw_control == MW::LFOcontrol::speed) {
        lfo2.setSpeed(lfo2_speed * modwheel);
    } else {
        lfo2.setSpeed(lfo2_speed);
    }
}

const audio::osc::LUT &Model::getLUT1() const {
    if (!lut1) {
        throw std::runtime_error("lut1 is not initialized");
    }
    return *lut1; // Dereference lut1 to return a reference to the actual object
}

const audio::osc::LUT &Model::getLUT2() const {
    if (!lut2) {
        throw std::runtime_error("lut2 is not initialized");
    }
    return *lut2; // Dereference lut1 to return a reference to the actual object
}
} // namespace Synth::Subreal
