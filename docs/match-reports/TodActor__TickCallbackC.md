# TodActor__TickCallbackC -- round 75: matched (splat-generated, empty body)

> Renamed from `Class65650__TickCallbackC` on 2026-09-26 (tools/rename.py). Address 0x800661cc.

Unit `TodActor`, 0x800661CC. The body is `jr $ra; nop`, which splat emits as C itself; no matching work was done. This report exists for the naming record.

## Naming

Round 75 (charlie), track 3.

- `TodActor__TickCallbackC` (was `func_800661CC`), tier A. Occupies +0x120, the 'C' tick callback; empty body. No override found in gEntityMethods.

## Track 4 (2026-09-25, round 85, alpha)

The class (id 0x234, table `gTodActorMethods`) is unified as `TodActor` in `include/TodActor.h`: an Actor subclass (its ctor chains to Actor's first) and Entity's base. Any source block above is the pre-unification spelling (the local `TodActorMethods` of `include/code_55dd4.h`, `linkCompanion`/`unlinkCompanion`, `companion2`, `Unk5CObj`/`Unk70ElemObj`); the live body in `src/code_55dd4.c` takes the unified types and the inherited slot and field names (`addChild`/`removeChild`, Actor's `ticker`, `Actor *` parts, `ModelData *` modelData), byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).
