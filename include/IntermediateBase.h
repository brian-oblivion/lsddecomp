#ifndef INTERMEDIATEBASE_H
#define INTERMEDIATEBASE_H

#include "basic_class.h"

/*
 * IntermediateBase -- class id 0x30, method table gIntermediateBaseMethods: a
 * BasicClass subclass that runs one attached job to a result. Methods in
 * src/app/task.c. Two classes derive from it (`typeviews.py --tree`):
 * TaskCore (0x130, gTaskCoreMethods: StreamTask, TitleMenu, GraphRoom
 * below it) and TimedTask (0x230: gDayTaskMethods's DayTask and gObjMMethods's ObjM
 * below it). It is abstract: +0x04C, +0x050 and +0x058 are NULL in its own
 * table, and init/deinit call the first two. It has no allocator; the object
 * is 0x28 bytes because both subclasses' own fields start at +0x028
 * (TimedTask's result, the +0x028 TaskCore__Reset sets).
 *
 * init(args, mode) keeps `args`, adds args->unk0, args->unk4 and the +0x010
 * helper as children, runs onInit and records `mode`. With mode 0 it also
 * attaches args->unk0 and the helper to the viewport and the helper to the
 * +0x014 object, then calls setState(2) and deinit, which undoes all of it
 * and releases whatever init created itself. Callers construct an object,
 * call init and use what it RETURNS (GameApplication__RunTask,
 * GameApplication__RunDayTask); the subclass overrides return a result field
 * (TimedTask__Init: +0x028 result; TaskCore__Init: +0x038 result).
 *
 * onNotify splits by the SENDER's root class nibble, as SceneNode's does:
 * a gDrawSystemMethods (1) goes to onDrawSystemEvent, a Pad (2) to onPadEvent, a
 * FrameClock (5) to update, which here only counts frames. setState stores
 * the state, passes it to notifyParents and runs onStart or onStop for 2
 * or 3. Both reset the frame counter and call initArgs->unk0.
 */

typedef struct IntermediateBase IntermediateBase;
typedef struct IntermediateBaseMethods IntermediateBaseMethods;
typedef struct IntermediateBaseInitArgs IntermediateBaseInitArgs;

/* The two states IntermediateBase__SetState acts on itself; a subclass's
 * own states sit above them (TaskCore.h's enum TaskCoreState). */
enum IntermediateBaseState {
    INTERMEDIATEBASE_STATE_START = 2, /* init's (mode 0); onStart starts the DrawSystem */
    INTERMEDIATEBASE_STATE_STOP = 3   /* onStop stops the DrawSystem */
};

/* init's argument. The caller owns it; the object keeps the pointer at
 * +0x00C. DayTask__DayTask fills +0x008..+0x010 itself (New_FrameClock(),
 * New_StageMap(0, 1), New_NodeGuardedViewport()). */
struct IntermediateBaseInitArgs {
    /* +0x000 */ BasicClass *drawSystem; /* Application__InitSystems: the DrawSystem; added as a child; onStart calls its +0x048, onStop its +0x04C */
    /* +0x004 */ BasicClass *pad; /* Application__InitSystems: the Pad; added as a child; onDrawSystemEvent's event 2 calls its +0x044, +0x048 */
    /* +0x008 */ BasicClass *frameClock; /* becomes frameClock; NULL: init makes one with New_FrameClock() */
    /* +0x00C */ BasicClass *lightRig; /* becomes lightRig; DayTask passes a StageMap (a LightRig); NULL: init makes one with New_LightRig() */
    /* +0x010 */ BasicClass *viewport; /* becomes viewport; NULL: init makes one with New_Viewport() */
};

