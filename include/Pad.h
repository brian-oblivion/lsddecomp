#ifndef PAD_H
#define PAD_H

#include "basic_class.h"

/*
 * Pad -- a controller port, class id 0x2, method table gPadMethods, a direct
 * BasicClass subclass (`tools/classtable.py gPadMethods --vs gBasicClassMethods`:
 * overrides ctor and finalize, adds six slots). Methods in src/app/Pad.c.
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

/* dispatchEvents' event codes: one base per edge plus the button's index
 * in sButtonMasks (PAD_EVENT_PRESSED + PAD_BUTTON_START is Start going
 * down). */
#define PAD_EVENT_HELD 0x02
#define PAD_EVENT_PRESSED 0x12
#define PAD_EVENT_RELEASED 0x22

/* Pad's class id (gPadMethods word +0x000). A single nibble, so
 * `(header & 0xF) == PAD_CLASS_ID` is its is-kind-of test (TextEntry's
 * addChild/removeChild/onNotify). */
#define PAD_CLASS_ID 0x2

/* sButtonMasks' indices. Pad__LoadButtonTable fills it from one fixed
 * table (sDefaultButtonMasks), whose words are libetc's masks in this order. */
enum PadButton {
    PAD_BUTTON_LUP = 0,     /* PADLup */
    PAD_BUTTON_LDOWN = 1,   /* PADLdown */
    PAD_BUTTON_LLEFT = 2,   /* PADLleft */
    PAD_BUTTON_LRIGHT = 3,  /* PADLright */
    PAD_BUTTON_RUP = 4,     /* PADRup (triangle) */
    PAD_BUTTON_RDOWN = 5,   /* PADRdown (cross) */
    PAD_BUTTON_RLEFT = 6,   /* PADRleft (square) */
    PAD_BUTTON_RRIGHT = 7,  /* PADRright (circle) */
    PAD_BUTTON_I = 8,       /* PADi */
    PAD_BUTTON_J = 9,       /* PADj */
    PAD_BUTTON_SELECT = 10, /* PADk, PADselect */
    PAD_BUTTON_R1 = 11,     /* PADl, PADR1 */
    PAD_BUTTON_R2 = 12,     /* PADm, PADR2 */
    PAD_BUTTON_L1 = 13,     /* PADn, PADL1 */
    PAD_BUTTON_L2 = 14,     /* PADo, PADL2 */
    PAD_BUTTON_START = 15,  /* PADh, PADstart */
    PAD_BUTTON_COUNT = 16   /* sButtonMasks' length, one event per button per frame at most */
};

typedef struct Pad Pad;
typedef struct PadMethods PadMethods;

struct PadMethods {
    BASICCLASS_SLOTS(Pad, (Pad * self, s32 mode, s32 port));
    /* +0x040 */ void (*init)(Pad *self, s32 port); /* Pad__Init */
    /* +0x044 */ u32 (*updateMasks)(Pad *self); /* Pad__UpdateMasks: returns the new held mask */
    /* +0x048 */ void (*dispatchEvents)(Pad *self); /* Pad__DispatchEvents */
    /* +0x04C */ void (*slot4C)(void);              /* Pad__NoOpSlot4C, empty, never called */
    /* +0x050 */ void (*loadButtonTable)(void);     /* Pad__LoadButtonTable */
    /* +0x054 */ void (*slot54)(void);              /* Pad__NoOpSlot54, empty, never called */
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
extern PadMethods *GetPadMethods(void);

Pad *New_Pad(s32 mode, s32 port);
void Pad__Pad(Pad *self, s32 mode, s32 port);
void Pad__Finalize(Pad *self);
void Pad__Init(Pad *self, s32 port);
u32 Pad__UpdateMasks(Pad *self);
void Pad__DispatchEvents(Pad *self);
void Pad__LoadButtonTable(void);

#endif
