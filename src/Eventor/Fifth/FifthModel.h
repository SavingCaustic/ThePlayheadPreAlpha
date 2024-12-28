#pragma once
#include "Eventor/EventorBase.h"
#include "core/player/Rack.h"
#include <cstdint>

using json = nlohmann::json;
namespace Eventor::Fifth {

class Model : public EventorBase {
    // dummy eventor generating a third.
    // tricky to make this independent of Rack. It needs access to both eventor2 and synth.
  public:
    // Constructor
    Model() {}

    void parseMidi(uint8_t cmd, uint8_t param1, uint8_t param2) {
        uint8_t cmdMSN = cmd & 0xf0;
        switch (cmdMSN) {
        case 0x80:
        case 0x90:
            // clone note +12
            this->sendMidi(cmd, param1, param2);
            this->sendMidi(cmd, param1 + 5, param2);
            break;
        default:
            this->sendMidi(cmd, param1, param2);
            break;
        }
    }

    void reset() {}

    void updateSetting(const std::string &type, void *object, uint32_t size, bool isStereo, Destructor::Record &recordDelete) override {}

    json getParamDefsAsJSON() override {
        return EventorBase::getParamDefsAsJson();
    }
};
} // namespace Eventor::Fifth
