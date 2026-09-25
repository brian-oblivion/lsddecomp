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
    CellRect_322b4 r;

    self->unkA8 = cell;
    func_80041C4C(&r, cell);
    self->u = r.u;
    self->v = r.v;
}
```
