# ReadCdFile -- MATCHED round 74 (56/56, length exact, whole image byte-exact)

REVISITED, round 74: MATCHED (56/56); names/types not relevant (existing
field names used unchanged; the lever was control flow and block layout).

> **ROUND 74 (2026-09-24), runner charlie -- MATCHED, four builds including the rebuild.**
>
> **Rebuild first.** The promoted `#ifdef NON_MATCHING` body, made live
> unchanged (OpenCdFile already matched and live): `build exit=2`, 8/56,
> `insertions 11 / deletions 11`, 48 positional skeleton diffs, built
> length 57/56 (one word long) -- matches the round-72 title.
>
> **Build 2 (outer loop as label+goto, early return kept):** 12/56, ins/del
> 11/11, still 57. The retry label moved but the cold `onError` block
> still sat right after the entry test, where retail puts it last.
>
> **Build 3 (`if (isOpen != 0) { retry: ... } else { onError; }
> return 0;`, `hi` computed before `CdControl`):** 20/56, **ins/del 1/1**,
> 57. The else-block now comes last, `hi == 0` falls into the shared
> `return 0` after `onError` (retail's `.L80028B40`), and `srl` lands in
> `CdControl`'s delay slot. The one remaining diff, per asm-differ: the
> CdSync wait loop's back-edge. Retail branches to `addu $a0,$zero,$zero`
> (the argument setup) with `li $v0,5` in the delay slot. Built branches to
> the `jal` with a COPY of `move $a0,$zero` stolen into the delay slot.
> That copy is the extra word.
>
> **Build 4 (the CdSync wait as label+goto too) -- MATCH.** 56/56,
> `insertions 0 / deletions 0`, `./build-and-verify.sh` OK,
> `tools/check-nonmatching.sh` green. The CdReadSync wait stays a
> `do/while`: its retail back-edge DOES target the `jal` with `a0 = 0`
> copied into the delay slot, i.e. the do-while shape.
>
> ```c
> s32 ReadCdFile(ObjA34_179D8H *self, char *arg1, s32 arg2) {
>     s32 hi;
>     s32 status;
>     char scratch[0x800];
>     char buf[0x10];
>
>     if (self->isOpen != 0) {
>     retry:
>         hi = (u32)arg2 >> 11;
>         CdControl(2, &self->pos, 0);
>     sync:
>         status = CdSync(0, buf);
>         if (status == 0) {
>             goto sync;
>         }
>         if (status == 5) {
>             goto retry;
>         }
>         if (hi != 0) {
>             CdRead(hi, arg1, 0x80);
>             do {
>                 status = CdReadSync(0, 0);
>             } while (status > 0);
>             if (status == -1) {
>                 goto retry;
>             }
>             return 0;
>         }
>     } else {
>         self->methods->onError(self);
>     }
>     return 0;
> }
> ```
>
> ### Proposed learning
>
> **You can tell a goto loop from a do-while in GCC 2.6.3 output by where
> the back-edge lands.** A `do { x = f(0, ..); } while (c);` back-edge
> targets the `jal`, and reorg copies the argument setup into the branch's
> delay slot, so that instruction appears twice (once before the loop, once
> in the slot). A `L: x = f(0, ..); if (c) goto L;` back-edge targets the
> argument setup itself, and the delay slot takes the next block's first
> instruction. When a stall is one word long and the extra word is a
> duplicated argument setup in a back-edge delay slot, rewrite that loop as
> a goto. Loops in the same function can differ: here two were gotos and
> one a do-while. Companion lever in OpenCdFile the same round: goto loops
> also get no loop-invariant hoisting.

---

(Historical record below; its title was: ReadCdFile -- STALL, NON_MATCHING body promoted round 72 (best: 8/56 words at length 57/56 [1 word long], structural / block-layout))


> **ROUND 72 (2026-09-23), runner charlie -- NON_MATCHING body promoted.**
> Per `docs/FINISHING-PLAN.md` track 1b: the "## Result" body (the one the
> round-64 note says is kept current against the present struct field
> names) is hand-derived across rounds 17/36/47/54 -- no permuter-found
> edit is in it; round 47's permuter check (b) explicitly DECLINED the
> search (insertions=5, deletions=4, reorderings=6, base score 1333), so
> nothing from a search ever entered this body. Live-measured this round
> under the current pinned maspsx flags (rounds 42/63) by making the body
> live C in place of `INCLUDE_ASM` (with `OpenCdFile` held at `INCLUDE_ASM`
> so the window isn't contaminated by that sibling stall's own drift) and
> reading `funcdiff.py`: **8/56 raw word-match, length 57/56 words (one
> word long)** -- `build/lsdde.map` confirms `NoOp4 - ReadCdFile = 0xE4` =
> 57 words against retail's 56, identical to the figure already in this
> report's title, so no title rewrite needed. Restored to the
> `#ifdef NON_MATCHING ... #else INCLUDE_ASM ... #endif` shape (not left as
> live C, since it is not byte-exact) with a comment carrying this score,
> its residue class (block-placement + register-role rotation), and this
> report. `./build-and-verify.sh` and `tools/check-nonmatching.sh` both
> green afterward; verified build bytes unchanged.

> Renamed from `func_80028A84` on 2026-09-21 (tools/rename.py). Address 0x80028a84.

> **ROUND 64 (2026-09-21), runner alpha -- field rename note.** `ObjA34_179D8H`'s
> fields were renamed this round: `unk0C` -> `isOpen`, `unk18` -> `pos`,
> `unk1C` -> `size`; `MethodsA34_179D8H`: `slot48` -> `onError`. All prose
> and code below this note PREDATES the rename and uses the old field names
> throughout (it is historical narrative, left as written); the
> `## Result` block's actual function body has been updated to compile
> against the CURRENT struct definitions in `src/cd/cd_driver.c` -- that is
> the one to splice if you pick this function up again.

> **ROUND 54 (2026-09-18), runner charlie -- rebuilt, then one more
> structural reshape, negative.**
>
> **Rebuild-before-trusting-the-score.** Spliced the preserved (round-36)
> body into `src/cd/cd_driver.c` in place of the `INCLUDE_ASM` unchanged and
> ran the real oracle: `build exit=2`, no compile-error grep hits,
> `funcdiff.py` reads 8/56, identical to the recorded figure; `build/lsdde.map`
> confirms the function is still one word (4 bytes) long. Restored
> immediately; `./build-and-verify.sh` -> `OK: build matches retail
> SLPS_015.56`.
>
> **One new reshape tried this round, on the "next axis" this report's
> round-36 entry explicitly left open** (a shape other than repositioning
> the same two blocks relative to each other): reading the actual retail
> asm's entry branch directly (`beqz v0, .L80028B28` where `v0 = self->unk0C`
> — i.e. "if unk0C == 0, jump PAST the whole loop to the slot48 block placed
> near the epilogue"), wrote the LITERAL CFG the branch encodes — a
> top-level forward `goto`, not an `if`/`return` wrapping the loop and not
> an `if`/`else`:
> ```c
> if (self->unk0C == 0) {
>     goto slot48;
> }
> do {
>     CdControl(2, &self->unk18, 0);
>     hi = (u32)arg2 >> 11;
>     do {
>         status = CdSync(0, buf);
>     } while (status == 0);
>     if (status == 5) {
>         continue;
>     }
>     if (hi == 0) {
>         return 0;
>     }
>     CdRead(hi, arg1, 0x80);
>     do {
>         status = CdReadSync(0, 0);
>     } while (status > 0);
>     if (status != -1) {
>         return 0;
>     }
> } while (1);
>
> slot48:
>     self->methods->slot48(self);
>     return 0;
> ```
> **Result: 3/56, with drift (248872 bytes outside range) and the frame's
> own register-SAVE ORDER at the very top of the function changed** (`sw
> s1,0x824` first in this attempt vs. retail's `sw s0,0x820` first) — worse
> than attempt 2 in this report's original body (also 3/56, also a changed
> frame), and no better than the recorded 8/56 best. The literal branch-shape
> transcription does not win over the "if (cond) {cold path; return 0;}
> everything-else" placement that scored 8/56, even though it encodes
> retail's actual branch polarity and target more directly. Reverted
> immediately, clean revert confirmed, `OK: build matches retail`.
>
> This is the THIRD distinct top-level shape tried for the `unk0C==0` guard
> (inline-if-then-return: 8/56 best; if/else wrapping the loop: 3/56; now
> top-level goto: also 3/56) — all three attempts that move the cold path
> OUT of its "natural" inline position score in the 3/56 band with a broken
> frame size, while only the inline placement reaches 8/56. That is now
> fairly strong evidence the frame-size regression is tied to WHERE the
> `slot48` call sits relative to the loop's own register allocation, not to
> the specific syntax (`if/else`, `goto`, or otherwise) used to place it
> there — i.e. this axis is close to exhausted, not merely under-explored.
> Restored to `INCLUDE_ASM`; `git status --porcelain` clean. Not re-running
> the permuter: round 47's check-(b) decline (insertions=5, deletions=4,
> reorderings=6, base score 1333) is unaffected — this round's reshape is a
> fourth data point in the SAME class (whole-basic-block placement +
> register-role rotation), not a new one.
>
> **Proposed learning:** when THREE independent syntactic transcriptions of
> the same two-block CFG (if/else, goto, and — implicitly, by comparison —
> the inline-guard form) all land in the same narrow score band with the
> same broken-frame symptom, that is itself the signal to stop varying
> syntax around the placement and treat the residue as confirmed structural
> (frame allocation reacting to block PLACEMENT, not to which C construct
> expresses it) rather than attempt a fourth rephrasing.

