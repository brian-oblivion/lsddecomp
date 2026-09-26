# TodActor__SetUnk64

> Renamed from `Class65650__SetUnk64` on 2026-09-26 (tools/rename.py). Address 0x80065bf4.

> Renamed from `func_80065BF4` on 2026-09-24 (tools/rename.py). Address 0x80065bf4.

**Unit:** code_55dd4 · **Size:** 2 words (0x8 bytes) · **Status:** MATCHED
(2/2 words, whole-image `./build-and-verify.sh` green)

## What it does

A trivial setter: stores its second argument into `self->unk64` (`+0x64` of
the `TodActor` object). Retail never sets `$v0` in this function, so its
return value is treated as unused (`void`).

```c
void TodActor__SetUnk64(TodActor *self, s32 value)
{
    self->unk64 = value;
}
```

### Proposed learning

None; this is the plain single-instruction setter shape.

## Naming

Round 75 (charlie), track 3.

- `TodActor__SetUnk64` (was `func_80065BF4`), tier B. Occupies +0x0F0 (first slot of this class's own range); a one-line setter of +0x64. InitDefaults sets it to 1 and TickCallbackA acts only while it is 1; what the value means is not known, so the field keeps its offset name.

## Track 4 (2026-09-25, round 85, alpha)

The class (id 0x234, table `gTodActorMethods`) is unified as `TodActor` in `include/TodActor.h`: an Actor subclass (its ctor chains to Actor's first) and Entity's base. Any source block above is the pre-unification spelling (the local `TodActorMethods` of `include/code_55dd4.h`, `linkCompanion`/`unlinkCompanion`, `companion2`, `Unk5CObj`/`Unk70ElemObj`); the live body in `src/code_55dd4.c` takes the unified types and the inherited slot and field names (`addChild`/`removeChild`, Actor's `ticker`, `Actor *` parts, `ModelData *` modelData), byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).
