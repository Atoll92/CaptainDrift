#include "ScaleQuantizer.h"
#include <cmath>

ScaleQuantizer::ScaleQuantizer()
{
    rebuildNoteSet();
}

void ScaleQuantizer::setRootNote (int root)
{
    rootNote = juce::jlimit (0, 11, root);
    rebuildNoteSet();
}

void ScaleQuantizer::setScale (Scale scale)
{
    currentScale = scale;
    rebuildNoteSet();
}

void ScaleQuantizer::setScale (int scaleIndex)
{
    setScale (static_cast<Scale> (juce::jlimit (0, (int) NumScales - 1, scaleIndex)));
}

int ScaleQuantizer::quantize (int rawNote) const
{
    if (rawNote < 0)   rawNote = 0;
    if (rawNote > 127) rawNote = 127;

    int pc = rawNote % 12;

    // Already in scale?
    if (inScale[pc])
        return rawNote;

    // Search outward for closest in-scale note
    for (int offset = 1; offset <= 6; ++offset)
    {
        int above = (pc + offset) % 12;
        int below = (pc - offset + 12) % 12;

        if (inScale[below])
            return rawNote - offset;
        if (inScale[above])
            return rawNote + offset;
    }

    return rawNote;   // fallback (shouldn't happen with chromatic)
}

int ScaleQuantizer::getScaleDegreeOffset (int fromNote, int degreeOffset) const
{
    auto notes = getNotesInRange (0, 127);
    if (notes.empty())
        return fromNote;

    // Find the closest index to fromNote
    int closestIdx = 0;
    int closestDist = std::abs (notes[0] - fromNote);

    for (int i = 1; i < (int) notes.size(); ++i)
    {
        int d = std::abs (notes[i] - fromNote);
        if (d < closestDist)
        {
            closestDist = d;
            closestIdx = i;
        }
    }

    int targetIdx = closestIdx + degreeOffset;
    targetIdx = juce::jlimit (0, (int) notes.size() - 1, targetIdx);

    return notes[targetIdx];
}

std::vector<int> ScaleQuantizer::getNotesInRange (int lowNote, int highNote) const
{
    std::vector<int> result;
    result.reserve (64);

    for (int n = juce::jmax (0, lowNote); n <= juce::jmin (127, highNote); ++n)
    {
        if (inScale[n % 12])
            result.push_back (n);
    }

    return result;
}

void ScaleQuantizer::rebuildNoteSet()
{
    for (int i = 0; i < 12; ++i)
        inScale[i] = false;

    auto& intervals = getScaleIntervals (currentScale);
    for (int interval : intervals)
        inScale[(rootNote + interval) % 12] = true;
}

const std::vector<int>& ScaleQuantizer::getScaleIntervals (Scale scale)
{
    // Static interval sets (semitones from root)
    static const std::vector<int> major      = { 0, 2, 4, 5, 7, 9, 11 };
    static const std::vector<int> minor      = { 0, 2, 3, 5, 7, 8, 10 };
    static const std::vector<int> dorian     = { 0, 2, 3, 5, 7, 9, 10 };
    static const std::vector<int> mixolydian = { 0, 2, 4, 5, 7, 9, 10 };
    static const std::vector<int> pentatonic = { 0, 2, 4, 7, 9 };
    static const std::vector<int> wholeTone  = { 0, 2, 4, 6, 8, 10 };
    static const std::vector<int> chromatic  = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 };

    switch (scale)
    {
        case Major:       return major;
        case Minor:       return minor;
        case Dorian:      return dorian;
        case Mixolydian:  return mixolydian;
        case Pentatonic:  return pentatonic;
        case WholeTone:   return wholeTone;
        case Chromatic:   return chromatic;
        default:          return major;
    }
}
