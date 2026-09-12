/*
 * code_179d8_n -- functions 80..82 of the original 274-function code_179d8
 * monolith, 0x1A1BC..0x1AB78 (vram 0x800299BC..0x8002A378).  Carved round 26
 * (2026-09-09) out of what had been the `code_179d8_mid` uncarved remainder;
 * renamed on carve because "mid" named a REMAINDER and this is no longer one
 * -- the whole remainder was consumed, so no `code_179d8_mid` segment exists
 * any more.  Do not look for one.
 *
 * Blocker census at carve time, four screens per function (gp_rel,
 * forward nop_mflo_mfhi, `jr $t2` trampoline, jtbl):
 *   func_800299BC (161w)  CLEAN
 *   func_80029C40 (180w)  CLEAN
 *   func_80029F10 (282w)  CLEAN
 * 3 of 3 clean, zero trivial `jr $ra` leaves.  These are BIG bodies, so this
 * unit is far larger in work than 3 suggests -- budget accordingly.
 *
 * Owns NO switch jump table (zero `jtbl_` references in the slice), so no
 * rodata sub-slot is attached to this unit and the 0x120C slot in the splat
 * yaml stays standalone.
 *
 * It DOES reference plain rodata SYMBOLS: D_80010984, D_80010994,
 * D_80010A0C, D_80010A14, D_80010A20, D_80010A28, D_80010A38.  Several of
 * those are ASCII.  They are SYMBOLS to reference (`extern const char
 * D_XXXXXXXX[];`), never strings to re-type as C literals -- splat has
 * already emitted those bytes and a literal emits a second copy, which
 * shifts the whole image (see CLAUDE.md, the duplicated-rodata-string
 * trap).
 *
 * This unit's own extern declarations are kept LOCAL to this file per the
 * project's multiple-independent-local-views convention.  It shares no
 * project header with any other unit.
 *
 * Round 26 (echo): all three functions were worked to near-misses and
 * STALLED -- see docs/match-reports/func_800299BC.md (162/161, 1 word LONG),
 * func_80029C40.md (178/180, 2 words short) and func_80029F10.md (278/282,
 * 4 words short).  Every residue is an already-characterized GCC 2.6.3
 * quirk (a hoisted-constant register choice, dead-code-eliminated redundant
 * masks, and delay-slot/addressing-mode scheduling) documented in
 * docs/DECOMPILATION_LEARNINGS.md as not fixable by hand C restructuring --
 * read the three reports before re-attempting; they carry the full
 * near-miss bodies and the exact levers already tried.
 *
 * Round 35 (echo): re-screened round 26's stalled bodies against the
 * current tree before re-attempting -- per CLAUDE.md's "BUILD any inherited
 * body ONCE" discipline. Four of the six `extern func_XXXXXXXX` helper
 * declarations round 26's preserved bodies used are now STALE placeholder
 * names -- SDK-object rounds since renamed them to their real Sony symbols
 * (`func_80025900` -> `VSync`, `func_80025AE4` -> `puts`,
 * `func_80024E64` -> `CheckCallback`, `func_80012C20` -> `printf`; see
 * `src/code_179d8_g.c`'s own already-updated local declarations for the
 * same globals). Round 26's bodies, spliced verbatim, would not have LINKED
 * under today's tree -- the "a preserved body's `jal` targets can go STALE
 * across an SDK-object round" hazard from round 31 (DECOMPILATION_LEARNINGS.md).
 * `func_8002A510` and `func_80029478` were NOT renamed (still real game
 * code, still INCLUDE_ASM/matched under those names in sibling units).
 *
 * All three bodies were rebuilt this round with the four names corrected,
 * plus a couple of quick untried levers per rounds 31/33's newer findings
 * (routing the func_800299BC hoisted-constant "2" through a separate named
 * local; reading func_80029C40's two flag bytes through a `volatile u8 *`
 * cast). All three REPRODUCED their round-26 recorded scores exactly
 * (162/161, 178/180, 278/282) and neither new lever moved anything --
 * consistent with round 33's finding that a GCSE/value-availability hoist
 * (func_800299BC's case) and a genuinely-redundant-mask DCE (func_80029C40's
 * case, independently re-confirmed here against the live `.s` rather than
 * inherited from a citation to the now-SDK-owned `func_8002B94C`) are both
 * immune to source-level rescue by construction, not by insufficient
 * effort. Re-filed as STALLS; see the three match reports for the updated
 * verdicts and the round-35 addenda.
 */
#include "common.h"

INCLUDE_ASM("asm/nonmatchings/code_179d8_n", func_800299BC);

INCLUDE_ASM("asm/nonmatchings/code_179d8_n", func_80029C40);

INCLUDE_ASM("asm/nonmatchings/code_179d8_n", func_80029F10);
