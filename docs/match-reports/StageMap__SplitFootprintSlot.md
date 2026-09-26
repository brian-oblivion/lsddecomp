# StageMap__SplitFootprintSlot — MATCHED 97/97 (round 71, delta; revisit). Previously: STALL, 97 words exact length, 62/97, first diff at the prologue register saves

> Renamed from `Class866E8__SplitFootprintSlot` on 2026-09-26 (tools/rename.py). Address 0x8004caf0.

> Renamed from `func_8004CAF0` on 2026-09-24 (tools/rename.py). Address 0x8004caf0.


> **REVISITED, round 71: MATCHED 97/97, byte-exact, whole-image SHA1 green; names/types used (parameters renamed col/row/width/height, locals overflow/span/elemArg/widthLeft; no struct change).** The sections below this one are the history and are superseded by it.

## Round 71 (delta) — MATCHED

**Rebuilt as given first.** The round-58/60 `#ifdef NON_MATCHING` body, made
live unchanged: **62/97, exact length, zero drift; funcdiff `insertions 0 /
deletions 0`, positional skeleton diffs 34.** (Round 58's "3 ins / 3 del" was
the permuter's figure on the 55/97 body; on the 62/97 body the real-build
opcode diff is 0/0 — diff B is two replacements plus a delay-slot fill, not an
ins/del.)

**Round 58's CAUSE was right in location (diff B) and wrong in mechanism.**
It called the reassociation "below the level a named temporary can reach". It
is not: it is a TREE-level `fold` rewrite, `(A + C) - B -> A - (B - C)`, after
which CSE shares `B - C` (`span - 20`) with the later `slot->h8 = span - 20`
and carries that across the call instead of `span`. Splitting the expression
into two statements (`widthLeft = width + 20; widthLeft = widthLeft - span;`)
defeats `fold`. Round 58 recorded the split as "drift" and the named local as
"inert"; both are true on the body they were measured on, because that body
had other compensating shapes (below) that the split disturbs.

Progression, every step through the full oracle, every one exact length
unless stated:

| step | shape | score | funcdiff ins/del |
| --- | --- | --- | --- |
| 0 | round-58 body as given | 62/97 | 0/0 |
| 1 | + split `(p7 + 20) - sum` into two statements | 45/97, **length change** | 1/1 — but asm-differ shows diff B CLOSED; the new residue is an extra `move v0,count` in the tail's delay slot and a jump to past the epilogue's copy |
| 2 | + one shared `return count;` at the end (if/else arms instead of three early returns) | **76/97** | 0/0 — residue now a 3-cycle count/nextArg/sum among `$s2`/`$s3`/`$s4` |
| 3 | + `slot->h4 = p5 ± 10` stored inside each arm, the two `__asm__("")` barriers dropped | 76/97 | 0/0 (identical; the barriers were no longer doing anything) |
| 4 | + drop the `do { } while (0)` | 46/97 | 1/1 — count's register now RIGHT (`$s2`); residue nextArg/sum swap + diff A (the `sh hA` became the `lh h4` load-delay filler again) |
| 5 | + bare `__asm__("")` after `slot->hA = hSpan2` (diagnostic only) | **86/97** | 0/0 — diff A closed, residue a pure 2-register swap nextArg/sum |
| 6 | instead of 5: `slot->hA = span;` written at the END OF EACH ARM of the `col < 10` if/else, not once after the join | **97/97, `OK: build matches retail`** | 0/0 |

