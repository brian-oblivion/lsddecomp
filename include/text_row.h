/**
 * @file text_row.h
 * @brief TextRow, a row of CharSprite cells showing a string.
 *
 * Declares the TextRow class (object and method table), its pitch
 * constants, the function types its mismatched overrides are called
 * through, and its methods, which are defined in src/ui/screen_widgets.c.
 */
#ifndef TEXT_ROW_H
#define TEXT_ROW_H

#include "char_sprite.h"

typedef struct TextRow TextRow;
typedef struct TextRowMethods TextRowMethods;

/** TextRow's class id (gTextRowMethods word +0x000). */
#define TEXTROW_CLASS_ID 0x11144

#define TEXTROW_DEFAULT_PITCH 7 /**< reset's cellPitch: pixels from one cell's x to the next */
#define TEXTROW_GAP_WIDTH 16    /**< the extra x attachToParent adds before cell `gapIndex` */

/**
 * @brief TextRow's method table: CharSprite's slots, then TextRow's own.
 *
 * TextRow overrides the inherited ctor (TextRow__TextRow), finalize, reset,
 * attachToParent, detachFromParent, setDisplay, setColor, setPosition,
 * setCell (TextRow__SetCellAt) and getCell (TextRow__NoOpGetCell, empty).
 */
struct TextRowMethods {
    CHARSPRITE_SLOTS(TextRow, (TextRow * self, void *texture, s32 count, char *text));
    /* +0x0CC */ void (*setText)(TextRow *self, char *text); /**< @see TextRow__SetText */
    /* +0x0D0 */ void (*slotD0)(void); /**< @see TextRow__NoOpSlotD0; never called */
    /* +0x0D4 */ void (*setCellPitch)(TextRow *self, s32 pitch); /**< @see TextRow__SetCellPitch */
};

/**
 * @brief A row of CharSprite cells showing a string, one character a cell.
 *
 * Class id 0x11144, method table gTextRowMethods, parent CharSprite: the
 * ctor chains to CharSprite's first (GetCharSpriteMethods()->ctor with cell
 * 0x20), so the id tree (0x1144 -> 0x11144) is the ctor chain. Methods in
 * src/ui/screen_widgets.c. No class derives from it, so it defines no
 * FIELDS/SLOTS macros.
 *
 * - The ctor makes `count` CharSprites (New_CharSprite, same texture, cell
 *   0x20) into `cells`, then reset sets the pitch to 7 and the text.
 * - setText (+0x0CC) hands each cell the next byte of a NUL-terminated
 *   string through its setCell; setCellAt (the +0x0C4 override) sets one
 *   cell by index (TextEntry__SetCharAt: the name-entry character at `pos`).
 * - setPosition and attachToParent place cell after cell, adding
 *   `cellPitch` to the x of a running ScreenSpritePos; attachToParent adds
 *   one extra 0x10 before cell `gapIndex`.
 * - Its callers make one per string: TaskCore's slot and item widgets (one
 *   per name, TaskCore__CreateSlotElements), ItemList's 26-character rows,
 *   TitleMenu's save title, TextEntry's `textRow`, ObjM's "Pause".
 *
 * The inherited slots it overrides forward to the cells: attachToParent,
 * detachFromParent, setDisplay and setColor walk the visible window
 * `cells[firstVisible .. firstVisible + visibleCount)`; setPosition and
 * finalize walk all `cellCount`. Three overrides take a parameter list their
 * slot does not have, and keep the inherited slot type:
 *  - reset (+0x040) takes the text; the ctor, which passes it, casts to
 *    TextRowResetFn, as CharSprite's ctor does for its cell.
 *  - setCell (+0x0C4) is TextRow__SetCellAt(self, cell, index); a caller
 *    that passes the index casts to TextRowSetCellAtFn (TextEntry__SetCharAt).
 *  - setDisplay (+0x060) is TextRow__SetDisplay(self, on, result); with no
 *    visible cell it returns `result`, which a caller through the slot does
 *    not pass.
 * attachToParent's position is a ScreenSpritePos where SceneNode's slot
 * types a LongVec3 offset, the cast ScreenSprite's class doc describes.
 *
 * The object is 0xB8 bytes (New_TextRow). Its own fields start at +0x0A9,
 * inside CharSprite's word padding after `cellIndex`, which a flat expansion
 * of CHARSPRITE_FIELDS puts exactly there.
 */
struct TextRow {
    CHARSPRITE_FIELDS(TextRowMethods);
    /* +0x0A9 */ u8 cellCount; /**< the ctor's count: cells allocated; finalize releases them, setPosition walks them */
    /* +0x0AA */ u8 gapIndex; /**< 0: none; else attachToParent adds 0x10 before this cell (TitleMenu's save title: 9) */
    /* +0x0AB */ u8 visibleCount; /**< the window attach/detach/setDisplay/setColor walk; the ctor sets it to count (save title: 8) */
    /* +0x0AC */ u8 firstVisible; /**< the window's first cell; the ctor sets 0 (save title: 4) */
    /* +0x0AD */ u8 padAD[3];
    /* +0x0B0 */ s32 cellPitch; /**< setCellPitch; added to x between cells */
    /* +0x0B4 */ CharSprite **cells; /**< cellCount New_CharSprite cells. The object is 0xB8 bytes (New_TextRow) */
};

