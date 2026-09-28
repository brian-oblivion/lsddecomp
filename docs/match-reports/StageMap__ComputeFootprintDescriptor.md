# StageMap__ComputeFootprintDescriptor -- MATCH (106/106 words, ins 0 / del 0, exact length)

> Renamed from `Class866E8__ComputeFootprintDescriptor` on 2026-09-26 (tools/rename.py). Address 0x8004c1c0.

> Renamed from `func_8004C1C0` on 2026-09-24 (tools/rename.py). Address 0x8004c1c0.

REVISITED, round 63: MATCHED 106/106, whole-image SHA1 green; names/types used
(the existing `Descriptor10Ext` / `SplitLongVec3` / `SplitCoord2` declarations
were correct and unchanged -- the stall was purely source SHAPE).

> **ROUND 63 (delta): MATCHED, 72/106 -> 106/106 in five builds.** This
> function had been re-verified at 72/106 in rounds 19, 20, 27, 32, 39, 40, 41
> and 47 with the residue attributed to two classes -- "$a1-vs-$a3 register
> identity" and "a GCC 2.6.3 store-then-reread sign-extension quirk". **Both
> attributions were wrong, and so was the claim that the in-context build has
> ZERO insertions and deletions.** All three are corrected below.
>
> ### Step (a): the inherited body reproduces its FIGURE and falsifies its CAUSE
>
> `stalesyms.py` first: this report has **no stale callee names**, so the
> preserved body links as written and step (a)'s number is trustworthy.
>
> Rebuilt the preserved `#if 0` body verbatim:
>
> ```
> StageMap__ComputeFootprintDescriptor: 72/106 words match (file 0x3C9C0-0x3CB68)
> StageMap__ComputeFootprintDescriptor: insertions 12 / deletions 12
> ```
>
> 72/106 reproduces exactly and there is no drift, so the recorded title's
> "no drift" is right and **plan.py's `len-off` tag was wrong** (plan.py
> derives the tag from the title text, and this title says "no drift" rather
> than naming a length). Length is EXACT: 106 words both sides.
>
> **ins/del 12/12 confirmed REAL by asm-differ**, per round 63's caution about
> the false-positive mode on repeating instruction skeletons: the diff shows
> four literal `>` markers (`sll v0,t0,0x18`, `sra v0,v0,0xd`,
> `sll a1,a1,0x18`, `sra a1,a1,0xd`) and four literal `<` markers
> (`lb v0,2(s0)`, `sll v0,v0,0xb`, `sw s3,0x24(s0)`, `addiu a0,a0,-0x400`).
> Markers present => not the artifact. So rounds 32 and 40's statement that
> "the real in-context build has ZERO insertions or deletions (72/106 is
> purely register identity plus one reread choice)" is **FALSIFIED** -- and
> that belief is what made both rounds discard their permuter scaffolds for
> "not reproducing the real residue" when the scaffolds' own 9/9 was in fact
> the *closer* reading of the two.
>
> ### The three real defects, each isolated through the pinned pipeline
>
> **1. Sign extension: an `s8` field must go through an `s32` local.**
> Retail reads `out->base.b2` back from memory with `lb` and shifts it with a
> single `sll 0xb`. Five forms measured side by side (`cpp|cc1|maspsx|as`,
> flags read from the Makefile):
>
> | source form | codegen |
> | --- | --- |
> | `o->b2 << 11` (field in the expression) | `lbu` + `sll 0x18` + `sra 0xd` |
> | `s8 t = o->b2; t << 11` | `lbu` + `sll 0x18` + `sra 0xd` |
> | `(s32)o->b2 << 11` (cast at the site) | `lbu` + `sll 0x18` + `sra 0xd` |
> | **`s32 t = o->b2; t << 11`** | **`lb` + `sll 0xb`** (retail) |
> | **`o->b2 * 2048`** | **`lb` + `sll 0xb`** (retail) |
>
> The cast does NOT work; only a real `s32` *object* does. This is the exact
> opposite of what this report's round-27 entry concluded, which dropped the
> locals and re-read the field directly -- the right move paired with the
> wrong type, which is why it regressed and why the (correct) idea was then
> abandoned for four rounds.
>
> **2. The `0x400` belongs INSIDE the subtracted group.** The inherited body
> wrote `(in->unk0.h - 0x400) - (u14b->unk18.h + (b2 << 11))`, which compiles
> to `li v1,0xfc00` + `addu` -- GCC narrows the constant to HImode (because
> the destination is a `s16` field and the source a `u16` one) and 0xfc00 then
> reads as the *unsigned* 64512, which no longer fits an `addiu` immediate.
> Writing the same arithmetic as
> `in->unk0.h - (u14b->unk18.h + (b2 << 11) + 0x400)` makes GCC reassociate
> and emit retail's `addiu a0,a0,-0x400`. Measured in isolation: this form
> reproduces retail's eight-instruction h4 block **instruction for
> instruction and in order**.
>
> **3. `out->unk24 = e;` is the LAST statement of the block.** With 1 and 2
> applied the score was 92/106, ins 2/del 2, the only residue being that
> retail schedules `sw s3,0x24(s0)` between the `sll` and the `addiu` of the
> h8 computation while the build hoisted it up next to the `lb`. Eight
> placements of that one statement were swept (one build each, the rest of
> the body byte-identical):
>
> | placement of `out->unk24 = e;` | score | ins/del |
> | --- | --- | --- |
> | first in the tail | 83/106 | 2/2 |
> | after the h4 store | 83/106 | 2/2 |
> | before the h6 store | 83/106 | 2/2 |
> | before `b3 = out->base.b3` | 92/106 | 2/2 |
> | after `b3 = ...`, before h8 | 92/106 | 2/2 |
> | with `b3` hoisted above h4 (3 forms) | 80-82/106 | 3/3-4/4 |
> | **after the h8 store (last)** | **106/106** | **0/0** |
>
> **The `$a1`-vs-`$a3` "register identity" residue was never a register
> problem.** It disappeared without being touched: `u14b` moved `$a3` -> `$a2`
> when the `s8` locals were dropped, and `$a2` -> `$a1` when the statement
> order was corrected. It was a symptom of the two caller-saved registers the
> `s8 b2`/`s8 b3` locals were occupying across that span, and of the schedule.
> **No register pinning and no operand constraint was used or needed.**
>
> Oracle: `build exit=0`, `OK: build matches retail SLPS_015.56`, funcdiff
> 106/106 ins 0/del 0, no drift. Unit `class_3bb8c` INCLUDE_ASM count 6 -> 5.
>
> ### Proposed learning
>
> **A stall attributed to "register identity" that ALSO carries nonzero
> insertions/deletions is mis-attributed, and the ins/del figure is the
> cheapest thing in the loop that says so.** Register identity is 0/0 by
> definition. Eight rounds re-verified this function's 72/106 and none
> recorded an ins/del figure, so the instruction-count difference sitting in
> the diff the whole time was never read. Confirm with asm-differ's `<`/`>`
> markers first (round 63 measured a false-positive mode on repeating loop
> skeletons), but when the markers are there, the recorded cause is wrong and
> the function is ordinary shape work.
>
> **Corollary, and it is the expensive one: a scaffold judged "unreliable"
> against a WRONG in-context figure was judged against nothing.** Rounds 32
> and 40 each built a permuter scaffold, measured 9 insertions / 9 deletions,
> compared that against a believed in-context 0/0, concluded the scaffold did
> not reproduce the real function, and declined to search -- twice, four
> rounds apart. The real in-context figure was 12/12. The scaffolds were
> broadly right and were thrown away on the strength of a number nobody had
> measured. Gate 3 check 3 compares the scaffold's `--stack-diffs` against
> funcdiff's ins/del; that comparison is only as good as the in-context side,
> so **measure the in-context ins/del before discarding a scaffold.**
>
> **Two reusable source-shape idioms** (also posted to broadcast):
> (a) to make GCC 2.6.3 fold a narrow signed field's sign extension into the
> load (`lb`, not `lbu` + `sll 0x18` + `sra 0xd`), assign the field to a
> **`s32` local**; an explicit `(s32)` cast at the use site does NOT do it,
> and an `s8` local does not either. A `* (1 << n)` multiply works as well as
> the local.
> (b) when retail shows `addiu rX,rX,-K` but the build shows
> `li rY,<64K-K>` + `addu`, the constant has been narrowed to HImode; move the
> constant inside the subtracted parenthesised group (`x - (y + K)` instead of
> `(x - K) - y`) and GCC reassociates it back out at full width.


