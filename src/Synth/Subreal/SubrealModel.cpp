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
    std::cout << "destructing LUT's" << std::endl;
    delete lut1;
    delete lut2;
}

void Model::updateVoiceLUT(const audio::osc::LUT &lut, int no) {
    for (int i = 0; i < VOICE_COUNT; i++) {
        Voice &myVoice = voices[i];
        myVoice.setLUTs(lut, no);
    }
}

// well maybe its not setting after all.. more like a factory needed here, but factory have no access to the model..
// so this class should rather be called mountObject
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
                std::cout << "i need to delete old LUT. Strange.. " << std::endl;
                recordDelete.ptr = lut1;
                recordDelete.deleter = [](void *ptr) { delete static_cast<audio::osc::LUT *>(ptr); }; // Create deleter for LUT
                // destructorQueue.push(lut1, sizeof(audio::osc::LUT), false, "LUT");
            }
            // Assign the new LUT
            std::cout << "setting new lut-1 wave" << std::endl;
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
                // destructorQueue.push(lut1, sizeof(audio::osc::LUT), false, "LUT");
            }
            // Assign the new LUT
            std::cout << "setting new lut-2 wave" << std::endl;
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

void Model::buildLUT(audio::osc::LUT &lut, const std::string val) {
    std::vector<float> values;
    std::istringstream stream(val);
    std::string token;
    int h = 1;
    while (std::getline(stream, token, ',')) {
        lut.applySine(h, std::stof(token));
        h++;
    }
    lut.normalize();
}

void Model::reset() {
}

void Model::bindBuffers(float *audioBuffer, std::size_t bufferSize) {
    this->buffer = audioBuffer;
    this->bufferSize = bufferSize;
}

