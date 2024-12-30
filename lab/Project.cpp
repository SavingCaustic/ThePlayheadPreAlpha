#include <map>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

namespace Storage {

struct Param {
    float val; // Parameters like "cutoff"
};

struct Setting {
    std::string val; // Parameters like "mode"
};

struct Unit {
    std::string type;                            // e.g., "Monolith" or "Delay"
    std::map<std::string, float> params;         // Contains float parameters
    std::map<std::string, std::string> settings; // Contains string parameters

    nlohmann::json to_json() const {
        nlohmann::json json_params;
        for (const auto &[key, param] : params) {
            json_params[key] = param; // Using param.val for the value
        }

        nlohmann::json json_settings;
        for (const auto &[key, setting] : settings) {
            json_settings[key] = setting; // Using setting.val for the value
        }

        return {{"type", type}, {"params", json_params}, {"settings", json_settings}};
    }

    static Unit from_json(const nlohmann::json &json) {
        Unit unit;
        unit.type = json.value("type", "");

        if (json.contains("params")) {
            for (const auto &[key, value] : json["params"].items()) {
                unit.params[key] = value; // Create Param from value
            }
        }

        if (json.contains("settings")) {
            for (const auto &[key, value] : json["settings"].items()) {
                unit.settings[key] = value; // Create Setting from value
            }
        }

        return unit;
    }
};

struct Rack {
    std::map<std::string, std::string> settings; // Settings for the rack itself
    Unit eventor1;
    Unit eventor2;
    Unit synth;
    Unit effect1;
    Unit effect2;
    Unit emitter;

    nlohmann::json to_json() const {
        nlohmann::json json_settings = nlohmann::json::object();
        for (const auto &[key, value] : settings) {
            json_settings[key] = value;
        }

        return {
            {"settings", json_settings},
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
            for (const auto &[key, value] : json["settings"].items()) {
                rack.settings[key] = value.get<std::string>();
            }
        }

        rack.eventor1 = Unit::from_json(json.at("eventor1"));
        rack.eventor2 = Unit::from_json(json.at("eventor2"));
        rack.synth = Unit::from_json(json.at("synth"));
        rack.effect1 = Unit::from_json(json.at("effect1"));
        rack.effect2 = Unit::from_json(json.at("effect2"));
        rack.emitter = Unit::from_json(json.at("emitter"));

        return rack;
    }
};

struct Master {
    Unit reverb;
    Unit chorus;
};

struct Project {
    std::vector<std::string> settings; // Plain list of settings
    Rack racks[4];
    Unit masterReverb;
    Unit masterDelay;

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

        if (json.contains("settings")) {
            project.settings = json["settings"].get<std::vector<std::string>>();
        }

        size_t rack_index = 0;
        if (json.contains("racks")) {
            for (const auto &rack_json : json["racks"]) {
                if (rack_index < 4) {
                    project.racks[rack_index] = Rack::from_json(rack_json);
                    ++rack_index;
                }
            }
        }

        project.masterReverb = Unit::from_json(json.at("masterReverb"));
        project.masterDelay = Unit::from_json(json.at("masterDelay"));

        return project;
    }
};

class DataStore {
    // should allow type-save getting and setting of all params and settings..
  public:
    Project data;

    void initProject() {
        // i could init a default project structure either from a default json,
        // or here - programatically..
    }

    void loadProject(std::string filename) {
        // get file from persistant storage
        // and then iterate somehow so player-engine is populated. Complicated..
        // much easier if we could disengage playerEngine and work directly with that,
        // but could be problematic with unit-model expecting object queues.
    }

    void saveProject(std::string filename) {
        // all data should already be here - nothing to aquire from racks/units. Just save it..
    }
};

} // namespace Storage

int main() {
    Storage::DataStore myDataStore;
    myDataStore.initProject();
    myDataStore.data.racks[3].synth.type = "Monolith";
    myDataStore.data.racks[3].synth.settings["testSettting"] = "test-string";
    myDataStore.data.racks[3].synth.params["cutoff"] = 0.4f;
    myDataStore.data.masterReverb.type = "Freeverb";
    return 0;
}