/* Second slice of the 365-function class_3bb8c block, 0x3CD88..0x3DA54 --
 * same class (Obj866E8, D_800866E8) as class_3bb8c.c's first slice, split
 * only for parallel runners, so this unit reuses that unit's header
 * (same convention as Entity.c/Entity_b.c). */
#include "common.h"
#include "class_3bb8c.h"

s32 func_8004C588(Obj866E8 *self, s32 key) {
    s32 result;
    s32 i;
    Elem *e;

    result = 0;
    for (i = 0; i < 7; i++) {
        e = &self->arr[i];
        if (e->unk4->unk32 == key) {
            result = i;
            break;
        }
    }
    return result;
}

s32 func_8004C5D0(Obj866E8 *self, s32 key) {
    s32 i;
    Elem *e;

    for (i = 0; i < 7; i++) {
        e = &self->arr[i];
        if (e->unk4->unk30 == key && e->unk4->unk2C != 0) {
            return i;
        }
    }
    return -1;
}

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_b", func_8004C620);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_b", func_8004C6A8);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_b", func_8004C93C);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_b", func_8004CAF0);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_b", func_8004CC74);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_b", func_8004CD38);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_b", func_8004CDA4);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_b", func_8004CE24);

void *func_8004CFA8(Obj866E8 *self) {
    return &self->unk1CC;
}

void func_8004CFB0(Obj866E8 *self, Bounds866E8_3bb8c_b *arg1) {
    self->unk1DC = arg1;
}

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_b", func_8004CFB8);

/* Forward declaration: defined later in this file (after func_8004D028 in
 * ROM-address order), but passed to func_8004D140 as a function-pointer
 * argument before its own definition appears. */
void func_8004D0D0(Obj866E8 *self, EntryChildObj *item);

void func_8004D028(Obj866E8 *self) {
    if (self->unk1E0 > 0) {
        func_8004D140(self, func_8004D0D0, 0);
        self->unk1E0 -= 1;
        if (self->unk1E0 == 0) {
            self->unk1E0 = -1;
        }
    }
}

/* Forward declaration: defined later in this file (after func_8004D088 in
 * ROM-address order), but passed to func_8004D140 as a function-pointer
 * argument before its own definition appears. */
void func_8004D108(Obj866E8 *self, EntryChildObj *item);

void func_8004D088(Obj866E8 *self) {
    if (self->unk1E0 != 0) {
        func_8004D140(self, func_8004D108, 0);
        self->unk1E0 = 0;
    }
}

void func_8004D0D0(Obj866E8 *self, EntryChildObj *item) {
    item->methods->slot48(item, 0, self->unk1E4);
}

void func_8004D108(Obj866E8 *self, EntryChildObj *item) {
    item->methods->slot48(item, 1, D_800869CC);
}

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_b", func_8004D140);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_b", func_8004D1D0);

Obj866E8Methods *func_8004D244(void) {
    return &D_800866E8;
}
