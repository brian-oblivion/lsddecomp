/*
 * class_3bb8c_q -- functions 94..95 of the class_3bb8c remainder,
 * 0x485BC..0x48738 (95 words).  Carved round 47 (2026-09-16).
 *
 * THE CARVE NOTE THAT STOOD HERE FOR 26 ROUNDS WAS STALE AND SAID "nothing to
 * staff here": func_80057DBC was filed as addiu-$at blocked (resolved round
 * 21) and func_80057DF4 as nop_mflo_mfhi blocked (resolved round 42).
 * `tools/uncarved.py` measures both blocker-clean.  Both are frameless leaves
 * (zero `addiu $sp, $sp, -N`), so neither is expected to have a stack frame.
 *
 * Owns no jump table, so no rodata attach.  Expect this slice to span more
 * than one class; identify each with `tools/classtable.py`.
 */

#include "common.h"

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_q", func_80057DBC);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_q", func_80057DF4);
