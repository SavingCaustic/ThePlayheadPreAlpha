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

void PlayerEngine::doReset() {
    std::cout << "Resetting player engine" << std::endl;
    reset(); // Calls the instance method
}

bool PlayerEngine::setupRackWithSynth(int rackId, const std::string &synthName) {
    auto rack = getRack(rackId);
    if (rack) {
        return rack->setSynth(synthName);
    }
    return false; // Rack not found
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
        pollMidiIn();
        turnRackAndRender();
        // sumToMaster(&buffer, numFrames, outer);
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
    // Poll MIDI input and forward to appropriate rack
    return false; // Placeholder return value
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

void PlayerEngine::sumToMaster(float *buffer, int outer) {
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
