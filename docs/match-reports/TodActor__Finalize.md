# TodActor__Finalize

> Renamed from `Class65650__Finalize` on 2026-09-26 (tools/rename.py). Address 0x8006573c.

> Renamed from `TodActor__Destructor` on 2026-09-25 (tools/rename.py). Address 0x8006573c.

> Renamed from `func_8006573C` on 2026-09-24 (tools/rename.py). Address 0x8006573c.

**Unit:** code_55dd4 · **Size:** 21 words (0x54 bytes) · **Status:** MATCHED
(21/21 words, whole-image `./build-and-verify.sh` green)

## What it does

The destructor override for the class at `gTodActorMethods` (table slot `+0x00C`,
overriding the intermediate class `gActorMethods`'s own dtor at the same
offset). Calls this class's own teardown helper (`self->methods->slot0xF8`,
i.e. `TodActor__TeardownModelData`, the guarded teardown for the `+0x5C` sub-object — still
`INCLUDE_ASM` this round), then chains to the base class's dtor
(`GetActorMethods()->dtor(self)`), matching the "destructor calls its own
cleanup, then the base dtor through the base table's own slot" idiom from
`docs/research/class-framework.md`.

```c
void TodActor__Finalize(TodActor *self)
{
    self->methods->slot_teardown5C(self);
    GetActorMethods()->dtor(self);
}
```

No reshaping needed; matched on the direct translation.

### Proposed learning

None beyond what `TodActor__TodActor.md` and `New_TodActor.md`
already record for this class.

## Naming

Round 75 (charlie), track 3.

- `TodActor__Finalize` (was `func_8006573C`), tier A. Occupies +0x00C (overrides SceneNode__Finalize): teardownModelData, then the base dtor. Named like Entity__Finalize, the subclass's own +0x00C.

## Track 4 (2026-09-25, round 85, alpha)

Renamed from `TodActor__Destructor`. Override of +0x00C, BasicClass's `finalize` (Actor's occupant is SceneNode__Finalize, `classtable.py gTodActorMethods --vs gActorMethods`), named for its slot: teardownModelData (+0x0F8), then the base finalize through GetActorMethods(). The body does nothing a finalize does not, so the slot name stands. The class (id 0x234, table `gTodActorMethods`) is unified as `TodActor` in `include/TodActor.h`: an Actor subclass, Entity's base. Any source block above is the pre-unification spelling; the live body in `src/code_55dd4.c` takes the unified types and slot names, byte-identical.
