# StyleCue12 -- MATCHED (41/41 words)

> Renamed from `func_80056194` on 2026-09-23 (tools/rename.py). Address 0x80056194.

Unit: `class_3bb8c_r` (round 17 continuation). Slot occupant #13 of
`gStyleCueCallbacks`. Four-way discrete-`kind` dispatch (0, 4, 0x14, and a
threshold), reusing the cached `kind` local across all four arms with no
reloads (unlike `StyleCue05`, which needed the opposite treatment --
compare the two reports).

## Final source

```c
void StyleCue12(ParamObj *ctx, ParamObj *self) {
    s32 kind;

    self->unk10 = ComputeStyleCueFalloff(ctx);
    kind = self->unk4;
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
        self->unk4 = -1;
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
