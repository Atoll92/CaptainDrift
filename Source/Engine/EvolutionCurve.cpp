#include "EvolutionCurve.h"
#include <chrono>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

EvolutionCurve::EvolutionCurve() {}

void EvolutionCurve::setSeed (int seed)
{
    curveSeed = seed;
}

void EvolutionCurve::setDepth (float depth)
{
    evolutionDepth = (depth < 0.0f) ? 0.0f : ((depth > 1.0f) ? 1.0f : depth);
}

float EvolutionCurve::evaluate (double secondsSinceMidnight) const
{
    if (evolutionDepth <= 0.0001f)
        return 0.5f;

    // Sum of 5 sinusoids at prime-ratio periods, each with a seed-based phase offset
    double sum = 0.0;

    for (int i = 0; i < 5; ++i)
    {
        // Phase offset derived from seed — different seed = different shape
        double phaseOffset = static_cast<double> ((curveSeed * 7919 + i * 6271) % 10000) / 10000.0 * 2.0 * M_PI;

        double angle = (2.0 * M_PI * secondsSinceMidnight / kPeriods[i]) + phaseOffset;
        sum += std::sin (angle);
    }

    // Normalize from [-5, 5] to [0, 1]
    double normalized = (sum / 5.0 + 1.0) * 0.5;

    // Apply depth: lerp between 0.5 (no evolution) and normalized (full evolution)
    float result = 0.5f + static_cast<float> ((normalized - 0.5) * evolutionDepth);

    return (result < 0.0f) ? 0.0f : ((result > 1.0f) ? 1.0f : result);
}

double EvolutionCurve::getCurrentTimeSeconds()
{
    auto now = std::chrono::system_clock::now();
    auto nowSeconds = std::chrono::duration_cast<std::chrono::seconds> (now.time_since_epoch()).count();

    // Calculate seconds since midnight (86400 seconds in a day)
    auto secondsSinceMidnight = nowSeconds % 86400;
    return static_cast<double> (secondsSinceMidnight);
}
