#include "PhaseAccumulator.h"
#include <cmath>

PhaseAccumulator::PhaseAccumulator() {}

void PhaseAccumulator::reset()
{
    phase = 0.0;
}

void PhaseAccumulator::setBasePeriod (double beatsPerNote)
{
    basePeriod = (beatsPerNote > 0.01) ? beatsPerNote : 0.01;
}

void PhaseAccumulator::setDrift (float driftAmount, int voiceIndex)
{
    // Each voice gets a unique drift offset proportional to its index.
    // Prime ratios ensure voices never re-align at simple intervals.
    static const double primeFactors[8] = {
        0.0,        // voice 0: no drift (reference)
        0.00731,    // voice 1
        0.01307,    // voice 2
        0.01901,    // voice 3
        0.02503,    // voice 4
        0.03109,    // voice 5
        0.03701,    // voice 6
        0.04303     // voice 7
    };

    int idx = (voiceIndex >= 0 && voiceIndex < 8) ? voiceIndex : 0;
    driftOffset = basePeriod * primeFactors[idx] * static_cast<double> (driftAmount);
}

bool PhaseAccumulator::advance (double beats)
{
    double period = getEffectivePeriod();
    if (period <= 0.0)
        return false;

    phase += beats;

    if (phase >= period)
    {
        // Wrap phase, preserving fractional overshoot for timing accuracy
        phase = std::fmod (phase, period);
        return true;   // trigger!
    }

    return false;
}

double PhaseAccumulator::getPhase() const
{
    double period = basePeriod + driftOffset;
    if (period <= 0.0)
        return 0.0;

    return phase / period;
}
