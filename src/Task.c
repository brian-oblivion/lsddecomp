/*
 * StreamTask, whole, and the first seven methods of its parent TaskCore.
 *
 * StreamTask (include/StreamTask.h) plays one movie stream through a
 * MoviePlayer inside TaskCore's fade and state machine: its allocator, ctor,
 * every override, its five setters and its table getter are all here.
 * TaskCore (include/TaskCore.h) is the base of the game's menu and screen
 * tasks; this file holds its allocator, ctor, finalize, resetCounters, init
 * and the onInit/onDeinit hooks, which build and tear down the TileAtlas ->
 * TileMap -> BgLayer chain, clear the screen and configure the viewport. The
 * rest of TaskCore is in TaskViewport.c, the task classes' second file.
 *
 * Edges (track 8, round 100, tools/tuboundary.py). The start edge is
 * libgs/gs_122, a placed Sony object. The end edge, to TaskViewport.c, is
 * not one: no rodata crossing, no forced boundary, and "probably one file
 * with code_2c054" on single-user data (0x8006e86c >= 0x8006e730), with
 * TaskCore's methods on both sides. The two are one file, and are not
 * merged only because `unitfile.py merge` refuses: TaskViewport owns the
 * `.rodata` line at 0x1890 (TaskCore's two jump tables) and this unit owns
 * none, and the fix it names, renaming that yaml line by hand, is outside a
 * files runner's rule. Parked for the head; TaskViewport.c's banner has the
 * rest. Named Task because the merged file is the task classes'; not
 * StreamTask or TaskCore, whose headers exist (this unit's header would
 * move onto one).
 */
#include "common.h"
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include "Task.h"
#include "BMemPMgr.h"
#include "VabStreamObj.h"
#include "BgLayer.h"
#include "TileMap.h"
#include "TileAtlas.h"

StreamTask *New_StreamTask(TaskCoreTarget *target, char *soundBankPath, BasicClass *sound,
                           DrawRect *initData) {
    StreamTask *self;

    self = BMemPMgrAlloc(sizeof(StreamTask));
    if (self != NULL) {
        Get_vtable_StreamTask()->ctor(self, target, soundBankPath, sound, initData);
        return self;
    }
    return NULL;
}

void StreamTask__StreamTask(StreamTask *self, TaskCoreTarget *target, char *soundBankPath,
                            BasicClass *sound, DrawRect *initData) {
    Get_vtable_TaskCore()->ctor((TaskCore *)self, target, soundBankPath, sound);
    self->methods = Get_vtable_StreamTask();
    if (initData != NULL) {
        self->initData = *initData;
    } else {
        self->initData = *GetDefaultMovieFrame();
    }
    self->player = New_MoviePlayer(GetDefaultMovieFrame(), 0, 0);
    self->streamName = 0;
    self->methods->resetCounters(self);
}

void StreamTask__Finalize(StreamTask *self) {
    self->player->methods->release(self->player);
    Get_vtable_TaskCore()->finalize((TaskCore *)self);
}

void StreamTask__Reset(StreamTask *self) {
    self->loopCount = -1;
    self->keepActive = 0;
    self->skipOnConfirm = 1;
    self->unkD0 = 0;
    self->abortBeforeFade = 1;
}

void StreamTask__Init(StreamTask *self, IntermediateBaseInitArgs *args, s32 streamName,
                      s32 streamGroup, s32 autoPlay) {
    self->streamName = streamName;
    self->streamGroup = streamGroup;
    self->autoPlay = autoPlay;
    Get_vtable_TaskCore()->init((TaskCore *)self, args, 0);
}

void StreamTask__OnInit(StreamTask *self) {
    /* IntermediateBase's onInit slot names init's (0, 0, 0); TaskCore__OnInit
     * takes self alone, and this up-call passes nothing else. */
    ((void (*)(TaskCore *))Get_vtable_TaskCore()->onInit)((TaskCore *)self);
    self->playDone = 0;
    self->player->methods->setAutoPlay(self->player, self->autoPlay);
    if (self->player->methods->play(self->player, (char *)self->streamName, self->streamGroup,
                                    self->keepActive, self->loopCount) != 0) {
        self->methods->setFrameBound(self, 0);
    }
}

void StreamTask__Update(StreamTask *self, BasicClass *sender, s32 event) {
    Get_vtable_TaskCore()->update((TaskCore *)self, sender, event);
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
    Get_vtable_TaskCore()->setState((TaskCore *)self, state);
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
            self->methods->refreshViewValue(self);
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
    Get_vtable_TaskCore()->onPadConfirm((TaskCore *)self);
    if (self->skipOnConfirm != 0) {
        self->result = STREAMTASK_RESULT_SKIPPED;
        self->methods->setState(self, STREAMTASK_STATE_SKIPPED);
    }
}

void StreamTask__OnPadPrev(StreamTask *self) {
    Get_vtable_TaskCore()->onPadPrev((TaskCore *)self);
}

void StreamTask__OnPadNext(StreamTask *self) {
    Get_vtable_TaskCore()->onPadNext((TaskCore *)self);
}

void StreamTask__NoOpSlot88(void) {}

void StreamTask__NoOpSlot8C(void) {}

