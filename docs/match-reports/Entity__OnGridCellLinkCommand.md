# Entity__OnGridCellLinkCommand

> Renamed from `Entity__OnClass86AA0LinkCommand` on 2026-09-26 (tools/rename.py). Address 0x8005d658.

> Renamed from `Entity__NotifyReset` on 2026-09-26 (tools/rename.py). Address 0x8005d658.

> Renamed from `func_8005D658` on 2026-09-19 (tools/rename.py). Address 0x8005d658.

**Unit:** Entity · **Size:** 31 words · **Status:** MATCHED (31/31 words, whole-image build verified byte-exact)

## What it does

Calls the shared "BasicClass" ancestor's `slotE0(this, a1, a2)`, then, if
`a2 == 4`, also calls this entity's own current `methods->slot160(this)` —
the same `slot160` that `Entity__DetachFromParent` and `Entity__UpdateDeactivationState` also dispatch
through.

## Final C

```c
void Entity__OnGridCellLinkCommand(Entity *this, s32 a1, s32 a2) {
    GetTodActorMethods()->slotE0(this, a1, a2);
    if (a2 == 4) {
        this->methods->slot160(this);
    }
}
```

## Attempt log

Matched on the first attempt.

## Proposed learning

None new.

## Naming

**Tier B.** Renamed from `func_8005D658` this round (tools/rename.py).
Forwards `(a1, a2)` to the base ancestor's `slotE0`, and on `a2 == 4` also
fires `EntityMethods::slot160` -- confirmed this round via
`tools/classtable.py` to resolve to `Entity__Deactivate` for a base Entity
(see `Entity__Deactivate.md`'s proposed field name for the slot itself).
"Reset" is deliberately weaker than "Deactivate": this function's own
evidence is only "forwards an event code, and on code 4 also deactivates",
not what the forwarded event fundamentally represents.

## Track 4 (2026-09-26, round 88, echo)

Renamed from `Entity__NotifyReset`: the occupant of +0x0E0, Actor's `onGridCellLinkCommand`. Body: chain TodActor's (Actor's) occupant, then deactivate on event 4. "Reset" was not in the body. (Entity__NotifyLinkStage, the +0x0DC override, keeps its name: its extra -- notify parents with a link-stage event on event 4 -- is what the name says.) Tier A: an override named for its slot (FINISHING-PLAN track 4 step 6).

The class (id 0x1F234, table `gEntityMethods`) is unified as `Entity` in `include/entity.h`: a TodActor subclass whose table and object expand `TODACTOR_SLOTS`/`TODACTOR_FIELDS`. Any source block above is the pre-unification spelling; the live body takes the inherited names (fields `parent`, `coord2`->`tx/ty/tz`, `tick`, `linkTarget`, `state` (was `moodState`), `lastOffsetValue`, `grid` (was `unk4C`), `ticker` (was `companion2`), `arg2` (was `soundCueChannel`), `parts`, `todPlaying`, `peer` (was `target`, cast to the `Unk94Obj` DreamSys view where its own slots are called); slots `reset`, `setDisplay`, `setLightMode`, `setTranslation`/`addTranslation`, `moveLocalZ/X/Y`, `moveLocalZOrFindLink`, `selectTickCallback`, `enableTickCallback`/`disableTickCallback`, `distanceToPeer`, `setTargetReached`, `updateActivationState`/`updateDeactivationState`), byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).

## Polish (round 96, bravo, track 7)

- Step 3: parameters a1, a2 -> sender, event (entity.h's prototype names, TodActor's link-command signature).

## Track 10 (2026-09-28, round 104, echo)

The link-event literals are spelled with their enums: 4 is `SCENENODE_EVENT_LINKED`, 2 and 3 `SCENENODE_EVENT_HULL_FIRST`/`HULL_LAST` (include/scene_node.h), 5 to 8 `ACTOR_EVENT_UNSWEPT`..`ACTOR_EVENT_MOVED_Y` (include/actor.h). Every function tested here receives the event through a DispatchLinkCommand/onActorLinkCommand/onGridCellLinkCommand chain from SceneNode's link protocol, so the numbers are that enum's. Byte-identical (`event < 9` spelled `event <= ACTOR_EVENT_MOVED_Y` compiles the same).
