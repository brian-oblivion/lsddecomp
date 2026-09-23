# StyleCue13 -- MATCHED (17/17 words)

> Renamed from `func_80056238` on 2026-09-23 (tools/rename.py). Address 0x80056238.

Unit: `class_3bb8c_r` (round 17 continuation). Slot occupant #14 (the
LAST) of `gStyleCueCallbacks`. The simplest of the fourteen: a single
`kind == 0` check, no `else`.

## Final source

```c
void StyleCue13(ParamObj *ctx, ParamObj *self) {
    self->unk10 = ComputeStyleCueFalloff(ctx);
    if (self->unk4 == 0) {
        self->unk1C = 0x18;
        self->unk20 = 0;
    }
}
```

## Derivation

Direct transcription, matched first try -- and confirms `gStyleCueCallbacks`'s
own slot table ends here (`asm/data/76DC8.data.s` lists exactly 14
function pointers, and this is the 14th).

### Proposed learning

None.
