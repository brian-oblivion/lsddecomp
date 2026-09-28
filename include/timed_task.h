/**
 * @file timed_task.h
 * @brief TimedTask, the IntermediateBase job with a frame timeout, a sound
 *        object and a result, and its method table.
 *
 * Nothing builds a bare TimedTask (New_TimedTask has no caller); the game
 * uses its two subclasses, whose ctors call TimedTask__TimedTask first:
 * DayTask (0x1F230, include/day_task.h), built by
 * GameApplication__RunDayTask, which switches on init's return; and ObjM
 * (0x2F230, include/objm.h), which DayTask builds and hands its sound.
 */
#ifndef TIMED_TASK_H
#define TIMED_TASK_H

#include "intermediate_base.h"

typedef struct TimedTask TimedTask;
typedef struct TimedTaskMethods TimedTaskMethods;

/** The state update enters once frameCounter passes timeoutFrames;
 * TimedTask__SetState answers it by setting result to
 * TIMEDTASK_RESULT_TIMED_OUT and calling onTimedOut. */
enum TimedTaskState {
    TIMEDTASK_STATE_TIMED_OUT = 4 /**< the timeout fired */
};

/** TimedTask::result after the timeout fired. */
#define TIMEDTASK_RESULT_TIMED_OUT 1

/** setTimeout(n) arms the timeout at n * TIMEDTASK_TIMEOUT_UNIT_FRAMES frames. */
#define TIMEDTASK_TIMEOUT_UNIT_FRAMES 20

/**
 * IntermediateBase's slots, then TimedTask's own, for TimedTaskMethods and
 * the subclasses' tables to expand first. gTimedTaskMethods overrides the
 * ctor, finalize, resetCounters (TimedTask__CancelTimeout), init, deinit,
 * onPadEvent (+0x058, TimedTask__NoOpSlot58), update
 * (TimedTask__CheckTimeout) and setState. Slots +0x074..+0x07C are NULL in
 * this class's own table (the last three words of its 0x80 bytes);
 * setState(4) calls +0x07C.
 */
/* clang-format off */
#define TIMEDTASK_SLOTS(Self, CtorParams)                                                         \
    INTERMEDIATEBASE_SLOTS(Self, CtorParams);                                                      \
    /* +0x06C */ void (*setTimeout)(Self *self, s32 timeout); /* TimedTask__SetTimeout: timeoutFrames = timeout * 20 (negative: kept, never fires) */ \
    /* +0x070 */ void (*playSound)(Self *self, s32 tone);     /* TimedTask__PlaySound: sound's PlayTone(tone, 0x7F, 0x7F) */ \
    /* +0x074 */ void (*togglePause)(Self *self);                 /* NULL; ObjM__TogglePause */ \
    /* +0x078 */ void *slot78;                                /* NULL in all three tables */     \
    /* +0x07C */ void (*onTimedOut)(Self *self)                 /* NULL; setState(4) calls it; empty in both subclasses */
/* clang-format on */

/**
 * IntermediateBase's fields, then TimedTask's own, for TimedTask and the
 * subclasses' objects to expand first. The object is 0x38 bytes.
 */
/* clang-format off */
#define TIMEDTASK_FIELDS(Methods)                                                                 \
    INTERMEDIATEBASE_FIELDS(Methods);                                                              \
    /* +0x028 */ s32 result;          /* init zeroes it and returns it; setState(4) sets 1 */      \
    /* +0x02C */ s32 timeoutFrames;   /* setTimeout; update: frameCounter past it (unsigned) is setState(4) */ \
    /* +0x030 */ char *soundBankPath; /* the ctor's; nonzero: finalize releases `sound` */         \
    /* +0x034 */ BasicClass *sound    /* New_VabStreamObj(soundBankPath) or the ctor's own; the object is 0x38 bytes */
/* clang-format on */

/** @brief TimedTask's method table: TIMEDTASK_SLOTS with its ctor's parameters. */
struct TimedTaskMethods {
    TIMEDTASK_SLOTS(TimedTask, (TimedTask * self, char *soundBankPath, BasicClass *sound));
};

