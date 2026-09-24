# Obj6EAC0__SetChar — MATCHED (19/19), round 19

> Renamed from `func_80040854` on 2026-09-18 (tools/rename.py). Address 0x80040854.

## Round 19: closed via the permuter, `do { ... } while (0)` scoping lever

**Final body (byte-exact, full oracle green -- `build-and-verify.sh` exits
0):**

```c
void Obj6EAC0__SetChar(Obj6EAC0 *self, s32 a1, s32 a2, s32 a3)
{
    void (*fn)();
    Obj6EAC0 *q;

    q = self;
    fn = q->methods->slot4C;
    /* The do/while(0) wrapper is a no-op scoping device, load-bearing for
     * delay-slot scheduling only -- see below. Without it GCC swaps the
     * prologue's $ra/$s1 callee-save STORE ORDER. */
    do {
        fn(q, a1, a2, a3);
        q->unk48 = 0;
        q->unk4C = a3;
    } while (0);
}
```

Ran the permuter this round (the one direction the round-2 report flagged
as untried, seeded directly from attempt 1's body as recommended).
`--debug --stack-diffs` confirmed the base score of 18 (2 register
differences at weight 5, plus 8 stack differences at weight 1 -- the
report's "prologue store order" framing was right, but `--stack-diffs`
had never actually been run against it before, so the 8-point stack
component was previously invisible). At `-j 6 --stack-diffs
--stop-on-zero --best-only`, a zero was found in 1579 iterations
(seconds, not the full 300s budget).

The zero candidate wrapped the call-plus-two-stores in a `do { ... }
while (0)` and used permuter-generated names (`new_var`/`new_var2`) for
the two locals. Verified the LITERAL candidate against the real oracle
first (per `docs/MATCHING-GUIDE.md`'s "a permuter zero is a lead, not an
answer") -- genuinely 19/19, `build-and-verify.sh` exit 0. Then
translated to idiomatic naming:

- Plain variable renames (`new_var`->`q`, `new_var2`->`fn`), wrapper kept:
  still 19/19, unchanged.
- **Removing the `do/while(0)` wrapper and using a PLAIN `{ }` block
  instead** (testing whether it was really about SCOPING, not the loop
  construct specifically): regressed straight back to 17/19, the exact
  original residue (prologue `$ra`/`$s1` store order swapped). So the
  loop construct itself is load-bearing, not merely a new scope.
- **This is a recognised, already-precedented project idiom, not a
  UB-flavoured trick to reject:** `docs/DECOMPILATION_LEARNINGS.md`
  documents the exact same lever from `Class86F88__AddChildAndSetState` ("A `do { ... }
  while (0)` wrapper around an otherwise-unconditional body can be
  load-bearing for delay-slot scheduling... mechanism unexplained"), and
  `src/Entity_c.c`'s `func_8005F544` already ships it in matched,
  committed code with an identical comment ("load-bearing for register
  allocation only... without it GCC swaps which callee-saved register
  holds `this` vs `out`"). This is a THIRD confirmed instance of the same
  lever, now specifically for prologue STORE ORDER rather than register
  identity -- worth broadening that learning's own wording to cover both
  symptoms.

### Proposed learning

Broadens the existing `do { ... } while (0)`-wrapper learning
(`Class86F88__AddChildAndSetState`, `func_8005F544`) with a third data point: the same
no-op-loop lever also fixes a pure **prologue callee-save STORE ORDER**
residue, not just a register-identity swap. Given this project's own
"prologue store order is not reachable from C" class (this function's
own round-2 history below, plus the general note in
`DECOMPILATION_LEARNINGS.md`) has now been broken exactly once, by
exactly this lever, it is worth trying BEFORE writing off a same-length,
pure-store-order residue as unreachable -- especially since a plain `{ }`
scope block does NOT reproduce it (confirmed above), so this is
genuinely about the loop construct's effect on GCC 2.6.3's scheduler, not
merely variable lifetime/scoping.

## Round-2 history (superseded, kept for the record)

STALL (prologue store-order residue, 17/19 words)

Unit: `src/code_2cc8c_f.c`. Blocker screen clean. Correct length (19/19
words at the right total size, confirmed via the funcdiff word count
matching the `.s` file's own `nonmatching Obj6EAC0__SetChar, 0x4C`).

```c
void Obj6EAC0__SetChar(Obj6EAC0 *self, s32 a1, s32 a2, s32 a3) {
    self->methods->slot4C(self, a1, a2, a3);
    self->unk48 = 0;
    self->unk4C = a3;
}
```

Forwards all 4 incoming args unchanged through `self->methods->slot4C`
(the widest observed use of that slot -- see
`include/code_2cc8c.h`'s `Obj6EAC0Methods` comment), then
unconditionally zeroes `unk48` and sets `unk4C` from `a3`.

## The residue

Both callee-saved registers (`$s0` for `self`, `$s1` for `a3`) and
their STACK OFFSETS are exactly right; only the STORE ORDER in the
prologue is swapped: retail stores `$ra` (`0x18`) before `$s1`
(`0x14`); every C shape reached here stores `$s1` before `$ra`. This is
`docs/DECOMPILATION_LEARNINGS.md`'s documented "prologue callee-save
store ORDER is not reachable from C" class.

## Attempts (4)

1. Direct body (as shown above) -- 17/19, prologue order only.
2. Bare `__asm__("");` as the function's literal first statement (the
   documented permitted lever for this class) -- made it WORSE, not
   better: the diff grew to cover words 3 through 12+, not just the
   2-word prologue window. Per CLAUDE.md rule 6's own test ("verify by
   removing it and confirming register allocation is unchanged"), this
   is disqualifying -- the barrier changed which registers held which
   values elsewhere in the function, not just instruction order, so it
   is REJECTED here even though it is the class's usual lever.
   Reverted immediately.
3. `s32 v = a3;` before the call, `self->unk4C = v;` after -- no
   change from attempt 1 (identical 17/19).
4. Swapped the two trailing field-write statements
   (`unk4C` before `unk48`) -- WORSE: also swapped which of THOSE two
   stores comes first (15/19), confirming the original order
   (`unk48` then `unk4C`) is the one to keep and that this function's
   two independent residues (prologue order, field-write order) are
   at least not entangled with each other in a helpful way.

## Round 15 update: two more attempts, still resistant

Re-attempted after the coordinator flagged this as one of the three
closest stalls to revisit, with `Obj6EAC0__Construct`'s freshly-discovered
"leftover register" and "narrower cast" levers in mind. Neither
applied here -- unlike `Obj6EAC0__Construct`, the CALL setup itself is
already exactly right (retail's own `self->methods->slot4C` call also
forwards `self`/`a1`/`a2`/`a3` with NO explicit register moves at all,
since none of the four have been touched since function entry; the
`addu $s1,$a3,$zero` cache for the LATER `unk4C` store is scheduled
into the `jalr`'s own delay slot, which my C already reproduces
correctly -- this was never the residue). The residue really is
isolated to the two prologue `sw`s' order, confirming this is a purer
instance of the class than it first looked.

5. The previously-untried "unused padding local" lever
   (`Class65650__SetLightMode`'s precedent): `s32 pad;` declared before the call
   body, unused otherwise -- NO CHANGE (17/19, identical object code
   to attempt 1). The lever that worked for `Class65650__SetLightMode` does not
   generalise here.
6. `s32 result = a3;` used in place of `a3` for BOTH the call argument
   and the `unk4C` store (an explicit single named copy, rather than
   attempt 3's cache-after-call) -- NO CHANGE (17/19, identical).

Both untried directions from the original report are now closed
negatives. Nothing in this round's two new general levers (switch/
if-else physical layout divergence; splitting a fused boolean to
defeat cross-jump) applies -- there is no switch, no boolean
condition, and no duplicated call site here to split apart. This
looks like a genuine instance of "prologue store order is not
reachable from C" with no lever found across 6 attempts and two
sessions.

## Direction NOT tried, and why

**Did not try:** the permuter. This is a small (19-word), branch-free,
call-having body -- a plausible permuter target given how narrow the
residue is (exactly 2 words, pure register-store-order, no
insertions/deletions expected in a `--debug` base score). Not
attempted due to session time; if revisited, seed it directly from
attempt 1's body rather than re-deriving from scratch.

### Proposed learning

Three more confirmed negatives for the "prologue store-order" class
(barrier; padding local; named single-copy local), on top of the
existing non-generalisation note in
`docs/DECOMPILATION_LEARNINGS.md` ("It did NOT generalise: two
runners tried it on unrelated residues and worsened them"). Six
attempts across two sessions found no C-level lever at all for this
specific instance -- the permuter (untried here) is the most credible
remaining direction.

## Naming

Round 54 (alpha), FINISHING-PLAN track 3.

| was | now | tier |
| --- | --- | --- |
| `func_80040854` | `Obj6EAC0__SetChar` | B |

**Evidence.** Base occupant of `slotC4`: forwards `(a1, a2, a3)` to
`self->methods->slot4C` (the "layout" slot -- see `Obj6EAC0__Layout`),
then `self->unk48 = 0; self->unk4C = a3;`. `slotC4`'s DERIVED occupant
(`Obj6EAC0__SetChildChar`, this unit) indexes into `self->children[a2]`
and dispatches THAT child's own `slotC4` with `a1 & 0xFF` -- an 8-bit
value. `Obj6EAC0__SetText` (this unit, `slotCC`'s derived occupant)
independently walks a byte string dispatching `elem->methods->slotC4(elem,
*p)` per child -- i.e. `slotC4` is called elsewhere with individual
string bytes, which is the evidence this slot sets a per-glyph character
code rather than an arbitrary value. Tier B (mechanics + one clean
cross-reference; not independently confirmed against gameplay/rendering
code outside this unit).
