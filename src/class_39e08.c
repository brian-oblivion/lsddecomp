#include "common.h"
#include "class_39e08.h"

INCLUDE_ASM("asm/nonmatchings/class_39e08", func_80049608);

INCLUDE_ASM("asm/nonmatchings/class_39e08", func_80049684);

INCLUDE_ASM("asm/nonmatchings/class_39e08", func_80049830);

INCLUDE_ASM("asm/nonmatchings/class_39e08", func_80049958);

void func_80049A14(Obj865C8 *self) {
    self->unk3C = 0;
}

INCLUDE_ASM("asm/nonmatchings/class_39e08", func_80049A1C);

INCLUDE_ASM("asm/nonmatchings/class_39e08", func_80049AC0);

INCLUDE_ASM("asm/nonmatchings/class_39e08", func_80049B54);

void func_80049C50(Obj865C8 *self) {
    SubObjA *sub = self->subA;

    sub->methods->slot90(sub);
    sub->methods->slot74(sub);
}

INCLUDE_ASM("asm/nonmatchings/class_39e08", func_80049CA8);

INCLUDE_ASM("asm/nonmatchings/class_39e08", func_80049E20);

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

INCLUDE_ASM("asm/nonmatchings/class_39e08", func_8004A19C);

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

INCLUDE_ASM("asm/nonmatchings/class_39e08", func_8004A364);

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