> **ROUND 47 (charlie): Gate 1b re-verified 72/106, no drift** (rebuild via
> `make clean && make extract` then the standard `#if 0`->`#if 1` swap).
> Identical two-class residue (`$a1`/`$a3` register identity for `u14b`;
> the `lb`-vs-value-propagation sign-extend class), unchanged from round
> 41.
>
> **This function is one of the FIVE `class_3bb8c`/`Obj866E8`-family
> members round 46 confirmed have a scaffold-vs-real-build MISMATCH**
> (round 32's own scaffold: 9 insertions/9 deletions isolated vs 0/0 in
> context; round 40 rebuilt an independent scaffold from scratch and got
> the SAME 9/9 mismatch). No search has ever been trusted here either --
> both rounds correctly declined to search. As with `StageMap__ApplyChunkLoads`, there
> is no inherited "permuter tried, negative" to revert to UNKNOWN, because
> none was ever recorded as trustworthy in the first place. Family count
> now stands at 5 confirmed mismatches (`StageMap__ApplyChunkLoads`, `StageMap__ComputeFootprintDescriptor`,
> `StageMap__SplitFootprintRect`, `StageMap__BuildFootprintRects`, `IsPointOutOfBounds`).
>
> Checked the 12th lever (hoist a field pair used on every path into
> locals PER `if`) against both residue classes: does not apply to either.
> Class 1 (`u14b` register identity) is straight-line, no `if` nearby.
> Class 2 (the `out->unkC`/`out->unk14` reload-after-store pattern feeding
> `out->unk18`/`out->unk20`) is also straight-line arithmetic with no
> conditional at all -- there is no `if` whose condition and body could
> share a hoisted pair. Neither is the lever's shape.
> **Disposition unchanged: 72/106.** No new attempt made; `INCLUDE_ASM`
> untouched throughout.

