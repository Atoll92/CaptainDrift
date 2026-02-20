#pragma once
#include <juce_audio_processors/juce_audio_processors.h>

namespace ID
{
    inline constexpr const char* heading   = "heading";    // Root note (0-11)
    inline constexpr const char* chart     = "chart";      // Scale type (0-6)
    inline constexpr const char* crew      = "crew";       // Active voices (1-8)
    inline constexpr const char* flotsam   = "flotsam";    // Note density
    inline constexpr const char* current   = "current";    // Internal tempo BPM
    inline constexpr const char* doldrums  = "doldrums";   // Note length (legato)
    inline constexpr const char* gale      = "gale";       // Velocity range
    inline constexpr const char* shallows  = "shallows";   // Octave min
    inline constexpr const char* depths    = "depths";     // Octave max
    inline constexpr const char* sargasso  = "sargasso";   // Phase drift amount
    inline constexpr const char* leeward   = "leeward";    // Microtonal detune (cents)
    inline constexpr const char* berth     = "berth";      // Evolution depth
    inline constexpr const char* maelstrom = "maelstrom";  // Randomness amount
}

juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
