# Entity__Activate

> Renamed from `func_8005D9F4` on 2026-09-19 (tools/rename.py). Address 0x8005d9f4.

**Unit:** Entity · **Size:** 18 instructions · **Status:** MATCHED (18/18 words, whole-image build verified byte-exact)

## What it does

Calls this entity's own vtable slot `+0x060` with a literal argument (`1`),
then sets two flag words: `this->unkF0 = 1;` and `this->unk24 = 0;`.

## Derivation

```
lw   $v0, 0x0($s0)          ; v0 = this->methods
lw   $v0, 0x60($v0)         ; v0 = methods->slot60
jalr $v0                     ; this->methods->slot60(this, 1)
 ori $a1, $zero, 0x1
ori  $v0, $zero, 0x1
sw   $v0, 0xF0($s0)          ; this->unkF0 = 1
sw   $zero, 0x24($s0)        ; this->unk24 = 0
```

## Final C

```c
void Entity__Activate(Entity *this) {
    this->methods->slot60(this, 1);
    this->unkF0 = 1;
    this->unk24 = 0;
}
```

`slot60` is typed `void (*slot60)(Entity *self, s32 arg1);` in
`EntityMethods` (`Entity.h`) — a vtable-dispatched call to a still-unnamed,
possibly-uncarved function, so per `docs/match-reports/
Class65650__Class65650.md`'s finding no extern prototype is needed for the
target itself, only a correctly-typed function-pointer field.

## Attempt log

Matched on the first attempt.

## Proposed learning

None new — a clean instance of the vtable-dispatch-needs-no-forward-
declaration pattern already documented for `code_55dd4`.

## Naming

**Tier B.** Renamed from `func_8005D9F4` this round (tools/rename.py). Sets
`this->unkF0 = 1`, clears `this->unk24`, dispatches `slot60(this, 1)`.
"Activate" is a deliberately NEUTRAL description of the `unkF0` toggle,
chosen over a more evocative "Detach"/"Attach" pairing that would overload
`Entity__GetLinkStage`'s already-established, semantically DIFFERENT "Link"
vocabulary (a per-mood table, not this flag) -- see
`Entity__UpdateActivationState.md` for the full reasoning, which reached the
same tie-break the other way and documents why. Pairs with
`Entity__Deactivate`.

## Proposed field names

- `Entity::unkF0` -> `active` -- **tier B.** Toggled by this function and
  its pair; read directly (not just through the accessor pair) by
  Entity_b.c (`grep -rn -- '->unkF0\b' src/Entity_b.c`). CROSS-UNIT,
  proposed rather than applied.

## Track 4 (2026-09-26, round 88, echo)

The class (id 0x1F234, table `gEntityMethods`) is unified as `Entity` in `include/Entity.h`: a Class65650 subclass whose table and object expand `CLASS65650_SLOTS`/`CLASS65650_FIELDS`. Any source block above is the pre-unification spelling; the live body takes the inherited names (fields `parent`, `coord2`->`tx/ty/tz`, `tick`, `linkTarget`, `state` (was `moodState`), `lastOffsetValue`, `grid` (was `unk4C`), `ticker` (was `companion2`), `arg2` (was `soundCueChannel`), `parts`, `todPlaying`, `peer` (was `target`, cast to the `Unk94Obj` DreamSys view where its own slots are called); slots `reset`, `setDisplay`, `setLightMode`, `setTranslation`/`addTranslation`, `moveLocalZ/X/Y`, `moveLocalZOrFindLink`, `selectTickCallback`, `enableTickCallback`/`disableTickCallback`, `distanceToPeer`, `setTargetReached`, `updateActivationState`/`updateDeactivationState`), byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).
