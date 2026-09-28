# TodActor__PlayTod

> Renamed from `Class65650__PlayTod` on 2026-09-26 (tools/rename.py). Address 0x800662a8.

> Renamed from `func_800662A8` on 2026-09-24 (tools/rename.py). Address 0x800662a8.

**Unit:** TodActor · **Size:** 3 words (0xC bytes) · **Status:** MATCHED
(3/3 words, whole-image `./build-and-verify.sh` green)

## What it does

Sets `self->unk90` (`+0x90`) to 1 and returns 1 — the same
"assignment-expression reuses the literal register" shape as
`TodActor__EnableTickCallback`, for the `+0x90` field instead of `+0x8C`.

```c
s32 TodActor__PlayTod(TodActor *self)
{
    return self->unk90 = 1;
}
```

### Proposed learning

None beyond `TodActor__EnableTickCallback.md`'s (same shape, different field).

## Naming

Round 75 (charlie), track 3.

- `TodActor__PlayTod` (was `func_800662A8`), tier A. Occupies +0x12C; sets todPlaying (+0x90) = 1 and returns it. Tick advances frames only while it is set. Called from Entity code (Entity__MoodCue42).

## Track 4 (2026-09-25, round 85, alpha)

The class (id 0x234, table `gTodActorMethods`) is unified as `TodActor` in `include/TodActor.h`: an Actor subclass (its ctor chains to Actor's first) and Entity's base. Any source block above is the pre-unification spelling (the local `TodActorMethods` of `include/code_55dd4.h`, `linkCompanion`/`unlinkCompanion`, `companion2`, `Unk5CObj`/`Unk70ElemObj`); the live body in `src/code_55dd4.c` takes the unified types and the inherited slot and field names (`addChild`/`removeChild`, Actor's `ticker`, `Actor *` parts, `ModelData *` modelData), byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).