---

> **ROUND 36 (2026-09-12), runner charlie -- MEASURED, not just re-named.**
> Round 34's SDK-object conversion renamed this function's four callees
> (`func_80028DF0`->`CdControl`, `func_80028D68`->`CdSync`,
> `func_80029274`->`CdRead`, `func_80029254`->`CdReadSync`; all confirmed in
> `config/symbols.slps01556.lsdde.txt` and already declared, per-call-site
> typed, in `src/cd/cd_driver.c` itself). The preserved body below still
> spelled the old names and was never rebuilt under the new ones, so its
> 8/56 figure was carried forward UNVERIFIED (flagged by
> `tools/stalesyms.py`). Corrected the four names, spliced the body into
> `src/cd/cd_driver.c` in place of the `INCLUDE_ASM`, and ran the real
> oracle: `build exit=0`; the function compiles one word (4 bytes) longer
> than retail's 56, and `asm-differ` confirms it is the IDENTICAL
> structural residue this report already documents -- retail places the
> `self->unk0C == 0` / `slot48` cold path AFTER the main loop near the
> epilogue (reached by a forward branch), this body places it inline at the
> top, and the register-role rotation (`$s0`/`$s1` swapped relative to
> retail) is unchanged too. The unverified figure is now measured, and it
> did not move.
>
> Tried one additional, previously-untested reshape within budget, per this
> report's own "next axis" note: wrote the cold path as a single
> `return self->methods->slot48(self), 0;` expression instead of two
> statements. **Identical compiled length and shape** -- no improvement.
> Restored to `INCLUDE_ASM`; the corrected, linkable body is preserved
> below (replacing the stale-symbol version). Not attempting the loop
> restructuring this report's "next axis" note also floats (untested,
> lower-confidence) -- out of scope for this round's time budget.

