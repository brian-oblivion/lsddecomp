#ifndef TIMEDTASK_H
#define TIMEDTASK_H

#include "IntermediateBase.h"

/*
 * TimedTask -- class id 0x230, method table gTimedTaskMethods: the
 * IntermediateBase subclass that adds a frame timeout and a sound object to
 * IntermediateBase's run-one-job-to-a-result. Methods in src/class_39e08.c
 * (New_TimedTask through TimedTask__SetTimeout) and src/class_3ac78.c
 * (TimedTask__PlaySound, GetTimedTaskMethods). The object is 0x38 bytes
 * (New_TimedTask). Two classes derive from it, each ctor calling
 * TimedTask__TimedTask first (`typeviews.py --tree`): gClass865C8Methods
 * (0x1F230, Class865C8, class_39e08) and gObjMMethods (0x2F230, ObjM,
 * class_3bb8c_k/_l/_m).
 *
 * Construction, ctor(soundBankPath, sound): the base ctor, then `sound` =
 * New_VabStreamObj(soundBankPath) when a path is given (Class865C8 passes
 * GetSoundEffectDir()) or the caller's object (ObjM passes Class865C8's), and
 * resetCounters, which here is TimedTask__CancelTimeout: setTimeout(-1).
 * Finalize releases `sound` only when the ctor made it, then runs the base
 * finalize. playSound(tone) is the sound's +0x080, VabStreamObj__PlayTone,
 * with 0x7F, 0x7F (TaskCore__PlaySound's shape; ObjM calls the same object's
 * +0x088/+0x08C, VabStreamObj's Mute/Unmute).
 *
 * The timeout. setTimeout(n) stores n * 20 frames, or n itself when
 * negative. update (TimedTask__CheckTimeout) runs the base update, which
 * counts frames, then calls setState(4) once frameCounter passes
 * timeoutFrames as UNSIGNED, so -1 never fires. setState
 * (TimedTask__SetState) runs the base setState and, for 4, sets result = 1
 * and calls onState4. init (TimedTask__Init) zeroes `result`, runs the base
 * init and returns `result`: Class6D3C8__PollStatusObj, through Class865C8,
 * switches on it (2 and 3 are Class865C8's own codes).
 *
 * Slots +0x074..+0x07C are NULL in this class's own table (the table is 0x80
 * bytes, its last three words zero; classtable.py stops at the last
 * non-NULL slot). +0x07C is the one this class calls.
 *
 * IntermediateBase's onInit (+0x04C) is (self, s32, s32, s32). Of this
 * class's two subclasses, ObjM__InitStyleAndWorld takes three arguments
 * after self and Class865C8__OnInit takes self alone; neither is retyped
 * here.
 */

typedef struct TimedTask TimedTask;
typedef struct TimedTaskMethods TimedTaskMethods;

/* clang-format off */
#define TIMEDTASK_SLOTS(Self, CtorParams)                                                         \
    INTERMEDIATEBASE_SLOTS(Self, CtorParams);                                                      \
    /* +0x06C */ void (*setTimeout)(Self *self, s32 timeout); /* TimedTask__SetTimeout: timeoutFrames = timeout * 20 (negative: kept, never fires) */ \
    /* +0x070 */ void (*playSound)(Self *self, s32 tone);     /* TimedTask__PlaySound: sound's PlayTone(tone, 0x7F, 0x7F) */ \
    /* +0x074 */ void (*slot74)(Self *self);                  /* NULL; ObjM__TogglePause */ \
    /* +0x078 */ void *slot78;                                /* NULL in all three tables */     \
    /* +0x07C */ void (*onState4)(Self *self)                 /* NULL; setState(4) calls it; empty in both subclasses */
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
