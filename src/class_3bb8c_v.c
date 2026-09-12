/*
 * class_3bb8c_v -- carved round 27 (2026-09-10).  ONE function,
 * func_80050B28 (12 words), file 0x41328..0x41358, vram 0x80050B28.
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
 *     extern s32 func_80050B28(s32 arg0);        // class_3bb8c_e.c:276
 *     ...
 *     func_80050B28(self->unk10);                // class_3bb8c_e.c:296
 *
 * and documents `unk10` (+0x010) as "a resource handle passed to
 * _card_info / _card_load / func_80050B28" -- the other two being the
 * 0xA0 BIOS trampolines either side of this function.  So this is a small
 * resource/handle call returning `s32`.  MATCH THAT DECLARATION rather than
 * writing a second one: a competing prototype for a function another unit
 * declares is the shared-header mistake that git does not mark, and it broke
 * round 15 four times.  Keep any declaration you need in THIS `.c`.
 *
 * 12 words -- this should close in one sitting, and then the unit is done.
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
INCLUDE_ASM("asm/nonmatchings/class_3bb8c_v", func_80050B28);
