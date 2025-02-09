#include "Rack.h"
#include "../PlayerEngine.h"

Rack::Rack()
    : audioIsStereo(false),
      emitter(audioBufferLeft.data(), audioBufferRight.data(), TPH_RACK_RENDER_SIZE) {}

Rack::~Rack() {
    delete synth;
    delete effect1;
    delete effect2;
}

void Rack::setPlayerEngine(PlayerEngine &engine) {
    playerEngine = &engine;
}

void Rack::sendLog(int code, const std::string &message) {
    if (playerEngine) {
        // uhm - string used here - not good.
        // playerEngine->logTemp.code = code;
        // playerEngine->logTemp.message = &message;
    }
}

void Rack::clockReset() {
    // dunno really..
    nextClockPulse = 0;
    clock24 = 0;
    // patternPlayer->clockReset();
}

void Rack::probeNewClock(float pulse) {
    // if no chance swung fire may hit, return..
    if (pulse - nextClockPulse > 50)
        return;
    if (pulse >= nextClockPulse) {
        if (this->eventor1)
            this->eventor1->processClock();
        if (this->eventor2)
            this->eventor2->processClock();
        // synth??
        if (this->effect1)
            this->effect1->processClock(clock24);
        if (this->effect2)
            this->effect2->processClock(clock24);
        // clock management:
        this->clock24++;
        if (this->clock24 == 24) {
            this->clock24 = 0;
            FORMAT_LOG_MESSAGE(logTmp, LOG_INFO, "BEAT %d", 0);
            // audioHallway.logMessage(logTmp);
        }
        this->calcNextClockPulse();
    }
}

void Rack::calcNextClockPulse() {
    // calculate what mPulse the next clock will happen.
    if (this->swingOverride) {
        // calc swing based on rack
        this->nextClockPulse = this->calcSwungClock(this->swingCycle, this->swingDepth);
    } else {
        // calc swing based on PE
        this->nextClockPulse = this->calcSwungClock(this->playerEngine->swingCycle, this->playerEngine->swingDepth);
    }
    // we should probably send midi clock here, at least when playing.
}

float Rack::calcSwungClock(const uint8_t swingCycle, float swingDepth) {
    float angle = (this->clock24 % swingCycle) / swingCycle;
    float swing = (0.5 - AudioMath::ccos(angle) * 0.5) * swingDepth * swingCycle / 4;
    float ncp = (this->clock24 + swing) * TPH_TICKS_PER_CLOCK;
    // echo 'At angle ' . $angle . ', swing is: ' . $swing . ', so next clock at ' . $nextClockPulse . "\r\n";
    if (ncp >= 120.0f)
        ncp -= 120.0f;
    return ncp;
}

void Rack::probeNewTick(float pulse) {}

void Rack::render(int num) {
    bool isStereo;
    if (synth) {
        isStereo = synth->renderNextBlock();
    } else {
        isStereo = false;
        float *ptr = audioBufferLeft.data();
        for (int i = 0; i < TPH_RACK_RENDER_SIZE; i++) {
            *ptr++ = 0.05f * ((static_cast<float>(rand()) / RAND_MAX) * 2.0f - 1.0f); // Noise
        }
    }
    if (effect1) {
        debugCnt++;
        isStereo = effect1->renderNextBlock(isStereo);
        if (effect2) {
            isStereo = effect2->renderNextBlock(isStereo);
        }
    }
    isStereo = emitter.process(isStereo);
    audioIsStereo = isStereo;
}

void Rack::parseMidi(uint8_t cmd, uint8_t param1, uint8_t param2) {
    char ccIdx[2];
    if ((cmd & 0xf0) == 0xb0) {
        if (param1 >= 84 && param1 <= 89) {
            ccIdx[0] = static_cast<char>('0' + (param1 - 84));
            ccIdx[1] = '\0';
            passParamToUnit(UnitType::Effect1, ccIdx, param2);
            return;
        } else if (param1 >= 90 && param1 <= 95) {
            passParamToUnit(UnitType::Effect2, "time", param2);
            return;
        }
    }
    if (!eventor1) {
        synth->parseMidi(cmd, param1, param2);
    } else {
        if (eventor2) {
            eventor1->parseMidiAndForward(cmd, param1, param2, *eventor2);
        } else {
            eventor1->parseMidiAndForward(cmd, param1, param2, *synth);
        }
    }
}

Rack::UnitType Rack::stringToUnitType(const char *unit) {
    if (strcmp(unit, "synth") == 0)
        return UnitType::Synth;
    if (strcmp(unit, "emitter") == 0)
        return UnitType::Emitter;
    if (strcmp(unit, "eventor1") == 0)
        return UnitType::Eventor1;
    if (strcmp(unit, "eventor2") == 0)
        return UnitType::Eventor2;
    if (strcmp(unit, "effect1") == 0)
        return UnitType::Effect1;
    if (strcmp(unit, "effect2") == 0)
        return UnitType::Effect2;
    return UnitType::_unknown;
}

void Rack::passParamToUnit(UnitType unit, const char *name, int val) {
    float fVal = static_cast<float>(val) / 127.0f;
    switch (unit) {
    case UnitType::Synth:
        synth->pushStrParam(name, fVal);
        break;
    case UnitType::Effect1:
        if (effect1)
            effect1->pushStrParam(name, fVal);
        break;
    case UnitType::Effect2:
        if (effect2)
            effect2->pushStrParam(name, fVal);
        break;
    default:
        std::cerr << "Unknown unit type!" << std::endl;
        break;
    }
}

std::string Rack::getSynthParams() {
    return synth ? synth->getParamDefsAsJSON() : "{}";
}

bool Rack::setSynth(SynthBase *newSynth) {
    delete synth;
    synth = newSynth;
    if (synth) {
        synth->bindBuffers(audioBufferLeft.data(), audioBufferRight.data(), TPH_RACK_RENDER_SIZE);
        enabled = true;
    } else {
        enabled = false;
    }
    return synth != nullptr;
}

bool Rack::setEffect(EffectBase *newEffect, int effectSlot) {
    EffectInterface **effectTarget = (effectSlot == 1) ? &effect1 : &effect2;
    *effectTarget = newEffect;
    if (*effectTarget) {
        (*effectTarget)->bindBuffers(audioBufferLeft.data(), audioBufferRight.data(), TPH_RACK_RENDER_SIZE);
    }
    return *effectTarget != nullptr;
}

bool Rack::setEventor(EventorBase *newEventor, int eventorSlot) {
    EventorInterface **eventorTarget = (eventorSlot == 1) ? &eventor1 : &eventor2;
    *eventorTarget = newEventor;
    if (*eventorTarget) {
        (*eventorTarget)->setMidiTarget(synth);
        (*eventorTarget)->setPosition(1);
    }
    return *eventorTarget != nullptr;
}
