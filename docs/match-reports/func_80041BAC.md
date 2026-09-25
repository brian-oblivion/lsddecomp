# func_80041BAC -- MATCHED (12/12 words), round 82

Round 82, runner alpha (third re-staffed slot of the round). Unit `src/code_322b4.c`. Fresh ground (carved in FINISHING-PLAN revision 18), no prior attempt, no report before this one.

- **Where:** D_8006EC74 slot +0x040 (reset) (`tools/classtable.py`).
- **What:** Tail-dispatches its `u8` argument through the object's own slot +0x0C4 (`func_80041BDC`, the cell setter); the `andi a1,0xFF` sits in the `jalr` delay slot. Local method-table view `SpriteMethods_322b4`.
- **Result:** byte-exact; 12/12 words, 0 insertions / 0 deletions, whole-image SHA1 green (`./build-and-verify.sh` OK). First build.
- **Types:** local views and prototypes live in the unit; no shared header was touched.

## Source

```c
/* D_8006EC74 slot +0x040 (reset): re-select the cell through slot +0x0C4. */
void func_80041BAC(SpriteView_322b4 *self, u8 cell) {
    self->methods->setCell(self, cell);
}
```