Unit: `CdDriver`. Runner: echo, round 17 (second assignment). Restored to
`INCLUDE_ASM`.

## Class: structural (basic-block placement), plus an unresolved
register-role rotation similar to `OpenCdFile`'s stall in this same unit

Screened clean on both documented blockers. Confirmed via `tools/m2ctx.py
CdDriver --sig 's32 ReadCdFile(ObjA34_179D8H *self, char *arg1, s32
arg2)' --run`, whose independent reconstruction matches this report's
reading of the algorithm.

## What it does (high confidence)

CD-ROM read/retry loop: if `self->unk0C == 0`, dispatch through
`self->methods->slot48(self)` (a new vtable slot on the SAME
`ObjA34_179D8H` class this unit already established) and return 0.
Otherwise, loop: reset something via `func_80028DF0(2, &self->unk18, 0)`
(this unit's own `unk18` field, matched in `OpenCdFile`'s stall report),
poll `func_80028D68` (already matched elsewhere, in `libcd_bios.c`) until
it returns nonzero; on `5` specifically, restart the whole loop; on any
other nonzero, and only if the caller-supplied `arg2 >> 11` ("sector
count"?) is nonzero, kick off `func_80029274` (a retry-writer, already
matched in `libcd_bios.c`) and poll `func_80029254` (also matched there)
until it settles, returning 0 unless the settle value is exactly -1 (in
which case retry the whole outer loop again).

## Result (best, 8/56, `build exit=0`, size drift present)

