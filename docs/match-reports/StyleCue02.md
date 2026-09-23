# StyleCue02 -- MATCHED (23/23 words)

> Renamed from `func_80055B6C` on 2026-09-23 (tools/rename.py). Address 0x80055b6c.

Unit: `class_3bb8c_r` (round 17 continuation). Slot occupant #3 of
`gStyleCueCallbacks`. Same shape as `StyleCue01`.

## Final source

```c
void StyleCue02(ParamObj *ctx, ParamObj *self) {
    s32 kind;

    self->unk10 = ComputeStyleCueFalloff(ctx);
    kind = self->unk4;
    if (kind == 0) {
        self->unk1C = 0xC;
        self->unk20 = 2;
    } else if (kind >= 5) {
        self->unk4 = -1;
    }
}
```

## Derivation

Direct transcription, matched first try.

### Proposed learning

None -- see `ComputeStyleCueFalloff`'s and `StyleCue07`'s reports.
