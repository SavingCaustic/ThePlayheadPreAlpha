#include "Synth/SynthBase.h"
#include "Synth/SynthFactory.h"
#include "api/endpointsCrow.h"
#include "constants.h"
#include "core/audio/AudioMath.h"
#include "core/errors/AudioErrorBuffer.h"
#include "core/errors/ErrorBuffer.h"
#include "core/errors/ErrorHandler.h"
#include "core/errors/ErrorLogger.h"
#include "core/messages/MessageInBuffer.h"
#include "core/messages/MessageOutBuffer.h"
#include "core/messages/MessageOutReader.h"
#include "core/parameters/SettingsManager.h"
#include "core/player/PlayerEngine.h"
#include "core/player/Rack.h"
#include "core/runner/StudioRunner.h"
#include "crow.h"
#include "drivers/AudioDriver.h"
#include "drivers/AudioManager.h"
#include "drivers/FileDriver.h"
#include "drivers/MidiManager.h"
#include <atomic>
#include <chrono>
#include <filesystem>
#include <iostream>
#include <nlohmann/json.hpp>
#include <string>
#include <thread>
#include <unordered_map>
#include <variant>

/*
 * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  ### # # ###   ##  #    #  # # # # ###  #  ##   *
 *   #  ### ##    ### #   ###  #  ### ##  ### # #  *
 *   #  # # ###   #   ### # #  #  # # ### # # ##   *
 * * * * * * * * * * * * * * * * * * * * * * * * * *
 */

#define DEBUG_MODE 1

// Define the global shutdown flag. Set by endpoint shutdown and signal_handler below
std::atomic<bool> shutdown_flag(false);

// Custom signal handler. Working well.
void signal_handler(int signal) {
    if (signal == SIGINT) {
        std::cout << "Caught SIGINT (Ctrl+C), setting shutdown flag..." << std::endl;
        shutdown_flag = true;
    }
}

// Global objects. Could possibly be hierarchial but could lead to poor testing environments.
PlayerEngine sPlayerEngine;
MessageInBuffer sMessageInBuffer(8);
MessageOutBuffer sMessageOutBuffer;
MessageOutReader sMessageOutReader(sMessageOutBuffer, nullptr); // Initialize without connection

AudioDriver sAudioDriver;
AudioManager sAudioManager(sAudioDriver, sPlayerEngine);
MidiManager sMidiManager;

// Add global ErrorHandler
AudioErrorBuffer sAudioErrorBuffer;                            // Error buffer used by the audio engine
ErrorBuffer sErrorBuffer;                                      // Main error buffer for logging
ErrorHandler sErrorHandler(&sAudioErrorBuffer, &sErrorBuffer); // Error handler with threads

// StudioRunner - performing various low-prio task (micro-scheduler)
StudioRunner sStudioRunner(sMidiManager, sAudioManager);

// Entry point of the program
int main() {
    // Create the object
    crow::SimpleApp api;
    // Remove default signal handler
    api.signal_clear();
    // Add our custom handler
    signal(SIGINT, signal_handler);
    //
    std::unordered_map<std::string, std::string> deviceSettings;
    SettingsManager::jsonRead(deviceSettings, "device.json");
    // initialize
    AudioMath::initialize();
    // Kickstart the StudioRunner (low-priority job scheduler)
    sStudioRunner.reset();

    // meh - almost factory here..
    sPlayerEngine.updateMidiSettings(deviceSettings["scroller_cc"], deviceSettings["scroller_dials"]);
    // postpone this so we wait with the audio-thread..
    // sStudioRunner.start();

    // this could be and endpoint..
    //  now what playerEngine is initiated, setup the callback.
    unsigned long framesPerBuffer = static_cast<unsigned long>(std::stoi(deviceSettings["buffer_size"]));

    //
    sPlayerEngine.bindMessageInBuffer(sMessageInBuffer);
    sPlayerEngine.bindMessageOutBuffer(sMessageOutBuffer);
    sPlayerEngine.bindErrorBuffer(sAudioErrorBuffer);
    sPlayerEngine.bindMidiManager(sMidiManager);
    sErrorHandler.start();
    sPlayerEngine.initializeRacks();

    // this should go, but debugging now..
    if (false) {
        sPlayerEngine.setupRackWithSynth(0, "Monolith");
    } else {
        SynthBase *synth = nullptr;
        SynthFactory::setupSynth(synth, "Subreal");
        sPlayerEngine.loadSynth(synth, 0); // maybe return an nullptr from loadSynth?
        // synth->parseMidi(0x90, 0x40, 0x050);
        //  set up another synth..
        SynthBase *synth2 = nullptr;
        SynthFactory::setupSynth(synth2, "Monolith");
        SynthFactory::patchLoad(synth2, "Portabello");
        sPlayerEngine.loadSynth(synth2, 1);
        // synth2->parseMidi(0x90, 0x44, 0x050);
        //   set up another synth..
    }
    // Start the audio driver
    sAudioManager.mountPreferedOrDefault(deviceSettings["audio_device"]);
    sStudioRunner.start();
    //
    crowSetupEndpoints(api, sPlayerEngine, sAudioManager, sMidiManager, sMessageInBuffer, sMessageOutBuffer, sMessageOutReader, sErrorBuffer);
    int httpPort = std::stoi(deviceSettings["http_port"]);
    std::thread server_thread([&api, httpPort]() { api.port(httpPort).run(); });
    while (!shutdown_flag.load()) {
        if (false & DEBUG_MODE) {
            std::cout << "housekeeping.. " << std::endl;
            // should be errorBuffer sErrorHandler.addError(200, "testing error");
        }
        std::this_thread::sleep_for(std::chrono::seconds(3));
    }
    std::cout << "beginning shutdown.." << std::endl;
    sMessageOutReader.stop();
    std::cout << "stopped web-socket feeder" << std::endl;
    //
    std::cout << "Stopping the server..." << std::endl;
    api.stop();
    std::this_thread::sleep_for(std::chrono::seconds(1));
    if (server_thread.joinable()) {
        server_thread.join();
    }
    sErrorHandler.stopThreads();

    sStudioRunner.stop();
    std::cout << "Server stopped gracefully." << std::endl;
    return 0;
}
