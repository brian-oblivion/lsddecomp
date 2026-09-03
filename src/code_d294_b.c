#include "common.h"
#include "code_d294.h"

/* Sibling of func_8001D344/D374/D3A0/D3CC/D3F8 (code_d294.c): a thin
 * wrapper around func_8001EDAC over &self->unk10, shift 0 width 3. Raw
 * pass-through value and raw pass-through result -- same shape as
 * func_8001D374/D3A0/D3F8 (no `== 0` on either side). */
u32 func_8001D424(Class6B5CCObj *self, u32 a1) {
    return func_8001EDAC(&self->unk10, 0, 3, a1);
}

/* Sibling of func_8001D344 (the ONLY one of the five already-matched
 * self->unk10 bitfield accessors that both converts its input to a boolean
 * (`a1 == 0`) AND inverts its own result (`== 0`)). This function does
 * exactly that double-inversion, at shift 7 width 1, hence the same `s32`
 * return type as func_8001D344 rather than the plain `u32` of the other
 * three siblings. */
s32 func_8001D450(Class6B5CCObj *self, s32 a1) {
    return func_8001EDAC(&self->unk10, 7, 1, a1 == 0) == 0;
}

/* Same family as func_8001D424, shift 9 width 3. Raw pass-through. */
u32 func_8001D480(Class6B5CCObj *self, u32 a1) {
    return func_8001EDAC(&self->unk10, 9, 3, a1);
}

/* Same family as func_8001D450: double-inversion shape, shift 8 width 1. */
s32 func_8001D4AC(Class6B5CCObj *self, s32 a1) {
    return func_8001EDAC(&self->unk10, 8, 1, a1 == 0) == 0;
}

INCLUDE_ASM("asm/nonmatchings/code_d294_b", func_8001D4DC);

INCLUDE_ASM("asm/nonmatchings/code_d294_b", func_8001D568);

/* Forwards self->unk20 (still opaque, retyped `void *` this round -- see
 * include/code_d294.h) and its own 2nd argument straight through to
 * func_8001F51C, untouched. func_8001F51C's own body (psyq_GsLinkObject4.s)
 * has no deliberate return value -- see the extern's own comment -- so this
 * wrapper is void, not `return func_8001F51C(...)`. */
void func_8001D600(Class6B5CCObj *self, void *dest) {
    func_8001F51C(self->unk20, dest);
}

INCLUDE_ASM("asm/nonmatchings/code_d294_b", func_8001D624);

void func_8001D6A4(void) {
}

void func_8001D6AC(void) {
}

INCLUDE_ASM("asm/nonmatchings/code_d294_b", func_8001D6B4);

INCLUDE_ASM("asm/nonmatchings/code_d294_b", func_8001D714);

INCLUDE_ASM("asm/nonmatchings/code_d294_b", func_8001D950);

INCLUDE_ASM("asm/nonmatchings/code_d294_b", func_8001DA28);

INCLUDE_ASM("asm/nonmatchings/code_d294_b", func_8001DDF4);

INCLUDE_ASM("asm/nonmatchings/code_d294_b", func_8001E110);

INCLUDE_ASM("asm/nonmatchings/code_d294_b", func_8001E2E8);

void func_8001E49C(void) {
}

INCLUDE_ASM("asm/nonmatchings/code_d294_b", func_8001E4A4);

/* This unit's own no-argument vtable getter -- see the extended note on
 * D_8006B5CC in include/code_d294.h and the file banner up top. */
Class6B5CCMethods *func_8001E57C(void) {
    return &D_8006B5CC;
}
