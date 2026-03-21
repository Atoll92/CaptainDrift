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

GenerativeEngine::~GenerativeEngine()
{
    // Clean up shared state when instance is destroyed
    if (lastPublishedGroup > 0)
        SharedScaleState::getInstance().unpublish (lastPublishedGroup);
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
    generationEnabled = apvts.getRawParameterValue (ID::genEnabled)->load() >= 0.5f;
    droneMode = apvts.getRawParameterValue (ID::droneMode)->load() >= 0.5f;
    linkGroup = static_cast<int> (apvts.getRawParameterValue (ID::linkGroup)->load());
    linkRole  = static_cast<int> (apvts.getRawParameterValue (ID::linkRole)->load());

    // --- Scale Link: master publishes, follower overrides ---
    auto& shared = SharedScaleState::getInstance();

    // Clean up if we changed groups or went to unlinked
    if (lastPublishedGroup != 0 && lastPublishedGroup != linkGroup)
        shared.unpublish (lastPublishedGroup);
    lastPublishedGroup = (linkRole == 0) ? linkGroup : 0;

    if (linkGroup > 0)
    {
        if (linkRole == 0) // Master: publish our scale
        {
            shared.publish (linkGroup, rootNote, scaleIdx);
        }
        else // Follower: read master's scale
        {
            int masterRoot = 0, masterScale = 0;
            if (shared.subscribe (linkGroup, masterRoot, masterScale))
            {
                rootNote = masterRoot;
                scaleIdx = masterScale;
            }
            // If no master active, keep our own scale (graceful fallback)
        }
    }

    // Update scale (possibly overridden by master)
    scaleQuantizer.setRootNote (rootNote);
    scaleQuantizer.setScale (scaleIdx);

    // Update evolution depth (drone mode forces high evolution for slow breathing)
    float evoDepth = droneMode ? 0.9f : paramBerth;
    evolutionDensity.setDepth (evoDepth);
    evolutionVelocity.setDepth (evoDepth);
    evolutionOctave.setDepth (evoDepth);

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

    // If generation just got disabled, silence all voices
    if (! generationEnabled && wasGenerationEnabled)
    {
        for (int i = 0; i < kMaxVoices; ++i)
        {
            if (voices[i].isNoteActive())
                voices[i].reset();
        }
    }
    wasGenerationEnabled = generationEnabled;

    if (! generationEnabled)
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

int GenerativeEngine::getVoiceNote (int index) const
{
    if (index >= 0 && index < kMaxVoices)
        return voices[index].getCurrentNote();
    return -1;
}

bool GenerativeEngine::isVoiceActive (int index) const
{
    if (index >= 0 && index < kMaxVoices)
        return voices[index].isNoteActive();
    return false;
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
    if (densityMod < 0.01f) densityMod = 0.01f;
    float velocityMod = paramGale * (0.7f + 0.6f * evoV);

    // Evolution can shift octave range by ±1
    int octaveShift = static_cast<int> ((evoO - 0.5f) * 2.0f);
    int modShallows = juce::jlimit (2, 6, paramShallows + octaveShift);
    int modDepths   = juce::jlimit (3, 7, paramDepths + octaveShift);
    if (modDepths <= modShallows)
        modDepths = modShallows + 1;

    // In drone mode, override parameters for slow-evolving sustained tones
    float density   = droneMode ? 0.18f  : densityMod;
    float legato    = droneMode ? 1.0f   : paramDoldrums;
    float velRange  = droneMode ? 0.1f   : velocityMod;
    int   octLo     = droneMode ? 3      : modShallows;
    int   octHi     = droneMode ? 4      : modDepths;
    float drift     = droneMode ? 0.8f   : paramSargasso;
    float micro     = droneMode ? 25.0f  : paramLeeward;
    float chaos     = droneMode ? 0.08f  : paramMaelstrom;

    for (int i = 0; i < kMaxVoices; ++i)
    {
        voices[i].setScaleQuantizer (&scaleQuantizer);
        voices[i].setNoteDensity (density);
        voices[i].setLegatoFactor (legato);
        voices[i].setVelocityRange (velRange);
        voices[i].setOctaveRange (octLo, octHi);
        voices[i].setPhaseDrift (drift);
        voices[i].setMicrotonalDepth (micro);
        voices[i].setRandomness (chaos);
    }
}

double GenerativeEngine::getBeatsPerSample (float bpm) const
{
    if (bpm <= 0.0f || sampleRate <= 0.0)
        return 0.0;

    return static_cast<double> (bpm) / (60.0 * sampleRate);
}
