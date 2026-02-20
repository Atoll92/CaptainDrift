#pragma once
#include <juce_core/juce_core.h>
#include <vector>

class ScaleQuantizer
{
public:
    enum Scale
    {
        Major = 0,
        Minor,
        Dorian,
        Mixolydian,
        Pentatonic,
        WholeTone,
        Chromatic,
        NumScales
    };

    ScaleQuantizer();

    void setRootNote (int root);   // 0=C .. 11=B
    void setScale (Scale scale);
    void setScale (int scaleIndex);

    /** Quantize a raw MIDI note to the nearest note in the current scale. */
    int quantize (int rawNote) const;

    /** Get the note N scale degrees above/below a given note within the scale.
        Returns the MIDI note number. */
    int getScaleDegreeOffset (int fromNote, int degreeOffset) const;

    /** Get all scale notes within a MIDI note range. */
    std::vector<int> getNotesInRange (int lowNote, int highNote) const;

    int getRootNote() const { return rootNote; }
    Scale getScale() const { return currentScale; }

private:
    void rebuildNoteSet();

    int rootNote = 0;
    Scale currentScale = Major;

    // 12-element boolean array: which pitch classes are in-scale
    bool inScale[12] = {};

    // Scale interval definitions (semitones from root)
    static const std::vector<int>& getScaleIntervals (Scale scale);
};
