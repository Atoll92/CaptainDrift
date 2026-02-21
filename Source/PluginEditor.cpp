#include "PluginEditor.h"
#include "Engine/ParameterLayout.h"

CaptainDriftEditor::CaptainDriftEditor (CaptainDriftProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
{
    setLookAndFeel (&driftLnf);
    setSize (700, 510);

    addAndMakeVisible (background);

    // --- Generation on/off toggle ---
    genToggle.setButtonText ("GEN");
    addAndMakeVisible (genToggle);

    // --- MIDI Visualizer ---
    midiVisualizer.setVoiceNoteSource (processor.voiceNotes);
    addAndMakeVisible (midiVisualizer);

    // --- Navigation group ---
    setupSectionLabel (navigationTitle, "NAVIGATION");
    setupKnob (headingKnob,  headingLabel,  "Heading");
    setupKnob (chartKnob,    chartLabel,    "Chart");
    setupKnob (crewKnob,     crewLabel,     "Crew");

    // --- Rhythm group ---
    setupSectionLabel (rhythmTitle, "RHYTHM");
    setupKnob (flotsamKnob,  flotsamLabel,  "Flotsam");
    setupKnob (currentKnob,  currentLabel,  "Current");
    setupKnob (doldrumsKnob, doldrumsLabel, "Doldrums");

    // --- Dynamics group ---
    setupSectionLabel (dynamicsTitle, "DYNAMICS");
    setupKnob (galeKnob,     galeLabel,     "Gale");
    setupKnob (shallowsKnob, shallowsLabel, "Shallows");
    setupKnob (depthsKnob,   depthsLabel,   "Depths");

    // --- Drift group ---
    setupSectionLabel (driftTitle, "DRIFT");
    setupKnob (sargassoKnob, sargassoLabel, "Sargasso");
    setupKnob (leewardKnob,  leewardLabel,  "Leeward");

    // --- Evolution group ---
    setupSectionLabel (evolutionTitle, "EVOLUTION");
    setupKnob (berthKnob,     berthLabel,     "Berth");
    setupKnob (maelstromKnob, maelstromLabel, "Maelstrom");

    // --- Attachments ---
    auto& apvts = processor.apvts;
    headingAtt   = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (apvts, ID::heading,   headingKnob);
    chartAtt     = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (apvts, ID::chart,     chartKnob);
    crewAtt      = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (apvts, ID::crew,      crewKnob);
    flotsamAtt   = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (apvts, ID::flotsam,   flotsamKnob);
    currentAtt   = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (apvts, ID::current,   currentKnob);
    doldrumsAtt  = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (apvts, ID::doldrums,  doldrumsKnob);
    galeAtt      = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (apvts, ID::gale,      galeKnob);
    shallowsAtt  = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (apvts, ID::shallows,  shallowsKnob);
    depthsAtt    = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (apvts, ID::depths,    depthsKnob);
    sargassoAtt  = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (apvts, ID::sargasso,  sargassoKnob);
    leewardAtt   = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (apvts, ID::leeward,   leewardKnob);
    berthAtt     = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (apvts, ID::berth,     berthKnob);
    maelstromAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (apvts, ID::maelstrom, maelstromKnob);

    genToggleAtt = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (apvts, ID::genEnabled, genToggle);
}

CaptainDriftEditor::~CaptainDriftEditor()
{
    setLookAndFeel (nullptr);
}

void CaptainDriftEditor::paint (juce::Graphics& g)
{
    // Background is handled by DriftBackground component
}

void CaptainDriftEditor::resized()
{
    auto bounds = getLocalBounds();
    background.setBounds (bounds);

    // Layout constants
    const int knobSize = 70;
    const int labelH = 18;
    const int sectionLabelH = 22;
    const int padX = 15;
    const int padY = 10;
    const int colWidth = (bounds.getWidth() - padX * 4) / 3;

    // --- On/Off toggle (top-right, in title bar area) ---
    genToggle.setBounds (bounds.getWidth() - 90, 10, 75, 24);

    // --- Top row: Navigation | Rhythm | Dynamics ---
    int topY = padY + 35;  // Space for plugin title area

    // Navigation section
    int navX = padX;
    navigationTitle.setBounds (navX, topY, colWidth, sectionLabelH);
    int knobY = topY + sectionLabelH + 5;
    int knobSpacing = knobSize + 5;

    headingKnob.setBounds (navX, knobY, knobSize, knobSize);
    headingLabel.setBounds (navX, knobY + knobSize, knobSize, labelH);

    chartKnob.setBounds (navX + knobSpacing, knobY, knobSize, knobSize);
    chartLabel.setBounds (navX + knobSpacing, knobY + knobSize, knobSize, labelH);

    crewKnob.setBounds (navX + knobSpacing * 2, knobY, knobSize, knobSize);
    crewLabel.setBounds (navX + knobSpacing * 2, knobY + knobSize, knobSize, labelH);

    // Rhythm section
    int rhythmX = padX + colWidth + padX;
    rhythmTitle.setBounds (rhythmX, topY, colWidth, sectionLabelH);

    flotsamKnob.setBounds (rhythmX, knobY, knobSize, knobSize);
    flotsamLabel.setBounds (rhythmX, knobY + knobSize, knobSize, labelH);

    currentKnob.setBounds (rhythmX + knobSpacing, knobY, knobSize, knobSize);
    currentLabel.setBounds (rhythmX + knobSpacing, knobY + knobSize, knobSize, labelH);

    doldrumsKnob.setBounds (rhythmX + knobSpacing * 2, knobY, knobSize, knobSize);
    doldrumsLabel.setBounds (rhythmX + knobSpacing * 2, knobY + knobSize, knobSize, labelH);

    // Dynamics section
    int dynX = padX + (colWidth + padX) * 2;
    dynamicsTitle.setBounds (dynX, topY, colWidth, sectionLabelH);

    galeKnob.setBounds (dynX, knobY, knobSize, knobSize);
    galeLabel.setBounds (dynX, knobY + knobSize, knobSize, labelH);

    shallowsKnob.setBounds (dynX + knobSpacing, knobY, knobSize, knobSize);
    shallowsLabel.setBounds (dynX + knobSpacing, knobY + knobSize, knobSize, labelH);

    depthsKnob.setBounds (dynX + knobSpacing * 2, knobY, knobSize, knobSize);
    depthsLabel.setBounds (dynX + knobSpacing * 2, knobY + knobSize, knobSize, labelH);

    // --- Bottom row: Drift | Evolution ---
    int botY = knobY + knobSize + labelH + padY + 15;

    // Drift section
    int driftX = padX + 30;
    driftTitle.setBounds (driftX, botY, colWidth, sectionLabelH);
    int botKnobY = botY + sectionLabelH + 5;

    sargassoKnob.setBounds (driftX, botKnobY, knobSize, knobSize);
    sargassoLabel.setBounds (driftX, botKnobY + knobSize, knobSize, labelH);

    leewardKnob.setBounds (driftX + knobSpacing, botKnobY, knobSize, knobSize);
    leewardLabel.setBounds (driftX + knobSpacing, botKnobY + knobSize, knobSize, labelH);

    // Evolution section
    int evoX = bounds.getWidth() / 2 + 40;
    evolutionTitle.setBounds (evoX, botY, colWidth, sectionLabelH);

    berthKnob.setBounds (evoX, botKnobY, knobSize, knobSize);
    berthLabel.setBounds (evoX, botKnobY + knobSize, knobSize, labelH);

    maelstromKnob.setBounds (evoX + knobSpacing, botKnobY, knobSize, knobSize);
    maelstromLabel.setBounds (evoX + knobSpacing, botKnobY + knobSize, knobSize, labelH);

    // --- MIDI Visualizer (bottom strip) ---
    int vizH = 55;
    int vizY = bounds.getHeight() - vizH - 8;
    midiVisualizer.setBounds (padX, vizY, bounds.getWidth() - padX * 2, vizH);
}

void CaptainDriftEditor::setupKnob (juce::Slider& knob, juce::Label& label, const juce::String& text)
{
    knob.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    knob.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 60, 16);
    knob.setTextBoxIsEditable (true);
    addAndMakeVisible (knob);

    label.setText (text, juce::dontSendNotification);
    label.setJustificationType (juce::Justification::centred);
    label.setFont (juce::Font (12.0f));
    label.setColour (juce::Label::textColourId, DriftLookAndFeel::textColour);
    addAndMakeVisible (label);
}

void CaptainDriftEditor::setupSectionLabel (juce::Label& label, const juce::String& text)
{
    label.setText (text, juce::dontSendNotification);
    label.setJustificationType (juce::Justification::centredLeft);
    label.setFont (juce::Font (14.0f, juce::Font::bold));
    label.setColour (juce::Label::textColourId, DriftLookAndFeel::accent);
    addAndMakeVisible (label);
}
