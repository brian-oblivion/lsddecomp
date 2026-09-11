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
 *                                                     -> tolower
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
 *   func_80050948  79w.  THE REAL BODY, and it is a `strtol`.  Fully decoded
 *                  from the disassembly round 27 -- transcribe this rather
 *                  than re-deriving it.  Called from
 *                  class_3bb8c_g/func_800507F8 (external) and from
 *                  func_80050A84.
 *
 *                  Register roles, read off the prologue:
 *                    $s0 = the cursor (arg0, a `const char *`)
 *                    $s3 = sign, initialised 1, negated per '-' seen
 *                    $s1 = base, initialised 10
 *                    $s2 = the accumulator, initialised 0
 *
 *                  Shape:
 *                    if (!p) return 0;
 *                    while (D_80066841[*p] & 8) p++;    // bit 8 == isspace
 *                    while (*p == '-') { sign = -sign; p++; }
 *                    if (*p == '0') {                   // base prefix
 *                        p++;
 *                        switch (*p) {                  // MUST be a switch --
 *                        case 'X': case 'x': p++; base = 16; break;
 *                        case 'B': case 'b': p++; base =  2; break;
 *                        default:                 base =  8; break;
 *                        }
 *                    }
 *                    for (;;) {                         // digit loop
 *                        d = func_80050AA4(*p++);
 *                        if ((u32)d >= (u32)base) break;
 *                        acc = acc * base + d;
 *                    }
 *                    return acc * sign;
 *
 *                  Three details that are easy to get wrong:
 *                  - The loop bound is an UNSIGNED compare (`sltu`), which is
 *                    what makes func_80050AA4's 0x98967F (9999999) sentinel
 *                    terminate the loop for ANY base without a separate test.
 *                  - The '-' handling is a LOOP, not an `if`, so "--5" parses
 *                    as +5.  There is no '+' handling at all.
 *                  - **THE BASE-PREFIX DISPATCH MUST BE A `switch`, NOT AN
 *                    if/else-if CHAIN.  This comment had it wrong and it cost
 *                    four words.**  An earlier version of this decode wrote
 *                    `if (*p=='X' || *p=='x') ... else if (*p=='B' || ...)`,
 *                    which compiles FOUR WORDS SHORT.  Retail's compare tree
 *                    splits first on `*p < 'Y'` -- grouping uppercase against
 *                    lowercase by ASCII value -- which is what a `switch` over
 *                    the four case values lowers to and what an if/else chain
 *                    over two `||` conditions does not.  Corrected round 27 by
 *                    runner alpha, which closed 4 of 5 words with that one
 *                    change (`docs/match-reports/func_80050948.md`).
 *                  - Split the accumulate as `acc *= base; acc += d;` rather
 *                    than `acc = acc*base + d;` -- the combined form leaves an
 *                    `mflo` target register-identity residue that the split
 *                    form fixes for free.  Also alpha's, same round.
 *                  - `base = 8` for a bare leading '0' arrives via a DELAY
 *                    SLOT (`ori $s1,$zero,0x8` under a `beq` that may not be
 *                    taken), so it executes on both paths.  Read the delay
 *                    slots, not just the branches.
 *
 *                  Blocker note: this function contains `mult`/`mflo` pairs,
 *                  but they are all `mult` THEN `mflo` -- retail's ordinary
 *                  idiom, and the SAFE direction.  `nop_mflo_mfhi` blocks the
 *                  opposite order (an `mflo`/`mfhi` FOLLOWED within two
 *                  instructions BY a `mult`/`div`).  Screened clean; do not
 *                  re-file this as blocked.
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
 *   func_80050AA4  25w.  Tail of the chain; calls tolower.  2 internal
 *                  `.L` labels.
 */

#include "common.h"

/* STALL -- see docs/match-reports/func_80050948.md. Compiled length ONE
 * WORD SHORT (78/79); the whole remaining function is content-identical to
 * retail once that one word is accounted for (confirmed via asm-differ).
 * The residue is a single delay-slot scheduling choice: after loading *p,
 * retail schedules a genuine nop before computing D_80066841's address,
 * where this build hoists the independent `lui` into that slot instead --
 * a pure instruction-scheduling preference, not a logic or register-
 * identity difference. Six reshapes tried, all inert; see report. */
#if 0
extern const u8 D_80066841[];
extern s32 func_80050AA4(s32 c);

/* strtol -- see this unit's own header comment for the full decode (register
 * roles, control flow, base-prefix logic) this transcribes verbatim.
 *
 * OLD-STYLE (K&R) parameter syntax is deliberate, not a stylistic choice:
 * func_80050A84 below calls this function with ZERO arguments (retail's own
 * delay slot there is a bare `nop` -- no argument register is ever set up),
 * which only compiles if THIS definition does not act as a full ANSI
 * prototype. An ordinary `s32 func_80050948(const char *p) { ... }`
 * definition DOES establish one even for call sites appearing later in the
 * same translation unit, and cc1 then correctly refuses func_80050A84's
 * call as "too few arguments" -- confirmed by hitting that exact error
 * first. A K&R-style definition does not carry prototype force, letting
 * func_80050A84's argument-less call through exactly as retail's own build
 * evidently allowed it (almost certainly because the two were originally
 * separate translation units, not because the mismatch was intentional). */
s32 func_80050948(p)
const char *p;
{
    s32 sign = 1;
    s32 base = 10;
    s32 acc = 0;
    s32 d;

    if (!p) {
        return 0;
    }
    while (D_80066841[(u8)*p] & 8) {
        p++;
    }
    while (*p == '-') {
        sign = -sign;
        p++;
    }
    if (*p == '0') {
        p++;
        switch (*p) {
        case 'X':
        case 'x':
            p++;
            base = 16;
            break;
        case 'B':
        case 'b':
            p++;
            base = 2;
            break;
        default:
            base = 8;
            break;
        }
    }
    for (;;) {
        d = func_80050AA4(*p++);
        if ((u32)d >= (u32)base) {
            break;
        }
        acc *= base;
        acc += d;
    }
    return acc * sign;
}
#endif
INCLUDE_ASM("asm/nonmatchings/class_3bb8c_u", func_80050948);

extern s32 func_80050948();

/* An argument-less wrapper: retail's delay slot is a bare `nop`, so no
 * argument register is set up at the call at all.  Per CLAUDE.md, the byte
 * match tells us NOTHING about the return type here -- a `void` wrapper around
 * an `s32` tail call is byte-identical -- so this is written as a returning
 * wrapper absent positive evidence of `void`.  The empty-parens (K&R-style)
 * extern above matches what func_80050948's own eventual real definition
 * will need to be declared as (see docs/match-reports/func_80050948.md) so
 * this call, which retail makes with zero arguments, keeps compiling. */
s32 func_80050A84(void) {
    return func_80050948();
}

extern const u8 D_80066841[];
extern s32 tolower(s32 c);

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
    return (tolower(c) & 0xFF) - 0x57;
}
