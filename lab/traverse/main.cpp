#include <nlohmann/json.hpp>
#include <iostream>
#include <memory>
#include <vector>

using json = nlohmann::json;

// Base class for objects with settings
class Settings {
  public:
    float settingsVal;
    std::string settingsStr;

    virtual void applySettings(float val, const std::string &str) {
        settingsVal = val;
        settingsStr = str;
    }

    virtual ~Settings() = default;
};

// Unit class (Synth, Effect, etc.)
class Unit : public Settings {
  public:
    std::string type;

    void applySettings(float val, const std::string &str, const std::string &unitType) {
        settingsVal = val;
        settingsStr = str;
        type = unitType;
        std::cout << "Created " << unitType << " with settingsVal: " << settingsVal
                  << " and settingsStr: " << settingsStr << "\n";
    }
};

// Rack class holding multiple units
class Rack : public Settings {
  public:
    std::vector<std::unique_ptr<Unit>> units;

    void addUnit(std::unique_ptr<Unit> unit) {
        units.push_back(std::move(unit));
    }
};

// PlayerEngine class holding multiple racks
class PlayerEngine : public Settings {
  public:
    std::vector<std::unique_ptr<Rack>> racks;

    void addRack(std::unique_ptr<Rack> rack) {
        racks.push_back(std::move(rack));
    }

    // Recursive method to traverse the JSON and create objects
    void loadFromJson(const json &jsonObj) {
        applySettings(jsonObj["settingsVal"], jsonObj["settingsStr"]);

        if (jsonObj.contains("racks")) {
            for (const auto &rackJson : jsonObj["racks"]) {
                auto rack = std::make_unique<Rack>();
                rack->applySettings(rackJson["settingsVal"], rackJson["settingsStr"]);

                if (rackJson.contains("units")) {
                    for (const auto &unitJson : rackJson["units"]) {
                        auto unit = std::make_unique<Unit>();
                        unit->applySettings(unitJson["settingsVal"], unitJson["settingsStr"], unitJson["type"]);
                        rack->addUnit(std::move(unit));
                    }
                }
                addRack(std::move(rack));
            }
        }
    }
};

// Main function to test loading from JSON
int main() {
    // Example JSON document
    std::string jsonData = R"({
        "settingsVal": 1.0,
        "settingsStr": "MainPlayer",
        "racks": [
            {
                "settingsVal": 0.8,
                "settingsStr": "SynthRack",
                "units": [
                    {
                        "type": "Synth",
                        "settingsVal": 0.9,
                        "settingsStr": "MainSynth"
                    },
                    {
                        "type": "Effect",
                        "settingsVal": 0.6,
                        "settingsStr": "Reverb"
                    }
                ]
            },
            {
                "settingsVal": 0.7,
                "settingsStr": "EffectRack",
                "units": [
                    {
                        "type": "Filter",
                        "settingsVal": 0.5,
                        "settingsStr": "LowPassFilter"
                    }
                ]
            }
        ]
    })";

    // Parse JSON data
    json parsedJson = json::parse(jsonData);

    // Create PlayerEngine and load from JSON
    PlayerEngine player;
    player.loadFromJson(parsedJson);

    return 0;
}
