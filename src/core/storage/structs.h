#pragma once
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

} // namespace Storage