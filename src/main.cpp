#include "constants.h"
#include "core/ErrorLogger.h"
#include "core/MessageReciever.h"
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
MessageReciever *hMessageReciever;

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
    // std::cout << "Current path: " << std::filesystem::current_path() << std::endl;

    // Look, we want these to be static but there's an issue with Crow endpoints stop working..

    // Initialize the global PlayerEngine instance
    hMidiDriver = new MidiDriver();

    // Initialitze the message-router
    const size_t bufferSize = 8; // Choose a reasonable size for your use case
    hMessageReciever = new MessageReciever(bufferSize);
    // MessageRouter hMessageRouter(bufferSize);

    // rtPlayerEngine = new PlayerEngine(*hMessageRouter);
    rtPlayerEngine = new PlayerEngine();
    rtPlayerEngine->BindMessageReciever(*hMessageReciever);
    // PlayerEngine rtPlayerEngine(hMessageRouter);
    //  rtPlayerEngine->doReset(); // Initialize the player engine

    // Initialize global drivers. I'm not sure i wanna pass rtPlayerEngine..
    hAudioDriver = new AudioDriver(rtPlayerEngine);
    // AudioDriver hAudioDriver(&rtPlayerEngine);

    //
    AudioMath::generateLUT(); // sets up a sine lookup table of 1024 elements.
    std::cout << "sin 0.125 => " << AudioMath::csin(1 / 8) << std::endl;
    std::cout << "cos 0.125 => " << AudioMath::ccos(1 / 8) << std::endl;

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
        // hMidiDriver->playerSynthSetup();
        return crow::response(200, "Test synth setup");
    });

    api.route_dynamic("/setRackUnitParam")([](const crow::request &req) {
        auto rack_str = req.url_params.get("rack");
        auto unit = req.url_params.get("unit");
        auto name = req.url_params.get("name");
        // to speed up parsing, lets accept 0-127 as the range.
        auto value_str = req.url_params.get("value");

        std::cout << "value_str:" << value_str << std::endl;
        // Initialize default values
        int rack = -1; // invalid as default
        int value = 64;

        // Convert rack to integer with fallback
        if (rack_str) {
            try {
                rack = std::stoi(rack_str);
            } catch (...) { // Catch all exceptions
                rack = -1;  // Fallback value
            }
        }

        if (value_str) {
            try {
                value = std::stoi(value_str);
            } catch (...) { // Catch all exceptions
                value = 0;  // Fallback value
            }
        }
        std::cout << "value:" << value << std::endl;

        std::string name_str(name ? name : "");
        Message msg{rack, "synth", name_str.c_str(), value};
        if (hMessageReciever->push(msg)) {
            // Return success message
            return crow::response(200, "Pushed message to the rack");
        } else {
            return crow::response(400, "You're too fast. All your fault.");
        }
    });

    // Run the server in a separate thread
    std::thread server_thread([&api]() { api.port(18080).run(); });

    // Monitor the shutdown flag
    while (!shutdown_flag.load()) {
        std::this_thread::sleep_for(std::chrono::seconds(3));
        if (DEBUG_MODE) {
            std::cout << "housekeeping.. " << std::endl;
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
    delete hMessageReciever;

    std::cout << "Server stopped gracefully." << std::endl;
    return 0;
}