So the `do { } while (0)` that round 58 found was compensating for the missing
join-point barrier (loop notes act as a scheduling barrier in 2.6.3's sched),
at the price of perturbing `count`'s allocation priority, and the "3-register
rotation" was downstream of that, not of diff B alone. The real source has the
`hA` store duplicated in both arms: cross-jumping merges the two identical
`sh s3,0xa(s0)` into the join block, so retail shows ONE store — and the join
label, which exists because of that merge, keeps the following `lh h4` from
being scheduled above it. That is why no single-store spelling reached it.

Negatives measured this round on the way (each through the full oracle):
`20 - (hSpan2 - p7)` 37/97 drift; `20 - hSpan2 + p7` 45/97 drift;
`p7 + (20 - hSpan2)` 45/97 drift; `20 - slot->h4` 36/97 drift;
`hSpan2 + -20` inert; `hSpan2 -= 20; slot->h8 = hSpan2;` inert; reusing
`baseIdx` (`baseIdx += 2`) instead of `nextArg` 11/97 drift; separate `sum`
local instead of reusing `hSpan2` 40/97 (without barrier) / 82/97 3-3 (with);
reusing `p8` as the copy 4/97; moving the `hSpan2 = hSpan` copy 7/97 or inert;
declaration order inert. The single-expression form of the h8 store on the
final body: 38/97, 1/1 — the split is load-bearing. No permuter search was
spent (Gate 3 not run; not needed).

### Matched body

```c
#if 0
/* needs: common.h, class_3bb8c.h (Obj866E8, CellRect, slots8C, methods->slot120) */
s32 StageMap__SplitFootprintSlot(Obj866E8 *self, CellRect *slot, s32 count, s32 baseIdx, s32 col, s32 row, s32 width, s32 height) {
    s32 overflow;
    s32 span;
    s32 elemArg;
    s32 widthLeft;

    if (row + height >= 21) {
        /* The rectangle runs past the bottom edge (row 20): clip this slot
         * and open a new one for the part below. */
        overflow = (row + height) - 20;
        span = overflow;
        slot->hA = height - overflow;
        count = count + 1;
        slot = &self->slots8C[count];

        if (col < 10) {
            elemArg = baseIdx + 2;
            slot->elemIdx = self->methods->slot120(self, elemArg);
            slot->h4 = col + 10;
            slot->hA = span;
        } else {
            elemArg = baseIdx + 3;
            slot->elemIdx = self->methods->slot120(self, elemArg);
            slot->h4 = col - 10;
            slot->hA = span;
        }

        span = slot->h4 + width;
        slot->h6 = 0;
        if (span >= 21) {
            /* ...and past the right edge (column 20) too. */
            count = count + 1;
            widthLeft = width + 20;
            widthLeft = widthLeft - span;
            slot->h8 = widthLeft;
            slot = &self->slots8C[count];
            slot->elemIdx = self->methods->slot120(self, elemArg + 1);
            slot->h4 = 0;
            slot->h6 = 0;
            slot->h8 = span - 20;
            slot->hA = overflow;
        } else {
            slot->h8 = width;
        }
    } else {
        slot->hA = height;
    }
    return count;
}
#endif
```

### Proposed learning

1. **A duplicated store in both arms of an if/else is a real source shape
   even when retail shows ONE copy at the join.** Cross-jumping merges the
   identical tails; the join label that results is a basic-block boundary the
   scheduler will not move a later load across. Symptom: retail keeps a store
   BEFORE a following independent load and leaves the load-delay `nop`, where
   every one-store spelling lets GCC fill the delay slot with the store. A
   `do { } while (0)` or `__asm__("")` reproduces the ordering but perturbs
   allocation priorities elsewhere (here a callee-saved 3-cycle).
2. **`(a + K) - b` is rewritten by `fold` to `a - (b - K)` at the TREE level**,
   and CSE will then share `b - K` with any later `b - K`, carrying it across
   calls. Retail showing `addiu t,a,K; subu t,t,b` and a separate later
   `addiu u,b,-K` means the source split the expression into two statements.
   Test the split on a body WITHOUT compensating shapes: here it read as
   "drift" / "inert" for two rounds because it was measured on one.

---


> **ROUND 19 (bravo) UPDATE: the round-9 "reconstruction problem, frame off
> by 8 bytes" diagnosis is SUPERSEDED.** Acting directly on the round-9 head
> note's suggested lever ("give each distinct value its own named local"),
> this round closed almost the entire structural gap:
>
> 1. **Giving the `slot120` result and each `p5`-derived offset its own
>    named local** (instead of reusing a single `v`) grew the frame from
>    `-0x30`/6 registers to `-0x30`/7 registers on the first try — real
>    movement, confirming the head's hypothesis.
> 2. **Duplicating `hSpan` into a second variable** (`hSpan2 = hSpan;`,
>    used for exactly one of the three `slot->hA = hSpan` writes) closed the
>    LAST missing register: frame `-0x38`, all eight `$s0`-`$s7`, matching
>    retail's register SET exactly. This is the project's established
>    "mention the value twice" idiom, applied to a scalar kept in ONE extra
>    register rather than a loop bound.
> 3. **Both outer guards needed inverting from "early-return, then process"
>    to "if (long-condition) { <entire rest of function>; return; } short
>    return;`.** Confirmed via `asm-differ`: retail relocates BOTH short-
>    circuit return blocks to the very end of the function and falls
>    through directly into the long path; the early-return spelling (even
>    via `goto`) reproduces neither relocation. This is the "big if/else",
>    not the "inverted guard clause" idiom the project's docs generally
>    recommend for large bodies — worth noting as a case where the LARGE
>    body genuinely does want the big-if form despite the docs' general
>    caution against it for large bodies.
> 4. **A bare `__asm__("")` scheduling barrier, placed right after each of
>    the two `slot->elemIdx = self->methods->slot120(...)` calls (before
>    the following `h4 = p5±10;`), stopped GCC's cross-jump pass from
>    tail-merging the two branches' otherwise-identical store instructions
>    into one shared copy.** Retail keeps the store immediately after each
>    call, undestructive of that decision; without the barrier GCC merges
>    both stores into the branches' shared continuation and computes
>    `h4 = p5±10` in the merge-jump's delay slot instead — a **fully
>    correct semantic transformation** but a byte-level regression here of
>    exactly 1 word net (2 deletions, 1 spurious insertion elsewhere).
>    Removing the barrier does NOT change which register anything lands
>    in — verified by rebuilding without it and confirming the ONLY effect
>    is the store's presence/position, never a register identity swap — so
>    this is the permitted "scheduling nudge" use per CLAUDE.md rule 6, not
>    a banned register pin.
>
> **After all four fixes: 55/97, frame `-0x38` (matches), all 8 callee-saved
> registers used (matches), zero insertions, zero deletions — a clean
> register-identity ROTATION is all that remains** (self/slot/one reused
> temporary permuted across `$s0`/`$s1`/`$s3`; every other register --
> `count`=`$s2`, `nextArg`=`$s4`, `hSpan`=`$s5`, `p5`=`$s6`, `p7`=`$s7` --
> already matches retail exactly). Confirmed via `tools/asm-differ/diff.py`
> that every remaining mismatched word is the identical instruction on a
> different register, nothing else.
>
> **A declaration-order retry (moving `hSpan2`'s declaration to last) was
> tried once, per this round's brief on the inert axis, and confirmed
> inert again (byte-identical 55/97).** A permuter scaffold was set up but
> its `--debug` base score (869: 24 stack differences, 3 insertions/3
> deletions, 37 register diffs) does **not** match the real build's
> structural state (55/97, 0 insertions/deletions, pure rotation) — the
> same scaffold-vs-real-build mismatch `IsPointOutOfBounds`'s report already
> documented for this project (the standalone compile schedules
> differently than the in-context one). Running a search against that
> scaffold would target the wrong residue, so none was run; this is a
> genuine "not yet attempted with a trustworthy tool" gap, not a negative
> result.
>
> **Restored to `INCLUDE_ASM`** per the hard rule (no score short of
> byte-exact stays in `src/`). The preserved body below is the round-19
> state, not the original round-9 one.

---

# Original round-9 diagnosis (superseded above, kept for context)

STALL (best 8/92 words; frame size off by 8 bytes)

> **HEAD VERIFICATION, round 9: classification CONFIRMED, and it is the most
> actionable of this round's three stalls despite having the worst score.**
>
> The reasoning holds and the score is misleading in a specific way worth
> naming: retail's frame is `-0x38` saving `$s0`-`$s7` + `$ra`, ours is
> `-0x30` saving at most six. A frame-size difference moves every stack
> offset and every saved-register slot in the function, so 8/92 measures the
> cascade, not the distance. Do not read it as "nowhere near" — the report's
> own claim that control flow, field writes and semantics are settled is
> consistent with it.
>
> **The concrete next move follows from the diagnosis and has not been tried.**
> Retail keeping two MORE callee-saved registers means retail's source had two
> more values that had to survive the `slot120` vtable call. The preserved body
> reuses a single `v` for several unrelated intermediates (the `slot120`
> return, then `p5 ± 10`, then later reuses), which is exactly what lets GCC
> coalesce them into one register and shrink the frame. **Give each distinct
> value its own named local** — one for the slot120 result, one for the h4
> value, one for each quantity live across the call — and let the frame grow
> to match rather than trying to reshape control flow. That is the opposite of
> the usual advice (fewer temps, reuse), which is why it is easy to miss.
>
> This is a reconstruction, not a residue fix, so it is runner-scale work
> rather than head triage — it wants a fresh attempt budget with the
> variable-lifetime hypothesis stated up front. It is NOT a permuter target:
> the permuter mutates a source whose shape is already close, and here the
> variable set itself is wrong.

The biggest attempted this round (97 words) and the one that resisted
byte-exactness. An 8-parameter function that populates one or two
`CellRect` entries (the same type established this round from
`StageMap__SetFootprintCellFlag`/`StageMap__InitFootprintSlot`), advancing and returning `self->unk88`
(the slot count) as it goes.

## What the function does (control flow and semantics, not in doubt)

```c
s32 StageMap__SplitFootprintSlot(Obj866E8 *self, CellRect *slot, s32 count, s32 baseIdx, s32 p5, s32 p6, s32 p7, s32 p8) {
    s32 v;
    s32 hSpan;
    s32 h4sum;
    s32 nextArg;

    if (p6 + p8 < 21) {
        slot->hA = p8;
        return count;
    }

    hSpan = (p6 + p8) - 20;
    v = p8 - hSpan;
    slot->hA = v;
    count = count + 1;
    slot = &self->slots8C[count];

    if (p5 < 10) {
        nextArg = baseIdx + 2;
        v = self->methods->slot120(self, nextArg);
        slot->elemIdx = v;
        v = p5 + 10;
    } else {
        nextArg = baseIdx + 3;
        v = self->methods->slot120(self, nextArg);
        slot->elemIdx = v;
        v = p5 - 10;
    }
    slot->h4 = v;
    slot->hA = hSpan;

    h4sum = slot->h4 + p7;
    slot->h6 = 0;
    if (h4sum < 21) {
        slot->h8 = p7;
        return count;
    }

    count = count + 1;
    v = (p7 + 20) - h4sum;
    slot->h8 = v;
    slot = &self->slots8C[count];
    v = self->methods->slot120(self, nextArg + 1);
    slot->elemIdx = v;
    slot->h4 = 0;
    slot->h6 = 0;
    slot->h8 = h4sum - 20;
    slot->hA = hSpan;
    return count;
}
```

Parameters (register trace, o32 ABI — 4 in `$a0-$a3`, 4 more on the
caller's stack at `$sp+0x48/0x4C/0x50/0x54` relative to THIS function's
own `-0x38` frame, i.e. the 5th-8th arguments):

- `self` (`$a0`)
- `slot` (`$a1`) — a `CellRect *`, already resolved by the caller to
  `&self->slots8C[count]` (the FIRST slot this call may touch)
- `count` (`$a2`) — the running slot count, also the return value
- `baseIdx` (`$a3`) — seed passed (with a small per-branch offset) to
  `Obj866E8Methods::slot120`
- `p5` (5th arg) — gates which of two `slot120` offsets/signs is used
- `p6`, `p7`, `p8` (6th-8th args) — combined into the two `< 21` /
  `-20`/`+20` range-clamp computations that produce `hA`/`h8`

Confirmed against the raw asm line-by-line: every branch target, every
field write (`elemIdx`@0, `h4`@4, `h6`@6, `h8`@8, `hA`@0xA — the SAME
`CellRect` layout `StageMap__SetFootprintCellFlag` established), and every arithmetic
op matches retail's OPERATIONS. The residue is a REGISTER ALLOCATION /
frame-size difference, not a logic difference.

## The residue

Retail's frame is `-0x38` (56 bytes) and saves **eight** callee-saved
registers (`$s0`-`$s7` plus `$ra`). Every C shape tried compiles to a
frame of `-0x30` (48 bytes) or smaller, saving at most six. This means
retail's compiled function keeps MORE independent values alive in
registers simultaneously than any attempt reproduced — most likely
because retail's source keeps the ORIGINAL `slot` pointer (this
function's own `arg1`) in its own dedicated register for the entire
function, distinct from the register holding the "current" slot being
populated, whereas every attempt here reassigns the same C variable
(`slot = &self->slots8C[count];`), letting the compiler collapse both
uses onto one register/lifetime.

Attempts:

1. Reassigning `slot` in place (shown above) — 8/92 in the fixed window,
   frame `-0x30`, `StageMap__SetFootprintFromQuery` (the next function) shifts by -0x14
   (20 bytes) low, meaning this function compiles noticeably SHORTER
   than retail overall (missing register save/restore pairs, not just a
   handful of instructions).
2. Introducing a second pointer variable (`slot2`) for the reassigned
   slot, leaving the original `slot` parameter untouched throughout —
   made the frame size WORSE (one word shorter still), and the register
   diff (checked via `objdump`) showed the two variables collapsing onto
   overlapping registers again rather than the two independent ones
   retail uses.

Given the scale of the register-pressure gap (missing 2 whole
callee-saved registers, not a 1-2 instruction residue), this smells like
either: (a) a source shape not yet tried that keeps more values alive at
once (e.g. NOT recomputing `nextArg + 1` inline but holding a running
"next elemIdx query index" variable that's live across the whole
function rather than scoped to inside each `if`), or (b) the true
parameter/local variable SET is larger than the 8 named here (e.g. `p6`
might need to survive far longer than its single early use suggests, if
retail's source reads it again somewhere not evidenced by this
executable's actual behavior — unlikely, but the double-digit-instruction
gap is large enough to warrant suspicion of a missed variable rather than
a pure scheduling residue).

## Best-attempt body (inline, `#if 0`)

```c
#if 0
s32 StageMap__SplitFootprintSlot(Obj866E8 *self, CellRect *slot, s32 count, s32 baseIdx, s32 p5, s32 p6, s32 p7, s32 p8) {
    s32 v;
    s32 hSpan;
    s32 h4sum;
    s32 nextArg;

    if (p6 + p8 < 21) {
        slot->hA = p8;
        return count;
    }

    hSpan = (p6 + p8) - 20;
    v = p8 - hSpan;
    slot->hA = v;
    count = count + 1;
    slot = &self->slots8C[count];

    if (p5 < 10) {
        nextArg = baseIdx + 2;
        v = self->methods->slot120(self, nextArg);
        slot->elemIdx = v;
        v = p5 + 10;
    } else {
        nextArg = baseIdx + 3;
        v = self->methods->slot120(self, nextArg);
        slot->elemIdx = v;
        v = p5 - 10;
    }
    slot->h4 = v;
    slot->hA = hSpan;

    h4sum = slot->h4 + p7;
    slot->h6 = 0;
    if (h4sum < 21) {
        slot->h8 = p7;
        return count;
    }

    count = count + 1;
    v = (p7 + 20) - h4sum;
    slot->h8 = v;
    slot = &self->slots8C[count];
    v = self->methods->slot120(self, nextArg + 1);
    slot->elemIdx = v;
    slot->h4 = 0;
    slot->h6 = 0;
    slot->h8 = h4sum - 20;
    slot->hA = hSpan;
    return count;
}
#endif
```

## New struct knowledge (`include/class_3bb8c.h`)

- New vtable slot `Obj866E8Methods::slot120` (`s32 (*)(Obj866E8*, s32)`,
  +0x120) — carved out of the existing `pad11C[0x124-0x11C]` gap between
  `slot118` and `slot124`. This one IS load-bearing (used to type the
  call sites even with the function itself unmatched) and is correct
  regardless of this stall — confirmed by the call sites' own register
  trace, independent of the surrounding function's residue.

## Attempts (round 9)

2 (see above). Restored to `INCLUDE_ASM`.

### Proposed learning (round 9)

**A frame-size gap of multiple WHOLE registers (not 1-2 instructions) is
a different kind of residue than the usual one-instruction classes in
this file — it says the source is keeping systematically MORE values
alive at once than the attempted C did, not that one expression is
phrased wrong.** When `objdump`'s save/restore register list is shorter
than retail's by 2+ registers, look for a variable whose LIFETIME should
span the whole function (not just the block it is naturally scoped to)
before hunting for a single-statement fix. Reassigning a pointer
parameter in place (`slot = &self->slots8C[count];`) collapses its
lifetime with the original value's, even when a second, distinct
variable is introduced for it — suggests the fix is elsewhere (a
genuinely separate value neither attempt captured), not just the naming.

---

## Round 19 preserved body (55/97, best reached)

```c
#if 0
s32 StageMap__SplitFootprintSlot(Obj866E8 *self, CellRect *slot, s32 count, s32 baseIdx, s32 p5, s32 p6, s32 p7, s32 p8) {
    s32 hSpan;
    s32 hSpan2;
    s32 h4;
    s32 nextArg;

    if (p6 + p8 >= 21) {
        hSpan = (p6 + p8) - 20;
        hSpan2 = hSpan;
        slot->hA = p8 - hSpan;
        count = count + 1;
        slot = &self->slots8C[count];

        if (p5 < 10) {
            nextArg = baseIdx + 2;
            slot->elemIdx = self->methods->slot120(self, nextArg);
            __asm__("");
            h4 = p5 + 10;
        } else {
            nextArg = baseIdx + 3;
            slot->elemIdx = self->methods->slot120(self, nextArg);
            __asm__("");
            h4 = p5 - 10;
        }
        slot->h4 = h4;
        slot->hA = hSpan2;

        hSpan2 = slot->h4 + p7;
        slot->h6 = 0;
        if (hSpan2 >= 21) {
            count = count + 1;
            slot->h8 = (p7 + 20) - hSpan2;
            slot = &self->slots8C[count];
            slot->elemIdx = self->methods->slot120(self, nextArg + 1);
            slot->h4 = 0;
            slot->h6 = 0;
            slot->h8 = hSpan2 - 20;
            slot->hA = hSpan;
            return count;
        }
        slot->h8 = p7;
        return count;
    }
    slot->hA = p8;
    return count;
}
#endif
```

Register mapping at this state (`objdump -d build/src/class_3bb8c_b.c.o`):
`count`=`$s2`, `nextArg`=`$s4`, `hSpan`=`$s5`, `p5`=`$s6`, `p7`=`$s7` all
match retail exactly. Only `self`, `slot`, and the reused
`hSpan2`-then-`h4sum` temporary rotate: retail is
`self=$s1, slot=$s0, temp=$s3`; this body is `self=$s3, slot=$s1,
temp=$s0` — a clean 3-cycle, not a random scatter.

### Proposed learning (round 19)

**A "frame size off by N whole registers" verdict can be a SUPERSET of a
plain register-rotation residue, not a fundamentally different class.**
Three of round 9's four missing registers closed with exactly the two
already-documented lightweight levers (per-value named locals; mention-a-
value-twice for a duplicate scalar) plus one CFG-shape fix (the "big
if/else" polarity for a large body, the opposite of the project's usual
"inverted guard clause" advice) and one legitimate scheduling barrier
(preventing an unwanted tail-merge, not pinning a register). What's left
after all that is the SAME clean rotation this round's brief describes as
the least-reliable, most commonly-misdiagnosed class in this corpus.
**Do not accept "frame size gap of multiple registers" as evidence the
function needs deep reconstruction without first testing whether ordinary,
cheap levers close most of the gap** — here they closed all but a plain
3-way rotation.

Also: **a permuter scaffold's `--debug` base score can disagree with the
real build's own state even when the scaffold "compiles and assembles"
successfully** (this scaffold: 869/24 stack diffs/3 ins/3 del vs the real
build's 55/97/0 ins/0 del). Sanity-checking the base score against the
CURRENT known-best structural state (not just against an old report's
stale number) before running a search is the same discipline
`IsPointOutOfBounds`'s report already flags, now confirmed on a second,
independently-carved function.

**Checked against the head's second mid-round broadcast (aggregate/whole-
struct assignment closing 5 sibling functions elsewhere this round): does
NOT apply here.** Every `CellRect` field written in this function is
a freshly computed value (`slot->elemIdx = self->methods->slot120(...)`,
`slot->h4 = h4;`, etc.), never a verbatim copy of several adjacent fields
from one existing struct instance into another. The shape does not occur.

---

## Round 33 (charlie) — re-derived from the raw asm by hand, register rotation confirmed genuine; permuter scaffold mismatch re-checked, still unusable

Traced every instruction in `asm/nonmatchings/class_3bb8c_b/StageMap__SplitFootprintSlot.s`
fresh against the round-19 preserved body, line by line, specifically hunting
for anything the earlier rounds might have missed (in the spirit of this
round's other stall on this unit, `StageMap__ComputeFootprintFromRotation`, where the same exercise
found two real fixes). Found none here: every field write, every branch
target, every delay-slot placement, and every value's SOURCE POSITION
(including the `hSpan2 = hSpan;` copy sitting in the `p5<10` guard's own
delay slot, and the `slot->h4` re-load from memory rather than reuse of the
local right after storing it) already matches the round-19 body exactly.
The one asymmetry worth recording for whoever looks next: retail assigns
`self` to `$s1` and `slot` (the incoming `$a1`) to `$s0` in the PROLOGUE,
before either has a real use later in the function — i.e., this is not a
first-use-order effect reachable by moving a statement, it is a prologue-time
register-number choice.

**Re-ran the permuter scaffold's `--debug` against the CURRENT round-19 body**
(not an old snapshot): identical to what the round-19 entry already reported,
869 base score, 24 stack diffs, 3 insertions/3 deletions — still disagreeing
with the real build's 55/97-with-zero-drift state. The mismatch is not stale;
it reproduces on a fresh scaffold build from the exact preserved source. Not
investigated further (would require comparing the scaffold's own expanded
`base.c` against the real in-context compile unit's preprocessed output,
which is head-scale work, not a bounded runner attempt) — flagging so the
next round does not re-spend a scaffold-setup cycle rediscovering the same
disagreement before doing that comparison.

**Disposition: unchanged, still `INCLUDE_ASM`, still a pure register-identity
rotation with no new lever found.** Not a permuter target until the scaffold
mismatch itself is root-caused.

---

## Round 53 (bravo) — CALIBRATION attempt, two fresh levers, both inert, negative

Assigned as one of three functions in a round-53 Sonnet calibration slot for
the track-1 stop rule (`docs/FINISHING-PLAN.md`), alongside `IsPointOutOfBounds`
and `StageMap__ConfigureRateEntry`. Round 33's disposition ("still a pure register-identity
rotation with no new lever found") is the reason this unit was picked over
the plan's higher-ranked but levers-measurably-spent `code_2cc8c_e` job.

Rebuilt the round-19/33 preserved body fresh first, per hygiene: confirmed
**55/97, exact length (0x184), zero outside-range drift** — unchanged from
both prior rounds' recorded figure.

**Lever 1: full local-declaration reorder (not just `hSpan2`'s position).**
Round 19 tried moving only `hSpan2` and found it inert. The current body
still declares `h4` before `nextArg` even though `nextArg` is assigned
FIRST in source order (inside the `p5 < 10` branch, before `h4`). Reordered
to match first-use order (`hSpan; hSpan2; nextArg; h4;`):

**Result: byte-identical 55/97.** No effect — confirms the declaration-order
lever is inert for every permutation of these four locals, not just the one
round 19 tried.

**Lever 2: "mention self twice" — the same trick that closed `hSpan` in
round 19, applied to `self` in the function's final block.** Round 19's key
lever for closing the last register was duplicating `hSpan` into `hSpan2`,
used for exactly one of the three `slot->hA = hSpan` writes. Applied the
same idea to `self` (which the report's own residue analysis names as one
of the three rotating registers), scoped to the final `if (hSpan2 >= 21)`
block only:

```c
if (hSpan2 >= 21) {
    Obj866E8 *selfCopy;

    selfCopy = self;
    count = count + 1;
    slot->h8 = (p7 + 20) - hSpan2;
    slot = &selfCopy->slots8C[count];
    slot->elemIdx = selfCopy->methods->slot120(selfCopy, nextArg + 1);
    slot->h4 = 0;
    slot->h6 = 0;
    slot->h8 = hSpan2 - 20;
    slot->hA = hSpan;
    return count;
}
```

**Result: byte-identical 55/97.** No effect — unlike `hSpan` (a genuinely
reused VALUE with two independent lifetimes the compiler could choose to
split or coalesce), `self` is a PARAMETER already live across the entire
function body and every method call; duplicating its name does not give
cc1 a new coalescing decision to make, because there was never a point
where its old value could have been considered dead. The "mention it twice"
lever's precondition (a value whose lifetime the compiler is currently
choosing to SHORTEN) does not hold for a parameter that is live throughout.

Both reverted (`git checkout -- src/class_3bb8c_b.c`; clean `OK: build
matches retail` confirmed after each).

**Disposition unchanged: `INCLUDE_ASM`, still 55/97, still a clean 3-register
rotation (self/slot/temp among `$s0`/`$s1`/`$s3`) per CLAUDE.md HARD RULE 6
— not something a `register`/asm-constraint fix is permitted to close.**
Four rounds (9, 19, 33, 53) have now worked this function; round 19 closed
most of the gap with real levers, and rounds 33 and 53 each independently
re-derived the residue and found nothing further. This is the honest
negative half of this round's Sonnet track-1 calibration measurement.

### Proposed learning

**The "mention a value twice" lever's precondition is a value whose
lifetime the compiler could otherwise shorten — a local intermediate, not a
parameter that is already live for the function's entire body.** Round 19
closed a register with this lever on `hSpan` (assigned once, used at two
points with a gap between); this round's negative on `self` (live
continuously, referenced at every method call) is the boundary case that
makes the precondition explicit rather than assumed. Worth checking against
any OTHER register-rotation stall before spending an attempt: does the
candidate value for the "duplicate it" trick have a genuine second,
independent USE POINT the compiler could split from the first, or is it
just a parameter mentioned in more than one place? Only the former has
shown a positive result in this corpus so far.

