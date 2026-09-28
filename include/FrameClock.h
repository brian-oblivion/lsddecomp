#ifndef FRAMECLOCK_H
#define FRAMECLOCK_H

#include "BasicClass.h"

/*
 * FrameClock -- class id 0x5, method table gFrameClockMethods (22 slots), a
 * direct BasicClass subclass (its ctor calls GetBasicClassMethods()->ctor
 * first; `classtable.py gFrameClockMethods --vs gBasicClassMethods` overrides the
 * ctor, finalize, removeParentRef and notifyParents and adds seven slots).
 * Methods in src/graphics/Sprite.c. No class derives from it.
 *
 * A per-frame clock that its parents listen to. Each tick (+0x044) sends
 * notifyParents(self, event) with event 2 (running: frameCount += 1 first),
 * 3 (paused: not counted) or 4 (stopped: not counted, takes precedence; only reset clears it).
 * Its tick comes from the DrawSystem's per-VSync event 2
 * (include/DrawSystem.h): IntermediateBase__Init (src/app/Task.c) keeps
 * one at +0x010 (initArgs->unk8, or New_FrameClock()), adds it as a child of
 * itself, of the viewport and of the LightRig, and IntermediateBase__OnDrawSystemEvent
 * calls its tick on the DrawSystem's event 2. DayTask__DayTask
 * (src/world/DayTaskStageMap.c) makes the one handed in as initArgs->unk8.
 *
 * Its listeners, all dispatching on the sender's class nibble 5:
 * IntermediateBase's update counts every event; Viewport__OnNotifyTag5
 * redraws on 2 and 3 but not 4; DreamSys__TimerTick advances the dream timer
 * on 2 only; TodActor__Update ticks on 2 and releases itself on 4.
 * `paused` is named from ObjM (src/world/ObjMStyleActor.c): ObjM__AdvancePauseSetup
 * calls +0x04C pause on its +0x010 FrameClock in the same step as WBgm__Pause
 * on its WBgm (the same slot, +0x04C), and ObjM__TeardownPauseOverlay calls
 * +0x050 resume beside WBgm__Resume. What calls stop (+0x058) is not found
 * in C: no caller reaches the slot through a field this project has typed.
 *
 * removeParentRef and notifyParents are overridden only to keep
 * `parentCursor` valid: notifyParents walks parentRefs with it, and a parent
 * that drops the clock mid-walk (a TodActor releasing itself on event 4
 * removes its children, this clock among them) steps the cursor past its own
 * node first.
 */

typedef struct FrameClock FrameClock;
typedef struct FrameClockMethods FrameClockMethods;

/* The events tick sends its parents (notifyParents(self, event)). */
enum FrameClockEvent {
    FRAMECLOCK_EVENT_RUNNING = 2, /* counted: frameCount += 1 first */
    FRAMECLOCK_EVENT_PAUSED = 3,  /* paused set: not counted */
    FRAMECLOCK_EVENT_STOPPED = 4  /* stopped set: not counted, takes precedence */
};

/* FrameClock's class id (gFrameClockMethods word +0x000). A single nibble,
 * so `(header & 0xF) == FRAMECLOCK_CLASS_ID` is its is-kind-of test (the
 * listeners above; TextEntry's addChild/removeChild/onNotify). */
#define FRAMECLOCK_CLASS_ID 0x5

/* BasicClass's slots, then this class's own, named for their occupants. */
struct FrameClockMethods {
    BASICCLASS_SLOTS(FrameClock, (FrameClock * self)); /* FrameClock__FrameClock */
    /* +0x040 */ void (*reset)(FrameClock *self, s32 frameCount); /* FrameClock__Reset: frameCount = arg, flags and cursor cleared; the ctor passes 0 */
    /* +0x044 */ void (*tick)(FrameClock *self); /* FrameClock__Tick: IntermediateBase__OnDrawSystemEvent on the DrawSystem's event 2 */
    /* +0x048 */ s32 (*getFrameCount)(FrameClock *self); /* FrameClock__GetFrameCount */
    /* +0x04C */ void (*pause)(FrameClock *self); /* FrameClock__Pause: ObjM__AdvancePauseSetup */
    /* +0x050 */ void (*resume)(FrameClock *self); /* FrameClock__Resume: ObjM__TeardownPauseOverlay */
    /* +0x054 */ s32 (*isPaused)(FrameClock *self); /* FrameClock__IsPaused */
    /* +0x058 */ void (*stop)(FrameClock *self); /* FrameClock__Stop: ticks then send event 4 until reset */
};

struct FrameClock {
    BASICCLASS_FIELDS(FrameClockMethods);
    /* +0x00C */ s32 frameCount; /* ticks while neither paused nor stopped; reset sets it */
    /* +0x010 */ s32 paused;     /* pause 1, resume 0, reset 0: tick sends 3 and does not count */
    /* +0x014 */ s32 stopped;    /* stop 1, reset 0: tick sends 4 and does not count */
    /* +0x018 */ BasicClassListNode *parentCursor; /* notifyParents' walk over parentRefs; removeParentRef steps it past a removed parent */
}; /* 0x1C bytes: New_FrameClock */

extern FrameClockMethods gFrameClockMethods;
extern FrameClockMethods *GetFrameClockMethods(void); /* returns &gFrameClockMethods */

/* The class's own methods, in address order. */
FrameClock *New_FrameClock(void); /* BMemPMgrAlloc(0x1C), then ctor */
void FrameClock__FrameClock(FrameClock *self);
void FrameClock__Finalize(FrameClock *self);
void FrameClock__RemoveParentRef(FrameClock *self, BasicClass *parent);
void FrameClock__NotifyParents(FrameClock *self, s32 event);
void FrameClock__Reset(FrameClock *self, s32 frameCount);
void FrameClock__Tick(FrameClock *self);
s32 FrameClock__GetFrameCount(FrameClock *self);
void FrameClock__Pause(FrameClock *self);
void FrameClock__Resume(FrameClock *self);
s32 FrameClock__IsPaused(FrameClock *self);
void FrameClock__Stop(FrameClock *self);

#endif
