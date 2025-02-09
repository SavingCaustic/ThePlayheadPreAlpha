#pragma once
#include "./RackEmitter.h"
#include "Effect/Chorus/ChorusModel.h"
#include "Effect/Chorus2/Chorus2Model.h"
#include "Effect/Delay/DelayModel.h"
#include "Synth/Monolith/MonolithModel.h"
// #include "Synth/Sketch/SketchModel.h"
#include "Synth/Subreal/SubrealModel.h"
//
#include "Eventor/EventorBase.h"
#include "Eventor/EventorInterface.h"
#include "Synth/SynthBase.h"
#include "Synth/SynthInterface.h"
#include "constants.h"
// #include "ParamInterfaceBase.h"
#include <array>
#include <cstddef> // for std::size_t
#include <iostream>
#include <memory>
#include <string>

// forward declaration
class PlayerEngine;

class Rack {
  public:
    enum class UnitType {
        Synth,
        Eventor1,
        Eventor2,
        Effect1,
        Effect2,
        Emitter,
        _unknown
    };

    enum class SynthType {
        Monolith,
        Subreal,
        Sketch,
        Unknown
    };

    enum class EffectType {
        Delay,
        Chorus,
        Chorus2,
        Unknown
    };

    Rack();
    ~Rack();

    void setPlayerEngine(PlayerEngine &engine);

    // std::array<float, TPH_RACK_BUFFER_SIZE> &getAudioBuffer();

    void clockReset();
    void probeNewClock(float pulse);
    void probeNewTick(float pulse);

    void sendLog(int code, const std::string &message);

    void render(int num);

    void parseMidi(uint8_t cmd, uint8_t param1 = 0x00, uint8_t param2 = 0x00);
    static UnitType stringToUnitType(const char *unit);

    void passParamToUnit(UnitType unit, const char *name, int val);
    std::string getSynthParams();

    bool setSynth(SynthBase *newSynth);
    bool setEffect(EffectBase *newEffect, int effectSlot = 1);
    bool setEventor(EventorBase *newEventor, int eventorSlot = 1);

  public:
    alignas(32) std::array<float, TPH_RACK_RENDER_SIZE> audioBufferLeft;
    alignas(32) std::array<float, TPH_RACK_RENDER_SIZE> audioBufferRight;
    LoggerRec logTmp;
    bool audioIsStereo = false;
    RackEmitter emitter;
    bool enabled = false;
    bool swingOverride = false;
    uint8_t swingCycle = 12;
    float swingDepth = 0;

    uint16_t debugCnt = 0;
    uint16_t nextClockPulse;
    uint8_t clock24;

    SynthInterface *synth = nullptr;
    EffectInterface *effect1 = nullptr;
    EffectInterface *effect2 = nullptr;
    EventorInterface *eventor1 = nullptr;
    EventorInterface *eventor2 = nullptr;

  private:
    void calcNextClockPulse();
    float calcSwungClock(const uint8_t swingCycle, const float swingDepth);

    PlayerEngine *playerEngine = nullptr;
};
