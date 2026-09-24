# Class866E8__Finalize — MATCHED (113/113, round 19)

> Renamed from `func_8004A7C0` on 2026-09-22 (tools/rename.py). Address 0x8004a7c0.

**Unit:** class_3ac78 · **Size:** 113 instructions · **Status:** MATCHED,
whole-image green. See "Round 19 (echo): MATCHED" at the end of this
report for the winning source, the two idioms recovered from the
already-matched sibling ctor `Class866E8__Class866E8`, and the one word-count
statement-order fix that closed it.

**Historical status (this round's earlier pass, kept for context): STALL.
Best reached: 62/113
words, WITH address drift (not a trustworthy score — see below)

## What it does

The destructor (`Class866E8Methods::dtor`, per the header comment already
on that field). Calls `self->methods->slot14(self, func_80020C5C(self))`
once, then walks the SAME 7-element `unkEC[]` slot array `Class866E8__ResetAllElements`
walks, tearing each entry down: calls `self->methods->slot88(self, 6,
entry, i)` (same call shape as `Class866E8__ResetAllElements`); refreshes-and-discards
`entry->unk4` through its base-class `unk04` slot; for `entry->unk8`,
refreshes-and-discards its OWN `unk2C` field first, then refreshes AND
STORES BACK `entry->unk8` itself through its own `unk04` slot (note: this
proves `UnkSlotListObj_3ac78` has a `methods` pointer at offset 0, not
just the `unk2C` field `Class866E8__ResetAllElements` already established); refreshes-
and-discards a new field `entry->unkC`; scans a 0x668-byte array of
`GenericObject*` at `entry->unk10`, refreshing-and-discarding every
non-NULL entry; frees `entry->unk10` via `BMemPMgrFree`. After the loop:
calls `((*(void***)func_800428E4(self))[3])(self)` — resolves a table
through the return of `func_800428E4(self)` and calls its slot `+0xC`.

New struct/vtable knowledge added regardless of the stall: `UnkSlotEntry_
3ac78::unkC` (`GenericObject *`) and `::unk10` (`GenericObject **`, a
0x668-byte scan array); `UnkSlotListObj_3ac78::methods` (a
`GenericMethodsHeader *` at offset 0 — this struct was previously only
known to have `unk2C` and implicitly started with `0x2C` bytes of
padding; now it starts with the shared `methods` pointer instead);
`UnkSlotChildMethods_3ac78::unk04` (same shared BasicClass slot pattern,
now confirmed on this class too); `Class866E8Methods::slot14`
(`Class6B5CC__RemoveChild`, not decompiled). `GenericMethodsHeader`/`GenericObject`
were converted from anonymous-struct typedefs to named-struct typedefs
(zero behavior change) so they could be forward-declared for use inside
`UnkSlotEntry_3ac78`, which itself has to be defined before `Class866E8`
for the SAME reason `Class866E8__ResetAllElements`'s commit documented (array member
needs a complete type).

## Best-reached body (does NOT compile to retail bytes — DRIFTED, treat the
62/113 as informational only, not a real partial score)

```c
#if 0
extern void *func_80020C5C(Class866E8 *self);
extern void BMemPMgrFree(void *arg1);
extern void *func_800428E4(Class866E8 *self);

void Class866E8__Finalize(Class866E8 *self)
{
    s32 i;
    s32 offset;
    void *tmp;
    UnkSlotEntry_3ac78 *entry;
    GenericObject **p;
    GenericObject **end;
    GenericObject *obj;
    void (*fn)(Class866E8 *self);

    tmp = func_80020C5C(self);
    self->methods->slot14(self, tmp);

    offset = 0xEC;
    for (i = 0; i < 7; i++) {
        entry = (UnkSlotEntry_3ac78 *)((u8 *)self + offset);
        self->methods->slot88(self, 6, entry, i);

        if (entry->unk4 != NULL) {
            entry->unk4->methods->unk04(entry->unk4);
        }

        if (entry->unk8 != NULL) {
            if (entry->unk8->unk2C != NULL) {
                entry->unk8->unk2C->methods->unk04(entry->unk8->unk2C);
            }
            entry->unk8 = (UnkSlotListObj_3ac78 *)entry->unk8->methods->unk04(entry->unk8);
        }

        if (entry->unkC != NULL) {
            entry->unkC->methods->unk04(entry->unkC);
        }

        p = entry->unk10;
        end = (GenericObject **)((u8 *)p + 0x668);
        while (p < end) {
            obj = *p;
            p++;
            if (obj != NULL) {
                obj->methods->unk04(obj);
            }
        }

        BMemPMgrFree(entry->unk10);
        offset += 0x1C;
    }

    fn = *(void (**)(Class866E8 *))((u8 *)func_800428E4(self) + 0xC);
    fn(self);
}
#endif
```

