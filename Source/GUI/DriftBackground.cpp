#include "DriftBackground.h"
#include "DriftLookAndFeel.h"
#include "BinaryData.h"
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

DriftBackground::DriftBackground()
{
    setInterceptsMouseClicks (false, false);
    backgroundImage = juce::ImageCache::getFromMemory (BinaryData::background_png,
                                                        BinaryData::background_pngSize);
}

void DriftBackground::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    // --- Background image (if available and not just a placeholder) ---
    if (backgroundImage.isValid() && backgroundImage.getWidth() > 1)
    {
        // Draw image covering the full area (cover mode)
        auto imgAspect = (float) backgroundImage.getWidth() / (float) backgroundImage.getHeight();
        auto boundsAspect = bounds.getWidth() / bounds.getHeight();

        juce::Rectangle<float> drawArea;
        if (boundsAspect > imgAspect)
        {
            float h = bounds.getWidth() / imgAspect;
            drawArea = { 0, (bounds.getHeight() - h) * 0.5f, bounds.getWidth(), h };
        }
        else
        {
            float w = bounds.getHeight() * imgAspect;
            drawArea = { (bounds.getWidth() - w) * 0.5f, 0, w, bounds.getHeight() };
        }

        g.drawImage (backgroundImage, drawArea);

        // Dark overlay so controls remain readable
        g.setColour (juce::Colour (0xcc0a0a1e));
        g.fillRect (bounds);
    }
    else
    {
        // Fallback: original gradient background
        juce::ColourGradient bgGrad (DriftLookAndFeel::bgTop, 0, 0,
                                      DriftLookAndFeel::bgBottom, 0, bounds.getHeight(), false);
        bgGrad.addColour (0.5, DriftLookAndFeel::bgMid);
        g.setGradientFill (bgGrad);
        g.fillRect (bounds);
    }

    // --- Subtle horizontal fog bands ---
    for (int i = 0; i < 5; ++i)
    {
        float y = bounds.getHeight() * (0.15f + i * 0.18f);
        float alpha = 0.03f + (i % 2) * 0.02f;

        juce::ColourGradient fogBand (juce::Colours::white.withAlpha (alpha),
                                       bounds.getCentreX(), y - 20,
                                       juce::Colours::transparentWhite,
                                       bounds.getCentreX(), y + 40, false);
        g.setGradientFill (fogBand);
        g.fillRect (0.0f, y - 20, bounds.getWidth(), 60.0f);
    }

    // --- Compass rose watermark ---
    drawCompassRose (g, bounds.getWidth() * 0.82f, bounds.getHeight() * 0.55f, 80.0f);

    // --- Plugin title ---
    g.setColour (DriftLookAndFeel::textColour.withAlpha (0.9f));
    g.setFont (juce::Font (22.0f, juce::Font::bold));
    g.drawText ("KIKINATOR", bounds.removeFromTop (40.0f).reduced (15, 8),
                juce::Justification::centredLeft);

    // --- Subtitle ---
    auto subtitleBounds = getLocalBounds().toFloat();
    g.setColour (DriftLookAndFeel::secondary.withAlpha (0.5f));
    g.setFont (juce::Font (11.0f));
    g.drawText ("Generative MIDI  |  Infinite Ambient Pads",
                subtitleBounds.removeFromTop (40.0f).reduced (15, 8),
                juce::Justification::centredRight);

    // --- Nautical chart lines (subtle grid) ---
    g.setColour (DriftLookAndFeel::secondary.withAlpha (0.04f));
    for (float x = 50; x < bounds.getWidth(); x += 100)
        g.drawLine (x, 0, x, bounds.getHeight(), 0.5f);
    for (float y = 50; y < bounds.getHeight(); y += 80)
        g.drawLine (0, y, bounds.getWidth(), y, 0.5f);

    // --- Vignette ---
    auto fullBounds = getLocalBounds().toFloat();
    juce::ColourGradient vignetteTop (juce::Colour (0x40000000), fullBounds.getCentreX(), 0,
                                       juce::Colours::transparentBlack, fullBounds.getCentreX(),
                                       fullBounds.getHeight() * 0.1f, false);
    g.setGradientFill (vignetteTop);
    g.fillRect (fullBounds);

    juce::ColourGradient vignetteBot (juce::Colours::transparentBlack, fullBounds.getCentreX(),
                                       fullBounds.getHeight() * 0.9f,
                                       juce::Colour (0x50000000), fullBounds.getCentreX(),
                                       fullBounds.getHeight(), false);
    g.setGradientFill (vignetteBot);
    g.fillRect (fullBounds);
}

void DriftBackground::drawCompassRose (juce::Graphics& g, float cx, float cy, float size)
{
    float alpha = 0.06f;

    // Outer circle
    g.setColour (DriftLookAndFeel::secondary.withAlpha (alpha));
    g.drawEllipse (cx - size, cy - size, size * 2, size * 2, 1.0f);

    // Inner circle
    g.drawEllipse (cx - size * 0.6f, cy - size * 0.6f, size * 1.2f, size * 1.2f, 0.5f);

    // Cardinal lines
    g.setColour (DriftLookAndFeel::secondary.withAlpha (alpha * 1.5f));
    g.drawLine (cx, cy - size, cx, cy + size, 0.8f);  // N-S
    g.drawLine (cx - size, cy, cx + size, cy, 0.8f);   // E-W

    // Diagonal lines
    g.setColour (DriftLookAndFeel::secondary.withAlpha (alpha));
    float diag = size * 0.7f;
    g.drawLine (cx - diag, cy - diag, cx + diag, cy + diag, 0.5f);
    g.drawLine (cx + diag, cy - diag, cx - diag, cy + diag, 0.5f);

    // North pointer (small triangle)
    juce::Path northPtr;
    northPtr.addTriangle (cx, cy - size * 0.9f,
                           cx - 5, cy - size * 0.7f,
                           cx + 5, cy - size * 0.7f);
    g.setColour (DriftLookAndFeel::accent.withAlpha (alpha * 2));
    g.fillPath (northPtr);

    // Cardinal labels
    g.setColour (DriftLookAndFeel::secondary.withAlpha (alpha * 2));
    g.setFont (juce::Font (9.0f, juce::Font::bold));
    g.drawText ("N", juce::Rectangle<float> (cx - 6, cy - size - 14, 12, 12), juce::Justification::centred);
    g.drawText ("S", juce::Rectangle<float> (cx - 6, cy + size + 2, 12, 12), juce::Justification::centred);
    g.drawText ("E", juce::Rectangle<float> (cx + size + 2, cy - 6, 12, 12), juce::Justification::centred);
    g.drawText ("W", juce::Rectangle<float> (cx - size - 14, cy - 6, 12, 12), juce::Justification::centred);
}
