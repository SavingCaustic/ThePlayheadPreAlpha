#include "constants.h"
#include "core/AudioMath.h"
#include "core/PlayerEngine.h"
#include "core/Rack.h"
#include "crow.h"
#include "drivers/AudioDriver.h"
#include "drivers/MidiDriver.h"
#include <atomic>
#include <chrono>
#include <iostream>
#include <thread>

/*      ### # # ###   ##  #    #  # # # # ###  #  ##
         #  ### ##    ### #   ###  #  ### ##  ### # #
         #  # # ###   #   ### # #  #  # # ### # # ##     */

#define DEBUG_MODE 1

// Define the global shutdown flag. Set by endpoint shutdown (not signals)
std::atomic<bool> shutdown_flag(false);

// Define and initialize the static variables outside the class
int AudioDriver::gNumNoInputs = 0;
double AudioDriver::vol = 0.7;
PlayerEngine *rtPlayerEngine; // Global pointer to PlayerEngine

// Global instances of drivers
AudioDriver *hAudioDriver;
MidiDriver *hMidiDriver;

// Entry point of the program
int main() {
    // Initialize the global PlayerEngine instance
    rtPlayerEngine = new PlayerEngine();
    // rtPlayerEngine->doReset(); // Initialize the player engine

    // Initialize global drivers
    hAudioDriver = new AudioDriver(rtPlayerEngine);
    hMidiDriver = new MidiDriver(rtPlayerEngine);

    // Create and start the Crow app
    crow::SimpleApp api;

    CROW_ROUTE(api, "/")
    ([]() { return crow::response(200, "PLAYHEAD AUDIO SERVER"); });

    CROW_ROUTE(api, "/shutdown")
    ([]() {
        auto response = crow::response(200, "Service shutting down");
        shutdown_flag.store(true);  // Set shutdown flag to true when /shutdown is hit
        return response; });

    CROW_ROUTE(api, "/startup")
    ([]() {
        hAudioDriver->start();
        hMidiDriver->start();
        return crow::response(200, "Services started");
    });

    // Endpoint to start audio generation
    CROW_ROUTE(api, "/device/audio/doStart")
    ([]() {
        if (hAudioDriver->start()) {
            return crow::response(200, "Audio started successfully");
        } else {
            return crow::response(500, "Failed to start audio");
        } });

    // Endpoint to stop audio generation
    CROW_ROUTE(api, "/device/audio/doStop")
    ([]() {
        hAudioDriver->stop();
        return crow::response(200, "Audio stopped successfully"); });

    CROW_ROUTE(api, "/device/midi/doStart")
    ([]() {
        hMidiDriver->start();
        return crow::response(200, "Midi started successfully"); });

    CROW_ROUTE(api, "/device/midi/doStop")
    ([]() {
        hMidiDriver->stop();
        return crow::response(200, "Midi stopped successfully"); });

    CROW_ROUTE(api, "/test/pe/ping")
    ([]() {
        //rtPlayerEngine->ping();
        hMidiDriver->playerPing();
        return crow::response(200, "Ping sent from PlayerEngine"); });

    CROW_ROUTE(api, "/test/pe/rackSetup")
    ([]() {
        //rtPlayerEngine->ping();
        hMidiDriver->playerSynthSetup();
        return crow::response(200, "Test synth setup"); });

    /*
        // Endpoint to setup rack with synth
        CROW_ROUTE(api, "/racks/<int>/setup")
            .methods(crow::HTTPMethod::GET)([](int rackId, const crow::request &req) {
                auto synthType = req.url_params.get("synth");
                if (synthType) {
                    std::string synthName(synthType);

                    // Call PlayerEngine method to handle rack setup
                    if (rtPlayerEngine) { // Check if rtPlayerEngine is valid
                        bool result = rtPlayerEngine->setupRackWithSynth(rackId, synthName);
                        if (result) {
                            return crow::response(200, "Synth set up successfully.");
                        } else {
                            return crow::response(400, "Failed to set up synth.");
                        }
                    } else {
                        return crow::response(500, "PlayerEngine instance is not initialized.");
                    }
                } else {
                    return crow::response(400, "Missing 'synth' parameter.");
                }
            });
    */

    // Run the server in a separate thread
    std::thread server_thread([&api]() { api.port(18080).run(); });

    // Monitor the shutdown flag
    while (!shutdown_flag.load()) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        if (DEBUG_MODE) {
            std::cout << "sanity check.." << std::endl;
        }
    }

    // Stop the server gracefully
    std::cout << "Stopping the server..." << std::endl;
    api.stop();
    std::this_thread::sleep_for(std::chrono::seconds(1));

    // Wait for the server thread to finish
    if (server_thread.joinable()) {
        server_thread.join();
    }

    // Cleanup
    delete hAudioDriver;
    delete hMidiDriver;
    delete rtPlayerEngine;

    std::cout << "Server stopped gracefully." << std::endl;
    return 0;
}
