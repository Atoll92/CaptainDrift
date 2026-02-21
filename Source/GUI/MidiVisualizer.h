#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <atomic>

class MidiVisualizer : public juce::Component,
                       private juce::Timer
{
public:
    static constexpr int kNumVoices = 8;

    MidiVisualizer();

    /** Call this to point the visualizer at the processor's voice note atomics. */
    void setVoiceNoteSource (std::atomic<int>* noteArray);

    void paint (juce::Graphics& g) override;

private:
    void timerCallback() override;

    std::atomic<int>* voiceNotes = nullptr;

    // Smoothed brightness per voice (0..1)
    float brightness[kNumVoices] = {};
    int   displayedNote[kNumVoices] = {};

    static juce::String noteNameFromMidi (int midiNote);
};
