#include "common.h"
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include "class_39e08.h"
#include "VabStreamObj.h"
#include "NodeGuardedViewport.h"
#include "StageMap.h"
#include "WBgm.h"
#include "TimImage.h"
#include "FrameClock.h"
#include "DreamSys.h"
#include "LinkResource.h"
#include "ObjM.h"

/* The viewpoint and view-reference vectors DayTask__OnInit hands the
 * viewport's attachViewChild: (0, -1200, 0) and (0, -1200, 10000), the data
 * right before gTimedTaskMethods. */
extern LongVec3 sDayViewPoint;
extern LongVec3 sDayViewRef;

DayTask *New_DayTask(IntermediateBaseInitArgs *initArgs, DreamSys *dreamSys, s32 arg3) {
    DayTask *self;

    self = BMemPMgrAlloc(0x50);
    if (self != NULL) {
        GetDayTaskMethods()->ctor(self, initArgs, dreamSys, arg3);
        return self;
    }
    return NULL;
}

void DayTask__DayTask(DayTask *self, IntermediateBaseInitArgs *initArgs, DreamSys *dreamSys, s32 arg3) {
    /* MATCHING: mode is never set, but a bare ResourceSource shrinks the
     * frame by 8. */
    ResourceRequest req;
    s32 tmp;

    GetTimedTaskMethods()->ctor((TimedTask *)self, (char *)GetSoundEffectDir(0), 0);
    self->methods = GetDayTaskMethods();
    InitDreamAux();
    self->etcTim = New_TimImage((char *)sEtcTimPath);
    ((TimImageUploadFn)self->etcTim->methods->processBuffer)(self->etcTim);
    self->etcTim->methods->freeBuffer(self->etcTim);
    req.src.buffer = NULL;
    req.src.name = (char *)sDreamerTmdPath;
    self->dreamerTmd = New_LinkResource(&req.src);
    tmp = PickWeeklyGroup(0);
    self->bgm = New_WBgm((char *)tmp, NULL, 1);
    RegisterRecordTableFiles(1);
    SetActiveDataSourceDriverMode((u32)arg3 < 1, 1, 1);
    self->initArgs = initArgs;
    initArgs->viewport = (BasicClass *)New_NodeGuardedViewport();
    initArgs->frameClock = (BasicClass *)New_FrameClock();
    initArgs->lightRig = (BasicClass *)New_StageMap(NULL, 1);
    self->dreamSys = dreamSys;
    self->methods->addChild(self, (BasicClass *)dreamSys);
    dreamSys->methods->setSoundObj(dreamSys, (s32)self->sound);
    dreamSys->methods->slot114(dreamSys, (s32)self->etcTim);
    self->methods->resetCounters(self);
}

void DayTask__Finalize(DayTask *self) {
    IntermediateBaseInitArgs *o = self->initArgs;
    BasicClass *g;

    self->methods->removeChild(self, (BasicClass *)self->dreamSys);
    g = o->lightRig;
    o->lightRig = g->methods->release(g);
    g = o->frameClock;
    o->frameClock = g->methods->release(g);
    g = o->viewport;
    o->viewport = g->methods->release(g);
    self->bgm->methods->release(self->bgm);
    self->dreamerTmd->methods->release(self->dreamerTmd);
    self->etcTim->methods->release(self->etcTim);
    TickDreamAuxSlots();
    GetTimedTaskMethods()->finalize((TimedTask *)self);
}

void DayTask__OnNotify(DayTask *self, BasicClass *sender, s32 event) {
    s32 tag;

    GetTimedTaskMethods()->onNotify((TimedTask *)self, sender, event);
    tag = sender->methods->header;
    if ((tag & 0xFFFF) == 0x1F34) {
        self->methods->onDreamSysNotify(self, sender, event);
    } else if ((tag & 0xFFFFF) == 0x2F230) {
        self->methods->onObjMNotify(self, sender, event);
    }
}

void DayTask__ResetPhase(DayTask *self) {
    self->phase = 0;
}

s32 DayTask__Init(DayTask *self) {
    DreamSys *sub = self->dreamSys;

    sub->methods->addChild(sub, self->initArgs->pad);
    sub->methods->addChild(sub, self->initArgs->frameClock);
    sub->methods->setViewport(sub, (Viewport *)self->initArgs->viewport);
    return GetTimedTaskMethods()->init((TimedTask *)self, self->initArgs, 0);
}

void DayTask__Deinit(DayTask *self) {
    DreamSys *sub = self->dreamSys;

    GetTimedTaskMethods()->deinit((TimedTask *)self);
    sub->methods->setViewport(sub, 0);
    sub->methods->removeChild(sub, self->initArgs->pad);
    sub->methods->removeChild(sub, self->unk10);
}

void DayTask__OnInit(DayTask *self) {
    SubObjE *obj;
    Viewport *vp;
    SceneNode *ret;
    ViewportSize *size;

    obj = (SubObjE *)self->initArgs->drawSystem;
    vp = (Viewport *)self->viewport;
    size = obj->methods->slot7C(obj, 0);
    vp->methods->setScreenSize(vp, size);
    ret = vp->methods->getFadeBox(vp);
    ret->methods->setDisplay(ret, 1);
    vp->methods->setUnk44(vp, 0x4B0);
    vp->methods->attachViewChild(vp, (BasicClass *)self->dreamSys, &sDayViewPoint, &sDayViewRef, 0);
    vp->methods->initOt(vp);
    self->phase = 1;
}

void DayTask__OnDeinit(DayTask *self) {
    Viewport *vp = (Viewport *)self->viewport;

    vp->methods->deinitOt(vp);
    vp->methods->detachViewChild(vp);
}

