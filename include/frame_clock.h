#ifndef FRAME_CLOCK_H
#define FRAME_CLOCK_H

#include "basic_class.h"

/**
 * @file frame_clock.h
 * @brief FrameClock, the per-frame clock that tells its parents each frame
 *        whether it is running, paused or stopped.
 *
 * Each tick (+0x044) sends notifyParents(self, event) with a
 * FrameClockEvent: RUNNING (frameCount += 1 first), PAUSED (not counted) or
 * STOPPED (not counted, takes precedence; only reset clears it). Its tick
 * comes from the DrawSystem's per-VSync event (include/draw_system.h):
 * IntermediateBase__Init (src/app/task.c) keeps one at +0x010
 * (initArgs->frameClock, or New_FrameClock()), adds it as a child of
 * itself, of the viewport and of the LightRig, and
 * IntermediateBase__OnDrawSystemEvent calls its tick on the DrawSystem's
 * event 2. DayTask__DayTask (src/world/dream_day.c) makes the one handed in
 * as initArgs->frameClock.
 *
 * Its listeners, all dispatching on the sender's class nibble 5:
 * IntermediateBase's update counts every event; Viewport__OnFrameClockEvent
 * redraws on RUNNING and PAUSED but not STOPPED; DreamSys__TimerTick
 * advances the dream timer on RUNNING only; TodActor__Update ticks on
 * RUNNING and releases itself on STOPPED. `paused` is named from ObjM
 * (src/world/objm.c): ObjM__AdvancePauseSetup calls +0x04C pause on
 * its +0x010 FrameClock in the same step as WBgm__Pause on its WBgm (the
 * same slot, +0x04C), and ObjM__TeardownPauseOverlay calls +0x050 resume
 * beside WBgm__Resume. No caller of stop (+0x058) is known: none reaches
 * the slot through a typed field.
 */

typedef struct FrameClock FrameClock;
typedef struct FrameClockMethods FrameClockMethods;

/** The events tick sends its parents (notifyParents(self, event)). */
enum FrameClockEvent {
    FRAMECLOCK_EVENT_RUNNING = 2, /**< counted: frameCount += 1 first */
    FRAMECLOCK_EVENT_PAUSED = 3,  /**< paused set: not counted */
    FRAMECLOCK_EVENT_STOPPED = 4  /**< stopped set: not counted, takes precedence */
};

/** FrameClock's class id (gFrameClockMethods word +0x000). A single nibble,
 * so `(header & CLASS_ID_ROOT_MASK) == FRAMECLOCK_CLASS_ID` is its is-kind-of test (the
 * listeners above; TextEntry's addChild/removeChild/onNotify). */
#define FRAMECLOCK_CLASS_ID 0x5

/** FrameClock's method table: BasicClass's slots, then seven of its own. */
struct FrameClockMethods {
    BASICCLASS_SLOTS(FrameClock, (FrameClock * self));            /* FrameClock__FrameClock */
    /* +0x040 */ void (*reset)(FrameClock *self, s32 frameCount); /**< @see FrameClock__Reset */
    /* +0x044 */ void (*tick)(FrameClock *self);                  /**< @see FrameClock__Tick */
    /* +0x048 */ s32 (*getFrameCount)(FrameClock *self); /**< @see FrameClock__GetFrameCount */
    /* +0x04C */ void (*pause)(FrameClock *self);        /**< @see FrameClock__Pause */
    /* +0x050 */ void (*resume)(FrameClock *self);       /**< @see FrameClock__Resume */
    /* +0x054 */ s32 (*isPaused)(FrameClock *self);      /**< @see FrameClock__IsPaused */
    /* +0x058 */ void (*stop)(FrameClock *self);         /**< @see FrameClock__Stop */
};

