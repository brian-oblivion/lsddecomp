/*
 * class_3bb8c_n -- functions 0..22 of the old 113-function class_3bb8c
 * remainder, 0x44F14..0x46288.  23 functions, 1245 words.
 * Carved round 45 (2026-09-15) by the head.
 *
 * DELIBERATELY UNWORKED.  This unit was carved to BANK ground for a future
 * round, not to be worked in round 45 -- it was cut while six runners were
 * already live on other units and nobody was free to take it.  That exact
 * phrase is what `tools/progress.py` keys on to count these 23 functions in
 * the `banked` column instead of `fresh`; without it the carve would inflate
 * `fresh` by its whole size and the next head would mis-triage.  Delete the
 * phrase when you staff someone here.
 *
 * WHY IT WAS UNCARVED UNTIL NOW, and why that reason is dead.  The splat
 * yaml called this segment "the gp_rel-densest ground in the executable",
 * censused it at 3 of 23 clean in round 26, and ended with a directive:
 * "Not worth a runner until the gp-relative blocker moves."  It moved --
 * round 42, maspsx `--gp-symbols`, pinned in the Makefile, whole image
 * byte-exact (CLAUDE.md, "Open toolchain blockers").  `tools/uncarved.py`
 * measures this segment at **23 of 23 blocker-clean**.  Both the census and
 * the directive are retracted in the yaml entry; if you find either quoted
 * anywhere else, it is stale.
 *
 * Carve-time screens, four per function over the live segment: 23 of 23
 * clean; zero `jr $t2` BIOS trampolines; zero `jtbl_` references; zero
 * `alabel`; zero non-`.L` alt-entry labels; zero `.word .L`; no function
 * with more than one `addiu $sp, $sp, -N` prologue.  So NO rodata attach --
 * every `%hi` here is a named dlabel in the D_80087xxx class-table region,
 * which a standalone data segment resolves.  Sizes run 13w..111w with no
 * trivial leaves: every one is a real body.
 *
 * EXPECT THIS SLICE TO SPAN MORE THAN ONE CLASS.  It is cut at ROM
 * addresses, not at class boundaries.  Identify each with
 * `tools/classtable.py`, never by counting slots -- and note the game is
 * plain C with a hand-rolled class framework, not C++.
 *
 * This unit includes include/class_3bb8c.h, which eleven other units also
 * include.  Whoever is staffed here should be the ONLY runner in the
 * class_3bb8c block that round, or the head should price the contention
 * with `python3 tools/headercontention.py` first.
 */

#include "common.h"

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_n", func_80054714);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_n", func_80054758);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_n", func_80054850);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_n", func_800549A8);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_n", func_80054B1C);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_n", func_80054B50);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_n", func_80054B84);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_n", func_80054C74);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_n", func_80054CFC);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_n", func_80054D30);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_n", func_80054DA4);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_n", func_80054F30);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_n", func_80054FD8);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_n", func_8005511C);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_n", func_80055258);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_n", func_80055410);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_n", func_8005556C);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_n", func_80055620);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_n", func_800557DC);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_n", func_8005582C);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_n", func_80055874);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_n", func_800558F0);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_n", func_80055A24);
