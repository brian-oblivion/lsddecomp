/*
 * code_179d8_p -- func_80031F3C, 0x2273C..0x22948 (vram 0x80031F3C..
 * 0x80032148).  A single 131-word function.  Carved round 26 (2026-09-09)
 * out of what had been the `code_179d8_mid_d` remainder; renamed on carve
 * because "mid_d" named a leftover and the leftover is now fully consumed.
 *
 * Blocker census at carve time, four screens: BLOCKER-CLEAN -- zero gp_rel,
 * zero forward nop_mflo_mfhi, zero `jr $t2` trampolines, zero `jtbl_`, zero
 * `alabel`.  The body is FRAMELESS (no `addiu $sp, $sp, -N` anywhere), which
 * is worth knowing before you write C for it.
 *
 * It had been parked for several rounds as "addiu-$at blocked"; `addiu_at`
 * was resolved in round 21 and was its only obstruction.
 *
 * Owns NO jump table, so no rodata sub-slot is attached.  It does reference
 * plain rodata/data SYMBOLS -- reference them as symbols, never re-type a
 * string literal (splat has already emitted those bytes; a literal emits a
 * second copy and shifts the whole image).
 *
 * READ THIS BEFORE STARTING: func_80031F3C touches the same global family as
 * `code_179d8_m` -- D_8008D988/98A/98C/98E/996/998/99A/99C/9A3 and
 * D_8006DAD4.  The "split scaled index" entry in
 * docs/DECOMPILATION_LEARNINGS.md (a mask on the PRODUCT means a halfword
 * array indexed by a truncated `idx*8`, NOT a struct array indexed by a cast
 * index) was derived on exactly those globals, together with its
 * loop-versus-non-loop refinement.  It is very likely to apply here.
 *
 * This unit's extern declarations stay LOCAL to this file; it shares no
 * project header with any other unit and should not acquire one.
 */
#include "common.h"

INCLUDE_ASM("asm/nonmatchings/code_179d8_p", func_80031F3C);
