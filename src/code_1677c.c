#include "common.h"
#include "Class6D3C8.h"

INCLUDE_ASM("asm/nonmatchings/code_1677c", new_class_6d3c8);

INCLUDE_ASM("asm/nonmatchings/code_1677c", func_80025FDC);

extern void func_80048CFC(s32 day, s32 unused);

/* Advances the day cursor: reads the running tick count kept in scratchpad
 * (0x1F800000, the PS-X data-cache-as-RAM region) and reduces it mod 365. */
void func_800260A4(void) {
    func_80048CFC(*(s32 *)0x1F800000 % 365, 0);
}

/* Defers to the base class's own implementation of this slot when this
 * object hasn't been given an override (unk18 == 0). */
void func_80026108(Class6D3C8 *self, void *a1, void *a2) {
    if (self->unk18 == 0) {
        func_8003B20C()->slot44(self, a1, a2, 0);
    }
}

INCLUDE_ASM("asm/nonmatchings/code_1677c", func_80026170);

INCLUDE_ASM("asm/nonmatchings/code_1677c", func_80026254);

extern s32 func_8004A070(s32 a0);

s32 func_80026328(void) {
    return func_8004A070(0);
}

INCLUDE_ASM("asm/nonmatchings/code_1677c", func_80026348);

INCLUDE_ASM("asm/nonmatchings/code_1677c", func_80026410);

INCLUDE_ASM("asm/nonmatchings/code_1677c", func_80026518);

INCLUDE_ASM("asm/nonmatchings/code_1677c", func_8002658C);

void func_80026690(void) {
}

INCLUDE_ASM("asm/nonmatchings/code_1677c", func_80026698);

INCLUDE_ASM("asm/nonmatchings/code_1677c", func_8002677C);

INCLUDE_ASM("asm/nonmatchings/code_1677c", func_80026900);