> **ROUND 41 (alpha): Gate 1b re-verified 72/106, no drift; dead-reload
> lever screened by hand per this round's staffing instruction, does NOT
> apply -- no new attempt.** Rebuilt the exact preserved body from a clean
> `INCLUDE_ASM` baseline first: confirmed 72/106, exact length, identical
> two-class residue (`$a1`/`$a3` for `u14b`; the `lb`-vs-value-propagation
> sign-extend class), unchanged from round 40.
>
> **Checked this body specifically for the "already-held value reloaded a
> second time into the same local" shape** (the lever that closed real
> residue on `StageMap__LoadChunksAround`/`StageMap__PopulateSlotCells` last round). `u14a` and `u14b`
> both come from `e->unkC->unk14`-shaped expressions, but they are
> DIFFERENT VALUES: `u14a = self->methods->slot118(self, e->unk4->unk32)
> ->unkC->unk14` (a *different* `Elem`, resolved via a key lookup) and
> `u14b = e->unkC->unk14` (the original `e` directly) -- confirmed by
> re-reading this report's own "What it does" section and the body itself,
> not just the variable names. There is no point where the SAME expression
> is computed, used, and then recomputed into the same local for a second
> use; `u14a` and `u14b` are each assigned exactly once. The lever's
> precondition is absent, so this residue is NOT a dead-reload instance --
> it is the two independently-confirmed classes this report already
> documents (register identity for `u14b`, and a GCC 2.6.3 store/reread
> sign-extension quirk, both isolated with reproducers across 5 prior
> rounds). Not attempted further this round; time went to the closer
> targets (`StageMap__FindSlotForPosition`, permuter search) instead, per this round's
> staffing priority.

