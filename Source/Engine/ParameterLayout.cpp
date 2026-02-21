#include "ParameterLayout.h"

juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    // --- Navigation ---
    layout.add (std::make_unique<juce::AudioParameterInt> (
        juce::ParameterID { ID::heading, 1 }, "Heading",
        0, 11, 0));   // Root note C=0 .. B=11

    layout.add (std::make_unique<juce::AudioParameterChoice> (
        juce::ParameterID { ID::chart, 1 }, "Chart",
        juce::StringArray { "Major", "Minor", "Dorian", "Mixolydian",
                            "Pentatonic", "Whole Tone", "Chromatic" },
        0));

    layout.add (std::make_unique<juce::AudioParameterInt> (
        juce::ParameterID { ID::crew, 1 }, "Crew",
        1, 8, 4));   // Active voices

    // --- Rhythm ---
    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { ID::flotsam, 1 }, "Flotsam",
        juce::NormalisableRange<float> (0.25f, 4.0f, 0.0f, 0.5f),
        1.0f));   // Notes per bar

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { ID::current, 1 }, "Current",
        juce::NormalisableRange<float> (20.0f, 200.0f, 0.1f, 0.5f),
        60.0f));   // Internal BPM

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { ID::doldrums, 1 }, "Doldrums",
        juce::NormalisableRange<float> (0.1f, 1.0f, 0.01f),
        0.7f));   // Legato factor

    // --- Dynamics ---
    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { ID::gale, 1 }, "Gale",
        juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f),
        0.5f));   // Velocity range

    layout.add (std::make_unique<juce::AudioParameterInt> (
        juce::ParameterID { ID::shallows, 1 }, "Shallows",
        2, 6, 3));   // Octave min

    layout.add (std::make_unique<juce::AudioParameterInt> (
        juce::ParameterID { ID::depths, 1 }, "Depths",
        3, 7, 5));   // Octave max

    // --- Drift ---
    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { ID::sargasso, 1 }, "Sargasso",
        juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f),
        0.3f));   // Phase drift

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { ID::leeward, 1 }, "Leeward",
        juce::NormalisableRange<float> (0.0f, 50.0f, 0.1f),
        15.0f));   // Microtonal cents

    // --- Evolution ---
    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { ID::berth, 1 }, "Berth",
        juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f),
        0.5f));   // Evolution depth

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { ID::maelstrom, 1 }, "Maelstrom",
        juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f),
        0.2f));   // Randomness

    // --- Generation on/off ---
    layout.add (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID { ID::genEnabled, 1 }, "Generate",
        true));   // On by default

    return layout;
}