void DayTask__AdvancePhase(DayTask *self, BasicClass *sender, s32 event) {
    s32 result;

    GetTimedTaskMethods()->onTag1Notify((TimedTask *)self, sender, event);
    if (event == 2 && self->phase != event) {
        switch (self->phase) {
            case 1:
                result = self->dreamSys->methods->startDay(self->dreamSys);
                if (result < 0) {
                    self->dreamSys->methods->endDay(self->dreamSys, 0);
                    self->result = event;
                    self->methods->setState(self, 3);
                    return;
                }
                DayTask__StartObjM(self, result);
                break;
            case 2:
                break;
            case 3:
                self->objM->methods->deinit(self->objM);
                self->objM->methods->release(self->objM);
                result = self->dreamSys->methods->getCurrentStage(self->dreamSys);
                DayTask__StartObjM(self, result);
                break;
        }
    }
}

void DayTask__StartObjM(DayTask *self, s32 stage) {
    self->objM = New_ObjM(self->sound, self->bgm, self->etcTim, self->dreamerTmd, stage);
    self->methods->addChild(self, (BasicClass *)self->objM);
    self->objM->methods->init(self->objM, self->initArgs, (s32)self->dreamSys);
    self->phase = 2;
}

void DayTask__OnState4(void) {}

void DayTask__OnDreamSysNotify(void) {}

void DayTask__OnObjMNotify(DayTask *self, BasicClass *sender, s32 event) {
    CinematicCall pos;
    s32 result;

    switch (event) {
        case 4:
            self->objM->methods->deinit(self->objM);
            self->objM->methods->release(self->objM);
            result = self->dreamSys->methods->endDay(self->dreamSys, 0);
            if (result == 0) {
                pos = self->dreamSys->methods->getCinematic(self->dreamSys);
                self->result = pos.entry < 0 ? 1 : 2;
            } else {
                self->result = 3;
            }
            self->methods->setState(self, 3);
            break;
        case 5:
        case 6:
        case 7:
        case 8:
        case 0xA:
            self->phase = 3;
            break;
        case 0xC:
        case 0xD:
            self->objM->methods->deinit(self->objM);
            self->objM->methods->release(self->objM);
            self->dreamSys->methods->endDay(self->dreamSys, event != 0xC ? 2 : 1);
            self->result = 3;
            self->methods->setState(self, 3);
            break;
    }
}

DayTaskMethods *GetDayTaskMethods(void) {
    return &gDayTaskMethods;
}

/* Sony's, from the still-uncarved psyq_39094 SDK segment
 * (asm/psyq_39094.s): `if (out != NULL) *out = 0x230; return &gRecordTable;`
 * -- an unconditional out-param write (the address passed here is always a
 * stack address, never NULL) plus a fixed .data address, unrelated to the
 * write. Declared locally per CLAUDE.md's rule against writing C for
 * SDK-owned code. */
extern void *GetRecordTable(s32 *out);
extern s32 RegisterFileTableEntries(void *arg0, s32 arg1);

extern s32 sRecordRegisterCalls;
extern s32 sRecordFirstBatchCount;

s32 RegisterRecordTableFiles(s32 arg0) {
    s32 local;
    void *obj;
    s32 prev;
    s32 result;

    obj = GetRecordTable(&local);
    prev = sRecordRegisterCalls;
    sRecordRegisterCalls = prev + 1;

    switch (prev + 1) {
        case 1:
            if (arg0 != 0) {
                sRecordRegisterCalls = prev + 2;
            } else {
                local = local / 2;
                sRecordFirstBatchCount = local;
            }
            break;
        case 2:
            local = local - sRecordFirstBatchCount;
            break;
        default:
            local = 0;
            break;
    }

    while ((result = RegisterFileTableEntries(obj, local)) == 0) {
    }
    return result;
}

TimedTask *New_TimedTask(char *soundBankPath, BasicClass *sound) {
    TimedTask *self;

    self = BMemPMgrAlloc(0x38);
    if (self != NULL) {
        GetTimedTaskMethods()->ctor(self, soundBankPath, sound);
        return self;
    }
    return NULL;
}

void TimedTask__TimedTask(TimedTask *self, char *soundBankPath, BasicClass *sound) {
    Get_vtable_IntermediateBase()->ctor((IntermediateBase *)self);
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
    Get_vtable_IntermediateBase()->finalize((IntermediateBase *)self);
}

void TimedTask__CancelTimeout(TimedTask *self) {
    self->methods->setTimeout(self, -1);
}

s32 TimedTask__Init(TimedTask *self, IntermediateBaseInitArgs *args, s32 mode) {
    self->result = 0;
    Get_vtable_IntermediateBase()->init((IntermediateBase *)self, args, mode);
    return self->result;
}

void TimedTask__Deinit(TimedTask *self) {
    Get_vtable_IntermediateBase()->deinit((IntermediateBase *)self);
}

void TimedTask__NoOpSlot58(void) {}

void TimedTask__CheckTimeout(TimedTask *self, BasicClass *sender, s32 event) {
    Get_vtable_IntermediateBase()->update((IntermediateBase *)self, sender, event);
    if ((u32)self->frameCounter > (u32)self->timeoutFrames) {
        self->methods->setState(self, 4);
    }
}

void TimedTask__SetState(TimedTask *self, s32 state) {
    Get_vtable_IntermediateBase()->setState((IntermediateBase *)self, state);
    if (state == 4) {
        self->result = 1;
        self->methods->onState4(self);
    }
}

void TimedTask__SetTimeout(TimedTask *self, s32 timeout) {
    self->timeoutFrames = (timeout < 0) ? timeout : timeout * 20;
}
