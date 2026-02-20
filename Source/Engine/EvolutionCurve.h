#pragma once
#include <cmath>

/**
 * EvolutionCurve — 24-hour deterministic modulation source.
 *
 * Returns a value 0–1 that changes slowly over a 24-hour period.
 * Uses a sum of sinusoids at prime-ratio periods to create complex,
 * non-repeating (within the day) but deterministic modulation.
 *
 * Multiple independent curves with different phase seeds
 * allow different parameters to evolve differently.
 */
class EvolutionCurve
{
public:
    EvolutionCurve();

    /** Set the seed for this particular curve instance.
        Different seeds produce different evolution shapes. */
    void setSeed (int seed);

    /** Set the evolution depth (0–1). At 0, the curve always returns 0.5.
        At 1, the curve uses full range. */
    void setDepth (float depth);

    /** Evaluate the curve at a given time (seconds since midnight).
        Returns a value in [0, 1]. */
    float evaluate (double secondsSinceMidnight) const;

    /** Convenience: get current time as seconds since midnight. */
    static double getCurrentTimeSeconds();

private:
    int curveSeed = 0;
    float evolutionDepth = 0.5f;

    // Prime-ratio periods in seconds (hours * 3600)
    static constexpr double kPeriods[5] = {
        7.0 * 3600.0,    // 7 hours
        11.0 * 3600.0,   // 11 hours
        13.0 * 3600.0,   // 13 hours
        17.0 * 3600.0,   // 17 hours
        23.0 * 3600.0    // 23 hours
    };
};