/* clang-format off */
#define INTERMEDIATEBASE_SLOTS(Self, CtorParams)                                                   \
    BASICCLASS_SLOTS(Self, CtorParams);                                                            \
    /* +0x040 */ void (*resetCounters)(Self *self);          /* IntermediateBase__ResetCounters; the ctor's last call */ \
    /* +0x044 */ s32 (*init)(Self *self, IntermediateBaseInitArgs *args, s32 mode); /* IntermediateBase__Init; s32: RunTask/RunDayTask use the result */ \
    /* +0x048 */ void (*deinit)(Self *self);                 /* IntermediateBase__Deinit */      \
    /* +0x04C */ void (*onInit)(Self *self, s32 arg1, s32 arg2, s32 arg3); /* NULL; init calls it (0, 0, 0) after adding the children */ \
    /* +0x050 */ void (*onDeinit)(Self *self);               /* NULL; deinit's first call */     \
    /* +0x054 */ void (*onDrawSystemEvent)(Self *self, BasicClass *sender, s32 event); /* IntermediateBase__OnDrawSystemEvent: onNotify's gDrawSystemMethods (1) case */ \
    /* +0x058 */ void (*onPadEvent)(Self *self, BasicClass *sender, s32 event);   /* NULL; onNotify's Pad (2) case */ \
    /* +0x05C */ void (*update)(Self *self, BasicClass *sender, s32 event);       /* IntermediateBase__IncrementFrameCounter: onNotify's FrameClock (5) case */ \
    /* +0x060 */ void (*setState)(Self *self, s32 state);    /* IntermediateBase__SetState; TaskCore__SetState, TitleMenu__SetState */ \
    /* +0x064 */ void (*onStart)(Self *self);               /* IntermediateBase__OnStart */    \
    /* +0x068 */ void (*onStop)(Self *self)                /* IntermediateBase__OnStop */
/* clang-format on */

/* clang-format off */
#define INTERMEDIATEBASE_FIELDS(Methods)                                                           \
    BASICCLASS_FIELDS(Methods);                                                                    \
    /* +0x00C */ IntermediateBaseInitArgs *initArgs; /* init's argument, kept */                   \
    /* +0x010 */ BasicClass *frameClock; /* initArgs->frameClock, or init's own New_FrameClock() object */ \
    /* +0x014 */ BasicClass *lightRig;  /* initArgs->lightRig, or init's own New_LightRig() object */ \
    /* +0x018 */ BasicClass *viewport;  /* initArgs->viewport, or init's own New_Viewport() */     \
    /* +0x01C */ s32 frameCounter;      /* update adds 1; resetCounters, onStart, onStop clear it */ \
    /* +0x020 */ s32 state;             /* setState's argument; resetCounters clears it */         \
    /* +0x024 */ s32 initMode           /* init's mode: 0 attaches to the viewport and runs to the end */
/* clang-format on */

struct IntermediateBaseMethods {
    INTERMEDIATEBASE_SLOTS(IntermediateBase, (IntermediateBase * self));
};

struct IntermediateBase {
    INTERMEDIATEBASE_FIELDS(IntermediateBaseMethods);
};

extern IntermediateBaseMethods gIntermediateBaseMethods;
extern IntermediateBaseMethods *GetIntermediateBaseMethods(void); /* returns &gIntermediateBaseMethods */

void IntermediateBase__IntermediateBase(IntermediateBase *self);
void IntermediateBase__OnNotify(IntermediateBase *self, BasicClass *sender, s32 event);
void IntermediateBase__ResetCounters(IntermediateBase *self);
void IntermediateBase__Init(IntermediateBase *self, IntermediateBaseInitArgs *args, s32 mode);
void IntermediateBase__Deinit(IntermediateBase *self);
void IntermediateBase__OnDrawSystemEvent(IntermediateBase *self, BasicClass *sender, s32 event);
void IntermediateBase__IncrementFrameCounter(IntermediateBase *self);
void IntermediateBase__SetState(IntermediateBase *self, s32 state);
void IntermediateBase__OnStart(IntermediateBase *self);
void IntermediateBase__OnStop(IntermediateBase *self);

#endif
