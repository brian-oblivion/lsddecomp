# CdStreamObj__StartRead -- MATCHED (exact length, 57/57 words), round 82

> Renamed from `func_800473E4` on 2026-09-25 (tools/rename.py). Address 0x800473e4.

Round 82, runner delta (second session). Unit `src/code_3770c.c`. Fresh
ground, no prior attempt. Byte-exact on the first build; whole-image SHA1
green.

- **Where:** slot +0x050 of gCdStreamObjMethods (start streaming).
- **What:** if state is 1 (seeking) and active: read mode = 0x1C0 when
  `unk34 < 4`, else 0x140; store a non-zero arg2 to +0x40; clear +0x58;
  `StSetStream(0, startFrame, -1, 0, 0)`; mute; retry
  `CdControl(2 /*CdlSetloc*/, self->loc, 0)` then `CdRead2(mode)` until both
  succeed; demute; state = 2.
- **Levers:** the two-stage retry loop (`beqz` on CdControl and on CdRead2
  both branch back to the CdControl call) is a single
  `while (CdControl(...) == 0 || CdRead2(mode) == 0) {}`. The mode select is
  `mode = 0x140; if (unk34 < 4) mode = 0x1C0;` (the 0x140 lands in the
  branch delay slot).
- **Context:** local view gained `s32 unk34` (+0x34), `s32 unk40` (+0x40),
  `s32 unk58` (+0x58), replacing the pads there (sizes unchanged, object
  still 0x5C). Unit-local externs for `CdRead2` and `StSetStream` (Psy-Q
  LIBCD.H prototypes, `u_long` spelled `u32`).

## Naming

Kept `func_`. Not renamed this round (brief: no renames).

## Source

```c
void CdStreamObj__StartRead(CdStreamObj *self, u32 startFrame, s32 arg2) {
    u32 mode;

    if (self->unk2C == 1 && gActiveCdStreamObj == self) {
        mode = 0x140;
        if (self->unk34 < 4) {
            mode = 0x1C0;
        }
        if (arg2 != 0) {
            self->unk40 = arg2;
        }
        self->unk58 = 0;
        StSetStream(0, startFrame, -1, 0, 0);
        self->methods->mute(self);
        while (CdControl(2, self->loc, 0) == 0 || CdRead2(mode) == 0) {
        }
        self->methods->demute(self);
        self->unk2C = 2;
    }
}
```
