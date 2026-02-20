#include "MicrotonalPitchBend.h"
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

MicrotonalPitchBend::MicrotonalPitchBend() {}

void MicrotonalPitchBend::setMaxCents (float cents)
{
    maxCents = (cents < 0.0f) ? 0.0f : ((cents > 50.0f) ? 50.0f : cents);
}

void MicrotonalPitchBend::setVoiceIndex (int index)
{
    voiceIndex = (index >= 0 && index < 8) ? index : 0;
}

void MicrotonalPitchBend::advance (double seconds)
{
    phase += seconds * getLFORate();

    // Keep phase bounded to avoid precision loss over long periods
    if (phase > 1000000.0)
        phase = std::fmod (phase, 1000000.0);
}

void MicrotonalPitchBend::reset()
{
    phase = 0.0;
}

float MicrotonalPitchBend::getCurrentCents() const
{
    if (maxCents < 0.01f)
        return 0.0f;

    // Sum of two slow sinusoids at different rates for organic movement
    double sin1 = std::sin (2.0 * M_PI * phase);
    double sin2 = std::sin (2.0 * M_PI * phase * 1.618033988749895);  // golden ratio

    double combined = (sin1 * 0.7 + sin2 * 0.3);
    return static_cast<float> (combined * maxCents);
}

int MicrotonalPitchBend::getPitchBendValue() const
{
    return centsToPitchBend (getCurrentCents());
}

int MicrotonalPitchBend::centsToPitchBend (float cents)
{
    // ±2 semitones = ±200 cents = full pitch bend range
    // 8192 = center (no bend)
    // 0 = -200 cents, 16383 = +200 cents
    float normalized = cents / 200.0f;  // -1 to +1
    int bendValue = 8192 + static_cast<int> (normalized * 8191.0f);

    if (bendValue < 0)     bendValue = 0;
    if (bendValue > 16383) bendValue = 16383;

    return bendValue;
}

double MicrotonalPitchBend::getLFORate() const
{
    // Each voice gets a unique slow rate based on prime numbers
    // Rates range from ~0.01 Hz to ~0.05 Hz (20-100 second periods)
    static const double rates[8] = {
        0.0137,   // ~73s period
        0.0191,   // ~52s
        0.0239,   // ~42s
        0.0293,   // ~34s
        0.0347,   // ~29s
        0.0401,   // ~25s
        0.0457,   // ~22s
        0.0509    // ~20s
    };

    return rates[voiceIndex];
}