```c
#if 0
/* MethodsA34_179D8H and ObjA34_179D8H are ALREADY declared earlier in
 * src/cd/cd_driver.c (current names: MethodsA34_179D8H::onError,
 * ObjA34_179D8H::isOpen/pos) -- do not re-paste this typedef when splicing,
 * only the function body below. Shown here again only so this block reads
 * standalone. */
typedef struct MethodsA34_179D8H {
    u8 pad000[0x48];
    void (*onError)(ObjA34_179D8H *self);
} MethodsA34_179D8H;
/* ObjA34_179D8H gets a `MethodsA34_179D8H *methods;` field at +0x000,
 * with the leading padding through +0xC unchanged in total size -- see
 * the struct definition already landed in src/cd/cd_driver.c. */

/* CdControl/CdSync/CdRead/CdReadSync (was func_80028DF0/func_80028D68/
 * func_80029274/func_80029254): Sony's, linked from lib/libcd/sys.o since
 * round 34's SDK-object conversion -- corrected round 36. */
extern void CdControl(s32 arg0, Pair16_179D8H *buf, s32 arg2);
extern s32 CdSync(s32 arg0, void *buf);
extern s32 CdRead(s32 arg0, void *arg1, s32 arg2);
extern s32 CdReadSync(s32 arg0, s32 arg1);

s32 ReadCdFile(ObjA34_179D8H *self, char *arg1, s32 arg2) {
    s32 hi;
    s32 status;
    char scratch[0x800];
    char buf[0x10];

    if (self->isOpen == 0) {
        self->methods->onError(self);
        return 0;
    }
    do {
        CdControl(2, &self->pos, 0);
        hi = (u32)arg2 >> 11;
        do {
            status = CdSync(0, buf);
        } while (status == 0);
        if (status == 5) {
            continue;
        }
        if (hi == 0) {
            return 0;
        }
        CdRead(hi, arg1, 0x80);
        do {
            status = CdReadSync(0, 0);
        } while (status > 0);
        if (status != -1) {
            return 0;
        }
    } while (1);
}
#endif
```

Stack layout notes worth keeping: frame is `-0x838`; the "arg shadow" for
outgoing calls occupies `sp+0x0..0x10`; `func_80028D68`'s `buf` argument is
`sp+0x810`, and since the frame's saved registers start at `sp+0x820`, the
region `sp+0x10..0x810` (a suspicious, exact `0x800` = one CD sector) is
otherwise unreferenced by this function's own instructions -- a `scratch[0x800]`
declared before the small `buf[0x10]` reproduces this layout (confirmed:
the `lwl`/`swl`-style address `sp+0x810` came out right with this
declaration order, matching `OpenCdFile`'s earlier finding that GCC
allocates locals low-to-high in DECLARATION order here).

## The residue (two distinct issues)

1. **Block placement: the `self->unk0C == 0` case (`self->methods->slot48`)
   is NOT inline where the source's `if` appears.** Retail places this
   entire block (5 instructions) AFTER the main loop, near the epilogue,
   reached via a forward `beqz` branch -- exactly as if it were the RARE
   path of an if/else where the loop is the primary body. Writing it as an
   `if (self->unk0C != 0) { ...loop... } else { slot48; return 0; }` (the
   "natural" placement matching retail's actual code layout) made the
   residue dramatically WORSE (3/56, plus the function's frame size itself
   changed -- `-0x838` became something else), not better. An explicit
   `goto` to a label placed after the loop gave the IDENTICAL 3/56 result
   as the if/else form. The INLINE-first form (`if (unk0C==0) {...; return
   0;} loop...`, matching what a human would write first) scored better
   (8/56) despite putting the code in the "wrong" position relative to
   retail -- suggesting GCC's decision about where to place this block is
   NOT controlled by simple source reordering the way it was for smaller
   functions earlier this round.
2. **Even in the closer (8/56) attempt, register roles differ from retail**
   in the same style as `OpenCdFile`'s stall in this unit: which
   callee-saved register holds `self` vs. the loop's other live values
   does not match, and the frame's REGISTER SAVE ORDER at the top of the
   function differs (`sw s0` vs `sw s1` first).

## Attempts (3, all build-verified)

1. `if (unk0C == 0) { slot48; return 0; } do { ...loop... } while (1);`
   (INLINE placement, matching source-order-first-thing-checked) -- 8/56,
   BEST. Frame size and instruction shape closest to retail, but block
   order and register roles both differ.
2. `if (unk0C != 0) { do {...loop...} while(1); } slot48; return 0;`
   (else-clause placement, matching retail's apparent block ORDER) -- 3/56,
   WORSE. Also changed the frame size unexpectedly (`ra` no longer saved
   the same way), suggesting GCC treated the tail-position `slot48` call
   differently (possibly eligible for some tail-call-adjacent
   simplification that removed the need to preserve `$ra` the same way).
3. `if (unk0C == 0) goto no_disc; do {...} while(1); no_disc: slot48;
   return 0;` (explicit goto to a label after the loop) -- identical 3/56
   to attempt 2; GCC treats it the same as the if/else form.

**Given the SEVERE structural gap between attempts 1 and 2/3 (a whole
`-0x838` vs. different frame size), this function needs a genuinely
different source shape than any of the three tried -- possibly the
`self->unk0C == 0` check needs to be written differently again (e.g. an
early `if (...) return self->methods->slot48(self), 0;`-style single
expression, or the loop itself needs restructuring to not be a `do
{...} while(1)` with `continue`) rather than just repositioning the same
two blocks relative to each other.** This was not tried due to time; noted
as the next axis to explore.

### Proposed learning

For a function shaped like "if (rare condition) { short cold path; return;
} main loop;", do NOT assume the source-order-first placement (matching how
a human would naturally write it) is closer to retail just because it
scored numerically better here -- check whether the OTHER placement changed
something as fundamental as the frame size before concluding the first
form is "more correct." A frame-size change on an otherwise-equivalent
restructuring is a strong signal that the SPECIFIC C shape (not just block
order) needs more work, not that the first attempt was closer to done.

