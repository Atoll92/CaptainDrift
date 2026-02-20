#pragma once

/**
 * PhaseAccumulator — Per-voice timing with drift.
 *
 * Each voice has a base period (beats per note event) plus a tiny drift offset.
 * The drift offset creates the Steve Reich-style phase-shifting effect:
 * voices that start in unison gradually spread apart over time.
 */
class PhaseAccumulator
{
public:
    PhaseAccumulator();

    /** Reset the accumulator to zero. */
    void reset();

    /** Set the base period in beats (e.g., 4.0 = one note every 4 beats). */
    void setBasePeriod (double beatsPerNote);

    /** Set the drift offset as a fraction of the base period.
        driftAmount 0–1, voiceIndex 0–7. Each voice gets a unique drift. */
    void setDrift (float driftAmount, int voiceIndex);

    /** Advance the accumulator by a number of beats.
        Returns true if a note trigger occurred during this advance. */
    bool advance (double beats);

    /** Get the current phase (0..1 within the current period). */
    double getPhase() const;

    /** Get the effective period (base + drift). */
    double getEffectivePeriod() const { return basePeriod + driftOffset; }

private:
    double basePeriod  = 4.0;
    double driftOffset = 0.0;
    double phase       = 0.0;
};
