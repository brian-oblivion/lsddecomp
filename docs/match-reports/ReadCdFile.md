# ReadCdFile -- STALL (best: 8/56 words at length 57/56 [1 word long], structural / block-layout)

> Renamed from `func_80028A84` on 2026-09-21 (tools/rename.py). Address 0x80028a84.

> **ROUND 64 (2026-09-21), runner alpha -- field rename note.** `ObjA34_179D8H`'s
> fields were renamed this round: `unk0C` -> `isOpen`, `unk18` -> `pos`,
> `unk1C` -> `size`; `MethodsA34_179D8H`: `slot48` -> `onError`. All prose
> and code below this note PREDATES the rename and uses the old field names
> throughout (it is historical narrative, left as written); the
> `## Result` block's actual function body has been updated to compile
> against the CURRENT struct definitions in `src/code_179d8_h.c` -- that is
> the one to splice if you pick this function up again.

> **ROUND 54 (2026-09-18), runner charlie -- rebuilt, then one more
> structural reshape, negative.**
>
> **Rebuild-before-trusting-the-score.** Spliced the preserved (round-36)
> body into `src/code_179d8_h.c` in place of the `INCLUDE_ASM` unchanged and
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
> typed, in `src/code_179d8_h.c` itself). The preserved body below still
> spelled the old names and was never rebuilt under the new ones, so its
> 8/56 figure was carried forward UNVERIFIED (flagged by
> `tools/stalesyms.py`). Corrected the four names, spliced the body into
> `src/code_179d8_h.c` in place of the `INCLUDE_ASM`, and ran the real
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

Unit: `code_179d8_h`. Runner: echo, round 17 (second assignment). Restored to
`INCLUDE_ASM`.

## Class: structural (basic-block placement), plus an unresolved
register-role rotation similar to `OpenCdFile`'s stall in this same unit

Screened clean on both documented blockers. Confirmed via `tools/m2ctx.py
code_179d8_h --sig 's32 ReadCdFile(ObjA34_179D8H *self, char *arg1, s32
arg2)' --run`, whose independent reconstruction matches this report's
reading of the algorithm.

## What it does (high confidence)

CD-ROM read/retry loop: if `self->unk0C == 0`, dispatch through
`self->methods->slot48(self)` (a new vtable slot on the SAME
`ObjA34_179D8H` class this unit already established) and return 0.
Otherwise, loop: reset something via `func_80028DF0(2, &self->unk18, 0)`
(this unit's own `unk18` field, matched in `OpenCdFile`'s stall report),
poll `func_80028D68` (already matched elsewhere, in `code_179d8_b.c`) until
it returns nonzero; on `5` specifically, restart the whole loop; on any
other nonzero, and only if the caller-supplied `arg2 >> 11` ("sector
count"?) is nonzero, kick off `func_80029274` (a retry-writer, already
matched in `code_179d8_b.c`) and poll `func_80029254` (also matched there)
until it settles, returning 0 unless the settle value is exactly -1 (in
which case retry the whole outer loop again).

## Result (best, 8/56, `build exit=0`, size drift present)

```c
#if 0
/* MethodsA34_179D8H and ObjA34_179D8H are ALREADY declared earlier in
 * src/code_179d8_h.c (current names: MethodsA34_179D8H::onError,
 * ObjA34_179D8H::isOpen/pos) -- do not re-paste this typedef when splicing,
 * only the function body below. Shown here again only so this block reads
 * standalone. */
typedef struct MethodsA34_179D8H {
    u8 pad000[0x48];
    void (*onError)(ObjA34_179D8H *self);
} MethodsA34_179D8H;
/* ObjA34_179D8H gets a `MethodsA34_179D8H *methods;` field at +0x000,
 * with the leading padding through +0xC unchanged in total size -- see
 * the struct definition already landed in src/code_179d8_h.c. */

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
body into `src/code_179d8_h.c` in place of the `INCLUDE_ASM` and ran the
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
  caller's buffer. Confirmed by `src/code_179d8_s.c`'s `func_800276D0`,
  which calls this function directly when CD-async mode is off and
  otherwise reimplements the identical `CdRead`/`CdReadSync` retry loop for
  its async path. Paired with `OpenCdFile`/`CloseCdFile`/`GetCdFileSize`
  (also this unit) as an Open/Close/Size/Read quad.
- `MethodsA34_179D8H::slot48` -> `onError` (tier B): the slot NUMBER (+0x48)
  matches `src/code_179d8_s.c`'s own independent view
  (`Methods80027480::slot48`), dispatched there on an unrelated
  allocation-failure path (`func_80027800`) -- two unrelated give-up paths
  at the identical offset. Not applied in `code_179d8_s.c` (out of unit);
  PROPOSED there under the same name. Recorded in full in
  `src/code_179d8_h.c`'s own field comment and in `OpenCdFile.md`.
