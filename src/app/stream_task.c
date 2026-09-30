/*
 * StreamTask's methods (include/stream_task.h: a TaskCore that plays one
 * movie stream through a MoviePlayer inside TaskCore's fade and state
 * machine), in ROM order: New_StreamTask through GetStreamTaskMethods; its
 * method table closes the file. TaskCore, its parent, follows in
 * task_core.c, then IntermediateBase and Viewport in intermediate_base.c
 * and viewport.c.
 */
#include "common.h"
#include "stream_task.h"
#include "movie_player.h"
#include "bmem_pmgr.h"

StreamTask *New_StreamTask(TaskCoreTarget *target, char *soundBankPath, BasicClass *sound,
                           DrawRect *initData) {
    StreamTask *self;

    self = BMemPMgrAlloc(sizeof(StreamTask));
    if (self != NULL) {
        GetStreamTaskMethods()->ctor(self, target, soundBankPath, sound, initData);
        return self;
    }
    return NULL;
}

void StreamTask__StreamTask(StreamTask *self, TaskCoreTarget *target, char *soundBankPath,
                            BasicClass *sound, DrawRect *initData) {
    GetTaskCoreMethods()->ctor((TaskCore *)self, target, soundBankPath, sound);
    self->methods = GetStreamTaskMethods();
    if (initData != NULL) {
        self->initData = *initData;
    } else {
        self->initData = *GetDefaultMovieFrame();
    }
    self->player = New_MoviePlayer(GetDefaultMovieFrame(), 0, 0);
    self->streamName = NULL;
    self->methods->resetCounters(self);
}

void StreamTask__Finalize(StreamTask *self) {
    self->player->methods->release(self->player);
    GetTaskCoreMethods()->finalize((TaskCore *)self);
}

void StreamTask__Reset(StreamTask *self) {
    self->loopCount = -1;
    self->keepActive = 0;
    self->skipOnConfirm = 1;
    self->unkD0 = 0;
    self->abortBeforeFade = 1;
}

void StreamTask__Init(StreamTask *self, IntermediateBaseInitArgs *args, const char *streamName,
                      s32 streamGroup, s32 autoPlay) {
    self->streamName = streamName;
    self->streamGroup = streamGroup;
    self->autoPlay = autoPlay;
    GetTaskCoreMethods()->init((TaskCore *)self, args, INTERMEDIATEBASE_INIT_RUN);
}

void StreamTask__OnInit(StreamTask *self) {
    /* IntermediateBase's onInit slot names init's (0, 0, 0); TaskCore__OnInit
     * takes self alone, and this up-call passes nothing else. */
    ((void (*)(TaskCore *))GetTaskCoreMethods()->onInit)((TaskCore *)self);
    self->playDone = 0;
    self->player->methods->setAutoPlay(self->player, self->autoPlay);
    if (self->player->methods->play(self->player, (char *)self->streamName, self->streamGroup,
                                    self->keepActive, self->loopCount) != 0) {
        self->methods->setFrameBound(self, 0);
    }
}

void StreamTask__Update(StreamTask *self, BasicClass *sender, s32 event) {
    GetTaskCoreMethods()->update((TaskCore *)self, sender, event);
    if (self->playDone != 0) {
        return;
    }
    self->playDone = self->player->methods->advance(self->player);
    if (self->playDone == 0) {
        return;
    }
    if (self->fadingOut != 0) {
        return;
    }
    self->methods->setState(self, TASKCORE_STATE_FADE_OUT);
}

void StreamTask__SetState(StreamTask *self, s32 state) {
    GetTaskCoreMethods()->setState((TaskCore *)self, state);
    switch (state) {
        case TASKCORE_STATE_ACTIVE:
            self->fadingOut = 0;
            break;
        case TASKCORE_STATE_FADE_OUT:
            self->fadingOut = 1;
            break;
        case TASKCORE_STATE_FADED_OUT:
            if (self->abortBeforeFade == 0) {
                self->player->methods->abort(self->player);
            }
            break;
        case STREAMTASK_STATE_SKIPPED:
            self->methods->exit(self);
            break;
    }
}

