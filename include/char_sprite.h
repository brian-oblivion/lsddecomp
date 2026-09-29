#ifndef CHAR_SPRITE_H
#define CHAR_SPRITE_H

#include "screen_sprite.h"

/**
 * @file char_sprite.h
 * @brief CharSprite, one character of an 8x8 font drawn as a screen-space
 *        sprite, and the font texture's cell layout.
 *
 * The name is for what the class's own methods do, and the evidence is this:
 *  - setCell (+0x0C4) stores the byte in `cellIndex` and points the
 *    GsSPRITE's u,v at its cell through GetCellRect: column `cell & 0x1F`,
 *    row `cell >> 5`, 8x8 each, from the sCharSpriteCellRect origin. 256
 *    codes in a 32-wide grid is the ASCII layout.
 *  - The ctor sizes the sprite from cell 0x20, ASCII space, before it
 *    selects the caller's cell.
 *  - Its callers agree: TextRow (0x11144, the class below this one) makes
 *    a row of CharSprites and hands each the next byte of a NUL-terminated
 *    string through setCell (TextRow__SetText), and TextEntry__
 *    LoadCardResources makes one on FONTICON.TIM with 0x5F, '_'.
 *
 * reset (+0x040) is overridden with a parameter list SceneNode's slot does
 * not have: CharSprite__Reset takes the cell. The slot keeps SceneNode's
 * type; the ctor, which passes the cell, casts to CharSpriteResetFn.
 */

typedef struct CharSprite CharSprite;
typedef struct CharSpriteMethods CharSpriteMethods;

/** CharSprite's class id (gCharSpriteMethods word +0x000): ScreenSprite's 0x144, one level down. */
#define CHARSPRITE_CLASS_ID 0x1144

/** @name Font grid
 * The font texture's layout, as GetCellRect reads it: cells of
 * CHARSPRITE_CELL_SIZE square, CHARSPRITE_GRID_COLUMNS to a row, cell n at
 * column n % columns, row n / columns, from sCharSpriteCellRect. @{ */
#define CHARSPRITE_GRID_COLUMNS 32 /**< cells per row of the font texture */
#define CHARSPRITE_CELL_SIZE 8     /**< a cell's width and height, in texels */
/** @} */

/**
 * ScreenSprite's slots, then CharSprite's own, for CharSpriteMethods and
 * TextRow's table to expand first. gCharSpriteMethods overrides the ctor and
 * reset with CharSprite__CharSprite and CharSprite__Reset.
 */
/* clang-format off */
#define CHARSPRITE_SLOTS(Self, CtorParams)                                                         \
    SCREENSPRITE_SLOTS(Self, CtorParams);                                                          \
    /* +0x0C4 */ void (*setCell)(Self *self, u8 cell); /* CharSprite__SetCell; gTextRowMethods: TextRow__SetCellAt */ \
    /* +0x0C8 */ u8 (*getCell)(Self *self)             /* CharSprite__GetCell; gTextRowMethods: TextRow__NoOpGetCell (empty) */
/* clang-format on */

/**
 * ScreenSprite's fields, then CharSprite's one own, for TextRow to expand
 * first. The object is 0xAC bytes: `cellIndex` at +0x0A8, then word padding,
 * inside which TextRow's own u8 fields start at +0x0A9.
 */
/* clang-format off */
#define CHARSPRITE_FIELDS(Methods)                                                                 \
    SCREENSPRITE_FIELDS(Methods);                                                                  \
    /* +0x0A8 */ u8 cellIndex /* setCell stores it, getCell returns it. The object is 0xAC bytes (New_CharSprite) */
/* clang-format on */

/** CharSprite's method table: CHARSPRITE_SLOTS with its ctor parameters. */
struct CharSpriteMethods {
    CHARSPRITE_SLOTS(CharSprite, (CharSprite * self, void *texture, u8 cell));
};

/**
 * CharSprite: one character of an 8x8 font, a screen-space sprite whose
 * texture cell is picked by a one-byte character code. Class id 0x1144,
 * table gCharSpriteMethods, parent ScreenSprite, whose ctor it chains to
 * first; methods in src/graphics/sprite.c. One class derives from it:
 * TextRow (0x11144, include/text_row.h), which expands these macros. The
 * object is 0xAC bytes (New_CharSprite).
 */
struct CharSprite {
    CHARSPRITE_FIELDS(CharSpriteMethods);
};

/** CharSprite's method table (class id 0x1144). */
extern CharSpriteMethods gCharSpriteMethods;

/**
 * @brief The CharSprite method table.
 * @return &gCharSpriteMethods.
 */
extern CharSpriteMethods *GetCharSpriteMethods(void);

/** CharSprite__Reset's type, as the ctor calls it through the inherited
 * reset slot (+0x040), which SceneNode types without the cell. */
typedef void (*CharSpriteResetFn)(CharSprite *self, u8 cell);

/**
 * @brief Allocates a CharSprite and runs its ctor through the table.
 * @param texture The font's TimImage.
 * @param cell The character code to show.
 * @return The new sprite, or NULL when the allocation fails.
 */
CharSprite *New_CharSprite(void *texture, u8 cell);

/**
 * @brief Constructor (slot +0x008): ScreenSprite's ctor on the space
 *        character's cell (which sets the sprite's size), installs
 *        gCharSpriteMethods, then resets to `cell`.
 * @param self The sprite.
 * @param texture The font's TimImage.
 * @param cell The character code to show.
 */
void CharSprite__CharSprite(CharSprite *self, void *texture, u8 cell);

/**
 * @brief Reset (slot +0x040): selects the cell through setCell.
 * @param self The sprite.
 * @param cell The character code.
 */
void CharSprite__Reset(CharSprite *self, u8 cell);

/**
 * @brief setCell (slot +0x0C4): stores the character code in `cellIndex` and
 *        points the GsSPRITE's u, v at its 8x8 cell (GetCellRect).
 * @param self The sprite.
 * @param cell The character code.
 */
void CharSprite__SetCell(CharSprite *self, u8 cell);

/**
 * @brief getCell (slot +0x0C8): the character code setCell stored.
 * @param self The sprite.
 * @return cellIndex.
 */
u8 CharSprite__GetCell(CharSprite *self);

/**
 * @brief The texture cell of a character code: sCharSpriteCellRect moved to
 *        the code's column and row in the 32-wide grid of 8x8 cells.
 * @param dst Out: the cell.
 * @param cell The character code; only its low byte counts.
 */
void GetCellRect(SpriteRect *dst, u32 cell);

#endif
