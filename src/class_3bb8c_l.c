/*
 * class_3bb8c_l -- sixth carved slice of the class_3bb8c block
 * (0x435E0..0x44518, vram 0x80052DE0..0x80053D18), 20 functions.
 * Carved round 15.
 *
 * Blocker profile (head's Gate 1 three-grep screen at carve time):
 *   func_80052F10  addiu-$at
 *   func_800534C8  gp_rel
 *   func_80053984  addiu-$at, and it OWNS jtbl_8001174C -- the rodata slot
 *                  at 0x1F4C is attached to this unit for that reason
 * All three have stub reports; do not attempt them. The other 17 are clean
 * (func_80052DE0 and func_800534C0 are bare `jr $ra; nop` stubs splat
 * generated itself, so 15 are real work).
 *
 * include/class_3bb8c.h is SHARED with every other class_3bb8c_* slice.
 * Header edits must be strictly ADDITIVE.
 */
#include "common.h"
#include "class_3bb8c.h"

void func_80052DE0(void) {
}

void func_80052DE8(Obj87034_3bb8c_l *self, Obj87034_3bb8c_l *arg1, s32 arg2) {
    arg1->unkC->methods->slotC8(arg1->unkC, func_80052E7C, self);
    self->unk3C = (DreamSysObj_3bb8c_l *)arg2;
    func_8004A4B8()->slot44(self, arg1, 1);
    self->methods->slot10(self, arg2);
}

void func_80052E7C(Obj87034_3bb8c_l *self, s32 code, s32 arg2, s32 arg3) {
    if (code >= 0) {
        func_80049060(self->unk38);
    } else {
        func_80049098(self->unk38, arg2, arg3);
    }
}

void func_80052EBC(Obj87034_3bb8c_l *self) {
    self->methods->slot14(self, self->unk3C);
    func_8004A4B8()->slot48(self);
}

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_l", func_80052F10);

void func_80053134(Obj87034_3bb8c_l *self) {
    self->methods->slot84(self);
    func_8005C76C();
    func_80054D30();
    self->unk54->methods->slot48(self->unk54);
}

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_l", func_800531A0);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_l", func_800531CC);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_l", func_80053358);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_l", func_800533F0);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_l", func_80053458);

void func_800534C0(void) {
}

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_l", func_800534C8);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_l", func_800536B0);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_l", func_80053764);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_l", func_8005393C);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_l", func_80053984);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_l", func_80053ACC);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_l", func_80053BE8);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_l", func_80053C94);
