# Entity__DetachFromParent

> Renamed from `Entity__DetachUnk4C` on 2026-09-26 (tools/rename.py). Address 0x8005d418.

> Renamed from `func_8005D418` on 2026-09-19 (tools/rename.py). Address 0x8005d418.

**Unit:** Entity · **Size:** 26 words · **Status:** MATCHED (26/26 words, whole-image build verified byte-exact)

## What it does

A conditional teardown: if `this->unk0C` (a gate flag; see the parallel
noted in `entity.h`'s struct comment with `TodActor`/`DreamSys`'s own
shared-base `+0xC` gate) is set, calls this entity's own current
`methods->slot160(this)`, then the shared "BasicClass" ancestor's own
`slot50` (`GetTodActorMethods()->slot50(this)`, offset `+0x050` in the shared
table), then clears `this->unk4C`.

## Final C

```c
void Entity__DetachFromParent(Entity *this) {
    if (this->unk0C != 0) {
        this->methods->slot160(this);
        GetTodActorMethods()->slot50(this);
        this->unk4C = 0;
    }
}
```

## Attempt log

Matched on the first attempt.

## Proposed learning

This function (along with `Entity__OnGridCellLinkCommand`, `Entity__Update`, `Entity__GetOrCreateFadeBox`)
is what established `GetTodActorMethods()`'s SHARED-vtable role for this unit —
see the class-framework comment block now at the top of `entity.h`. Worth
flagging for anyone touching `Entity` next: `GetTodActorMethods()` (matched,
`code_55dd4.c`) is not TodActor-specific despite living in that unit and
returning `TodActorMethods*` there — it's the common ancestor's vtable
accessor, reused verbatim by Entity. A local, Entity-scoped `BasicClassMethods`
view of the SAME table (rather than importing `TodActorMethods` and
coupling the two units) is in `entity.h` now, typed only as far as this
unit's own call sites need.

## Naming

**Tier B.** Renamed from `func_8005D418` this round (tools/rename.py). The
exact mirror of `Entity__AttachToParent` (clears `this->unk4C` under the same
`this->unk0C` gate, plus a base-ancestor `slot50` call and `methods->slot160`
-- now known to resolve to `Entity__Deactivate`, see the proposed field
names in `Entity__Deactivate.md`). Same caveat as AttachUnk4C: mechanics
established, the unk4C object's own purpose is not.

## Track 4 (2026-09-26, round 88, echo)

Renamed from `Entity__DetachUnk4C`: the occupant of +0x050, SceneNode's `detachFromParent`. Body: when attached (`parent`), deactivate, TodActor's detachFromParent, clear `grid`. Tier A: an override named for its slot (FINISHING-PLAN track 4 step 6).

The class (id 0x1F234, table `gEntityMethods`) is unified as `Entity` in `include/entity.h`: a TodActor subclass whose table and object expand `TODACTOR_SLOTS`/`TODACTOR_FIELDS`. Any source block above is the pre-unification spelling; the live body takes the inherited names (fields `parent`, `coord2`->`tx/ty/tz`, `tick`, `linkTarget`, `state` (was `moodState`), `lastOffsetValue`, `grid` (was `unk4C`), `ticker` (was `companion2`), `arg2` (was `soundCueChannel`), `parts`, `todPlaying`, `peer` (was `target`, cast to the `Unk94Obj` DreamSys view where its own slots are called); slots `reset`, `setDisplay`, `setLightMode`, `setTranslation`/`addTranslation`, `moveLocalZ/X/Y`, `moveLocalZOrFindLink`, `selectTickCallback`, `enableTickCallback`/`disableTickCallback`, `distanceToPeer`, `setTargetReached`, `updateActivationState`/`updateDeactivationState`), byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).
