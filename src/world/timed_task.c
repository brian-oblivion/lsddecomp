/*
 * TimedTask's methods (include/timed_task.h: the IntermediateBase job with a
 * frame timeout and a sound object, DayTask's and ObjM's parent), in ROM
 * order: the allocator, ctor and finalize, resetCounters (CancelTimeout),
 * init and deinit, the empty onPadEvent, update (CheckTimeout), setState,
 * setTimeout and PlaySound, ending with its getter GetTimedTaskMethods. Its
 * method table opens the file; a slot whose function is declared for
 * another class's `self` (a parent's method) takes a `void *` cast.
 */
#include "common.h"
#include <libgte.h>
#include "wbgm.h"
#include "timed_task.h"
#include "bmem_pmgr.h"

/* TimedTask's method table, class id 0x230: +0x074..+0x07C are NULL here
 * and filled by its subclasses. */
/* clang-format off */
TimedTaskMethods gTimedTaskMethods = {
    /* +0x000 header */ 0x230,
    /* +0x004 release */ (void *)BasicClass__Release,
    /* +0x008 ctor */ TimedTask__TimedTask,
    /* +0x00C finalize */ TimedTask__Finalize,
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
    /* +0x038 onNotify */ (void *)IntermediateBase__OnNotify,
    /* +0x03C slot3C */ NULL,
    /* +0x040 resetCounters */ TimedTask__CancelTimeout,
    /* +0x044 init */ TimedTask__Init,
    /* +0x048 deinit */ TimedTask__Deinit,
    /* +0x04C onInit */ NULL,
    /* +0x050 onDeinit */ NULL,
    /* +0x054 onDrawSystemEvent */ (void *)IntermediateBase__OnDrawSystemEvent,
    /* +0x058 onPadEvent */ (void *)TimedTask__NoOpOnPadEvent,
    /* +0x05C update */ TimedTask__CheckTimeout,
    /* +0x060 setState */ TimedTask__SetState,
    /* +0x064 onStart */ (void *)IntermediateBase__OnStart,
    /* +0x068 onStop */ (void *)IntermediateBase__OnStop,
    /* +0x06C setTimeout */ TimedTask__SetTimeout,
    /* +0x070 playSound */ TimedTask__PlaySound,
    /* +0x074 togglePause */ NULL,
    /* +0x078 slot78 */ NULL,
    /* +0x07C onTimedOut */ NULL,
};
/* clang-format on */

TimedTask *New_TimedTask(char *soundBankPath, BasicClass *sound) {
    TimedTask *self;

    self = BMemPMgrAlloc(sizeof(TimedTask));
    if (self != NULL) {
        GetTimedTaskMethods()->ctor(self, soundBankPath, sound);
        return self;
    }
    return NULL;
}

void TimedTask__TimedTask(TimedTask *self, char *soundBankPath, BasicClass *sound) {
    GetIntermediateBaseMethods()->ctor((IntermediateBase *)self);
    self->methods = GetTimedTaskMethods();
    if (soundBankPath != NULL) {
        self->sound = (BasicClass *)New_VabStreamObj(soundBankPath);
    } else {
        self->sound = sound;
    }
    self->soundBankPath = soundBankPath;
    self->methods->resetCounters(self);
}

void TimedTask__Finalize(TimedTask *self) {
    if (self->soundBankPath != NULL) {
        self->sound->methods->release(self->sound);
    }
    GetIntermediateBaseMethods()->finalize((IntermediateBase *)self);
}

void TimedTask__CancelTimeout(TimedTask *self) {
    self->methods->setTimeout(self, -1);
}

s32 TimedTask__Init(TimedTask *self, IntermediateBaseInitArgs *args, s32 mode) {
    self->result = TIMEDTASK_RESULT_DONE;
    GetIntermediateBaseMethods()->init((IntermediateBase *)self, args, mode);
    return self->result;
}

void TimedTask__Deinit(TimedTask *self) {
    GetIntermediateBaseMethods()->deinit((IntermediateBase *)self);
}

void TimedTask__NoOpOnPadEvent(void) {}

void TimedTask__CheckTimeout(TimedTask *self, BasicClass *sender, s32 event) {
    GetIntermediateBaseMethods()->update((IntermediateBase *)self, sender, event);
    if (self->frameCounter > self->timeoutFrames) {
        self->methods->setState(self, TIMEDTASK_STATE_TIMED_OUT);
    }
}

void TimedTask__SetState(TimedTask *self, s32 state) {
    GetIntermediateBaseMethods()->setState((IntermediateBase *)self, state);
    if (state == TIMEDTASK_STATE_TIMED_OUT) {
        self->result = TIMEDTASK_RESULT_TIMED_OUT;
        self->methods->onTimedOut(self);
    }
}

void TimedTask__SetTimeout(TimedTask *self, s32 timeout) {
    self->timeoutFrames = (timeout < 0) ? timeout : timeout * TIMEDTASK_TIMEOUT_UNIT_FRAMES;
}

/* `sound` may be the ctor's own argument; as a VabStreamObj, its +0x080 is
 * VabStreamObj__PlayTone. */
void TimedTask__PlaySound(TimedTask *self, s32 tone) {
    VabStreamObj *sound = (VabStreamObj *)self->sound;

    if (sound != NULL) {
        sound->methods->playTone(sound, tone, TIMEDTASK_TONE_VOLUME, TIMEDTASK_TONE_VOLUME);
    }
}

TimedTaskMethods *GetTimedTaskMethods(void) {
    return &gTimedTaskMethods;
}
