# func_8005DBF0

**Unit:** Entity · **Size:** 74 words · **Status:** STALL — 8 bytes / 2 words
short of byte-exact, everything else matches. Restored to `INCLUDE_ASM`.
Attempted round 2026-09-01 (runner bravo, Entity 11-function pass).

## What it does

Gated by `this->unkF0 == 0 && this->unk44 != 1`: looks up this entity's mood
row (`D_80089EA4[this->moodIndex]`) and decides whether to detach based on
`row->detachKind`:

- `detachKind == 4`: detach iff `(rand() & 0x7F) == 0`.
- otherwise, only if `row->unk5 != 0`: calls
  `func_8005D714(this, &this->unk14->x, row->unk5, row->unk9)`
  (`addiu_at`-blocked, but its call-site shape is fully known). If that call
  returned non-zero: detach iff `detachKind == 1`, or (iff `detachKind ==
  3`) with the SAME `rand()&0x7F==0` check the `detachKind==4` arm uses — a
  genuinely shared code path (retail reaches the one `jal <rand>` from both
  the `detachKind==4` case and this `==3` case). If it returned zero: detach
  iff `detachKind == 2`.

Detaching calls `this->methods->slot15C(this)`.

This is func_8005DD18's near-identical twin (see that report, matched in
the same pass) — same mood-row shape, same `func_8005D714` call, same
"detach based on a small kind enum, with one shared branch" structure. The
techniques that closed DD18 (branchy `if`, not boolean-expression
assignment; per-branch reloads, not a shared "expected" temp) got this one
from 37 words short down to 8, but not the rest of the way.

## Final C (compiles, size-correct, NOT byte-exact — preserved for the next attempt)

```c
#if 0
s32 func_8005DBF0(Entity *this) {
    EntityMoodRow *row;
    s32 doDetach;

    if (this->unkF0 == 0 && this->unk44 != 1) {
        row = &D_80089EA4[this->moodIndex];
        doDetach = 0;
        if (row->detachKind != 0) {
            if (row->detachKind == 4) {
                goto randCheck;
            }
            if (row->unk5 != 0) {
                if (func_8005D714(this, &this->unk14->x, row->unk5, row->unk9) != 0) {
                    if (row->detachKind == 1) {
                        doDetach = 1;
                    } else if (row->detachKind == 3) {
                    randCheck:
                        if ((rand() & 0x7F) == 0) {
                            doDetach = 1;
                        }
                    }
                } else if (row->detachKind == 2) {
                    doDetach = 1;
                }
            }
        }
        if (doDetach) {
            this->methods->slot15C(this);
        }
    }
    return this->unkF0;
}
#endif
```

## The residue

Retail's `detachKind == 2` case (the `func_8005D714`-returned-zero arm) does
**not** share its `doDetach = 1;` with the `rand()==0` case's `doDetach =
1;`, even though both are the literal same statement reaching the literal
same merge point. Retail spends 2 EXTRA instructions to keep them separate:

```
; kind==2 case (reached when func_8005D714 returned 0):
lb   $v1, 0x3($s0)
li   $v0, 0x2
bne  $v1, $v0, MERGE
 nop
j    MERGE
 li  $s2, 0x1          ; <-- retail's OWN dedicated "doDetach=1", in the delay slot of an otherwise-unconditional jump

; ... elsewhere, the rand-check-passed case ...
li   $s2, 0x1          ; <-- a SECOND, textually distinct "doDetach=1"
MERGE:
beqz $s2, SKIP
```

This build's compiler (the SAME pinned toolchain) instead recognizes the two
`doDetach = 1;` writes as reaching an identical control-flow join and lets
the `detachKind==2` case simply fall through into the SAME `li $s2,1` the
rand-check-passed case uses — objectively fewer instructions, and correct,
but not what retail has.

## What was tried (11+ structural variants, all logically equivalent)

- Both `switch (row->detachKind) { case 3: ...; case 1: ...; }` (Duff's-device-style
  fallthrough matching m2c's own "irregular switch" reading) and the
  equivalent `if`/`else if`/goto form — byte-identical output either way, so
  switch-vs-if is not the axis that matters here.
- Both orderings of the primary gate (`if (func_8005D714(...) != 0) {kind1/3}
  else if (kind==2) {...}` vs. the inverted `if (...== 0) {kind==2} else
  {kind1/3}`) — the inverted form does NOT reproduce retail's layout order
  either; instead it triggers a DIFFERENT unwanted optimization (GCC
  recognizes `if (kind==2) { doDetach=1; } /* nothing else in this arm */`
  as a boolean-settable pattern and compiles it via `xori`/`sltiu` instead
  of a branch — worse, and a new failure mode, not a step closer).
- Reordering which of `detachKind==1` / `detachKind==3` is checked first
  inside the `dist != 0` arm — changes the compiled layout, does not fix the
  duplication, and stops matching retail's proven-correct instruction ORDER
  for the parts that already matched.
- A scheduling barrier (`__asm__("")`, the one legitimate lever for
  order-only residues) placed both before and after the `detachKind==2`
  arm's `doDetach = 1;` — no effect, confirming this is a cross-basic-block
  CFG/tail-merge decision, not an intra-block scheduling one, and therefore
  not something the permitted barrier can influence.
- The argument-register lever (per the head's mid-round broadcast): checked
  every call in this function (`func_8005D714` — already fully matched,
  4-argument call site confirmed correct; `rand()` — genuinely no
  arguments, confirmed against `include/psyq/RAND.H`'s
  `extern int rand(void);`; `this->methods->slot15C(this)` — single-argument
  vtable dispatch, matches). **No hidden-argument instance found in this
  function** — every call's argument registers are fully accounted for.
  (This lever DID close the analogous residue in `func_8005DD18` — see that
  report — so it was worth checking carefully here too; it just isn't the
  answer for this specific function.)

## Proposed learning

Add a fourth entry to `docs/MATCHING-GUIDE.md`'s residue list, tentatively:
**"Identical assignment, different merge points."** When two textually
identical simple statements (here, `doDetach = 1;`) are reached from
different predecessors and retail keeps them as two separate instructions
where this compiler tail-merges them into one, that is NOT a
declaration-order or scheduling residue — no reshaping of the surrounding
`if`/`else`/`switch`, no barrier, and no argument fix moved it in this case.
It may be specific to how many total predecessors converge on the shared
target (2 in `func_8005DD18`'s analogous spot, which matched; 3 here, which
didn't) — worth testing on the next instance of this shape before spending
another 10+ attempts re-deriving the same negative result.
