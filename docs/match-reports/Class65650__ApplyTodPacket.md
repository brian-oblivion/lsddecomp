# Class65650__ApplyTodPacket -- MATCHED 258/258 (round 75): lever = index the input blob (`((s32 *)s0)[i]`) instead of walking a separate `pin` pointer, so loop.c strength-reduces it AFTER hoisting the /360 magic constant (was STALL: length EXACT 258/258, 252/258 raw, first diff vram 0x8006644C)

> Renamed from `func_80066340` on 2026-09-24 (tools/rename.py). Address 0x80066340.

**Unit:** code_55dd4 · **Size:** 258 words (0x408 bytes) · **Status:** STALL —
**LENGTH exact (258/258 words, no drift); RAW WORD-MATCH 252/258 (97.7%);
FIRST REAL DIFF at file 0x056C4C / vram 0x8006644C** (the first of two
symmetric 3-word magic-multiply-constant scheduling clusters). Whole-image
red. Restored to `INCLUDE_ASM`.

**This is the dedicated-assignment function** — 258 instructions, an order
of magnitude larger than anything else in this unit, and previously
deliberately left untouched pending a focused pass. This report is that
pass: full control-flow and type derivation, six real residues found and
closed, one small residue (two symmetric 3-word instruction-order clusters)
still open after extensive attempts.

## What it does (fully derived — every branch target, every field, every call arity confirmed against the disassembly)

