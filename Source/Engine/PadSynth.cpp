#include "PadSynth.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

PadSynth::PadSynth()
{
    channelPitchBend.fill (1.0);
}

void PadSynth::prepare (double newSampleRate, int /*blockSize*/)
{
    sampleRate = newSampleRate;
    reset();
}

void PadSynth::reset()
{
    for (auto& v : voices)
    {
        v.active = false;
        v.noteNumber = -1;
        v.envelope = 0.0f;
        v.releasing = false;
    }

    lpState[0] = 0.0f;
    lpState[1] = 0.0f;
    channelPitchBend.fill (1.0);
}

void PadSynth::processBlock (juce::AudioBuffer<float>& audioBuffer,
                              const juce::MidiBuffer& midiBuffer)
{
    auto numSamples = audioBuffer.getNumSamples();
    auto numChannels = audioBuffer.getNumChannels();

    // Process MIDI events
    for (const auto metadata : midiBuffer)
    {
        auto msg = metadata.getMessage();

        if (msg.isNoteOn())
            noteOn (msg.getChannel(), msg.getNoteNumber(), msg.getFloatVelocity());
        else if (msg.isNoteOff())
            noteOff (msg.getChannel(), msg.getNoteNumber());
        else if (msg.isPitchWheel())
            handlePitchBend (msg.getChannel(), msg.getPitchWheelValue());
    }

    // Render audio
    for (int sample = 0; sample < numSamples; ++sample)
    {
        float monoSample = 0.0f;

        for (auto& voice : voices)
        {
            if (! voice.active)
                continue;

            // Update pitch bend factor from channel
            if (voice.channel >= 1 && voice.channel <= 16)
                voice.pitchBendFactor = channelPitchBend[static_cast<size_t> (voice.channel - 1)];

            monoSample += renderVoice (voice);

            // Deactivate voice when envelope reaches zero during release
            if (voice.releasing && voice.envelope <= 0.0001f)
            {
                voice.active = false;
                voice.noteNumber = -1;
            }
        }

        // Simple one-pole lowpass for warmth (cutoff ~3kHz)
        float lpCoeff = 1.0f - std::exp (-2.0f * static_cast<float> (M_PI) * 3000.0f / static_cast<float> (sampleRate));
        lpState[0] += lpCoeff * (monoSample - lpState[0]);

        // Soft clip
        float out = std::tanh (lpState[0] * 0.7f);

        // Write to all output channels
        for (int ch = 0; ch < numChannels; ++ch)
            audioBuffer.addSample (ch, sample, out);
    }
}

void PadSynth::noteOn (int channel, int note, float velocity)
{
    // Check if this note is already playing on this channel
    for (auto& v : voices)
    {
        if (v.active && v.noteNumber == note && v.channel == channel)
        {
            // Retrigger: reset release state
            v.releasing = false;
            v.velocity = velocity;
            return;
        }
    }

    // Find a free voice
    SynthVoice* freeVoice = nullptr;

    // First, look for an inactive voice
    for (auto& v : voices)
    {
        if (! v.active)
        {
            freeVoice = &v;
            break;
        }
    }

    // If no free voice, steal the quietest releasing voice
    if (freeVoice == nullptr)
    {
        float quietest = 999.0f;
        for (auto& v : voices)
        {
            if (v.releasing && v.envelope < quietest)
            {
                quietest = v.envelope;
                freeVoice = &v;
            }
        }
    }

    // Last resort: steal the quietest voice
    if (freeVoice == nullptr)
    {
        float quietest = 999.0f;
        for (auto& v : voices)
        {
            if (v.envelope < quietest)
            {
                quietest = v.envelope;
                freeVoice = &v;
            }
        }
    }

    if (freeVoice == nullptr)
        return;

    freeVoice->active = true;
    freeVoice->noteNumber = note;
    freeVoice->channel = channel;
    freeVoice->velocity = velocity;
    freeVoice->baseFreq = midiNoteToFreq (note);
    freeVoice->releasing = false;
    freeVoice->releaseLevel = 0.0f;
    freeVoice->releasePhase = 0.0f;

    // Don't reset phases for smoother transitions
    if (freeVoice->envelope < 0.001f)
    {
        freeVoice->phase1 = 0.0;
        freeVoice->phase2 = 0.0;
        freeVoice->phase3 = 0.0;
        freeVoice->phase4 = 0.0;
        freeVoice->envelope = 0.0f;
    }

    // Apply channel pitch bend
    if (channel >= 1 && channel <= 16)
        freeVoice->pitchBendFactor = channelPitchBend[static_cast<size_t> (channel - 1)];
}

