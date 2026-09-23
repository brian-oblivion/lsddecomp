# StyleCue08 -- MATCHED (29/29 words)

> Renamed from `func_80055F74` on 2026-09-23 (tools/rename.py). Address 0x80055f74.

Unit: `class_3bb8c_r` (round 17 continuation). Slot occupant #9 of
`gStyleCueCallbacks`. A single `% 20` divisibility gate.

## Final source

```c
void StyleCue08(ParamObj *ctx, ParamObj *self) {
    self->unk10 = ComputeStyleCueFalloff(ctx);
    if (self->unk4 % 20 == 0) {
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
