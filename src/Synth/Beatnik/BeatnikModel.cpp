#include "./BeatnikModel.h"

namespace Synth::Beatnik {
constexpr int VOICE_COUNT = 12;

// Constructor to accept buffer and size
Model::Model() {
    setupParams(UP::up_count); // creates the array with attributes and lambdas for parameters - NOT INTERFACE
    SynthBase::initParams();
    SynthInterface::setupCCmapping("Beatnik");
    reset();                     // setup luts. Must come before voice allocation.
    voices.reserve(VOICE_COUNT); // Preallocate memory for voices
    for (int i = 0; i < VOICE_COUNT; ++i) {
        voices.emplace_back(*this); // Pass reference to Model
    }
}

void Model::reset() {
}

// well maybe its not setting after all.. more like a factory needed here, but factory have no access to the model..
// so this class should rather be called mountObject
void Model::updateSetting(const std::string &type, void *object, uint32_t size, bool isStereo, Destructor::Record &recordDelete) {
    // Hash the key
    std::cout << "at update setting in Beatnik!" << std::endl;
    uint32_t keyFNV = Utils::Hash::fnv1a(type);
    // Extract the first character
    char firstChar = type[0];
    int sampleID = firstChar - 'a';
    std::cout << "sample id extracted to : " << sampleID << std::endl;
    auto *sample = reinterpret_cast<audio::sample::SimpleSample *>(object);
    if (samples[sampleID].getDataPointer()) {
        std::cout << "deleting sample.. " << std::endl;
        recordDelete.ptr = const_cast<float *>(samples[sampleID].getDataPointer());
        recordDelete.deleter = [](void *ptr) { delete[] static_cast<float *>(ptr); }; // Delete sample - not simplesample.
        samples[sampleID].unmountSample();
    }
    // Assign the new sample
    std::cout << "sample length is: " << sample->length << std::endl;
    samples[sampleID] = *sample;
}

void Model::bindBuffers(float *audioBuffer, std::size_t bufferSize) {
    this->buffer = audioBuffer;
    this->bufferSize = bufferSize;
}

void Model::setupParams(int upCount) {
    // um. we need something clever here to route parameter to sample-setting.
    // not clear yet the routing here. maybe samples <-> voices
    if (SynthBase::paramDefs.empty()) {
        // after declaration, indexation requested, see below..
        SynthBase::paramDefs = {
            {UP::a_pan, {"a_pan", 0.0f, 0, false, -1, 1, [this](float v) {
                             // what would i need to do?
                         }}},
            {UP::b_pan, {"b_pan", 0.0f, 0, false, -1, 1, [this](float v) {
                             // what would i need to do?
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
        // One-shot samples so no action..
        break;
    case 0xb0:
        SynthInterface::handleMidiCC(param1, fParam2);
        break;
    case 0xe0: {
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
        dist = dist * dist;         // skip polarity..
        buffer[i] = synthBuffer[i]; // * (5 - dist) * 0.1f;
    }

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

} // namespace Synth::Beatnik
