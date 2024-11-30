#include <./MonolithModel.h>
#include <Synth/SynthAdminUtility.h>

// meh.. maybe this should be called MonolithFactory.

namespace Synth::Monolith {
class MonolithAdmin : public SynthAdminUtility {
  public:
    static bool loadPatch(Model *synthInstance, const std::string &patchName) {
        if (!synthInstance) {
            std::cerr << "Error: Null synth instance provided" << std::endl;
            return false;
        }
        std::cout << "Loading patch: " << patchName << " (Monolith-specific admin)" << std::endl;
        return true;
    }

    static std::string formatParametersAsString(const Model *synthInstance) {
        if (!synthInstance) {
            return "Error: Null synth instance";
        }
        return "Monolith-specific parameter string";
    }
};
} // namespace Synth::Monolith