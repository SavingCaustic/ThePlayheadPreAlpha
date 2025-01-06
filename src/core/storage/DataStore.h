// DataStore for project and patch management
#pragma once
#include "./DocumentManager.h"
#include "./structs.h"

namespace Storage {

class DataStore {
  public:
    Project project;
    // std::vector<Unit> synthPatches; // Store standalone patches

    void loadProject(const std::string &projectName);

    void saveProject(const std::string &filename) const;

    void loadSynthPatch(const std::string &filename, size_t rackID);
};

} // namespace Storage