#ifndef INTERMEDIATE_BASE_H
#define INTERMEDIATE_BASE_H

#include "basic_class.h"

/**
 * @file intermediate_base.h
 * @brief IntermediateBase, the base of the game's tasks: a BasicClass that
 * runs one attached job to a result, with the argument block every task is
 * started with.
 *
 * Declares the class (INTERMEDIATEBASE_SLOTS, INTERMEDIATEBASE_FIELDS for its
 * subclasses), its two built-in states, the IntermediateBaseInitArgs block,
 * and its methods, defined in src/app/task.c.
 */

typedef struct IntermediateBase IntermediateBase;
typedef struct IntermediateBaseMethods IntermediateBaseMethods;

/** IntermediateBase's class id (gIntermediateBaseMethods word +0x000). */
#define INTERMEDIATEBASE_CLASS_ID 0x30

typedef struct IntermediateBaseInitArgs IntermediateBaseInitArgs;

/** The two states IntermediateBase__SetState acts on itself; a subclass's
 * own states sit above them (task_core.h's enum TaskCoreState). */
enum IntermediateBaseState {
    INTERMEDIATEBASE_STATE_START = 2, /**< init's (INTERMEDIATEBASE_INIT_RUN); onStart starts the DrawSystem */
    INTERMEDIATEBASE_STATE_STOP = 3 /**< onStop stops the DrawSystem */
};

/** init's `mode`, kept as initMode. */
enum IntermediateBaseInitMode {
    /** Attach to the viewport, start, and run the task to its STOP before
     * init returns; then deinit. How the game runs a whole task. */
    INTERMEDIATEBASE_INIT_RUN = 0,
    /** Any nonzero value: add the children and run onInit only; whoever
     * holds the task drives it (ObjM's init passes its DreamSys pointer). */
    INTERMEDIATEBASE_INIT_ATTACH = 1
};

/** init's argument: the objects a task works with. The caller owns it; the
 * task keeps the pointer (initArgs). Application__InitSystems fills
 * drawSystem and pad and leaves the rest NULL; DayTask__DayTask fills
 * frameClock..viewport itself (New_FrameClock(), New_StageMap(0, 1),
 * New_NodeGuardedViewport()). */
struct IntermediateBaseInitArgs {
    /** The DrawSystem: added as a child; onStart calls its start, onStop its
     * stop. */
    /* +0x000 */ BasicClass *drawSystem;

    /** The Pad: added as a child; onDrawSystemEvent's VSync case calls its
     * updateMasks and dispatchEvents. */
    /* +0x004 */ BasicClass *pad;
    /* +0x008 */ BasicClass *frameClock; /**< becomes frameClock; NULL: init makes one with New_FrameClock() */
    /* +0x00C */ BasicClass *lightRig; /**< becomes lightRig; DayTask passes a StageMap (a LightRig); NULL: init makes one with New_LightRig() */
    /* +0x010 */ BasicClass *viewport; /**< becomes viewport; NULL: init makes one with New_Viewport() */
};

/** IntermediateBase's slots, BasicClass's first. */
/* clang-format off */
#define INTERMEDIATEBASE_SLOTS(Self, CtorParams)                                                   \
    BASICCLASS_SLOTS(Self, CtorParams);                                                            \
    /* +0x040 */ void (*resetCounters)(Self *self);          /**< @see IntermediateBase__ResetCounters; the ctor's last call */ \
    /* +0x044 */ s32 (*init)(Self *self, IntermediateBaseInitArgs *args, s32 mode); /**< @see IntermediateBase__Init; the subclasses' overrides return a result (RunTask, RunDayTask read it) */ \
    /* +0x048 */ void (*deinit)(Self *self);                 /**< @see IntermediateBase__Deinit */      \
    /* +0x04C */ void (*onInit)(Self *self, s32 arg1, s32 arg2, s32 arg3); /**< NULL here; init calls it (0, 0, 0) after adding the children */ \
    /* +0x050 */ void (*onDeinit)(Self *self);               /**< NULL here; deinit's first call */     \
    /* +0x054 */ void (*onDrawSystemEvent)(Self *self, BasicClass *sender, s32 event); /**< @see IntermediateBase__OnDrawSystemEvent: onNotify's DrawSystem (1) case */ \
    /* +0x058 */ void (*onPadEvent)(Self *self, BasicClass *sender, s32 event);   /**< NULL here; onNotify's Pad (2) case */ \
    /* +0x05C */ void (*update)(Self *self, BasicClass *sender, s32 event);       /**< @see IntermediateBase__IncrementFrameCounter: onNotify's FrameClock (5) case */ \
    /* +0x060 */ void (*setState)(Self *self, s32 state);    /**< @see IntermediateBase__SetState; TaskCore__SetState, TitleMenu__SetState */ \
    /* +0x064 */ void (*onStart)(Self *self);               /**< @see IntermediateBase__OnStart */    \
    /* +0x068 */ void (*onStop)(Self *self)                /**< @see IntermediateBase__OnStop */
/* clang-format on */

