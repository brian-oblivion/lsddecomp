# DrawSystem__StoreImage -- MATCHED (31/31 words), round 82

> Renamed from `func_800209A0` on 2026-09-25 (tools/rename.py). Address 0x800209a0.

Round 82, runner alpha. Unit `src/code_10ee0.c`. Fresh ground, no prior attempt.

- **Where:** D_8006C070 slot +0x05C (`tools/classtable.py D_8006C070`), the
  sibling of DrawSystem__LoadImage (+0x058).
- **What:** the StoreImage twin of DrawSystem__LoadImage (LoadImage), with the two
  data arguments in the OTHER order: `(self, pixels, src)`. The asm shows it
  directly: `$a1` is saved to `$s1` and handed to StoreImage, `$a2` is moved
  into `$a1` for ConvertRect.
- **Result:** byte-exact on the FIRST build, no levers; `funcdiff.py` reports
  31/31 words, 0 insertions / 0 deletions, and the whole-image SHA1 is green
  (`OK: build matches retail`).

## Source

```c
extern int StoreImage(RECT *rect, u_long *p);       /* LIBGPU.H */

void DrawSystem__StoreImage(Class6C070 *self, u_long *pixels, Class6C070Rect *src) {
    RECT rect;

    if (self->unk10 == 0 || self->unk2C != 0) {
        ConvertRect(&rect, src);
        StoreImage(&rect, pixels);
        if (self->unk2C != 0) {
            DrawSync(0);
        }
    }
}
```

The declarations it needs are the unit-local `Class6C070`, `RECT` and
`Class6C070Rect` view at the top of `src/code_10ee0.c` (reproduced in full in
`docs/match-reports/DrawSystem__LoadImage.md`), plus `DrawSync` and `ConvertRect`.

## Naming

`DrawSystem__StoreImage`, tier B. Wraps LIBGPU.H's `StoreImage`, the mirror
of `DrawSystem__LoadImage`; unlike LoadImage/MoveImage, no other unit's
local view names this slot (+0x05C), so it rests on the SDK-wrapper
mechanic alone rather than cross-unit agreement.

## Track 4 (2026-09-26, round 87, bravo)

The class is unified: its one definition is `include/DrawSystem.h` (object, method table, `ScreenDims`, `DrawRect`); the unit-local view the declarations above quote is gone. Field renames, settled by this unit's accessors (the only ones): `unk20` -> `vsyncCount` (SetVSyncCount stores, GetVSyncCount returns, RunLoop passes it to VSync, CountFrames compares against it), `unk24` -> `frameCount` (CountFrames counts it). `DrawSystemRect`/`DrawSystemDims` are one type, `DrawRect` {s16 x, y; s32 w, h} (ConvertRect's halfword loads at +0/+2/+4/+8 compile identically from it), and `DrawSystemSize` is `ScreenDims`. The singleton `D_8008A83C` is `gDrawSystem` (rename.py). Byte-identical.
