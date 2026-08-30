#include "common.h"
#include "code_171e0.h"

void *func_800269E0(void) {
    return D_8006D3C8;
}

void *func_800269F0(UnkFlagsObj_171e0 *this) {
    this->unk20 = 0;
    this->methods->dtor(this);
    func_80018390()->dtor(this);
    func_80017CFC(this);
    return NULL;
}

void func_80026A50(UnkFlagsObj_171e0 *this) {
    func_80018390()->ctor(this);
    this->methods = (UnkFlagsObjMethods_171e0 *) func_80026C9C();
    this->unk0C = 0;
    this->unk10 = NULL;
    this->unk14 = 0;
    this->unk20 = 0;
    this->unk22 = 0;
    this->unknown_value_0x24 = 0;
    this->unk28 = 0;
    this->unk2A = 0;
}

void *func_80026AB4(UnkFlagsObj_171e0 *this) {
    this->methods->slot48(this);
    return this->methods->slot5C(this);
}

INCLUDE_ASM("asm/nonmatchings/code_171e0", func_80026B08);

INCLUDE_ASM("asm/nonmatchings/code_171e0", func_80026C20);

void func_80026C80(void) {
}

void func_80026C88(UnkFlagsObj_171e0 *this) {
    this->unknown_value_0x24 |= 1;
}

void *func_80026C9C(void) {
    return D_8006D430;
}

INCLUDE_ASM("asm/nonmatchings/code_171e0", func_80026CAC);

Vec3_171e0 *func_80026CE8(Vec3_171e0 *this, s32 x, s32 y, s32 z) {
    this->x = x;
    this->y = y;
    this->z = z;
    return this;
}

INCLUDE_ASM("asm/nonmatchings/code_171e0", func_80026CFC);

INCLUDE_ASM("asm/nonmatchings/code_171e0", func_80026D88);

INCLUDE_ASM("asm/nonmatchings/code_171e0", func_80026E0C);

INCLUDE_ASM("asm/nonmatchings/code_171e0", func_80026E38);

INCLUDE_ASM("asm/nonmatchings/code_171e0", func_80026E64);

INCLUDE_ASM("asm/nonmatchings/code_171e0", func_80026E98);

INCLUDE_ASM("asm/nonmatchings/code_171e0", func_80026ECC);

INCLUDE_ASM("asm/nonmatchings/code_171e0", func_80026F00);

INCLUDE_ASM("asm/nonmatchings/code_171e0", func_80026F34);

INCLUDE_ASM("asm/nonmatchings/code_171e0", func_80026FAC);

INCLUDE_ASM("asm/nonmatchings/code_171e0", func_80026FE8);

INCLUDE_ASM("asm/nonmatchings/code_171e0", func_80027024);

INCLUDE_ASM("asm/nonmatchings/code_171e0", func_800270AC);

INCLUDE_ASM("asm/nonmatchings/code_171e0", func_800270B8);

INCLUDE_ASM("asm/nonmatchings/code_171e0", func_800270C4);

INCLUDE_ASM("asm/nonmatchings/code_171e0", strcat);
