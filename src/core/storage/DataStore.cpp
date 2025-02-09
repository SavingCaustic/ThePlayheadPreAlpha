#include "./DataStore.h"

namespace Storage {

void DataStore::projectLoad(const std::string &projectName) {
    auto json = DocumentManager::loadProjectFromFile(projectName);
    project = Project::from_json(json);
}

void DataStore::projectSave(const std::string &filename) const {
    DocumentManager::saveToFile(project.to_json(), filename);
}

void DataStore::synthPatchLoad(const std::string &filename, size_t rackID) {
    if (rackID >= 4) {
        std::cerr << "Invalid rackID: " << rackID << "\n";
        return;
    }
    const std::string &synthType = project.racks[rackID].synth.type;

    auto json = DocumentManager::loadSynthPatchFromFile(filename, synthType);

    if (!json.empty()) {
        // std::cout << "Synth (patch) JSON: " << json["synth"].dump(4) << std::endl; // Pretty print with 4-space indentation
        std::cout << "Before loading: " << project.racks[rackID].synth.to_json().dump(4) << std::endl;
        project.racks[rackID].synth = Unit::from_json(json);
        std::cout << "After loading: " << project.racks[rackID].synth.to_json().dump(4) << std::endl;
    } else {
        std::cerr << "Failed to load synth patch file: " << filename << "\n";
    }
}
}; // namespace Storage