> **ROUND 40 (bravo): Gate 1b re-verified 72/106, no drift; fresh permuter
> scaffold built, independently RE-CONFIRMS round 32's scaffold-unreliable
> finding, no search run.** Rebuilt the exact preserved body per Gate 1b:
> confirmed 72/106, no drift, identical two-class residue (`$a1`/`$a3`
> register identity for `u14b`; the `lb`-vs-value-propagation sign-extend
> class).
>
> Built a brand-new scaffold from scratch (`tools/setup-permuter.sh
> StageMap__ComputeFootprintDescriptor <seed>`, seed = this report's own 72/106 body verbatim) and
> ran `--debug --stack-diffs` before searching: **base score 2040, with 9
> insertions and 9 deletions** -- matching round 32's exact finding (9/9)
> on an independently-built scaffold, four rounds later. The real
> in-context build has ZERO insertions or deletions (72/106 is purely
> register identity plus one reread-vs-value-propagate choice, no
> missing/extra instructions anywhere). This function's isolated-compile
> residue is not close to the real one, confirmed twice now with two
> different scaffold builds -- not a one-off fluke.
>
> **No search was run** -- per the project's own rule, a result against a
> scaffold provably scoring a different residue would not transfer, and
> this scaffold's mismatch (9 vs 0 insertions/deletions) is even larger
> than `StageMap__ApplyChunkLoads`'s sibling case this same round. Not attempted
> further this round; time went to the three never-searched functions in
> this unit instead. **Both of this unit's "searched" functions
> (`StageMap__ApplyChunkLoads`, `StageMap__ComputeFootprintDescriptor`) turn out to have scaffolds that do not
> reproduce their real residue** -- worth flagging as a proposed learning:
> a function whose match report says "permuter searched" should also say
> whether the scaffold's `--debug` score was ever validated against the
> real build's residue, because two of this unit's three "searched"
> entries were not.

> **ROUND 39 (charlie): re-verified 72/106, no drift; this round's headline
> "hoist both before either" lever checked against retail's own instruction
> order and found NOT APPLICABLE.** Rebuilt the exact preserved body per
> Gate 1b: confirmed 72/106, identical residue, same diff window
> (`0x8004C280`-`0x8004C338`). The pre-screen flagged 12 adjacent load/mult
> pairs for this function, but reading retail's own `.s` shows `u14a`
> (`e->unkC->unk14`'s use at `0x8004C24C`) and `u14b` (the SAME expression,
> loaded again at `0x8004C280`) are **34 bytes apart with `u14a`'s entire
> consumption (`out->unkC`/`unk10`/`unk14`, three stores) in between** --
> retail loads `u14b` LAZILY, right at its own first use, not adjacently to
> `u14a`. This is the mirror case of `StageMap__FindSlotForPosition`'s finding: the
> diagnostic ("are the two loads adjacent, with consumers later?") answers
> NO here, so the lever's own precondition is unmet and no hoist-based
> restructuring of `u14a`/`u14b` was attempted this round. The two residue
> classes below (`$a1`/`$a3` register identity for `u14b`, and the
> store-then-reread narrow-field codegen sensitivity) are unchanged from 16
> prior attempts across 5 rounds; not re-attempted further -- time went to
> the two higher-pre-screen-count functions instead
> (`StageMap__LoadChunksAround`/`StageMap__PopulateSlotCells`).

> **ROUND 32 (bravo2): re-verified, no drift; permuter scaffold checked and
> found UNRELIABLE for this function.** Rebuilt the preserved body: 72/106
> confirmed, identical residue. Set up an isolated `tools/setup-permuter.sh`
> scaffold to probe class 2 (the store-then-reread codegen sensitivity),
> and ran its own `--debug` sanity check BEFORE searching, per the script's
> own advice. Result: the isolated scaffold's base score showed **9
> insertions and 9 deletions** -- the real in-context build has ZERO of
> either (72/106 is purely register-identity + one reread choice, no
> missing/extra instructions). This is the identical scaffold-context-
> mismatch trap documented for `StageMap__ApplyChunkLoads` in round 17 (an isolated
> compile schedules a computation differently than the real surrounding
> file does). Not searched -- a result against a scaffold provably scoring
> a different residue would not transfer. Scaffold deleted. Not attempted
> further this round; time went to `StageMap__FindSlotForPosition` (matched) instead.

> **ROUND 27 (delta): re-verified, one new attempt on class 2, negative.**
> Rebuilt the exact preserved body from a clean `INCLUDE_ASM` baseline:
> confirmed 72/106, no drift. Tried dropping the `b2`/`b3` locals entirely
> and re-reading `out->base.b2`/`out->base.b3` directly at the `h4`/`h8`
> use sites (the natural next thing to try given retail's OWN disassembly
> genuinely does `lb v0,2(s0)` / `lb v1,3(s0)` -- signed reloads -- right
> before the `sll ...,0xb` in each case, which is the exact shape a real
> field re-read should produce). Result: **worse, not better** -- 72/106
> raw match dropped, and `funcdiff.py` reported 133936 bytes of drift
> outside the function's own range, traced to an extra callee-saved
> register appearing in the PROLOGUE (the frame-setup words differ before
> the first residue even in-range). So the two locals are load-bearing for
> keeping this function's overall register pressure at the point retail
> assumed -- removing them changes allocation far outside the h4/h8 tail
> alone, not just the two `lb` sites this was aimed at. Reverted
> immediately; not attempted further. This reinforces the existing
> report's own conclusion (item 5 below, the isolated repro not
> transferring to the real function) rather than adding a new lead. Time
> this round went to `StageMap__UpdateFootprintTracking` (matched) instead.

> **ROUND 20 (charlie): re-verified, no new attempt.** Rebuilt the exact
> preserved body below from a clean `INCLUDE_ASM` baseline: 72/106, no
> drift, unchanged since round 19's re-verification. Not re-attempted
> further -- both residue classes below (`$a1`/`$a3` register identity for
> `u14b`, and the store-then-reread narrow-field codegen sensitivity) match
> patterns independently re-confirmed elsewhere in this unit this round
> (see `StageMap__PopulateSlotCells`'s report for a THIRD confirmation of the
> "reread-from-memory rather than keep-the-register-live" idiom this
> report's class-2 residue is an instance of). Time this round went to
> `StageMap__PopulateSlotCells` (fresh ground) instead, per the staffing guidance.

> **ROUND 19 (bravo): drift claim RE-VERIFIED per the head's mid-round
> broadcast** (two other reports' "clean/drift-free" claims turned out
> false this round). Rebuilt this exact preserved body from a clean
> `INCLUDE_ASM` baseline: `funcdiff.py` reports 72/106 with NO "differs
> outside range" warning, and `objdump -t` confirms compiled length
> `0x1A8` (106 words) -- IDENTICAL to retail's own `.s` header
> (`nonmatching StageMap__ComputeFootprintDescriptor, 0x1A8`). **Claim confirmed accurate.**
>
> Also checked against this round's other broadcast (aggregate/whole-struct
> assignment closing 5 sibling functions elsewhere): **the shape does not
> occur here.** Every struct field write in this function is either a
> freshly-computed value (`out->base.b2 = b2;`, `out->unk18 = in->unk0.w -
> out->unkC;`, etc.) or reads from a DIFFERENT struct/pointer (`u14a`,
> `u14b`) than the one being written (`out`) -- there is no place where
> several adjacent fields of `out` are populated by copying the same
> adjacent fields from one other struct INSTANCE verbatim. Not
> applicable; not tested further.
>
> Not re-attempted beyond verification this round -- restored to
> `INCLUDE_ASM` unchanged. The report's own two residue classes (register
> identity `$a1`/`$a3` for `u14b`, and the store-then-reread narrow-field
> codegen sensitivity) were independently re-checked via `asm-differ`
> against this round's build: every mismatching word is a same-opcode,
> same-immediate, DIFFERENT-REGISTER substitution -- no missing or extra
> arithmetic term anywhere in the diff, which rules out the
> "misdiagnosed-missing-field-offset" mechanism the head's broadcast named
> for this specific residue.

