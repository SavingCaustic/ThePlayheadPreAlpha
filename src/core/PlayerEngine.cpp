#include "PlayerEngine.h"
#include "Rack.h"

PlayerEngine::PlayerEngine()
    : noiseVolume(0.2f),
      hRotator() {
    // racks.fill(nullptr); // Initialize racks array with nullptr
}

void PlayerEngine::reset() {
    noiseVolume = 0.2f;
}

void PlayerEngine::midiEnable(MidiDriver *midiDriver) {
    this->hMidiDriver = midiDriver;
}

void PlayerEngine::midiDisable() {
    this->hMidiDriver = nullptr;
}

void PlayerEngine::ping() {
    std::cout << "ping from player engine" << std::endl;
}

void PlayerEngine::doReset() {
    std::cout << "Resetting player engine" << std::endl;
    reset(); // Calls the instance method
}

void PlayerEngine::testRackSetup() {
    this->setupRackWithSynth(0, "Dummy");
}

bool PlayerEngine::setupRackWithSynth(int rackId, const std::string &synthName) {
    // Check if the rack already exists
    auto &rackPtr = racks[rackId];
    std::cout << "setting up rack with synth now" << std::endl;
    if (!rackPtr) {
        // Create a new Rack if it doesn't exist
        rackPtr = std::make_unique<Rack>(*this);
    }
    rackPtr->setSynth(synthName);
    //  Now, setup the synth for the rack
    return false;
}

bool PlayerEngine::loadRack(std::unique_ptr<Rack> rack, std::size_t position) {
    if (position < MAX_RACKS) {
        racks[position] = std::move(rack);
        return true;
    }
    return false;
}

Rack *PlayerEngine::getRack(std::size_t position) const {
    if (position < MAX_RACKS) {
        return racks[position].get();
    }
    return nullptr;
}

void PlayerEngine::render(float *buffer, unsigned long numFrames) {
    // not used
    for (unsigned long i = 0; i < numFrames; ++i) {
        *buffer++ = noiseVolume * (((float)rand() / RAND_MAX) * 2.0f - 1.0f);
        *buffer++ = noiseVolume * (((float)rand() / RAND_MAX) * 2.0f - 1.0f);
    }
    if (noiseVolume > 0.2f) {
        noiseVolume -= 0.02f;
    }
}

void PlayerEngine::noteOnDemo() {
    noiseVolume = 0.6f;
}

void PlayerEngine::renderNextBlock(float *buffer, unsigned long numFrames) {
    if (clockReset) {
        clockResetMethod(); // Reset the clock
    }
    int outerCnt = TPH_AUDIO_BUFFER_SIZE / TPH_RACK_RENDER_SIZE;
    for (int outer = 0; outer < outerCnt; ++outer) {
        pollMidiIn(); // large audio buffers is bad for midi-in accuracy. Fly low.
        turnRackAndRender();
        // sumToMaster(buffer, numFrames, outer);
        // temporary fix:
        int offset;
        offset = outer * 2 * TPH_RACK_RENDER_SIZE;
        for (std::size_t i = 0; i < MAX_RACKS; ++i) {
            if (racks[i]) { // Check if the rack is initialized (i.e., not null)
                for (std::size_t sample = 0; sample < TPH_RACK_RENDER_SIZE * 2; sample += 2) {
                    *(buffer + offset + sample) = racks[i]->audioBuffer[sample];
                    *(buffer + offset + sample + 1) = racks[i]->audioBuffer[sample + 1];
                    //*(buffer + offset + sample) = 0.3 * (((float)rand() / RAND_MAX) * 2.0f - 1.0f);
                    //*(buffer + offset + sample + 1) = 0.5 * (((float)rand() / RAND_MAX) * 2.0f - 1.0f);
                }
            }
        }
    }
}

void PlayerEngine::sumToMaster(float *buffer, unsigned long numFrames, int outer) {
    // this isn't working strangely..
    int offset = outer * 2 * TPH_RACK_RENDER_SIZE; // Offset in the master buffer
    for (std::size_t i = 0; i < MAX_RACKS; ++i) {
        if (racks[i]) { // Check if the rack is initialized (i.e., not null)
            for (std::size_t sample = 0; sample < TPH_RACK_RENDER_SIZE * 2; sample += 2) {
                buffer[offset + sample] += racks[i]->audioBuffer[sample];
                buffer[offset + sample + 1] += racks[i]->audioBuffer[sample + 1];
            }
        }
    }
}

void PlayerEngine::clockResetMethod() {
    hRotator.pulse = 0; // Reset the rotator's pulse

    for (std::size_t i = 0; i < MAX_RACKS; ++i) {
        if (racks[i]) {
            racks[i]->clockReset(); // Reset each rack's clock
        }
    }

    clockReset = false; // Reset the clockReset flag
}

bool PlayerEngine::pollMidiIn() {
    if (hMidiDriver) {
        MidiMessage newMessage;
        bool test = hMidiDriver->bufferRead(newMessage);
        // If there's a message available
        if (test) {
            racks[0]->parseMidi(newMessage.cmd, newMessage.param1, newMessage.param2);
        }
        return test; // Return the result of getMessage
    }
    return false; // Return false if hMidiDriver is nullptr
}

void PlayerEngine::turnRackAndRender() {
    for (std::size_t i = 0; i < MAX_RACKS; ++i) {
        if (racks[i]) {                              // Check if the rack is initialized (i.e., not null)
            racks[i]->probeNewClock(hRotator.pulse); // Call probeNewClock

            if (isPlaying) {
                racks[i]->probeNewTick(hRotator.pulse); // Call probeNewTick if playing
            }

            racks[i]->render(1); // Call render
        }
    }
}
