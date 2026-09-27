# IntermediateBase__OnState2 — MATCH (16/16 words, 2 attempts)

> Renamed from `Obj86B60__NotifyTargetReset` on 2026-09-25 (tools/rename.py). Address 0x8003e538.

> Renamed from `func_8003E538` on 2026-09-19 (tools/rename.py). Address 0x8003e538.

**Unit:** code_2cc8c_c · **Size:** 16 instructions

## What it does

A shared "base-class" method, reached through `Obj86B60Methods::slot64`
-- and, per `include/class_39e08.h`'s own note on an UNRELATED class
(`Obj865C8Methods`/`TimedTaskMethods` both list `IntermediateBase__OnState2` at their
own `+0x064`), this function is genuinely shared across multiple otherwise-
unrelated classes at the same vtable slot, not something `Obj86B60`
introduces itself.

Body: zero `self->unk1C`, then dispatch through a NEW field,
`self->unkC` (a pointer-to-a-pointer-to-object, `Obj86B60UnkC` in the
header): `self->unkC->target->methods->slot48(target)`.

## The C

```c
void IntermediateBase__OnState2(Obj86B60 *self)
{
    Obj86B60UnkCTarget *target;

    self->unk1C = 0;
    target = self->unkC->target;
    target->methods->slot48(target);
}
```

## Residue and fix (1 wasted attempt, then matched)

**First attempt** wrote the two statements in the opposite order
(`target = self->unkC->target;` THEN `self->unk1C = 0;`) and scored 11/16
with several register-identity/extra-instruction diffs:

- retail computes `self->unkC` into `$v0`, THEN stores `self->unk1C = 0`
  (filling the MIPS1 load-delay slot with the independent store), THEN
  loads `$v0->target` directly into `$a0` -- `$a0` (no longer needed as
  `self`) is REUSED to hold `target`, so the eventual `jalr`'s argument is
  already in place with no extra move.
- my first attempt's statement order made GCC allocate `target` to a
  different register (`$v1`) than `$a0`, so it needed an extra
  `move $a0, $v1` right before the `jalr` -- one extra instruction, which
  is exactly the kind of thing that shifts every later function in the
  unit (funcdiff's "differs OUTSIDE this range" warning fired on all 8 of
  this round's functions until this was fixed, since this was the one
  that wasn't byte-exact).

**Fix: write `self->unk1C = 0;` BEFORE the `target = ...` assignment.**
Swapping just the statement order (no other change) made GCC allocate
`target` into `$a0` directly, matching retail exactly.

### Proposed learning

For a `void`-returning helper with an independent store sandwiched between
two dependent loads (here: load `self->unkC`, store `self->unk1C = 0`,
then load `->target` from what was just loaded), write the store in SOURCE
ORDER between the two loads, even though the store has no data dependency
on either and a scheduler-only view would say it "can go anywhere". GCC
2.6.3 does fill the MIPS1 load-delay slot with the independent store either
way, but which REGISTER it allocates to the value crossing from the first
load to the second appears to depend on source statement order, not just
data-flow -- reordering it changed a register-identity residue at zero
control-flow cost.

## Provenance

round 12 (2026-09-03), runner alpha, unit code_2cc8c_c. 2 attempts.

## Naming

**IntermediateBase__OnState2** (renamed from `func_8003E538`, round 55,
runner alpha). Tier B: `Obj86B60Methods::slot64` occupant (dispatched by
`IntermediateBase__SetState` on mode 2). Mechanics fully known: zeroes
`self->unk1C` (the frame counter) then forwards a notification to
`self->initArgs->target` (the retained InitArgs' own target object,
`Obj86B60UnkCTargetMethods::slot48`). The game-level MEANING of "target"
and of mode 2 vs mode 3 (see `IntermediateBase__OnState3`) is not
established, hence tier B; the name records the confirmed mechanic
(reset the counter, then notify the target) rather than a guessed role.

## Track 4 (2026-09-25, round 82, charlie)

The class is IntermediateBase (class id 0x30, gIntermediateBaseMethods; `tools/classtable.py gIntermediateBaseMethods` lists this function as one of its own occupants), declared once in include/IntermediateBase.h. `self` is now `IntermediateBase *`, not TaskCore's `Obj86B60` view; byte-identical. Renamed from Obj86B60__NotifyTargetReset. It occupies +0x064, which IntermediateBase__SetState runs on state 2, so the slot is `onState2` and so is this occupant. "Target" was the Obj86B60UnkC view's name for initArgs +0x000, the same field IntermediateBase__OnState3 reaches (which was called "Child"): both clear frameCounter and call that object, this one at its +0x048. Tier B.

## Track 7 (round 98, echo)

initArgs->drawSystem is the DrawSystem, so the call is its +0x048
`start` (DrawSystem__Start, include/DrawSystem.h), not the unit-local
`slot48`. Local `obj0` -> `drawSystem`. Byte-identical.

The unit-local view these calls went through was removed; it read, verbatim
(the only history in it is the "not established" claim, which Pad.h and
DrawSystem.h have since settled):

```c
/* One local reading of the objects IntermediateBase calls outside
 * BasicClass's slots: initArgs->unk0 (+0x048 in onState2, +0x04C in
 * onState3), initArgs->unk4 (+0x044, +0x048 in onTag1Notify) and unk10
 * (+0x044 in onTag1Notify). Their classes are not established; every call
 * passes the object alone. */
typedef struct IntermediateBaseLinked IntermediateBaseLinked;

typedef struct IntermediateBaseLinkedMethods {
    u8 pad000[0x044];
    void (*slot44)(IntermediateBaseLinked *self); /* +0x044 */
    void (*slot48)(IntermediateBaseLinked *self); /* +0x048 */
    void (*slot4C)(IntermediateBaseLinked *self); /* +0x04C */
} IntermediateBaseLinkedMethods;

struct IntermediateBaseLinked {
    IntermediateBaseLinkedMethods *methods; /* +0x000 */
};
```
