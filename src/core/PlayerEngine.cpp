#include "PlayerEngine.h"
#include "Rack.h"

PlayerEngine::PlayerEngine()
    : noiseVolume(0.2f),
      hRotator() {
}

void PlayerEngine::BindMessageReciever(MessageReciever &hMessageReciever) {
    messageReciever = &hMessageReciever;
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
    // Get the current time in microseconds
    // Calculate time for next frame based on sample rate and numFrames
    double frameDurationMicroSec = (numFrames * 1'000'000.0) / TPH_DSP_SR;
    auto nextFrameTime = std::chrono::high_resolution_clock::now() + std::chrono::microseconds(static_cast<long long>(frameDurationMicroSec));
    //
    if (clockReset) {
        clockResetMethod(); // Reset the clock (requested by play- or stop-command)
    }
    int outerCnt = TPH_AUDIO_BUFFER_SIZE / TPH_RACK_RENDER_SIZE;
    for (int outer = 0; outer < outerCnt; ++outer) {
        pollMidiIn(); // large audio buffers is bad for midi-in accuracy. Fly low.
        turnRackAndRender();
        sumToMaster(buffer, numFrames, outer);
    }
    //
    auto endTime = std::chrono::high_resolution_clock::now();
    auto timeLeftUs = std::chrono::duration_cast<std::chrono::microseconds>(nextFrameTime - endTime).count();
    if (timeLeftUs > 1500) {
        // check if there's any parameter - permanent or not(?) that should be forwarded to a rack module..
        auto optionalMessage = messageReciever->pop();
        if (optionalMessage) { // Check if a message was retrieved
            newMessage = *optionalMessage;
            std::cout << "New message received and stored," << newMessage.paramName << "value:" << newMessage.paramValue << std::endl;
            racks[newMessage.rackId]->passParamToUnit(
                Rack::stringToUnitType(newMessage.target),
                newMessage.paramName,
                newMessage.paramValue);
        }
    }
}

void PlayerEngine::sumToMaster(float *buffer, unsigned long numFrames, int outer) {
    int offset = outer * 2 * TPH_RACK_RENDER_SIZE; // Offset in the master buffer

    for (std::size_t i = 0; i < MAX_RACKS; ++i) {
        if (racks[i]) { // Check if the rack is initialized (i.e., not null)
            for (std::size_t sample = 0; sample < TPH_RACK_RENDER_SIZE * 2; sample += 2) {
                *(buffer + offset + sample) = racks[i]->audioBuffer[sample];
                *(buffer + offset + sample + 1) = racks[i]->audioBuffer[sample + 1];
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
        // If there's a message available, and a rack receiving midi..
        if (test && this->rackReceivingMidi >= 0) {
            // affect eventors and effects with midiCC? currently no..
            racks[this->rackReceivingMidi]->parseMidi(newMessage.cmd, newMessage.param1, newMessage.param2);
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