---

## Round 58 (bravo) — the 39-round "permuter scaffold is untrustworthy" blocker is RETIRED (it was a unit error, not a scaffold fault); residue re-characterised as 3 real insertions + 3 real deletions that cancel, NOT a pure register rotation

Assigned as PRIMARY with the brief noting `plan.py` tags this function
`unspent, never-searched, len-exact`. Rebuilt the round-19 preserved body
first, per hygiene: **55/97, exact length (0x184), zero outside-range drift**
— unchanged from rounds 19, 33 and 53.

### 1. The residue is not what three rounds have recorded

Rounds 19, 33 and 53 all describe what is left as "a clean register-identity
ROTATION … zero insertions, zero deletions". Read with
`tools/asm-differ/diff.py` rather than with `funcdiff.py`'s word count, that
is false in a way that matters: there are **three distinct local structural
differences**, and they happen to cancel in total length, which is why every
prior round's "length exact / zero drift" reading passed them over.

**Structural diff A — the load-delay slot at `0x3D3C0`.**

```
retail                          ours (55/97)
3d3bc: sh   v0,4(s0)            3d3bc: sh   v0,4(s1)
3d3c0: sh   s3,0xa(s0)          3d3c0: lh   v0,4(s1)
3d3c4: lh   v0,4(s0)            3d3c4: sh   s0,0xa(s1)
3d3c8: nop                      3d3c8: addu s0,v0,s7
3d3cc: addu s3,v0,s7
```