void StreamTask__RefreshViewValue(StreamTask *self) {
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

StreamTaskMethods *Get_vtable_StreamTask(void) {
    return &gStreamTaskMethods;
}

TaskCore *New_TaskCore(TaskCoreTarget *target, char *soundBankPath, BasicClass *sound) {
    TaskCore *self;

    self = BMemPMgrAlloc(sizeof(TaskCore));
    if (self != NULL) {
        Get_vtable_TaskCore()->ctor(self, target, soundBankPath, sound);
        return self;
    }
    return NULL;
}

void TaskCore__TaskCore(TaskCore *self, TaskCoreTarget *target, char *soundBankPath, BasicClass *sound) {
    struct TileAtlas *atlas;
    struct TileMap *tileMap;
    TaskCoreMethods *methods;

    Get_vtable_IntermediateBase()->ctor((IntermediateBase *)self);
    /* MATCHING: one Get_vtable_TaskCore() call; a second one for setTarget adds a jal. */
    methods = Get_vtable_TaskCore();
    self->methods = methods;
    methods->setTarget(self, target);
    if (soundBankPath != NULL) {
        self->sound = (BasicClass *)New_VabStreamObj(soundBankPath);
    } else {
        self->sound = sound;
    }
    self->soundBankPath = soundBankPath;
    self->methods->setSubHandle(self, NULL, NULL);
    atlas = New_TileAtlas(0);
    self->tileAtlas = atlas;
    tileMap = New_TileMap(0, atlas);
    self->tileMap = tileMap;
    self->bgLayer = New_BgLayer(tileMap, 1);
    self->methods->resetCounters(self);
}

void TaskCore__Finalize(TaskCore *self) {
    self->bgLayer->methods->release(self->bgLayer);
    self->tileMap->methods->release(self->tileMap);
    self->tileAtlas->methods->release(self->tileAtlas);
    if (self->soundBankPath != NULL) {
        self->sound->methods->release(self->sound);
    }
    if (self->subHandlePath != NULL) {
        self->subHandle->methods->release(self->subHandle);
    }
    self->methods->releaseTarget(self);
    Get_vtable_IntermediateBase()->finalize((IntermediateBase *)self);
}

void TaskCore__Reset(TaskCore *self) {
    /* MATCHING: self->methods reloaded after each call is a word longer. */
    TaskCoreMethods *methods = self->methods;
    methods->setFrameBound(self, -1);
    methods->setColors(self, sTaskCoreDefaultColors[0], sTaskCoreDefaultColors[1],
                       sTaskCoreDefaultColors[2]);
    methods->setFadeCallbackEnabled(self, 1);
    methods->setFadeOutCallbackEnabled(self, 1);
    self->fadeRate = 9;
    self->otLength = 3;
    self->unk2C = 300;
    self->packetSize = 64;
    self->viewCallback = NULL;
    self->viewCallbackCtx = NULL;
    self->unk34 = 1;
    self->inputMode = TASKCORE_INPUT_NONE;
}

s32 TaskCore__Init(TaskCore *self, IntermediateBaseInitArgs *args, s32 mode) {
    Get_vtable_IntermediateBase()->init((IntermediateBase *)self, args, mode);
    return self->result;
}

/* Hangs the slot widgets and the BgLayer under the light rig (unk14), sets the
 * fade-in colour, clears the default movie frame (no sub handle) and then the
 * screen to baseColor, and configures and opens the viewport's OT.
 * initArgs->drawSystem and the viewport are `BasicClass *` fields, cast to
 * their classes, DrawSystem and Viewport. */
void TaskCore__OnInit(TaskCore *self) {
    Viewport *viewport;
    ViewportMethods *viewportMethods;

    /* MATCHING: retail loads both before the first call and keeps them to the end. */
    viewport = (Viewport *)self->viewport;
    viewportMethods = viewport->methods;
    self->methods->updateSlotElements(self, self->unk14);
    self->bgLayer->methods->attachToParent(self->bgLayer, (SceneNode *)self->unk14, NULL);
    if (self->fadeInCallback != NULL) {
        self->methods->broadcastToSlots(self, self->baseColor);
        self->bgLayer->methods->setColor(self->bgLayer, 1, (BgLayerRgb *)self->baseColor);
    }
    if (self->subHandle == NULL) {
        ((DrawSystem *)self->initArgs->drawSystem)
            ->methods->clearImage((DrawSystem *)self->initArgs->drawSystem, self->baseColor,
                                  &gDefaultMovieFrame);
    }
    ((DrawSystem *)self->initArgs->drawSystem)
        ->methods->clearImage((DrawSystem *)self->initArgs->drawSystem, self->baseColor, NULL);
    viewportMethods->setOtLength(viewport, self->otLength);
    viewportMethods->setMaxPackets(viewport, self->unk2C);
    viewportMethods->setPacketSize(viewport, self->packetSize);
    viewportMethods->attachViewChild(viewport, self->unk14, &sTaskCoreViewOrigin,
                                     &sTaskCoreViewOrigin, NULL);
    viewportMethods->initOt(viewport);
    self->result = 0;
}

/* Closes the viewport's OT, detaches the view and the BgLayer, and clears the
 * screen to unk93 while unk34 is set. */
void TaskCore__OnDeinit(TaskCore *self) {
    /* MATCHING: without the local, self->viewport is reloaded and the frame shrinks. */
    Viewport *viewport = (Viewport *)self->viewport;
    viewport->methods->deinitOt(viewport);
    viewport->methods->detachViewChild(viewport);
    self->bgLayer->methods->detachFromParent(self->bgLayer);
    if (self->unk34 != 0) {
        ((DrawSystem *)self->initArgs->drawSystem)
            ->methods->clearImage((DrawSystem *)self->initArgs->drawSystem, self->unk93, NULL);
    }
}