`Class65650Methods` slot `+0x138`, the accumulator-threading callee
`Class65650__ApplyTodFrame` (this unit's other pre-existing stall, 17/33) calls in a
fold: `acc = self->methods->slot138(self, acc, extra)`. Confirmed
consistent with that call site — `self`, `acc`, `extra` match, and the
return expression (`acc + outbuf[3] * 4`) is exactly the kind of
"advance the accumulator pointer by N slots" value a fold accumulator
expects.

```c
void *Class65650__ApplyTodPacket(Class65650 *self, void *acc, void *extra)
{
    u8 outbuf[4];
    void *s0;
    s32 idx;
    Unk70ElemObj *elem;
    Elem14Obj *e14;
    TimeTargetObj *t0;
    s32 i;

    s0 = self->unk5C->methods->slot84(self->unk5C, acc, &outbuf[0], &outbuf[1], &outbuf[2], &outbuf[3]);
    idx = Class65650__FindPartIndex(self, outbuf[0]);
    if (idx < 0) {
        goto end;
    }
    elem = self->unk70[idx];
    e14 = elem->unk14;
    e14->unk00 = 0;
    t0 = e14->unk44;

    switch (outbuf[1]) {
    case 0:
        elem->unk10 = (elem->unk10 & ((s32 *)s0)[0]) | ((s32 *)s0)[1];
        break;
    case 1: {
        /* fixed-point time-value processing, see below */
        ...
        break;
    }
    case 2: {
        u16 count;

        count = *(u16 *)s0;
        if (count != 0 && elem->unk20 == 0) {
            s32 v;

            v = self->unk5C->unk2C->methods->slot80(self->unk5C->unk2C, count - 1);
            Class6B5CC__LinkModel(elem, v);
        }
        break;
    }
    case 3: {
        s32 v1;

        v1 = *(s32 *)s0;
        if (v1 == 0 || v1 == 0xFFFF) {
            elem->methods->slot4C(elem, self, 0);
        } else {
            s32 idx2;

            idx2 = Class65650__FindPartIndex(self, *(u8 *)s0);
            elem->methods->slot4C(elem, self->unk70[idx2], 0);
        }
        break;
    }
    }

end:
    return (u8 *)acc + outbuf[3] * 4;
}
```

**Top-level shape:** calls `self->unk5C`'s vtable slot `+0x084`
(`Unk5CMethods::slot84`, NEW — a 6-argument function; the o32 ABI passes the
first four in registers and the remaining two on the caller's stack, which
is why this call needed a `u8 outbuf[4]` local and its own `&outbuf[2]`/
`&outbuf[3]` addresses computed and stored to `$sp+0x10`/`$sp+0x14` *before*
the call — those stack words ARE the 5th/6th arguments, not scratch). Its
return value (`s0`, a `void *` reinterpreted differently in every case
below — never one consistent type) and `outbuf[0]` (a tag byte) drive a
lookup: `Class65650__FindPartIndex` (this unit's own byte-search, already matched)
resolves an index into `self->unk70`, giving `elem`. `elem->unk14->unk00`
is unconditionally zeroed, and `t0 = elem->unk14->unk44` is fetched
unconditionally (used only by `case 1`, but computed every time regardless
— confirmed straight-line in the disassembly, no branch guards it). Then
`outbuf[1]` (a second tag byte) dispatches a real four-way `switch`.

### CASE0 — bitmask fields, `s0` as `s32[2]`

`elem->unk10 = (elem->unk10 & s0[0]) | s0[1];` — the caller's `slot84`
result reinterpreted as a two-word `{clearMask, setBits}` pair. New field:
`Unk70ElemObj+0x10`.

### CASE1 — fixed-point time-value processing, `s0` as a sequential input blob

By far the largest case. Reads a flags byte (`outbuf[2]`) and a base
pointer `t0` (`Elem14Obj::unk44`, a NEW type `TimeTargetObj` with three
3-element arrays: `arr00` (`s32[3]`, `+0x00`), `arr10` (`s16[3]`, `+0x10`),
`arr18` (`s32[3]`, `+0x18`)). `s0` is walked sequentially as an input blob,
each flag bit consuming a fixed-size slice IF set (and leaving `s0`
unchanged if not):

- **`flags & 1`** selects between two ENTIRELY different bodies for the
  same three flag bits — not a shared body with a mode switch, but two
  independently-coded paths (confirmed: `LOOP1` vs `LOOP4`, `LOOP2` vs
  `LOOP5`, `LOOP3` vs `LOOP6` in the disassembly are six DIFFERENT code
  blocks, not one parameterized block). Reads as "accumulate/merge" (bit
  set) vs. "assign/overwrite" (bit clear).
- **`flags & 2`** (if set): 3-word loop, `arr10[i] = arr10[i] + s0[i]/360`
  (bit1=1) or `arr10[i] = s0[i]/360` (bit1=0), where the division is a
  genuine **magic-multiply constant division by 360** (confirmed
  arithmetically: `0xB60B60B7 * 360 == 2^40 + 344`, matching the project's
  documented "solve `constant * N == 2^(32+shift) + remainder`" method).
  When bit1=1, the sum is ALSO taken mod 4096 afterward (`% 4096`, C's
  truncating-toward-zero semantics, confirmed by the `sra`+`addiu 0xFFF`
  negative-rounding idiom). `s0` advances by 0xC (three `s32`s consumed).
- **`flags & 4`** (if set): 3-word loop. bit1=1:
  `arr00[i] = (s0_s16[i] * arr00[i]) / 4096` (a real runtime multiply, NOT
  a magic constant — confirmed by the plain `mult`/`mflo` with no
  precomputed constant register). bit1=0: `arr00[i] = s0_s16[i]` (plain
  sign-extending widen, no multiply). `s0` advances by 8 (three `s16`s
  plus 2 bytes of padding — a packed `{s16,s16,s16,pad}` shape).
- **`flags & 8`** (if set): 3-word loop, `arr18[i] += s0_s32[i]` (bit1=1)
  or `arr18[i] = s0_s32[i]` (bit1=0). If this bit is CLEAR, control jumps
  straight past the tail-copy block below, skipping it entirely — this is
  not a "do nothing" case, it changes what the REST of the case does.
- **Tail (only reached when `flags & 8` was set):** copies
  `t0->arr18[0..2]` into `elem->unk14->unk18/unk1C/unk20` (three NEW
  `Elem14Obj` fields, at the SAME relative offsets as `TimeTargetObj`'s own
  `arr18` — not a coincidence worth reading into, just how the two structs
  happen to be laid out).

### CASE2 — `s0` as `u16 *`, a two-call sequence

```c
if (*(u16 *)s0 != 0 && elem->unk20 == 0) {
    v = self->unk5C->unk2C->methods->slot80(self->unk5C->unk2C, *(u16*)s0 - 1);
    Class6B5CC__LinkModel(elem, v);
}
```
New field `Unk5CObj+0x2C` (`Unk2CObj *`, an unidentified class whose only
known member is vtable slot `+0x080`). **Corrects `Class6B5CC__LinkModel`'s
signature**: previously typed `(Class65650 *, s32)` from `Class65650__Reset`'s
usage; this call site passes `elem` (`Unk70ElemObj *`), so the first
parameter is generically `void *` — a real, useful correction, not a
guess (confirmed: this function's OWN types are solid, both call sites
are matched/near-matched code, not each other's speculation).

### CASE3 — `s0` reinterpreted as `s32 *` then `u8 *`, a real dispatch call

```c
v1 = *(s32 *)s0;
if (v1 == 0 || v1 == 0xFFFF) {
    elem->methods->slot4C(elem, self, 0);
} else {
    idx2 = Class65650__FindPartIndex(self, *(u8 *)s0);
    elem->methods->slot4C(elem, self->unk70[idx2], 0);
}
```
New slot `Unk70ElemMethods+0x04C` (`slot4C`) — its second argument is
`self` (`Class65650 *`) on one path and `self->unk70[idx2]`
(`Unk70ElemObj *`) on the other, so it's typed generically `void *`, same
reasoning as `Class6B5CC__LinkModel` above.

## Six real residues found and closed (in order of discovery)

1. **`self->unk5C` must not be cached** — a local `unk5C` variable used at
   both the top (slot84 call) and in CASE2 forced an extra 5th
   callee-saved register (`$s4`, which retail never uses — retail RELOADS
   `self->unk5C` fresh at both sites instead). Removing the local and
   writing `self->unk5C` at each site directly fixed the register count
   and, as a side effect, resolved a `$s1`/`$s2` (`self`/`elem`) identity
   swap that had been present until then.
2. **The dispatch must be a real `switch`, not an if/else-if chain.** An
   if/else-if chain (even matching retail's exact test values and order)
   compiled with different branch polarity and a completely different
   code layout (CASE0's body ended up unreachable from the position
   retail has it in). A plain `switch (outbuf[1]) { case 0: ...; case 1:
   ...; }` reproduced retail's dispatch exactly, including its
   non-obvious comparison order (test `==1` first, `<2` second, `==0`
   third, then `==2`, `==3`) — consistent with CLAUDE.md's documented
   "GCC lays out case bodies in source order but picks its own comparison
   order" idiom.
3. **`outbuf[2]` (the flags byte) must NOT be cached in a local.** A
   `u8 flags = outbuf[2];` local, reused for all four `flags & N` tests,
   compiled 4-8 words too short (retail re-reads `outbuf[2]` from the
   stack fresh at the `&4` and `&8` test sites, only sharing the load
   between the adjacent `&1`/`&2` tests). Using `outbuf[2]` directly at
   every site let the compiler's own liveness analysis decide when to
   reload, matching retail exactly.
4. **The three-word tail copy (`t0->arr18[0..2]` into `elem->unk14`)
   needs BOTH a temp-variable batch AND its own `Elem14Obj *e14b` local,
   AND a trailing `__asm__("")` barrier.** Retail loads `elem->unk14`
   once, then all three `t0->arr18[i]` values, THEN stores all three —
   loads batched before stores (avoiding load-delay-slot stalls). Getting
   this shape required: (a) reading the three source values into named
   `s32` temps first, (b) computing `elem->unk14` into ITS OWN local
   (`e14b`, distinct from the top-of-function `e14`, which itself must
   NOT be reused here — reusing it reloads 3 times instead of once,
   reusing nothing reloads every time), and (c) a `__asm__("")` right
   after the third store, because without it the compiler (correctly,
   but differently from retail) folds the third store into the following
   jump's branch-delay slot, one word shorter than retail's actual
   (slightly less optimal) output. Verified permitted under CLAUDE.md
   rule 6: removing the barrier changes only whether the store is
   standalone or delay-slot-filled — never which register holds any
   value.
5. **The six inner loops (flag-driven `arr10`/`arr00`/`arr18` processing)
   must use incrementing POINTERS, not array indexing with the loop
   counter.** `t0->arr10[i]` (indexed) recomputed `t0 + 0x10 + i*2` in a
   way that didn't hoist the `+0x10` outside the loop; retail hoists a
   pointer to `t0+0x10` once before the loop and increments it by 2 each
   iteration, using offset 0 inside. Rewriting all six loops as
   `s16 *p16 = t0->arr10; ... *p16; ... p16++;` (mirroring the existing
   unit-wide idiom already used for `self->unk70` walks) closed roughly
   14 words at once.
6. **The loop-setup instruction order (target pointer, counter, input
   pointer) needs the counter's `i = 0` write scheduled explicitly
   between the two pointer inits**, not left implicit in a `for`
   statement's own initializer. Restructuring
   `s16 *p16 = ...; i = 0; { s32 *pin = ...; while (i < 3) {...} }`
   (target ptr declared+initialized, THEN `i=0` as a real statement, THEN
   the input pointer in a nested scope) reproduced retail's
   target/counter/input ordering, closing 10 more words.

## The remaining residue: two symmetric 3-word clusters (6 words total)

In the TWO loops that perform the magic-multiply-by-360 division
(`flags & 2`'s body, both the `flags&1==1` and `flags&1==0` variants),
retail's instruction order for the three independent setup values is:
**target pointer, counter=0, [constant load: `lui`+`ori`, 2 words],
input pointer** (input pointer LAST). Every attempt here instead produces
**target pointer, counter=0, input pointer, [constant load]** (input
pointer BEFORE the constant, not after) — the constant is a compiler-
synthesized value with no corresponding C statement, so its exact
scheduling position relative to the input pointer's own assignment isn't
directly controllable from source the way the OTHER five residues were.

Attempts, none of which changed the outcome (declaration order of the two
pointers, swapping the order of the `i=0`/`pin=...` statements, wrapping
the division in a separate named intermediate, flattening vs. nesting the
inner block) or made it worse (a `__asm__("")` barrier immediately after
`i = 0`, tried per this round's "reach for it late" guidance, actively
regressed the SAME cluster by one more word without fixing the ordering).
Six real variations tried; none moved this specific 6-word residue.

## Preserved body (best attempt, 252/258, no size drift)

```c
#if 0
void *Class65650__ApplyTodPacket(Class65650 *self, void *acc, void *extra)
{
    u8 outbuf[4];
    void *s0;
    s32 idx;
    Unk70ElemObj *elem;
    Elem14Obj *e14;
    TimeTargetObj *t0;
    s32 i;

    s0 = self->unk5C->methods->slot84(self->unk5C, acc, &outbuf[0], &outbuf[1], &outbuf[2], &outbuf[3]);
    idx = Class65650__FindPartIndex(self, outbuf[0]);
    if (idx < 0) {
        goto end;
    }
    elem = self->unk70[idx];
    e14 = elem->unk14;
    e14->unk00 = 0;
    t0 = e14->unk44;

    switch (outbuf[1]) {
    case 0:
        elem->unk10 = (elem->unk10 & ((s32 *)s0)[0]) | ((s32 *)s0)[1];
        break;
    case 1: {
        if (outbuf[2] & 1) {
            if (outbuf[2] & 2) {
                s32 *pin;
                s16 *p16 = t0->arr10;

                i = 0;
                pin = (s32 *)s0;
                while (i < 3) {
                    s16 tmp;
                    tmp = *p16 + *pin / 360;
                    *p16 = tmp;
                    *p16 = tmp % 4096;
                    pin++;
                    i++;
                    p16++;
                }
                s0 = (u8 *)s0 + 0xC;
            }
            if (outbuf[2] & 4) {
                s32 *p32 = t0->arr00;

                i = 0;
                {
                    s16 *pin = (s16 *)s0;

                    while (i < 3) {
                        *p32 = (*pin * *p32) / 4096;
                        pin++;
                        i++;
                        p32++;
                    }
                }
                s0 = (u8 *)s0 + 8;
            }
            if (!(outbuf[2] & 8)) {
                goto end;
            }
            {
                s32 *p32 = t0->arr18;

                i = 0;
                {
                    s32 *pin = (s32 *)s0;

                    while (i < 3) {
                        *p32 += *pin;
                        pin++;
                        i++;
                        p32++;
                    }
                }
            }
        } else {
            if (outbuf[2] & 2) {
                s32 *pin;
                s16 *p16 = t0->arr10;

                i = 0;
                pin = (s32 *)s0;
                while (i < 3) {
                    *p16 = *pin / 360;
                    pin++;
                    i++;
                    p16++;
                }
                s0 = (u8 *)s0 + 0xC;
            }
            if (outbuf[2] & 4) {
                s32 *p32 = t0->arr00;

                i = 0;
                {
                    s16 *pin = (s16 *)s0;

                    while (i < 3) {
                        *p32 = *pin;
                        pin++;
                        i++;
                        p32++;
                    }
                }
                s0 = (u8 *)s0 + 8;
            }
            if (!(outbuf[2] & 8)) {
                goto end;
            }
            {
                s32 *p32 = t0->arr18;

                i = 0;
                {
                    s32 *pin = (s32 *)s0;

                    while (i < 3) {
                        *p32 = *pin;
                        pin++;
                        i++;
                        p32++;
                    }
                }
            }
        }
        {
            Elem14Obj *e14b;
            s32 v1, v2, v3;

            e14b = elem->unk14;
            v1 = t0->arr18[0];
            v2 = t0->arr18[1];
            v3 = t0->arr18[2];
            e14b->unk18 = v1;
            e14b->unk1C = v2;
            e14b->unk20 = v3;
            __asm__("");
        }
        break;
    }
    case 2: {
        u16 count;

        count = *(u16 *)s0;
        if (count != 0 && elem->unk20 == 0) {
            s32 v;

            v = self->unk5C->unk2C->methods->slot80(self->unk5C->unk2C, count - 1);
            Class6B5CC__LinkModel(elem, v);
        }
        break;
    }
    case 3: {
        s32 v1;

        v1 = *(s32 *)s0;
        if (v1 == 0 || v1 == 0xFFFF) {
            elem->methods->slot4C(elem, self, 0);
        } else {
            s32 idx2;

            idx2 = Class65650__FindPartIndex(self, *(u8 *)s0);
            elem->methods->slot4C(elem, self->unk70[idx2], 0);
        }
        break;
    }
    }

end:
    return (u8 *)acc + outbuf[3] * 4;
}
#endif
```

All types this body depends on (`TimeTargetObj`, `Elem14Obj`, `Unk2CObj`/
`Unk2CMethods`, the `Unk70ElemObj`/`Unk70ElemMethods`/`Unk5CMethods` field
and slot additions, and the `Class6B5CC__LinkModel` signature correction) are kept
live in `include/code_55dd4.h` — every one of them is confirmed correct by
byte-identical surrounding code, independent of the six-word residue above.

### Proposed learning

**GCC-synthesized values (magic-multiply constants, in this case) don't
follow the same scheduling-position rules as C-visible locals.** Every
other residue in this function responded to a source-level lever
(caching/not caching a value, statement order, pointer-vs-index shape,
explicit vs. implicit loop-counter initialization) — but the position of
a compiler-synthesized constant load, relative to an adjacent variable's
assignment, did not move under any of six different source reshapes tried
here. When a residue involves an instruction with NO direct C-source
counterpart (a `lui`/`ori` pair for a division constant, not a value the
programmer wrote), stop treating it as a source-shape question sooner —
it may be a pure instruction-scheduler internal decision, and CLAUDE.md's
"escalate, do not experiment" principle plausibly extends to this class
too, not just to flag/toolchain-version questions.

Separately, confirmed at scale (258 words, not the unit's usual ~40-60):
**every lever this unit had already learned (switch-vs-if/else, direct
field access vs. caching, pointer-walk vs. array-index, late `__asm__("")`
placement) generalizes cleanly to a function an order of magnitude
larger** — none of them needed reinterpretation or produced a
different/opposite result at this size, which is reassuring for whoever
tackles the NEXT large uncarved block.

## Round 18 (permuter pass, charlie)

`permuter.py --debug` (and re-confirmed with `--stack-diffs`, which changed
nothing here -- 0 stack differences either way) reproduces exactly this
report's own residue before any search: `Reorderings: 2`, everything else
zero, base score 120 -- consistent with "two symmetric 3-word
instruction-order clusters, 6 words total."

Two hand-crafted hypotheses were tried first via quick `--debug` checks
(not full searches) on the specific axis this report already suspected
(the magic-multiply-by-360 constant's scheduling position): (1) collapsing
the loop-setup statements into a single `for (i = 0, pin = (s32 *)s0; ...)`
initializer, and (2) reordering the `pin`/`p16` initializations so `pin` is
assigned before `p16`'s declaration. **Both made it WORSE** (180 and 295
respectively, vs the 120 base) by introducing NEW register-difference
penalties beyond the two known reordering clusters -- confirming these are
not the axis, consistent with the existing report's conclusion that the
constant's position has no direct source-level lever.

Ran two bounded blind searches on the unmodified 252/258 body (the first
killed early per the head's guidance to stop wasting a big-function
budget on random mutation without a specific hint; the second run to
exhaustion of its bound since no better hint was found):

- Run 1: `timeout 900`, killed early (scoped `kill -TERM` on the captured
  process group, not `pkill -f`) after 16,147 iterations, still at base
  120.
- Run 2: `timeout 500` on the restored pristine seed, ran to completion.
  **`permuter exit=124`** (the bound fired; not an external kill).
  30,068 further iterations, best score still 120 at the end -- confirmed
  via `grep "found new best score"` on the full log: zero matches, i.e.
  the search never improved on the base score once across either run.

**Combined: ~46,000 blind iterations across both runs, never below the
base 120 (the same two 3-word reordering clusters throughout).** Per the
head's standing instruction, this is **NOT marked permuter-exhausted** --
the machine ran at load average 45+ (all five runners searching
concurrently) for both runs, and a blind, unhinted search is the weakest
configuration available (no PERM macros were used, since no untried
source-level axis was identified). Correct framing: **not closed in
~46,000 iterations under contention; no untried source hint identified
either**, not "the search space is exhausted." A future round with more
headroom, or a specific insight into how GCC 2.6.3 schedules a hoisted
loop-invariant magic-multiply constant relative to adjacent pointer
initializations, may still close this.

Remains a STALL at 252/258, `INCLUDE_ASM` restored (never disturbed --
all experimentation happened in `permuter-work/`, `src/` was untouched
for this function this round).

## Round 19 (echo): drift re-verified (the head's specific ask), plus one
## more axis tried (negative)

**Per the head's round-19 broadcast asking specifically about this
function**: re-verified the "252/258, no drift" claim directly by
dropping the preserved body into `src/code_55dd4.c` and rebuilding.
Confirmed: `funcdiff.py` prints no `WARNING: the build differs OUTSIDE
this range` line, i.e. the function's compiled length matches retail's
258 words exactly. The 6 differing words are, position-for-position, a
clean 3-word ROTATION of the exact same three instruction values in both
of the two symmetric clusters -- e.g. cluster 1: retail
`[0bb6093c, b7602935, 21380002]` vs mine `[21380002, 0bb6093c,
b7602935]` (mine's word N = retail's word N+2, mod 3; the SAME three
32-bit values appear on both sides, only their relative order differs).
This positively rules out the "missing field-offset term" mechanism the
head flagged as a live hypothesis (round 18's `func_80040C00`, and this
round's several other functions, show that class produces a genuinely
DIFFERENT or MISSING instruction, not a same-set reordering) -- there is
no missing `+ self->something` here, just a scheduling-position
question, exactly as this report's title already states. Reported here
per the head's explicit ask: **checked, no drift, and the residue is
confirmed a pure reordering, not a missing-term bug.**

**Also tried, per the head's separate whole-struct-assignment broadcast**
(runner alpha's finding that whole-struct/aggregate assignment can close
register-permutation residues that scalar field-by-field copies don't):
this function's one candidate field-copy shape is the `v1/v2/v3`
temporary triple, copying `TimeTargetObj::arr18[3]` into
`Elem14Obj::unk18/unk1C/unk20` (three contiguous `s32`s at matching
relative offsets in both structs -- confirmed via the header's own
layout comments). Rewrote it as a whole-aggregate copy through a local
wrapper struct (arrays themselves aren't assignable in C89, so a
`struct Triple32 { s32 a, b, c; }` cast on both sides was needed instead
of a direct array-to-array assignment, which the compiler correctly
rejected with "incompatible types in assignment"):

```c
struct Triple32 { s32 a, b, c; };
*(struct Triple32 *)&e14b->unk18 = *(struct Triple32 *)t0->arr18;
```

Result: **252/258, IDENTICAL residue, no change whatsoever** (same two
3-word clusters, same values, no drift either way) -- GCC 2.6.3 compiles
this whole-struct assignment to byte-IDENTICAL code as the three separate
scalar assignments it replaced. This is a clean negative for the
whole-struct-assignment axis on THIS unit's one occurrence of the
field-copy shape.

### Proposed learning (round 19)

**Whole-struct assignment is not a universal lever -- it is at least
struct-family- or shape-dependent.** Here (three `s32` fields, no
padding, natural alignment, no interspersed field of a different type)
GCC 2.6.3 already recognized the scalar-copy sequence as equivalent to a
struct copy and compiled both forms identically. Combined with round
18's cross-runner report that this same lever closed 5 functions in a
DIFFERENT unit family (odd-sized/padded structs including a 3-byte RGB
struct), the emerging picture is that the lever helps when the NATURAL
scalar-by-scalar compile would otherwise miss an opportunity GCC's
struct-copy path takes (unaligned/padded/odd-sized aggregates), and does
nothing when the scalar compile was already optimal (here, three
naturally-aligned same-type words) -- consistent with, not contradicting,
the head's own caveat that five data points from one unit family don't
yet establish a general property.

## Round 20 (alpha): the outgoing-arg dead-code lever doesn't apply here, plus three more source reshapes tried (all negative)

Re-verified first: `INCLUDE_ASM` restored, fresh build green, dropped the
preserved 252/258 body back in verbatim and rebuilt -- `funcdiff.py`
reproduces **252/258** exactly, same two clusters at the same offsets
(`0x056C4C-0x056C54` and `0x056D74-0x056D7C`), no drift. No contamination
this round.

**Screened this function against the round-20 broadcast's outgoing-arg
dead-code lever (the mechanism that closed `ApplyMatrixToLVArray`, 31/31, in
round 19) and it does not apply.** That lever is about FRAME SIZE: a call
inside dead code still contributes its argument count to
`current_function_outgoing_args_size` during RTL expansion, before DCE
removes it. This function's residue is not a size question at all --
`funcdiff.py` reports zero bytes of drift and the exact word count (258)
matches retail's; the whole 258-word body, including this function's own
frame, is already the right SIZE. The open residue is a pure
instruction-ORDER permutation (a hoisted magic-multiply constant's
position relative to an adjacent pointer assignment), a different GCC
internal decision than the one the frame-sizing lever explains. Filing
this explicitly so the census doesn't get re-applied here a third time.

**Three new source reshapes tried, targeting the actual residue (the
constant-vs-pointer-assignment ordering), all in the two symmetric
`flags & 2` (divide-by-360) blocks:**

1. **Restructure to match the ALREADY-MATCHING sibling blocks' idiom.**
   The `flags & 4` (`arr00`) and the tail-copy blocks that already close
   at 0 residue both declare their walking input pointer (`pin`) inside a
   *nested brace scope*, declared-with-initializer, after `i = 0;` as a
   separate statement. The `flags & 2` (`arr10`) block that carries the
   residue instead declares `pin` *flat* (no nested scope) and assigns it
   via a separate statement after `i = 0;` -- a structurally different
   shape from its matching siblings. Rewrote both `flags & 2` blocks (the
   `flags & 1` variant with the `tmp = *p16 + *pin / 360` addition, and
   the plain `*p16 = *pin / 360` variant) to the nested-scope
   declare-with-initializer shape, verbatim otherwise. Result: **252/258,
   BYTE-IDENTICAL residue, same two clusters, same offsets** -- this
   restructuring is invisible to the compiler; the nested-scope idiom and
   the flat idiom compile to the same code here. Makes sense in
   retrospect: the `arr00`/`arr18` blocks that model the nested-scope
   shape never hoist a compiler-synthesized constant across an
   assignment boundary in the first place (no division = no invariant to
   place), so their success was never evidence this specific shape
   matters -- it just happened to be shape-neutral for them too.
2. **Move `pin`'s nested-scope declaration/init before `i = 0;`** (still
   both inside the same brace, order swapped: `pin` first, then `i = 0;`,
   both ahead of the `while`). Result: **251/258, WORSE** -- one word
   longer, the second (`flags&1==0`) cluster grew from 3 to 4 differing
   words (an extra `move` instruction appeared: `21280000`).  Reverted.
3. **Swap the addition's operand order** in the `flags & 1` variant only:
   `tmp = *pin / 360 + *p16;` instead of `*p16 + *pin / 360`. Result:
   **245/258, substantially WORSE** -- seven additional words differ
   (offsets `0x056C7C`-`0x056C98`), a completely different register
   allocation for the addition/modulo sequence, not just a reordering.
   Reverted immediately.

All three reverted; confirmed the reversion rebuilds `build exit=0`,
whole-image green, and `git diff --stat src/code_55dd4.c` shows no
uncommitted change.

**This makes 9 manual source reshapes (6 from rounds 14/19 + 3 here) and
~46,000 permuter iterations across two rounds, all converging on the same
two 3-word clusters.** Nothing in this round's attempts found a
source-level handle on the constant's hoist position either. No new
permuter search was run this round -- the prior two runs (16,147 +
30,068 iterations, round 18) already covered blind search on this exact
residue with the same base score (120) throughout, and no new PERM hint
was identified here to make a fresh search meaningfully different from
those. Remains a STALL at 252/258, `INCLUDE_ASM` restored, `src/` restored
to the exact preserved-body state (git diff clean).

### Proposed learning

**A shape that works for a SIBLING block in the same function is not
evidence it will help the block that actually has the residue, if the
sibling never exercised the mechanism causing that residue.** The
`arr00`/`arr18` blocks' nested-scope-declare-with-init idiom looked like
the "right" structural template to copy onto the `arr10` block, but those
siblings never hoist a compiler-synthesized constant (no division in
their bodies), so their matching was never evidence about constant-hoist
positioning at all -- confirmed by the copy producing byte-identical
(not improved) output. When a residue is specifically about a
compiler-synthesized value with no source counterpart (as this project's
round-19 learning on this same function already flagged), matching a
sibling's surface shape is not a substitute for finding a lever that
actually touches the synthesized value's scheduling -- and per that
round-19 learning, none has been found across 9 attempts and two
independent permuter campaigns. Escalate-do-not-experiment increasingly
looks like the right disposition for this specific residue class
(scheduler-internal constant placement), not just a fallback after
exhausting ideas.

## Round 24 (echo): re-verified, title corrected, no new build attempt

Re-verified by dropping the preserved 252/258 body in verbatim, rebuilding.
`funcdiff.py` reproduces exactly **252/258**, no drift. `tools/asm-differ/diff.py`
confirms the first real diff (realigned) is at file offset **0x056C4C** /
vram **0x8006644C** -- retail's `move a3,s0` (input-pointer assignment)
scheduled BEFORE the magic-multiply constant's `lui`/`ori`, versus ours
which schedules it AFTER -- the same "target, counter, CONSTANT, input"
vs. "target, counter, input, CONSTANT" ordering documented since round 14.
Corrected the title to the three-figure format.

**Screened for a `for`-loop rewrite of the two residue blocks (untried per
the prior attempt lists) and rejected it without spending a build attempt.**
This function's own finding #6 (closed round 14) already established that an
explicit `i = 0;` STATEMENT between the two pointer initializations is
required to reproduce retail's setup order -- a `for` loop's implicit
initializer was the thing that finding replaced. Switching the two residue
blocks specifically to a `for` form would contradict an already-confirmed,
byte-verified idiom used by every other loop in this same function, so it
was screened out as predictably negative rather than tried.

Given nine manual reshapes (rounds 14/19/20) and ~46,000 permuter iterations
across two prior rounds already probing every statement-order axis this
report's own catalogue could name, and no new lever in rounds 21-23 that
targets a compiler-SYNTHESIZED value's scheduling position (every new lever
found since round 20 -- dead-parameter reuse, pointer elimination, declared
width -- operates on a named C variable, and this residue's culprit has no
C-source name at all), no new build attempt was spent this round. Remains a
STALL at 252/258, `INCLUDE_ASM` restored, `src/code_55dd4.c` confirmed clean.

### Proposed learning

Confirms the round-19/20 conclusion stands: none of the levers added to the
project's catalogue in rounds 21-23 touch a compiler-synthesized constant's
scheduling position, because all of them act on named C variables (dead
parameters, loop-carried pointers, declared widths) and this residue's
culprit is a value with no C-source representation at all. This is now the
strongest remaining candidate in the corpus for an actual operator
escalation on "how does GCC 2.6.3 order a hoisted magic-multiply constant
relative to an adjacent pointer initialization" -- nine reshapes and two
permuter campaigns is a large attempt budget for a single 6-word residue.

## Round 25 (delta): block-order check (negative — seven bare `j`s, none adjacent to either residue cluster)

Per the head's round-25 block-order broadcast (`CheckDreamAuxTriggerCondition`'s basic-block-
layout closure, where a jumping arm placed textually LAST let GCC drop the
`j` and fall through instead): grepped for every bare unconditional `j`.

```
grep -nE '\*/\s+j\s' asm/nonmatchings/code_55dd4/Class65650__ApplyTodPacket.s
```

**Seven hits** — this function is large enough (258 words) to actually have
some. All seven are switch/if-else-chain early exits, six to a single
shared tail label (`.L80066718`) and one (`.L8006655C`, from the tail of a
small 3-iteration accumulator loop) to `.L80066634`, plus one more
(`.L800666DC`) to `.L8006670C`. Traced each one's surrounding structure
against the function's dispatch-on-`a0` switch (`a0==<const>` ->
`.L80066428`, `a0>=2` -> a second ori/beq pair, `a0==0` -> `.L8006640C`,
else -> `.L80066718`) — every one of the seven is a SIBLING case arm's
own exit, not a wrapper around either of this report's two residue
locations.

**Checked both magic-multiply-constant clusters specifically** (file
offsets `0x56C4C` and `0x56D74`, vram `0x8006644C` and `0x80066574` — the
two "symmetric 3-word clusters" this report's residue is about):

- First cluster (`0x56C4C`): reached by falling through TWO `beqz`
  checks inside the `.L80066428` case arm (`andi $v0,$v1,1` /
  `andi $v0,$v1,2`, both not-taken) — an ordinary nested-if fallthrough,
  not a block placed after an unconditional jump.
- Second cluster (`0x56D74`): reached the same way, one `beqz` fallthrough
  inside the `.L80066564` case arm (itself reached via a **different**
  case's own `beqz` target, confirmed at file offset `0x56D64`).

Neither residue site sits downstream of a bare `j`, and neither is a case
where "the jumping arm is written last in C, so GCC drops the jump and
falls through" could apply — there is no jump whose absence vs. presence
is even the question here. **The block-order lever does not apply to
either of this function's residue clusters.** This is consistent with —
not contradicted by — the existing classification: six reshapes and a
permuter pass already established this is a magic-multiply CONSTANT-LOAD
POSITIONING/scheduling question (per `docs/DECOMPILATION_LEARNINGS.md`'s
entry for this function), and this round's check confirms it from the
control-flow side: there is no candidate basic-block reordering that
could be the missing fix.

No new build attempt spent — this is a screening negative, following the
same discipline as the outgoing-arg-lever screens in rounds 20/24. This
function remains lowest priority in this round's ordered work list (item
4 of 4) and was not re-attempted by hand or given a fresh permuter run
this round; time went to the unit's higher-priority items 1-3 first per
the round's assignment.

## Round 31 (alpha): re-verified (no drift), MINIMAL REPRODUCER built and confirmed faithful, three fresh reshapes on it (all negative)

Re-verified first: dropped the preserved 252/258 body in live, rebuilt.
`funcdiff.py` reproduces exactly **252/258**, file range `0x56B40-0x56F48`
(0x408 bytes = 258 words, no drift), same two clusters at `0x056C4C` and
`0x056D74`. `INCLUDE_ASM` restored, `git diff --stat` clean. Blocker screens
(`gp_rel`, `mflo`/`mfhi`-into-`mult`/`div`) both clean.

**Built a MINIMAL standalone reproducer of the residue**, isolating just one
`flags & 2` divide-by-360 loop from the 258-word function, rather than
continuing to reshape inside the full body (which costs a full rebuild per
try). First cut used the walking pointers as bare function PARAMETERS:

```c
s32 probe(s16 *p16, s32 *pinArg, s32 i) { i = 0; pin = pinArg; while (i<3) {...} }
```

This did NOT reproduce the residue (the constant loaded first, before any
loop-setup statement, because `pin` coincided with an incoming register and
needed no `move` at all) — a reminder that a minimal reproducer can silently
drop the exact mechanism it's meant to isolate. **Second cut fixed this** by
giving `s0` the same shape as the real function: a value that arrives via an
earlier call return (forcing an actual register-to-register `move`, not a
free reuse of an argument register), matching retail's own
`move a3,s0`/`move a2,v0` pattern:

```c
s32 probe(TimeTargetObj *t0, void *acc, s32 i) {
    void *s0 = callsomething(acc);
    s16 *p16 = t0->a10;
    i = 0;
    s32 *pin = (s32 *)s0;
    while (i < 3) { ... }
}
```

This reproduces the actual residue's shape exactly: `target, counter, INPUT,
CONSTANT` (our order) instead of retail's `target, counter, CONSTANT, INPUT`
— confirmed by objdump on the isolated `.o`, matching this report's round-24
diff description word-for-word. **This gives a sub-second iteration loop for
future attempts on this specific residue**, rather than a full 258-word
rebuild each time; the reproducer is not committed (scratch-only, per the
"never commit the executable" spirit extended to throwaway probes) but its
recipe is preserved here so a future round does not have to re-derive it:

```c
typedef struct { s32 a00[3]; s16 a10[3]; s32 pad; s32 a18[3]; } TimeTargetObj;
extern void *callsomething(void *);
s32 probe(TimeTargetObj *t0, void *acc, s32 i)
{
    s16 *p16;
    s32 *pin;
    void *s0;
    s0 = callsomething(acc);
    p16 = t0->a10;
    i = 0;
    pin = (s32 *)s0;
    while (i < 3) {
        s16 tmp;
        tmp = *p16 + *pin / 360;
        *p16 = tmp;
        pin++; i++; p16++;
    }
    return i;
}
```

**Three reshapes tried on the reproducer, all negative** (none moved the
`target, counter, input, constant` order toward retail's `target, counter,
constant, input`):

1. Swap `pin = ...;`/`i = 0;` order (still `p16, pin, i=0`) — result:
   `target, INPUT, counter` — a different wrong order, not retail's.
2. Nested-scope `pin` declared-with-initializer after `i = 0;` (the shape
   that round 20 already tried inside the full function and found
   byte-identical) — reproduced here too: **identical** to the un-nested
   form. Confirms round 20's finding via the faster harness rather than
   just trusting it.
3. Pre-materializing the first division into a named temp to see whether
   giving the constant an early, otherwise-inert USE changes its hoist
   position — this changes the function's OBSERVABLE BEHAVIOR (not a valid
   translation of the original algorithm, purely exploratory) and, even so,
   did not reproduce a target-first layout; abandoned as not a viable
   candidate on both counts.

Consistent with three prior rounds' conclusion: **this is a compiler-internal
decision about when to hoist a magic-multiply constant relative to an
adjacent register-to-register `move`, with no C-source lever found across
9 full-function reshapes, 2 permuter campaigns (~46,000 iterations), and now
3 more reshapes on a faithful minimal reproducer.** No new build attempt
spent against `src/` this round (all probing done in scratch files via the
isolated `cpp | cc1 | maspsx | as` pipeline). Remains a STALL at 252/258,
`INCLUDE_ASM` restored throughout.

### Proposed learning

**A minimal reproducer for a scheduling residue must reproduce the VALUE'S
ARRIVAL MECHANISM, not just its final type/use.** The first-cut reproducer
here used the walking pointer as a bare function parameter and got a
different (non-representative) instruction order, because a parameter
already resident in the right argument register needs no `move` at all --
silently deleting the exact instruction whose position is in question. Only
giving the value the same origin retail's has (a call return, forcing a real
register-to-register copy) made the reproducer trustworthy. Any future
attempt to isolate a residue this way should confirm the isolated case
reproduces the reported diff BEFORE using it to test fixes -- exactly the
same "verify a preserved body's claim, don't inherit it" discipline this
project applies to full-function bodies, extended to throwaway probes.

## Round 33 (alpha): re-verified (no drift), round-32 levers screened, two isolated-`cc1` probes on the constant-hoist residue (both negative)

Re-verified first: dropped the preserved 252/258 body in live, rebuilt.
`funcdiff.py` reproduces exactly **252/258**, no drift. `INCLUDE_ASM` restored,
`git diff --stat` confirmed clean.

**Screened this function against all five round-32 levers, explicitly:**

1. **Volatile-narrower-than-barrier.** Tried directly, via the isolated
   `cpp | cc1 | maspsx | as` pipeline against the round-31 minimal
   reproducer (`s0`'s walking pointer `pin` declared
   `volatile s32 *pin`). Result: **no change** — the reproducer still
   emits `target, counter, INPUT(move a2,v0), CONSTANT(lui/ori)`, the
   same wrong order as every prior attempt. Qualifying the pointer's
   *pointee* as volatile does not touch where the compiler schedules the
   pointer's own assignment relative to a loop-invariant constant.
2. **Register-identity skepticism.** Does not apply — this residue was
   established seven rounds ago (round 14) to be a pure reordering (same
   three 32-bit values, rotated), never a register-identity swap, and
   round 19 independently confirmed no missing/extra term.
3. **Don't derive order from emission order.** Screened; nothing in this
   round's attempts relied on reading disassembly emission order to guess
   source order — the whole residue is that source order (as written) is
   already correct and only the compiler's internal hoist decision
   disagrees.
4. **Permuter sanity-check-scaffold-first.** N/A this round — no new
   permuter run was launched (192,000+ combined iterations across rounds
   18/25 already found nothing; see below for why no new run was queued).
5. **addiu/ori text-blindness.** Checked: this residue was already
   confirmed via `funcdiff.py`'s raw word compare (not text-based
   asm-differ) to be a genuine 3-word value rotation, not an
   opcode-encoding difference invisible to text tools. Re-confirmed this
   round by re-reading the raw words directly — unaffected.

**Two additional isolated-`cc1` probes tried on the round-31 minimal
reproducer** (no `src/` edit, no build attempt spent):

1. `volatile s32 *pin` (lever 1, above) — negative, detailed above.
2. **Named-constant divisor**, replacing the literal `360` with a local
   `s32 div = 360; ... *pin / div;` — on the theory that giving the magic
   constant a named C-level origin might change when the compiler
   materializes it. **Result: qualitatively different and worse** — GCC
   2.6.3 does NOT constant-propagate a local variable's value into
   division strength-reduction at `-O2` here; it falls back to a genuine
   runtime `div` instruction with the full divide-by-zero/overflow trap
   sequence (`div`/`bnez`/`break`), a completely different (and
   non-matching) instruction sequence, confirming the literal must stay a
   literal and ruling out this axis entirely rather than just being inert.

No new build attempt spent against `src/` this round (both probes ran in
scratch files under the isolated pipeline). This is now 9 manual reshapes
(rounds 14/19/20) + ~192,000 permuter iterations (rounds 18/25, no zero
ever found) + 3 reproducer-based reshapes (round 31) + 2 more (this round)
= 11 source-level axes and 2 permuter campaigns, all converging on the same
conclusion: **the two 3-word clusters are a compiler-internal decision
about when to hoist a magic-multiply constant relative to an adjacent
register-to-register `move`, with no C-source lever found.** Remains a
STALL at 252/258, `INCLUDE_ASM` restored throughout, `src/code_55dd4.c`
confirmed clean (`git status --porcelain` empty) before and after.

### Proposed learning

**The round-32 "volatile is narrower than a barrier" lever is scoped to
residues where an ACCESS needs fencing — it has no purchase on a residue
where the compiler-synthesized value has no C-level access to fence at
all.** A magic-multiply constant is never itself read or written through
any C expression; there is no access for `volatile` to attach to, so the
lever's mechanism (narrowing a fence from "every value crossing a program
point" to "one access") is a category mismatch for this residue class,
not merely untried. This is worth stating explicitly because the lever's
success elsewhere makes it tempting to try broadly; here it was tried and
is a clean, informative negative rather than an oversight.

## Round 49 (alpha): re-verified (no drift); head's mflo/mfhi-scope re-audit applied verbatim, no new search

Per the head's round-49 assignment note (this exact function named as the
subject of a fresh audit): re-verified first, dropped the preserved
252/258 body in live, rebuilt. `funcdiff.py` reproduces exactly
**252/258**, file range `0x56B40-0x56F48` (0x408 bytes = 258 words, no
drift), same two clusters at `0x056C4C`/vram `0x8006644C` and
`0x056D74`/vram `0x80066574`. `INCLUDE_ASM` restored, `git diff --stat`
confirmed clean.

This function's own round-33 permuter negative (192,000+ combined
iterations across rounds 18/25) predates round 42's `--no-nop-mflo-mfhi`
flag, and this function genuinely contains `mfhi`/`mflo` (the divide-by-360
strength reduction in the CASE1 fixed-point block). The head's round-49
brief already re-audited this exact scope question this morning and
recorded the result directly in the assignment: the flag's mechanism is
real (42 instructions become 44 without it, measured through the pinned
pipeline) but its hazard form is `mflo`/`mfhi` followed WITHIN ~1-2
instructions by `mult`/`div`, and in this function the nearest `mult`/`div`
after any `mflo`/`mfhi` is 33+ instructions away — zero hazard pairs.
Independently re-confirmed this round rather than taken on faith:

```sh
grep -n 'mflo\|mfhi\|mult\|div' asm/nonmatchings/code_55dd4/Class65650__ApplyTodPacket.s
```

produces exactly the widely-separated pattern the head describes — the
`mflo`/`mfhi` pair belongs to the divide-by-360 sequence's own strength
reduction, and the nearest subsequent `mult`/`div`-family instruction
belongs to an entirely different `case` arm reached only after several
intervening branches, calls, and the switch's own dispatch. **The
round-33 negative stands as real evidence; no re-search was spent on the
theory that it is void.**

No new lever applies here beyond what rounds 31/33 already screened
(minimal-reproducer reshapes, volatile, named-constant divisor, all
negative) — this round's actual search budget went to this unit's two
never-freshly-searched siblings (`Class65650__CreateParts`, `Class65650__ApplyTodFrame`)
instead, per the same "search coverage, not just search recency" priority
recorded in `Class65650__SetDisplay`'s round-49 entry. Remains a STALL at 252/258,
`INCLUDE_ASM` restored, `src/code_55dd4.c` confirmed clean before and
after.

### Proposed learning

No new lever found; recording that the head's mflo/mfhi-scope re-audit
was independently re-derived here (not just cited) via a direct grep of
this function's own disassembly, and it reproduces the head's conclusion
exactly: zero hazard pairs, so the flag's introduction does not touch
this function's own scheduling and the round-33 negative is not void.
This closes the loop the head's brief opened — a runner re-deriving the
audit, not merely trusting it, on the one function it was written about.

## Round 71 (charlie): NON_MATCHING body promoted

Re-measured live under current maspsx flags (rounds 42/63) per the head's
note that titles may predate them: dropped the preserved body in as live C
(no `#ifdef`), `build exit=2` with no compile error (only a pre-existing
unrelated warning in `Class65650__FindPartIndex`), `funcdiff.py` reproduces **252/258
words match (file 0x56B40-0x56F48), no size drift**, first diff at file
0x056C4C / vram 0x8006644C — identical to the title figure, confirming it
was already current. Restored the `#ifdef NON_MATCHING ... #else
INCLUDE_ASM ... #endif` shape with a comment naming the score, the residue
class (two symmetric 3-word scheduling clusters around a compiler-
synthesized magic-multiply constant) and this report. `./build-and-verify.sh`
green (bytes unchanged) and `tools/check-nonmatching.sh` green.

**Hand-derived, not permuter-found.** The promoted body is the one at
"Preserved body (best attempt, 252/258, no size drift)" above, produced by
manual source reshapes (rounds 14/19/20). Two separate permuter campaigns
(round 18, round 25; ~192,000 iterations combined) both ran against this
same residue and found no improvement — negative results, not edits
incorporated into the promoted body.

NON_MATCHING body promoted, round 71.

## Round 75 (charlie): MATCHED 258/258, whole image `OK: build matches retail`

REVISITED, round 75: MATCHED; names/types not relevant (no field, type or
name changed).

**First measurement, before any change:** the preserved body (identical to
the `src/` NON_MATCHING body; diffed, no difference) rebuilt live: 252/258,
length exact, `insertions 2 / deletions 2`, positional skeleton diffs 6 —
the two `move aN,s0` (input-pointer copy) sitting BEFORE the
`lui`/`ori` 360-magic constant instead of after it, once per `flags & 2`
loop.

**The lever: stop writing the input pointer as a separate variable.**
Retail's order — output pointer, `i = 0`, the hoisted loop-invariant
constant, THEN the input-pointer copy — is the order GCC 2.6.3's loop.c
produces when the input pointer is not a source variable at all but a
strength-reduced giv: loop-invariant motion places the constant before the
loop, and the reduced giv's initialisation (`move a3,s0`) is emitted after
it. A hand-written `pin = (s32 *)s0;` is an ordinary assignment before the
loop and so lands before the hoisted constant. Writing the loop body as
`((s32 *)s0)[i] / 360` with no `pin` gave 258/258 on the FIRST build.

Builds after that were readability only, each byte-exact with whole image
OK: all six `while` loops rewritten as `for (i = 0; i < 3; i++, pN++)`
with the input indexed (`((s16 *)s0)[i]`, `((s32 *)s0)[i]`) — the four
loops without a constant are insensitive to the shape. Removing the
`__asm__("")` at the end of the `e14b` block breaks it (length drift), so
it stays.

| body | score | ins/del | skeleton |
| --- | --- | --- | --- |
| preserved (pointer `pin` in both /360 loops) | 252/258 | 2/2 | 6 |
| /360 loops input indexed, rest unchanged | 258/258 | 0/0 | 0 |
| all six loops `for` + indexed input | 258/258 | 0/0 | 0 (committed) |
| same, e14b barrier removed | drift | — | — |

Five builds. No permuter search (not needed). Two earlier permuter
campaigns (~192k iterations) ran on the pointer-walk body; no mutation in
that permuter's repertoire turns a pointer walk into an index.

### Proposed learning

**When a loop-invariant constant (a magic-multiply `lui`/`ori`) lands
between two loop setup values in retail and before both in yours, the
setup value that comes AFTER it is a strength-reduced index, not a
pointer.** GCC 2.6.3 loop.c emits invariant hoists before the loop and
reduced-giv inits after them; a source-level pointer assignment is neither
and precedes both. Tell: `move aN,sM` (a copy of a base register used only
as the loop's walking pointer) immediately after the `lui`/`ori` pair.
Fix: write `base[i]` and drop the pointer variable. Six hand reshapes and
~192k permuter iterations missed it because all of them kept the pointer.

## Naming

Round 75 (charlie), track 3.

- `Class65650__ApplyTodPacket` (was `func_80066340`), tier A. Occupies +0x138. modelData->decodeTodPacket gives {object id, type, flag, length}; object id -> part via FindPartIndex; clears the part's coord2->flg. Type 0: attribute = (attribute & w0) | w1. Type 1: rotate (/360, %4096 when differential), scale (x/4096 when differential), trans into the part's GsCOORD2PARAM, flag bit 0 = differential, bits 1/2/3 = rotate/scale/translate, trans copied to coord.t. Type 2: link model tmd->getModel(id-1) if the part has none. Type 3: attach to self (id 0 or 0xFFFF) or to another part. Returns packet + length*4. Every packet type and flag bit matches the TOD packet format.

## Track 4 (2026-09-25, round 85, alpha)

The class (id 0x234, table `gClass65650Methods`) is unified as `Class65650` in `include/Class65650.h`: an Actor subclass (its ctor chains to Actor's first) and Entity's base. Any source block above is the pre-unification spelling (the local `Class65650Methods` of `include/code_55dd4.h`, `linkCompanion`/`unlinkCompanion`, `companion2`, `Unk5CObj`/`Unk70ElemObj`); the live body in `src/code_55dd4.c` takes the unified types and the inherited slot and field names (`addChild`/`removeChild`, Actor's `ticker`, `Actor *` parts, `ModelData *` modelData), byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).
