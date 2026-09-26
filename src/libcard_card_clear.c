/*
 * ===================================================================
 * ROUND 39 (head): _card_clear IS NOT GAME CODE.  DO NOT STAFF IT.
 * ===================================================================
 *
 * It is Psy-Q **libcard**.  Its object is simply not on the discs in `sdk/`,
 * which is why no placed object covers it and why `tools/sdkstalls.py`
 * correctly reports no overlap -- that tool answers "is this owned by an
 * object we HAVE", not "is this Sony's".  Three independent lines of
 * evidence, all re-measurable in seconds:
 *
 *  1. It is bracketed on BOTH sides by placed libcard objects and tiles
 *     their run exactly:  c172 _card_load @0x41308, c171 _card_info
 *     @0x41318, [this, 0x41328..0x41358], a78 _card_write @0x41358,
 *     a74 InitCARD, a75 StartCARD, c112 _bu_init, a80 _new_card.
 *  2. Its only two callees are `_new_card` and `_card_write` -- both libcard.
 *  3. Its word 5 is `addiu $a1,$zero,0x3F`, and that form appears EXACTLY
 *     ONCE in the whole executable's disassembly against 1089 `ori`:
 *         grep -rcE '\b(addiu|ori) +\$[a-z0-9]+, \$zero, 0x' asm/
 *     Sony's own objects in `lib/` carry both forms (226 addiu / 386 ori,
 *     because the game mixed library builds) and **libcard is 100% addiu,
 *     14 of 14**.  Our pipeline cannot emit that form at all: maspsx's
 *     `expand_load_immediate()` picks `ori` for every 0 < K < 0x10000 under
 *     the pinned `--aspsx-version=2.34`.
 *
 * So the round-32 "li-encoding residue is toolchain-unreachable" mechanism
 * is CORRECT and its conclusion was not: a rule that is wrong for 1
 * instruction in 1090 is not wrong, and the instruction came from Sony's
 * assembler.  Full evidence in docs/match-reports/_card_clear.md.
 *
 * Everything below this banner predates that finding.  It is kept because
 * the derivation is sound and because the comment's own history is the
 * point: every blocker screen passes this function (the BLOCKER PROFILE
 * line below is accurate), and at 12 words it sorts FIRST in
 * `nearmiss.py`'s ASSIGN FROM HERE.  It has been the most attractive
 * target in the queue for four rounds and is unmatchable by construction.
 */

/*
 * libcard_card_clear -- carved round 27 (2026-09-10).  ONE function,
 * _card_clear (12 words), file 0x41328..0x41358, vram 0x80050B28.
 *
 * A one-function unit is deliberate, not an accident of carving.  This
 * function is wedged between two PSX BIOS trampoline clusters inside the old
 * `class_3bb8c_h` segment -- 2 trampolines before it (now `class_3bb8c_h_b`)
 * and 5 after (now `class_3bb8c_h_c`) -- so the only way to make it
 * assignable without putting unmatchable trampolines into a C unit was to
 * give it its own segment.  See the splat yaml at `class_3bb8c_h` for why
 * the trampolines are being kept strictly in `asm` segments.
 *
 * BLOCKER PROFILE: CLEAN.  Re-screened 2026-09-10 with the two live greps
 * (`python3 tools/nearmiss.py`): zero `gp_rel`, zero `nop_mflo_mfhi`.  It was
 * `addiu_at`-blocked, and `addiu_at` was RESOLVED in round 21 -- do not
 * screen for it, and do not file a stall against it.
 *
 * NOT a class-table slot (`tools/classtable.py --scan` does not list it).
 *
 * THE SIGNATURE IS ALREADY ESTABLISHED -- DO NOT GUESS IT, AND DO NOT WRITE
 * YOUR OWN.  `src/class_3bb8c_e.c` already carries the canonical
 * declaration, from its own call site at line 296:
 *
 *     extern s32 _card_clear(s32 arg0);        // class_3bb8c_e.c:276
 *     ...
 *     _card_clear(self->unk10);                // class_3bb8c_e.c:296
 *
 * and documents `unk10` (+0x010) as "a resource handle passed to
 * _card_info / _card_load / _card_clear" -- the other two being the
 * 0xA0 BIOS trampolines either side of this function.  So this is a small
 * resource/handle call returning `s32`.  MATCH THAT DECLARATION rather than
 * writing a second one: a competing prototype for a function another unit
 * declares is the shared-header mistake that git does not mark, and it broke
 * round 15 four times.  Keep any declaration you need in THIS `.c`.
 *
 * 12 words -- this should close in one sitting, and then the unit is done.
 *   ^^^ WRONG, see the round-39 banner at the top of this file.  This is
 *       the sentence that made it attractive; it is kept so the next
 *       reader can see what a confident stale directive looks like.
 */

#include "common.h"

/* ROUND 36 (runner charlie): re-verified round 32's preserved 9/12 body
 * after round 34's SDK-object rename retargeted its two callees.  The old
 * names (func_80050B98/func_80050B58) no longer exist in this tree;
 * corrected to `_new_card`/`_card_write` (config/symbols.slps01556.lsdde.txt)
 * and rebuilt -- measured 9/12, exact length, no drift, identical to round
 * 32's recorded figure.  Confirmed genuinely stalled (li-encoding residue is
 * toolchain-unreachable, frame-offset residue resists all tried levers, see
 * the match report). Restored to INCLUDE_ASM per the no-score-short-of-
 * byte-exact rule; the corrected, linkable body is preserved in the report. */
INCLUDE_ASM("asm/nonmatchings/libcard_card_clear", _card_clear);