/** IntermediateBase's fields, BasicClass's first. */
/* clang-format off */
#define INTERMEDIATEBASE_FIELDS(Methods)                                                           \
    BASICCLASS_FIELDS(Methods);                                                                    \
    /* +0x00C */ IntermediateBaseInitArgs *initArgs; /**< init's argument, kept */                   \
    /* +0x010 */ BasicClass *frameClock; /**< initArgs->frameClock, or init's own New_FrameClock() object */ \
    /* +0x014 */ BasicClass *lightRig;  /**< initArgs->lightRig, or init's own New_LightRig() object */ \
    /* +0x018 */ BasicClass *viewport;  /**< initArgs->viewport, or init's own New_Viewport() */     \
    /* +0x01C */ s32 frameCounter;      /**< update adds 1; resetCounters, onStart, onStop clear it */ \
    /* +0x020 */ s32 state;             /**< setState's argument; resetCounters clears it */         \
    /* +0x024 */ s32 initMode           /**< init's mode, an enum IntermediateBaseInitMode */
/* clang-format on */

/** IntermediateBase's method table: BasicClass's slots and
 * INTERMEDIATEBASE_SLOTS' own. */
struct IntermediateBaseMethods {
    INTERMEDIATEBASE_SLOTS(IntermediateBase, (IntermediateBase * self));
};

/**
 * IntermediateBase -- the base of the game's tasks: class id 0x30, method
 * table gIntermediateBaseMethods, a BasicClass subclass that runs one
 * attached job to a result. Methods in src/app/task.c. Two classes derive
 * from it: TaskCore (0x130, task_core.h: StreamTask, TitleMenu and GraphRoom
 * below it) and TimedTask (0x230: DayTask and ObjM below it). It is
 * abstract: +0x04C, +0x050 and +0x058 are NULL in its own table, and
 * init/deinit call the first two. It has no allocator; the object is 0x28
 * bytes, where both subclasses' own fields start.
 *
 * Lifecycle. A caller constructs a subclass, calls init and uses what the
 * subclass's init override RETURNS (GameApplication__RunTask,
 * GameApplication__RunDayTask; TimedTask__Init returns its +0x028 result,
 * TaskCore__Init its +0x038 result), then releases it.
 * init(args, mode) keeps `args`, takes frameClock, lightRig and viewport
 * from it or creates its own, adds args->drawSystem, args->pad and the
 * frame clock as children, runs onInit and records `mode`. With
 * INTERMEDIATEBASE_INIT_RUN it
 * also attaches the DrawSystem and the frame clock to the viewport and the
 * frame clock to the light rig, then calls setState(START) and deinit, so
 * the whole job runs inside init. deinit runs onDeinit, undoes the
 * attachments and releases whatever init created itself.
 *
 * Events. onNotify splits by the SENDER's root class nibble, as SceneNode's
 * does: the DrawSystem (1) goes to onDrawSystemEvent, a Pad (2) to
 * onPadEvent, a FrameClock (5) to update, which here only counts frames.
 * setState stores the state, passes it to notifyParents and runs onStart or
 * onStop for START or STOP; both reset the frame counter and start or stop
 * the DrawSystem.
 */
struct IntermediateBase {
    INTERMEDIATEBASE_FIELDS(IntermediateBaseMethods);
};

/** IntermediateBase's own method table. */
extern IntermediateBaseMethods gIntermediateBaseMethods;

/** @brief IntermediateBase's method-table getter.
 * @return &gIntermediateBaseMethods */
extern IntermediateBaseMethods *GetIntermediateBaseMethods(void);

/** @brief Constructor: BasicClass's, then this table and resetCounters.
 * @param self the object */
void IntermediateBase__IntermediateBase(IntermediateBase *self);

/** @brief onNotify override: BasicClass's, then routes by the sender's root
 * class: DrawSystem to onDrawSystemEvent, Pad to onPadEvent, FrameClock to
 * update.
 * @param self the task
 * @param sender the notifying object
 * @param event its event code */
void IntermediateBase__OnNotify(IntermediateBase *self, BasicClass *sender, s32 event);

/** @brief Clears the frame counter and the state.
 * @param self the task */
void IntermediateBase__ResetCounters(IntermediateBase *self);

/** @brief Binds the task to its objects and, with INTERMEDIATEBASE_INIT_RUN,
 * runs it to the end
 * (see the class documentation).
 * @param self the task
 * @param args the objects to work with; kept by pointer
 * @param mode an enum IntermediateBaseInitMode: INTERMEDIATEBASE_INIT_RUN
 *        attaches, starts and deinits before returning; any other value only
 *        attaches */
void IntermediateBase__Init(IntermediateBase *self, IntermediateBaseInitArgs *args, s32 mode);

/** @brief Runs onDeinit, detaches what init attached, removes the children
 * and releases the frame clock, light rig and viewport init created itself.
 * @param self the task */
void IntermediateBase__Deinit(IntermediateBase *self);

/** @brief On the DrawSystem's VSync event, ticks the frame clock and polls
 * the pad (updateMasks, then dispatchEvents).
 * @param self the task
 * @param sender the DrawSystem
 * @param event its event code */
void IntermediateBase__OnDrawSystemEvent(IntermediateBase *self, BasicClass *sender, s32 event);

/** @brief update's base occupant: adds 1 to the frame counter.
 * @param self the task */
void IntermediateBase__IncrementFrameCounter(IntermediateBase *self);

/** @brief Stores `state`, passes it to the parents (notifyParents), and runs
 * onStart for START or onStop for STOP.
 * @param self the task
 * @param state the new state */
void IntermediateBase__SetState(IntermediateBase *self, s32 state);

/** @brief Clears the frame counter and starts the DrawSystem.
 * @param self the task */
void IntermediateBase__OnStart(IntermediateBase *self);

/** @brief Stops the DrawSystem and clears the frame counter.
 * @param self the task */
void IntermediateBase__OnStop(IntermediateBase *self);

#endif
