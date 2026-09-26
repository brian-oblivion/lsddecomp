#ifndef INTERMEDIATEBASE_H
#define INTERMEDIATEBASE_H

#include "BasicClass.h"

/*
 * IntermediateBase -- class id 0x30, method table gIntermediateBaseMethods: a
 * BasicClass subclass that runs one attached job to a result. Methods in
 * src/code_2cc8c_c.c. Two classes derive from it (`typeviews.py --tree`):
 * TaskCore (0x130, gTaskCoreMethods: StreamTaskObj, Class86B60, GraphRoom
 * below it) and Class86668 (0x230: D_800865C8's Obj865C8 and D_80087034's ObjM
 * below it). It is abstract: +0x04C, +0x050 and +0x058 are NULL in its own
 * table, and init/deinit call the first two. It has no allocator; the object
 * is 0x28 bytes because both subclasses' own fields start at +0x028
 * (Class86668's result, the +0x028 TaskCore__Reset sets).
 *
 * init(args, mode) keeps `args`, adds args->unk0, args->unk4 and the +0x010
 * helper as children, runs onInit and records `mode`. With mode 0 it also
 * attaches args->unk0 and the helper to the viewport and the helper to the
 * +0x014 object, then calls setState(2) and deinit, which undoes all of it
 * and releases whatever init created itself. Callers construct an object,
 * call init and use what it RETURNS (Class6D3C8__RunPollTask,
 * Class6D3C8__PollStatusObj); the subclass overrides return a result field
 * (Class86668__Init: +0x028 result; TaskCore__Init: +0x038 result).
 *
 * onNotify splits by the SENDER's root class nibble, as Class6B5CC's does:
 * a D_8006C070 (1) goes to onTag1Notify, a Pad (2) to onPadEvent, a
 * D_8006EF50 (5) to update, which here only counts frames. setState stores
 * the state, passes it to notifyParents and runs onState2 or onState3 for 2
 * or 3. Both reset the frame counter and call initArgs->unk0.
 */

typedef struct IntermediateBase IntermediateBase;
typedef struct IntermediateBaseMethods IntermediateBaseMethods;
typedef struct IntermediateBaseInitArgs IntermediateBaseInitArgs;

/* init's argument. The caller owns it; the object keeps the pointer at
 * +0x00C. Obj865C8__Obj865C8 fills +0x008..+0x010 itself (New_D8006EF50(),
 * New_Class866E8(0, 1), New_Class869D8()). */
struct IntermediateBaseInitArgs {
    /* +0x000 */ BasicClass *unk0;     /* added as a child; onState2 calls its +0x048, onState3 its +0x04C */
    /* +0x004 */ BasicClass *unk4;     /* added as a child; onTag1Notify's event 2 calls its +0x044, +0x048 */
    /* +0x008 */ BasicClass *unk8;     /* becomes unk10; NULL: init makes one with New_D8006EF50() */
    /* +0x00C */ BasicClass *unkC;     /* becomes unk14; NULL: init makes one with New_LightRig() */
    /* +0x010 */ BasicClass *viewport; /* becomes viewport; NULL: init makes one with New_Viewport() */
};

#define INTERMEDIATEBASE_SLOTS(Self, CtorParams)                                                   \
    BASICCLASS_SLOTS(Self, CtorParams);                                                            \
    /* +0x040 */ void (*resetCounters)(Self *self);          /* IntermediateBase__ResetCounters; the ctor's last call */ \
    /* +0x044 */ s32 (*init)(Self *self, IntermediateBaseInitArgs *args, s32 mode); /* IntermediateBase__Init; s32: RunPollTask/PollStatusObj use the result */ \
    /* +0x048 */ void (*deinit)(Self *self);                 /* IntermediateBase__Deinit */      \
    /* +0x04C */ void (*onInit)(Self *self, s32 arg1, s32 arg2, s32 arg3); /* NULL; init calls it (0, 0, 0) after adding the children */ \
    /* +0x050 */ void (*onDeinit)(Self *self);               /* NULL; deinit's first call */     \
    /* +0x054 */ void (*onTag1Notify)(Self *self, BasicClass *sender, s32 event); /* IntermediateBase__OnTag1Notify: onNotify's D_8006C070 (1) case */ \
    /* +0x058 */ void (*onPadEvent)(Self *self, BasicClass *sender, s32 event);   /* NULL; onNotify's Pad (2) case */ \
    /* +0x05C */ void (*update)(Self *self, BasicClass *sender, s32 event);       /* IntermediateBase__IncrementFrameCounter: onNotify's D_8006EF50 (5) case */ \
    /* +0x060 */ void (*setState)(Self *self, s32 state);    /* IntermediateBase__SetState; TaskCore__SetState, Class86B60__SetState */ \
    /* +0x064 */ void (*onState2)(Self *self);               /* IntermediateBase__OnState2 */    \
    /* +0x068 */ void (*onState3)(Self *self)                /* IntermediateBase__OnState3 */

#define INTERMEDIATEBASE_FIELDS(Methods)                                                           \
    BASICCLASS_FIELDS(Methods);                                                                    \
    /* +0x00C */ IntermediateBaseInitArgs *initArgs; /* init's argument, kept */                   \
    /* +0x010 */ BasicClass *unk10;     /* initArgs->unk8, or init's own New_D8006EF50() object */ \
    /* +0x014 */ BasicClass *unk14;     /* initArgs->unkC, or init's own New_LightRig() object */ \
    /* +0x018 */ BasicClass *viewport;  /* initArgs->viewport, or init's own New_Viewport() */     \
    /* +0x01C */ s32 frameCounter;      /* update adds 1; resetCounters, onState2, onState3 clear it */ \
    /* +0x020 */ s32 state;             /* setState's argument; resetCounters clears it */         \
    /* +0x024 */ s32 initMode           /* init's mode: 0 attaches to the viewport and runs to the end */

struct IntermediateBaseMethods {
    INTERMEDIATEBASE_SLOTS(IntermediateBase, (IntermediateBase *self));
};

struct IntermediateBase {
    INTERMEDIATEBASE_FIELDS(IntermediateBaseMethods);
};

extern IntermediateBaseMethods gIntermediateBaseMethods;
extern IntermediateBaseMethods *Get_vtable_IntermediateBase(void); /* returns &gIntermediateBaseMethods */

void IntermediateBase__IntermediateBase(IntermediateBase *self);
void IntermediateBase__OnNotify(IntermediateBase *self, BasicClass *sender, s32 event);
void IntermediateBase__ResetCounters(IntermediateBase *self);
void IntermediateBase__Init(IntermediateBase *self, IntermediateBaseInitArgs *args, s32 mode);
void IntermediateBase__Deinit(IntermediateBase *self);
void IntermediateBase__OnTag1Notify(IntermediateBase *self, BasicClass *sender, s32 event);
void IntermediateBase__IncrementFrameCounter(IntermediateBase *self);
void IntermediateBase__SetState(IntermediateBase *self, s32 state);
void IntermediateBase__OnState2(IntermediateBase *self);
void IntermediateBase__OnState3(IntermediateBase *self);

#endif