/**
 * @brief TimedTask: an IntermediateBase (a job run to a result) that adds a
 *        frame timeout ending the job with result 1, a sound object it can
 *        play tones on, and `result` itself, which init returns.
 *
 * Class id 0x230, table gTimedTaskMethods, parent IntermediateBase
 * (include/intermediate_base.h); methods in src/world/dream_day.c. 0x38
 * bytes.
 *
 * Lifecycle. ctor(soundBankPath, sound): with a path, `sound` is
 * New_VabStreamObj(soundBankPath) and finalize releases it; without one it
 * is the caller's object and finalize leaves it alone. The ctor ends with
 * resetCounters (TimedTask__CancelTimeout: setTimeout(-1)). init zeroes
 * `result`, runs IntermediateBase's init and returns `result`.
 *
 * The timeout. setTimeout(n) arms it at n * 20 frames; a negative n
 * disarms it. update (TimedTask__CheckTimeout) runs the base update, which
 * counts frames, then calls setState(4) once frameCounter passes
 * timeoutFrames compared unsigned, so -1 never fires. setState(4)
 * (TimedTask__SetState) sets result = 1 and calls onTimedOut. The only
 * setTimeout call in the game is CancelTimeout's -1: the mechanism is
 * complete but never armed.
 *
 * `sound` is typed `BasicClass *`, and the callers that play on it cast it
 * to VabStreamObj. Overrides in the subclasses whose parameters differ from
 * the slot (onInit, +0x04C) keep the slot's type; each subclass header lists
 * its own.
 */
struct TimedTask {
    TIMEDTASK_FIELDS(TimedTaskMethods);
};

/** TimedTask's method table (class id 0x230). */
extern TimedTaskMethods gTimedTaskMethods;

/**
 * @brief The TimedTask method table.
 * @return &gTimedTaskMethods.
 */
extern TimedTaskMethods *GetTimedTaskMethods(void);

/**
 * @brief Allocates a TimedTask and runs its ctor through the table. No
 *        caller.
 * @param soundBankPath The sound bank to open, or NULL to use `sound`.
 * @param sound The sound object to use when soundBankPath is NULL.
 * @return The new TimedTask, or NULL when the allocation fails.
 */
TimedTask *New_TimedTask(char *soundBankPath, BasicClass *sound);

/**
 * @brief Constructor (slot +0x008): IntermediateBase's ctor, installs
 *        gTimedTaskMethods, takes the sound object and disarms the timeout.
 * @param self The task.
 * @param soundBankPath Non-NULL: `sound` becomes New_VabStreamObj(path), and
 *        finalize releases it.
 * @param sound The caller's sound object, used when soundBankPath is NULL.
 */
void TimedTask__TimedTask(TimedTask *self, char *soundBankPath, BasicClass *sound);

/**
 * @brief finalize (slot +0x00C): releases `sound` when the ctor opened it,
 *        then IntermediateBase's finalize.
 * @param self The task.
 */
void TimedTask__Finalize(TimedTask *self);

/**
 * @brief resetCounters (slot +0x040): disarms the timeout (setTimeout(-1)).
 * @param self The task.
 */
void TimedTask__CancelTimeout(TimedTask *self);

/**
 * @brief init (slot +0x044): zeroes `result`, runs IntermediateBase's init
 *        and returns `result` as the job left it.
 * @param self The task.
 * @param args The shared init arguments (draw system, pad, viewport, ...).
 * @param mode Handed on to IntermediateBase's init.
 * @return The task's result, as the job left it.
 */
s32 TimedTask__Init(TimedTask *self, IntermediateBaseInitArgs *args, s32 mode);

/**
 * @brief deinit (slot +0x048): IntermediateBase's deinit.
 * @param self The task.
 */
void TimedTask__Deinit(TimedTask *self);

/**
 * @brief onPadEvent (slot +0x058): empty; a TimedTask ignores the pad.
 */
void TimedTask__NoOpSlot58(void);

/**
 * @brief update (slot +0x05C): IntermediateBase's update (counts the frame),
 *        then setState(TIMEDTASK_STATE_TIMED_OUT) once frameCounter passes
 *        timeoutFrames, compared unsigned so a negative timeout never fires.
 * @param self The task.
 * @param sender The FrameClock that sent the tick.
 * @param event The tick's event code, handed on.
 */
void TimedTask__CheckTimeout(TimedTask *self, BasicClass *sender, s32 event);

/**
 * @brief setState (slot +0x060): IntermediateBase's setState; on
 *        TIMEDTASK_STATE_TIMED_OUT also sets result to 1 and calls onTimedOut.
 * @param self The task.
 * @param state The new state.
 */
void TimedTask__SetState(TimedTask *self, s32 state);

/**
 * @brief setTimeout (slot +0x06C): arms the timeout at timeout * 20 frames,
 *        or stores a negative timeout as it is, which never fires.
 * @param self The task.
 * @param timeout The timeout in units of TIMEDTASK_TIMEOUT_UNIT_FRAMES, or
 *        negative to disarm.
 */
void TimedTask__SetTimeout(TimedTask *self, s32 timeout);

/**
 * @brief playSound (slot +0x070): plays a tone at full volume on the sound
 *        object (VabStreamObj__PlayTone(tone, 127, 127)), if there is one.
 * @param self The task.
 * @param tone The tone to play.
 */
void TimedTask__PlaySound(TimedTask *self, s32 tone);

#endif
