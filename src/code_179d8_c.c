/*
 * code_179d8_c -- window [200..219] of the original 274-function code_179d8
 * monolith, now 0x22A1C..0x22BA8 (vram 0x8003221C..0x800323A8).
 *
 * ROUND 33: this slice lost TWO functions to Sony and was split in half.
 *   - func_80032148, its old FIRST function, is `SpuVmVSetUp`
 *     (`libsnd/vm_vsu.o`, Psy-Q 3.3), linked from the object.
 *   - func_800323A8 (120w) is `SsSetTableSize` (`libsnd/sstable.o`, Psy-Q
 *     3.5), linked from the object. It sat in the MIDDLE, so the slice became
 *     [c code_179d8_c][o sstable][c code_179d8_c_b] and everything from
 *     func_80032588 on now lives in `src/code_179d8_c_b.c`.
 * Neither was ever matchable as C; both stall reports are kept, re-titled
 * CONVERTED. This unit is now three functions: func_8003221C and its two
 * one-line callers.
 *
 * THE 0x14D8 RODATA ATTACH IS NO LONGER OURS. It belongs to func_80032588
 * (jtbl_80010CD8), which went to code_179d8_c_b, and the yaml attach moved
 * with it. Do not move it back.
 *
 * Carved round 16 by blocker DENSITY (see code_179d8_b's header for the
 * full window census). This window screened 16/20 clean.
 *
 * STALE CLAIM REMOVED, round 23 (2026-09-07): this comment listed
 * func_80032148, func_80032588, func_80032BF0 and func_80032C28 as blocked,
 * "all addiu_at". **`addiu_at` was resolved in round 21** (maspsx
 * `--addiu-at`; docs/research/addiu-at-blocker.md), and round 23 MATCHED
 * func_80032BF0 and func_80032C28 byte-exact and took the other two to
 * 48/53 and ~90/96 with characterised non-toolchain residues. (The 48/53 was
 * func_80032148 -- round 32 then found it is Sony's, so that effort was spent
 * on library code; see docs/match-reports/func_80032148.md.) Nothing in
 * this window is toolchain-blocked. The two live blockers are `gp_rel` and
 * `nop_mflo_mfhi`; screen with `python3 tools/nearmiss.py`, never by
 * re-implementing the greps, and never for `addiu_at`.
 *
 * Round 23's runner bravo spotted this line as stale and correctly did not
 * edit it (parallel-mode rules); the head fixed it at consolidation.
 *
 * Note func_80032AD0/SetRCnt: this window holds what look like Psy-Q root
 * counter routines linked into game text rather than into a psyq_* segment.
 * They are ordinary work, but do not generalise a finding from them to the
 * game's own code.
 *
 * This slice was cut at ROM-address boundaries, so it has no reason to
 * align with class boundaries -- expect it to span more than one class,
 * and identify each with tools/classtable.py rather than assuming one.
 *
 * Declarations: keep anything that encodes THIS unit's reading of a class
 * next to the code, in this file. Do not create a shared code_179d8*.h --
 * the sibling slices are staffed independently and a shared header is what
 * makes their merges collide.
 */
#include "common.h"


INCLUDE_ASM("asm/nonmatchings/code_179d8_c", func_8003221C);

extern void func_8003221C(s32 arg0);

void func_80032368(void)
{
    func_8003221C(0);
}

void func_80032388(void)
{
    func_8003221C(1);
}
