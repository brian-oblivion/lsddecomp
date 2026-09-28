# TodActor__TickMoveZ

> Renamed from `TodActor__TickCallbackA` on 2026-09-28 (tools/rename.py). Address 0x80066150.

> Renamed from `Class65650__TickCallbackA` on 2026-09-26 (tools/rename.py). Address 0x80066150.

> Renamed from `func_80066150` on 2026-09-24 (tools/rename.py). Address 0x80066150.

**Unit:** TodActor · **Size:** 29 words (0x74 bytes) · **Status:** MATCHED
(29/29 words, whole-image `./build-and-verify.sh` green)

## What it does

`TodActorMethods` slot `+0x118`. Unconditionally calls the class's own
vtable slot `+0x0C4` with two literal arguments (`-0x1E`, `0`), then, only
if `self->unk64 == 1` **and** `self->unk68` is set, calls `unk68`'s own
vtable slot `+0x088` with the literal `6`.

```c
void TodActor__TickMoveZ(TodActor *self)
{
    self->methods->slotC4(self, -0x1E, 0);
    if (self->unk64 == 1 && self->unk68 != NULL) {
        self->unk68->methods->slot88(self->unk68, 6);
    }
}
```

The `&&` reproduces retail's two chained `beqz`/`bne` guards exactly: the
first false short-circuits past the second check entirely, matching a
single combined condition rather than nested `if`s (which would still be
semantically equivalent but is not what was tried first — the combined form
matched immediately).

Corrects `self->unk68`'s type from the generic `void *` the first pass gave
it to `Unk68Obj *` (a new minimal type in `src/world/tod_actor.c`, typed
only at its `+0x088` slot, the only one this unit calls). Adds `slotC4`
(`+0x0C4`) to `TodActorMethods`.

Matched on the direct translation, no reshaping.

### Proposed learning

Two independent field-guard branches, the first `bne`/`beqz`-style skipping
straight past the second on failure, read naturally as a single `&&`
condition rather than nested `if`s — worth trying `&&` first on this shape
before nesting.

## Naming

Round 75 (charlie), track 3.

- `TodActor__TickMoveZ` (was `func_80066150`), tier B. Occupies +0x118, the callback SelectTickCallback installs for 'A' (InitDefaults' default). Calls slotC4 (Actor__MoveLocalZ) with (-0x1E, 0), and if unk64 == 1 and mainPart is set, mainPart->slot88(6) (Actor__NotifyMove). Neither callee has a purpose name yet, hence B.

## Track 4 (2026-09-25, round 85, alpha)

The class (id 0x234, table `gTodActorMethods`) is unified as `TodActor` in `include/tod_actor.h`: an Actor subclass (its ctor chains to Actor's first) and Entity's base. Any source block above is the pre-unification spelling (the local `TodActorMethods` of `include/code_55dd4.h`, `linkCompanion`/`unlinkCompanion`, `companion2`, `Unk5CObj`/`Unk70ElemObj`); the live body in `src/code_55dd4.c` takes the unified types and the inherited slot and field names (`addChild`/`removeChild`, Actor's `ticker`, `Actor *` parts, `ModelData *` modelData), byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).

## Track 7 (round 99, bravo)

`-0x1E` is `TODACTOR_STEP_Z` (-30), tier B: the distance moved along local Z
each tick while the callback is enabled; what it is for is not established.
