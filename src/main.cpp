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
#include "endpointsCrow.h"
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
    // initialize
    deviceSettings["buffer_size"] = "128";
    deviceSettings["audio_sr"] = "48000";
    deviceSettings["audio_device"] = "1";
    deviceSettings["midi_device"] = "1";
    deviceSettings["http_port"] = "18080";
    deviceSettings["scroller_cc"] = "84";
    deviceSettings["scroller_dials"] = "1,2,3,4,5,6,7";
    // so who's the reciever of the scroller dials? The rack? Could have different scrollpos for different racks so yeah..
    // so playerEngine knows the scrollerDials. Rack knows its scroll-position. incoming cc checked with dials.
    // if in dials, remap to scrolled name. (huh). If not in scroller, just?
    // actually, this should possibly be in the synth-interface (base-class), no scroll in eventor/effect.
    // possibly, a synth could be scroller-enabled or not. dunno..

    // Load settings from JSON and override default settings

    // Kickstart the StudioRunner (low-priority job scheduler)
    sStudioRunner.start();

    SettingsManager::loadJsonToSettings("device.json", true, deviceSettings);
    // this could be and endpoint..
    //  now what playerEngine is initiated, setup the callback.
    unsigned long framesPerBuffer = static_cast<unsigned long>(std::stoi(deviceSettings["buffer_size"]));
    /*sAudioManager.registerCallback([framesPerBuffer](float *buffer, unsigned long) {
        // Call the static renderNextBlock method from sPlayerEngine
        sPlayerEngine.renderNextBlock(buffer, framesPerBuffer);
    });*/
    // Start the audio driver
    // sAudioDriver.start();
    //
    sPlayerEngine.bindMessageInBuffer(sMessageInBuffer);
    sPlayerEngine.bindMessageOutBuffer(sMessageOutBuffer);
    sPlayerEngine.bindErrorBuffer(sAudioErrorBuffer);
    sPlayerEngine.bindMidiManager(sMidiManager);
    sErrorHandler.start();
    sPlayerEngine.initializeRacks();

    sPlayerEngine.setupRackWithSynth(0, "Monolith");

    // AudioMath::generateLUT(); // sets up a sine lookup table of 1024 elements.
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
