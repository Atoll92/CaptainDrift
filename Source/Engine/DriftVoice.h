#pragma once
#include <juce_audio_basics/juce_audio_basics.h>
#include "ScaleQuantizer.h"
#include "PhaseAccumulator.h"
#include "MicrotonalPitchBend.h"
#include <random>

/**
 * DriftVoice — A single generative voice.
 *
 * Performs a constrained random walk within a musical scale,
 * generating MIDI note-on/off events and pitch bend messages.
 * Each voice occupies its own MIDI channel (1–8) for independent
 * microtonal pitch bend.
 */
class DriftVoice
{
public:
    DriftVoice();

    /** Initialize the voice with its index (0–7) and MIDI channel (1–8). */
    void init (int voiceIndex, int midiChannel);

    /** Reset to initial state. */
    void reset();

    /** Set parameters for this voice. */
    void setScaleQuantizer (const ScaleQuantizer* quantizer);
    void setNoteDensity (float notesPerBar);
    void setLegatoFactor (float legato);
    void setVelocityRange (float range);
    void setOctaveRange (int minOctave, int maxOctave);
    void setPhaseDrift (float driftAmount);
    void setMicrotonalDepth (float cents);
    void setRandomness (float chaos);

    /** Process a block of time. Adds MIDI events to the buffer.
        beatsPerSample: how many beats each sample represents.
        numSamples: number of samples in the block. */
    void processBlock (juce::MidiBuffer& midiBuffer, double beatsPerSample,
                       int numSamples, double secondsPerSample);

    /** Check if this voice is currently holding a note. */
    bool isNoteActive() const { return currentNote >= 0; }

    int getVoiceIndex() const { return voiceIdx; }

private:
    int voiceIdx = 0;
    int channel = 1;

    // State
    int currentNote = -1;        // Currently held MIDI note (-1 = none)
    int currentVelocity = 0;
    double noteOffCountdown = 0; // Beats until note-off
    int lastScaleDegreeOffset = 0;

    // Components
    PhaseAccumulator phaseAcc;
    MicrotonalPitchBend pitchBend;
    const ScaleQuantizer* scaleQ = nullptr;

    // Parameters
    float legatoFactor = 0.7f;
    float velocityRange = 0.5f;
    int octaveMin = 3;
    int octaveMax = 5;
    float randomness = 0.2f;

    // RNG
    std::mt19937 rng;

    // Internal methods
    void triggerNewNote (juce::MidiBuffer& midiBuffer, int samplePosition);
    void releaseCurrentNote (juce::MidiBuffer& midiBuffer, int samplePosition);
    int generateNextNote();
    int generateVelocity();
};
