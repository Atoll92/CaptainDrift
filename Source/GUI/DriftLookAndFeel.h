#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

class DriftLookAndFeel : public juce::LookAndFeel_V4
{
public:
    // Nautical fog/mist palette
    static inline const juce::Colour bgTop     { 0xff1a1a2e };
    static inline const juce::Colour bgMid     { 0xff16213e };
    static inline const juce::Colour bgBottom  { 0xff0f3460 };
    static inline const juce::Colour accent    { 0xffe94560 };   // Warm amber/red
    static inline const juce::Colour secondary { 0xffd4a574 };   // Pale gold
    static inline const juce::Colour textColour{ 0xffe0d8cc };   // Warm off-white
    static inline const juce::Colour knobBg    { 0xff1e2a3a };   // Dark blue-grey
    static inline const juce::Colour knobTrack { 0xff2a3a4e };   // Slightly lighter

    DriftLookAndFeel();

    void drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                           float sliderPosProportional, float rotaryStartAngle,
                           float rotaryEndAngle, juce::Slider& slider) override;

    void drawLabel (juce::Graphics& g, juce::Label& label) override;
};
