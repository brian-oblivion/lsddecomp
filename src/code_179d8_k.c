/*
 * code_179d8_k -- functions 238..255 of the original 274-function code_179d8
 * monolith, 0x24938..0x2673C (vram 0x80034138..0x80035F3C).  Carved round 24
 * (2026-09-08) out of what had been the `code_179d8_tail` asm remainder,
 * which this unit consumes WHOLE -- there is no remainder left on either
 * side (code_179d8_i in front, code_179d8_f behind).
 *
 * WHY IT WAS UNCARVED FOR SIX ROUNDS, AND WHY THAT VERDICT IS DEAD.
 * The splat comment on the old remainder read "17 of its 18 functions are
 * addiu-$at blocked".  `addiu_at` was RESOLVED in round 21 (maspsx
 * `--addiu-at`; docs/research/addiu-at-blocker.md), so that census measured
 * an obstruction that no longer exists.  Re-censused 2026-09-08 with
 * `python3 tools/nearmiss.py`'s four screens, canonical shell forms
 * (`grep -A2` FORWARD for nop_mflo_mfhi -- the direction is load-bearing):
 *
 *   18 of 18 CLEAN.  Zero gp_rel, zero nop_mflo_mfhi, zero `jr $t2`
 *   trampolines.  This is the single best carve left in the executable.
 *
 * Sizes, cheapest first -- six functions at 31..51 words, which is the
 * cheap seam the `fresh` queue had run out of:
 *   func_80034614  31w   func_800350D8  31w   func_80035154  31w
 *   func_80035A7C  44w   func_80035E80  47w   func_80034D90  51w
 *   func_80034138  69w   func_800344FC  70w   func_80034E5C  77w
 *   func_800349B0  79w   func_80034AEC  79w   func_80034F90  82w
 *   func_80034C28  90w   func_8003424C 172w   func_800357B0 179w
 *   func_80034690 200w   func_80035B2C 213w   func_800351D0 376w
 *
 * THIS UNIT OWNS THREE SWITCH JUMP TABLES, not the two the old remainder
 * comment claimed: func_80034690 -> jtbl_80010CF0, and func_800357B0 ->
 * jtbl_80010ED8 AND jtbl_80010F38 (a double switch).  The 0x14F0 rodata
 * slot holds exactly those three tables and nothing else, is referenced
 * from nowhere outside this unit, and is attached whole in the splat yaml.
 * You do not need to do anything about it -- but if you match either
 * function, remember a `%lo(jtbl_*)` load is ordinary matchable code now.
 *
 * Boundary checks at carve time, both sides: no function has more than one
 * `addiu $sp, $sp, -N`, every one ends in its own `jr $ra`, zero `alabel`,
 * and the one frameless function (func_80035E80) opens on
 * `sll $a0, $a0, 16` -- leaf argument narrowing, not a caller-frame read.
 *
 * Expect this slice to span more than one class; a ~20-function slice cut
 * at ROM-address boundaries has no reason to align with class boundaries.
 * Identify each with tools/classtable.py rather than assuming the unit has
 * one.  Expect low-level driver-shaped code rather than class-framework
 * code, as elsewhere in code_179d8; confirm, do not assume.
 */
#include "common.h"

INCLUDE_ASM("asm/nonmatchings/code_179d8_k", func_80034138);

INCLUDE_ASM("asm/nonmatchings/code_179d8_k", func_8003424C);

INCLUDE_ASM("asm/nonmatchings/code_179d8_k", func_800344FC);

INCLUDE_ASM("asm/nonmatchings/code_179d8_k", func_80034614);

INCLUDE_ASM("asm/nonmatchings/code_179d8_k", func_80034690);

INCLUDE_ASM("asm/nonmatchings/code_179d8_k", func_800349B0);

INCLUDE_ASM("asm/nonmatchings/code_179d8_k", func_80034AEC);

INCLUDE_ASM("asm/nonmatchings/code_179d8_k", func_80034C28);

INCLUDE_ASM("asm/nonmatchings/code_179d8_k", func_80034D90);

INCLUDE_ASM("asm/nonmatchings/code_179d8_k", func_80034E5C);

INCLUDE_ASM("asm/nonmatchings/code_179d8_k", func_80034F90);

INCLUDE_ASM("asm/nonmatchings/code_179d8_k", func_800350D8);

INCLUDE_ASM("asm/nonmatchings/code_179d8_k", func_80035154);

INCLUDE_ASM("asm/nonmatchings/code_179d8_k", func_800351D0);

INCLUDE_ASM("asm/nonmatchings/code_179d8_k", func_800357B0);

INCLUDE_ASM("asm/nonmatchings/code_179d8_k", func_80035A7C);

INCLUDE_ASM("asm/nonmatchings/code_179d8_k", func_80035B2C);

INCLUDE_ASM("asm/nonmatchings/code_179d8_k", func_80035E80);
