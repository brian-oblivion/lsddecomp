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

INCLUDE_ASM("asm/nonmatchings/code_1677c", func_80026108);

INCLUDE_ASM("asm/nonmatchings/code_1677c", func_80026170);

INCLUDE_ASM("asm/nonmatchings/code_1677c", func_80026254);

INCLUDE_ASM("asm/nonmatchings/code_1677c", func_80026328);

INCLUDE_ASM("asm/nonmatchings/code_1677c", func_80026348);

INCLUDE_ASM("asm/nonmatchings/code_1677c", func_80026410);

INCLUDE_ASM("asm/nonmatchings/code_1677c", func_80026518);

INCLUDE_ASM("asm/nonmatchings/code_1677c", func_8002658C);

void func_80026690(void) {
}

INCLUDE_ASM("asm/nonmatchings/code_1677c", func_80026698);

INCLUDE_ASM("asm/nonmatchings/code_1677c", func_8002677C);

INCLUDE_ASM("asm/nonmatchings/code_1677c", func_80026900);
