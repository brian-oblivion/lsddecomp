# Class65650__StopTod

> Renamed from `func_800662B4` on 2026-09-24 (tools/rename.py). Address 0x800662b4.

**Unit:** code_55dd4 · **Size:** 2 words (0x8 bytes) · **Status:** MATCHED
(2/2 words, whole-image `./build-and-verify.sh` green)

## What it does

Sets `self->unk90` (`+0x90`) to 0. `$v0` is never touched, so the return
value is unused (`void`). The "set to 1 and return 1" sibling is
`Class65650__PlayTod`.

```c
void Class65650__StopTod(Class65650 *self)
{
    self->unk90 = 0;
}
```

### Proposed learning

None; plain single-instruction setter shape.

## Naming

Round 75 (charlie), track 3.

- `Class65650__StopTod` (was `func_800662B4`), tier A. Occupies +0x130; clears todPlaying. Called by InitDefaults and Entity__StopSoundCue.

## Track 4 (2026-09-25, round 85, alpha)

The class (id 0x234, table `gClass65650Methods`) is unified as `Class65650` in `include/Class65650.h`: an Actor subclass (its ctor chains to Actor's first) and Entity's base. Any source block above is the pre-unification spelling (the local `Class65650Methods` of `include/code_55dd4.h`, `linkCompanion`/`unlinkCompanion`, `companion2`, `Unk5CObj`/`Unk70ElemObj`); the live body in `src/code_55dd4.c` takes the unified types and the inherited slot and field names (`addChild`/`removeChild`, Actor's `ticker`, `Actor *` parts, `ModelData *` modelData), byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).
