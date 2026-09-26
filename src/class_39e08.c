#include "common.h"
#include "class_39e08.h"
#include "VabStreamObj.h"
#include "Class869D8.h"
#include "Class866E8.h"
#include "WBgm.h"
#include "TimImage.h"
#include "FrameClock.h"
#include "DreamSys.h"
#include "LinkResource.h"
#include "ObjM.h"

/* The viewpoint and view-reference vectors Class865C8__OnInit hands the
 * viewport's attachViewChild: (0, -1200, 0) and (0, -1200, 10000), the data
 * right before gClass86668Methods. */
extern LongVec3 D_80086650;
extern LongVec3 D_8008665C;

Class865C8 *New_Class865C8(IntermediateBaseInitArgs *initArgs, DreamSys *dreamSys, s32 arg3) {
    Class865C8 *self;

    self = BMemPMgrAlloc(0x50);
    if (self != NULL) {
        GetClass865C8Methods()->ctor(self, initArgs, dreamSys, arg3);
        return self;
    }
    return NULL;
}

void Class865C8__Class865C8(Class865C8 *self, IntermediateBaseInitArgs *initArgs,
                            DreamSys *dreamSys, s32 arg3) {
    LoadRequest req;
    s32 tmp;

    GetClass86668Methods()->ctor((Class86668 *)self, (char *)GetSoundEffectDir(0), 0);
    self->methods = GetClass865C8Methods();
    InitDreamAux();
    self->etcTim = New_TimImage((char *)D_800113EC);
    ((TimImageUploadFn)self->etcTim->methods->slot78)(self->etcTim);
    self->etcTim->methods->freeBuffer(self->etcTim);
    req.type = 0;
    req.path = D_800113F8;
    self->dreamerTmd = New_LinkResource((struct Src6F240 *)&req);
    tmp = PickWeeklyGroup(0);
    self->bgm = New_WBgm((char *)tmp, NULL, 1);
    func_8004A070(1);
    SetActiveDataSourceDriverMode((u32)arg3 < 1, 1, 1);
    self->initArgs = initArgs;
    initArgs->viewport = (BasicClass *)New_Class869D8();
    initArgs->unk8 = (BasicClass *)New_FrameClock();
    initArgs->unkC = (BasicClass *)New_Class866E8(NULL, 1);
    self->dreamSys = dreamSys;
    self->methods->addChild(self, (BasicClass *)dreamSys);
    dreamSys->methods->setSoundObj(dreamSys, (s32)self->sound);
    dreamSys->methods->slot114(dreamSys, (s32)self->etcTim);
    self->methods->resetCounters(self);
}

void Class865C8__Finalize(Class865C8 *self) {
    IntermediateBaseInitArgs *o = self->initArgs;
    BasicClass *g;

    self->methods->removeChild(self, (BasicClass *)self->dreamSys);
    g = o->unkC;
    o->unkC = g->methods->release(g);
    g = o->unk8;
    o->unk8 = g->methods->release(g);
    g = o->viewport;
    o->viewport = g->methods->release(g);
    self->bgm->methods->release(self->bgm);
    self->dreamerTmd->methods->release(self->dreamerTmd);
    self->etcTim->methods->release(self->etcTim);
    TickDreamAuxSlots();
    GetClass86668Methods()->finalize((Class86668 *)self);
}

void Class865C8__OnNotify(Class865C8 *self, BasicClass *sender, s32 event) {
    s32 tag;

    GetClass86668Methods()->onNotify((Class86668 *)self, sender, event);
    tag = sender->methods->header;
    if ((tag & 0xFFFF) == 0x1F34) {
        self->methods->onDreamSysNotify(self, sender, event);
    } else if ((tag & 0xFFFFF) == 0x2F230) {
        self->methods->onObjMNotify(self, sender, event);
    }
}

void Class865C8__ResetPhase(Class865C8 *self) {
    self->phase = 0;
}

s32 Class865C8__Init(Class865C8 *self) {
    DreamSys *sub = self->dreamSys;

    sub->methods->addChild(sub, self->initArgs->unk4);
    sub->methods->addChild(sub, self->initArgs->unk8);
    sub->methods->setViewport(sub, (Viewport *)self->initArgs->viewport);
    return GetClass86668Methods()->init((Class86668 *)self, self->initArgs, 0);
}

void Class865C8__Deinit(Class865C8 *self) {
    DreamSys *sub = self->dreamSys;

    GetClass86668Methods()->deinit((Class86668 *)self);
    sub->methods->setViewport(sub, 0);
    sub->methods->removeChild(sub, self->initArgs->unk4);
    sub->methods->removeChild(sub, self->unk10);
}

void Class865C8__OnInit(Class865C8 *self) {
    SubObjE *obj;
    Viewport *vp;
    SceneNode *ret;
    ViewportSize *size;

    obj = (SubObjE *)self->initArgs->unk0;
    vp = (Viewport *)self->viewport;
    size = obj->methods->slot7C(obj, 0);
    vp->methods->setScreenSize(vp, size);
    ret = vp->methods->getSubHandle(vp);
    ret->methods->setDisplay(ret, 1);
    vp->methods->setUnk44(vp, 0x4B0);
    vp->methods->attachViewChild(vp, (BasicClass *)self->dreamSys, &D_80086650, &D_8008665C, 0);
    vp->methods->initOt(vp);
    self->phase = 1;
}