void Model::setupParams(int upCount) {
    if (SynthBase::paramDefs.empty()) {
        // after declaration, indexation requested, see below..
        SynthBase::paramDefs = {
            {UP::osc1_wf, {"osc1_wf", 0.0f, OSC1::WF::_count, false, 0, OSC1::WF::_count - 1, [this](float v) {
                               std::cout << "setting osc1 wf to" << v << std::endl;
                           }}},
            {UP::osc1_fmsens, {"osc1_fmsens", 0.0f, 0, false, 0, 1, [this](float v) {
                                   fmSens = v;
                               }}},
            /*{UP::osc1_amsens, {"osc1_amsens", 0.0f, 0, false, 0, 1, [this](float v) {
                                   amSens = v;
                               }}},*/
            {UP::osc_mix, {"osc_mix", 0.5f, 0, false, 0, 1, [this](float v) {
                               // easer should be at model. skipping easer for now
                               oscMix = v;
                           }}},
            {UP::osc_detune, {"osc_detune", 0.5f, 0, false, -25, 25, [this](float v) {
                                  // easer should be at model. skipping easer for now
                                  oscDetune = v;
                              }}},
            {UP::osc1_senstrack, {"osc1_senstrack", 0.5f, 0, false, -1, 1, [this](float v) {
                                      senseTracking = v;
                                  }}},
            {UP::osc2_semi, {"osc2_semi", 0.5f, 13, false, -6, 6, [this](float v) {
                                 semitone = round(v);
                             }}},
            {UP::osc2_oct, {"osc2_oct", 0.5f, 7, false, -3, 3, [this](float v) {
                                osc2octave = round(v);
                            }}},
            {UP::osc2_noise_mix, {"osc2_noise_mix", 0.0f, 0, false, 0, 1, [this](float v) {
                                      osc2noiseMix = v;
                                  }}},
            {UP::osc2_freqtrack, {"osc2_freqtrack", 1.0f, 0, false, 0, 1, [this](float v) {
                                      osc2freqTrack = v;
                                  }}},
            {UP::vcf_cutoff, {"vcf_cutoff", 1.0f, 0, true, 100, 8, [this](float v) {
                                  filterCutoff = v;
                                  // filter.setCutoff(v);
                                  // filter.initFilter();
                              }}},
            {UP::vcf_resonance, {"vcf_resonance", 0.0f, 0, false, 0, 1, [this](float v) {
                                     filterResonance = v;
                                     // filter.setResonance(v);
                                     // filter.initFilter();
                                 }}},
            {UP::vcf_type, {"vcf_type", 0.0f, 4, false, 0, 3, [this](float v) {
                                // LFP,HPF,BPF&NOTCH not totally safe but compact:
                                filterType = static_cast<audio::filter::FilterType>(static_cast<int>(v));
                                // filter.initFilter();
                            }}},
            {UP::vcf_shape, {"vcf_shape", 0.0f, 4, false, 0, 3, [this](float v) {
                                 // not totally safe but compact:
                                 int i = round(v);
                                 filterPoles = static_cast<audio::filter::FilterPoles>(i % 2);
                                 vcfInverse = (i > 1);
                                 // filter.initFilter();
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
            {UP::vcf_fade, {"vcf_fade", 0.3f, 0, false, 0, 1, [this](float v) {
                                vcfAR.setLeak(audio::envelope::ADSFRState::FADE, v);
                            }}},
            {UP::vcf_release, {"vcf_release", 0.2f, 0, true, 10, 8, [this](float v) {
                                   vcfAR.setTime(audio::envelope::ADSFRState::RELEASE, v);
                               }}},
            {UP::vca_attack, {"vca_attack", 0.0f, 0, true, 2, 11, [this](float v) { // 8192 max
                                  std::cout << "setting vca_attack to " << v << std::endl;
                                  vcaAR.setTime(audio::envelope::ADSFRState::ATTACK, v - 1.5f);
                              }}},
            {UP::vca_decay, {"vca_decay", 0.5f, 0, true, 5, 7, [this](float v) { // 8192 max
                                 vcaAR.setTime(audio::envelope::ADSFRState::DECAY, v);
                                 std::cout << "setting decay to " << v << " ms" << std::endl;
                             }}},
            {UP::vca_sustain, {"vca_sustain", 0.8f, 0, false, 0, 1, [this](float v) {
                                   vcaAR.setLevel(audio::envelope::ADSFRState::SUSTAIN, v);
                                   std::cout << "setting sustain to " << v << std::endl;
                               }}},
            {UP::vca_fade, {"vca_fade", 0.3f, 0, false, 0, 1, [this](float v) {
                                vcaAR.setLeak(audio::envelope::ADSFRState::FADE, v);
                                std::cout << "setting fade to " << v << std::endl;
                            }}},
            {UP::vca_release, {"vca_release", 0.7f, 0, true, 10, 8, [this](float v) { //!!
                                   vcaAR.setTime(audio::envelope::ADSFRState::RELEASE, v);
                                   std::cout << "setting release to " << v << " ms" << std::endl;
                               }}},
            {UP::vca_spatial, {"vca_spatial", 0.2f, 0, false, 0, 1, [this](float v) {
                                   vcaSpatial = v;
                               }}},
            {UP::lfo1_shape, {"lfo1_shape", 0.0f, audio::lfo::LFOShape::_count, false, 0, audio::lfo::LFOShape::_count - 1, [this](float v) {
                                  // of what really..
                                  std::cout << "setting lfo1-shape to " << v << std::endl;
                                  lfo1.setShape(static_cast<audio::lfo::LFOShape>(static_cast<int>(v)));
                              }}},
            {UP::lfo1_speed, {"lfo1_speed", 0.5f, 0, true, 100, 9, [this](float v) {
                                  std::cout << "setting lfo1-speed (mHz) to " << v << std::endl;
                                  lfo1.setSpeed(v); // in mHz.
                              }}},
            {UP::lfo1_routing, {"lfo1_routing", 0.3f, 6, false, 0, 5, [this](float v) {
                                    lfo1Routing = static_cast<LFO1::Routing>(static_cast<int>(v));
                                    std::cout << "setting lfo1-routing to " << v << std::endl;
                                }}},
            {UP::lfo1_depth, {"lfo1_depth", 0.0f, 0, false, 0, 1, [this](float v) {
                                  // of what really..
                                  std::cout << "setting lfo1-depth to " << v << std::endl;
                                  lfo1depth = v;
                              }}},
            {UP::lfo2_shape, {"lfo2_shape", 0.0f, audio::lfo::LFOShape::_count, false, 0, audio::lfo::LFOShape::_count - 1, [this](float v) {
                                  // of what really..
                                  lfo2.setShape(static_cast<audio::lfo::LFOShape>(static_cast<int>(v)));
                              }}},
            {UP::lfo2_speed, {"lfo2_speed", 0.5f, 0, true, 100, 9, [this](float v) {
                                  std::cout << "setting lfo1-speed (mHz) to " << v << std::endl;
                                  lfo2.setSpeed(v); // in mHz.
                              }}},
            {UP::lfo2_routing, {"lfo2_routing", 0.0, LFO1::Routing::_count, false, 0, LFO1::Routing::_count - 1, [this](float v) {
                                    lfo2Routing = static_cast<LFO2::Routing>(static_cast<int>(v));
                                }}},
            {UP::lfo2_depth, {"lfo2_depth", 0.0f, 0, false, 0, 1, [this](float v) {
                                  // of what really..
                                  lfo2depth = v;
                              }}},
            {UP::peg_atime, {"peg_atime", 0.1f, 0, true, 10, 8, [this](float v) { // 8192 max
                                 // pegAR.setTime(audio::envelope::ASRState::ATTACK, v);
                             }}},
            {UP::peg_rtime, {"peg_rtime", 0.1f, 0, true, 10, 8, [this](float v) { // 8192 max
                                 // pegAR.setTime(audio::envelope::ASRState::RELEASE, v);
                             }}}};
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
        dist = dist * dist; // skip polarity.. But we'll get problems as dist² > 5.
        dist = fmin(4.0f, dist * dist);
        buffer[i] = synthBuffer[i] * (5 - dist) * 0.2f;
        // 0.1 => 0.001 => (5 - 0.01) * 0.1 => 0.499 => 0.0499
        // 0.5 => 0.25 => (5 - 0.25) * 0.1 => 0.475 => 0.287
        // 2 => 4 => (5 - 4) * 0.1 => 0.1 => 0.2
    }
    // motherboard-stuff..
    lfo1.updatePhase();
    lfo2.updatePhase();

    // debugging
    if (false) {
        for (std::size_t i = 0; i < bufferSize; i++) {
            buffer[i] += AudioMath::noise() * 0.01f - 0.005f;
        }
    }

    // not always stereo for now..
    return true;
}

void Model::addToSample(std::size_t sampleIdx, float val) {
    // use local buffer for spead. Possibly double..
    this->synthBuffer[sampleIdx] += val;
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