Retail emits the `slot->hA = hSpan2` store BEFORE the `slot->h4` reload and
leaves the reload's load-delay slot as a `nop`. Ours hoists the `lh` and uses
the `sh` as the delay-slot filler — **one word shorter here (a deletion)**.

**Structural diff B — the `h8` arithmetic at `0x3D3E0`.**

```
retail                          ours
3d3e0: addiu v0,s7,0x14         3d3dc: addiu s0,s0,-0x14
3d3e4: subu  v0,v0,s3           3d3e0: subu  v0,s7,s0
...                             ...
3d41c: addiu v0,s3,-0x14        3d41c: move  v0,s2
3d428: sh    v0,8(s0)           3d428: sh    s0,8(s1)
```

Retail evaluates `(p7 + 20) - h4sum` **literally**, keeps `h4sum` itself alive
in `$s3` across the `slot120` call, and computes `h4sum - 20` separately
afterwards. Ours reassociates: it computes `t = h4sum - 20` **early, before
the call**, destroying `h4sum` in place, then derives the first store as
`p7 - t`. Same value, different instruction stream, and it changes which
value is the one held across the call.

**Structural diff C — the third call site at `0x3D3FC`.**

```
retail                          ours
3d3fc: addu s0,s1,v0            3d3f8: addu s1,s3,v0
3d400: lw   v0,0(s1)            3d3fc: move a0,s3        <- insertion
3d404: move a0,s1               3d400: lw   v0,0(a0)
3d408: lw   v0,0x120(v0)        3d404: nop               <- insertion
```

