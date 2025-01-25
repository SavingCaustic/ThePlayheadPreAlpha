#pragma once

#include <functional>
#include <string>

// Struct for parameter definitions
struct ParamDefinition {
    std::string name;                         // string-name of param. for patch storage.
    float defaultValue;                       // Default value (e.g., 0-1 for continuous values)
    int snapSteps;                            // Snap-steps (e.g., semitone or octave selection)
    bool logCurve;                            // Logarithmic curve if true, linear if false
    int minValue;                             // Minimum value
    int rangeFactor;                          // Factor for scaling (based on log or linear curve)
    std::function<void(float)> transformFunc; // Lambda for handling parameter changes
};
