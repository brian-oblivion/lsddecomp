# CharSprite__CharSprite -- MATCHED (35/35 words), round 82

> Renamed from `D8006EC74__D8006EC74` on 2026-09-26 (tools/rename.py). Address 0x80041b20.

> Renamed from `func_80041B20` on 2026-09-25 (tools/rename.py). Address 0x80041b20.

Round 82, runner alpha (fifth slot on Sprite). Unit `src/graphics/sprite.c`. Fresh ground, no prior body attempt.

- **Where:** gCharSpriteMethods slot +0x008 (ctor) (`tools/classtable.py`).
- **What:** Fills a stack `SpriteRect` with cell 0x20's rect (`GetCellRect(&r, 0x20)`: u 0, v 8, 8x8), runs the ScreenSprite ctor (`GetScreenSpriteMethods()` slot +0x008) with (self, texture, &r, NULL), installs `GetCharSpriteMethods()`'s table and calls its reset (+0x040, CharSprite__Reset) with the caller's cell. The cell is a `u8` parameter: retail narrows it with `andi a1,s2,0xFF` in the reset call's delay slot (the round's New_CharSprite lever, applied directly).
- **Result:** byte-exact; 35/35 words, 0 insertions / 0 deletions, whole-image SHA1 green (`./build-and-verify.sh` OK). First build.
- **Types:** unit-local `SpriteMethods_322b4` gains `reset` at +0x040 (`(self, u8 cell)`, CharSprite__Reset's signature); no shared header touched.

## Source

```c
/* CharSprite slot +0x008 (ctor): the ScreenSprite ctor with cell 0x20's rect,
 * install the table, then reset to the caller's cell. */
void CharSprite__CharSprite(CharSprite *self, void *texture, u8 cell) {
    SpriteRect r;

    GetCellRect(&r, 0x20);
    GetScreenSpriteMethods()->ctor((ScreenSprite *)self, texture, &r, 0);
    self->methods = GetCharSpriteMethods();
    ((CharSpriteResetFn)self->methods->reset)(self, cell);
}
```

## Naming

- `D8006EC74__D8006EC74` -- tier A. Ctor (slot +0x008): calls D_8006ED4C's own ctor with a fixed cell (0x20)'s rect, installs the D_8006EC74 table, then resets to the caller's cell -- the "Class__Class" ctor convention (SceneNode__SceneNode, Sprite__Sprite).

## Track 4

2026-09-25, round 84 (charlie): D_8006EC74's parent class is unified in `include/ScreenSprite.h` (this class is still its own job). Its call of the parent ctor now goes through the unified table, `GetScreenSpriteMethods()->ctor((ScreenSprite *)self, texture, &r, 0)`: an upcast (no code) and `0` for the ctor's `s32 arg3`, where the deleted `CtorArg3Methods_322b4` view typed it `void *` and took NULL. It is the call that confirms this class's ctor chains to ScreenSprite's. Image byte-identical.

2026-09-26, round 86 (bravo): class 0x1144 unified as CharSprite in `include/char_sprite.h`. Renamed from `D8006EC74__D8006EC74`, tier A: the ctor slot (+0x008), `Class__Class`. `self` is `CharSprite *` (was the unit-local `SpriteView_322b4`). The reset call passes the cell, which SceneNode's +0x040 slot type does not take, so it casts to `CharSpriteResetFn` (no code), the round-85 TodActor +0x04C convention. The class name rests on this function: the default cell 0x20 is ASCII space (the header banner has the rest of the evidence). The Source block above is the unified spelling. Image byte-identical.

## Track 7 (round 99, charlie)

The sizing cell is spelled `' '` (was `0x20`): CharSprite's cells are character codes in an ASCII-ordered 32-column font (include/char_sprite.h's banner), and 0x20 is the space. Local `r` is `cellRect`. Byte-exact.