Ours schedules the `move a0, self` ahead of the `lw self->methods` and then
needs a load-delay `nop` — **two words longer here (insertions)**.

Net: −1 +2 and one more deletion elsewhere ⇒ **equal length**. The permuter's
own structural summary counts it exactly: **3 insertions, 3 deletions, 1
reordering, 37 register differences, 24 stack differences.**

### 2. Why that matters: the scaffold blocker was a UNIT ERROR

Rounds 19 and 33 both set up a permuter scaffold, both ran
`permuter.py --debug`, both got base score 869 with "24 stack differences, 3
insertions / 3 deletions", and both concluded the scaffold **disagreed with
the real build** ("55/97 with zero insertions/deletions") and was therefore
untrustworthy. Round 33 re-checked it on a fresh build and recorded the
disagreement as reproducible. On that basis **no permuter search has ever
been run on this function** — 39 rounds of a self-sustaining blocker.

**The scaffold was correct the whole time.** Gate 3 check 3
(`docs/PARALLEL-RUNS.md` §3.5) — does the scaffold's own compile of the same
body agree with that body rebuilt in the real tree? — was never actually run;
what was compared was two tools' *summary numbers*. Run properly this round:

```sh
tools/binutils/bin/mipsel-linux-gnu-objdump -d permuter-work/StageMap__SplitFootprintSlot/base.o
tools/binutils/bin/mipsel-linux-gnu-objdump -d build/src/class_3bb8c_b.c.o
```