/** TextRow's method table (class id 0x11144). */
extern TextRowMethods gTextRowMethods;

/**
 * @brief Returns TextRow's method table.
 * @return &gTextRowMethods.
 */
extern TextRowMethods *GetTextRowMethods(void);

/** The +0x040 reset slot's occupant, TextRow__Reset, as the ctor calls it with the text. */
typedef void (*TextRowResetFn)(TextRow *self, char *text);
/** The +0x0C4 setCell slot's occupant, TextRow__SetCellAt, as a caller passing the index casts it. */
typedef void (*TextRowSetCellAtFn)(TextRow *self, s32 cell, s32 index);

/* The class's own methods, in address order. */

/**
 * @brief Allocates a TextRow and runs its ctor through the method table.
 * @param texture the font TimImage every cell draws from.
 * @param count   the number of cells.
 * @param text    the NUL-terminated string to show.
 * @return the new row, or NULL when the allocation fails.
 */
TextRow *New_TextRow(void *texture, s32 count, char *text);

/**
 * @brief Constructs a TextRow: CharSprite's ctor, `count` cells, then reset with the text.
 *
 * The whole row is visible and has no gap. When the cell array cannot be
 * allocated the row keeps no cells and reset does not run.
 * @param self    the object to construct.
 * @param texture the font TimImage every cell draws from.
 * @param count   the number of cells, each a New_CharSprite of cell ' '.
 * @param text    the NUL-terminated string to show.
 */
void TextRow__TextRow(TextRow *self, void *texture, s32 count, char *text);

/**
 * @brief Releases every cell and the cell array, then runs CharSprite's finalize.
 * @param self the row.
 */
void TextRow__Finalize(TextRow *self);

/**
 * @brief Sets the default pitch (TEXTROW_DEFAULT_PITCH) and shows `text`.
 * @param self the row.
 * @param text the NUL-terminated string.
 */
void TextRow__Reset(TextRow *self, char *text);

/**
 * @brief Attaches an unattached row under `parent`, then each visible cell under the row.
 *
 * The cells go left to right from `pos`, `cellPitch` apart, with
 * TEXTROW_GAP_WIDTH more before cell `gapIndex` when it is set. Does
 * nothing when the row already has a parent.
 * @param self   the row.
 * @param parent the node to attach under.
 * @param pos    the first visible cell's screen position.
 */
void TextRow__AttachToParent(TextRow *self, SceneNode *parent, ScreenSpritePos *pos);

/**
 * @brief Detaches each visible cell, then the row, when the row is attached.
 * @param self the row.
 */
void TextRow__DetachFromParent(TextRow *self);

/**
 * @brief Shows or hides each visible cell.
 * @param self   the row.
 * @param on     nonzero to show.
 * @param result returned unchanged when no cell is visible.
 * @return the last visible cell's setDisplay result, or `result`.
 */
s32 TextRow__SetDisplay(TextRow *self, s32 on, s32 result);

/**
 * @brief Sets each visible cell's colour.
 * @param self the row.
 * @param rgb  the colour.
 */
void TextRow__SetColor(TextRow *self, ColorRgb *rgb);

/**
 * @brief Moves an attached row: the row to `pos`, and every cell `cellPitch` apart from it.
 *
 * Walks all `cellCount` cells, with no gap. Does nothing while the row is
 * unattached.
 * @param self the row.
 * @param pos  the first cell's screen position.
 */
void TextRow__SetPosition(TextRow *self, ScreenSpritePos *pos);

/**
 * @brief Shows character `cell` in the cell at `index`.
 * @param self  the row.
 * @param cell  the character (its low byte is used).
 * @param index the cell's index in `cells`.
 */
void TextRow__SetCellAt(TextRow *self, s32 cell, s32 index);

/** @brief The getCell override: does nothing. */
void TextRow__NoOpGetCell(void);

/**
 * @brief Shows `text`, one byte a cell from the first cell, up to its NUL.
 *
 * Cells past the end of the string keep what they showed. The string must
 * not be longer than the row.
 * @param self the row.
 * @param text the NUL-terminated string, or NULL for no change.
 */
void TextRow__SetText(TextRow *self, char *text);

/** @brief The empty +0x0D0 slot. */
void TextRow__NoOpSlotD0(void);

/**
 * @brief Sets the x distance between one cell and the next.
 * @param self  the row.
 * @param pitch the distance in pixels.
 */
void TextRow__SetCellPitch(TextRow *self, s32 pitch);

#endif
