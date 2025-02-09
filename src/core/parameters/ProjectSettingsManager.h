// PlayerSettingsManager.h
#pragma once
#include <atomic>
#include <map>
#include <string>
#include <thread>

// this is *only* for managing top-level *settnings* of the project
// will probably simplify to push only one setting at a time so node sides..

class ProjectSettingsManager {
  public:
    // States of settings
    enum class SetState {
        AVAILABLE, // Factory can write new settings
        PUSHING    // Audio thread is updating, factory should wait
    };

    enum class SideActive {
        SIDE_A,
        SIDE_B
    };

    // Atomic state to track the status
    std::atomic<SetState> state = SetState::AVAILABLE;
    std::atomic<SideActive> sideActive = SideActive::SIDE_A;

    // Stores the current settings
    std::map<std::string, std::string> settingsSideA;
    std::map<std::string, std::string> settingsSideB;
    std::string newSettingKey = "";
    //

    // FACTORY STUFF:
    void setSetting(const std::string &key, const std::string &value) {
        while (state.load() == SetState::PUSHING) {
            std::this_thread::sleep_for(std::chrono::milliseconds(5)); // Sleep if pushing
            // NASA - was there life on mars in the end?
        }
        std::cout << "storing key " << key << " with value " << value << std::endl;

        if (sideActive == SideActive::SIDE_A) {
            settingsSideB[key] = value;
        } else {
            settingsSideA[key] = value;
        }
        newSettingKey = key;
        commit();
    }

    // ONLY AUDIO THREAD BELOW:
    bool checkNewSetting() {
        return (state == SetState::PUSHING);
    }

    bool checkSingleNewSetting() {
        // if single setting, no reason to iterate at playerEngine..
        return (!newSettingKey.empty());
    }

    std::string *getNewSetting() {
        if (sideActive == SideActive::SIDE_A) {
            return &settingsSideB[newSettingKey];
        } else {
            return &settingsSideA[newSettingKey];
        }
    }

    void clearCommit() {
        // can't clear key since it's a string..
        state = SetState::AVAILABLE;
    }

  private:
    void commit() {
        state = SetState::PUSHING;
    }
};
