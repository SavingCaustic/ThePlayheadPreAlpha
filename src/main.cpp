#include "constants.h"
#include "core/ErrorLogger.h"
#include "core/PlayerEngine.h"
#include "core/Rack.h"
#include "core/audio/AudioMath.h"
#include "crow.h"
#include "drivers/AudioDriver.h"
#include "drivers/MidiDriver.h"
#include <atomic>
#include <chrono>
#include <filesystem> // C++17 for checking if files exist
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

bool ends_with(const std::string &str, const std::string &suffix) {
    return str.size() >= suffix.size() &&
           str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}

std::string load_file_content(const std::string &filepath) {
    std::ifstream file(filepath);

    if (!file) {
        return ""; // Return empty string if file not found
    }

    std::stringstream buffer;
    buffer << file.rdbuf(); // Load the whole file into the stringstream
    return buffer.str();
}

// Entry point of the program
int main() {
    // Initialize the global PlayerEngine instance
    hMidiDriver = new MidiDriver();
    rtPlayerEngine = new PlayerEngine();
    // rtPlayerEngine->doReset(); // Initialize the player engine

    // Initialize global drivers
    hAudioDriver = new AudioDriver(rtPlayerEngine);

    // Create and start the Crow app
    crow::SimpleApp api;

    CROW_ROUTE(api, "/")
    ([]() { return crow::response(200, "PLAYHEAD AUDIO SERVER"); });

    namespace fs = std::filesystem;

    CROW_ROUTE(api, "/fe/<string>")
    ([](const std::string &filename) {
        std::string filepath = "assets/fe/" + filename;

        // Check if file exists
        if (fs::exists(filepath)) {
            std::string fileContent = load_file_content(filepath);

            // Determine file type based on extension
            std::string contentType = "text/plain";
            if (ends_with(filename, ".html")) {
                contentType = "text/html";
            } else if (ends_with(filename, ".css")) {
                contentType = "text/css";
            } else if (ends_with(filename, ".js")) {
                contentType = "application/javascript";
            }
            crow::response res;
            res.code = 200;
            res.set_header("Content-Type", contentType);
            res.write(fileContent);
            return res;
        } else {
            return crow::response(404, "File not found");
        }
    });

    CROW_ROUTE(api, "/shutdown")
    ([]() {
        auto response = crow::response(200, "Service shutting down");
        shutdown_flag.store(true);  // Set shutdown flag to true when /shutdown is hit
        return response; });

    CROW_ROUTE(api, "/startup")
    ([]() {
        hAudioDriver->start();
        hMidiDriver->start();
        rtPlayerEngine->midiEnable(hMidiDriver);
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
        //also connect it to player engine..
        rtPlayerEngine->midiEnable(hMidiDriver);
        return crow::response(200, "Midi started successfully"); });

    CROW_ROUTE(api, "/device/midi/doStop")
    ([]() {
        rtPlayerEngine->midiDisable();
        hMidiDriver->stop();
        return crow::response(200, "Midi stopped successfully"); });

    CROW_ROUTE(api, "/test/pe/ping")
    ([]() {
        rtPlayerEngine->ping();
        //hMidiDriver->playerPing();
        return crow::response(200, "Ping sent from PlayerEngine"); });

    CROW_ROUTE(api, "/test/pe/rackSetup")
    ([]() {
        rtPlayerEngine->testRackSetup();
        //hMidiDriver->playerSynthSetup();
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
            std::cout << "sc.. ";
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
