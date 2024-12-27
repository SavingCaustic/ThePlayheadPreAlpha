#include <map>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

namespace Storage {

struct Setting {
    std::string value; // Plain storage
};

struct Param {
    float value; // Plain storage
};

struct Unit {
    std::map<std::string, Param> params;
    std::map<std::string, Setting> settings;

    nlohmann::json to_json() const {
        nlohmann::json json_params;
        for (const auto &[key, param] : params) {
            json_params[key] = param.value;
        }

        nlohmann::json json_settings;
        for (const auto &[key, setting] : settings) {
            json_settings[key] = setting.value;
        }

        return {{"params", json_params}, {"settings", json_settings}};
    }

    static Unit from_json(const nlohmann::json &json) {
        Unit unit;

        if (json.contains("params")) {
            for (const auto &[key, value] : json["params"].items()) {
                unit.params[key] = Param{value.get<float>()};
            }
        }

        if (json.contains("settings")) {
            for (const auto &[key, value] : json["settings"].items()) {
                unit.settings[key] = Setting{value.get<std::string>()};
            }
        }

        return unit;
    }
};

struct Rack {
    std::map<std::string, Setting> settings; // Settings for rack itself
    Unit eventor1;
    Unit eventor2;
    Unit synth;
    Unit effect1;
    Unit effect2;
    Unit emitter;

    nlohmann::json to_json() const {
        return {
            {"settings", [this]() {
                 nlohmann::json json_settings;
                 for (const auto &[key, setting] : settings) {
                     json_settings[key] = setting.value;
                 }
                 return json_settings;
             }()},
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
                rack.settings[key] = Setting{value.get<std::string>()};
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

struct Project {
    std::vector<Setting> settings;
    Rack racks[4];
    Unit masterReverb;
    Unit masterEmitter;

    nlohmann::json to_json() const {
        nlohmann::json json_settings;
        for (const auto &setting : settings) {
            json_settings.push_back(setting.value);
        }

        nlohmann::json json_racks;
        for (const auto &rack : racks) {
            json_racks.push_back(rack.to_json());
        }

        return {
            {"settings", json_settings},
            {"racks", json_racks},
            {"masterReverb", masterReverb.to_json()},
            {"masterEmitter", masterEmitter.to_json()},
        };
    }

    static Project from_json(const nlohmann::json &json) {
        Project project;

        if (json.contains("settings")) {
            for (const auto &value : json["settings"]) {
                project.settings.push_back(Setting{value.get<std::string>()});
            }
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
        project.masterEmitter = Unit::from_json(json.at("masterEmitter"));

        return project;
    }
};

} // namespace Storage
