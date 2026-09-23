# StyleCue10 -- MATCHED (36/36 words)

> Renamed from `func_80056054` on 2026-09-23 (tools/rename.py). Address 0x80056054.

Unit: `class_3bb8c_r` (round 17 continuation). Slot occupant #11 of
`gStyleCueCallbacks`. Dispatches on the ACTUAL REMAINDER of `self->kind % 20`
(not just divisibility) -- the first sibling in this family to keep the
remainder value itself rather than only testing it against zero.

## Final source

```c
void StyleCue10(StyleCueParam *ctx, StyleCueParam *self) {
    s32 rem;

    self->falloff = ComputeStyleCueFalloff(ctx);
    rem = self->kind % 20;
    if (rem == 1) {
        self->unk1C = 9;
        self->unk20 = -2;
    } else if (rem == 16) {
        self->unk30 = 9;
        self->unk34 = -2;
    }
}
```

## Derivation

Retail computes `q = kind/20` (same `%20` magic as `StyleCue03`) and
then `rem = kind - q*20` explicitly (the SUBTRACTION result itself is
kept and compared against 1 and 16, rather than the earlier siblings'
pattern of comparing the dividend against `q*N` directly). Writing
`self->kind % 20` and caching it in a `rem` local reproduces this --
GCC's ordinary `%` codegen computes exactly this quotient-then-subtract
sequence, and caching it avoids two redundant re-derivations for the two
comparisons.

### Proposed learning

None -- a natural extension of the `%20` idiom already confirmed in
`StyleCue03`'s report, this time keeping the remainder value itself.

## Naming

**Tier B.** `StyleCue10` is row +0x02C of `gStyleCueCallbacks` (`tools/classtable.py 0x800874B0`, round 73). `TryStartStyleCue` (class_3bb8c_n.c) installs `gStyleCueCallbacks[sub->countSign]` as `SoundCueSet::callback` via `InitSoundCueSet` (code_179d8_e.c) -- the same per-tag sound-cue-callback slot `gEntityMoodHandlerTable`'s `MoodCueNN` occupants hold for `Entity` (`Entity__MoodCueNN` match reports). The `StyleCueNN` numbering follows table row order, same convention as `MoodCueNN`. Mechanics are established (a per-tag callback that reads `self->kind` and writes a handful of numeric fields, calling `ComputeStyleCueFalloff` first); which dream/style object or which field means what in the running game is not, so the specific `kind` branches and the numeric literals they write stay unnamed.
