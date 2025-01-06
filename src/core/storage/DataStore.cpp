#include "./DataStore.h"

namespace Storage {

void DataStore::loadProject(const std::string &projectName) {
    auto json = DocumentManager::loadProjectFromFile(projectName);
    project = Project::from_json(json);
}

void DataStore::saveProject(const std::string &filename) const {
    DocumentManager::saveToFile(project.to_json(), filename);
}

void DataStore::loadSynthPatch(const std::string &filename, size_t rackID) {
    if (rackID >= 4) {
        std::cerr << "Invalid rackID: " << rackID << "\n";
        return;
    }

    auto &rack = project.racks[rackID];
    auto json = DocumentManager::loadSynthPatchFromFile(filename);
    // maybe i could skip or simplify below, using stuff in structs.h

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
}; // namespace Storage
