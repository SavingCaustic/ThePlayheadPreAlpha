#include "crow.h"

int main() {
    crow::SimpleApp app;  // Create the Crow app

    // Define the endpoint
    CROW_ROUTE(app, "/devices")
    ([]() {
        // Create a vector of device names (can be dynamically generated)
        std::vector<std::string> devices = {"Device1", "Device2", "Device3"};

        // Create a JSON object for the response
        crow::json::wvalue jsonResponse;

        // Populate the JSON array correctly with `crow::json::wvalue`
        crow::json::wvalue::list deviceArray;
        for (const auto& deviceName : devices) {
            deviceArray.emplace_back(deviceName);  // Add each device to the list
        }

        // Assign the device array to the JSON response
        jsonResponse["devices"] = std::move(deviceArray);

        // Return the JSON response
        return crow::response(200, jsonResponse);
    });

    // Run the app on localhost:18080
    app.port(18080).multithreaded().run();
}