void Class865C8__OnDeinit(Class865C8 *self) {
    Viewport *vp = (Viewport *)self->viewport;

    vp->methods->deinitOt(vp);
    vp->methods->detachViewChild(vp);
}

void Class865C8__AdvancePhase(Class865C8 *self, BasicClass *sender, s32 event) {
    s32 result;

    GetClass86668Methods()->onTag1Notify((Class86668 *)self, sender, event);
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
                Class865C8__StartObjM(self, result);
                break;
            case 2:
                break;
            case 3:
                self->objM->methods->deinit(self->objM);
                self->objM->methods->release(self->objM);
                result = self->dreamSys->methods->getCurrentStage(self->dreamSys);
                Class865C8__StartObjM(self, result);
                break;
        }
    }
}

void Class865C8__StartObjM(Class865C8 *self, s32 stage) {
    self->objM = New_ObjM(self->sound, self->bgm, self->etcTim, self->dreamerTmd, stage);
    self->methods->addChild(self, (BasicClass *)self->objM);
    self->objM->methods->init(self->objM, self->initArgs, (s32)self->dreamSys);
    self->phase = 2;
}

void Class865C8__OnState4(void) {}

void Class865C8__OnDreamSysNotify(void) {}

void Class865C8__OnObjMNotify(Class865C8 *self, BasicClass *sender, s32 event) {
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

Class865C8Methods *GetClass865C8Methods(void) {
    return &gClass865C8Methods;
}

/* Sony's, from the still-uncarved psyq_39094 SDK segment
 * (asm/psyq_39094.s): `if (out != NULL) *out = 0x230; return &gRecordTable;`
 * -- an unconditional out-param write (the address passed here is always a
 * stack address, never NULL) plus a fixed .data address, unrelated to the
 * write. Declared locally per CLAUDE.md's rule against writing C for
 * SDK-owned code. */
extern void *GetRecordTable(s32 *out);
extern s32 RegisterFileTableEntries(void *arg0, s32 arg1);

extern s32 D_8008A978;
extern s32 D_8008A97C;

s32 func_8004A070(s32 arg0) {
    s32 local;
    void *obj;
    s32 prev;
    s32 result;

    obj = GetRecordTable(&local);
    prev = D_8008A978;
    D_8008A978 = prev + 1;

    switch (prev + 1) {
        case 1:
            if (arg0 != 0) {
                D_8008A978 = prev + 2;
            } else {
                local = local / 2;
                D_8008A97C = local;
            }
            break;
        case 2:
            local = local - D_8008A97C;
            break;
        default:
            local = 0;
            break;
    }

    while ((result = RegisterFileTableEntries(obj, local)) == 0) {
    }
    return result;
}

Class86668 *New_Class86668(char *soundBankPath, BasicClass *sound) {
    Class86668 *self;

    self = BMemPMgrAlloc(0x38);
    if (self != NULL) {
        GetClass86668Methods()->ctor(self, soundBankPath, sound);
        return self;
    }
    return NULL;
}

void Class86668__Class86668(Class86668 *self, char *soundBankPath, BasicClass *sound) {
    Get_vtable_IntermediateBase()->ctor((IntermediateBase *)self);
    self->methods = GetClass86668Methods();
    if (soundBankPath != NULL) {
        self->sound = (BasicClass *)New_VabStreamObj(soundBankPath);
    } else {
        self->sound = sound;
    }
    self->soundBankPath = soundBankPath;
    self->methods->resetCounters(self);
}

void Class86668__Finalize(Class86668 *self) {
    if (self->soundBankPath != NULL) {
        self->sound->methods->release(self->sound);
    }
    Get_vtable_IntermediateBase()->finalize((IntermediateBase *)self);
}

void Class86668__CancelTimeout(Class86668 *self) {
    self->methods->setTimeout(self, -1);
}

s32 Class86668__Init(Class86668 *self, IntermediateBaseInitArgs *args, s32 mode) {
    self->result = 0;
    Get_vtable_IntermediateBase()->init((IntermediateBase *)self, args, mode);
    return self->result;
}

void Class86668__Deinit(Class86668 *self) {
    Get_vtable_IntermediateBase()->deinit((IntermediateBase *)self);
}

void Class86668__NoOpSlot58(void) {}

void Class86668__CheckTimeout(Class86668 *self, BasicClass *sender, s32 event) {
    Get_vtable_IntermediateBase()->update((IntermediateBase *)self, sender, event);
    if ((u32)self->frameCounter > (u32)self->timeoutFrames) {
        self->methods->setState(self, 4);
    }
}

void Class86668__SetState(Class86668 *self, s32 state) {
    Get_vtable_IntermediateBase()->setState((IntermediateBase *)self, state);
    if (state == 4) {
        self->result = 1;
        self->methods->onState4(self);
    }
}

void Class86668__SetTimeout(Class86668 *self, s32 timeout) {
    self->timeoutFrames = (timeout < 0) ? timeout : timeout * 20;
}
