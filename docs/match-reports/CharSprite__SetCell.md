# CharSprite__SetCell -- MATCHED (19/19 words), round 82

> Renamed from `D8006EC74__SetCell` on 2026-09-26 (tools/rename.py). Address 0x80041bdc.

> Renamed from `func_80041BDC` on 2026-09-25 (tools/rename.py). Address 0x80041bdc.

Round 82, runner alpha (third re-staffed slot of the round). Unit `src/graphics/sprite.c`. Fresh ground (carved in FINISHING-PLAN revision 18), no prior attempt, no report before this one.

- **Where:** gCharSpriteMethods slot +0x0C4 (`tools/classtable.py`).
- **What:** Stores the `u8` cell index at +0x0A8, has `GetCellRect` fill a 12-byte `CellRect_322b4` local at sp+0x10, and copies its low bytes of `u`/`v` into the GsSPRITE u/v at +0x072/+0x073 (`lbu` of a `u16` field narrowed by the `u8` store).
- **Result:** byte-exact; 19/19 words, 0 insertions / 0 deletions, whole-image SHA1 green (`./build-and-verify.sh` OK). First build.
- **Types:** local views and prototypes live in the unit; no shared header was touched.

## Source

```c
/* CharSprite slot +0x0C4: store the cell index and point u,v at its 8x8 cell. */
void CharSprite__SetCell(CharSprite *self, u8 cell) {
    SpriteRect r;

    self->cellIndex = cell;
    GetCellRect(&r, cell);
    self->sprite.u = r.u;
    self->sprite.v = r.v;
}
```

## Track 4 (2026-09-25, round 82, alpha)

`CellRect_322b4` became `SpriteRect` (include/sprite.h): the same 12-byte {u16 u, v; s32 w, h} cell Sprite__Reset copies into Sprite.rect. This function belongs to D_8006EC74 (a Sprite subclass, since round 86 CharSprite) and keeps its unit-local `SpriteView_322b4` self type. The the class is unified as `Sprite` in `include/sprite.h` (the base sprite class, id 0x44, table `gSpriteMethods`, formerly `D_8006EE1C`); the Source block above is the unified spelling, byte-identical (whole image green, 0 new `-Wall` warnings).

## Naming

- `D8006EC74__SetCell` -- tier A. Slot +0x0C4: stores the cell index and derives u,v from GetCellRect. Pure setter, mechanics is the purpose.

## Track 4

2026-09-26, round 86 (bravo): class 0x1144 unified as CharSprite in `include/char_sprite.h`. Renamed from `D8006EC74__SetCell`, tier A: slot +0x0C4, which the header names `setCell` for it. `self` is `CharSprite *`; `cellIndex` (+0x0A8) is the class's one own field, and u,v are Sprite's `sprite.u`/`sprite.v` (+0x072/+0x073, the offsets `SpriteView_322b4` gave them). The Source block above is the unified spelling. Image byte-identical.

## Track 7 (round 99, charlie)

Local `r` is `cellRect`. Byte-exact.
