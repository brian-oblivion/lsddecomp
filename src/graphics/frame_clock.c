/*
 * FrameClock's methods (include/frame_clock.h: the per-frame clock that
 * counts frames and tells its parents when it stops), in ROM order: the
 * allocator, ctor, finalize, removeParentRef and notifyParents, then reset,
 * tick, getFrameCount, pause, resume, isPaused and stop, ending with its
 * getter GetFrameClockMethods; its method table closes the file. A (void *)
 * entry in it is a method whose declared parameters differ from the slot's,
 * one inherited from BasicClass and declared on its type.
 */
#include "common.h"
#include <libgte.h>
#include "frame_clock.h"
#include "bmem_pmgr.h"

/* Allocate and construct a FrameClock at frame 0. */
FrameClock *New_FrameClock(void) {
    FrameClock *obj = BMemPMgrAlloc(sizeof(FrameClock));

    if (obj != NULL) {
        GetFrameClockMethods()->ctor(obj);
        return obj;
    }
    return NULL;
}

/* gFrameClockMethods slot +0x008 (ctor): the BasicClass ctor, install the table, reset(0). */
void FrameClock__FrameClock(FrameClock *self) {
    GetBasicClassMethods()->ctor((BasicClass *)self);
    self->methods = GetFrameClockMethods();
    self->methods->reset(self, 0);
}

/* gFrameClockMethods slot +0x00C (finalize): the BasicClass finalize. */
void FrameClock__Finalize(FrameClock *self) {
    GetBasicClassMethods()->finalize((BasicClass *)self);
}

/* gFrameClockMethods slot +0x024 (removeParentRef): step the cursor past the parent
 * being removed, then the BasicClass removeParentRef. */
void FrameClock__RemoveParentRef(FrameClock *self, BasicClass *parent) {
    if (self->parentCursor != NULL && parent == self->parentCursor->value) {
        self->parentCursor = self->parentCursor->next;
    }
    GetBasicClassMethods()->removeParentRef((BasicClass *)self, parent);
}

/* gFrameClockMethods slot +0x030 (notifyParents): walk the parent refs with
 * parentCursor (which removeParentRef keeps valid) and pass each the
 * event through its onNotify. */
void FrameClock__NotifyParents(FrameClock *self, s32 event) {
    BasicClass *parent;

    self->parentCursor = self->parentRefs;
    for (GetNextBasicClass(&parent, &self->parentCursor); parent != NULL;
         GetNextBasicClass(&parent, &self->parentCursor)) {
        parent->methods->onNotify(parent, self, event);
    }
    self->parentCursor = NULL;
}

/* gFrameClockMethods slot +0x040 (reset): set the frame count, clear both
 * flags and the cursor. */
void FrameClock__Reset(FrameClock *self, s32 frameCount) {
    self->frameCount = frameCount;
    self->stopped = 0;
    self->paused = 0;
    self->parentCursor = 0;
}

/* gFrameClockMethods slot +0x044 (tick): notify STOPPED if stopped is set, else
 * PAUSED if paused, else count the frame and notify RUNNING. */
void FrameClock__Tick(FrameClock *self) {
    s32 event;

    if (self->stopped != 0) {
        event = FRAMECLOCK_EVENT_STOPPED;
    } else if (self->paused != 0) {
        event = FRAMECLOCK_EVENT_PAUSED;
    } else {
        self->frameCount++;
        event = FRAMECLOCK_EVENT_RUNNING;
    }
    self->methods->notifyParents(self, event);
}

/* gFrameClockMethods slot +0x048 (getFrameCount): the frames counted so far. */
s32 FrameClock__GetFrameCount(FrameClock *self) {
    return self->frameCount;
}

/* gFrameClockMethods slot +0x04C (pause): tick stops counting and notifies PAUSED. */
void FrameClock__Pause(FrameClock *self) {
    self->paused = 1;
}

/* gFrameClockMethods slot +0x050 (resume): tick counts again, unless stopped. */
void FrameClock__Resume(FrameClock *self) {
    self->paused = 0;
}

/* gFrameClockMethods slot +0x054 (isPaused). */
s32 FrameClock__IsPaused(FrameClock *self) {
    return self->paused;
}

/* gFrameClockMethods slot +0x058 (stop): tick notifies STOPPED until the next reset. */
void FrameClock__Stop(FrameClock *self) {
    self->stopped = 1;
}

/* Returns the gFrameClockMethods method table. */
FrameClockMethods *GetFrameClockMethods(void) {
    return &gFrameClockMethods;
}

/* FrameClock (include/frame_clock.h): BasicClass's slots with its ctor,
 * finalize, removeParentRef and notifyParents, then its seven clock slots. */
FrameClockMethods gFrameClockMethods = {
    /* +0x000 header */ FRAMECLOCK_CLASS_ID,
    /* +0x004 release */ (void *)BasicClass__Release,
    /* +0x008 ctor */ FrameClock__FrameClock,
    /* +0x00C finalize */ FrameClock__Finalize,
    /* +0x010 addChild */ (void *)BasicClass__AddChild,
    /* +0x014 removeChild */ (void *)BasicClass__RemoveChild,
    /* +0x018 removeAllChildren */ (void *)BasicClass__RemoveAllChildren,
    /* +0x01C getNextChild */ (void *)BasicClass__GetNextChild,
    /* +0x020 addParentRef */ (void *)BasicClass__AddParentRef,
    /* +0x024 removeParentRef */ FrameClock__RemoveParentRef,
    /* +0x028 clearParentRefs */ (void *)BasicClass__ClearParentRefs,
    /* +0x02C getNextParentRef */ (void *)BasicClass__GetNextParentRef,
    /* +0x030 notifyParents */ FrameClock__NotifyParents,
    /* +0x034 slot34 */ BasicClass__NoOpSlot34,
    /* +0x038 onNotify */ (void *)BasicClass__OnNotify,
    /* +0x03C slot3C */ NULL,
    /* +0x040 reset */ FrameClock__Reset,
    /* +0x044 tick */ FrameClock__Tick,
    /* +0x048 getFrameCount */ FrameClock__GetFrameCount,
    /* +0x04C pause */ FrameClock__Pause,
    /* +0x050 resume */ FrameClock__Resume,
    /* +0x054 isPaused */ FrameClock__IsPaused,
    /* +0x058 stop */ FrameClock__Stop,
};
