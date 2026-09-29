/*
 * The Pad class: a controller port wrapped as a BasicClass (include/pad.h
 * documents the class and its events; main() creates the one instance).
 * Instances share the Psy-Q pad library: the first ctor calls PadInit and
 * the last finalize PadStop (sPadRefCount). On every DrawSystem vsync
 * event (IntermediateBase__OnDrawSystemEvent) updateMasks turns two consecutive
 * PadRead words into held, pressed and released edge masks, and
 * dispatchEvents sends at most one event per button to the pad's parents.
 * loadButtonTable copies sDefaultButtonMasks, a fixed table of libetc's
 * mask values, into sButtonMasks, whose order is enum PadButton. The two
 * remaining slots are empty and never called. The method table ends the
 * file.
 */
#include "common.h"
#include <libetc.h>
#include "pad.h"
#include "bmem_pmgr.h"

/* What only this file's bodies use; the class itself is include/pad.h. The
 * pad library it wraps (PadInit, PadRead, PadStop) is Sony's <libetc.h>. */

extern s32 sPadRefCount; /* live instances: the first ctor calls PadInit, the last finalize PadStop */
extern u32 sButtonMasks[PAD_BUTTON_COUNT]; /* runtime copy of the button-mask table, filled by Pad__LoadButtonTable */

/** @brief A 0x40-byte block: Pad__LoadButtonTable copies the default table
 * as one. */
/* MATCHING: copied as one struct assignment, not a word-by-word loop, as retail copies it. */
typedef struct {
    u32 w[16]; /**< the 16 button masks */
} Block64;

/* The unit's own read-only table of libetc's 16 button masks, in enum
 * PadButton order. */
const Block64 sDefaultButtonMasks = {{
    PADLup,    /* PAD_BUTTON_LUP */
    PADLdown,  /* PAD_BUTTON_LDOWN */
    PADLleft,  /* PAD_BUTTON_LLEFT */
    PADLright, /* PAD_BUTTON_LRIGHT */
    PADRup,    /* PAD_BUTTON_RUP */
    PADRdown,  /* PAD_BUTTON_RDOWN */
    PADRleft,  /* PAD_BUTTON_RLEFT */
    PADRright, /* PAD_BUTTON_RRIGHT */
    PADi,      /* PAD_BUTTON_I */
    PADj,      /* PAD_BUTTON_J */
    PADselect, /* PAD_BUTTON_SELECT */
    PADR1,     /* PAD_BUTTON_R1 */
    PADR2,     /* PAD_BUTTON_R2 */
    PADL1,     /* PAD_BUTTON_L1 */
    PADL2,     /* PAD_BUTTON_L2 */
    PADstart,  /* PAD_BUTTON_START */
}};

Pad *New_Pad(s32 mode, s32 port) {
    Pad *self;

    self = BMemPMgrAlloc(sizeof(Pad));
    if (self != NULL) {
        GetPadMethods()->ctor(self, mode, port);
        return self;
    }
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

    /* MATCHING: an ordering barrier; without it the entry code comes out in another order. */
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
    /* MATCHING: i is cleared ahead of src; cleared in the for header, the two setups swap. */
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

/* Pad's method table (include/pad.h): BasicClass's slots with the ctor and
 * finalize, then init, updateMasks, dispatchEvents, loadButtonTable and the
 * two empty slots. */
PadMethods gPadMethods = {
    /* +0x000 header */ PAD_CLASS_ID,
    /* +0x004 release */ (void *)BasicClass__Release,
    /* +0x008 ctor */ Pad__Pad,
    /* +0x00C finalize */ Pad__Finalize,
    /* +0x010 addChild */ (void *)BasicClass__AddChild,
    /* +0x014 removeChild */ (void *)BasicClass__RemoveChild,
    /* +0x018 removeAllChildren */ (void *)BasicClass__RemoveAllChildren,
    /* +0x01C getNextChild */ (void *)BasicClass__GetNextChild,
    /* +0x020 addParentRef */ (void *)BasicClass__AddParentRef,
    /* +0x024 removeParentRef */ (void *)BasicClass__RemoveParentRef,
    /* +0x028 clearParentRefs */ (void *)BasicClass__ClearParentRefs,
    /* +0x02C getNextParentRef */ (void *)BasicClass__GetNextParentRef,
    /* +0x030 notifyParents */ (void *)BasicClass__NotifyParents,
    /* +0x034 slot34 */ BasicClass__NoOpSlot34,
    /* +0x038 onNotify */ (void *)BasicClass__OnNotify,
    /* +0x03C slot3C */ NULL,
    /* +0x040 init */ Pad__Init,
    /* +0x044 updateMasks */ Pad__UpdateMasks,
    /* +0x048 dispatchEvents */ Pad__DispatchEvents,
    /* +0x04C slot4C */ Pad__NoOpSlot4C,
    /* +0x050 loadButtonTable */ Pad__LoadButtonTable,
    /* +0x054 slot54 */ Pad__NoOpSlot54,
};
