/*
 * code_179d8_g -- functions 83..99 of the original 274-function code_179d8
 * monolith, 0x1AB78..0x1C440 (vram 0x8002A378..0x8002BC40).  Carved round 17
 * (2026-09-04) off the back of `code_179d8_mid`, which keeps that name for
 * the three functions still in front of this slice (all three addiu-$at
 * blocked, so there is nothing left to staff there).
 *
 * Blocker census, three-grep screen run per function at carve time:
 * 14 of the 17 clean, zero trivial leaves.  These are BIG bodies -- 196,
 * 223, 186 and 189 instructions among them -- so this unit is smaller in
 * count and considerably larger in work than 17 suggests.  Budget fewer
 * functions per pass here than in a leaf-heavy unit.
 *
 * BLOCKED, stub reports already filed, do NOT spend attempts on these:
 *   addiu_at: func_8002AEE0 (174 insn), func_8002B640 (186 insn),
 *             func_8002B94C (189 insn)
 *
 * Owns NO switch jump table -- zero `jtbl_` references in the slice -- so no
 * rodata sub-slot is attached to this unit.
 */
#include "common.h"

INCLUDE_ASM("asm/nonmatchings/code_179d8_g", func_8002A378);

INCLUDE_ASM("asm/nonmatchings/code_179d8_g", func_8002A400);

INCLUDE_ASM("asm/nonmatchings/code_179d8_g", func_8002A510);

INCLUDE_ASM("asm/nonmatchings/code_179d8_g", func_8002A5F8);

INCLUDE_ASM("asm/nonmatchings/code_179d8_g", func_8002A6EC);

INCLUDE_ASM("asm/nonmatchings/code_179d8_g", func_8002A75C);

INCLUDE_ASM("asm/nonmatchings/code_179d8_g", func_8002AA6C);

INCLUDE_ASM("asm/nonmatchings/code_179d8_g", func_8002ADE8);

INCLUDE_ASM("asm/nonmatchings/code_179d8_g", func_8002AEE0);

INCLUDE_ASM("asm/nonmatchings/code_179d8_g", func_8002B198);

INCLUDE_ASM("asm/nonmatchings/code_179d8_g", func_8002B304);

INCLUDE_ASM("asm/nonmatchings/code_179d8_g", func_8002B3E4);

INCLUDE_ASM("asm/nonmatchings/code_179d8_g", func_8002B3F4);

INCLUDE_ASM("asm/nonmatchings/code_179d8_g", func_8002B4D4);

INCLUDE_ASM("asm/nonmatchings/code_179d8_g", func_8002B640);

INCLUDE_ASM("asm/nonmatchings/code_179d8_g", func_8002B928);

INCLUDE_ASM("asm/nonmatchings/code_179d8_g", func_8002B94C);
