#include "PlayerEngine.h"
#include "ErrorWriter.h"

PlayerEngine::PlayerEngine()
    : noiseVolume(0.2f), isWritingMessage(false), hRotator(), errorWriter_(*this) {
    this->rackReceivingMidi = 0; // meh
    this->loadAvg = 0.0f;
}

void PlayerEngine::reset() {
    noiseVolume = 0.2f;
}

void PlayerEngine::ping() {
    std::cout << "ping from player engine" << std::endl;
}

void PlayerEngine::doReset() {
    std::cout << "Resetting player engine" << std::endl;
    reset(); // Calls the instance method
}

void PlayerEngine::initializeRacks() {
    for (int i = 0; i < TPH_RACK_COUNT; ++i) {
        racks[i].setPlayerEngine(*this);
        racks[i].setErrorWriter(errorWriter_); // Pass the ErrorWriter reference
    }
}

void PlayerEngine::bindMessageInBuffer(MessageInBuffer &hMessageInBuffer) {
    messageInBuffer = &hMessageInBuffer;
}

void PlayerEngine::bindMessageOutBuffer(MessageOutBuffer &hMessageOutBuffer) {
    messageOutBuffer = &hMessageOutBuffer;
}

void PlayerEngine::bindErrorBuffer(AudioErrorBuffer &hAudioErrorBuffer) {
    audioErrorBuffer = &hAudioErrorBuffer;
}

void PlayerEngine::bindMidiManager(MidiManager &hMidiManager) {
    midiManager = &hMidiManager;
}

bool PlayerEngine::sendMessage(int rackId, const char *target, float paramValue, const char *paramName, const char *paramLabel) {
    if (isWritingMessage.exchange(true, std::memory_order_acquire)) {
        // Return false to indicate the message couldn't be sent
        return false;
    }

    MessageOut message;

    message.rackId = rackId;
    message.paramValue = paramValue;

    // Safely copy the strings
    strncpy(message.target, target, msgOutTargetSize - 1);
    message.target[msgOutTargetSize - 1] = '\0';

    strncpy(message.paramName, paramName, msgOutParamNameSize - 1);
    message.paramName[msgOutParamNameSize - 1] = '\0';

    strncpy(message.paramLabel, paramLabel, msgOutParamLabelSize - 1);
    message.paramLabel[msgOutParamLabelSize - 1] = '\0';

    // Push the message to the queue
    messageOutBuffer->push(message);

    // Release the write lock
    isWritingMessage.store(false, std::memory_order_release);
    return true;
}

void PlayerEngine::sendError(int code, const std::string &message) {
    // dunno if this should be kept. But still, units have to be context-aware..
    audioErrorBuffer->addAudioError(code, message);
}

bool PlayerEngine::loadSynth(SynthBase *&synth, int rackID) {
    // Delegate synth setup to the rack
    bool result = racks[rackID].setSynth(synth);
    // Reset the caller's pointer to avoid accidental reuse
    synth = nullptr;
    return result;
}

bool PlayerEngine::setupRackWithSynth(int rackId, const std::string &synthName) {
    // Check if the rack already exists
    racks[rackId].setSynthFromStr(synthName);
    sendError(200, "real audio error hello");
    //   Now, setup the synth for the rack
    // racks[rackId].setEffect("Delay"); // Chorus
    // racks[rackId].setEffect("Delay", 2);
    //   to be improved..
    rackReceivingMidi = 0;
    return false;
}

float PlayerEngine::getLoadAvg() {
    // this should really be atomic..
    return this->loadAvg;
}

void PlayerEngine::renderNextBlock(float *buffer, unsigned long numFrames) {
    // Get the current time in microseconds
    // Calculate time for next frame based on sample rate and numFrames
    int64_t frameDurationMicroSec = static_cast<long>(numFrames * (1'000'000.0 / TPH_DSP_SR));
    std::chrono::time_point nextFrameTime = std::chrono::high_resolution_clock::now() + std::chrono::microseconds(frameDurationMicroSec);

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
    std::chrono::time_point endTime = std::chrono::high_resolution_clock::now();
    int64_t timeLeftUs = std::chrono::duration_cast<std::chrono::microseconds>(nextFrameTime - endTime).count();

    // Time used for rendering in microseconds
    int64_t timeUsedUs = frameDurationMicroSec - timeLeftUs;

    // Clamp the timeUsedUs to avoid negative values (if timeLeftUs is greater than frameDurationMicroSec)
    timeUsedUs = std::max<int64_t>(0, timeUsedUs); // Ensure no negative values

    // Calculate raw load average as the ratio of time used to frame duration (as a float for precision)
    double rawLoad = (100.0 * static_cast<double>(timeUsedUs)) / static_cast<double>(frameDurationMicroSec);

    // Smoothing with a factor (alpha), for example, alpha = 0.1 for smooth update
    constexpr double alpha = 0.1;

    // Update loadAvg with smoothing
    this->loadAvg = (alpha * rawLoad) + ((1 - alpha) * this->loadAvg);

    debugCnt++;
    if ((debugCnt & (1024 - 1)) == 0) {
        sendError(100, "Stat: " + std::to_string(this->loadAvg));
        sendError(105, "TimeLeftUs: " + std::to_string(timeLeftUs));
    }
    // this code not working so fake it..
    if (timeLeftUs > 500) {
        // check if there's any parameter - permanent or not(?) that should be forwarded to a rack module..
        auto optionalMessage = messageInBuffer->pop();
        if (optionalMessage) { // Check if a message was retrieved
            newMessage = *optionalMessage;
            std::cout << "New message received," << newMessage.paramName << "value:" << newMessage.paramValue << std::endl;
            // this->sendError(404, "duh message recieved");
            racks[newMessage.rackId].passParamToUnit(
                Rack::stringToUnitType(newMessage.target),
                newMessage.paramName,
                newMessage.paramValue);
            sendMessage(1, "synth", newMessage.paramValue, newMessage.paramName, "yaba daba");
        }
    }
}

