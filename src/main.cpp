#include "constants.h"
#include "core/ErrorLogger.h"
#include "core/PlayerEngine.h"
#include "core/Rack.h"
#include "core/audio/AudioMath.h"
#include "core/messages/MessageReciever.h"
#include "core/messages/MessageSender.h"
#include "core/messages/WebSocketPusher.h"
#include "core/parameters/SettingsManager.h"
#include "crow.h"
#include "drivers/AudioDriver.h"
#include "drivers/FileDriver.h"
#include "drivers/MidiDriver.h"
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
MessageReciever sMessageReciever(8);
MessageSender sMessageSender(8);
WebSocketPusher wsPusher(sMessageSender, nullptr); // Initialize without connection

AudioDriver sAudioDriver;
MidiDriver sMidiDriver;

// Entry point of the program
int main() {
    // Create the object
    crow::SimpleApp api;
    // Remove default signal handler
    api.signal_clear();
    // Add our custom handler
    signal(SIGINT, signal_handler);
    //
    std::unordered_map<std::string, VariantType> deviceSettings;
    // initialize
    deviceSettings["buffer_size"] = 128;
    deviceSettings["audio_sr"] = 48000;
    deviceSettings["audio_device"] = 1;
    deviceSettings["midi_device"] = 1;
    deviceSettings["http_port"] = 18080;

    // Load settings from JSON and override default settings

    SettingsManager::loadJsonToSettings("device.json", true, deviceSettings);
    // this could be and endpoint..
    //  now what playerEngine is initiated, setup the callback.
    unsigned long framesPerBuffer = std::get<int>(deviceSettings["buffer_size"]);
    sAudioDriver.registerCallback([framesPerBuffer](float *buffer, unsigned long) {
        // Call the static renderNextBlock method from sPlayerEngine
        sPlayerEngine.renderNextBlock(buffer, framesPerBuffer);
    });
    // Start the audio driver
    // sAudioDriver.start();
    //
    sPlayerEngine.BindMessageReciever(sMessageReciever);
    sPlayerEngine.BindMessageSender(sMessageSender);
    AudioMath::generateLUT(); // sets up a sine lookup table of 1024 elements.
    //
    crowSetupEndpoints(api, sPlayerEngine, sAudioDriver, sMidiDriver, sMessageReciever, sMessageSender, wsPusher);
    int httpPort = std::get<int>(deviceSettings["http_port"]);
    std::thread server_thread([&api, httpPort]() { api.port(httpPort).run(); });
    while (!shutdown_flag.load()) {
        if (false & DEBUG_MODE) {
            std::cout << "housekeeping.. " << std::endl;
        }
        std::this_thread::sleep_for(std::chrono::seconds(3));
    }
    std::cout << "Stopping the server..." << std::endl;
    api.stop();
    std::this_thread::sleep_for(std::chrono::seconds(1));
    if (server_thread.joinable()) {
        server_thread.join();
    }
    std::cout << "Server stopped gracefully." << std::endl;
    return 0;
}
