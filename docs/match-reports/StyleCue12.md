# StyleCue12 -- MATCHED (41/41 words)

> Renamed from `func_80056194` on 2026-09-23 (tools/rename.py). Address 0x80056194.

Unit: `class_3bb8c_r` (round 17 continuation). Slot occupant #13 of
`gStyleCueCallbacks`. Four-way discrete-`kind` dispatch (0, 4, 0x14, and a
threshold), reusing the cached `kind` local across all four arms with no
reloads (unlike `StyleCue05`, which needed the opposite treatment --
compare the two reports).

## Final source

```c
void StyleCue12(StyleCueParam *ctx, StyleCueParam *self) {
    s32 kind;

    self->falloff = ComputeStyleCueFalloff(ctx);
    kind = self->kind;
    if (kind == 0) {
        self->unk1C = 0x14;
        self->unk20 = -2;
        self->unk30 = 0x14;
        self->unk34 = -2;
    } else if (kind == 4) {
        self->unk30 = 0x14;
        self->unk34 = -2;
    } else if (kind == 0x14) {
        self->unk1C = 0x10;
        self->unk20 = -2;
        self->unk30 = 0x12;
        self->unk34 = -2;
    } else if (kind >= 0xC9) {
        self->kind = -1;
    }
}
```

## Derivation

Direct transcription of the four-way chain; matched first try. Confirms
that a cached `kind` local reproduces retail's own register reuse across
ALL four arms here (no intervening writes appear between the branch
TESTS themselves -- each arm's own field writes happen only after the
relevant comparison has already selected that arm, so nothing forces a
reload the way `StyleCue05`'s cross-arm arithmetic did).

### Proposed learning

None -- see `StyleCue05`'s report for the contrasting "must NOT
cache" case, and `StyleCue07`'s for a case needing a cached local
mid-arm.

## Naming

**Tier B.** `StyleCue12` is row +0x034 of `gStyleCueCallbacks` (`tools/classtable.py 0x800874B0`, round 73). `TryStartStyleCue` (class_3bb8c_n.c) installs `gStyleCueCallbacks[sub->countSign]` as `SoundCueSet::callback` via `InitSoundCueSet` (code_179d8_e.c) -- the same per-tag sound-cue-callback slot `gEntityMoodHandlerTable`'s `MoodCueNN` occupants hold for `Entity` (`Entity__MoodCueNN` match reports). The `StyleCueNN` numbering follows table row order, same convention as `MoodCueNN`. Mechanics are established (a per-tag callback that reads `self->kind` and writes a handful of numeric fields, calling `ComputeStyleCueFalloff` first); which dream/style object or which field means what in the running game is not, so the specific `kind` branches and the numeric literals they write stay unnamed.
