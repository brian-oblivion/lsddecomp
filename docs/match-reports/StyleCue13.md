# StyleCue13 -- MATCHED (17/17 words)

> Renamed from `func_80056238` on 2026-09-23 (tools/rename.py). Address 0x80056238.

Unit: `class_3bb8c_r` (round 17 continuation). Slot occupant #14 (the
LAST) of `gStyleCueCallbacks`. The simplest of the fourteen: a single
`kind == 0` check, no `else`.

## Final source

```c
void StyleCue13(StyleCueParam *ctx, StyleCueParam *self) {
    self->falloff = ComputeStyleCueFalloff(ctx);
    if (self->kind == 0) {
        self->unk1C = 0x18;
        self->unk20 = 0;
    }
}
```

## Derivation

Direct transcription, matched first try -- and confirms `gStyleCueCallbacks`'s
own slot table ends here (`asm/data/76DC8.data.s` lists exactly 14
function pointers, and this is the 14th).

### Proposed learning

None.

## Naming

**Tier B.** `StyleCue13` is row +0x038 of `gStyleCueCallbacks` (`tools/classtable.py 0x800874B0`, round 73). `TryStartStyleCue` (class_3bb8c_n.c) installs `gStyleCueCallbacks[sub->countSign]` as `SoundCueSet::callback` via `InitSoundCueSet` (code_179d8_e.c) -- the same per-tag sound-cue-callback slot `gEntityMoodHandlerTable`'s `MoodCueNN` occupants hold for `Entity` (`Entity__MoodCueNN` match reports). The `StyleCueNN` numbering follows table row order, same convention as `MoodCueNN`. Mechanics are established (a per-tag callback that reads `self->kind` and writes a handful of numeric fields, calling `ComputeStyleCueFalloff` first); which dream/style object or which field means what in the running game is not, so the specific `kind` branches and the numeric literals they write stay unnamed.
