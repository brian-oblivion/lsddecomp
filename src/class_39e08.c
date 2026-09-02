#include "common.h"
#include "class_39e08.h"

Obj865C8 *func_80049608(Obj0C *arg1, SubObjD *arg2, s32 arg3)
{
    Obj865C8 *self;

    self = func_80017B34(0x50);
    if (self != NULL) {
        func_8004A060()->ctor(self, arg1, arg2, arg3);
        return self;
    }
    return NULL;
}

void func_80049684(Obj865C8 *self, Obj0C *arg1, SubObjD *arg2, s32 arg3) {
    LoadRequest req;
    s32 tmp;

    func_8004A4B8()->ctor(self, func_80048E08(0), 0);
    self->methods = func_8004A060();
    func_8005C508();
    self->unk44 = func_8003B39C(D_800113EC);
    self->unk44->methods->slot78(self->unk44);
    self->unk44->methods->slot5C(self->unk44);
    req.type = 0;
    req.path = D_800113F8;
    self->unk48 = func_80043840(&req);
    tmp = func_80048D74(0);
    self->unk40 = func_800398E0(tmp, 0, 1);
    func_8004A070(1);
    func_80026F34((u32)arg3 < 1, 1, 1);
    self->unk0C = arg1;
    arg1->unk10 = func_8004D254();
    arg1->unk8 = func_80042400();
    arg1->unkC = (SubObjG *)func_8004A4C8(0, 1);
    self->unk38 = arg2;
    self->methods->slot10(self, (Obj4C *)arg2);
    arg2->methods->slot10C(arg2, self->subB);
    arg2->methods->slot114(arg2, self->unk44);
    self->methods->resetUnk3C(self);
}

void func_80049830(Obj865C8 *self) {
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
    func_8005C5E8();
    func_8004A4B8()->dtor(self);
}

void func_80049958(Obj865C8 *self, EventArg *arg1, s32 arg2) {
    s32 tag;

    func_8004A4B8()->slot38(self, arg1, arg2);
    tag = arg1->target->header;
    if ((tag & 0xFFFF) == 0x1F34) {
        self->methods->slot80(self, arg1, arg2);
    } else if ((tag & 0xFFFFF) == 0x2F230) {
        self->methods->slot84(self, arg1, arg2);
    }
}

void func_80049A14(Obj865C8 *self) {
    self->unk3C = 0;
}

void func_80049A1C(Obj865C8 *self) {
    SubObjD *sub = self->unk38;

    sub->methods->slot10(sub, self->unk0C->unk4);
    sub->methods->slot10(sub, (s32)self->unk0C->unk8);
    sub->methods->slot110(sub, (s32)self->unk0C->unk10);
    func_8004A4B8()->slot44(self, (s32)self->unk0C, 0);
}

void func_80049AC0(Obj865C8 *self) {
    SubObjD *sub = self->unk38;

    func_8004A4B8()->slot48(self);
    sub->methods->slot110(sub, 0);
    sub->methods->slot14(sub, self->unk0C->unk4);
    sub->methods->slot14(sub, self->unk10);
}

void func_80049B54(Obj865C8 *self) {
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
    self->unk3C = 1;
}

void func_80049C50(Obj865C8 *self) {
    SubObjA *sub = self->subA;

    sub->methods->slot90(sub);
    sub->methods->slot74(sub);
}

/* Defined later in this file (ROM order); forward-declared here since
 * func_80049CA8 calls it. */
extern void func_80049E20(Obj865C8 *self, s32 arg1);

void func_80049CA8(Obj865C8 *self, s32 arg1, s32 arg2) {
    s32 result;

    func_8004A4B8()->slot54(self, arg1, arg2);
    if (arg2 == 2 && self->unk3C != arg2) {
        switch (self->unk3C) {
        case 1:
            result = self->unk38->methods->slot1B4(self->unk38);
            if (result < 0) {
                self->unk38->methods->slot1B8(self->unk38, 0);
                self->unk28 = arg2;
                self->methods->onEventArg(self, 3);
                return;
            }
            func_80049E20(self, result);
            break;
        case 2:
            break;
        case 3:
            self->unk4C->methods->slot48(self->unk4C);
            self->unk4C->methods->slot4(self->unk4C);
            result = self->unk38->methods->slot1E0(self->unk38);
            func_80049E20(self, result);
            break;
        }
    }
}

void func_80049E20(Obj865C8 *self, s32 arg1) {
    self->unk4C = func_80052B70(self->subB, (s32)self->unk40, (s32)self->unk44, (s32)self->unk48, arg1);
    self->methods->slot10(self, self->unk4C);
    self->unk4C->methods->slot44(self->unk4C, (s32)self->unk0C, (s32)self->unk38);
    self->unk3C = 2;
}

void func_80049EA4(void) {
}

void func_80049EAC(void) {
}

INCLUDE_ASM("asm/nonmatchings/class_39e08", func_80049EB4);

Class865C8Methods *func_8004A060(void) {
    return &D_800865C8;
}

INCLUDE_ASM("asm/nonmatchings/class_39e08", func_8004A070);

Obj865C8 *func_8004A130(s32 arg1, SubObjB *arg2)
{
    Obj865C8 *self;

    self = func_80017B34(0x38);
    if (self != NULL) {
        func_8004A4B8()->ctor(self, arg1, arg2);
        return self;
    }
    return NULL;
}

void func_8004A19C(Obj865C8 *self, s32 arg1, SubObjB *arg2) {
    func_8003E5C8()->ctor(self);
    self->methods = (Class865C8Methods *)func_8004A4B8();
    if (arg1 != 0) {
        self->subB = func_8002C480(arg1);
    } else {
        self->subB = arg2;
    }
    self->unk30 = arg1;
    self->methods->resetUnk3C(self);
}

void func_8004A228(Obj865C8 *self) {
    if (self->unk30 != 0) {
        self->subB->methods->slot4(self->subB);
    }
    func_8003E5C8()->dtor(self);
}

void func_8004A294(Obj865C8 *self) {
    self->methods->setUnk2C(self, -1);
}

s32 func_8004A2C4(Obj865C8 *self, s32 arg1, s32 arg2) {
    self->unk28 = 0;
    func_8003E5C8()->slot44(self, arg1, arg2);
    return self->unk28;
}

void func_8004A324(Obj865C8 *self) {
    func_8003E5C8()->slot48(self);
}

void func_8004A35C(void) {
}

void func_8004A364(Obj865C8 *self, s32 arg1, s32 arg2) {
    func_8003E5C8()->slot5C(self, arg1, arg2);
    if ((u32)self->unk1C > (u32)self->unk2C) {
        self->methods->onEventArg(self, 4);
    }
}

void func_8004A3EC(Obj865C8 *self, s32 arg1) {
    func_8003E5C8()->slot60(self, arg1);
    if (arg1 == 4) {
        self->unk28 = 1;
        self->methods->noop7C(self);
    }
}

void func_8004A458(Obj865C8 *self, s32 arg1) {
    self->unk2C = (arg1 < 0) ? arg1 : arg1 * 20;
}
