# StyleCue06 -- MATCHED (23/23 words)

> Renamed from `func_80055E94` on 2026-09-23 (tools/rename.py). Address 0x80055e94.

Unit: `class_3bb8c_r` (round 17 continuation). Slot occupant #7 of
`gStyleCueCallbacks`. Same two-way discrete-`kind` shape as `StyleCue01`/
`StyleCue02`.

## Final source

```c
void StyleCue06(StyleCueParam *ctx, StyleCueParam *self) {
    s32 kind;

    self->falloff = ComputeStyleCueFalloff(ctx);
    kind = self->kind;
    if (kind == 0) {
        self->unk1C = 7;
        self->unk20 = 2;
    } else if (kind >= 0x1B) {
        self->kind = -1;
    }
}
```

## Derivation

Direct transcription, matched first try.

### Proposed learning

None -- see `ComputeStyleCueFalloff`'s and `StyleCue07`'s reports.

## Naming

**Tier B.** `StyleCue06` is row +0x01C of `gStyleCueCallbacks` (`tools/classtable.py 0x800874B0`, round 73). `TryStartStyleCue` (class_3bb8c_n.c) installs `gStyleCueCallbacks[sub->countSign]` as `SoundCueSet::callback` via `InitSoundCueSet` (code_179d8_e.c) -- the same per-tag sound-cue-callback slot `gEntityMoodHandlerTable`'s `MoodCueNN` occupants hold for `Entity` (`Entity__MoodCueNN` match reports). The `StyleCueNN` numbering follows table row order, same convention as `MoodCueNN`. Mechanics are established (a per-tag callback that reads `self->kind` and writes a handful of numeric fields, calling `ComputeStyleCueFalloff` first); which dream/style object or which field means what in the running game is not, so the specific `kind` branches and the numeric literals they write stay unnamed.
