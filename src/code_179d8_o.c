/*
 * code_179d8_o -- functions 0..3 of the original 274-function code_179d8
 * monolith, 0x179d8..0x17AD0 (vram 0x800271D8..0x800272D0).  Carved round 26
 * (2026-09-09): the FRONT window of the head remainder, taken because it is
 * the cheapest blocker-clean ground left in the executable.
 *
 * Blocker census at carve time, four screens per function (gp_rel, forward
 * nop_mflo_mfhi, `jr $t2` trampoline, jtbl):
 *   new_class_6d4e8 (20w)  CLEAN
 *   func_80027228   (19w)  CLEAN
 *   func_80027274   (21w)  CLEAN
 *   func_800272C8    (2w)  CLEAN  -- a bare `jr $ra; nop` leaf
 * 4 of 4 clean, and the window references NO rodata or data symbol at all
 * (zero `%hi`/`%lo` in the whole slice), so no rodata sub-slot is attached.
 *
 * The head remainder that keeps the name `code_179d8` starts immediately
 * after this unit and is gp_rel-saturated (33 of its 39 functions), which is
 * why the cut is here: this window carries neither a jump table nor any
 * blocked-function stub-report debt.
 *
 * This unit calls out to func_80027E68 and func_80027E78, which are still
 * INCLUDE_ASM in that remainder -- their prototypes belong in THIS file, not
 * in a shared header (this unit has none and should not acquire one).
 *
 * `new_class_6d4e8` carries a name inherited from FirecatFG. Treat it as a
 * HYPOTHESIS, not evidence: see CLAUDE.md on the hand-rolled class framework
 * and resolve any method-table slot with tools/classtable.py rather than by
 * counting.
 */
#include "common.h"

INCLUDE_ASM("asm/nonmatchings/code_179d8_o", new_class_6d4e8);

INCLUDE_ASM("asm/nonmatchings/code_179d8_o", func_80027228);

INCLUDE_ASM("asm/nonmatchings/code_179d8_o", func_80027274);

INCLUDE_ASM("asm/nonmatchings/code_179d8_o", func_800272C8);
