#include "common.h"
#include "code_2c054.h"
#include "VabStreamObj.h"
#include "BgLayer.h"
#include "TileMap.h"
#include "TileAtlas.h"

StreamTask *New_StreamTask(TaskCoreTarget *target, char *soundBankPath, BasicClass *sound, StreamTaskInitData *initData)
{
    StreamTask *self;

    self = BMemPMgrAlloc(0xDC);
    if (self != NULL) {
        Get_vtable_StreamTask()->ctor(self, target, soundBankPath, sound, initData);
        return self;
    }
    return NULL;
}

void StreamTask__StreamTask(StreamTask *self, TaskCoreTarget *target, char *soundBankPath, BasicClass *sound, StreamTaskInitData *initData) {
    Get_vtable_TaskCore()->ctor((TaskCore *)self, target, soundBankPath, sound);
    self->methods = Get_vtable_StreamTask();
    if (initData != NULL) {
        self->initData = *initData;
    } else {
        self->initData = *GetDefaultStreamTaskInitData();
    }
    self->player = New_MoviePlayer((DrawRect *)GetDefaultStreamTaskInitData(), 0, 0);
    self->streamName = 0;
    self->methods->resetCounters(self);
}

void StreamTask__Finalize(StreamTask *self) {
    self->player->methods->release(self->player);
    Get_vtable_TaskCore()->finalize((TaskCore *)self);
}

void StreamTask__Reset(StreamTask *self) {
    self->loopCount = -1;
    self->unkC4 = 0;
    self->skipOnConfirm = 1;
    self->unkD0 = 0;
    self->abortBeforeFade = 1;
}