`StageMap__SplitFootprintSlot` is **byte-identical** between the two, instruction for
instruction, with the only textual differences being the absolute targets of
`j`/`bnez` (`j 4cbbc` vs `j cc`) — which is what a standalone object always
shows, because those are section-relative until link time. 98 vs 99 lines,
6 differing lines, all of them branch/jump target addresses.

So the two numbers never disagreed about anything:

| figure | what it actually measures |
| --- | --- |
| funcdiff `55/97` | words equal at the same offset |
| funcdiff "zero drift" | bytes differing OUTSIDE the function's range |
| permuter `3 ins / 3 del` | instructions present in one stream and not the other |
| permuter `24 stack diffs` | `sw/lw sN,0xNN(sp)` pairs whose slot offset differs — i.e. the rotation, seen in the prologue |

**`funcdiff.py` does not report insertions or deletions at all.** The
"zero insertions, zero deletions" that rounds 19 and 33 weighed against the
permuter was never measured — it was *inferred from "length exact"*, and
equal length is not zero ins/del. Here it is 3 and 3, cancelling.

### 3. The bounded search (the function's first, ever)

**Gate 3 (`docs/PARALLEL-RUNS.md` §3.5) — which checks were run.** All three.
Check 1 (the target asm is the current extraction) and check 2 (the base
compiles with 0 errors) pass trivially. Check 3 is the one that had never been
run, and it is recorded in §2 above: `base.o` is byte-identical to the real
tree's object for this function. It was re-run a second time for the improved
base before the second search window, with the same result.

