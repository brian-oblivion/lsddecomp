# CdStream__StartRead -- MATCHED (exact length, 57/57 words), round 82

> Renamed from `CdStreamObj__StartRead` on 2026-09-26 (tools/rename.py). Address 0x800473e4.

> Renamed from `func_800473E4` on 2026-09-25 (tools/rename.py). Address 0x800473e4.

Round 82, runner delta (second session). Unit `src/CdStream.c`. Fresh
ground, no prior attempt. Byte-exact on the first build; whole-image SHA1
green.

- **Where:** slot +0x050 of gCdStreamMethods (start streaming).
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

Tier A. `CdStream__StartRead` -- slot +0x050. Evidence: only fires from the seeking state; selects a read mode, calls `StSetStream`, mutes, retries `CdControl(CdlSetloc)`/`CdRead2` until both succeed, demutes, and transitions to the reading state -- the point where actual streamed reads begin.

## Source

```c
void CdStream__StartRead(CdStreamObj *self, u32 startFrame, s32 arg2) {
    u32 mode;

    if (self->unk2C == 1 && gActiveCdStream == self) {
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

## Track 4 (2026-09-26, round 87)

Class unified as `CdStream` (include/CdStream.h; table gCdStreamObjMethods -> gCdStreamMethods, type CdStreamObj -> CdStream, the Obj suffix dropped per FINISHING-PLAN track 4 step 2). The unit's local view is gone; slots +0x044 open, +0x050 startRead and +0x06C getNextFrame are typed from their occupants, and the object's +0x00C `seekLoc[0x18]` is the CdlFILE `file` (CdStreamFile) that CdSearchFile fills. Zero bytes changed.

Renamed from CdStreamObj__StartRead (tools/rename.py), the class rename only.

## Constants (track 7, round 99)

`0x1C0` / `0x140` are `CDSTREAM_MODE_2X` (`CdlModeStream | CdlModeSpeed |
CdlModeRT`) and `CDSTREAM_MODE_1X` (`CdlModeStream | CdlModeRT`), Sony's
bits from `<libcd.h>`; command 2 is `CdlSetloc`.
