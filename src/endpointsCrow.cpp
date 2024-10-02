#include "endpointsCrow.h"
#include "constants.h"
#include "core/MessageReciever.h"
#include "core/PlayerEngine.h" // Include your relevant headers
#include "drivers/AudioDriver.h"
#include "drivers/FileDriver.h"
#include "drivers/MidiDriver.h"

void crowSetupEndpoints(crow::SimpleApp &api, PlayerEngine &playerEngine, AudioDriver &audioDriver, MidiDriver &midiDriver, MessageReciever &messageReciever) {
    CROW_ROUTE(api, "/")
    ([]() { return crow::response(200, "PLAYHEAD AUDIO SERVER"); });

    // namespace fs = std::filesystem;

    CROW_ROUTE(api, "/fe/<string>")
    ([](const std::string &filename) {
        std::string filepath = "assets/fe/" + filename;

        // Check if file exists
        if (std::filesystem::exists(filepath)) {
            std::string fileContent = FileDriver::load_file_content(filepath);

            // Determine file type based on extension
            std::string contentType = "text/plain";
            if (FileDriver::ends_with(filename, ".html")) {
                contentType = "text/html";
            } else if (FileDriver::ends_with(filename, ".css")) {
                contentType = "text/css";
            } else if (FileDriver::ends_with(filename, ".js")) {
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
    ([&audioDriver, &midiDriver, &playerEngine]() {
        audioDriver.start();
        midiDriver.start();
        playerEngine.midiEnable(&midiDriver);
        return crow::response(200, "Services started");
    });

    // Endpoint to start audio generation
    CROW_ROUTE(api, "/device/audio/doStart")
    ([&audioDriver]() {
        if (audioDriver.start()) {
            return crow::response(200, "Audio started successfully");
        } else {
            return crow::response(500, "Failed to start audio");
        } });

    // Endpoint to stop audio generation
    CROW_ROUTE(api, "/device/audio/doStop")
    ([&audioDriver]() {
        audioDriver.stop();
        return crow::response(200, "Audio stopped successfully"); });

    CROW_ROUTE(api, "/device/midi/doStart")
    ([&audioDriver, &playerEngine, &midiDriver]() {
        midiDriver.start();
        //also connect it to player engine..
        playerEngine.midiEnable(&midiDriver);
        return crow::response(200, "Midi started successfully"); });

    CROW_ROUTE(api, "/device/midi/doStop")
    ([&playerEngine, &midiDriver]() {
        playerEngine.midiDisable();
        midiDriver.stop();
        return crow::response(200, "Midi stopped successfully"); });

    CROW_ROUTE(api, "/stat/pe/loadAvg")
    ([&playerEngine]() {
        float loadAvg = playerEngine.getLoadAvg();
        std::cout << "LoadAvg: " << (loadAvg*100) << std::endl;
        return crow::response(200, "Loadavg:"); });

    CROW_ROUTE(api, "/test/pe/ping")
    ([&playerEngine]() {
        playerEngine.ping();
        return crow::response(200, "Ping sent from PlayerEngine"); });

    CROW_ROUTE(api, "/test/pe/rackSetup")
    ([&playerEngine]() {
        playerEngine.testRackSetup();
        return crow::response(200, "Test synth setup");
    });

    CROW_ROUTE(api, "/rack/getFakeSynthParams")
    ([&playerEngine]() {
        std::string json = playerEngine.getSynthParams(0);
        if (json.empty()) {
            return crow::response(500, "Failed to retrieve synth parameters");
        }

        // Return the JSON response with a 200 OK status
        return crow::response(200, json);
    });

    api.route_dynamic("/setRackUnitParam")([&messageReciever](const crow::request &req) {
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
        if (messageReciever.push(msg)) {
            // Return success message
            return crow::response(200, "Pushed message to the rack");
        } else {
            return crow::response(400, "You're too fast. All your fault.");
        }
    });
}