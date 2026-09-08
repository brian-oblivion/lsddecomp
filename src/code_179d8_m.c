/*
 * code_179d8_m -- BACK half of what was the `code_179d8_mid_c` asm
 * remainder: functions 161..172 of the original 274-function code_179d8
 * monolith, 0x1ECD8..0x20ADC (vram 0x8002E4D8..0x800302DC), 12 functions.
 * Carved round 24 (2026-09-08).  `code_179d8_l` is the front half and
 * carries the shared carve-time census; `code_179d8_j` follows behind.
 *
 * WHY IT WAS UNCARVED, AND WHY THAT VERDICT IS DEAD.  The old remainder was
 * left as "the addiu-$at dense heart of this monolith".  `addiu_at` was
 * RESOLVED in round 21 (maspsx `--addiu-at`;
 * docs/research/addiu-at-blocker.md).  Re-censused 2026-09-08 with the four
 * screens, canonical shell forms (`grep -A2` FORWARD for nop_mflo_mfhi):
 *
 *   12 of 12 CLEAN -- the whole nop_mflo_mfhi cluster (3 functions) fell in
 *   the front half, so this unit has NO blocked function at all.  Zero
 *   gp_rel, zero nop_mflo_mfhi, zero `jr $t2` trampolines.
 *
 * Sizes, cheapest first -- four functions at 32..60 words:
 *   func_8002F368   32w   func_8002F20C   38w   func_8002F2A4   49w
 *   func_8002F610   60w   func_8002E874  116w   func_800300D0  131w
 *   func_8002F3E8  138w   func_8002EA44  228w   func_8002E4D8  231w
 *   func_8002F700  241w   func_8002EDD4  270w   func_8002FAC4  387w
 *
 * func_8002E874 is a near-identical sibling of func_8002E308 in
 * code_179d8_l -- same prologue (`addu $t3, $a0, $zero`), same s16
 * argument-narrowing shape, same early-out branch.  That opening is
 * REGISTER PRESSURE, not a BIOS trampoline; checked by hand at carve time
 * because the `jr $t2` screen is blind to variants.  If you match it, say
 * so in the report: the other unit's runner is deriving the same shape.
 *
 * Owns NO jump table and needs no rodata attach (see code_179d8_l's header
 * for the survey).  Boundary checks both sides: no function has more than
 * one `addiu $sp, $sp, -N`, every one ends in its own `jr $ra`, zero
 * `alabel`, and the frameless ones open on their own arguments or on a
 * global, never on $sp.
 *
 * Expect this slice to span more than one class; identify each with
 * tools/classtable.py rather than assuming the unit has one.
 */
#include "common.h"

INCLUDE_ASM("asm/nonmatchings/code_179d8_m", func_8002E4D8);

INCLUDE_ASM("asm/nonmatchings/code_179d8_m", func_8002E874);

INCLUDE_ASM("asm/nonmatchings/code_179d8_m", func_8002EA44);

INCLUDE_ASM("asm/nonmatchings/code_179d8_m", func_8002EDD4);

INCLUDE_ASM("asm/nonmatchings/code_179d8_m", func_8002F20C);

INCLUDE_ASM("asm/nonmatchings/code_179d8_m", func_8002F2A4);

INCLUDE_ASM("asm/nonmatchings/code_179d8_m", func_8002F368);

INCLUDE_ASM("asm/nonmatchings/code_179d8_m", func_8002F3E8);

INCLUDE_ASM("asm/nonmatchings/code_179d8_m", func_8002F610);

INCLUDE_ASM("asm/nonmatchings/code_179d8_m", func_8002F700);

INCLUDE_ASM("asm/nonmatchings/code_179d8_m", func_8002FAC4);

INCLUDE_ASM("asm/nonmatchings/code_179d8_m", func_800300D0);
