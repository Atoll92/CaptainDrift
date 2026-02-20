#include "DriftVoice.h"
#include <cmath>

DriftVoice::DriftVoice()
{
}

void DriftVoice::init (int voiceIndex, int midiChannel)
{
    voiceIdx = voiceIndex;
    channel = midiChannel;
    pitchBend.setVoiceIndex (voiceIndex);

    // Seed RNG with voice index for deterministic but unique behavior
    rng.seed (static_cast<unsigned> (42 + voiceIndex * 997));
}

void DriftVoice::reset()
{
    currentNote = -1;
    currentVelocity = 0;
    noteOffCountdown = 0;
    lastScaleDegreeOffset = 0;
    phaseAcc.reset();
    pitchBend.reset();
}

void DriftVoice::setScaleQuantizer (const ScaleQuantizer* quantizer)
{
    scaleQ = quantizer;
}

void DriftVoice::setNoteDensity (float notesPerBar)
{
    // Convert notes-per-bar to beats-per-note (assuming 4/4 time)
    double beatsPerNote = 4.0 / static_cast<double> (notesPerBar);
    phaseAcc.setBasePeriod (beatsPerNote);
}

void DriftVoice::setLegatoFactor (float legato)
{
    legatoFactor = juce::jlimit (0.1f, 1.0f, legato);
}

void DriftVoice::setVelocityRange (float range)
{
    velocityRange = juce::jlimit (0.0f, 1.0f, range);
}

void DriftVoice::setOctaveRange (int minOctave, int maxOctave)
{
    octaveMin = juce::jlimit (2, 6, minOctave);
    octaveMax = juce::jlimit (3, 7, maxOctave);

    if (octaveMax <= octaveMin)
        octaveMax = octaveMin + 1;
}

void DriftVoice::setPhaseDrift (float driftAmount)
{
    phaseAcc.setDrift (driftAmount, voiceIdx);
}

void DriftVoice::setMicrotonalDepth (float cents)
{
    pitchBend.setMaxCents (cents);
}

void DriftVoice::setRandomness (float chaos)
{
    randomness = juce::jlimit (0.0f, 1.0f, chaos);
}

void DriftVoice::processBlock (juce::MidiBuffer& midiBuffer, double beatsPerSample,
                                int numSamples, double secondsPerSample)
{
    for (int sample = 0; sample < numSamples; ++sample)
    {
        // Advance microtonal LFO
        pitchBend.advance (secondsPerSample);

        // Count down note-off timer
        if (currentNote >= 0)
        {
            noteOffCountdown -= beatsPerSample;

            if (noteOffCountdown <= 0.0)
            {
                releaseCurrentNote (midiBuffer, sample);
            }
        }

        // Advance phase accumulator and check for trigger
        if (phaseAcc.advance (beatsPerSample))
        {
            // Release any held note first
            if (currentNote >= 0)
                releaseCurrentNote (midiBuffer, sample);

            triggerNewNote (midiBuffer, sample);
        }

        // Send pitch bend updates periodically (every ~64 samples to save bandwidth)
        if (currentNote >= 0 && (sample % 64 == 0))
        {
            int bendValue = pitchBend.getPitchBendValue();
            auto bendMsg = juce::MidiMessage::pitchWheel (channel, bendValue);
            midiBuffer.addEvent (bendMsg, sample);
        }
    }
}

void DriftVoice::triggerNewNote (juce::MidiBuffer& midiBuffer, int samplePosition)
{
    int note = generateNextNote();
    int velocity = generateVelocity();

    if (note < 0 || note > 127)
        return;

    currentNote = note;
    currentVelocity = velocity;

    // Note duration = legato factor * effective period
    noteOffCountdown = phaseAcc.getEffectivePeriod() * static_cast<double> (legatoFactor);

    // Send pitch bend before note-on
    int bendValue = pitchBend.getPitchBendValue();
    midiBuffer.addEvent (juce::MidiMessage::pitchWheel (channel, bendValue), samplePosition);

    // Send note-on
    midiBuffer.addEvent (juce::MidiMessage::noteOn (channel, note, (juce::uint8) velocity), samplePosition);
}

void DriftVoice::releaseCurrentNote (juce::MidiBuffer& midiBuffer, int samplePosition)
{
    if (currentNote >= 0)
    {
        midiBuffer.addEvent (juce::MidiMessage::noteOff (channel, currentNote, (juce::uint8) 0), samplePosition);
        currentNote = -1;
        currentVelocity = 0;
    }
}

int DriftVoice::generateNextNote()
{
    if (scaleQ == nullptr)
        return 60;  // Middle C fallback

    int lowNote = octaveMin * 12;    // MIDI note at bottom of range
    int highNote = (octaveMax + 1) * 12 - 1;  // MIDI note at top of range

    auto scaleNotes = scaleQ->getNotesInRange (lowNote, highNote);
    if (scaleNotes.empty())
        return 60;

    // Constrained random walk: move ±1–3 scale degrees from current position
    int maxStep = 1 + static_cast<int> (randomness * 3.0f);

    std::uniform_int_distribution<int> stepDist (-maxStep, maxStep);
    int step = stepDist (rng);

    // Bias toward small steps (more melodic)
    if (std::abs (step) > 1)
    {
        std::uniform_real_distribution<float> biasDist (0.0f, 1.0f);
        if (biasDist (rng) > randomness)
            step = (step > 0) ? 1 : -1;
    }

    lastScaleDegreeOffset += step;

    // Find the target note
    int centerIndex = static_cast<int> (scaleNotes.size()) / 2;
    int targetIndex = centerIndex + lastScaleDegreeOffset;

    // Wrap around if out of range (bounce off edges)
    if (targetIndex < 0)
    {
        targetIndex = -targetIndex;
        lastScaleDegreeOffset = targetIndex - centerIndex;
    }
    if (targetIndex >= static_cast<int> (scaleNotes.size()))
    {
        targetIndex = static_cast<int> (scaleNotes.size()) - 1 - (targetIndex - static_cast<int> (scaleNotes.size()) + 1);
        if (targetIndex < 0) targetIndex = 0;
        lastScaleDegreeOffset = targetIndex - centerIndex;
    }

    return scaleNotes[targetIndex];
}

int DriftVoice::generateVelocity()
{
    // Base velocity 60–100, modulated by velocityRange parameter
    float baseVelocity = 80.0f;
    float spread = 30.0f * velocityRange;

    std::uniform_real_distribution<float> dist (-spread, spread);
    float vel = baseVelocity + dist (rng);

    return juce::jlimit (30, 120, static_cast<int> (vel));
}
