/*
 * class_3bb8c_m -- seventh carved slice of the class_3bb8c block
 * (0x44518..0x44F14, vram 0x80053D18..0x80054714), 20 functions.
 * Carved round 15.
 *
 * Blocker profile (head's Gate 1 three-grep screen at carve time):
 *   func_800544E4  gp_rel
 *   func_80054558  gp_rel AND addiu-$at -- blocked until both move
 *   func_800545FC  addiu-$at
 *   func_80054660  gp_rel
 * All four have stub reports; do not attempt them. They are the LAST four in
 * ROM order, so the workable run is contiguous from the top of the unit.
 * This unit owns NO switch jump table.
 *
 * include/class_3bb8c.h is SHARED with every other class_3bb8c_* slice.
 * Header edits must be strictly ADDITIVE.
 */
#include "common.h"
#include "class_3bb8c.h"

/* Forward declaration: defined later in this same unit, but called by
 * func_80053D18/func_80053D9C/func_80053E00 above its own definition. */
extern void func_80053EB4(ObjM *self, s32 arg1, s32 arg2, s32 arg3, s32 arg4);

void func_80053D18(ObjM *self) {
    s32 val;
    self->unk20 = 7;
    self->unk3C->methods->slotF0(self->unk3C, &val, -1);
    func_80053EB4(self, val, 0, 5, 1);
    self->unk3C->methods->slotFC(self->unk3C);
}

void func_80053D9C(ObjM *self) {
    self->unk20 = 8;
    func_80053EB4(self, 0, 0, 6, 1);
    self->unk3C->methods->slotF4(self->unk3C, 1);
}

void func_80053E00(ObjM *self) {
    self->unk20 = 0xA;
    func_80053EB4(self, 0, 0, 6, 1);
    self->unk3C->methods->slot13C(self->unk3C, 2);
    self->unk3C->methods->slotF4(self->unk3C, 2);
}

void func_80053E84(ObjM *self) {
    self->methods->slot30(self, 0xB);
}

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_m", func_80053EB4);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_m", func_80053F84);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_m", func_800540E8);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_m", func_80054120);

void func_800541CC(void) {
}

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_m", func_800541D4);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_m", func_80054200);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_m", func_80054208);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_m", func_8005426C);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_m", func_800542D0);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_m", func_800543FC);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_m", func_800544D4);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_m", func_800544E4);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_m", func_80054558);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_m", func_800545FC);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_m", func_80054660);
