// endpointsCrow.cpp
#include "ServerContext.h"
#include "crow.h"
#include "drivers/FileDriver.h"
#include <filesystem>
#include <mutex>
#include <vector>

// Mutex to protect access to WebSocket connections
std::mutex conn_mutex;
std::vector<crow::websocket::connection *> connections;

void crowSetupEndpoints(crow::SimpleApp &api, ServerContext &context) {
    // root endpoint
    CROW_ROUTE(api, "/")
    ([]() { return crow::response(200, "PLAYHEAD AUDIO SERVER"); });

    // front-end resources
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

    // WebSocket handling
    CROW_WEBSOCKET_ROUTE(api, "/ws")
        .onopen([&](crow::websocket::connection &conn) {
            std::lock_guard<std::mutex> lock(conn_mutex);
            connections.push_back(&conn);
            std::cout << "WebSocket connection opened!" << std::endl;
            context.messageOutReader.setConnection(&conn);
            context.messageOutReader.start();
        })
        .onclose([&](crow::websocket::connection &conn, const std::string &reason, uint16_t code) {
            std::lock_guard<std::mutex> lock(conn_mutex);
            connections.erase(std::remove(connections.begin(), connections.end(), &conn), connections.end());
            std::cout << "WebSocket connection closed: " << reason << std::endl;
            context.messageOutReader.setConnection(nullptr);
        })
        .onmessage([&](crow::websocket::connection &conn, const std::string &data, bool is_binary) {
            if (!is_binary) {
                std::cout << "Received message: " << data << std::endl;
            }
        });

    // Other routes using context
    CROW_ROUTE(api, "/shutdown")
    ([&context]() {
        auto response = crow::response(200, "Service shutting down");
        context.thePlayhead.setShutdownFlag();
        return response;
    });

    CROW_ROUTE(api, "/startup")
    ([&context]() {
        context.audioDriver.start();
        context.midiDriver.start();
        context.playerEngine.midiEnable(&context.midiDriver);
        return crow::response(200, "Services started");
    });

    CROW_ROUTE(api, "/device/audio/doStart")
    ([&context]() {
        if (context.audioDriver.start()) {
            return crow::response(200, "Audio started successfully");
        } else {
            return crow::response(500, "Failed to start audio");
        }
    });

    // Endpoint to stop audio generation
    CROW_ROUTE(api, "/device/audio/doStop")
    ([&context]() {
        context.audioDriver.stop();
        return crow::response(200, "Audio stopped successfully");
    });

    CROW_ROUTE(api, "/device/midi/doStart")
    ([&context]() {
        context.midiDriver.start();
        // also connect it to player engine..
        context.playerEngine.midiEnable(&context.midiDriver);
        return crow::response(200, "Midi started successfully");
    });

    CROW_ROUTE(api, "/device/midi/doStop")
    ([&context]() {
        context.playerEngine.midiDisable();
        context.midiDriver.stop();
        return crow::response(200, "Midi stopped successfully");
    });

    CROW_ROUTE(api, "/stat/pe/loadAvg")
    ([&context]() {
        float loadAvg = context.playerEngine.getLoadAvg();
        std::cout << "LoadAvg: " << (loadAvg * 100) << std::endl;
        return crow::response(200, "Loadavg:");
    });

    CROW_ROUTE(api, "/test/pe/ping")
    ([&context]() {
        context.playerEngine.ping();
        return crow::response(200, "Ping sent from PlayerEngine");
    });

    CROW_ROUTE(api, "/test/pe/rackSetup")
    ([&context]() {
        context.playerEngine.testRackSetup();
        return crow::response(200, "Test synth setup");
    });

    CROW_ROUTE(api, "/rack/getFakeSynthParams")
    ([&context]() {
        std::string json = context.playerEngine.getSynthParams(0);
        if (json.empty()) {
            return crow::response(500, "Failed to retrieve synth parameters");
        }

        // Return the JSON response with a 200 OK status
        return crow::response(200, json);
    });

    api.route_dynamic("/setRackUnitParam")([&context](const crow::request &req) {
        auto rack_str = req.url_params.get("rack");
        auto unit = req.url_params.get("unit");
        auto name = req.url_params.get("name");
        // to speed up parsing, lets accept 0-127 as the range.
        auto value_str = req.url_params.get("value");

        // std::cout << "value_str:" << value_str << std::endl;
        //  Initialize default values
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
        // std::cout << "value:" << value << std::endl;

        std::string name_str(name ? name : "");
        MessageIn msg{rack, "synth", name_str.c_str(), value};
        if (context.messageInBuffer.push(msg)) {
            // Return success message
            return crow::response(200, "Pushed message to the queue");
        } else {
            return crow::response(400, "You're too fast. All your fault.");
        }
    });
}