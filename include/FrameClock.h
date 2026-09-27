#ifndef FRAMECLOCK_H
#define FRAMECLOCK_H

#include "BasicClass.h"

/*
 * FrameClock -- class id 0x5, method table gFrameClockMethods (22 slots), a
 * direct BasicClass subclass (its ctor calls Get_vtable_BasicClass()->ctor
 * first; `classtable.py gFrameClockMethods --vs gBasicClassMethods` overrides the
 * ctor, finalize, removeParentRef and notifyParents and adds seven slots).
 * Methods in src/code_322b4.c. No class derives from it.
 *
 * A per-frame clock that its parents listen to. Each tick (+0x044) sends
 * notifyParents(self, event) with event 2 (running: frameCount += 1 first),
 * 3 (paused: not counted) or 4 (flag14 set: not counted, takes precedence).
 * Its tick comes from the DrawSystem's per-VSync event 2
 * (include/DrawSystem.h): IntermediateBase__Init (src/code_2cc8c_c.c) keeps
 * one at +0x010 (initArgs->unk8, or New_FrameClock()), adds it as a child of
 * itself, of the viewport and of the LightRig, and IntermediateBase__OnTag1Notify
 * calls its tick on the DrawSystem's event 2. DayTask__DayTask
 * (src/class_39e08.c) makes the one handed in as initArgs->unk8.
 *
 * Its listeners, all dispatching on the sender's class nibble 5:
 * IntermediateBase's update counts every event; Viewport__OnNotifyTag5
 * redraws on 2 and 3 but not 4; DreamSys__TimerTick advances the dream timer
 * on 2 only; TodActor__Update ticks on 2 and releases itself on 4.
 * `paused` is named from ObjM (src/class_3bb8c_m.c): ObjM__AdvancePauseSetup
 * calls +0x04C pause on its +0x010 FrameClock in the same step as WBgm__Pause
 * on its WBgm (the same slot, +0x04C), and ObjM__TeardownPauseOverlay calls
 * +0x050 resume beside WBgm__Resume. What sets flag14 (+0x058) is not found
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

/* FrameClock's class id (gFrameClockMethods word +0x000). A single nibble, so
 * `(header & CLASS_ID_ROOT_MASK) == FRAMECLOCK_CLASS_ID` tests for it or a subclass. */
#define FRAMECLOCK_CLASS_ID 0x5

/* The events tick sends its parents (notifyParents(self, event)). */
enum FrameClockEvent {
    FRAMECLOCK_EVENT_RUNNING = 2, /* counted: frameCount += 1 first */
    FRAMECLOCK_EVENT_PAUSED = 3,  /* paused set: not counted */
    FRAMECLOCK_EVENT_FLAG14 = 4   /* flag14 set: not counted, takes precedence */
};

/* BasicClass's slots, then this class's own, named for their occupants. */
struct FrameClockMethods {
    BASICCLASS_SLOTS(FrameClock, (FrameClock * self)); /* FrameClock__FrameClock */
    /* +0x040 */ void (*reset)(FrameClock *self, s32 frameCount); /* FrameClock__Reset: frameCount = arg, flags and cursor cleared; the ctor passes 0 */
    /* +0x044 */ void (*tick)(FrameClock *self); /* FrameClock__Tick: IntermediateBase__OnTag1Notify on the DrawSystem's event 2 */
    /* +0x048 */ s32 (*getFrameCount)(FrameClock *self); /* FrameClock__GetFrameCount */
    /* +0x04C */ void (*pause)(FrameClock *self); /* FrameClock__Pause: ObjM__AdvancePauseSetup */
    /* +0x050 */ void (*resume)(FrameClock *self); /* FrameClock__Resume: ObjM__TeardownPauseOverlay */
    /* +0x054 */ s32 (*isPaused)(FrameClock *self); /* FrameClock__IsPaused */
    /* +0x058 */ void (*setFlag14)(FrameClock *self); /* FrameClock__SetFlag14: ticks then send event 4 */
};

struct FrameClock {
    BASICCLASS_FIELDS(FrameClockMethods);
    /* +0x00C */ s32 frameCount; /* ticks while neither paused nor flag14; reset sets it */
    /* +0x010 */ s32 paused;     /* pause 1, resume 0, reset 0: tick sends 3 and does not count */
    /* +0x014 */ s32 flag14;     /* setFlag14 1, reset 0: tick sends 4 and does not count */
    /* +0x018 */ BasicClassListNode *parentCursor; /* notifyParents' walk over parentRefs; removeParentRef steps it past a removed parent */
}; /* 0x1C bytes: New_FrameClock */

extern FrameClockMethods gFrameClockMethods;
extern FrameClockMethods *Get_vtable_FrameClock(void); /* returns &gFrameClockMethods */

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
void FrameClock__SetFlag14(FrameClock *self);

#endif
