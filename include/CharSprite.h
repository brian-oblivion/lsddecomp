#ifndef CHARSPRITE_H
#define CHARSPRITE_H

#include "ScreenSprite.h"

/*
 * CharSprite -- one character of an 8x8 font (class id 0x1144, method table
 * gCharSpriteMethods): ScreenSprite's subclass, a screen-space sprite whose
 * texture cell is picked by a one-byte character code. Methods in
 * src/Sprite.c. The name is for what the class's own methods do, and the
 * evidence is this:
 *  - setCell (+0x0C4) stores the byte in `cellIndex` and points the
 *    GsSPRITE's u,v at its cell through GetCellRect: column `cell & 0x1F`,
 *    row `cell >> 5`, 8x8 each, from the gCharSpriteCellRect origin. 256 codes in a
 *    32-wide grid is the ASCII layout.
 *  - The ctor sizes the sprite from cell 0x20, ASCII space, before it
 *    selects the caller's cell.
 *  - Its callers agree: TextRow (0x11144, the class below this one) makes
 *    a row of CharSprites and hands each the next byte of a NUL-terminated
 *    string through setCell (TextRow__SetText), and TextEntry__
 *    LoadCardResources makes one on FONTICON.TIM with 0x5F, '_'.
 *
 * The ctor chains to ScreenSprite's first (GetScreenSpriteMethods()->ctor
 * with cell 0x20's rect and 0), so the id tree (0x144 -> 0x1144) is the ctor
 * chain. One class derives from it: TextRow (0x11144, include/TextRow.h),
 * which expands these macros.
 *
 * reset (+0x040) is overridden with a parameter list SceneNode's slot does
 * not have: CharSprite__Reset takes the cell. The slot keeps SceneNode's
 * type; the ctor, which passes the cell, casts to CharSpriteResetFn (no
 * code).
 *
 * The object is 0xAC bytes (New_CharSprite): `cellIndex` at +0x0A8 is the
 * one field this class's methods touch, and 0xAC is sizeof rounded to the
 * word alignment the method pointer gives the struct. TextRow's own u8
 * fields start at +0x0A9, inside that padding, which a flat expansion
 * of CHARSPRITE_FIELDS puts exactly there.
 */

typedef struct CharSprite CharSprite;
typedef struct CharSpriteMethods CharSpriteMethods;

/* The font texture's layout, as GetCellRect reads it: cells of
 * CHARSPRITE_CELL_SIZE square, CHARSPRITE_GRID_COLUMNS to a row, cell n at
 * column n % columns, row n / columns, from gCharSpriteCellRect. */
#define CHARSPRITE_GRID_COLUMNS 32
#define CHARSPRITE_CELL_SIZE 8

/* ScreenSprite's slots, then this class's own. `tools/classtable.py
 * gCharSpriteMethods --vs gScreenSpriteMethods` lists the overrides of the
 * inherited ones (CharSprite__CharSprite, CharSprite__Reset). */
/* clang-format off */
#define CHARSPRITE_SLOTS(Self, CtorParams)                                                         \
    SCREENSPRITE_SLOTS(Self, CtorParams);                                                          \
    /* +0x0C4 */ void (*setCell)(Self *self, u8 cell); /* CharSprite__SetCell; gTextRowMethods: TextRow__SetCellAt */ \
    /* +0x0C8 */ u8 (*getCell)(Self *self)             /* CharSprite__GetCell; gTextRowMethods: TextRow__NoOpGetCell (empty) */
/* clang-format on */

/* clang-format off */
#define CHARSPRITE_FIELDS(Methods)                                                                 \
    SCREENSPRITE_FIELDS(Methods);                                                                  \
    /* +0x0A8 */ u8 cellIndex /* setCell stores it, getCell returns it. The object is 0xAC bytes (New_CharSprite) */
/* clang-format on */

struct CharSpriteMethods {
    CHARSPRITE_SLOTS(CharSprite, (CharSprite * self, void *texture, u8 cell));
};

struct CharSprite {
    CHARSPRITE_FIELDS(CharSpriteMethods);
};

extern CharSpriteMethods gCharSpriteMethods;
extern CharSpriteMethods *GetCharSpriteMethods(void); /* returns &gCharSpriteMethods */

/* +0x040's occupant in this class's table, as the ctor calls it through the
 * inherited slot (see the banner). */
typedef void (*CharSpriteResetFn)(CharSprite *self, u8 cell);

/* The class's own methods, in address order, and the free helper its ctor
 * and setCell share. */
CharSprite *New_CharSprite(void *texture, u8 cell);
void CharSprite__CharSprite(CharSprite *self, void *texture, u8 cell);
void CharSprite__Reset(CharSprite *self, u8 cell);
void CharSprite__SetCell(CharSprite *self, u8 cell);
u8 CharSprite__GetCell(CharSprite *self);
void GetCellRect(SpriteRect *dst, u32 cell); /* cell -> its 8x8 rect in the 32-wide grid */

#endif