Unit: `class_3bb8c`. Slot `Obj866E8Methods::slot110` (verified against
`tools/classtable.py 0x800866E8`). Not toolchain-blocked: no `gp_rel` hit, no
`addiu $at,$at,%lo` hit in `asm/nonmatchings/class_3bb8c/StageMap__ComputeFootprintDescriptor.s`.

## What it does

Resolves a query (`in`, a `SplitLongVec3*`) via `self->methods->slot11C`
(still `INCLUDE_ASM`, `StageMap__FindSlotForPosition`). On a miss, returns `1`. On a hit
(`e`), fills `out` (`Descriptor10Ext*`):

- `out->base.b0`/`b1` via `StageMap__SplitChunkIndex(self, out, e->unk4->unk30)` (mod/div
  by `self->unk68->divisor`), and `out->unk28` = the same raw rate.
- `out->unkC/unk10/unk14` from `self->methods->slot118(self, e->unk4->unk32)
  ->unkC->unk14` (a *different* `Elem`'s `SplitCoord2`, called `u14a` below):
  `unk18+0x5000`, `unk1C` (new field, raw), `unk20+0x5000`.
- `out->unk18/unk1C/unk20` from `in`'s three fields minus the just-computed
  `out->unkC`/`out->unk14` (a genuine RELOAD of the just-stored value, not a
  cached local -- confirmed by retail's own `lw $v1,0xC($s0)` /
  `lw $v1,0x14($s0)` reloads).
- `out->base.b2/b3`: `(in->x - u14b->unk18) >> 11` and `(in->z - u14b->unk20)
  >> 11`, each rounded toward zero (`+0x7FF` before the shift when negative),
  where `u14b = e->unkC->unk14` (the FIRST elem's `SplitCoord2`, distinct from
  `u14a` above).
- `out->base.h4/h6/h8`: halfword-precision versions of the same math, reusing
  `b2`/`b3`. `h6` is a raw truncating copy of `in->unk4`'s low halfword.
- `out->unk24 = e`.

Full derivation, including the exact retail instruction trace this was built
from, is in this report's body below and in the header comments for
`Descriptor10Ext`/`SplitLongVec3`/`SplitCoord2` in `include/class_3bb8c.h`.

## SUPERSEDED by round 63 -- the matching body

The round-63 match is live in `src/class_3bb8c.c`. It differs from the
72/106 body preserved below in exactly three places: `b2`/`b3` are `s32`
locals re-read from `out->base.b2`/`b3` (not `s8` locals carrying the
computed value), the `0x400` sits inside the subtracted group, and
`out->unk24 = e;` is the last statement. Matching body:

```c
s32 StageMap__ComputeFootprintDescriptor(Obj866E8 *self, Descriptor10Ext *out, SplitLongVec3 *in) {
    Elem *e;
    SplitCoord2 *u14a;
    SplitCoord2 *u14b;
    s32 rate;
    s32 t;
    s32 b2;
    s32 b3;

    e = self->methods->slot11C(self, in);
    if (e != 0) {
        rate = e->unk4->unk30;
        out->unk28 = rate;
        StageMap__SplitChunkIndex(self, (u8 *)out, rate);

        u14a = self->methods->slot118(self, e->unk4->unk32)->unkC->unk14;
        out->unkC = u14a->unk18.w + 0x5000;
        out->unk10 = u14a->unk1C;
        out->unk14 = u14a->unk20.w + 0x5000;

        u14b = e->unkC->unk14;
        out->unk18 = in->unk0.w - out->unkC;
        out->unk1C = in->unk4.w;
        out->unk20 = in->unk8.w - out->unk14;

        t = in->unk0.w - u14b->unk18.w;
        if (t < 0) {
            t += 0x7FF;
        }
        out->base.b2 = t >> 11;

        t = in->unk8.w - u14b->unk20.w;
        if (t < 0) {
            t += 0x7FF;
        }
        out->base.b3 = t >> 11;

        b2 = out->base.b2;
        out->base.h4 = in->unk0.h - (u14b->unk18.h + (b2 << 11) + 0x400);
        out->base.h6 = in->unk4.h;
        b3 = out->base.b3;
        out->base.h8 = in->unk8.h - (u14b->unk20.h + (b3 << 11) + 0x400);
        out->unk24 = e;

        return 0;
    }
    return 1;
}
```

## HISTORICAL -- best body reached before round 63 (72/106 words)

```c
s32 StageMap__ComputeFootprintDescriptor(Obj866E8 *self, Descriptor10Ext *out, SplitLongVec3 *in) {
    Elem *e;
    SplitCoord2 *u14a;
    SplitCoord2 *u14b;
    s32 rate;
    s32 t;
    s8 b2;
    s8 b3;

    e = self->methods->slot11C(self, in);
    if (e != 0) {
        rate = e->unk4->unk30;
        out->unk28 = rate;
        StageMap__SplitChunkIndex(self, (u8 *)out, rate);

        u14a = self->methods->slot118(self, e->unk4->unk32)->unkC->unk14;
        out->unkC = u14a->unk18.w + 0x5000;
        out->unk10 = u14a->unk1C;
        out->unk14 = u14a->unk20.w + 0x5000;

        u14b = e->unkC->unk14;
        out->unk18 = in->unk0.w - out->unkC;
        out->unk1C = in->unk4.w;
        out->unk20 = in->unk8.w - out->unk14;

        t = in->unk0.w - u14b->unk18.w;
        if (t < 0) {
            t += 0x7FF;
        }
        b2 = t >> 11;
        out->base.b2 = b2;

        t = in->unk8.w - u14b->unk20.w;
        if (t < 0) {
            t += 0x7FF;
        }
        b3 = t >> 11;
        out->base.b3 = b3;

        out->base.h4 = (in->unk0.h - 0x400) - (u14b->unk18.h + (b2 << 11));
        out->base.h6 = in->unk4.h;
        out->unk24 = e;
        out->base.h8 = (in->unk8.h - 0x400) - (u14b->unk20.h + (b3 << 11));

        return 0;
    }
    return 1;
}
```

This needs `Descriptor10Ext`, `SplitLongVec3` and the retyped `SplitCoord2`
(`unk18`/`unk20` as `union { s32 w; u16 h; }`, plus new `unk1C`) from
`include/class_3bb8c.h`, and the retyped `slot110`/new `slot11C` in
`Obj866E8Methods` (same file). All already committed to the header; this
body is preserved here only as the literal near-miss source, per the report
convention.

## HISTORICAL residue analysis -- BOTH CLASSES WRONG, see the round-63 entry at the top

### 1. Register identity: `$a1` vs `$a3` for `u14b` (STALL class per project rule)

Retail keeps `u14b` (`e->unkC->unk14`, loaded once at `3CA80`) live in `$a1`
for the rest of the function (no intervening calls, so a caller-saved
register is safe the whole way). The build here allocates it to `$a3`
instead -- same value, same instructions around it, purely a different
register choice, confirmed stable across every structural variant tried:
single reused `u14` vs split `u14a`/`u14b`, `e2` as a named local vs inlined,
declaration-order swaps, and a `__asm__("")` barrier immediately after the
assignment (which only made things WORSE, see below -- consistent with
CLAUDE.md's test that a real barrier only reorders, and this one changed
register identity, i.e. was actively harmful here). Per CLAUDE.md and
MATCHING-GUIDE, this is the textbook "register identity" residue: **do not**
reach for `register T v asm("$a1")` or an operand constraint -- both are
banned. Left as a STALL discriminator; a future attempt could try controlling
it by changing how many *other* pointer-typed locals are simultaneously live
at that program point (register pressure), which was not exhaustively
explored.

### 2. Sign-extension codegen: retail reloads `out->base.b2`/`b3` via `lb`; this build computes it from a live register via `sll 0x18`/`sra 0xd`

Retail's `h4`/`h8` computation re-reads `out->base.b2`/`b3` from MEMORY with
a genuine sign-extending `lb` (confirmed: `3CAF4 lb $v0,2($s0)`), immediately
followed by a plain `sll $v0,$v0,0xb`. Every structural variant tried here
that references the struct FIELD again (`out->base.b2` instead of a cached
local) produces an UNSIGNED reload (`lbu`) plus a manual sign-extend/shift
combo (`sll ...,0x18` / `sra ...,0xd`) -- functionally identical, but ONE
INSTRUCTION LONGER, which is what caused the address-drift seen in earlier
attempts. Switching to a cached local (`s8 b2 = t >> 11; out->base.b2 = b2;`
... later use `b2` directly) removes the drift (matches retail's total
instruction COUNT again) but does so by using the double-shift form for BOTH
the local-var-only path *and* what would have been retail's memory reload --
i.e. it happens to reach the right SIZE by a different, still-wrong
mechanism, not by actually finding retail's `lb`.

**Isolated reproducer (confirms this is a genuine GCC 2.6.3 codegen quirk,
not an artifact of this project's headers):**

```c
typedef signed char s8;
typedef unsigned short u16;
typedef int s32;
struct D { s8 b0, b1, b2, b3; short h4, h6, h8; };

int test(struct D *out, s32 t, u16 inh, u16 u18h) {
    out->b2 = t >> 11;
    return (inh - 0x400) - (u18h + (out->b2 << 11));
}
```
compiles (through the pinned `cpp|cc1|maspsx|as` pipeline) to `sra a1,a1,0xb;
sb a1,2(a0); ...; sll a1,a1,0x18; sra a1,a1,0xd; ...` -- i.e. GCC value-
propagates the just-stored byte from the register that computed it, rather
than reloading, UNLESS an intervening store to a DIFFERENT field of the same
struct via the same pointer is added between the store and the re-read:

```c
int test(struct D *out, s32 t, u16 inh, u16 u18h) {
    out->b2 = t >> 11;
    out->other = 5;              /* new field, forces a real reload below */
    return (inh - 0x400) - (u18h + (out->b2 << 11));
}
```
This second form DOES emit a genuine `lb` (see repro at `/tmp/repro2.c` in
this session -- not preserved in the tree, reproduce with the snippet above).
**But applying the analogous intervening store in the real function (moving
`out->unk24 = e;` to sit between the `b2` store and its `h4` use) did NOT
reproduce this on the real function** -- it instead REGRESSED the match
count (63/106), so whatever triggers retail's real `lb` depends on more
context than this isolated repro captures (most likely overall register
pressure at that specific point, given how sensitive class-1 above is to the
same thing). Not resolved; flagging for the permuter as a promising target
once class 1 (register identity) is separately understood, since the
permuter cannot fix a banned-technique register pin either but CAN cheaply
explore many structural variants of the b2/b3/h4/h8 tail.

## Attempts

~16 manual structural variants over this session (return-guard direction,
single vs split `u14`, named vs inlined `e2`, local vs struct-field re-read
for `b2`/`b3`, addition operand order in the `h4`/`h8` expression, explicit
`(s32)` cast, declaration-order swap, one `__asm__("")` barrier). Best stable
result: 72/106, correct length, no address drift.

### Proposed learning

A **store to a struct field followed by re-reading that SAME field a few
statements later, in a leaf-shaped straight-line block with no intervening
call**, is not reliably reproduced by writing the same C shape: GCC 2.6.3
sometimes value-propagates the byte from the register that computed it
(wrong: emits an unsigned reload + manual double-shift sign-extend, one
instruction longer) instead of reloading via a real sign-extending `lb`
(right, retail's form). An intervening store to a DIFFERENT field of the SAME
struct through the SAME pointer, in an isolated 4-line reproducer, is enough
to force the correct reload -- but does not reliably transfer to a real,
larger function with more competing live values, where it can regress the
match instead. Suspect this interacts with the SAME register-pressure
sensitivity documented as class 1 above (`$a1` vs `$a3` for an unrelated
pointer live across the same span) rather than being independent. Screen
early for this shape (store-then-reread-a-narrow-signed-field) before
spending attempts reshaping the surrounding statements one at a time --
it likely needs either the permuter or a specific understanding of what
retail's original source held live at that point that this derivation didn't
reconstruct.

## Naming

Round 78 (track 3, naming pass, bravo).

| symbol | name | tier | evidence |
| --- | --- | --- | --- |
| `func_8004C1C0` | `StageMap__ComputeFootprintDescriptor` | B | Occupant of `gStageMapMethods` +0x110 (`slot110`). `class_39e08`'s own `StageMap__ApplyToSenderFootprint` (already matched) calls this exact slot to fill a `buf` that is then fed DIRECTLY to `StageMap__SetFootprintFromCell`/`StageMap__SetFootprintRect` as their own `desc` parameter -- i.e. this function's output IS the footprint descriptor those two already-named functions consume. Computes cell row/column (`base.b2`/`base.b3`) and sub-cell offsets (`base.h4`/`h6`/`h8`) from a `SplitLongVec3` world position via `StageMap__FindSlotForPosition` and an `SplitCoord2` position pair -- a position-to-grid-cell conversion, matching the caller-side evidence exactly. |

## Track 6 (2026-09-26, round 93, alpha)

The class `Class866E8` (table `gClass866E8Methods`, id 0x114, LightRig's
subclass) is now `StageMap` (`python3 tools/renametype.py Class866E8
StageMap`, tier B): it keeps seven slots loaded with map chunks of the
current stage (LbdFile, `STGnn\Mnnn.LBD`) around a tracked target, the
centre chunk and its six staggered neighbours (`sChunkNeighbourDeltas`), laid
out by the stage's `StageGridDimensions` (`setConfig`, from ObjM's
`GetStageGridDimensions(stage)`), each slot's placements linked into a 20 x
20 lattice of GridCells whose drawn window follows the target. Tier B: the
mechanics are established; "the stage's map" rests on the files it loads and
the per-stage config. Header now `include/StageMap.h`; evidence in its banner.

Member types, same pass: `Unk68Struct` is `StageGridDimensions`
(include/StageGrid.h), `Unk54Struct` is `LongVec3` (include/SceneNode.h),
`EntryDesc866E8` is `Ratio16[3]` (include/SceneNode.h), all by layout and
use; `Class866E8Elem` -> `ChunkSlot`, `QueryPos866E8` -> `SplitLongVec3`,
`SetupEntry866E8` -> `ChunkLoadEntry`, `SetupSub866E8` ->
`ChunkLoadEntryTail`, `TargetSpec866E8` -> `ChunkSlotSpec`, `GridSlot866E8`
-> `CellRect`, `GridSlotList866E8` -> `CellRectSet`, `Bounds866E8_3bb8c_b`
-> `CellBounds`, `Class866E8ValueFn` -> `ChunkFileFn`,
`Class866E8OnElementEventFn` -> `StageMapOnSlotEventFn`,
`Class866E8ElemFn` -> `ChunkSlotFn`, `Class866E8CellFn` -> `StageMapCellFn`;
new `ChunkNeighbourDelta` for `sChunkNeighbourDeltas` (was typed as the
3-word placeholder). renametype.py also rewrote the old names inside
earlier sections' history prose in this and sibling reports (known, pending
an operator decision; not hand-reverted).

## Round 94 (track 6, charlie)

`Unk14Obj` is `SplitCoord2` (include/class_3bb8c.h): the slot's
cellParent->coord2, a GsCOORDINATE2 with tx/tz as word-or-halfword unions.
Its fields are named for the GsCOORDINATE2 words they overlay: `unk18` ->
`tx` (coord.t[0]), `unk1C` -> `ty`, `unk20` -> `tz`; `unk0` (flg) had no
accessor through this view and is padding. Zero bytes changed.

## Track 7 (2026-09-27, round 95, charlie)

Parameters and locals, tier A: `in` -> `pos`, `e` -> `slot`, `u14a` -> `chunkOrigin` (the origin of the slot holding this slot's neighbour key, from which the chunk centre is taken), `u14b` -> `origin` (this slot's own cellParent origin), `rate` -> `chunkIndex`, `t` -> `rel`, `b2`/`b3` -> `cellCol`/`cellRow`.

Constants: 0x5000 -> `STAGE_CHUNK_SIZE / 2`, 0x7FF -> `STAGE_CELL_SIZE - 1` (the bias that makes the shift of a negative offset truncate toward zero, as a division would), 11 -> `STAGE_CELL_SHIFT`, 0x400 -> `STAGE_CELL_SIZE / 2`. The three source-shape points keep a two-line `MATCHING` note.

Left: `StageMap__SplitChunkIndex(self, (u8 *)out, ...)`; the prototype and the +0x114 slot take `u8 *`. Proposed: `Descriptor10 *` (it writes b0/b1).

The comment that stood above the function in `src/class_3bb8c.c`, moved here verbatim (its local names are the pre-track-7 ones):

```c
/* MATCH, round 63 (delta): closed a six-round stall (72/106 since round 19)
 * with three source-shape corrections, none of them register pinning -- see
 * docs/match-reports/StageMap__ComputeFootprintDescriptor.md.
 *   1. `b2`/`b3` are s32 locals RE-READ from `out->base.b2`/`b3` after the
 *      byte stores. An s8 field shifted directly in the expression compiles
 *      to `lbu` + `sll 0x18` + `sra 0xd`; assigning it to an s32 local first
 *      folds the sign extension into retail's `lb` + `sll 0xb`.
 *   2. The 0x400 sits INSIDE the subtracted group -- `x - (y + (b<<11) +
 *      0x400)`. GCC reassociates that to retail's `addiu a0,a0,-0x400`.
 *      Writing `(x - 0x400) - (...)` instead narrows the constant to HImode
 *      and emits `li 0xfc00` + `addu`.
 *   3. `out->unk24 = e;` is the LAST statement of the block. Every earlier
 *      placement schedules its `sw` too early; only trailing it after the
 *      h8 store reproduces retail's order. */
```
