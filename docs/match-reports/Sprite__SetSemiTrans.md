# Sprite__SetSemiTrans -- MATCHED (11/11 words), round 82

> Renamed from `func_8004223C` on 2026-09-25 (tools/rename.py). Address 0x8004223c.

Round 82, runner alpha (third re-staffed slot of the round). Unit `src/code_322b4.c`. Fresh ground (carved in FINISHING-PLAN revision 18), no prior attempt, no report before this one.

- **Where:** slot +0x064 of D_8006EC74, D_8006ED4C, gSpriteMethods, D_8006EB90 and D_800879C4 (`tools/classtable.py`).
- **What:** `GetSetBitField(&self->attribute, 0x1E, 1, a1 != 0)` over the GsSPRITE attribute at +0x064; same shape as `func_80040714` in `code_2cc8c_f.c` and the `code_d294.c` +0x10 family. `GetSetBitField` prototype copied locally from `include/code_d294.h`.
- **Result:** byte-exact; 11/11 words, 0 insertions / 0 deletions, whole-image SHA1 green (`./build-and-verify.sh` OK). First build.
- **Types:** local views and prototypes live in the unit; no shared header was touched.

## Source

```c
/* Sprite classes slot +0x064: attribute bit 30. */
s32 Sprite__SetSemiTrans(SpriteView_322b4 *self, s32 a1) {
    return GetSetBitField(&self->attribute, 0x1E, 1, a1 != 0);
}
```
