/*
 * The Pad class: a controller port wrapped as a BasicClass (include/Pad.h
 * documents the class and its events; main() creates the one instance).
 * Instances share the Psy-Q pad library: the first ctor calls PadInit and
 * the last finalize PadStop (sPadRefCount). On every DrawSystem vsync
 * event (IntermediateBase__OnDrawSystemEvent) updateMasks turns two consecutive
 * PadRead words into held, pressed and released edge masks, and
 * dispatchEvents sends at most one event per button to the pad's parents.
 * loadButtonTable copies sDefaultButtonMasks, a fixed table of libetc's
 * mask values, into sButtonMasks, whose order is enum PadButton. The two
 * remaining slots are empty and never called.
 */
#include "common.h"
#include <libetc.h>
#include "Pad.h"
#include "bmem_pmgr.h"

/* What only this file's bodies use; the class itself is include/Pad.h. The
 * pad library it wraps (PadInit, PadRead, PadStop) is Sony's <libetc.h>. */

extern s32 sPadRefCount; /* live instances: the first ctor calls PadInit, the last finalize PadStop */
extern u32 sButtonMasks[PAD_BUTTON_COUNT]; /* runtime copy of the button-mask table, filled by Pad__LoadButtonTable */

/* A 0x40-byte block, copied as a whole (GCC's inlined block-move codegen for
 * a struct assignment, not a word loop) rather than word-indexed. */
typedef struct {
    u32 w[16];
} Block64;

/* The unit's own read-only table of libetc's 16 button masks, in enum
 * PadButton order (PADLup, PADLdown, ... PADstart). */
extern Block64 sDefaultButtonMasks;

Pad *New_Pad(s32 mode, s32 port) {
    Pad *self;

    self = BMemPMgrAlloc(sizeof(Pad));
    if (self == NULL) {
        goto fail;
    }
    GetPadMethods()->ctor(self, mode, port);
    return self;
fail:
    return NULL;
}

void Pad__Pad(Pad *self, s32 mode, s32 port) {
    GetBasicClassMethods()->ctor((BasicClass *)self);
    self->methods = GetPadMethods();
    if (sPadRefCount++ == 0) {
        PadInit(mode);
    }
    self->methods->init(self, port);
}

void Pad__Finalize(Pad *self) {
    if (--sPadRefCount == 0) {
        PadStop();
    }
    GetBasicClassMethods()->finalize((BasicClass *)self);
}

void Pad__Init(Pad *self, s32 port) {
    self->port = (port != 0);
    self->heldMask = 0;
    self->releasedMask = 0;
    self->pressedMask = 0;
    self->methods->loadButtonTable();
}

u32 Pad__UpdateMasks(Pad *self) {
    u32 newMask;
    u32 oldMask;
    u32 changed;

    newMask = PadRead(self->port);
    oldMask = self->heldMask;
    self->heldMask = newMask;
    changed = newMask ^ oldMask;
    self->releasedMask = changed & oldMask;
    self->pressedMask = changed & newMask;
    return newMask;
}

void Pad__DispatchEvents(Pad *self) {
    s32 events[PAD_BUTTON_COUNT];
    void (*notifyParents)(Pad *self, s32 event);
    u32 held;
    u32 released;
    u32 pressed;
    s32 *p;
    s32 code;
    s32 i;

    /* MATCHING: without this barrier the prologue saves s0 before ra, s2, s1. */
    __asm__("");
    held = self->heldMask;
    released = self->releasedMask;
    pressed = self->pressedMask;
    p = events;
    if (held == 0 && released == 0 && pressed == 0) {
        return;
    }

    for (i = 0; i < PAD_BUTTON_COUNT; i++) {
        u32 mask = sButtonMasks[i];

        code = -1;
        if (released & mask) {
            code = PAD_EVENT_RELEASED;
        } else if (pressed & mask) {
            code = PAD_EVENT_PRESSED;
        } else if (held & mask) {
            code = PAD_EVENT_HELD;
        }
        if (code >= 0) {
            *p++ = code + i;
        }
    }

    notifyParents = self->methods->notifyParents;
    for (p--; p >= events; p--) {
        notifyParents(self, *p);
    }
}

void Pad__NoOpSlot4C(void) {}

void Pad__LoadButtonTable(void) {
    Block64 local;
    u32 *dst;
    u32 *src;
    s32 i;

    dst = sButtonMasks;
    local = sDefaultButtonMasks;
    i = 0;
    src = local.w;
    for (; i < PAD_BUTTON_COUNT; i++) {
        *dst++ = *src++;
    }
}

void Pad__NoOpSlot54(void) {}

PadMethods *GetPadMethods(void) {
    return &gPadMethods;
}
