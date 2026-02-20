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

const juce::String CaptainDriftProcessor::getName() const { return "CaptainDrift"; }

bool CaptainDriftProcessor::acceptsMidi()  const { return false; }
bool CaptainDriftProcessor::producesMidi() const { return true; }
bool CaptainDriftProcessor::isMidiEffect() const { return false; }
double CaptainDriftProcessor::getTailLengthSeconds() const { return 0.0; }

int CaptainDriftProcessor::getNumPrograms()    { return 1; }
int CaptainDriftProcessor::getCurrentProgram() { return 0; }
void CaptainDriftProcessor::setCurrentProgram (int) {}
const juce::String CaptainDriftProcessor::getProgramName (int) { return {}; }
void CaptainDriftProcessor::changeProgramName (int, const juce::String&) {}

bool CaptainDriftProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    // We output stereo audio (silence) + MIDI
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    return true;
}

void CaptainDriftProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    engine.prepare (sampleRate, samplesPerBlock);
}

void CaptainDriftProcessor::releaseResources()
{
    engine.reset();
}

void CaptainDriftProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                           juce::MidiBuffer& midiMessages)
{
    // Clear audio output (we only generate MIDI)
    buffer.clear();

    // Clear incoming MIDI (we generate our own)
    midiMessages.clear();

    // Update engine parameters from APVTS
    engine.updateParameters (apvts);

    // Generate MIDI events
    engine.processBlock (midiMessages, buffer.getNumSamples(), getPlayHead());
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
