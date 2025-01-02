// DataStore for project and patch management
#pragma once
#include "./DocumentManager.h"
#include "./structs.h"

namespace Storage {

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