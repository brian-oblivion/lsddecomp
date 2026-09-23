# StyleCue09 -- MATCHED (27/27 words)

> Renamed from `func_80055FE8` on 2026-09-23 (tools/rename.py). Address 0x80055fe8.

Unit: `class_3bb8c_r` (round 17 continuation). Slot occupant #10 of
`gStyleCueCallbacks`. A single `% 20` divisibility gate, same shape as
`StyleCue08` with fewer field writes.

## Final source

```c
void StyleCue09(StyleCueParam *ctx, StyleCueParam *self) {
    self->falloff = ComputeStyleCueFalloff(ctx);
    if (self->kind % 20 == 0) {
        self->unk1C = 9;
        self->unk20 = -2;
    }
}
```

## Derivation

Direct transcription, matched first try.

### Proposed learning

None -- see `StyleCue03`'s report.

## Naming

**Tier B.** `StyleCue09` is row +0x028 of `gStyleCueCallbacks` (`tools/classtable.py 0x800874B0`, round 73). `TryStartStyleCue` (class_3bb8c_n.c) installs `gStyleCueCallbacks[sub->countSign]` as `SoundCueSet::callback` via `InitSoundCueSet` (code_179d8_e.c) -- the same per-tag sound-cue-callback slot `gEntityMoodHandlerTable`'s `MoodCueNN` occupants hold for `Entity` (`Entity__MoodCueNN` match reports). The `StyleCueNN` numbering follows table row order, same convention as `MoodCueNN`. Mechanics are established (a per-tag callback that reads `self->kind` and writes a handful of numeric fields, calling `ComputeStyleCueFalloff` first); which dream/style object or which field means what in the running game is not, so the specific `kind` branches and the numeric literals they write stay unnamed.
