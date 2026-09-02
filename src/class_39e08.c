#include "common.h"
#include "class_39e08.h"

INCLUDE_ASM("asm/nonmatchings/class_39e08", func_80049608);

INCLUDE_ASM("asm/nonmatchings/class_39e08", func_80049684);

INCLUDE_ASM("asm/nonmatchings/class_39e08", func_80049830);

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
    sub->methods->slot10(sub, self->unk0C->unk8);
    sub->methods->slot110(sub, self->unk0C->unk10);
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

INCLUDE_ASM("asm/nonmatchings/class_39e08", func_80049CA8);

void func_80049E20(Obj865C8 *self, s32 arg1) {
    self->unk4C = func_80052B70(self->subB, self->unk40, self->unk44, self->unk48, arg1);
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

INCLUDE_ASM("asm/nonmatchings/class_39e08", func_8004A130);

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
