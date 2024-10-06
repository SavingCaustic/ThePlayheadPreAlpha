#pragma once

#include <functional>
#include <string>
#include <unordered_map>

// Struct for parameter definitions
struct ParamDefinition {
    float defaultValue;                       // Default value (e.g., 0-1 for continuous values)
    int snapSteps;                            // Snap-steps (e.g., semitone or octave selection)
    bool logCurve;                            // Logarithmic curve if true, linear if false
    int minValue;                             // Minimum value
    int rangeFactor;                          // Factor for scaling (based on log or linear curve)
    std::function<void(float)> transformFunc; // Lambda for handling parameter changes
};

// Probably move to SynthInterface..
// extern std::unordered_map<std::string, ParamDefinition> parameterDefinitions;
