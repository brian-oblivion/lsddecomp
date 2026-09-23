# StyleCue08 -- MATCHED (29/29 words)

> Renamed from `func_80055F74` on 2026-09-23 (tools/rename.py). Address 0x80055f74.

Unit: `class_3bb8c_r` (round 17 continuation). Slot occupant #9 of
`gStyleCueCallbacks`. A single `% 20` divisibility gate.

## Final source

```c
void StyleCue08(StyleCueParam *ctx, StyleCueParam *self) {
    self->falloff = ComputeStyleCueFalloff(ctx);
    if (self->kind % 20 == 0) {
        self->unk1C = 9;
        self->unk20 = 0;
        self->unk24 = 0x40;
        self->unk28 = 0x40;
    }
}
```

## Derivation

Same `%20` magic+shift as `StyleCue03`'s own first block (already
confirmed there: `sll,addu,sll` recombination = `q*20`). Direct
transcription, matched first try -- this function is the simplest of the
divisibility-gated siblings (one condition, four flat writes).

### Proposed learning

None -- see `StyleCue03`'s report for the divisor-derivation caution.

## Naming

**Tier B.** `StyleCue08` is row +0x024 of `gStyleCueCallbacks` (`tools/classtable.py 0x800874B0`, round 73). `TryStartStyleCue` (class_3bb8c_n.c) installs `gStyleCueCallbacks[sub->countSign]` as `SoundCueSet::callback` via `InitSoundCueSet` (code_179d8_e.c) -- the same per-tag sound-cue-callback slot `gEntityMoodHandlerTable`'s `MoodCueNN` occupants hold for `Entity` (`Entity__MoodCueNN` match reports). The `StyleCueNN` numbering follows table row order, same convention as `MoodCueNN`. Mechanics are established (a per-tag callback that reads `self->kind` and writes a handful of numeric fields, calling `ComputeStyleCueFalloff` first); which dream/style object or which field means what in the running game is not, so the specific `kind` branches and the numeric literals they write stay unnamed.