/**
 * FrameClock: a per-frame clock its parents listen to. Class id 0x5
 * (FRAMECLOCK_CLASS_ID), table gFrameClockMethods, a direct BasicClass
 * subclass (its ctor chains to BasicClass's first) that overrides the ctor,
 * finalize, removeParentRef and notifyParents and adds seven slots; methods
 * in src/graphics/sprite.c; no class derives from it. The object is 0x1C
 * bytes (New_FrameClock).
 *
 * removeParentRef and notifyParents are overridden only to keep
 * `parentCursor` valid: notifyParents walks parentRefs with it, and a parent
 * that drops the clock mid-walk (a TodActor releasing itself on STOPPED
 * removes its children, this clock among them) steps the cursor past its
 * own node first.
 */
struct FrameClock {
    BASICCLASS_FIELDS(FrameClockMethods);
    /* +0x00C */ s32 frameCount; /**< ticks while neither paused nor stopped; reset sets it */
    /* +0x010 */ s32 paused; /**< pause 1, resume 0, reset 0: tick sends PAUSED and does not count */
    /* +0x014 */ s32 stopped; /**< stop 1, reset 0: tick sends STOPPED and does not count */
    /* +0x018 */ BasicClassListNode *parentCursor; /**< notifyParents' walk over parentRefs; removeParentRef steps it past a removed parent */
};

/** FrameClock's method table (class id 0x5). */
extern FrameClockMethods gFrameClockMethods;

/**
 * @brief The FrameClock method table.
 * @return &gFrameClockMethods.
 */
extern FrameClockMethods *GetFrameClockMethods(void);

/**
 * @brief Allocates a FrameClock and runs its ctor through the table.
 * @return The new clock at frame 0, or NULL when the allocation fails.
 */
FrameClock *New_FrameClock(void);

/**
 * @brief Constructor (slot +0x008): BasicClass's ctor, installs
 *        gFrameClockMethods, then reset(0).
 * @param self The clock.
 */
void FrameClock__FrameClock(FrameClock *self);

/**
 * @brief Finalizer (slot +0x00C): BasicClass's finalize.
 * @param self The clock.
 */
void FrameClock__Finalize(FrameClock *self);

/**
 * @brief removeParentRef (slot +0x024): steps parentCursor past `parent` when
 *        it points at it, then BasicClass's removeParentRef.
 * @param self The clock.
 * @param parent The parent being removed.
 */
void FrameClock__RemoveParentRef(FrameClock *self, BasicClass *parent);

/**
 * @brief notifyParents (slot +0x030): sends `event` to every parent's
 *        onNotify, walking parentRefs with parentCursor so that a parent may
 *        drop the clock during the walk. Clears the cursor after.
 * @param self The clock.
 * @param event The event.
 */
void FrameClock__NotifyParents(FrameClock *self, s32 event);

/**
 * @brief reset (slot +0x040): sets the frame count and clears paused,
 *        stopped and the cursor.
 * @param self The clock.
 * @param frameCount The new frame count.
 */
void FrameClock__Reset(FrameClock *self, s32 frameCount);

/**
 * @brief tick (slot +0x044): notifies the parents with STOPPED if stopped is
 *        set, else PAUSED if paused is, else counts the frame and notifies
 *        RUNNING.
 * @param self The clock.
 */
void FrameClock__Tick(FrameClock *self);

/**
 * @brief getFrameCount (slot +0x048): the frames counted.
 * @param self The clock.
 * @return frameCount.
 */
s32 FrameClock__GetFrameCount(FrameClock *self);

/**
 * @brief pause (slot +0x04C): sets paused; ticks then send PAUSED.
 * @param self The clock.
 */
void FrameClock__Pause(FrameClock *self);

/**
 * @brief resume (slot +0x050): clears paused.
 * @param self The clock.
 */
void FrameClock__Resume(FrameClock *self);

/**
 * @brief isPaused (slot +0x054): whether pause is in effect.
 * @param self The clock.
 * @return paused: non-zero while paused.
 */
s32 FrameClock__IsPaused(FrameClock *self);

/**
 * @brief stop (slot +0x058): sets stopped; ticks then send STOPPED until reset.
 * @param self The clock.
 */
void FrameClock__Stop(FrameClock *self);

#endif
