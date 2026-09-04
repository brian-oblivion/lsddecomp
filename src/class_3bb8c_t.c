/*
 * class_3bb8c_t -- functions 96..112 of the 113-function `class_3bb8c_n`
 * remainder, 0x48738..0x48F74 (vram 0x80057F38..0x80058774).  Carved
 * MID-round 17 (2026-09-04) to re-staff a runner whose own unit was
 * exhausted.  This is the LAST slice of the class_3bb8c block.
 *
 * Blocker census, three-grep screen run per function at carve time:
 * 16 of 17 clean.
 *
 * BLOCKED, stub report already filed, do NOT spend attempts on it:
 *   addiu_at: func_800585B4
 *
 * Four of the 17 are 2-instruction leaves that splat matched itself.
 *
 * This slice holds D_8001176C and D_80011778, the two strings left
 * STANDALONE when the 0x1EF4 rodata slot was split -- referenced from
 * func_80057FEC and func_80058084, both of which are in this unit.  A
 * string is referenced by SYMBOL and a standalone rodata object resolves
 * that fine (the `code_8220` / 0xA8C precedent in Gate 2), so no attach was
 * needed and the link came up green, which is the check that settles it.
 * The slice owns no `jtbl_` reference either.
 *
 * EXPECT THIS SLICE TO SPAN MORE THAN ONE CLASS.  It is cut at ROM
 * addresses, not class boundaries.  Identify each with tools/classtable.py.
 */
#include "common.h"

/* The class allocated by this unit's own func_80057F68, table D_800879C4
 * (49 slots, resolved via tools/classtable.py). Its ctor (func_80057D10)
 * and its own funcs 80057F38/40/48/50 live in the neighbouring
 * `class_3bb8c_p` unit (round 2026-09-04 earlier this round), which
 * already carries its own local view of this table
 * (`D_800879C4Methods`/`D_800879C4Obj` in that file). This unit's own
 * view is kept separate per the multiple-independent-local-views
 * convention -- func_80057F58 itself needs no fields, only the address. */
typedef struct D_800879C4Table D_800879C4Table;
extern D_800879C4Table D_800879C4;

void func_80057F38(void) {
}

void func_80057F40(void) {
}

void func_80057F48(void) {
}

void func_80057F50(void) {
}

D_800879C4Table *func_80057F58(void) {
    return &D_800879C4;
}

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_t", func_80057F68);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_t", func_80057FC8);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_t", func_80058078);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_t", func_800580E0);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_t", func_800581C4);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_t", func_80058228);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_t", func_80058308);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_t", func_80058390);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_t", func_80058404);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_t", func_800585B4);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_t", func_80058694);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_t", func_80058764);
