/*
 * class_3bb8c_k -- fifth carved slice of the class_3bb8c block
 * (0x429D4..0x435E0, vram 0x800521D4..0x80052DE0), 20 functions.
 * Carved round 15.
 *
 * Blocker profile (head's Gate 1 three-grep screen at carve time):
 *   func_800522DC  addiu-$at, and it OWNS jtbl_800116F4 -- the rodata slot
 *                  at 0x1EF4 is attached to this unit for that reason
 *   func_80052644  gp_rel
 * Both have stub reports; do not attempt either. The other 18 are clean.
 *
 * include/class_3bb8c.h is SHARED with every other class_3bb8c_* slice.
 * Header edits must be strictly ADDITIVE.
 */
#include "common.h"
#include "class_3bb8c.h"
#include "class_39e08.h"

void func_800521D4(Class86F88 *self, s32 state)
{
    self->unk30 = 0;
    if (state < 2) {
        goto end;
    }
    if (state < 4) {
        goto case_lt4;
    }
    if (state == 4) {
        goto case_eq4;
    }
    goto end;
case_lt4:
    self->methods->slot14(self, self->unk34);
    self->methods->slot48(self);
    self->unk2C = state;
    goto end;
case_eq4:
    self->methods->slot30(self, self->unk2C);
end:
    return;
}

void func_8005227C(Class86F88 *self)
{
    s32 old;

    if (self->unk2C >= 4) {
        return;
    }
    if (self->unk2C < 2) {
        return;
    }
    old = self->unk30;
    self->unk30 = old + 1;
    if (old == 0) {
        return;
    }
    self->methods->slot54(self, 4);
}

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_k", func_800522DC);

void func_800523F0(Class86F88 *self, s32 arg1)
{
    Class86F88 *other = self->unk3C;

    if (other != NULL) {
        other->methods->slot80(other, arg1, 0x60, 0x60);
    }
}

void func_80052430(Class86F88 *self)
{
    Class86F88Methods *methods;
    s32 tmp;
    s32 count;

    if (!self->unk50) {
        return;
    }
    tmp = self->unk24;
    count = tmp;
    if (count + 0x1A >= self->unk14) {
        return;
    }
    methods = self->methods;
    count++;
    self->unk24 = count;
    methods->slot94(self, self->unk20, count, self->unk28, 1);
}

void func_80052498(Class86F88 *self)
{
    s32 count;

    if (!self->unk50) {
        return;
    }
    count = self->unk24 - 1;
    if (count < 0) {
        return;
    }
    self->unk24 = count;
    self->methods->slot94(self, self->unk20, count, self->unk28, 1);
}

void func_800524F8(Class86F88 *self, s32 arg1, s32 arg2, s32 arg3)
{
    s32 count;
    s32 newUnk20;
    s32 newUnk28;

    if (!self->unk50) {
        return;
    }
    count = self->unk28;
    if (count - 1 < 0) {
        return;
    }
    if (count - self->unk20 > 0) {
        self->methods->slot98(self, 0, 1, arg3);
    } else {
        self->unk20--;
        newUnk20 = self->unk20;
        self->unk28--;
        newUnk28 = self->unk28;
        self->methods->slot94(self, newUnk20, self->unk24, newUnk28, 1);
    }
}

void func_80052598(Class86F88 *self, s32 arg1, s32 arg2, s32 arg3)
{
    s32 newUnk20;
    s32 newUnk28;
    s32 prevUnk20;

    if (!self->unk50) {
        return;
    }
    if (self->unk28 + 1 >= self->unk10) {
        return;
    }
    prevUnk20 = self->unk20 - 1;
    if (self->unk28 - prevUnk20 < 4) {
        self->methods->slot98(self, 1, 1, arg3);
    } else {
        self->unk20++;
        newUnk20 = self->unk20;
        self->unk28++;
        newUnk28 = self->unk28;
        self->methods->slot94(self, newUnk20, self->unk24, newUnk28, 1);
    }
}

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_k", func_80052644);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_k", func_8005278C);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_k", func_8005281C);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_k", func_8005292C);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_k", func_800529FC);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_k", func_80052A58);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_k", func_80052B54);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_k", func_80052B60);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_k", func_80052B70);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_k", func_80052C10);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_k", func_80052CD8);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_k", func_80052D10);
