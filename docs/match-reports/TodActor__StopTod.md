# TodActor__StopTod

> Renamed from `Class65650__StopTod` on 2026-09-26 (tools/rename.py). Address 0x800662b4.

> Renamed from `func_800662B4` on 2026-09-24 (tools/rename.py). Address 0x800662b4.

**Unit:** code_55dd4 · **Size:** 2 words (0x8 bytes) · **Status:** MATCHED
(2/2 words, whole-image `./build-and-verify.sh` green)

## What it does

Sets `self->unk90` (`+0x90`) to 0. `$v0` is never touched, so the return
value is unused (`void`). The "set to 1 and return 1" sibling is
`TodActor__PlayTod`.

```c
void TodActor__StopTod(TodActor *self)
{
    self->unk90 = 0;
}
```

### Proposed learning

None; plain single-instruction setter shape.

## Naming

Round 75 (charlie), track 3.

- `TodActor__StopTod` (was `func_800662B4`), tier A. Occupies +0x130; clears todPlaying. Called by InitDefaults and Entity__StopSoundCue.

## Track 4 (2026-09-25, round 85, alpha)

The class (id 0x234, table `gTodActorMethods`) is unified as `TodActor` in `include/TodActor.h`: an Actor subclass (its ctor chains to Actor's first) and Entity's base. Any source block above is the pre-unification spelling (the local `TodActorMethods` of `include/code_55dd4.h`, `linkCompanion`/`unlinkCompanion`, `companion2`, `Unk5CObj`/`Unk70ElemObj`); the live body in `src/code_55dd4.c` takes the unified types and the inherited slot and field names (`addChild`/`removeChild`, Actor's `ticker`, `Actor *` parts, `ModelData *` modelData), byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).
