/*
 * ROUND 42 CORRECTION (2026-09-15) -- READ BEFORE ANY "BLOCKED" LINE BELOW:
 * every claim in this comment that a function is BLOCKED by `gp_rel`,
 * `nop_mflo_mfhi` or `addiu_at` is STALE.  All three constructs are RESOLVED
 * by pinned maspsx flags (CLAUDE.md, "Open toolchain blockers");
 * `tools/nearmiss.py` reports them tagged (RESOLVED-not-a-blocker) and counts
 * none of them.  Any "do NOT spend attempts on these" directive below is
 * therefore RETRACTED: those functions are ordinary matching work, and most
 * carry a mechanism-correct partial derivation already.  The rest of this
 * comment still stands -- only the blocker verdicts are withdrawn.
 * Screen: `python3 tools/nearmiss.py`, round 43 (2026-09-15).
 *
 * code_179d8_b -- window [60..79] of the original 274-function code_179d8
 * monolith, originally 0x194E0..0x1A1BC (vram 0x80028CE0..0x800299BC).
 *
 * ROUND 34 (head): NINETEEN of the twenty functions left this unit.  Everything
 * from func_80028CE0 (CdSetDebug) through func_800293F8 (CdPosToInt) is
 * `libcd/sys.o` (Psy-Q 3.3), which starts four functions earlier in
 * code_179d8_h and is now linked from the object.  Sixteen of them had been
 * matched as C and three -- CdControl (func_80028DF0), CdControlF
 * (func_80028F38), CdControlB (func_80029074) -- were INCLUDE_ASM stalls with
 * about 1200 lines of derivation between them that could never have closed.
 * The C is gone because Sony's object owns those bytes now (CLAUDE.md: never
 * write C for a function a Sony object owns); the reports are kept, retitled
 * CONVERTED.  The unit is now 0x19C78..0x1A1BC and holds ONE function,
 * func_80029478, which still owns jtbl_800109F8 and so the 0x11F8 rodata
 * attach.  The "low-level serial/link driver" reading below was written
 * about the whole window and is now mostly a reading of libcd itself.
 *
 * Carved round 16 by blocker DENSITY, not by "next": code_179d8 is 44%
 * blocked in aggregate but the blockers CLUSTER, so the aggregate says
 * nothing about any particular window. This one screened 16/20 clean.
 * NONE OF THE THREE "BLOCKED" FUNCTIONS IS BLOCKED ANY MORE.  All three
 * were blocked on `addiu_at` ALONE, and `addiu_at` was RESOLVED in round 21
 * (maspsx `--addiu-at`; docs/research/addiu-at-blocker.md).  Re-screened
 * with `python3 tools/nearmiss.py` on 2026-09-08 (round 24):
 *   func_80028CF8  MATCHED    func_80028D30  MATCHED
 *   func_80029478  blocker-clean, still INCLUDE_ASM -- assignable
 * The previous version of this comment said all three "are already stubbed
 * as match reports", which by round 24 was a stale DIRECTIVE over free
 * ground; their stubs are gone.
 * func_800292F4 was misclassified nop_mflo_mfhi by an inverted screen
 * (round 16 head correction) -- it is fresh ground, not blocked. It
 * contains mult->mfhi (the hazard-slot direction, not a blocker), retail's
 * signed-divide-by-constant idiom.
 *
 * func_80029478 owns jtbl_800109F8, whose sub-slot of the 0xFD8 rodata
 * region is ATTACHED to this unit in the splat yaml. Leave that alone.
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

INCLUDE_ASM("asm/nonmatchings/code_179d8_b", func_80029478);
