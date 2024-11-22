#include "Easer.h"
#include <constants.h>

namespace audio::misc {

void Easer::setTarget(float target) {
    this->targetValue = target;
    this->active = true;
    this->delta = (targetValue - currentValue) / TPH_RACK_RENDER_SIZE;
}

float Easer::getNewValue() {
    currentValue += delta;
    // Disable easing if we've reached or overshot the target
    if ((delta >= 0 && currentValue >= targetValue) ||
        (delta <= 0 && currentValue <= targetValue)) {
        currentValue = targetValue;
        active = false;
    }
    return currentValue;
}

} // namespace audio::misc
