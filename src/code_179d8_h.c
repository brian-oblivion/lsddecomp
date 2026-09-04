/*
 * code_179d8_h -- functions 43..59 of the original code_179d8 monolith's head,
 * 0x19098..0x194E0 (vram 0x80028898..0x800294E0).  Carved MID-round 17
 * (2026-09-04) to re-staff a runner whose own unit was exhausted.
 *
 * Blocker census, three-grep screen run per function at carve time:
 * 16 of 17 clean.
 *
 * BLOCKED, stub report already filed, do NOT spend attempts on it:
 *   gp_rel: func_80028B6C (only 3 instructions, so nothing is lost)
 *
 * Three of the entries carry real names inherited from FirecatFG's lsddecomp
 * (`strcpy`, `strstr`, `CdStatus`) -- treat those names as HYPOTHESES like any
 * other inherited symbol, but they are a strong hint about the shape.  Three
 * more are 2-instruction leaves that splat matched itself.
 *
 * The 43 functions in FRONT of this slice (still `code_179d8`) are
 * gp_rel-saturated -- 33 of 43 blocked -- and that remainder also owns this
 * segment's ONLY switch jump table (func_80027A24, which will need the Gate 2
 * rodata attach/split when it is carved).  The cut is placed here to leave
 * both debts behind: THIS slice owns no jump table and needs no rodata attach.
 *
 * Class-framework status: measured, not assumed.  Zero functions in this slice
 * reference any of the 60 method tables tools/classtable.py --scan finds.  The
 * sibling slice code_179d8_e DOES contain two class-table accessors, so the
 * "code_179d8 is not class-framework code" note is neighbourhood-scoped -- run
 * the check for your own functions rather than inheriting either verdict.
 */
#include "common.h"

INCLUDE_ASM("asm/nonmatchings/code_179d8_h", func_80028898);

INCLUDE_ASM("asm/nonmatchings/code_179d8_h", func_800288E0);

void func_80028918(void) {
}

INCLUDE_ASM("asm/nonmatchings/code_179d8_h", func_80028920);

INCLUDE_ASM("asm/nonmatchings/code_179d8_h", func_800289CC);

INCLUDE_ASM("asm/nonmatchings/code_179d8_h", func_80028A34);

INCLUDE_ASM("asm/nonmatchings/code_179d8_h", func_80028A50);

void func_80028A7C(void) {
}

INCLUDE_ASM("asm/nonmatchings/code_179d8_h", func_80028A84);

void func_80028B64(void) {
}

INCLUDE_ASM("asm/nonmatchings/code_179d8_h", func_80028B6C);

INCLUDE_ASM("asm/nonmatchings/code_179d8_h", strcpy);

INCLUDE_ASM("asm/nonmatchings/code_179d8_h", strstr);

INCLUDE_ASM("asm/nonmatchings/code_179d8_h", CdStatus);

INCLUDE_ASM("asm/nonmatchings/code_179d8_h", func_80028C44);

INCLUDE_ASM("asm/nonmatchings/code_179d8_h", func_80028C54);

INCLUDE_ASM("asm/nonmatchings/code_179d8_h", func_80028CC0);