void StreamTask__SetFrameBound(StreamTask *self, s32 bound) {
    self->frameBound = bound;
    if (bound >= 0) {
        self->frameBound = bound * STREAMTASK_FRAMES_PER_SECOND;
    }
}

void StreamTask__OnPadConfirm(StreamTask *self) {
    GetTaskCoreMethods()->onPadConfirm((TaskCore *)self);
    if (self->skipOnConfirm != 0) {
        self->result = STREAMTASK_RESULT_SKIPPED;
        self->methods->setState(self, STREAMTASK_STATE_SKIPPED);
    }
}

void StreamTask__OnPadPrev(StreamTask *self) {
    GetTaskCoreMethods()->onPadPrev((TaskCore *)self);
}

void StreamTask__OnPadNext(StreamTask *self) {
    GetTaskCoreMethods()->onPadNext((TaskCore *)self);
}

void StreamTask__NoOpSlot88(void) {}

void StreamTask__NoOpSlot8C(void) {}

void StreamTask__Exit(StreamTask *self) {
    if (self->abortBeforeFade != 0) {
        self->player->methods->abort(self->player);
    } else {
        self->methods->setState(self, TASKCORE_STATE_FADE_OUT);
    }
}

void StreamTask__SetKeepActive(StreamTask *self, s32 keepActive) {
    self->keepActive = keepActive;
}

void StreamTask__SetLoopCount(StreamTask *self, s32 count) {
    self->loopCount = count;
}

void StreamTask__SetSkipOnConfirm(StreamTask *self, s32 enable) {
    self->skipOnConfirm = enable;
}

void StreamTask__SetUnkD0(StreamTask *self, s32 value) {
    self->unkD0 = value;
}

void StreamTask__SetAbortBeforeFade(StreamTask *self, s32 enable) {
    self->abortBeforeFade = enable;
}

StreamTaskMethods *GetStreamTaskMethods(void) {
    return &gStreamTaskMethods;
}

/* The method table. A (void *) entry is a function whose declared type
 * differs from its slot's: a method inherited from a parent class and
 * declared on the parent's type, or an empty method declared (void). */

/* StreamTask (include/stream_task.h): TaskCore's table with the movie
 * player's reset, init, onInit, update, state machine and pad handlers, then
 * its five setters. */
