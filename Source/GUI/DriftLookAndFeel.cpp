#include "DriftLookAndFeel.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

DriftLookAndFeel::DriftLookAndFeel()
{
    // Set default colours
    setColour (juce::Slider::textBoxTextColourId,    textColour);
    setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    setColour (juce::Slider::textBoxBackgroundColourId, knobBg.withAlpha (0.5f));
    setColour (juce::Label::textColourId, textColour);
    setColour (juce::ResizableWindow::backgroundColourId, bgTop);
}

void DriftLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                                          float sliderPos, float rotaryStartAngle,
                                          float rotaryEndAngle, juce::Slider&)
{
    auto bounds = juce::Rectangle<float> (static_cast<float> (x), static_cast<float> (y),
                                           static_cast<float> (width), static_cast<float> (height));

    // Make it square, centered
    auto side = juce::jmin (bounds.getWidth(), bounds.getHeight());
    auto area = bounds.withSizeKeepingCentre (side, side).reduced (4.0f);
    auto radius = area.getWidth() / 2.0f;
    auto centreX = area.getCentreX();
    auto centreY = area.getCentreY();

    auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

    // --- Knob background circle ---
    g.setColour (knobBg);
    g.fillEllipse (area);

    // --- Track arc (full range, dim) ---
    {
        juce::Path trackArc;
        auto trackRadius = radius - 6.0f;
        trackArc.addCentredArc (centreX, centreY, trackRadius, trackRadius,
                                 0.0f, rotaryStartAngle, rotaryEndAngle, true);
        g.setColour (knobTrack);
        g.strokePath (trackArc, juce::PathStrokeType (3.0f, juce::PathStrokeType::curved,
                                                       juce::PathStrokeType::rounded));
    }

    // --- Value arc (warm amber gradient) ---
    {
        juce::Path valueArc;
        auto arcRadius = radius - 6.0f;
        valueArc.addCentredArc (centreX, centreY, arcRadius, arcRadius,
                                 0.0f, rotaryStartAngle, angle, true);

        juce::ColourGradient arcGrad (accent, centreX, centreY - radius,
                                       secondary, centreX, centreY + radius, false);
        g.setGradientFill (arcGrad);
        g.strokePath (valueArc, juce::PathStrokeType (3.0f, juce::PathStrokeType::curved,
                                                       juce::PathStrokeType::rounded));
    }

    // --- Pointer line ---
    {
        auto pointerLength = radius - 12.0f;
        auto pointerThickness = 2.5f;

        juce::Path pointer;
        pointer.addRectangle (-pointerThickness / 2.0f, -pointerLength, pointerThickness, pointerLength);

        pointer.applyTransform (juce::AffineTransform::rotation (angle)
                                     .translated (centreX, centreY));
        g.setColour (textColour);
        g.fillPath (pointer);
    }

    // --- Glowing dot at pointer tip ---
    {
        auto dotRadius = 3.5f;
        auto dotDist = radius - 14.0f;
        auto dotX = centreX + dotDist * std::sin (angle);
        auto dotY = centreY - dotDist * std::cos (angle);

        // Glow
        g.setColour (accent.withAlpha (0.4f));
        g.fillEllipse (dotX - dotRadius * 2, dotY - dotRadius * 2, dotRadius * 4, dotRadius * 4);

        // Dot
        g.setColour (accent);
        g.fillEllipse (dotX - dotRadius, dotY - dotRadius, dotRadius * 2, dotRadius * 2);
    }

    // --- Subtle outer ring ---
    g.setColour (accent.withAlpha (0.15f));
    g.drawEllipse (area.reduced (1.0f), 1.0f);
}

void DriftLookAndFeel::drawLabel (juce::Graphics& g, juce::Label& label)
{
    g.setColour (label.findColour (juce::Label::textColourId));
    g.setFont (label.getFont());

    auto textArea = label.getBorderSize().subtractedFrom (label.getLocalBounds());
    g.drawFittedText (label.getText(), textArea, label.getJustificationType(),
                      juce::jmax (1, (int) ((float) textArea.getHeight() / label.getFont().getHeight())),
                      label.getMinimumHorizontalScale());
}
