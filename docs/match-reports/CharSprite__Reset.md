# CharSprite__Reset -- MATCHED (12/12 words), round 82

> Renamed from `D8006EC74__Reset` on 2026-09-26 (tools/rename.py). Address 0x80041bac.

> Renamed from `func_80041BAC` on 2026-09-25 (tools/rename.py). Address 0x80041bac.

Round 82, runner alpha (third re-staffed slot of the round). Unit `src/graphics/sprite.c`. Fresh ground (carved in FINISHING-PLAN revision 18), no prior attempt, no report before this one.

- **Where:** gCharSpriteMethods slot +0x040 (reset) (`tools/classtable.py`).
- **What:** Tail-dispatches its `u8` argument through the object's own slot +0x0C4 (`CharSprite__SetCell`, the cell setter); the `andi a1,0xFF` sits in the `jalr` delay slot. Local method-table view `SpriteMethods_322b4`.
- **Result:** byte-exact; 12/12 words, 0 insertions / 0 deletions, whole-image SHA1 green (`./build-and-verify.sh` OK). First build.
- **Types:** local views and prototypes live in the unit; no shared header was touched.

## Source

```c
/* CharSprite slot +0x040 (reset): re-select the cell through slot +0x0C4. */
void CharSprite__Reset(CharSprite *self, u8 cell) {
    self->methods->setCell(self, cell);
}
```

## Naming

- `D8006EC74__Reset` -- tier A. Reset override (slot +0x040): forwards to the class's own setCell slot with the caller's cell. Pure leaf, mechanics is the purpose.

## Track 4

2026-09-26, round 86 (bravo): class 0x1144 unified as CharSprite in `include/char_sprite.h`. Renamed from `D8006EC74__Reset`, tier A: the reset slot (+0x040), whose override adds the cell parameter (the header names the typedef the ctor calls it through). `self` is `CharSprite *`; the call goes through the unified slot name `setCell` (+0x0C4). The Source block above is the unified spelling. Image byte-identical.
