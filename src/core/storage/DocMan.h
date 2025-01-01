#pragma once
#include "drivers/FileDriver.h"
#include <optional>
#include <string>

namespace storage {

// Document Manager
class DocMan {

    static void synthPatchList(const std::string &synthName) {
        // list all available patches. order to be discussed..
    }

    static std::optional<std::string> synthPatchLoad(const std::string &synthName, const std::string &patchName) {
        // look in users, then assets..
        // we should have a function to strip synthName and patchName from / and .. etc..
        std::string patchPath = "Synth/" + FileDriver::secure(synthName) + "/" + FileDriver::secure(patchName) + ".json";
        if (FileDriver::userFileExists(patchPath)) {
            return FileDriver::userFileRead(patchPath);
        }
        if (FileDriver::assetFileExists(patchPath)) {
            return FileDriver::assetFileRead(patchPath);
        }
        return std::nullopt;
    }

    static void synthPatchSave() {
    }

    static void rackPatchList(const std::string &synthName) {
        // list all available patches. order to be discussed..
    }

    static std::optional<std::string> rackPatchLoad(const std::string &synthName, const std::string &patchName) {
        // look in users, then assets..
        std::string patchPath = synthName + "/" + patchName + ".json";
        if (FileDriver::userFileExists(patchPath)) {
            return FileDriver::userFileRead(patchPath);
        }
        if (FileDriver::assetFileExists(patchPath)) {
            return FileDriver::assetFileRead(patchPath);
        }
        return std::nullopt;
    }

    static void rackPatchSave() {
    }

    static void projectLoad() {
    }

    static void projectSave() {
    }
};
} // namespace storage