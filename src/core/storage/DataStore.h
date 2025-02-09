// DataStore for project and patch management
#pragma once
#include "./DocumentManager.h"
#include "./structs.h"

namespace Storage {

class DataStore {
  public:
    Project project;
    // std::vector<Unit> synthPatches; // Store standalone patches

    void projectLoad(const std::string &projectName);

    void projectSave(const std::string &filename) const;

    void synthPatchLoad(const std::string &filename, size_t rackID);
    // void synthPatchSave(const std::string &filename, size_t rackID);

    void settingsSet(const std::string &key, const std::string &value) {
        project.settings[key] = value;
    }

    // void settingsGet(const std::string &key, const std::string &value) {

    std::map<std::string, std::string> settingsGetCopy() {
        return project.settings; // Safe copy
    }
};

} // namespace Storage