void PadSynth::noteOff (int channel, int note)
{
    for (auto& v : voices)
    {
        if (v.active && v.noteNumber == note && v.channel == channel && ! v.releasing)
        {
            v.releasing = true;
            v.releaseLevel = v.envelope;
            v.releasePhase = 0.0f;
        }
    }
}

void PadSynth::handlePitchBend (int channel, int bendValue)
{
    if (channel < 1 || channel > 16)
        return;

    // Convert 14-bit pitch bend to frequency ratio
    // Center = 8192, range = ±2 semitones
    double semitones = (static_cast<double> (bendValue) - 8192.0) / 8192.0 * 2.0;
    channelPitchBend[static_cast<size_t> (channel - 1)] = std::pow (2.0, semitones / 12.0);
}

float PadSynth::renderVoice (SynthVoice& voice)
{
    double freq = voice.baseFreq * voice.pitchBendFactor;

    // Detune factors
    double detuneUp   = std::pow (2.0, kDetuneCents / 1200.0);
    double detuneDown = std::pow (2.0, -kDetuneCents / 1200.0);

    // Phase increments
    double inc1 = freq / sampleRate;
    double inc2 = freq * detuneUp / sampleRate;
    double inc3 = freq * detuneDown / sampleRate;
    double inc4 = (freq * 0.5) / sampleRate;   // Sub octave

    // Advance phases
    voice.phase1 += inc1;
    voice.phase2 += inc2;
    voice.phase3 += inc3;
    voice.phase4 += inc4;

    if (voice.phase1 >= 1.0) voice.phase1 -= 1.0;
    if (voice.phase2 >= 1.0) voice.phase2 -= 1.0;
    if (voice.phase3 >= 1.0) voice.phase3 -= 1.0;
    if (voice.phase4 >= 1.0) voice.phase4 -= 1.0;

    // Generate waveforms (sine for smooth pad sound)
    float osc1 = static_cast<float> (std::sin (2.0 * M_PI * voice.phase1));
    float osc2 = static_cast<float> (std::sin (2.0 * M_PI * voice.phase2));
    float osc3 = static_cast<float> (std::sin (2.0 * M_PI * voice.phase3));
    float osc4 = static_cast<float> (std::sin (2.0 * M_PI * voice.phase4));

    // Mix: main + detuned + sub
    float mix = osc1 * 0.4f + osc2 * 0.2f + osc3 * 0.2f + osc4 * 0.2f;

    // Update envelope
    if (! voice.releasing)
    {
        // Attack
        voice.envelope += kAttackRate;
        if (voice.envelope > 1.0f)
            voice.envelope = 1.0f;
    }
    else
    {
        // Release
        voice.releasePhase += kReleaseRate;
        if (voice.releasePhase >= 1.0f)
        {
            voice.envelope = 0.0f;
        }
        else
        {
            // Exponential release curve
            voice.envelope = voice.releaseLevel * (1.0f - voice.releasePhase) * (1.0f - voice.releasePhase);
        }
    }

    return mix * voice.envelope * voice.velocity * 0.15f;
}

double PadSynth::midiNoteToFreq (int note) const
{
    return 440.0 * std::pow (2.0, (static_cast<double> (note) - 69.0) / 12.0);
}