---

## Round 47 (2026-09-16), runner delta -- rebuilt in-tree, then permuter DECLINED on check (b)

**Rebuild-before-trusting-the-score.** Spliced the preserved (round-36)
body into `src/cd/cd_driver.c` in place of the `INCLUDE_ASM` and ran the
real oracle: `build exit=2`, no compile-error grep hits, `funcdiff.py`
reads **8/56 raw word-match**, identical to the recorded figure.
`build/lsdde.map` (`NoOp4 - ReadCdFile = 0xE4` = 57 words)
confirms the function is still exactly one word (4 bytes) longer than
retail's 56, matching round 36's own re-verification exactly. Restored to
`INCLUDE_ASM` immediately after; diffed the restored file byte-for-byte
against the pre-splice copy (identical) and `./build-and-verify.sh`
confirms `OK: build matches retail SLPS_015.56`.

**Permuter pre-checks -- (a) and (b) run, (c) not reached:**

- **(a) scaffold compiles and scores:** yes (one harmless "unused variable
  `scratch`" warning -- `scratch` exists only to reproduce retail's stack
  layout, per this report's own stack-layout note above, and is never
  read).
- **(b) insertion/deletion penalties, `--debug --stack-diffs`:** **NOT**
  near 0/0, and further from it than `OpenCdFile`'s sibling residue in
  this same unit. Measured: `Insertions: 5 (100)`, `Deletions: 4 (100)`,
  `Reorderings: 6 (60)`, `Register Differences: 13 (5)`, `Stack
  Differences: 8 (1)`, **base score = 1333**. Consistent with this
  report's own finding that the gap between attempts is a whole
  BASIC-BLOCK-PLACEMENT difference (the cold path lives after the loop in
  retail, at the top in every attempt that scored closer) plus the
  register-role rotation already documented -- not something a
  same-length-ish expression-tree mutation search is shaped to reach.
- **(c) scaffold-vs-real-build agreement:** not run as a separate command;
  the in-tree rebuild above (one word long, 8/56, `build exit=2` matching
  the round-36 figure exactly) and the permuter's own base score both
  measure the SAME already-on-file residue rather than a contradictory
  one, so there is nothing to flag as a scaffold/real-build disagreement.

**Verdict: search DECLINED**, for the same reason as `OpenCdFile` in
this unit and made stronger by the larger insertion/deletion/reordering
counts here: this is a block-placement (whole basic block relocated
relative to the loop) plus register-role-rotation residue, which is a
CONTROL-FLOW-SHAPE gap, not an expression-tree rewrite a source-mutation
search is built to close. Recorded as NOT SEARCHED (declined on evidence
from check (b)), not as a spent, failed search.

---

## Naming (round 64, runner alpha)

