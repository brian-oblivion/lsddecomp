# func_8004CAF0 — STALL, 97 words (exact length, zero drift), 55/97 raw word-match, first diff at the prologue register saves (`self`/`slot` register-number swap; re-confirmed round 34)

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
> same scaffold-vs-real-build mismatch `func_8004CD38`'s report already
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
`GridSlot866E8` entries (the same type established this round from
`func_8004CE24`/`func_8004CDA4`), advancing and returning `self->unk88`
(the slot count) as it goes.

## What the function does (control flow and semantics, not in doubt)

```c
s32 func_8004CAF0(Obj866E8 *self, GridSlot866E8 *slot, s32 count, s32 baseIdx, s32 p5, s32 p6, s32 p7, s32 p8) {
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
- `slot` (`$a1`) — a `GridSlot866E8 *`, already resolved by the caller to
  `&self->slots8C[count]` (the FIRST slot this call may touch)
- `count` (`$a2`) — the running slot count, also the return value
- `baseIdx` (`$a3`) — seed passed (with a small per-branch offset) to
  `Obj866E8Methods::slot120`
- `p5` (5th arg) — gates which of two `slot120` offsets/signs is used
- `p6`, `p7`, `p8` (6th-8th args) — combined into the two `< 21` /
  `-20`/`+20` range-clamp computations that produce `hA`/`h8`

Confirmed against the raw asm line-by-line: every branch target, every
field write (`elemIdx`@0, `h4`@4, `h6`@6, `h8`@8, `hA`@0xA — the SAME
`GridSlot866E8` layout `func_8004CE24` established), and every arithmetic
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
   frame `-0x30`, `func_8004CC74` (the next function) shifts by -0x14
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
s32 func_8004CAF0(Obj866E8 *self, GridSlot866E8 *slot, s32 count, s32 baseIdx, s32 p5, s32 p6, s32 p7, s32 p8) {
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
s32 func_8004CAF0(Obj866E8 *self, GridSlot866E8 *slot, s32 count, s32 baseIdx, s32 p5, s32 p6, s32 p7, s32 p8) {
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
`func_8004CD38`'s report already flags, now confirmed on a second,
independently-carved function.

**Checked against the head's second mid-round broadcast (aggregate/whole-
struct assignment closing 5 sibling functions elsewhere this round): does
NOT apply here.** Every `GridSlot866E8` field written in this function is
a freshly computed value (`slot->elemIdx = self->methods->slot120(...)`,
`slot->h4 = h4;`, etc.), never a verbatim copy of several adjacent fields
from one existing struct instance into another. The shape does not occur.

---

## Round 33 (charlie) — re-derived from the raw asm by hand, register rotation confirmed genuine; permuter scaffold mismatch re-checked, still unusable

Traced every instruction in `asm/nonmatchings/class_3bb8c_b/func_8004CAF0.s`
fresh against the round-19 preserved body, line by line, specifically hunting
for anything the earlier rounds might have missed (in the spirit of this
round's other stall on this unit, `func_8004C6A8`, where the same exercise
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
the track-1 stop rule (`docs/FINISHING-PLAN.md`), alongside `func_8004CD38`
and `func_8004CFB8`. Round 33's disposition ("still a pure register-identity
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
