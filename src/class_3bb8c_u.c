/*
 * class_3bb8c_u -- carved round 27 (2026-09-10) out of the middle of the
 * old `class_3bb8c_h` BIOS-trampoline segment.  File 0x41148..0x41308,
 * vram 0x80050948..0x80050B08.  Three functions, 112 words.
 *
 * WHY THIS UNIT EXISTS, because its ground was written off for four rounds.
 * `class_3bb8c_h` was screened in round 17 as "15 of 17 clean" and correctly
 * judged the LEAST matchable ground in the executable: 13 of its 17 functions
 * are PSX BIOS trampolines (`addiu $t2,$zero,0xB0 / jr $t2 / addiu $t1,$zero,N`)
 * that no C compiles to, and the other four were all `addiu_at`-blocked.
 * `addiu_at` was RESOLVED in round 21, which converted those four into the
 * densest blocker-clean ground left uncarved -- and nothing re-measured it,
 * because the round-17 note reads as settled and its advice ("carves start
 * after the trampolines") is a directive.  See the splat yaml at the
 * `class_3bb8c_h` entry for the full correction.
 *
 * BLOCKER PROFILE: all three CLEAN.  Re-screened 2026-09-10 with the two
 * live greps (`python3 tools/nearmiss.py`): zero `gp_rel`, zero
 * `nop_mflo_mfhi`.  `addiu_at` is NOT a blocker and screening for it now
 * invents blockers -- do not add it back.  No jump table anywhere in the
 * span, so this unit needs no rodata attach.
 *
 * NOT A CLASS.  None of the three is a class-table slot (`tools/classtable.py
 * --scan` lists none of them), so do not go looking for a vtable.  They are a
 * plain three-deep call chain, measured off the `jal` census:
 *
 *     func_80050A84 (8w)  -> func_80050948 (79w) -> func_80050AA4 (25w)
 *                                                     -> func_800133AC
 *
 * The block sits among BIOS heap trampolines and its one external caller
 * chain treats these as resource/handle management -- `src/class_3bb8c_e.c`
 * documents `func_80050B08`/`func_80050B18` (the two 0xA0 trampolines
 * immediately after this unit) and `func_80050B28` as taking "a resource
 * handle".  Treat that as a hint about the neighbourhood, not as evidence
 * about these three.
 *
 * PER-FUNCTION NOTES for whoever takes this unit:
 *
 *   func_80050948  79w.  The real body.  Called from
 *                  class_3bb8c_g/func_800507F8 (external) and from
 *                  func_80050A84.  9 internal `.L` branch labels, so expect
 *                  real control flow.
 *
 *   func_80050A84  8w.  A wrapper whose only `jal` is func_80050948, with NO
 *                  callers anywhere -- no `jal` from any segment and no data
 *                  pointer to it.  Either dead or an exported entry point.
 *                  **It is 8 words and it tail-calls, so the byte match will
 *                  tell you NOTHING about its return type** -- a `void`
 *                  wrapper around an `s32` tail call is byte-identical.
 *                  Write `return func_80050948(...);` unless you find
 *                  positive evidence it is void.
 *
 *   func_80050AA4  25w.  Tail of the chain; calls func_800133AC.  2 internal
 *                  `.L` labels.
 */

#include "common.h"

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_u", func_80050948);

/* An argument-less wrapper: retail's delay slot is a bare `nop`, so no
 * argument register is set up at the call at all.  Per CLAUDE.md, the byte
 * match tells us NOTHING about the return type here -- a `void` wrapper around
 * an `s32` tail call is byte-identical -- so this is written as a returning
 * wrapper absent positive evidence of `void`.
 */
extern s32 func_80050948(void);

s32 func_80050A84(void) {
    return func_80050948();
}

extern const u8 D_80066841[];
extern s32 func_800133AC(s32 c);

s32 func_80050AA4(s32 c) {
    u8 flags;
    c &= 0xFF;
    flags = D_80066841[c];
    if (flags & 4) {
        return c - 0x30;
    }
    if (!(flags & 3)) {
        return 0x98967F;
    }
    return (func_800133AC(c) & 0xFF) - 0x57;
}
