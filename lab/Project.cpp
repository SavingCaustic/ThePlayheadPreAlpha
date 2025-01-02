#include <fstream>
#include <iostream>
#include <map>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

namespace Storage {

struct Param {
    float val;
};

struct Setting {
    std::string val;
};

struct Unit {
    std::string type;
    std::map<std::string, float> params;
    std::map<std::string, std::string> settings;

    nlohmann::json to_json() const {
        nlohmann::json rounded_params;
        for (const auto &[key, value] : params) {
            rounded_params[key] = std::round(value * 1000.0) / 1000.0; // Round to 3 decimal places
        }
        return {{"type", type}, {"params", rounded_params}, {"settings", settings}};
    }

    static Unit from_json(const nlohmann::json &json) {
        Unit unit;
        unit.type = json.value("type", "");
        if (json.contains("params")) {
            unit.params = json["params"].get<std::map<std::string, float>>();
        }
        if (json.contains("settings")) {
            unit.settings = json["settings"].get<std::map<std::string, std::string>>();
        }
        return unit;
    }
};

struct Rack {
    std::map<std::string, std::string> settings;
    Unit eventor1, eventor2, synth, effect1, effect2, emitter;

    nlohmann::json to_json() const {
        return {
            {"settings", settings},
            {"eventor1", eventor1.to_json()},
            {"eventor2", eventor2.to_json()},
            {"synth", synth.to_json()},
            {"effect1", effect1.to_json()},
            {"effect2", effect2.to_json()},
            {"emitter", emitter.to_json()},
        };
    }

    static Rack from_json(const nlohmann::json &json) {
        Rack rack;
        if (json.contains("settings")) {
            rack.settings = json["settings"].get<std::map<std::string, std::string>>();
        }
        if (json.contains("eventor1")) {
            rack.eventor1 = Unit::from_json(json["eventor1"]);
        }
        if (json.contains("eventor1")) {
            rack.eventor2 = Unit::from_json(json["eventor2"]);
        }
        if (json.contains("synth")) {
            rack.synth = Unit::from_json(json["synth"]);
        }
        if (json.contains("effect1")) {
            rack.effect1 = Unit::from_json(json["effect1"]);
        }
        if (json.contains("effect1")) {
            rack.effect2 = Unit::from_json(json["effect2"]);
        }
        /*
        rack.emitter = Unit::from_json(json["emitter"]);
        */
        return rack;
    }
};

struct Project {
    std::map<std::string, std::string> settings;
    Rack racks[4];
    Unit masterReverb, masterDelay;

    nlohmann::json to_json() const {
        nlohmann::json json_racks;
        for (const auto &rack : racks) {
            json_racks.push_back(rack.to_json());
        }
        return {
            {"settings", settings},
            {"racks", json_racks},
            {"masterReverb", masterReverb.to_json()},
            {"masterDelay", masterDelay.to_json()},
        };
    }

    static Project from_json(const nlohmann::json &json) {
        Project project;
        project.settings = json["settings"].get<std::map<std::string, std::string>>();
        for (size_t i = 0; i < 4 && i < json["racks"].size(); ++i) {
            project.racks[i] = Rack::from_json(json["racks"][i]);
        }
        project.masterReverb = Unit::from_json(json["masterReverb"]);
        project.masterDelay = Unit::from_json(json["masterDelay"]);
        return project;
    }
};

// DocumentManager for JSON reading/writing
class DocumentManager {
  public:
    static nlohmann::json loadFromFile(const std::string &filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open file: " + filename);
        }
        nlohmann::json json;
        file >> json;
        return json;
    }

    static void saveToFile(const nlohmann::json &json, const std::string &filename) {
        std::ofstream file(filename);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open file: " + filename);
        }
        file << json.dump(4); // Pretty print with 4 spaces
    }
};

// Unit Factory for initialization
class UnitFactory {
  public:
    static Unit initializeFromJson(const nlohmann::json &json) {
        Unit unit = Unit::from_json(json);
        setupUnit(unit);
        return unit;
    }

    static void setupUnit(const Unit &unit) {
        std::cout << "Setting up unit: " << unit.type << "\n";
        for (const auto &[key, value] : unit.params) {
            std::cout << "  Param: " << key << " = " << value << "\n";
        }
        for (const auto &[key, value] : unit.settings) {
            std::cout << "  Setting: " << key << " = " << value << "\n";
        }
    }
};

// Rack Factory for initialization
class RackFactory {
  public:
    static Rack initializeFromJson(const nlohmann::json &json) {
        Rack rack = Rack::from_json(json);
        UnitFactory::setupUnit(rack.eventor1);
        UnitFactory::setupUnit(rack.eventor2);
        UnitFactory::setupUnit(rack.synth);
        UnitFactory::setupUnit(rack.effect1);
        UnitFactory::setupUnit(rack.effect2);
        return rack;
    }
};

// DataStore for project and patch management
class DataStore {
  public:
    Project project;
    // std::vector<Unit> synthPatches; // Store standalone patches

    void loadProject(const std::string &filename) {
        auto json = DocumentManager::loadFromFile(filename);
        project = initializeProjectFromJson(json);
    }

    void saveProject(const std::string &filename) const {
        DocumentManager::saveToFile(project.to_json(), filename);
    }

    void loadSynthPatch(const std::string &filename, size_t rackID) {
        if (rackID >= 4) {
            std::cerr << "Invalid rackID: " << rackID << "\n";
            return;
        }

        auto &rack = project.racks[rackID];
        auto json = DocumentManager::loadFromFile(filename);

        if (!json.empty()) {
            Unit patchUnit = Unit::from_json(json);

            if (patchUnit.type == rack.synth.type) {
                // Apply the patch settings and parameters to the existing synth
                rack.synth.settings = patchUnit.settings;
                rack.synth.params = patchUnit.params;

                // UnitFactory::initialize(rack.synth); // Reinitialize the synth with new settings
                std::cout << "Synth patch loaded and applied to Rack " << rackID << ".\n";
            } else {
                std::cerr << "Patch type (" << patchUnit.type
                          << ") does not match the synth type (" << rack.synth.type
                          << ") in Rack " << rackID << ".\n";
            }
        } else {
            std::cerr << "Failed to load synth patch file: " << filename << "\n";
        }
    }

  private:
    Project initializeProjectFromJson(const nlohmann::json &json) {
        Project project = Project::from_json(json);
        for (auto &rack : project.racks) {
            // rack = RackFactory::initializeFromJson(rack.to_json());
        }
        return project;
    }
};

} // namespace Storage

int main() {
    try {
        Storage::DataStore myDataStore;

        // Load a project file
        std::cout << "Loading project.json...\n";
        myDataStore.loadProject("project.json");

        // Load a standalone synth-patch file
        std::cout << "Loading synth-patch.json...\n";
        myDataStore.loadSynthPatch("patch.json", 0);

        // Save the project
        myDataStore.saveProject("saved_project.json");

        return 0;
    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
}
