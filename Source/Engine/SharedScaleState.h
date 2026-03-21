#pragma once
#include <atomic>

/**
 * SharedScaleState — In-process singleton for scale linking between
 * multiple Kikinator instances running in the same DAW.
 *
 * A master instance writes its root note and scale index into a group slot.
 * Follower instances read from that slot and override their local scale.
 *
 * All Kikinator instances share the same static data because they're
 * loaded as plugins in the same process address space.
 *
 * Lock-free: uses std::atomic for thread safety across audio threads.
 */
class SharedScaleState
{
public:
    static constexpr int kMaxGroups = 4;   // Groups 1–4 (0 = unlinked)

    /** Returns the singleton instance. */
    static SharedScaleState& getInstance()
    {
        static SharedScaleState instance;
        return instance;
    }

    /** Master writes its current scale info to a group slot.
        @param group  1-based group index (1–4)
        @param root   Root note 0–11
        @param scale  Scale index (matches ScaleQuantizer::Scale enum) */
    void publish (int group, int root, int scale)
    {
        if (group < 1 || group > kMaxGroups)
            return;

        auto& slot = slots[group - 1];
        slot.rootNote.store (root, std::memory_order_relaxed);
        slot.scaleIndex.store (scale, std::memory_order_relaxed);
        slot.active.store (true, std::memory_order_release);
    }

    /** Follower reads the master's scale info from a group slot.
        @param group  1-based group index (1–4)
        @param root   [out] root note
        @param scale  [out] scale index
        @return true if a master has published to this group */
    bool subscribe (int group, int& root, int& scale) const
    {
        if (group < 1 || group > kMaxGroups)
            return false;

        auto& slot = slots[group - 1];
        if (! slot.active.load (std::memory_order_acquire))
            return false;

        root  = slot.rootNote.load (std::memory_order_relaxed);
        scale = slot.scaleIndex.load (std::memory_order_relaxed);
        return true;
    }

    /** Check whether a group has an active master. */
    bool isGroupActive (int group) const
    {
        if (group < 1 || group > kMaxGroups)
            return false;
        return slots[group - 1].active.load (std::memory_order_acquire);
    }

    /** Called when a master instance is destroyed or unlinks.
        Marks the group as inactive so followers revert to local scale. */
    void unpublish (int group)
    {
        if (group < 1 || group > kMaxGroups)
            return;
        slots[group - 1].active.store (false, std::memory_order_release);
    }

private:
    SharedScaleState() = default;

    struct GroupSlot
    {
        std::atomic<int>  rootNote   { 0 };
        std::atomic<int>  scaleIndex { 0 };
        std::atomic<bool> active     { false };
    };

    GroupSlot slots[kMaxGroups];
};
