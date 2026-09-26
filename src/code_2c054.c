#include "common.h"
#include "code_2c054.h"
#include "VabStreamObj.h"

StreamTaskObj *New_StreamTask(s32 a1, s32 a2, s32 a3, s32 a4)
{
    StreamTaskObj *self;

    self = BMemPMgrAlloc(0xDC);
    if (self != NULL) {
        Get_vtable_StreamTask()->ctor(self, a1, a2, a3, (StreamTaskInitData *)a4);
        return self;
    }
    return NULL;
}

void StreamTask__StreamTask(StreamTaskObj *self, s32 a1, s32 a2, s32 a3, StreamTaskInitData *a4) {
    Get_vtable_TaskCore()->ctor((TaskCore *)self, (TaskCoreTarget *)a1, (char *)a2, (BasicClass *)a3);
    self->methods = Get_vtable_StreamTask();
    if (a4 != NULL) {
        self->unkA8 = *a4;
    } else {
        self->unkA8 = *GetDefaultStreamTaskInitData();
    }
    self->unkB4 = New_MoviePlayer(GetDefaultStreamTaskInitData(), 0, 0);
    self->unkB8 = 0;
    self->methods->resetCounters(self);
}

void StreamTask__Finalize(StreamTaskObj *self) {
    self->unkB4->methods->slot04(self->unkB4);
    Get_vtable_TaskCore()->finalize((TaskCore *)self);
}

void StreamTask__Reset(StreamTaskObj *self) {
    self->unkC8 = -1;
    self->unkC4 = 0;
    self->unkCC = 1;
    self->unkD0 = 0;
    self->unkD4 = 1;
}

void StreamTask__Init(StreamTaskObj *self, s32 a1, s32 arg2, s32 typeLookup, s32 flag) {
    self->unkB8 = arg2;
    self->unkBC = typeLookup;
    self->unkC0 = flag;
    Get_vtable_TaskCore()->init((TaskCore *)self, (IntermediateBaseInitArgs *)a1, 0);
}

void StreamTask__OnInit(StreamTaskObj *self) {
    /* IntermediateBase's onInit slot names init's (0, 0, 0); TaskCore__OnInit
     * takes self alone, and this up-call passes nothing else. */
    ((void (*)(TaskCore *))Get_vtable_TaskCore()->onInit)((TaskCore *)self);
    self->unkA4 = 0;
    self->unkB4->methods->slot6C(self->unkB4, self->unkC0);
    if (self->unkB4->methods->slot40(self->unkB4, self->unkB8, self->unkBC, self->unkC4, self->unkC8) != 0) {
        self->methods->setFrameBound(self, 0);
    }
}

void StreamTask__Update(StreamTaskObj *self, s32 a1, s32 a2) {
    Get_vtable_TaskCore()->update((TaskCore *)self, (BasicClass *)a1, a2);
    if (self->unkA4 != 0) {
        return;
    }
    self->unkA4 = self->unkB4->methods->slot48(self->unkB4);
    if (self->unkA4 == 0) {
        return;
    }
    if (self->unkD8 != 0) {
        return;
    }
    self->methods->setState(self, 7);
}

void StreamTask__SetState(StreamTaskObj *self, s32 a1) {
    Get_vtable_TaskCore()->setState((TaskCore *)self, a1);
    switch (a1) {
    case 5:
        self->unkD8 = 0;
        break;
    case 7:
        self->unkD8 = 1;
        break;
    case 8:
        if (self->unkD4 == 0) {
            self->unkB4->methods->slot4C(self->unkB4);
        }
        break;
    case 0x12:
        self->methods->refreshViewValue(self);
        break;
    }
}

void StreamTask__SetFrameBound(StreamTaskObj *self, s32 a1) {
    self->frameBound = a1;
    if (a1 >= 0) {
        self->frameBound = a1 * 15;
    }
}

void StreamTask__OnPadConfirm(StreamTaskObj *self) {
    Get_vtable_TaskCore()->onPadConfirm((TaskCore *)self);
    if (self->unkCC != 0) {
        self->result = 2;
        self->methods->setState(self, 0x12);
    }
}

void StreamTask__OnPadPrev(StreamTaskObj *self) {
    Get_vtable_TaskCore()->onPadPrev((TaskCore *)self);
}

void StreamTask__OnPadNext(StreamTaskObj *self) {
    Get_vtable_TaskCore()->onPadNext((TaskCore *)self);
}

void StreamTask__NoOpSlot88(void) {
}

void StreamTask__NoOpSlot8C(void) {
}

void StreamTask__RefreshViewValue(StreamTaskObj *self) {
    if (self->unkD4 != 0) {
        self->unkB4->methods->slot4C(self->unkB4);
    } else {
        self->methods->setState(self, 7);
    }
}

void StreamTask__SetUnkC4(StreamTaskObj *self, s32 a1) {
    self->unkC4 = a1;
}

void StreamTask__SetLoopCount(StreamTaskObj *self, s32 a1) {
    self->unkC8 = a1;
}

void StreamTask__SetSkipOnConfirm(StreamTaskObj *self, s32 a1) {
    self->unkCC = a1;
}

void StreamTask__SetUnkD0(StreamTaskObj *self, s32 a1) {
    self->unkD0 = a1;
}

void StreamTask__SetAbortBeforeFade(StreamTaskObj *self, s32 a1) {
    self->unkD4 = a1;
}

StreamTaskObjMethods *Get_vtable_StreamTask(void) {
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
    StreamTaskUnkB4Obj *tmp;
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
    self->tileAtlas = (BasicClass *)tmp;
    tmp = New_TileMap(0, tmp);
    self->tileMap = (BasicClass *)tmp;
    self->bgLayer = (BasicClass *)New_BgLayer(tmp, 1);
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
 * viewport as a Viewport (include/Viewport.h) and bgLayer through
 * StreamTaskUnk78Obj (BgLayer): TaskCore.h types all three BasicClass *. */
void TaskCore__OnInit(TaskCore *self) {
    Viewport *viewport;
    ViewportMethods *core;

    viewport = (Viewport *)self->viewport;
    core = viewport->methods;
    self->methods->updateSlotElements(self, self->unk14);
    ((StreamTaskUnk78Obj *)self->bgLayer)->methods->slot4C((StreamTaskUnk78Obj *)self->bgLayer, self->unk14, 0);
    if (self->fadeInCallback != 0) {
        self->methods->broadcastToSlots(self, self->baseColor);
        ((StreamTaskUnk78Obj *)self->bgLayer)->methods->slotB8((StreamTaskUnk78Obj *)self->bgLayer, 1, self->baseColor);
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
    ((StreamTaskUnk78Obj *)self->bgLayer)->methods->slot50((StreamTaskUnk78Obj *)self->bgLayer);
    if (self->unk34 != 0) {
        ((TaskTextObj *)self->initArgs->unk0)->methods->slot78((TaskTextObj *)self->initArgs->unk0, self->unk93, 0);
    }
}