void StreamTask__Init(StreamTask *self, IntermediateBaseInitArgs *args, s32 streamName, s32 streamGroup, s32 autoPlay) {
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
    if (self->player->methods->play(self->player, (char *)self->streamName, self->streamGroup, self->unkC4, self->loopCount) != 0) {
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
    self->methods->setState(self, 7);
}

void StreamTask__SetState(StreamTask *self, s32 state) {
    Get_vtable_TaskCore()->setState((TaskCore *)self, state);
    switch (state) {
    case 5:
        self->fadingOut = 0;
        break;
    case 7:
        self->fadingOut = 1;
        break;
    case 8:
        if (self->abortBeforeFade == 0) {
            self->player->methods->abort(self->player);
        }
        break;
    case 0x12:
        self->methods->refreshViewValue(self);
        break;
    }
}

void StreamTask__SetFrameBound(StreamTask *self, s32 bound) {
    self->frameBound = bound;
    if (bound >= 0) {
        self->frameBound = bound * 15;
    }
}

void StreamTask__OnPadConfirm(StreamTask *self) {
    Get_vtable_TaskCore()->onPadConfirm((TaskCore *)self);
    if (self->skipOnConfirm != 0) {
        self->result = 2;
        self->methods->setState(self, 0x12);
    }
}

void StreamTask__OnPadPrev(StreamTask *self) {
    Get_vtable_TaskCore()->onPadPrev((TaskCore *)self);
}

void StreamTask__OnPadNext(StreamTask *self) {
    Get_vtable_TaskCore()->onPadNext((TaskCore *)self);
}

void StreamTask__NoOpSlot88(void) {
}

void StreamTask__NoOpSlot8C(void) {
}

void StreamTask__RefreshViewValue(StreamTask *self) {
    if (self->abortBeforeFade != 0) {
        self->player->methods->abort(self->player);
    } else {
        self->methods->setState(self, 7);
    }
}

void StreamTask__SetUnkC4(StreamTask *self, s32 value) {
    self->unkC4 = value;
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

/* The TaskCore allocator: 0xA4 bytes, constructed through its own ctor. */
TaskCore *New_TaskCore(TaskCoreTarget *target, char *soundBankPath, BasicClass *sound)
{
    TaskCore *self;

    self = BMemPMgrAlloc(0xA4);
    if (self != NULL) {
        Get_vtable_TaskCore()->ctor(self, target, soundBankPath, sound);
        return self;
    }
    return NULL;
}

void TaskCore__TaskCore(TaskCore *self, TaskCoreTarget *target, char *soundBankPath, BasicClass *sound) {
    void *tmp; /* the TileAtlas, then the TileMap built over it */
    TaskCoreMethods *core;

    Get_vtable_IntermediateBase()->ctor((IntermediateBase *)self);
    core = Get_vtable_TaskCore();
    self->methods = core;
    core->setTarget(self, target);
    if (soundBankPath != 0) {
        self->sound = (BasicClass *)New_VabStreamObj(soundBankPath);
    } else {
        self->sound = sound;
    }
    self->soundBankPath = soundBankPath;
    self->methods->setSubHandle(self, 0, 0);
    tmp = New_TileAtlas(0);
    self->tileAtlas = tmp;
    tmp = New_TileMap(0, tmp);
    self->tileMap = tmp;
    self->bgLayer = New_BgLayer(tmp, 1);
    self->methods->resetCounters(self);
}

void TaskCore__Finalize(TaskCore *self) {
    self->bgLayer->methods->release(self->bgLayer);
    self->tileMap->methods->release(self->tileMap);
    self->tileAtlas->methods->release(self->tileAtlas);
    if (self->soundBankPath != 0) {
        self->sound->methods->release(self->sound);
    }
    if (self->subHandlePath != 0) {
        self->subHandle->methods->release(self->subHandle);
    }
    self->methods->releaseTarget(self);
    Get_vtable_IntermediateBase()->finalize((IntermediateBase *)self);
}

void TaskCore__Reset(TaskCore *self) {
    TaskCoreMethods *methods = self->methods;
    methods->setFrameBound(self, -1);
    methods->setColors(self, &D_8006E860[0], &D_8006E860[3], &D_8006E860[6]);
    methods->setFadeCallbackEnabled(self, 1);
    methods->setFadeOutCallbackEnabled(self, 1);
    self->fadeRate = 9;
    self->unk28 = 3;
    self->unk2C = 0x12C;
    self->unk30 = 0x40;
    self->viewCallback = NULL;
    self->viewCallbackCtx = NULL;
    self->unk34 = 1;
    self->inputMode = 0;
}

s32 TaskCore__Init(TaskCore *self, IntermediateBaseInitArgs *args, s32 mode) {
    Get_vtable_IntermediateBase()->init((IntermediateBase *)self, args, mode);
    return self->result;
}

/* initArgs->unk0 is reached through this unit's TaskTextObj view, the
 * viewport as a Viewport (include/Viewport.h), both `BasicClass *` in
 * TaskCore.h; bgLayer is a BgLayer (include/BgLayer.h). */
void TaskCore__OnInit(TaskCore *self) {
    Viewport *viewport;
    ViewportMethods *core;

    viewport = (Viewport *)self->viewport;
    core = viewport->methods;
    self->methods->updateSlotElements(self, self->unk14);
    self->bgLayer->methods->attachToParent(self->bgLayer, (Class6B5CC *)self->unk14, NULL);
    if (self->fadeInCallback != 0) {
        self->methods->broadcastToSlots(self, self->baseColor);
        self->bgLayer->methods->setColor(self->bgLayer, 1, (BgLayerRgb *)self->baseColor);
    }
    if (self->subHandle == 0) {
        ((TaskTextObj *)self->initArgs->unk0)->methods->slot78((TaskTextObj *)self->initArgs->unk0, self->baseColor, gDefaultStreamTaskInitData);
    }
    ((TaskTextObj *)self->initArgs->unk0)->methods->slot78((TaskTextObj *)self->initArgs->unk0, self->baseColor, 0);
    core->setOtLength(viewport, self->unk28);
    core->setUnk44(viewport, self->unk2C);
    core->setUnk48(viewport, self->unk30);
    core->attachViewChild(viewport, self->unk14, &D_8006E86C, &D_8006E86C, 0);
    core->initOt(viewport);
    self->result = 0;
}

void TaskCore__OnDeinit(TaskCore *self) {
    Viewport *viewport = (Viewport *)self->viewport;
    viewport->methods->deinitOt(viewport);
    viewport->methods->detachViewChild(viewport);
    self->bgLayer->methods->detachFromParent(self->bgLayer);
    if (self->unk34 != 0) {
        ((TaskTextObj *)self->initArgs->unk0)->methods->slot78((TaskTextObj *)self->initArgs->unk0, self->unk93, 0);
    }
}
