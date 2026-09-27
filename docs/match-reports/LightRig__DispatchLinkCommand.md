# LightRig__DispatchLinkCommand -- MATCHED (2/2 words), round 82

> Renamed from `D8006EFAC__DispatchLinkCommand` on 2026-09-26 (tools/rename.py). Address 0x80042820.

> Renamed from `func_80042820` on 2026-09-25 (tools/rename.py). Address 0x80042820.

Round 82, runner alpha (re-staffed slot). Unit `src/Sprite.c`. Fresh
ground (carved in FINISHING-PLAN revision 18), no prior attempt.

- **Where:** gLightRigMethods slot +0x09C (dispatchLinkCommand, over SceneNode__DispatchLinkCommand) (`tools/classtable.py`).
- **What:** empty override: `jr $ra; nop`.
- **Result:** byte-exact on the FIRST build; 2/2 words, 0 insertions /
  0 deletions, whole-image SHA1 green. No levers needed.
- **Types:** SceneNode-derived methods take `SceneNode *` from the UNIFIED
  `include/SceneNode.h` (untouched). The FrameClock and RequestedFile objects use
  unit-local views (`D_8006EF50Obj`, `D_8006EED8Obj`) declared at the top of
  the unit; nothing was added to a shared header.

## Source

```c
/* LightRig slot +0x09C (dispatchLinkCommand): empty override. */
void LightRig__DispatchLinkCommand(LightRig *self, void *sender, s32 event) {
}
```

## Naming

- `D8006EFAC__DispatchLinkCommand` -- tier A. Override of SceneNode's dispatchLinkCommand (slot +0x09C): empty body.

## Track 4

2026-09-26, round 86 (delta): class 0x14 unified as LightRig in `include/LightRig.h`. Renamed from `D8006EFAC__DispatchLinkCommand`, tier A: slot +0x09C, empty override. `self` is `LightRig *` (was `SceneNode *`). The Source block above is the unified spelling. Image byte-identical.
