# StyleCue09 -- MATCHED (27/27 words)

> Renamed from `func_80055FE8` on 2026-09-23 (tools/rename.py). Address 0x80055fe8.

Unit: `class_3bb8c_r` (round 17 continuation). Slot occupant #10 of
`gStyleCueCallbacks`. A single `% 20` divisibility gate, same shape as
`StyleCue08` with fewer field writes.

## Final source

```c
void StyleCue09(ParamObj *ctx, ParamObj *self) {
    self->unk10 = ComputeStyleCueFalloff(ctx);
    if (self->unk4 % 20 == 0) {
        self->unk1C = 9;
        self->unk20 = -2;
    }
}
```

## Derivation

Direct transcription, matched first try.

### Proposed learning

None -- see `StyleCue03`'s report.
