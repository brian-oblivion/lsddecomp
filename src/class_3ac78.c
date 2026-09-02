#include "common.h"
#include "class_3ac78.h"

void func_8004A478(Class86668 *self, s32 arg1)
{
    Class866E8 *sub = self->unk34;

    if (sub != NULL) {
        sub->methods->slot80(sub, arg1, 0x7F, 0x7F);
    }
}

Class86668Methods *func_8004A4B8(void)
{
    return &D_80086668;
}

INCLUDE_ASM("asm/nonmatchings/class_3ac78", func_8004A4C8);

INCLUDE_ASM("asm/nonmatchings/class_3ac78", func_8004A534);

INCLUDE_ASM("asm/nonmatchings/class_3ac78", func_8004A7C0);

INCLUDE_ASM("asm/nonmatchings/class_3ac78", func_8004A984);

INCLUDE_ASM("asm/nonmatchings/class_3ac78", func_8004AA10);

INCLUDE_ASM("asm/nonmatchings/class_3ac78", func_8004AA6C);

INCLUDE_ASM("asm/nonmatchings/class_3ac78", func_8004AB24);

void func_8004AB88(Class866E8 *self, GenericObject *other, s32 count)
{
    if ((u8)other->methods->header == 0x34) {
        self->methods->slotD0(self, other, count);
    }
}

INCLUDE_ASM("asm/nonmatchings/class_3ac78", func_8004ABD0);

INCLUDE_ASM("asm/nonmatchings/class_3ac78", func_8004ACF8);

void func_8004ADC4(Class866E8 *self, s32 arg1, s32 arg2)
{
    self->unk60 = arg1;
    self->unk64 = arg2;
}

void func_8004ADD0(Class866E8 *self, s32 arg1)
{
    self->unkE8 = arg1;
}

INCLUDE_ASM("asm/nonmatchings/class_3ac78", func_8004ADD8);

INCLUDE_ASM("asm/nonmatchings/class_3ac78", func_8004AEA4);

extern void func_8004C93C(Class866E8 *self);

void func_8004AFE0(Class866E8 *self, UnkArgObj_3ac78 *arg1, s32 arg2)
{
    s16 t;

    self->unk7C = arg1->unk2 - 1;
    t = arg1->unk3 - 1;
    self->unk80 = arg2;
    self->unk84 = arg2;
    self->unk7E = t;
    func_8004C93C(self);
}

INCLUDE_ASM("asm/nonmatchings/class_3ac78", func_8004B030);

INCLUDE_ASM("asm/nonmatchings/class_3ac78", func_8004B100);

void func_8004B2D4(Class866E8 *self)
{
    if (self != NULL && (self->flags36 & 0x80)) {
        self->methods->slot38(self);
    }
}

void *func_8004B31C(Class866E8 *self)
{
    return &self->unk1C0;
}

void func_8004B324(void) {
}

void func_8004B32C(Class866E8 *self, s32 arg1)
{
    self->unk74 = arg1;
    self->unk7A = (s16)(arg1 >> 11);
    self->unk78 = (s16)(arg1 >> 12);
}

void func_8004B344(Class866E8 *self, s32 arg1)
{
    self->methods->slot40(self);
    self->unk68 = arg1;
}
