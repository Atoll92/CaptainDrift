#include "GenerativeEngine.h"
#include "ParameterLayout.h"

GenerativeEngine::GenerativeEngine()
{
    // Initialize voices with unique indices and MIDI channels
    for (int i = 0; i < kMaxVoices; ++i)
        voices[i].init (i, i + 1);   // Voice 0 → channel 1, etc.

    // Seed evolution curves with different seeds
    evolutionDensity.setSeed (1);
    evolutionVelocity.setSeed (2);
    evolutionOctave.setSeed (3);
}

void GenerativeEngine::prepare (double newSampleRate, int /*blockSize*/)
{
    sampleRate = newSampleRate;
    reset();
}

void GenerativeEngine::reset()
{
    internalBeatPosition = 0.0;
    wasPlaying = false;

    for (int i = 0; i < kMaxVoices; ++i)
        voices[i].reset();
}

void GenerativeEngine::updateParameters (juce::AudioProcessorValueTreeState& apvts)
{
    // Read all parameters from APVTS
    int rootNote = static_cast<int> (apvts.getRawParameterValue (ID::heading)->load());
    int scaleIdx = static_cast<int> (apvts.getRawParameterValue (ID::chart)->load());
    activeVoiceCount = static_cast<int> (apvts.getRawParameterValue (ID::crew)->load());

    paramFlotsam   = apvts.getRawParameterValue (ID::flotsam)->load();
    paramCurrent   = apvts.getRawParameterValue (ID::current)->load();
    paramDoldrums  = apvts.getRawParameterValue (ID::doldrums)->load();
    paramGale      = apvts.getRawParameterValue (ID::gale)->load();
    paramShallows  = static_cast<int> (apvts.getRawParameterValue (ID::shallows)->load());
    paramDepths    = static_cast<int> (apvts.getRawParameterValue (ID::depths)->load());
    paramSargasso  = apvts.getRawParameterValue (ID::sargasso)->load();
    paramLeeward   = apvts.getRawParameterValue (ID::leeward)->load();
    paramBerth     = apvts.getRawParameterValue (ID::berth)->load();
    paramMaelstrom = apvts.getRawParameterValue (ID::maelstrom)->load();

    // Update scale
    scaleQuantizer.setRootNote (rootNote);
    scaleQuantizer.setScale (scaleIdx);

    // Update evolution depth
    evolutionDensity.setDepth (paramBerth);
    evolutionVelocity.setDepth (paramBerth);
    evolutionOctave.setDepth (paramBerth);

    // Internal BPM from the Current parameter
    internalBPM = paramCurrent;

    // Update all voice parameters
    updateVoiceParameters();
}

void GenerativeEngine::processBlock (juce::MidiBuffer& midiBuffer, int numSamples,
                                      juce::AudioPlayHead* playHead)
{
    // Determine BPM and beat position
    float bpm = internalBPM;
    bool isPlaying = true;

    if (playHead != nullptr)
    {
        auto posInfo = playHead->getPosition();
        if (posInfo.hasValue())
        {
            if (auto bpmOpt = posInfo->getBpm())
                bpm = static_cast<float> (*bpmOpt);

            isPlaying = posInfo->getIsPlaying();
        }
    }

    // If host is not playing, use internal clock with the Current (BPM) parameter
    // Always generate when in standalone or when host is stopped
    // (for ambient installations, we always want output)
    if (! isPlaying)
    {
        bpm = internalBPM;
        isPlaying = true;  // Force playing for continuous generation
    }

    if (! isPlaying)
        return;

    double beatsPerSample = getBeatsPerSample (bpm);
    double secondsPerSample = 1.0 / sampleRate;

    // Process each active voice
    for (int i = 0; i < activeVoiceCount && i < kMaxVoices; ++i)
    {
        voices[i].processBlock (midiBuffer, beatsPerSample, numSamples, secondsPerSample);
    }

    // Send note-off for voices that just became inactive
    // (when crew count is reduced)
    for (int i = activeVoiceCount; i < kMaxVoices; ++i)
    {
        if (voices[i].isNoteActive())
        {
            // Force a note-off by processing with zero beats
            juce::MidiBuffer tempBuf;
            voices[i].processBlock (tempBuf, 0.0, 0, 0.0);

            // If still active, manually send note-off
            if (voices[i].isNoteActive())
                voices[i].reset();
        }
    }

    // Advance internal clock
    internalBeatPosition += beatsPerSample * numSamples;
}

void GenerativeEngine::updateVoiceParameters()
{
    // Get evolution modulation values
    double timeSeconds = EvolutionCurve::getCurrentTimeSeconds();
    float evoD = evolutionDensity.evaluate (timeSeconds);    // 0–1
    float evoV = evolutionVelocity.evaluate (timeSeconds);   // 0–1
    float evoO = evolutionOctave.evaluate (timeSeconds);     // 0–1

    // Modulate parameters with evolution curves
    // Evolution adds ±30% variation to base values
    float densityMod = paramFlotsam * (0.7f + 0.6f * evoD);
    float velocityMod = paramGale * (0.7f + 0.6f * evoV);

    // Evolution can shift octave range by ±1
    int octaveShift = static_cast<int> ((evoO - 0.5f) * 2.0f);
    int modShallows = juce::jlimit (2, 6, paramShallows + octaveShift);
    int modDepths   = juce::jlimit (3, 7, paramDepths + octaveShift);
    if (modDepths <= modShallows)
        modDepths = modShallows + 1;

    for (int i = 0; i < kMaxVoices; ++i)
    {
        voices[i].setScaleQuantizer (&scaleQuantizer);
        voices[i].setNoteDensity (densityMod);
        voices[i].setLegatoFactor (paramDoldrums);
        voices[i].setVelocityRange (velocityMod);
        voices[i].setOctaveRange (modShallows, modDepths);
        voices[i].setPhaseDrift (paramSargasso);
        voices[i].setMicrotonalDepth (paramLeeward);
        voices[i].setRandomness (paramMaelstrom);
    }
}

double GenerativeEngine::getBeatsPerSample (float bpm) const
{
    if (bpm <= 0.0f || sampleRate <= 0.0)
        return 0.0;

    return static_cast<double> (bpm) / (60.0 * sampleRate);
}
