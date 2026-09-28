# Entity__Deactivate

> Renamed from `func_8005DA3C` on 2026-09-19 (tools/rename.py). Address 0x8005da3c.

**Unit:** Entity · **Size:** 28 words · **Status:** MATCHED (28/28 words, whole-image build verified byte-exact)

## What it does

A teardown counterpart to `Entity__Activate` (which sets `this->unkF0 = 1` via
`this->methods->slot60(this, 1)`): calls `slot60(this, 0)`, then two more of
this entity's own current vtable slots (`slot16C`, newly typed this round;
`slot164`, also newly typed), then clears `this->unkF0`.

## Final C

```c
void Entity__Deactivate(Entity *this) {
    this->methods->slot60(this, 0);
    this->methods->slot16C(this);
    this->methods->slot164(this, 0);
    this->unkF0 = 0;
}
```

## Attempt log

Matched on the first attempt.

## Proposed learning

None new.

## Naming

**Tier B.** Renamed from `func_8005DA3C` this round (tools/rename.py). The
exact mirror of `Entity__Activate` (clears `this->unkF0`, dispatches
`slot60(this, 0)`, plus two more self-only slot calls). Same naming
rationale as Activate: neutral on purpose, to avoid colliding with the
unrelated "Link" vocabulary already established for `Entity__GetLinkStage`.

## Proposed field names

- `EntityMethods::slot160` -> `deactivate` -- **tier B.** `tools/
  classtable.py` on `gEntityMethods` resolves +0x160 to this very function
  (self-referential dispatch). CROSS-UNIT: called from Entity/d/e/f/g
  (grep -rn -- '->slot160(' src/Entity_*.c), so proposed rather than
  applied even though this unit's OWN three callers
  (`Entity__DetachFromParent`, `Entity__OnGridCellLinkCommand`, `Entity__
  UpdateDeactivationState`) were updated to reflect the finding in the
  header comment directly.

## Track 4 (2026-09-26, round 88, echo)

The class (id 0x1F234, table `gEntityMethods`) is unified as `Entity` in `include/Entity.h`: a TodActor subclass whose table and object expand `TODACTOR_SLOTS`/`TODACTOR_FIELDS`. Any source block above is the pre-unification spelling; the live body takes the inherited names (fields `parent`, `coord2`->`tx/ty/tz`, `tick`, `linkTarget`, `state` (was `moodState`), `lastOffsetValue`, `grid` (was `unk4C`), `ticker` (was `companion2`), `arg2` (was `soundCueChannel`), `parts`, `todPlaying`, `peer` (was `target`, cast to the `Unk94Obj` DreamSys view where its own slots are called); slots `reset`, `setDisplay`, `setLightMode`, `setTranslation`/`addTranslation`, `moveLocalZ/X/Y`, `moveLocalZOrFindLink`, `selectTickCallback`, `enableTickCallback`/`disableTickCallback`, `distanceToPeer`, `setTargetReached`, `updateActivationState`/`updateDeactivationState`), byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).