**Window 1 — seeded from the round-19 body (55/97), base score 845.**
`-j 6 --stop-on-zero --best-only`, **45,150 iterations, 0 errors**, stopped by
hand once its lead had been harvested (it was still running under a 2700s
cap). Candidates written out at scores 680, 565, 400, 400 and 350.

**The permuter's score is NOT this project's metric, and window 1 shows it
sharply.** Every candidate was re-verified in the real tree with
`./build-and-verify.sh` + `funcdiff.py`, and the ranking did not survive:

| permuter score | in the real tree |
| --- | --- |
| 400 (`output-400-1`) | **62/97, exact length, zero drift — the improvement** |
| 400 (`output-400-2`) | 58/97 |
| 350 (`output-350-1`) | **length changes — drift**, unusable |

The permuter weights insertions and deletions at 100 each but will still take
a length change if it buys enough register and stack agreement; this project
scores only words equal at the same offset, and treats any length change as
disqualifying. So **a lower permuter score is a LEAD, not a result** — the
setup script says this about the base score and it is just as true of every
candidate. Rank candidates by re-verifying them, never by their search score.

**Window 2 — re-seeded from the 62/97 body**, after re-running Gate 3 check 3
against the new base. WINDOW2_RESULT

### 3a. The lever, isolated

`output-400-1` differs from the round-19 body in exactly two places, and
**neither works on its own** — each alone changes the function's length and
drifts the whole image:

| body | result |
| --- | --- |
| round-19 base | 55/97, exact length |
| + named `h8Val` temp only | **drift** |
| + `do { } while (0)` only | **drift** |
| + **both** | **62/97, exact length, zero drift** |

A plain brace block `{ ... }` in place of the `do { } while (0)` also drifts —
so this is not a scoping effect. **`do { } while (0)` is a real RTL construct
to GCC 2.6.3**: the loop pass sees a loop and runs over the body, and a plain
compound statement gives it nothing to run over. That is the whole mechanism,
and it is worth reaching for on any stall whose residue is a scheduling or
delay-slot artefact that resists statement reordering.

What the pair buys is **two of the three structural diffs, closed**:

- **Diff A closed.** Ours now emits `sh hA` before the `lh h4` reload and
  leaves the load-delay `nop` in place, exactly as retail does.
- **Diff C closed.** The spurious `move a0, self` ahead of
  `lw self->methods` and its load-delay `nop` are gone; ours now matches
  retail's `lw` / `move` order at the third call site.
- **Diff B survives** unchanged, and so does the 3-register rotation.

### 3b. Why the rotation is probably DOWNSTREAM of diff B — read this before filing this as a HARD RULE 6 stall

