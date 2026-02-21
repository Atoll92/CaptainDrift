#include "MidiVisualizer.h"
#include "DriftLookAndFeel.h"

MidiVisualizer::MidiVisualizer()
{
    startTimerHz (30);
}

void MidiVisualizer::setVoiceNoteSource (std::atomic<int>* noteArray)
{
    voiceNotes = noteArray;
}

void MidiVisualizer::timerCallback()
{
    if (voiceNotes == nullptr)
        return;

    bool changed = false;
    for (int i = 0; i < kNumVoices; ++i)
    {
        int note = voiceNotes[i].load (std::memory_order_relaxed);
        float target = (note >= 0) ? 1.0f : 0.0f;

        if (note >= 0)
            displayedNote[i] = note;

        float prev = brightness[i];
        // Smooth: fast attack, slow decay
        if (target > brightness[i])
            brightness[i] += (target - brightness[i]) * 0.5f;
        else
            brightness[i] += (target - brightness[i]) * 0.08f;

        if (brightness[i] < 0.01f)
            brightness[i] = 0.0f;

        if (std::abs (brightness[i] - prev) > 0.005f)
            changed = true;
    }

    if (changed)
        repaint();
}

void MidiVisualizer::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    // Background
    g.setColour (DriftLookAndFeel::knobBg.withAlpha (0.6f));
    g.fillRoundedRectangle (bounds, 4.0f);

    // Border
    g.setColour (DriftLookAndFeel::knobTrack);
    g.drawRoundedRectangle (bounds.reduced (0.5f), 4.0f, 1.0f);

    // Label
    g.setColour (DriftLookAndFeel::secondary.withAlpha (0.4f));
    g.setFont (juce::Font (9.0f));
    g.drawText ("MIDI OUT", bounds.reduced (4, 1), juce::Justification::topLeft);

    // Voice LEDs
    float ledAreaY = bounds.getY() + 14.0f;
    float ledAreaH = bounds.getHeight() - 18.0f;
    float totalW = bounds.getWidth() - 10.0f;
    float laneW = totalW / (float) kNumVoices;
    float startX = bounds.getX() + 5.0f;

    for (int i = 0; i < kNumVoices; ++i)
    {
        float x = startX + i * laneW;
        float cx = x + laneW * 0.5f;
        float cy = ledAreaY + ledAreaH * 0.4f;

        // LED circle
        float ledR = juce::jmin (laneW * 0.3f, ledAreaH * 0.25f);

        // Glow when active
        if (brightness[i] > 0.01f)
        {
            auto glowColour = DriftLookAndFeel::accent.withAlpha (brightness[i] * 0.3f);
            g.setColour (glowColour);
            g.fillEllipse (cx - ledR * 2, cy - ledR * 2, ledR * 4, ledR * 4);
        }

        // LED body
        auto ledColour = DriftLookAndFeel::accent.withAlpha (0.15f + brightness[i] * 0.85f);
        g.setColour (ledColour);
        g.fillEllipse (cx - ledR, cy - ledR, ledR * 2, ledR * 2);

        // Note name below LED
        if (brightness[i] > 0.1f && displayedNote[i] >= 0)
        {
            g.setColour (DriftLookAndFeel::textColour.withAlpha (brightness[i] * 0.8f));
            g.setFont (juce::Font (8.0f));
            g.drawText (noteNameFromMidi (displayedNote[i]),
                        juce::Rectangle<float> (cx - laneW * 0.5f, cy + ledR + 2, laneW, 10),
                        juce::Justification::centred);
        }

        // Voice number
        g.setColour (DriftLookAndFeel::secondary.withAlpha (0.25f));
        g.setFont (juce::Font (7.0f));
        g.drawText (juce::String (i + 1),
                    juce::Rectangle<float> (cx - 5, ledAreaY + ledAreaH - 10, 10, 10),
                    juce::Justification::centred);
    }
}

juce::String MidiVisualizer::noteNameFromMidi (int midiNote)
{
    static const char* names[] = { "C", "C#", "D", "D#", "E", "F",
                                    "F#", "G", "G#", "A", "A#", "B" };
    if (midiNote < 0 || midiNote > 127)
        return {};

    int octave = (midiNote / 12) - 1;
    return juce::String (names[midiNote % 12]) + juce::String (octave);
}
