# func_80041BDC -- MATCHED (19/19 words), round 82

Round 82, runner alpha (third re-staffed slot of the round). Unit `src/code_322b4.c`. Fresh ground (carved in FINISHING-PLAN revision 18), no prior attempt, no report before this one.

- **Where:** D_8006EC74 slot +0x0C4 (`tools/classtable.py`).
- **What:** Stores the `u8` cell index at +0x0A8, has `func_80041C4C` fill a 12-byte `CellRect_322b4` local at sp+0x10, and copies its low bytes of `u`/`v` into the GsSPRITE u/v at +0x072/+0x073 (`lbu` of a `u16` field narrowed by the `u8` store).
- **Result:** byte-exact; 19/19 words, 0 insertions / 0 deletions, whole-image SHA1 green (`./build-and-verify.sh` OK). First build.
- **Types:** local views and prototypes live in the unit; no shared header was touched.

## Source

```c
/* D_8006EC74 slot +0x0C4: store the cell index and point u,v at its 8x8 cell. */
void func_80041BDC(SpriteView_322b4 *self, u8 cell) {
    SpriteRect r;

    self->unkA8 = cell;
    func_80041C4C(&r, cell);
    self->u = r.u;
    self->v = r.v;
}
```

## Track 4 (2026-09-25, round 82, alpha)

`CellRect_322b4` became `SpriteRect` (include/Sprite.h): the same 12-byte {u16 u, v; s32 w, h} cell Sprite__Reset copies into Sprite.rect. This function belongs to D_8006EC74 (a Sprite subclass) and keeps its unit-local `SpriteView_322b4` self type. The the class is unified as `Sprite` in `include/Sprite.h` (the base sprite class, id 0x44, table `gSpriteMethods`, formerly `D_8006EE1C`); the Source block above is the unified spelling, byte-identical (whole image green, 0 new `-Wall` warnings).