- **`func_80028A84` -> `ReadCdFile`, tier B.** Mechanics: if not open,
  dispatches the object's own error/failure slot and returns; otherwise
  loops issuing `CdControl`/`CdSync` then `CdRead`/`CdReadSync` to fill the
  caller's buffer. Confirmed by `src/cd/cd_driver.c`'s `CdDriver__Read`,
  which calls this function directly when CD-async mode is off and
  otherwise reimplements the identical `CdRead`/`CdReadSync` retry loop for
  its async path. Paired with `OpenCdFile`/`CloseCdFile`/`GetCdFileSize`
  (also this unit) as an Open/Close/Size/Read quad.
- `MethodsA34_179D8H::slot48` -> `onError` (tier B): the slot NUMBER (+0x48)
  matches `src/cd/cd_driver.c`'s own independent view
  (`Methods80027480::slot48`), dispatched there on an unrelated
  allocation-failure path (`CdDriver__LoadFile`) -- two unrelated give-up paths
  at the identical offset. Not applied in `cd_driver.c` (out of unit);
  PROPOSED there under the same name. Recorded in full in
  `src/cd/cd_driver.c`'s own field comment and in `OpenCdFile.md`.

## Proposed field names

- `src/cd/cd_driver.c`'s `Methods80027480::slot48` (its own independent
  local view of what appears to be the SAME table this unit calls through
  `MethodsA34_179D8H`) -> `onError`, tier B. Same evidence as above: two
  unrelated give-up paths (this unit's `ReadCdFile` on "not open",
  `cd_driver.c`'s own `CdDriver__LoadFile` on allocation failure) dispatch the
  identical slot number. `cd_driver.c` is out of unit and not staffed
  this round; posted to the broadcast for the head to apply at merge time
  per FINISHING-PLAN.md track 3 step 3 (rename the field in the struct
  DEFINITION only, rebuild, fix exactly the accessors the compiler lists).

## Round 97 (runner bravo): Sony's `<libcd.h>` prototypes

The unit's local `CdControl`/`CdSync`/`CdRead`/`CdReadSync` declarations were
replaced by `<libcd.h>`'s. Call sites now read
`CdControl(CdlSetloc, (u_char *)&self->pos, 0)` (`CdLoc16` is the project's
spelling of `CdlLOC`) and `CdRead(hi, (u_long *)arg1, CdlModeSpeed)`; the
`CdSync` result buffer is `u_char buf[0x10]` (same size, same frame).
Byte-exact; whole-image SHA1 green.

## Source comment history (round 99, echo, track 7)

The comment above `ReadCdFile` in `src/code_179d8_h.c` before round 99's
track 7 pass, kept verbatim; the source now keeps `MATCHING:` lines. Names
as of round 98 (`arg1`/`arg2` are now `buf`/`size`, `hi` is `sectors`, the
CdSync result buffer `buf` is `syncResult`).

```c
/* MATCHED round 74 (charlie). Two of its three loops are label + goto
 * (the seek retry and the CdSync wait); only the CdReadSync wait is a
 * do-while. The loop kind is readable from the back-edge: a do-while's
 * branch targets the jal with the argument setup copied into its delay
 * slot, a goto loop's branch targets the argument setup itself --
 * docs/match-reports/ReadCdFile.md. `scratch` is never touched; it only
 * sizes the frame (retail's `buf` sits at sp+0x810). */
```

## Naming (round 99, echo, track 7)

- Parameters `arg1`/`arg2` -> `buf`/`size`, tier A: `CdDriver__Read`
  (`src/cd/cd_driver.c`) passes its own `buf`/`size` straight through, and
  the body hands `buf` to `CdRead` and shifts `size` down to a sector count.
  `buf` is `void *`, as the caller's declaration has it (was `char *`; zero
  bytes changed).
- Locals: `hi` -> `sectors` (the `CdRead` sector count), `buf` ->
  `syncResult` (`CdSync`'s result buffer).
- Constants: `>> 11` -> `>> CD_SECTOR_SHIFT` (`include/cd_driver.h`); the
  `CdSync` results `0`/`5` -> `<libcd.h>`'s `CdlNoIntr`/`CdlDiskError`.
  `CdReadSync`'s `-1` (error) stays a literal. `scratch[2048]` and
  `syncResult[16]` stay literals, decimal: `scratch` is never touched and
  only sizes the frame, so a sector name would claim a use nobody has seen.
- Left: the `(u_char *)&self->pos` cast. `CdControl`'s parameter is Sony's
  `u_char *`, and `pos` is the project's `CdLoc16` (file_resource.h) rather
  than `CdlLOC`; the cast goes when track 6 gives FileResource Sony's type.