Rounds 19, 33 and 53 all classify what is left as a pure register-identity
rotation, which is the class CLAUDE.md HARD RULE 6 forbids fixing. That
classification should not be inherited without re-testing, because diff B is a
**register-pressure** difference and the rotation is a **register-priority**
difference, and in GCC 2.6.3 the second is computed from the first.

`global.c` orders allocnos by roughly `log2(n_refs) * n_refs / live_length`
and hands out `$s0`, `$s1`, … in that order. The measured orders are:

| | rank 0 | rank 1 | rank 2 | rank 3 |
| --- | --- | --- | --- | --- |
| retail | `slot` (`$s0`) | `self` (`$s1`) | `count` (`$s2`) | temp (`$s3`) |
| ours | temp (`$s0`) | `slot` (`$s1`) | `count` (`$s2`) | `self` (`$s3`) |

`$s4`-`$s7` (`nextArg`, `hSpan`, `p5`, `p7`) already agree. The single value
whose rank moved furthest is the **temp** — from last of the four to first —
and diff B is precisely an extra use of that temp: ours computes
`h4sum - 20` early and carries it across the call in the temp, giving that
allocno an extra reference and a shorter effective live range, i.e. exactly
the two inputs that raise its priority. **Fix diff B and the temp's priority
should fall back**, which is the only thing that has to change for the
rotation to unwind.

So the honest disposition is: this is **not yet established as a register-
identity stall**. It is one arithmetic-shape residue with a rotation that has
a plausible, mechanism-level reason to be its consequence. It becomes a HARD
RULE 6 stall only if diff B is closed and the rotation survives.

### 3c. Diff B, characterised

```
retail                                  ours (62/97)
3d3e0: addiu v0,s7,0x14                 3d3e0: addiu s0,s0,-0x14
3d3e4: subu  v0,v0,s3                   3d3e4: subu  v0,s7,s0
...            (call)                   ...            (call)
3d41c: addiu v0,s3,-0x14                3d41c: move  v0,s2
3d428: sh    v0,8(s0)                   3d428: sh    s0,8(s1)
3d42c: j     4cc40                      3d42c: j     4cc44
```

Retail evaluates `(p7 + 20) - h4sum` literally and keeps `h4sum` itself in
`$s3` across the `slot120` call, recomputing `h4sum - 20` afterwards. Ours
reassociates to `p7 - (h4sum - 20)`, computing `h4sum - 20` **before** the
call and carrying that instead. The `j 4cc44` vs `j 4cc40` is a consequence,
not a separate diff: having hoisted the subtraction, ours has a spare slot and
puts the `move v0, count` return-value setup there, so it jumps past the
shared epilogue's copy of it.

Everything tried against it this round was inert or worse:

| shape | result |
| --- | --- |
| name `p7 + 20` in its own local before the subtraction | inert (55/97 on the old base, 62/97 on the new) |
| name the second `h8` value too (`h8Val` reused, or a second local) | inert, 62/97 |
| `__asm__("")` barrier after the first `h8` store | inert, 62/97 |
| `__asm__("")` barrier before the second `h8` store | inert, 62/97 |
| write the first store as `p7 - (hSpan2 - 20)` (i.e. concede the reassociation) | drift |
| write the first store as `p7 - hSpan2 + 20` | drift |
| split into `new = p7 + 20; new = new - hSpan2;` | drift |

A scheduling barrier does not touch it, which fits: this is a value-numbering
/ reassociation decision taken well before scheduling.


### 4. Source-shape levers tried this round (all against the 55/97 base)

| # | shape | result |
| --- | --- | --- |
| 1 | split the dual-use `hSpan2` into `hSpan2` (the `hSpan` copy) and a separate `h4sum` local | **46/97 — regression.** The variable's dual use is load-bearing; retail's `$s3` double duty is reachable from one C variable and not from two. |
| 2 | give `p7 + 20` its own named local before the subtraction, to block the reassociation in structural diff B | **55/97, byte-identical — inert.** The reassociation happens below the level a named temporary can reach. |
| 3 | swap the `slot->h4 = h4;` / `slot->hA = hSpan2;` statement order | **length changes (drift) — regression.** |
| 4 | swap which copy feeds which `hA` write (`hSpan` in the middle, `hSpan2` at the end) | **31/97 — regression.** Confirms round 19's assignment of the two copies is the right one. |
| 5 | split the reload into two statements (`hSpan2 = slot->h4; hSpan2 = hSpan2 + p7;`) to stop the delay-slot hoist in structural diff A | **length changes (drift) — regression.** |

Lever 1's negative is the informative one: it is the exact opposite of round
9's successful "give each distinct value its own named local", on the same
function. The two are not in conflict — round 9's lever ADDED live values to
grow the frame, and the frame is now correct; adding a ninth here perturbs an
allocation that is already the right size.

---

## Round 60 (charlie) — NON_MATCHING body promoted, round 60

Track 1b promotion. Score re-verified unchanged (62/97, exact length, zero
drift) before promoting. Placed the existing round-58 preserved body
(the do-while(0)-plus-named-h8Val body carried in `src/class_3bb8c_b.c`)
inside `#ifdef NON_MATCHING`, with `INCLUDE_ASM` restored in the `#else`.
No source change beyond the wrapper and comment; both oracles green:
`./build-and-verify.sh` (exit 0, `OK: build matches retail`) and
`tools/check-nonmatching.sh` (exit 0). Disposition otherwise unchanged
(still `INCLUDE_ASM` in the verified build; the remaining residue is not
yet established as a HARD RULE 6 register-identity stall per round 58).


## Naming

**Tier B.** Not a vtable slot. Clips a footprint slot against the grid's
row-20/column-20 wrap and opens the extra slot(s) needed for the
overflow. Named for what it does to the slot (splits/clips it at a grid
edge), not for a guessed game purpose.
