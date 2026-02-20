#pragma once

/**
 * MicrotonalPitchBend — Converts cents offsets to MIDI pitch bend values.
 *
 * Each voice gets a unique, slowly wandering microtonal detune amount.
 * This creates the shimmering, slightly out-of-tune quality characteristic
 * of acoustic instruments in ambient/minimalist music.
 *
 * Assumes standard ±2 semitone pitch bend range (±200 cents).
 */
class MicrotonalPitchBend
{
public:
    MicrotonalPitchBend();

    /** Set the maximum detune depth in cents (0–50). */
    void setMaxCents (float cents);

    /** Set the voice index (0–7) for unique wandering. */
    void setVoiceIndex (int index);

    /** Advance the internal LFO by a time step (seconds). */
    void advance (double seconds);

    /** Reset the internal state. */
    void reset();

    /** Get the current cents offset. */
    float getCurrentCents() const;

    /** Get the current detune as a 14-bit MIDI pitch bend value (0–16383, center=8192). */
    int getPitchBendValue() const;

    /** Convert cents to pitch bend value (static utility).
        Assumes ±2 semitone (±200 cents) pitch bend range. */
    static int centsToPitchBend (float cents);

private:
    float maxCents = 15.0f;
    int voiceIndex = 0;
    double phase = 0.0;

    // Each voice has a unique slow LFO rate
    double getLFORate() const;
};