StreamTaskMethods gStreamTaskMethods = {
    /* +0x000 header */ STREAMTASK_CLASS_ID,
    /* +0x004 release */ (void *)BasicClass__Release,
    /* +0x008 ctor */ StreamTask__StreamTask,
    /* +0x00C finalize */ StreamTask__Finalize,
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
    /* +0x040 resetCounters */ StreamTask__Reset,
    /* +0x044 init */ (void *)StreamTask__Init,
    /* +0x048 deinit */ (void *)IntermediateBase__Deinit,
    /* +0x04C onInit */ (void *)StreamTask__OnInit,
    /* +0x050 onDeinit */ (void *)TaskCore__OnDeinit,
    /* +0x054 onDrawSystemEvent */ (void *)IntermediateBase__OnDrawSystemEvent,
    /* +0x058 onPadEvent */ (void *)TaskCore__OnPadEvent,
    /* +0x05C update */ StreamTask__Update,
    /* +0x060 setState */ StreamTask__SetState,
    /* +0x064 onStart */ (void *)IntermediateBase__OnStart,
    /* +0x068 onStop */ (void *)IntermediateBase__OnStop,
    /* +0x06C setFrameBound */ StreamTask__SetFrameBound,
    /* +0x070 playSound */ (void *)TaskCore__PlaySound,
    /* +0x074 onPadStart */ (void *)TaskCore__OnPadStart,
    /* +0x078 onPadConfirm */ StreamTask__OnPadConfirm,
    /* +0x07C onPadCancel */ (void *)TaskCore__OnPadCancel,
    /* +0x080 onPadPrev */ StreamTask__OnPadPrev,
    /* +0x084 onPadNext */ StreamTask__OnPadNext,
    /* +0x088 slot88 */ StreamTask__NoOpSlot88,
    /* +0x08C slot8C */ StreamTask__NoOpSlot8C,
    /* +0x090 confirmSlot */ (void *)TaskCore__ConfirmSlot,
    /* +0x094 exit */ StreamTask__Exit,
    /* +0x098 setExitCallback */ (void *)TaskCore__SetExitCallback,
    /* +0x09C setFadeInCallbackEnabled */ (void *)TaskCore__SetFadeInCallbackEnabled,
    /* +0x0A0 setFadeOutCallbackEnabled */ (void *)TaskCore__SetFadeOutCallbackEnabled,
    /* +0x0A4 setColors */ (void *)TaskCore__SetColors,
    /* +0x0A8 setFadeRate */ (void *)TaskCore__SetFadeRate,
    /* +0x0AC tickFadeInCallback */ (void *)TaskCore__TickFadeInCallback,
    /* +0x0B0 tickFadeIn */ (void *)TaskCore__TickFadeIn,
    /* +0x0B4 slotB4 */ NULL,
    /* +0x0B8 slotB8 */ NULL,
    /* +0x0BC slotBC */ NULL,
    /* +0x0C0 tickFadeOutCallback */ (void *)TaskCore__TickFadeOutCallback,
    /* +0x0C4 tickFadeOut */ (void *)TaskCore__TickFadeOut,
    /* +0x0C8 slotC8 */ NULL,
    /* +0x0CC slotCC */ NULL,
    /* +0x0D0 slotD0 */ NULL,
    /* +0x0D4 setSubHandle */ (void *)TaskCore__SetSubHandle,
    /* +0x0D8 setTarget */ (void *)TaskCore__SetTarget,
    /* +0x0DC releaseTarget */ (void *)TaskCore__ReleaseTarget,
    /* +0x0E0 updateSlotElements */ (void *)TaskCore__UpdateSlotElements,
    /* +0x0E4 broadcastToSlots */ (void *)TaskCore__BroadcastToSlots,
    /* +0x0E8 findNextFreeSlot */ (void *)TaskCore__FindNextFreeSlot,
    /* +0x0EC findPrevFreeSlot */ (void *)TaskCore__FindPrevFreeSlot,
    /* +0x0F0 setActiveSlot */ (void *)TaskCore__SetActiveSlot,
    /* +0x0F4 getActiveSlot */ (void *)TaskCore__GetActiveSlot,
    /* +0x0F8 createSlotElements */ (void *)TaskCore__CreateSlotElements,
    /* +0x0FC releaseSlotElements */ (void *)TaskCore__ReleaseSlotElements,
    /* +0x100 refreshSlotView */ (void *)TaskCore__RefreshSlotView,
    /* +0x104 broadcastToSlotElements */ (void *)TaskCore__BroadcastToSlotElements,
    /* +0x108 beginElementScroll */ (void *)TaskCore__BeginElementScroll,
    /* +0x10C commitElementScroll */ (void *)TaskCore__CommitElementScroll,
    /* +0x110 cancelElementScroll */ (void *)TaskCore__CancelElementScroll,
    /* +0x114 advanceSlotCursor */ (void *)TaskCore__AdvanceSlotCursor,
    /* +0x118 retreatSlotCursor */ (void *)TaskCore__RetreatSlotCursor,
    /* +0x11C setSlotCursor */ (void *)TaskCore__SetSlotCursor,
    /* +0x120 getActiveItemCursor */ (void *)TaskCore__GetActiveItemCursor,
    /* +0x124 setKeepActive */ StreamTask__SetKeepActive,
    /* +0x128 setLoopCount */ StreamTask__SetLoopCount,
    /* +0x12C setSkipOnConfirm */ StreamTask__SetSkipOnConfirm,
    /* +0x130 setUnkD0 */ StreamTask__SetUnkD0,
    /* +0x134 setAbortBeforeFade */ StreamTask__SetAbortBeforeFade,
};
