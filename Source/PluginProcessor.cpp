#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Engine/ParameterLayout.h"

CaptainDriftProcessor::CaptainDriftProcessor()
    : AudioProcessor (BusesProperties()
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMETERS", createParameterLayout())
{
}

CaptainDriftProcessor::~CaptainDriftProcessor() {}

const juce::String CaptainDriftProcessor::getName() const { return "Kikinator"; }

bool CaptainDriftProcessor::acceptsMidi()  const { return true; }
bool CaptainDriftProcessor::producesMidi() const { return true; }
bool CaptainDriftProcessor::isMidiEffect() const { return false; }
double CaptainDriftProcessor::getTailLengthSeconds() const { return 5.0; }

int CaptainDriftProcessor::getNumPrograms()    { return 1; }
int CaptainDriftProcessor::getCurrentProgram() { return 0; }
void CaptainDriftProcessor::setCurrentProgram (int) {}
const juce::String CaptainDriftProcessor::getProgramName (int) { return {}; }
void CaptainDriftProcessor::changeProgramName (int, const juce::String&) {}

bool CaptainDriftProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    return true;
}

void CaptainDriftProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    engine.prepare (sampleRate, samplesPerBlock);
    padSynth.prepare (sampleRate, samplesPerBlock);
}

void CaptainDriftProcessor::releaseResources()
{
    engine.reset();
    padSynth.reset();
}

void CaptainDriftProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                           juce::MidiBuffer& midiMessages)
{
    // Clear audio output
    buffer.clear();

    // Clear incoming MIDI (we generate our own)
    midiMessages.clear();

    // Update engine parameters from APVTS
    engine.updateParameters (apvts);

    // Generate MIDI events
    engine.processBlock (midiMessages, buffer.getNumSamples(), getPlayHead());

    // Update voice activity for visualizer
    for (int i = 0; i < GenerativeEngine::kMaxVoices; ++i)
        voiceNotes[i].store (engine.getVoiceNote (i), std::memory_order_relaxed);

    // Render the generated MIDI through the built-in pad synth
    padSynth.processBlock (buffer, midiMessages);
}

bool CaptainDriftProcessor::hasEditor() const { return true; }

juce::AudioProcessorEditor* CaptainDriftProcessor::createEditor()
{
    return new CaptainDriftEditor (*this);
}

void CaptainDriftProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void CaptainDriftProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml (getXmlFromBinary (data, sizeInBytes));

    if (xml != nullptr && xml->hasTagName (apvts.state.getType()))
        apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

// This creates new instances of the plugin
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new CaptainDriftProcessor();
}
