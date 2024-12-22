#include "endpointsCrow.h"
#include "constants.h"
#include "core/errors/ErrorBuffer.h"
#include "core/messages/MessageInBuffer.h"
#include "core/messages/MessageOutReader.h"
#include "core/parameters/SettingsManager.h"
#include "core/player/PlayerEngine.h" // Include your relevant headers
#include "core/rpc/rpcParser.h"
#include "crow/json.h"
#include "drivers/AudioManager.h"
#include "drivers/FileDriver.h"
#include "drivers/MidiManager.h"

// Mutex to protect access to WebSocket connections
std::mutex conn_mutex;

// Store active WebSocket connections
std::vector<crow::websocket::connection *> connections;

void crowSetupEndpoints(
    crow::SimpleApp &api,
    PlayerEngine &playerEngine,
    AudioManager &audioManager,
    MidiManager &midiManager,
    MessageInBuffer &messageInBuffer,
    MessageOutBuffer &messageOutBuffer,
    MessageOutReader &messageOutReader,
    ErrorBuffer &errorBuffer,
    RPCParser &rpcParser) {
    // root endpoint. just info.
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

    // web-sockets
    CROW_WEBSOCKET_ROUTE(api, "/ws")
        .onopen([&](crow::websocket::connection &conn) {
            std::lock_guard<std::mutex> lock(conn_mutex);
            connections.push_back(&conn);
            std::cout << "WebSocket connection opened!" << std::endl;
            // maybe it's akward starting the reader here?!
            messageOutReader.setConnection(&conn); // Bind the WebSocket connection
            messageOutReader.start();              // Start the consumer thread
        })
        .onclose([&](crow::websocket::connection &conn, const std::string &reason, uint16_t code) {
            std::lock_guard<std::mutex> lock(conn_mutex);
            connections.erase(std::remove(connections.begin(), connections.end(), &conn), connections.end());
            std::cout << "WebSocket connection closed: " << reason << std::endl;
            messageOutReader.setConnection(nullptr);
        })
        .onmessage([&](crow::websocket::connection &conn, const std::string &data, bool is_binary) {
            if (!is_binary) {
                std::cout << "Received message: " << data << std::endl;
            }
        });

    CROW_ROUTE(api, "/rpc")
        .methods(crow::HTTPMethod::Get)([&rpcParser](const crow::request &req) {
            // Extract query parameters with safety
            std::string method = req.url_params.get("method") ? req.url_params.get("method") : "";
            std::string key = req.url_params.get("key") ? req.url_params.get("key") : "";
            std::string value = req.url_params.get("value") ? req.url_params.get("value") : "";
            std::string rack_id = req.url_params.get("rack_id") ? req.url_params.get("rack_id") : "";

            // Check if any of the parameters are missing
            if (method.empty()) {
                return crow::response(400, "Missing required query parameters");
            }

            // Split the method at the first dot
            size_t dot_pos = method.find('.');
            if (dot_pos != std::string::npos) {
                std::string class_name = method.substr(0, dot_pos);   // Extract the part before the dot
                std::string method_name = method.substr(dot_pos + 1); // Extract the part after the dot

                // Call the RPC parser with the split values
                rpcParser.parse(class_name, method_name, key, value, rack_id);
            } else {
                // Handle the error when there is no dot in the method
                // For example, you might want to send an error response
                return crow::response(400, "Invalid method format");
            }

            // Return a response
            return crow::response(200, "Request processed successfully");
        });
    // below to be removed

    api.route_dynamic("/setRackUnitParam")([&messageInBuffer](const crow::request &req) {
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
        if (messageInBuffer.push(msg)) {
            // Return success message
            return crow::response(200, "Pushed message to the queue");
        } else {
            return crow::response(400, "You're too fast. All your fault.");
        }
    });
}