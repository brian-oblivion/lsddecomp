# Class65650__TeardownParts

> Renamed from `func_80065DEC` on 2026-09-24 (tools/rename.py). Address 0x80065dec.

**Unit:** code_55dd4 · **Size:** 12 words (0x30 bytes) · **Status:** MATCHED
(12/12 words, whole-image `./build-and-verify.sh` green)

## What it does

`Class65650Methods` slot `+0x104` (`slot_teardown70`). The teardown mirror
of `Class65650__SetupParts`, matching `Class65650__TeardownModelData`'s shape for the `+0x70` array
pair, deferring to `Class65650__DestroyParts(self)`.

```c
void Class65650__TeardownParts(Class65650 *self)
{
    if (self->unk70 != NULL) {
        Class65650__DestroyParts(self);
    }
}
```

Matched on the direct translation, no reshaping.

### Proposed learning

None beyond `Class65650__SetupModelData.md`'s.

## Naming

Round 75 (charlie), track 3.

- `Class65650__TeardownParts` (was `func_80065DEC`), tier A. Occupies +0x104 (called by ReleaseModelData). Calls DestroyParts only if parts is set.

## Track 4 (2026-09-25, round 85, alpha)

The class (id 0x234, table `gClass65650Methods`) is unified as `Class65650` in `include/Class65650.h`: an Actor subclass (its ctor chains to Actor's first) and Entity's base. Any source block above is the pre-unification spelling (the local `Class65650Methods` of `include/code_55dd4.h`, `linkCompanion`/`unlinkCompanion`, `companion2`, `Unk5CObj`/`Unk70ElemObj`); the live body in `src/code_55dd4.c` takes the unified types and the inherited slot and field names (`addChild`/`removeChild`, Actor's `ticker`, `Actor *` parts, `ModelData *` modelData), byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).
