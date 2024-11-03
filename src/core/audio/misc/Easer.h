#pragma once

#include <cmath>
#include <constants.h>
#include <iostream>

// maybe this should just be a struct..
// but it's important that voices react to changes of these base settings.
namespace audio::misc {

struct Easer {
    bool active = 0;
    float targetValue;
    float currentValue;
    float delta;

    float setTarget(float target) {
        // returns delta
        if (target == currentValue) {
            return 0;
        }
        this->targetValue = target;
        this->active = true;
        this->delta = (targetValue - currentValue) / TPH_RACK_RENDER_SIZE;
        return delta;
    }

    void updateCurrent() {
        this->currentValue = targetValue;
        this->active = false;
    }
};
} // namespace audio::misc
