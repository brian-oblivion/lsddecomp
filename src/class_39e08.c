#include "common.h"
#include "class_39e08.h"

Obj865C8 *New_Obj865C8(Obj0C *arg1, SubObjD *arg2, s32 arg3)
{
    Obj865C8 *self;

    self = BMemPMgrAlloc(0x50);
    if (self != NULL) {
        GetObj865C8Methods()->ctor(self, arg1, arg2, arg3);
        return self;
    }
    return NULL;
}

void Obj865C8__Obj865C8(Obj865C8 *self, Obj0C *arg1, SubObjD *arg2, s32 arg3) {
    LoadRequest req;
    s32 tmp;

    GetClass86668Methods()->ctor(self, GetSoundEffectDir(0), 0);
    self->methods = GetObj865C8Methods();
    InitDreamAux();
    self->unk44 = func_8003B39C(D_800113EC);
    self->unk44->methods->slot78(self->unk44);
    self->unk44->methods->slot5C(self->unk44);
    req.type = 0;
    req.path = D_800113F8;
    self->unk48 = New_LinkResource(&req);
    tmp = PickWeeklyGroup(0);
    self->unk40 = New_WBgm(tmp, 0, 1);
    func_8004A070(1);
    SetActiveDataSourceDriverMode((u32)arg3 < 1, 1, 1);
    self->unk0C = arg1;
    arg1->unk10 = New_Class869D8();
    arg1->unk8 = New_D8006EF50();
    arg1->unkC = (SubObjG *)New_Class866E8(0, 1);
    self->unk38 = arg2;
    self->methods->slot10(self, (Obj4C *)arg2);
    arg2->methods->slot10C(arg2, self->subB);
    arg2->methods->slot114(arg2, self->unk44);
    self->methods->resetState(self);
}

void Obj865C8__Dtor(Obj865C8 *self) {
    Obj0C *o = self->unk0C;
    SubObjG *g;

    self->methods->slot14(self, self->unk38);
    g = o->unkC;
    o->unkC = g->methods->slot4(g);
    g = o->unk8;
    o->unk8 = g->methods->slot4(g);
    g = o->unk10;
    o->unk10 = g->methods->slot4(g);
    self->unk40->methods->slot4(self->unk40);
    self->unk48->methods->slot4(self->unk48);
    self->unk44->methods->slot4(self->unk44);
    TickDreamAuxSlots();
    GetClass86668Methods()->dtor(self);
}

void Obj865C8__OnNotify(Obj865C8 *self, EventArg *arg1, s32 arg2) {
    s32 tag;

    GetClass86668Methods()->slot38(self, arg1, arg2);
    tag = arg1->target->header;
    if ((tag & 0xFFFF) == 0x1F34) {
        self->methods->slot80(self, arg1, arg2);
    } else if ((tag & 0xFFFFF) == 0x2F230) {
        self->methods->slot84(self, arg1, arg2);
    }
}

void Obj865C8__ResetState(Obj865C8 *self) {
    self->state = 0;
}

void Obj865C8__Init(Obj865C8 *self) {
    SubObjD *sub = self->unk38;

    sub->methods->slot10(sub, self->unk0C->unk4);
    sub->methods->slot10(sub, (s32)self->unk0C->unk8);
    sub->methods->slot110(sub, (s32)self->unk0C->unk10);
    GetClass86668Methods()->slot44(self, (s32)self->unk0C, 0);
}

void Obj865C8__Deinit(Obj865C8 *self) {
    SubObjD *sub = self->unk38;

    GetClass86668Methods()->slot48(self);
    sub->methods->slot110(sub, 0);
    sub->methods->slot14(sub, self->unk0C->unk4);
    sub->methods->slot14(sub, self->unk10);
}

void Obj865C8__StartSubA(Obj865C8 *self) {
    SubObjE *obj;
    SubObjA *subA;
    SubObjF *ret;
    s32 result;

    obj = self->unk0C->obj;
    subA = self->subA;
    result = obj->methods->slot7C(obj, 0);
    subA->methods->slot44(subA, result);
    ret = subA->methods->slot0xAC(subA);
    ret->methods->slot60(ret, 1);
    subA->methods->slot4C(subA, 0x4B0);
    subA->methods->slot70(subA, self->unk38, D_80086650, D_8008665C, 0);
    subA->methods->slot8C(subA);
    self->state = 1;
}

void Obj865C8__RunSubUpdates(Obj865C8 *self) {
    SubObjA *sub = self->subA;

    sub->methods->slot90(sub);
    sub->methods->slot74(sub);
}

/* Defined later in this file (ROM order); forward-declared here since
 * Obj865C8__AdvanceState calls it. */
extern void Obj865C8__EnterState2(Obj865C8 *self, s32 arg1);

