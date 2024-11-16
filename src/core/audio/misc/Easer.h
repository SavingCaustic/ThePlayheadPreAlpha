#pragma once
#include <constants.h>
#include <iostream>
// can't see this being used anywhere really..

// maybe this should just be a struct..
// but it's important that voices react to changes of these base settings.
namespace audio::misc {

struct Easer {
    bool active = false;
    float targetValue = 0;
    float currentValue = 0;
    float delta;

    void setTarget(float target) {
        this->targetValue = target;
        this->active = true;
        this->delta = (targetValue - currentValue) / TPH_RACK_RENDER_SIZE;
    }

    inline float getValue() {
        if (!this->active) {
            return currentValue;
        }
        currentValue += delta;
        // Disable easing if we've reached or overshot the target
        if ((delta >= 0 && currentValue >= targetValue) ||
            (delta <= 0 && currentValue <= targetValue)) {
            currentValue = targetValue;
            active = false;
        }
        return currentValue;
    }
};
} // namespace audio::misc
