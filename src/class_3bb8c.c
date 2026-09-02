/* First slice of the 365-function class_3bb8c block -- 20 functions,
 * 0x3BB8C..0x3CD88. The remainder is `class_3bb8c_b` and is still a
 * monolithic asm segment.
 *
 * Carve notes for whoever takes the NEXT slice: this block holds all 13 of
 * the game's PSX BIOS trampolines (`jr $t2` with the vector in $t2 and the
 * call number in $t1) and 38 switch jump tables. None of either landed in
 * THIS slice -- verified, not assumed -- which is why it needs no attached
 * rodata slot and no `hasm` segment. The next slice will hit both, and both
 * have to be dispositioned at carve time rather than discovered by a runner
 * that has already spent its attempt budget. See Gate 2 in
 * docs/PARALLEL-RUNS.md.
 */
#include "common.h"
#include "class_3bb8c.h"

INCLUDE_ASM("asm/nonmatchings/class_3bb8c", func_8004B38C);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c", func_8004B418);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c", func_8004B44C);

s32 func_8004B570(Obj866E8 *self) {
    return self->unk70 = 1;
}

void func_8004B57C(Obj866E8 *self) {
    self->methods->slotC0(self);
    self->unk70 = 0;
}

INCLUDE_ASM("asm/nonmatchings/class_3bb8c", func_8004B5BC);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c", func_8004B700);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c", func_8004B930);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c", func_8004BA40);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c", func_8004BB3C);

s32 func_8004BCE0(Obj866E8 *self) {
    s32 count;
    s32 i;

    count = 0;
    for (i = 0; i < 7; i++) {
        if (self->arr[i].flag != 0) {
            count++;
        }
    }
    return count;
}

INCLUDE_ASM("asm/nonmatchings/class_3bb8c", func_8004BD14);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c", func_8004BE54);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c", func_8004C0AC);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c", func_8004C158);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c", func_8004C1C0);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c", func_8004C368);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c", func_8004C3F0);

Elem *func_8004C434(Obj866E8 *self, s32 key) {
    s32 i;
    Elem *e;

    for (i = 0; i < 7; i++) {
        e = &self->arr[i];
        if (e->unk4->unk32 == key) {
            return e;
        }
    }
}

INCLUDE_ASM("asm/nonmatchings/class_3bb8c", func_8004C470);
