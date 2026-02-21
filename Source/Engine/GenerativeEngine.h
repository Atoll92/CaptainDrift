#pragma once
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "DriftVoice.h"
#include "ScaleQuantizer.h"
#include "EvolutionCurve.h"

/**
 * GenerativeEngine — Master coordinator for Captain Drift.
 *
 * Owns 8 DriftVoice instances, a ScaleQuantizer, and EvolutionCurves.
 * Reads parameters from the APVTS and distributes them to voices.
 * Manages the internal clock when host transport is not running.
 */
class GenerativeEngine
{
public:
    static constexpr int kMaxVoices = 8;

    GenerativeEngine();

    /** Prepare the engine for playback. */
    void prepare (double sampleRate, int blockSize);

    /** Reset all voices and state. */
    void reset();

    /** Update parameters from APVTS. Call once per processBlock. */
    void updateParameters (juce::AudioProcessorValueTreeState& apvts);

    /** Generate MIDI events for the current block.
        Uses host transport if available, otherwise uses internal clock. */
    void processBlock (juce::MidiBuffer& midiBuffer, int numSamples,
                       juce::AudioPlayHead* playHead);

    /** Query voice activity (safe for GUI polling). */
    int getVoiceNote (int index) const;
    bool isVoiceActive (int index) const;

private:
    double sampleRate = 44100.0;

    // Voices
    DriftVoice voices[kMaxVoices];
    int activeVoiceCount = 4;

    // Scale
    ScaleQuantizer scaleQuantizer;

    // Evolution curves (one per modulation target)
    EvolutionCurve evolutionDensity;     // Modulates note density
    EvolutionCurve evolutionVelocity;    // Modulates velocity
    EvolutionCurve evolutionOctave;      // Modulates octave spread

    // Internal clock (used when host transport is not running)
    double internalBeatPosition = 0.0;
    float internalBPM = 60.0f;
    bool wasPlaying = false;
    bool generationEnabled = true;
    bool wasGenerationEnabled = true;

    // Cached parameters
    float paramFlotsam = 1.0f;
    float paramCurrent = 60.0f;
    float paramDoldrums = 0.7f;
    float paramGale = 0.5f;
    int   paramShallows = 3;
    int   paramDepths = 5;
    float paramSargasso = 0.3f;
    float paramLeeward = 15.0f;
    float paramBerth = 0.5f;
    float paramMaelstrom = 0.2f;

    // Internal methods
    void updateVoiceParameters();
    double getBeatsPerSample (float bpm) const;
};
