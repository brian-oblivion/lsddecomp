# Class65650__EnableTickCallback

> Renamed from `func_8006613C` on 2026-09-24 (tools/rename.py). Address 0x8006613c.

**Unit:** code_55dd4 · **Size:** 3 words (0xC bytes) · **Status:** MATCHED
(3/3 words, whole-image `./build-and-verify.sh` green)

## What it does

Sets `self->unk8C` (`+0x8C`) to 1 and returns 1 — retail computes the literal
once into `$v0` and reuses that same register for both the store and the
return, so the assignment-as-expression spelling reproduces it exactly with a
single `ori`.

```c
s32 Class65650__EnableTickCallback(Class65650 *self)
{
    return self->unk8C = 1;
}
```

Paired with `Class65650__DisableTickCallback` (the same field's "set to 0" sibling) and
`Class65650__PlayTod`/`Class65650__StopTod` (the same pair for `+0x90`).

### Proposed learning

For a "set field to literal N and return N" shape, write
`return self->field = N;` rather than two separate statements — the single
assignment-expression reuses the same register GCC computes for the literal,
matching retail's one-`ori` codegen. Two statements risk (though did not
here) a second, redundant load of the same literal.

## Naming

Round 75 (charlie), track 3.

- `Class65650__EnableTickCallback` (was `func_8006613C`), tier A. Occupies +0x110; sets tickCallbackEnabled (+0x8C) = 1 and returns it. Tick calls tickCallback only while it is set. Entity__StartSoundCue calls this slot.

## Track 4 (2026-09-25, round 85, alpha)

The class (id 0x234, table `gClass65650Methods`) is unified as `Class65650` in `include/Class65650.h`: an Actor subclass (its ctor chains to Actor's first) and Entity's base. Any source block above is the pre-unification spelling (the local `Class65650Methods` of `include/code_55dd4.h`, `linkCompanion`/`unlinkCompanion`, `companion2`, `Unk5CObj`/`Unk70ElemObj`); the live body in `src/code_55dd4.c` takes the unified types and the inherited slot and field names (`addChild`/`removeChild`, Actor's `ticker`, `Actor *` parts, `ModelData *` modelData), byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).
