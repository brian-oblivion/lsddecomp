# GridCell__OnActorLinkCommand -- MATCHED (33/33 words), round 17 permuter pass

> Renamed from `Class86AA0__OnActorLinkCommand` on 2026-09-26 (tools/rename.py). Address 0x8004d47c.

> Renamed from `Class86AA0__ForwardIfArg2InRange` on 2026-09-26 (tools/rename.py). Address 0x8004d47c.

> Renamed from `func_8004D47C` on 2026-09-22 (tools/rename.py). Address 0x8004d47c.

**Unit:** class_3bb8c_c · **Size:** 33 words · **Status:** MATCHED.
Two prior rounds (including a head re-verification) confirmed this as a
genuine register-identity stall unreachable by every manual reshaping
lever available (see the full history below, kept for the record). A
round-17 permuter pass, run explicitly BECAUSE those two rounds had
proven the residue was small, isolated, and free of the misdiagnosis
risk `funcdiff.py`/`docs/MATCHING-GUIDE.md` warn about (a "long attempt
list" that only explored one axis) closed it.

## Round 17: what the permuter found and how it was verified

`tools/setup-permuter.sh` scaffolded from the report's own preserved
23/33 body below. `--debug` confirmed the base score (50) matched this
report's own description exactly: 10 register differences, ZERO
insertions/deletions/reorderings -- the textbook signature of a pure
register-identity residue, not a misdiagnosed structural gap. A search at
`-j 8` found a zero at iteration 122 (~2 minutes). The winning mutation
was a single, minimal change: wrapping the SECOND early-return guard in
a `do { ... } while (0)`:

```c
void GridCell__OnActorLinkCommand(GridCell *self, GenericTagInst_3bb8c_c *arg1, s32 arg2)
{
    GetSceneNodeMethods(self)->slot9C(self, arg1, arg2);
    if (arg2 >= 9) {
        return;
    }
    do {
        if (arg2 < 5) {
            return;
        }
    } while (0);
    self->methods->slotA0(self, arg1, arg2);
}
```

**This is the final source, committed as-is.** Before committing, this was
NOT taken on faith from the permuter's own scorer: the exact candidate was
recompiled through the pinned pipeline by hand (`cpp | cc1 | maspsx | as`,
same invocation as CLAUDE.md's own escalation recipe) and `objdump`-diffed
against `permuter-work/GridCell__OnActorLinkCommand/target.o` (retail's own bytes,
independently assembled by the setup script) -- byte-identical, not just
permuter-score-zero. Two plainer-looking alternatives were tried and
REJECTED because they did NOT reproduce retail: a bare `{ }` compound
block around the same `if` (no loop) put `self` back on `$s0` instead of
retail's `$s1`; a `while (arg2 < 5) { return; }` did the same. Only the
`do`/`while(0)` form -- not merely "some kind of extra scope" -- produces
retail's register rotation. `./build-and-verify.sh` confirms the
WHOLE-IMAGE SHA1, not just this function's own bytes.

**Why a `do { } while (0)` around a single guard is plausible as genuine
source, not just a scoring artifact:** it is a standard single-exit-point
idiom from exactly this era of C, and CLAUDE.md's own register-identity
rule provides the test that clears it as a legitimate fix rather than a
banned pin -- nothing here names a register or a hard-codes a specific
`$sN`; it is ordinary control-flow surface area that (for reasons internal
to GCC 2.6.3's local register allocator, not tested further here) changes
reference-count-driven coloring decisions. This is consistent with, and
narrows, the earlier head-verification round's "unresolved, not refuted"
note on the reference-count hypothesis below.

### Proposed learning

**A `do { <single guard> } while (0)` wrap around one of several
early-return guards is a real, reproducible lever for a pure
register-identity residue (zero insertions/deletions, all register
differences) on GCC 2.6.3/`-mips1`/`-O2`.** It is NOT equivalent to a bare
`{ }` compound block or to a `while` loop with the same body -- both were
tested and do NOT reproduce it, so the loop-with-constant-false-condition
construct specifically (not "add a scope") is what moves the allocator's
decision. Cheap to try by hand on a future 2-or-3-parameter register-swap
stall BEFORE reaching for the permuter, given how small and mechanical
the change is once you already suspect this class (see the "signature"
in the base-score debug output: all-register penalty, zero
insertion/deletion/reordering penalty).

## Full history (rounds 9-10, before the round-17 permuter pass)

## What it does

`GridCell`'s own slot +0x0B8 occupant (called by `GridCell__DispatchLinkCommand`, already
matched, as `self->methods->slotB8(self)` -- but see the arity note below).
Unconditionally forwards `(self, arg1, arg2)` to the base ctor table's own
`+0x09C` slot, then, if `5 <= arg2 < 9`, forwards the same three arguments
to `self`'s own `+0x0A0` slot.

## The C (closest attempt, 23/33 -- shape-correct, register-identity wrong)

```c
void GridCell__OnActorLinkCommand(GridCell *self, GenericTagInst_3bb8c_c *arg1, s32 arg2)
{
    GetSceneNodeMethods(self)->slot9C(self, arg1, arg2);
    if (arg2 >= 9) {
        return;
    }
    if (arg2 < 5) {
        return;
    }
    self->methods->slotA0(self, arg1, arg2);
}
```

An equivalent nested-`if` form (`if (arg2 < 9) { if (arg2 >= 5) { ... } }`)
and a version with an explicit `BaseCtorTableB_3bb8c_c *table` local for the
getter's return value both produced the IDENTICAL score and the IDENTICAL
residue -- branch/comparison shape is not the problem.

## Why it stalls: genuine register-identity mismatch

Retail's own asm assigns the three incoming parameters to callee-saved
registers as `a0(self)->$s1`, `a1(arg1)->$s2`, `a2(arg2)->$s0` -- a
rotation, not the naive `a0->$s0, a1->$s1, a2->$s2` order. Every attempt
above compiles to the naive order instead (`self->$s0`, `arg1->$s2`,
`arg2->$s1`), and the SAME swap (`self`/`arg2` trade places, `arg1` stays
on `$s2` in both) is consistent in every one of the three saves/reloads and
every one of the four places each value is read back -- not a one-off
scheduling residue. Per CLAUDE.md rule 6, this is the literal test for a
banned-fix-required mismatch: *"if removing it changes WHICH REGISTER
holds a value, it is banned."* A bare `__asm__("")` scheduling barrier
(the documented, PERMITTED lever for a *prologue callee-save store order*
residue, see `docs/DECOMPILATION_LEARNINGS.md`'s
"Prologue callee-save store ORDER is not reachable from C" entry) was
tried as the function's first statement and made things WORSE (3/33, plus
an out-of-range size shift) rather than fixing the identity swap -- that
entry's lever is for a different residue class (instruction order within
one register's use, not which register a value lives in) and does not
generalise here.

No source-level reshaping tried (combined `&&`, nested `if`, `goto`+early
`return`, with/without an intermediate temp for the getter's return value)
changed which hard register `self` vs `arg2` landed in. This function's
size (33 words) and total structure otherwise matches -- only three
registers' identities are swapped throughout.

## New/changed header content (`include/class_3bb8c.h`)

Kept even though the function itself stalls -- these are genuine,
evidence-based struct/table facts established from reading this function's
own disassembly, independent of the register-identity residue (same
policy as `ComputeCellWorldOffsets`'s report, which kept its prototype after a
near-miss):

- **`GridCellMethods`**: added `slotA0`
  (`void (*)(GridCell *self, GenericTagInst_3bb8c_c *arg1, s32 arg2)`,
  +0x0A0), called by this function when `5 <= arg2 < 9`. Additive; existing
  `ctor` (+0x008) and `slotB8` (+0x0B8) fields untouched, only the padding
  between them was split to make room.
- **`GenericTagInst_3bb8c_c`**: added a forward `typedef struct
  GenericTagInst_3bb8c_c GenericTagInst_3bb8c_c;` ahead of
  `GridCellMethods` (needed for `slotA0`'s parameter type; the type's
  full definition, further down the file, already existed from
  `GridCell__DispatchLinkCommand`'s round). The later definition's OWN `typedef ... {
  } GenericTagInst_3bb8c_c;` had to become a plain `struct
  GenericTagInst_3bb8c_c { ... };` (drop the trailing re-typedef) because
  cc1 (this project's pinned GCC 2.6.3) rejects redefining an existing
  typedef name even to an identical type -- `include/class_3bb8c.h:577:
  redefinition of 'GenericTagInst_3bb8c_c'`. No byte-level effect: the
  named type is unchanged, only which line introduces the typedef name.
- **`GetSceneNodeMethods`'s return type changed from `BaseCtorTable_3bb8c_c *` to
  a NEW type, `BaseCtorTableB_3bb8c_c *`.** This function's own body reaches
  `+0x09C` on that getter's table with a 3-argument call
  (`self, arg1, arg2`), which conflicts in arity with
  `BaseCtorTable_3bb8c_c::slot9C` (1-argument, established this same round
  from `NodeGuardedViewport__Update` via the OTHER getter, `GetViewportMethods` -- a DIFFERENT
  global/table). Same-offset arity conflict means different table/different
  class, per this project's established split policy (see
  `TaskCoreObjMethods` in `include/code_2c054.h` for the precedent this
  follows). This is a type-NAME change only: `GridCell__GridCell`'s own
  already-matched call (`GetSceneNodeMethods(self)->ctor(self)`) only touches the
  `+0x008 ctor` slot, whose layout is byte-identical in both names, so
  renaming changes no bytes and does not disturb that match (confirmed:
  `./build-and-verify.sh` stays green with `GridCell__OnActorLinkCommand` back on
  `INCLUDE_ASM`).
- New `BaseCtorTableB_3bb8c_c` type: `ctor` at +0x008 (`GridCell__GridCell`),
  `slot9C` at +0x09C (`void (*)(void *self, void *arg1, s32 arg2)`, this
  function's own first, unconditional statement).

## Head verification, round 10 — the stall stands, but its shape is different

Verified the score and re-derived the residue independently. **The stall is
real and the classification is right.** Two corrections to how it is described,
and two levers spent so nobody spends them again.

**It is a 2-way exchange, not a 3-way rotation.** The report above says retail
uses `a0->$s1, a1->$s2, a2->$s0` against a naive `a0->$s0, a1->$s1, a2->$s2`.
But the attempt's own observed output is `self->$s0, arg1->$s2, arg2->$s1` —
so `arg1` sits on `$s2` in BOTH, and only `self` and `arg2` trade places. That
matters for the proposed learning, which generalises from "rotation" to a rule
about the last-declared parameter claiming the lowest saved register. That rule
does not describe this instance, and as a screen it would fire on functions
that are not this class. Corrected below.

**The ordering looks use-count driven, which suggests a lever — it does not
work.** GCC 2.6.3's local allocator prioritises pseudos by reference count, and
in retail `arg2` is read four times (two `slti`, two `addu $a2`) against two
each for `self` and `arg1`, which is consistent with `arg2` earning `$s0`.
Retail also emits **no `$a0` setup at all** before `jal GetSceneNodeMethods` — it
lets the incoming `$a0` stand — so retail's source called that getter with zero
arguments here, while the attempt writes `GetSceneNodeMethods(self)` and spends a
reference on `self`. Making the two reference counts match therefore looked
like the whole game.

Both ways of doing it failed:

1. **Block-scope `extern BaseCtorTableB_3bb8c_c *GetSceneNodeMethods();`** inside
   `GridCell__OnActorLinkCommand`, so the zero-argument call could coexist with
   `GridCell__GridCell`'s one-argument call in the same unit. Does not compile:
   C89 keeps the outer prototype in scope and applies its arity —
   `too few arguments to function 'GetSceneNodeMethods'`. A block-scope
   redeclaration cannot narrow an outer prototype.
2. **File-scope declaration changed to empty parens** (`GetSceneNodeMethods()`), no
   prototype, each call site passing its own argument list — which is what
   round 9 concluded retail's own source must have had. This compiles and is
   *worse*: `GridCell__OnActorLinkCommand` drops **23/33 -> 15/33**, and it regresses
   `GridCell__GridCell` from **20/20 -> 19/20**. So `GridCell__GridCell` genuinely needs
   the `$a0` setup that a prototype-less call elides, and round 9's
   do-not-reconcile note on this declaration is load-bearing in the other
   direction too: not just `(void)`, but any change that lets the argument
   setup be dropped.

The reference-count hypothesis is therefore **unresolved, not refuted** — the
counts could not be equalised without breaking a neighbouring match, so it was
never actually tested. That is the next lever if anyone returns here, and it
needs a way to drop one reference to `self` that does not touch the shared
declaration.

## Proposed learning

> **Head-corrected.** The runner's version below described this as a 3-way
> rotation and generalised to "last-declared param claims the numerically-lowest
> saved register". It is actually a 2-way exchange — `arg1` is on `$s2` in both
> retail and the attempt — so that generalisation does not hold and would
> misfire as a screen. What survives:
>
> **A callee-saved register assignment that differs from the compiler's by an
> EXCHANGE of two parameters is a genuine register-identity mismatch, not a
> scheduling residue** — and the likely mechanism is reference-count priority
> in GCC 2.6.3's local allocator, so before calling it unreachable, count how
> many times each parameter is read in RETAIL versus in your C. Equalising
> those counts is the lever. It could not be applied here only because the one
> surplus reference sat in a call whose declaration is shared with an
> already-matched neighbour.
>
> Original runner text, kept because its practical half stands:
>
> **A rotated (not merely reordered) callee-saved register assignment for a
> 3-parameter function -- `a0->$s1, a1->$s2, a2->$s0` instead of the naive
> `a0->$s0, a1->$s1, a2->$s2` -- is a genuine register-identity mismatch,
> not a scheduling residue.** Tested across four independently different
> source shapes (combined `&&`, nested `if`, `goto`+`return`, extra local
> temp) with zero effect on which hard register each parameter landed in;
> the bare-`__asm__("")`-as-first-statement lever documented for prologue
> *store order* residues does not generalise to this class and made this
> one worse. Treat this specific "rotation" signature (last-declared param
> claims the numerically-lowest saved register, others shift up by one) as
> a fast top-level screen for the same STALL class on sight, before
> spending attempts on source reshaping.

## Naming

**GridCell__OnActorLinkCommand** -- tier B. Mechanics are fully
established (round 17's permuter pass pinned the byte-exact body):
unconditionally forwards `(self, arg1, arg2)` to the base ctor table's own
`slot9C`, then, only when `5 <= arg2 < 9`, forwards the same three
arguments again to `self`'s own `slotA0`. Purpose is explicitly NOT
established -- the function's own history section already documents an
open arity/purpose conflict with its nominal vtable slot (`slotB8`, whose
call site from `GridCell__DispatchLinkCommand` passes only `self`, one
argument short of what this function's own body reads) that round 9/10
deliberately left unreconciled. Named for the one concrete, evidenced
mechanic (a numeric range gate on `arg2`) rather than for either disputed
caller's arity or for a guessed meaning of the `[5, 9)` band. `slotA0` and
`slotB8` (the vtable field names) are left unrenamed: this function's own
occupancy of `slotB8` is itself the open question the report documents, so
naming the slot after this function's behavior would misstate an
unresolved fact as settled.

## Track 4 (2026-09-26, round 88, alpha)

Renamed `GridCell__ForwardIfArg2InRange` -> `GridCell__OnActorLinkCommand`
(tools/rename.py). It is the sole occupant of gGridCellMethods's own slot
+0x0B8, and its only caller is `GridCell__DispatchLinkCommand`'s branch for
a sender whose class id byte is 0x34 (gActorMethods). The body is the same
as Actor's `onActorLinkCommand` base occupant: chain SceneNode's
dispatchLinkCommand, then for events 5..8 run `tryAttachNearby` with the
sender and event. The slot is named `onActorLinkCommand` after Actor's
+0x0DC, which handles the same sender class the same way. The call named
`slotA0` above is SceneNode's inherited `tryAttachNearby` (+0x0A0, occupant
`SceneNode__TryAttachNearby`); the unified header keeps SceneNode's slot
type and this caller casts at the call, as `Actor__OnActorLinkCommand` does.
The 1-argument `slotB8` declaration and the "not reconciled" note above are
retired: the call site has the three arguments in $a0..$a2 already.

The C since round 88 (byte-identical, whole image verified):

```c
void GridCell__OnActorLinkCommand(GridCell *self, void *sender, s32 event)
{
    GetSceneNodeMethods()->dispatchLinkCommand((SceneNode *)self, sender, event);
    if (event >= 9) {
        return;
    }
    do {
        if (event < 5) {
            return;
        }
    } while (0);
    ((void (*)(GridCell *, void *, s32))self->methods->tryAttachNearby)(self, sender, event);
}
```
