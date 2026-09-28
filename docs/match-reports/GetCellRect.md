# GetCellRect -- MATCHED (20/20 words), round 82

> Renamed from `func_80041C4C` on 2026-09-25 (tools/rename.py). Address 0x80041c4c.

Round 82, runner alpha (third re-staffed slot of the round). Unit `src/graphics/Sprite.c`. Fresh ground (carved in FINISHING-PLAN revision 18), no prior attempt, no report before this one.

- **Where:** not in any method table (called by CharSprite__SetCell) (`tools/classtable.py`).
- **What:** Copies the 12-byte record `sCharSpriteCellRect` = {u 0, v 0, w 8, h 8} into `*dst`, then adds `(cell & 0x1F) * 8` to u and `(cell >> 5) * 8` to v: an 8x8 cell in a 32-wide grid.
- **Result:** byte-exact; 20/20 words, 0 insertions / 0 deletions, whole-image SHA1 green (`./build-and-verify.sh` OK).
- **Types:** local views and prototypes live in the unit; no shared header was touched.

## Lever

Three builds. (1) `u8 cell` parameter: cc1 drops the `andi 0xFF` and folds `(cell >> 5) * 8` into `srl 2; andi 0x38` (1 word short, image shifted). (2) `s32 cell` with an explicit `cell &= 0xFF;` after the struct copy: right length, `andi` in retail's position, but `sra` instead of `srl` (19/20). (3) `u32 cell` with `cell &= 0xFF;`: byte-exact, whole image green. The caller's prototype was changed to match (its own `andi` before the `jal` is unaffected).

## Source

```c
/* Cell index -> 8x8 rect in a 32-wide grid, offset from sCharSpriteCellRect. */
void GetCellRect(SpriteRect *dst, u32 cell) {
    *dst = sCharSpriteCellRect;
    cell &= 0xFF;
    dst->u += (cell & 0x1F) * 8;
    dst->v += (cell >> 5) * 8;
}
```

### Proposed learning

Retail `andi a1,a1,0xFF` emitted mid-body (after unrelated work), followed by `srl 5; sll 3` rather than a folded `srl 2; andi 0x38`, is an UNSIGNED word parameter masked explicitly in the body (`u32 cell; ... cell &= 0xFF;`). A `u8` parameter lets cc1 drop the mask and fold the shift pair; an `s32` one gives `sra`.

## Track 4 (2026-09-25, round 82, alpha)

`CellRect_322b4` became `SpriteRect` (include/Sprite.h), the Sprite class's texture-cell type; sCharSpriteCellRect is declared `SpriteRect`. The the class is unified as `Sprite` in `include/Sprite.h` (the base sprite class, id 0x44, table `gSpriteMethods`, formerly `D_8006EE1C`); the Source block above is the unified spelling, byte-identical (whole image green, 0 new `-Wall` warnings).

## Naming

- `GetCellRect` -- tier A. Free function: cell index -> 8x8 rect in a 32-wide grid, offset from the sCharSpriteCellRect origin table. Pure computation, called by both CharSprite__CharSprite (fixed cell) and CharSprite__SetCell (caller's cell); not a method of one class.

## Track 4

2026-09-26, round 86 (bravo): the class whose methods call it is unified as CharSprite (`include/CharSprite.h`, formerly D_8006EC74); its prototype moved there from the unit, beside the ctor and setCell that share it. The function is unchanged. Image byte-identical.

## Track 7 (round 99, charlie)

`(cell & 0x1F) * 8` and `(cell >> 5) * 8` are spelled `(cell % CHARSPRITE_GRID_COLUMNS) * CHARSPRITE_CELL_SIZE` and `(cell / CHARSPRITE_GRID_COLUMNS) * CHARSPRITE_CELL_SIZE` (32 and 8, include/CharSprite.h); `cell` is `u32`, so `%`/`/` by 32 are the same `andi`/`srl`. `cell &= 0xFF` stays: a byte mask, hex.

### Naming: `D_8006ED40` -> `sCharSpriteCellRect`

Tier A. A 12-byte `SpriteRect` in data, {u 0, v 0, w 8, h 8}, read only here: GetCellRect copies it and moves u,v to the cell's column and row, so it is the rect of cell 0 of CharSprite's font texture. `g` prefix: a splat-owned global, not unit-static. Byte-exact (`tools/rename.py`).
