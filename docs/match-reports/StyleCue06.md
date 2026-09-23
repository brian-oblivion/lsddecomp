# StyleCue06 -- MATCHED (23/23 words)

> Renamed from `func_80055E94` on 2026-09-23 (tools/rename.py). Address 0x80055e94.

Unit: `class_3bb8c_r` (round 17 continuation). Slot occupant #7 of
`gStyleCueCallbacks`. Same two-way discrete-`kind` shape as `StyleCue01`/
`StyleCue02`.

## Final source

```c
void StyleCue06(ParamObj *ctx, ParamObj *self) {
    s32 kind;

    self->unk10 = ComputeStyleCueFalloff(ctx);
    kind = self->unk4;
    if (kind == 0) {
        self->unk1C = 7;
        self->unk20 = 2;
    } else if (kind >= 0x1B) {
        self->unk4 = -1;
    }
}
```

## Derivation

Direct transcription, matched first try.

### Proposed learning

None -- see `ComputeStyleCueFalloff`'s and `StyleCue07`'s reports.