## The residue: a conditional-skip-to-loop-check duplicates the loop test

Words 0-69 are mostly register-identity noise (same instructions,
`$s2`/`$s3`/`$s4` swapped in a few spots — not investigated further since
the real blocker is downstream). The genuine structural break is the
`entry->unk10` scan loop (retail's `+0x668`-byte array walk):

Retail computes the "keep looping" test (`sltu $v0,$s0,$s2`) in exactly
ONE place — right after the conditional-skip target — and both the
initial loop-entry check AND the per-iteration continue-check branch to
that SAME instruction. This function's array-scan loop is structurally
identical to `Class866E8__ResetAllElements`'s own inner "call each non-NULL entry"
pattern (`if (obj != NULL) methods->unk04(obj);` inside a walk), which
matched cleanly there — but HERE, wrapped in an explicit bounded `p < end`
loop (rather than `Class866E8__ResetAllElements`'s simpler fixed-count `for`), every
reconstruction produces the test in TWO places: once as the shared
continue-check (matching retail) and AGAIN duplicated right after the
conditional skip, adding a redundant `sltu` and an extra `j` — 4 extra
instructions total, which is exactly the address-drift source.

**Three source shapes tried, all producing byte-IDENTICAL output** (this
is itself informative — GCC 2.6.3 canonicalizes all three to one internal
form before this duplication happens):

1. `if (p < end) { do { ...; if (obj != NULL) {...} } while (p < end); }`
   — the idiomatic "guarded do-while" shape that closed an analogous
   residue on `ProcessDreamAuxTriggerRecord` per DECOMPILATION_LEARNINGS.
2. Literal `goto`s mirroring retail's exact label layout (`if (p>=end)
   goto scanEnd; scanLoop: ...; if (obj==NULL) goto scanCheck; ...;
   scanCheck: if (p<end) goto scanLoop; scanEnd:`) — the same lever that
   fixed `Class866E8__OnElementEvent`'s three-way dispatch this round. No effect here;
   identical object code to (1).
3. Plain `while (p < end) { ...; if (obj != NULL) {...} }` — GCC's own
   loop-rotation pass evidently normalizes this to the SAME do-while-with-
   preheader shape as (1)/(2) before the duplication occurs, so it's
   byte-identical too.

None of these got far enough to test whether the OUTER per-entry loop
(the `for (i = 0; i < 7; i++)`) has its own version of `Class866E8__ResetAllElements`'s
still-unresolved "offset increment lands in the wrong delay slot" residue
— the inner scan loop's drift makes every word past it untrustworthy, so
that comparison was not reachable this round.

### What's actually going on (best guess, unconfirmed)

This looks like the SAME class of GCC 2.6.3 quirk noted in
`Class866E8__ResetAllElements.md` — a scheduling/duplication choice that resists control-
flow-graph-level source changes (goto vs. loop-construct vs. do-while all
produced the identical wrong answer here, which is a stronger and more
useful negative result than `Class866E8__ResetAllElements`'s single failed lever). Given
THREE different CFG-equivalent spellings compiled identically, the
duplication is very likely happening in a pass that operates on an
already-canonicalized internal representation, downstream of anything a
source rewrite can influence — this is closer to a genuine "not reachable
from C" case than an unexplored reshape, though it has not been tested
against the permuter (see `docs/PARALLEL-RUNS.md` Gate 3).

### Proposed learning

**When multiple genuinely different C control-flow spellings (do-while,
goto, while) all compile to the IDENTICAL object code, that is itself a
signal the residue is not reachable by further source reshaping** — worth
recognizing early rather than trying a fourth or fifth variant of the same
CFG. This is a new, sharper version of the "read the branch targets before
classifying" rule from DECOMPILATION_LEARNINGS: here the branch targets
(CFG) were verified equivalent across three spellings, and the codegen
still didn't move, which rules out reshaping as a category rather than
just one attempt at it.

## Provenance

round 2026-09-02 (head-requested extension), runner ALPHA, unit
class_3ac78. Struct/vtable knowledge for the whole function fully derived
and cross-checked against `Class866E8__ResetAllElements`'s already-established
`unkEC[]`/generic-slot patterns; three control-flow-equivalent source
attempts on the one scan loop that drifts, all byte-identical to each
other and none matching retail. Moved on to close out the round.
Restored to `INCLUDE_ASM`.

## Round 19 (echo): MATCHED, 113/113, whole-image green

Re-verified the "62/113, drifted" claim first: the preserved body above
does NOT compile as-is against the CURRENT header (`func_80020C5C` and
`func_800428E4` have both since been given real, no-argument signatures
by the sibling ctor `Class866E8__Class866E8`'s own successful match -- see
`src/class_3ac78.c`'s own declarations, `extern s32 func_80020C5C(void);`
and `extern BaseCtorTable_3ac78 *func_800428E4(void);` -- rather than the
`(self)`-taking guesses this report's preserved body used). This alone
means the round-13 62/113 score was measuring a body that would not even
build against today's header; it was not re-derivable verbatim.

**The decisive resource was the ALREADY-MATCHED sibling ctor,
`Class866E8__Class866E8` (same unit, same class, right above this function in
`src/class_3ac78.c`), which allocates and fills the SAME `entry->unk10`
0x668-byte array this function tears down.** Its own byte-exact source
was read directly rather than re-guessing the idiom from scratch:

1. **`self->methods->slot14(self, (void *)func_80020C5C())`** and
   **`func_800428E4()->dtor(self)`** (a new `dtor` slot added to the
   locally-declared `BaseCtorTable_3ac78`, at `+0x00C`, immediately after
   the already-established `ctor` slot at `+0x008` -- purely additive,
   mirrors the "further-base ctor/dtor" pattern already documented on
   `ctor`) -- both call shapes copied directly from the ctor's own
   `func_800428E4()->ctor(self)` / `self->methods->slot10(self,
   func_80020C5C())` pattern, just the destructor's own slots.
2. **The array-scan loop's pointer idiom, copied verbatim from the
   ctor's own matched body**: a THREE-variable chain --
   `cellp = entry->unk10; end = (u8 *)cellp + 0x668; p = (u8 *)cellp;`
   -- rather than assigning `p` directly from `entry->unk10` in one
   statement. This single change (adding the `Class866E8 **cellp`
   intermediate, otherwise inert) took the function from **73/113 (with
   drift)** to **85/113 (with drift)**, closing exactly the "retail has a
   redundant `self->unk10`-materializing move that a direct assignment
   doesn't reproduce" gap -- the SAME shape of residue documented
   elsewhere in this round for `func_8004EA38` (a MISSING redundant move
   there too), except here the sibling ctor happened to already show the
   fix.
3. **The remaining 1-word gap** (85/113, still drifted by exactly one
   word) was the scan loop's own closing test: three different
   CFG-equivalent spellings (`while`, guarded `do-while`, `goto`) all
   still compiled identically at this point, exactly as the round-13
   section above found -- confirming that finding rather than
   overturning it. What closed it was NOT a CFG change but a
   **statement-order swap inside the loop body**: moving `p += 4;` from
   immediately after `obj = *(GenericObject **)p;` (before the
   null-check) to AFTER the `if (obj != NULL) { ... }` block:

```c
/* 85/113, still 1 word short: */
obj = *(GenericObject **)p;
p += 4;
if (obj != NULL) {
    obj->methods->unk04(obj);
}

/* 113/113: */
obj = *(GenericObject **)p;
if (obj != NULL) {
    obj->methods->unk04(obj);
}
p += 4;
```

Result: **113/113, `build exit=0`, whole-image `OK: build matches retail
SLPS_015.56`.** Full match.

Final source (verbatim, now in `src/class_3ac78.c` in place of the
`INCLUDE_ASM`):

```c
void Class866E8__Finalize(Class866E8 *self)
{
    s32 i;
    UnkSlotEntry_3ac78 *entry;
    GenericObject *obj;
    Class866E8 **cellp;
    u8 *p;
    u8 *end;

    self->methods->slot14(self, (void *)func_80020C5C());

    for (i = 0; i < 7; i++) {
        entry = &self->unkEC[i];
        self->methods->slot88(self, 6, entry, i);

        if (entry->unk4 != NULL) {
            entry->unk4->methods->unk04(entry->unk4);
        }

        if (entry->unk8 != NULL) {
            if (entry->unk8->unk2C != NULL) {
                entry->unk8->unk2C->methods->unk04(entry->unk8->unk2C);
            }
            entry->unk8 = (UnkSlotListObj_3ac78 *)entry->unk8->methods->unk04(entry->unk8);
        }

        if (entry->unkC != NULL) {
            entry->unkC->methods->unk04(entry->unkC);
        }

        cellp = entry->unk10;
        end = (u8 *)cellp + 0x668;
        p = (u8 *)cellp;
        while (p < end) {
            obj = *(GenericObject **)p;
            if (obj != NULL) {
                obj->methods->unk04(obj);
            }
            p += 4;
        }

        BMemPMgrFree(entry->unk10);
    }

    func_800428E4()->dtor(self);
}
```

`include/class_3ac78.h` updated: `Class866E8Methods::dtor` and the
ctor/dtor summary comment marked MATCHED; `UnkSlotEntry_3ac78::unk10`'s
comment extended to note the dtor also treats it as a flat 0x668-byte
array of cells (consistent with, not contradicting, `Class866E8__DispatchToRectCells`'s
2D-grid-of-`Class866E8*` reading -- both read through the shared
`methods` slot at the same +0x000 offset). No struct layout changed, only
comments.

### Proposed learning

**When a stalled function shares an allocation/teardown pattern with an
ALREADY-MATCHED sibling in the same unit, read that sibling's own
byte-exact source before re-deriving the idiom from scratch.** Here the
matched ctor `Class866E8__Class866E8` had ALREADY worked out, byte-exact, both (a)
the correct no-argument signatures for two helper functions the stalled
report had guessed wrong, and (b) the exact three-variable pointer-walk
idiom (`cellp` -> `end` -> `p`, not a direct one-statement assignment)
that closed 12 of the destructor's 40 missing words in one change. The
round-13 report's own words/CFG analysis was sound but was working from
a stale, wrong-signature body; checking the sibling first would have
skipped most of the rediscovery.

Separately: **a scan loop's closing test can be reshaped after all, even
when three CFG-equivalent CONTROL-FLOW spellings compile identically --
the lever here was a plain STATEMENT-ORDER swap inside the loop body**
(moving the pointer increment from before to after the conditional call),
not a change to the loop's own shape. The round-13 finding "multiple
genuinely different C control-flow spellings that compile to identical
object code rules out reshaping as a category" was correct as far as it
went (CFG-level reshaping truly was exhausted) but did not generalize to
in-body statement ordering, which was the axis that actually worked.

## Naming

Round 67 (track 3, naming pass).

| symbol | name | tier | evidence |
| --- | --- | --- | --- |
| `func_8004A7C0` | `Class866E8__Finalize` | A | Occupant of vtable slot `+0x00C`. `include/code_8220.h` establishes that slot as `BasicClassMethods::finalize` (the virtual teardown), distinct from `+0x004` `release` (finalize, then free self). This body matches: it tears down every `elems[]` entry and tail-calls the base table's own `+0x00C`. This CORRECTS the inherited hypothesis -- the field and this report both called it `dtor`, which is `release`'s job, not this slot's. |

Slot name changed in `include/class_3ac78.h` accordingly:
`Class866E8Methods::dtor` -> `finalize`.

This function is also the second independent witness that the shared
`+0x004` slot on every object it touches is BasicClass's `release`: it uses
the `x = x->methods->release(x)` release-and-store-back shape on
`entry->target`, `entry->list->unk2C`, `entry->list` itself and
`entry->cellParent`. The local views' `unk04` slots were renamed `release`
on that basis.
