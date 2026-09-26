# Class65650__TickCallbackB -- round 75: matched (splat-generated, empty body)

Unit `code_55dd4`, 0x800661C4. The body is `jr $ra; nop`, which splat emits as C itself; no matching work was done. This report exists for the naming record.

## Naming

Round 75 (charlie), track 3.

- `Class65650__TickCallbackB` (was `func_800661C4`), tier A. Occupies +0x11C, the 'B' tick callback; empty body (`jr $ra`). Entity overrides this slot with Entity__TickSoundCue and selects 'B' in Entity__Reset.

## Track 4 (2026-09-25, round 85, alpha)

The class (id 0x234, table `gClass65650Methods`) is unified as `Class65650` in `include/Class65650.h`: an Actor subclass (its ctor chains to Actor's first) and Entity's base. Any source block above is the pre-unification spelling (the local `Class65650Methods` of `include/code_55dd4.h`, `linkCompanion`/`unlinkCompanion`, `companion2`, `Unk5CObj`/`Unk70ElemObj`); the live body in `src/code_55dd4.c` takes the unified types and the inherited slot and field names (`addChild`/`removeChild`, Actor's `ticker`, `Actor *` parts, `ModelData *` modelData), byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).
