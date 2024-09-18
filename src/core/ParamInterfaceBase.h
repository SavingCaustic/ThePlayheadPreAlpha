#pragma once

#include <nlohmann/json.hpp>
#include <string>
#include <unordered_map>
#include <vector>
using json = nlohmann::json;

struct Parameter {
    std::string name;
    float value;
};

template <typename ParamEnum>
class ParamInterfaceBase {
  public:
    // virtual void pushParam(const std::string &paramName, float val) = 0; // Pure virtual method
    // virtual void pushParam(ParamEnum param, float val) = 0; // Pure virtual method

    // Set the parameter value by name
    void setParam(const std::string &name, float val) {
        for (auto &param : demoParams) {
            if (param.name == name) {
                param.value = val;
                return;
            }
        }
        // If parameter does not exist, add it
        demoParams.push_back({name, val});
    }

    // Get the parameter value by name
    float getParam(const std::string &name) const {
        for (const auto &param : demoParams) {
            if (param.name == name) {
                return param.value;
            }
        }
        // If parameter is not found, return a default value, e.g., 0.0f
        return 0.0f;
    }

    // Serialize parameters to a JSON string
    std::string serialize() const {
        json j;
        for (const auto &param : demoParams) {
            j[param.name] = param.value;
        }
        return j.dump(); // Convert JSON object to string
    }

    // Deserialize parameters from a JSON string
    void unserialize(const std::string &jsonStr) {
        json j = json::parse(jsonStr, nullptr, false);
        if (j.is_discarded()) {
            throw std::runtime_error("Failed to parse JSON");
        }
        demoParams.clear(); // Clear existing parameters
        for (auto &[name, value] : j.items()) {
            // demoParams.push_back({name, value.get<float>()});
        }
    }

  protected:
    std::vector<Parameter> demoParams; // This will hold the parameters
};
