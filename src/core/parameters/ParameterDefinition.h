#pragma once

#include <functional>
#include <string>
#include <unordered_map>

// Struct for parameter definitions
struct ParamDefinition {
    float defaultValue; // Default value (e.g., 0-1 for continuous values)
    int snapSteps;      // Snap-steps (e.g., semitone or octave selection)
    bool logCurve;      // F(flat) L(log) E(num)
    // char curve;                               // F(flat) L(log) E(num)
    int minValue;                             // Minimum value
    int rangeFactor;                          // Factor for scaling (based on log or linear curve)
    std::function<void(float)> transformFunc; // Lambda for handling parameter changes
};

// Constructor for easy initialization
// ParamDefinition(float defaultVal, int steps, bool logCurve, int minValue, int rangeFactor, std::function<void(float)> func)
//    : defaultValue(defaultVal), snapSteps(steps), logCurve(logCurve), minValue(minValue), rangeFactor(rangeFactor), transformFunc(func){}
