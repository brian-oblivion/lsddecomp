#ifndef TIMEDTASK_H
#define TIMEDTASK_H

#include "intermediate_base.h"

/*
 * TimedTask -- class id 0x230, method table gTimedTaskMethods. An
 * IntermediateBase (a job run to a result) that adds three things: a frame
 * timeout that ends the job with result 1, a sound object it can play tones
 * on, and `result` itself, which init returns. The object is 0x38 bytes.
 *
 * Nothing builds a bare TimedTask (New_TimedTask has no caller); the game
 * uses its two subclasses, whose ctors call TimedTask__TimedTask first:
 * DayTask (0x1F230, include/day_task.h), built by
 * GameApplication__RunDayTask, which switches on init's return; and ObjM
 * (0x2F230, include/ObjM.h), which DayTask builds and hands its sound.
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
 * playSound(tone) is the sound's VabStreamObj__PlayTone(tone, 0x7F, 0x7F);
 * `sound` is typed `BasicClass *`, and its callers cast it to VabStreamObj.
 *
 * Slots +0x074..+0x07C are NULL in this class's own table (0x80 bytes, the
 * last three words zero); setState(4) calls +0x07C. Overrides in the
 * subclasses whose parameters differ from the slot (onInit, +0x04C) keep the
 * slot's type; each subclass header lists its own.
 */

typedef struct TimedTask TimedTask;
typedef struct TimedTaskMethods TimedTaskMethods;

/* The state update enters once frameCounter passes timeoutFrames;
 * TimedTask__SetState answers it by setting result to
 * TIMEDTASK_RESULT_TIMED_OUT and calling onTimedOut. */
enum TimedTaskState { TIMEDTASK_STATE_TIMED_OUT = 4 };

#define TIMEDTASK_RESULT_TIMED_OUT 1

/* setTimeout(n) arms the timeout at n * TIMEDTASK_TIMEOUT_UNIT_FRAMES frames. */
#define TIMEDTASK_TIMEOUT_UNIT_FRAMES 20

/* clang-format off */
#define TIMEDTASK_SLOTS(Self, CtorParams)                                                         \
    INTERMEDIATEBASE_SLOTS(Self, CtorParams);                                                      \
    /* +0x06C */ void (*setTimeout)(Self *self, s32 timeout); /* TimedTask__SetTimeout: timeoutFrames = timeout * 20 (negative: kept, never fires) */ \
    /* +0x070 */ void (*playSound)(Self *self, s32 tone);     /* TimedTask__PlaySound: sound's PlayTone(tone, 0x7F, 0x7F) */ \
    /* +0x074 */ void (*togglePause)(Self *self);                 /* NULL; ObjM__TogglePause */ \
    /* +0x078 */ void *slot78;                                /* NULL in all three tables */     \
    /* +0x07C */ void (*onTimedOut)(Self *self)                 /* NULL; setState(4) calls it; empty in both subclasses */
/* clang-format on */

/* clang-format off */
#define TIMEDTASK_FIELDS(Methods)                                                                 \
    INTERMEDIATEBASE_FIELDS(Methods);                                                              \
    /* +0x028 */ s32 result;          /* init zeroes it and returns it; setState(4) sets 1 */      \
    /* +0x02C */ s32 timeoutFrames;   /* setTimeout; update: frameCounter past it (unsigned) is setState(4) */ \
    /* +0x030 */ char *soundBankPath; /* the ctor's; nonzero: finalize releases `sound` */         \
    /* +0x034 */ BasicClass *sound    /* New_VabStreamObj(soundBankPath) or the ctor's own; the object is 0x38 bytes */
/* clang-format on */

struct TimedTaskMethods {
    TIMEDTASK_SLOTS(TimedTask, (TimedTask * self, char *soundBankPath, BasicClass *sound));
};

struct TimedTask {
    TIMEDTASK_FIELDS(TimedTaskMethods);
};

extern TimedTaskMethods gTimedTaskMethods;
extern TimedTaskMethods *GetTimedTaskMethods(void); /* returns &gTimedTaskMethods */

TimedTask *New_TimedTask(char *soundBankPath, BasicClass *sound);
void TimedTask__TimedTask(TimedTask *self, char *soundBankPath, BasicClass *sound);
void TimedTask__Finalize(TimedTask *self);
void TimedTask__CancelTimeout(TimedTask *self);
s32 TimedTask__Init(TimedTask *self, IntermediateBaseInitArgs *args, s32 mode);
void TimedTask__Deinit(TimedTask *self);
void TimedTask__NoOpSlot58(void);
void TimedTask__CheckTimeout(TimedTask *self, BasicClass *sender, s32 event);
void TimedTask__SetState(TimedTask *self, s32 state);
void TimedTask__SetTimeout(TimedTask *self, s32 timeout);
void TimedTask__PlaySound(TimedTask *self, s32 tone);

#endif
