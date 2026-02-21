#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

class DriftBackground : public juce::Component
{
public:
    DriftBackground();
    void paint (juce::Graphics& g) override;

private:
    juce::Image backgroundImage;
    void drawCompassRose (juce::Graphics& g, float centreX, float centreY, float size);
};
