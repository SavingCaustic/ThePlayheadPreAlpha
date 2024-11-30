#include <./SynthInterface.h>
#include <drivers/FileDriver.h>
#include <ext/nlohmann/json.hpp>
#include <iostream>

// here all stuff that isn't deterministic should be here..

// this is a static class, just saving, loading and calculating stuff on a synth marked as offline.

class SynthAdminUtility {
  public:
    // parameter-changes affects these too, so we must be given permission from playerEngine..
    static bool loadPatch(SynthInterface &synthInstance, std::string patchName) {
        // get the name of the synth and retrive parameters and settings in the user-directory.
        // any samples and stuff need to be handled by specific synth-admin implementation
    }

    static bool savePatch(SynthInterface &synthInstance, std::string patchName) {
        // get the name of the synth and save parameters and settings in the user-directory.
        // any samples and stuff need to be handled by specific synth-admin implementation
    }
};