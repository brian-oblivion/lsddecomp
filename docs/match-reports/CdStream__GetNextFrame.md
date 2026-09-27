# CdStream__GetNextFrame -- MATCHED (exact length, 71/71 words), round 82

> Renamed from `CdStreamObj__GetNextFrame` on 2026-09-26 (tools/rename.py). Address 0x80047694.

> Renamed from `func_80047694` on 2026-09-25 (tools/rename.py). Address 0x80047694.

Round 82, runner delta (second session). Unit `src/code_3770c.c`. Fresh
ground, no prior attempt. Byte-exact on the first build; whole-image SHA1
green.

- **Where:** slot +0x06C of gCdStreamMethods (get next stream frame).
- **What:** retry `StGetNext(addr, &header)` up to `tries` times (a
  negative `tries` means 0x800000). On timeout, `freeRing(self, (u32 *)addr)`
  and return 0. Otherwise `*frame = header[2]` (StHEADER's frame count).
  When `unk40 > 0`, a frame at or past `unk40`, or before the last seen frame
  `unk58`, ends the stream: zero `*frame` if it went backwards, release the
  sector (`CdStream__ReleaseFrame`), finish (`CdStream__OnStreamEnd`), return -1. Otherwise
  record `unk58 = frame`. Normal path: release and return 1.
- **Levers:** none. The `lui s0,0x80` / `addiu s0,s0,-1` pair splat shows as
  `%hi/%lo(D_7FFFFF)` is just `tries = 0x800000` plus the loop's `--tries`
  sitting in the beqz delay slot. It is not a symbol.
- **Declaration change (unit-local):** `CdStream__ReleaseFrame` gained an unused third
  parameter `u32 frame`. Retail calls it with `(self, *addr, *frame)`, and
  its body leaves a1/a2 untouched across the `cb48(cbArg)` call. Its bytes
  are unchanged (24/24, image green). Prototypes for `CdStream__ReleaseFrame` and
  `CdStream__OnStreamEnd` were added near the top of the unit. `StGetNext` extern
  added (Psy-Q prototype, `u_long` spelled `u32`).

## Naming

Tier A. `CdStream__GetNextFrame` -- slot +0x06C. Evidence: retries `StGetNext` for the next decoded sector, reads its frame count, ends the stream past `totalFrames` or on a backwards jump, otherwise records `lastFrame` and returns the frame -- exactly "get the next stream frame".

## Source

```c
s32 CdStream__GetNextFrame(CdStreamObj *self, u32 **addr, u32 *frame, s32 tries) {
    u32 *header;
    u32 n;

    if (tries < 0) {
        tries = 0x800000;
    }
    while (StGetNext(addr, &header) != 0) {
        if (--tries < 0) {
            self->methods->freeRing(self, (u32 *)addr);
            return 0;
        }
    }
    n = header[2];
    *frame = n;
    if (self->unk40 > 0) {
        if (n >= self->unk40 || n < self->unk58) {
            if (n < self->unk58) {
                *frame = 0;
            }
            CdStream__ReleaseFrame(self, *addr, *frame);
            CdStream__OnStreamEnd(self);
            return -1;
        }
        self->unk58 = n;
    }
    CdStream__ReleaseFrame(self, *addr, *frame);
    return 1;
}
```

### Proposed learning

`CdStream__ReleaseFrame` shows how an unused parameter looks in the bytes: a
callback call that sets only a0 while a1/a2 still hold the caller's
arguments. Its matched C had two parameters. A caller passing three was
what exposed it.

## Track 4 (2026-09-26, round 87)

Class unified as `CdStream` (include/CdStream.h; table gCdStreamObjMethods -> gCdStreamMethods, type CdStreamObj -> CdStream, the Obj suffix dropped per FINISHING-PLAN track 4 step 2). The unit's local view is gone; slots +0x044 open, +0x050 startRead and +0x06C getNextFrame are typed from their occupants, and the object's +0x00C `seekLoc[0x18]` is the CdlFILE `file` (CdStreamFile) that CdSearchFile fills. Zero bytes changed.

Renamed from CdStreamObj__GetNextFrame (tools/rename.py), the class rename only.

## Constants (track 7, round 99)

`0x800000` is `CDSTREAM_NEXT_FRAME_TRIES` (8388608, decimal as a count).
`header[2]` is `((StHEADER *)header)->frameCount`: Sony's STR sector header
(`<libcd.h>`) has four u16 words, then `frameCount` at byte 8, the word the
body loaded. Same `lw`, zero bytes changed.