void Obj865C8__AdvanceState(Obj865C8 *self, s32 arg1, s32 arg2) {
    s32 result;

    GetClass86668Methods()->slot54(self, arg1, arg2);
    if (arg2 == 2 && self->state != arg2) {
        switch (self->state) {
        case 1:
            result = self->unk38->methods->slot1B4(self->unk38);
            if (result < 0) {
                self->unk38->methods->slot1B8(self->unk38, 0);
                self->eventCode = arg2;
                self->methods->onEventArg(self, 3);
                return;
            }
            Obj865C8__EnterState2(self, result);
            break;
        case 2:
            break;
        case 3:
            self->unk4C->methods->slot48(self->unk4C);
            self->unk4C->methods->slot4(self->unk4C);
            result = self->unk38->methods->slot1E0(self->unk38);
            Obj865C8__EnterState2(self, result);
            break;
        }
    }
}

void Obj865C8__EnterState2(Obj865C8 *self, s32 arg1) {
    self->unk4C = New_ObjM(self->subB, (s32)self->unk40, (s32)self->unk44, (s32)self->unk48, arg1);
    self->methods->slot10(self, self->unk4C);
    self->unk4C->methods->slot44(self->unk4C, (s32)self->unk0C, (s32)self->unk38);
    self->state = 2;
}

void Obj865C8__Noop7C(void) {
}

void Obj865C8__Noop80(void) {
}

/* Returned BY VALUE from SubObjDMethods::slot1BC. Kept LOCAL to this unit --
 * it encodes only what Obj865C8__OnTag2Notify establishes (8 bytes, an s16 at +2 whose
 * sign selects between two eventCode codes), which is not enough for a sibling to
 * reuse unchanged. */
struct SubObjDPos {
    s16 unk0;
    s16 unk2;
    s16 unk4;
    s16 unk6;
};

void Obj865C8__OnTag2Notify(Obj865C8 *self, s32 arg1, s32 arg2) {
    struct SubObjDPos pos;
    s32 result;

    switch (arg2) {
    case 4:
        self->unk4C->methods->slot48(self->unk4C);
        self->unk4C->methods->slot4(self->unk4C);
        result = self->unk38->methods->slot1B8(self->unk38, 0);
        if (result == 0) {
            pos = self->unk38->methods->slot1BC(self->unk38);
            self->eventCode = pos.unk2 < 0 ? 1 : 2;
        } else {
            self->eventCode = 3;
        }
        self->methods->onEventArg(self, 3);
        break;
    case 5:
    case 6:
    case 7:
    case 8:
    case 0xA:
        self->state = 3;
        break;
    case 0xC:
    case 0xD:
        self->unk4C->methods->slot48(self->unk4C);
        self->unk4C->methods->slot4(self->unk4C);
        self->unk38->methods->slot1B8(self->unk38, arg2 != 0xC ? 2 : 1);
        self->eventCode = 3;
        self->methods->onEventArg(self, 3);
        break;
    }
}

Class865C8Methods *GetObj865C8Methods(void) {
    return &D_800865C8;
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

s32 func_8004A070(s32 arg0)
{
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

Obj865C8 *New_Class86668(s32 arg1, SubObjB *arg2)
{
    Obj865C8 *self;

    self = BMemPMgrAlloc(0x38);
    if (self != NULL) {
        GetClass86668Methods()->ctor(self, arg1, arg2);
        return self;
    }
    return NULL;
}

void Class86668__Class86668(Obj865C8 *self, s32 arg1, SubObjB *arg2) {
    Get_vtable_IntermediateBase()->ctor((IntermediateBase *)self);
    self->methods = (Class865C8Methods *)GetClass86668Methods();
    if (arg1 != 0) {
        self->subB = New_VabStreamObj(arg1);
    } else {
        self->subB = arg2;
    }
    self->unk30 = arg1;
    self->methods->resetState(self);
}

void Class86668__Finalize(Obj865C8 *self) {
    if (self->unk30 != 0) {
        self->subB->methods->slot4(self->subB);
    }
    Get_vtable_IntermediateBase()->finalize((IntermediateBase *)self);
}

void Class86668__CancelTimeout(Obj865C8 *self) {
    self->methods->setTimeout(self, -1);
}

s32 Class86668__Init(Obj865C8 *self, s32 arg1, s32 arg2) {
    self->eventCode = 0;
    Get_vtable_IntermediateBase()->init((IntermediateBase *)self, (IntermediateBaseInitArgs *)arg1, arg2);
    return self->eventCode;
}

void Class86668__Deinit(Obj865C8 *self) {
    Get_vtable_IntermediateBase()->deinit((IntermediateBase *)self);
}

void Class86668__NoOpSlot58(void) {
}

void Class86668__CheckTimeout(Obj865C8 *self, s32 arg1, s32 arg2) {
    Get_vtable_IntermediateBase()->update((IntermediateBase *)self, (BasicClass *)arg1, arg2);
    if ((u32)self->frameCounter > (u32)self->timeoutFrames) {
        self->methods->onEventArg(self, 4);
    }
}

void Class86668__SetState(Obj865C8 *self, s32 arg1) {
    Get_vtable_IntermediateBase()->setState((IntermediateBase *)self, arg1);
    if (arg1 == 4) {
        self->eventCode = 1;
        self->methods->noop7C(self);
    }
}

void Class86668__SetTimeout(Obj865C8 *self, s32 arg1) {
    self->timeoutFrames = (arg1 < 0) ? arg1 : arg1 * 20;
}
