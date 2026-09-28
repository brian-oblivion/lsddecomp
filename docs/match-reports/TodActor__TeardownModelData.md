# TodActor__TeardownModelData

> Renamed from `Class65650__TeardownModelData` on 2026-09-26 (tools/rename.py). Address 0x80065c2c.

> Renamed from `func_80065C2C` on 2026-09-24 (tools/rename.py). Address 0x80065c2c.

**Unit:** TodActor · **Size:** 12 words (0x30 bytes) · **Status:** MATCHED
(12/12 words, whole-image `./build-and-verify.sh` green)

## What it does

`TodActorMethods` slot `+0x0F8` (`slot_teardown5C`, called from the
destructor `TodActor__Finalize`). The teardown mirror of `TodActor__SetupModelData`'s setup:
if `self->unk5C` is set, tears it down via `TodActor__ReleaseModelData(self)`; otherwise
does nothing. Retail never explicitly sets `$v0` to a fixed value here (the
reload of `self->unk5C` for the guard already leaves it at 0 on the
not-taken path, and the call's own return overwrites it on the taken path),
consistent with the caller ignoring the return value — typed `void` here.

```c
void TodActor__TeardownModelData(TodActor *self)
{
    if (self->unk5C != NULL) {
        TodActor__ReleaseModelData(self);
    }
}
```

Matched on the direct translation, no reshaping.

### Proposed learning

None beyond `TodActor__SetupModelData.md`'s (the setup/teardown pair share the same
guard shape).

## Naming

Round 75 (charlie), track 3.

- `TodActor__TeardownModelData` (was `func_80065C2C`), tier A. Occupies +0x0F8 (called by the Destructor). Calls ReleaseModelData only if modelData is set.

## Track 4 (2026-09-25, round 85, alpha)

The class (id 0x234, table `gTodActorMethods`) is unified as `TodActor` in `include/TodActor.h`: an Actor subclass (its ctor chains to Actor's first) and Entity's base. Any source block above is the pre-unification spelling (the local `TodActorMethods` of `include/code_55dd4.h`, `linkCompanion`/`unlinkCompanion`, `companion2`, `Unk5CObj`/`Unk70ElemObj`); the live body in `src/code_55dd4.c` takes the unified types and the inherited slot and field names (`addChild`/`removeChild`, Actor's `ticker`, `Actor *` parts, `ModelData *` modelData), byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).
