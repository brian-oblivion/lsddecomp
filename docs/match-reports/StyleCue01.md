# StyleCue01 -- MATCHED (23/23 words)

> Renamed from `func_80055B10` on 2026-09-23 (tools/rename.py). Address 0x80055b10.

Unit: `class_3bb8c_r` (round 17 continuation). Slot occupant #2 of
`gStyleCueCallbacks`. Same shape as `StyleCue00` (see that report and the
file banner) with a two-way `kind` dispatch instead of four-way.

## Final source

```c
void StyleCue01(ParamObj *ctx, ParamObj *self) {
    s32 kind;

    self->unk10 = ComputeStyleCueFalloff(ctx);
    kind = self->unk4;
    if (kind == 0) {
        self->unk1C = 0x18;
        self->unk20 = -2;
    } else if (kind >= 0x401) {
        self->unk4 = -1;
    }
}
```

## Derivation

Direct transcription; matched first try once the shared `ParamObj`/
`ComputeStyleCueFalloff` groundwork (established from `StyleCue00`) was in
place.

### Proposed learning

None -- see `ComputeStyleCueFalloff`'s and `StyleCue07`'s reports.
