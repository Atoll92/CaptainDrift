#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"
#include "GUI/DriftLookAndFeel.h"
#include "GUI/DriftBackground.h"
#include "GUI/MidiVisualizer.h"

class CaptainDriftEditor : public juce::AudioProcessorEditor
{
public:
    explicit CaptainDriftEditor (CaptainDriftProcessor&);
    ~CaptainDriftEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    CaptainDriftProcessor& processor;
    DriftLookAndFeel driftLnf;
    DriftBackground background;

    // --- On/Off toggle ---
    juce::ToggleButton genToggle;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> genToggleAtt;

    // --- MIDI Visualizer ---
    MidiVisualizer midiVisualizer;

    // --- Knobs ---
    // Navigation
    juce::Slider headingKnob, chartKnob, crewKnob;
    // Rhythm
    juce::Slider flotsamKnob, currentKnob, doldrumsKnob;
    // Dynamics
    juce::Slider galeKnob, shallowsKnob, depthsKnob;
    // Drift
    juce::Slider sargassoKnob, leewardKnob;
    // Evolution
    juce::Slider berthKnob, maelstromKnob;

    // --- Labels ---
    juce::Label headingLabel, chartLabel, crewLabel;
    juce::Label flotsamLabel, currentLabel, doldrumsLabel;
    juce::Label galeLabel, shallowsLabel, depthsLabel;
    juce::Label sargassoLabel, leewardLabel;
    juce::Label berthLabel, maelstromLabel;

    // Section labels
    juce::Label navigationTitle, rhythmTitle, dynamicsTitle, driftTitle, evolutionTitle;

    // --- Attachments ---
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> headingAtt, chartAtt, crewAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> flotsamAtt, currentAtt, doldrumsAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> galeAtt, shallowsAtt, depthsAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> sargassoAtt, leewardAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> berthAtt, maelstromAtt;

    // Helpers
    void setupKnob (juce::Slider& knob, juce::Label& label, const juce::String& text);
    void setupSectionLabel (juce::Label& label, const juce::String& text);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CaptainDriftEditor)
};