std::string PlayerEngine::getSynthParams(int rackId) {
    // this shoud really require rack to be non-playing..
    return this->racks[rackId].synth->getParamDefsAsJSON();
}

void PlayerEngine::sumToMaster(float *buffer, unsigned long numFrames, int outer) {
    int offset = outer * 2 * TPH_RACK_RENDER_SIZE; // Offset in the master buffer
    for (std::size_t sample = 0; sample < TPH_RACK_RENDER_SIZE * 2; sample += 2) {
        *(buffer + sample) = 0;
        *(buffer + sample + 1) = 0;
    }

    for (std::size_t i = 0; i < TPH_RACK_COUNT; ++i) {
        if (racks[i].enabled) {
            for (std::size_t sample = 0; sample < TPH_RACK_RENDER_SIZE * 2; sample += 2) {
                *(buffer + offset + sample) += racks[i].audioBuffer[sample];
                *(buffer + offset + sample + 1) += racks[i].audioBuffer[sample + 1];
            }
        }
    }
}

void PlayerEngine::clockResetMethod() {
    hRotator.pulse = 0; // Reset the rotator's pulse

    for (std::size_t i = 0; i < TPH_RACK_COUNT; ++i) {
        if (racks[i].enabled) {
            racks[i].clockReset(); // Reset each rack's clock
        }
    }

    clockReset = false; // Reset the clockReset flag
}

void PlayerEngine::updateMidiSettings(const std::string &strScrollerCC, const std::string &strScrollerDials) {
    this->scrollerCC = std::stoi(strScrollerCC);
    std::cout << "setting scrolerCC to " << this->scrollerCC << std::endl;
    // Initialize the scroller dials to zero
    std::fill(std::begin(ccScrollerDials), std::end(ccScrollerDials), 0);

    // Parse the comma-separated scroller dial values
    size_t start = 0, end = 0;
    int index = 0;

    while ((end = strScrollerDials.find(',', start)) != std::string::npos && index < 7) {
        ccScrollerDials[index++] = std::stoi(strScrollerDials.substr(start, end - start));
        start = end + 1;
    }

    // Add the last value (or only value if no commas were found)
    if (index < 7 && start < strScrollerDials.size()) {
        ccScrollerDials[index++] = std::stoi(strScrollerDials.substr(start));
    }
}

bool PlayerEngine::pollMidiIn() {
    MidiMessage newMessage;
    this->rackReceivingMidi = 1;
    if (this->rackReceivingMidi >= 0) {
        while (midiManager->getNextMessage(newMessage)) {
            // here, recover note-on with vel 0 to note off.
            if ((newMessage.cmd & 0xf0) == 0x90 && newMessage.param2 == 0) {
                newMessage.cmd -= 0x10;
            }
            const u_int8_t channel = static_cast<u_int8_t>(newMessage.cmd) & 0x03; // 0x03 for prototype - 4 racks..
            if (racks[channel].enabled) {
                // if cc-cmd, see if it should be re-routed.
                if ((newMessage.cmd & 0xf0) == 0xb0) { // note parenthesis!!
                    newMessage.param1 = remapCC(newMessage.param1, newMessage.param2);
                }
                if (newMessage.param1 != 255) {
                    racks[channel].parseMidi(newMessage.cmd, newMessage.param1, newMessage.param2);
                }
            }
            // racks[this->rackReceivingMidi].parseMidi(newMessage.cmd, newMessage.param1, newMessage.param2);
            //      this->sendError(200, "midi recieved");
            if (newMessage.cmd == 0x90)
                sendMessage(1, "synth", newMessage.param1, "note on", "see this? :)");
        }
    }
    return true; // means nothing.. Return the result of getMessage
}

u_int8_t PlayerEngine::remapCC(u_int8_t originalCC, u_int8_t param2) {
    // Check if the CC corresponds to a pot
    // std::cout << "orgCC is" << static_cast<int>(originalCC) << " and ccSP is " << static_cast<int>(scrollerCC) << std::endl;
    if (originalCC == scrollerCC) {
        u_int8_t testScroller = round(param2 * (8.0f / 127.0f));
        if ((testScroller & 0x01) == 0x00) {
            // at value (not threshold), now shift.
            testScroller = testScroller >> 1;
            if (ccScrollerPosition != testScroller) {
                std::cout << "setting pager to " << static_cast<int>(testScroller) << std::endl;
                ccScrollerPosition = testScroller;
            }
        }
        return 255; // surpress later processing
    }
    for (int i = 0; i < 7; i++) {
        if (originalCC == ccScrollerDials[i]) {
            // Remap based on scroller position
            // what if we hard-code it, big time.. 32->
            uint8_t newCC = 20 + ccScrollerPosition * 10 + i;
            std::cout << "routed CC:" << static_cast<int>(newCC) << std::endl;
            return newCC;
        }
    }
    return originalCC; // No remapping needed
}

void PlayerEngine::turnRackAndRender() {
    for (std::size_t i = 0; i < TPH_RACK_COUNT; ++i) {
        if (racks[i].enabled) {                     // Check if the rack is initialized (i.e., not null)
            racks[i].probeNewClock(hRotator.pulse); // Call probeNewClock

            if (isPlaying) {
                racks[i].probeNewTick(hRotator.pulse); // Call probeNewTick if playing
            }

            racks[i].render(1); // Call render
        }
    }
}