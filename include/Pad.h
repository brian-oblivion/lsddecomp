#ifndef PAD_H
#define PAD_H

#include "BasicClass.h"

/*
 * Pad -- a controller port, class id 0x2, method table gPadMethods, a direct
 * BasicClass subclass (`tools/classtable.py gPadMethods --vs gBasicClassMethods`:
 * overrides ctor and finalize, adds six slots). Methods in src/class_16334.c.
 *
 * A game-side wrapper around the Psy-Q pad library: the first live instance
 * calls PadInit and the last calls PadStop (sPadRefCount); updateMasks turns
 * two consecutive PadRead words into held/pressed/released edge masks, and
 * dispatchEvents turns those into one event code per button (0x02 held, 0x12
 * pressed, 0x22 released, plus the button's index in sButtonMasks) and sends
 * each, highest button first, through the INHERITED notifyParents slot: a
 * Pad's parents receive its button events in their onNotify(parent, pad,
 * event).
 */

typedef struct Pad Pad;
typedef struct PadMethods PadMethods;

struct PadMethods {
    BASICCLASS_SLOTS(Pad, (Pad * self, void *arg1, s32 port));
    /* +0x040 */ void (*init)(Pad *self, s32 port); /* Pad__Init */
    /* +0x044 */ u32 (*updateMasks)(Pad *self); /* Pad__UpdateMasks: returns the new held mask */
    /* +0x048 */ void (*dispatchEvents)(Pad *self); /* Pad__DispatchEvents */
    /* +0x04C */ void (*slot4C)(void);              /* Pad__func_80025E14, empty, never called */
    /* +0x050 */ void (*loadButtonTable)(void);     /* Pad__LoadButtonTable */
    /* +0x054 */ void (*slot54)(void);              /* Pad__func_80025E94, empty, never called */
};

struct Pad {
    BASICCLASS_FIELDS(PadMethods);
    /* +0x00C */ u16 port; /* 0 or 1: the ctor boolifies its argument */
    /* +0x00E */ u8 pad0E[2];
    /* +0x010 */ u32 heldMask;
    /* +0x014 */ u32 releasedMask;
    /* +0x018 */ u32 pressedMask;
    /* +0x01C */ u8 pad1C[4]; /* the object is 0x20 bytes (New_Pad) */
};

extern PadMethods gPadMethods;
extern PadMethods *Get_vtable_Pad(void);

Pad *New_Pad(void *arg1, s32 port);
void Pad__Pad(Pad *self, void *arg1, s32 port);
void Pad__Finalize(Pad *self);
void Pad__Init(Pad *self, s32 port);
u32 Pad__UpdateMasks(Pad *self);
void Pad__DispatchEvents(Pad *self);
void Pad__LoadButtonTable(void);

#endif
