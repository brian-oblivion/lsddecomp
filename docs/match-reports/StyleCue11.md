# StyleCue11 -- MATCHED (44/44 words)

> Renamed from `func_800560E4` on 2026-09-23 (tools/rename.py). Address 0x800560e4.

Unit: `class_3bb8c_r` (round 17 continuation). Slot occupant #12 of
`gStyleCueCallbacks`. Chains to `StyleCue10` (a PLAIN CALL by symbol, not
through `ComputeStyleCueFalloff` again) then dispatches on `self->kind % 70`.

## Final source

```c
void StyleCue11(StyleCueParam *ctx, StyleCueParam *self) {
    s32 rem;

    StyleCue10(ctx, self);
    rem = self->kind % 70;
    if (rem == 50) {
        self->unk44 = 0x14;
        self->unk48 = 1;
    } else if ((u32)(rem - 54) < 5) {
        self->unk44 = 0xD;
        self->unk48 = 1;
    } else if (rem == 61) {
        self->unk44 = 9;
        self->unk48 = -1;
    }
}
```

## Derivation

The FIRST instruction is `jal StyleCue10`, not `jal ComputeStyleCueFalloff` --
this function does not call the shared helper itself, it delegates
entirely to its sibling slot occupant (which internally calls
`ComputeStyleCueFalloff` and sets `self->falloff` on its own). `self->kind % 70` uses
magic `0xEA0EA0EB` with the "ADD variant" (`mfhi` result added to the
dividend before the final shift, rather than subtracted, matching the
same `addu`-before-`sra` pattern already seen for divisor 7 in
`StyleCue04`) -- recombination confirms N=70 (`sll3,addu,sll2,subu,sll1`
= `q*9, +q(=10), *4(=40)-q... ` reduces to `q*70`; verified directly
against the disassembly rather than by formula alone, per
`StyleCue03`'s caution). The `(u32)(rem-54) < 5` arm is the same
unsigned-range idiom as `StyleCue05`'s report, here testing
`rem` in `[54, 58]`.

### Proposed learning

None -- confirms the unsigned-range idiom and the "ADD variant" magic
division shape already documented elsewhere in this unit.

## Naming

**Tier B.** `StyleCue11` is row +0x030 of `gStyleCueCallbacks` (`tools/classtable.py 0x800874B0`, round 73). `TryStartStyleCue` (class_3bb8c_n.c) installs `gStyleCueCallbacks[sub->countSign]` as `SoundCueSet::callback` via `InitSoundCueSet` (code_179d8_e.c) -- the same per-tag sound-cue-callback slot `gEntityMoodHandlerTable`'s `MoodCueNN` occupants hold for `Entity` (`Entity__MoodCueNN` match reports). The `StyleCueNN` numbering follows table row order, same convention as `MoodCueNN`. Mechanics are established (a per-tag callback that reads `self->kind` and writes a handful of numeric fields, calling `ComputeStyleCueFalloff` first); which dream/style object or which field means what in the running game is not, so the specific `kind` branches and the numeric literals they write stay unnamed.
