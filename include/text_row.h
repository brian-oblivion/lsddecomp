#ifndef TEXT_ROW_H
#define TEXT_ROW_H

#include "char_sprite.h"

/*
 * TextRow -- a row of CharSprite cells showing a string (class id 0x11144,
 * method table gTextRowMethods): CharSprite's subclass. Methods in
 * src/ui/screen_widgets.c, New_TextRow to GetTextRowMethods. The name is for what
 * the class's own methods do, and the evidence is this:
 *  - The ctor makes `count` CharSprites (New_CharSprite, same texture, cell
 *    0x20) into `cells`, then reset sets the pitch to 7 and the text.
 *  - setText (+0x0CC) hands each cell the next byte of a NUL-terminated
 *    string through its setCell; setCellAt (the +0x0C4 override) sets one
 *    cell by index (TextEntry__SetCharAt: the name-entry character at `pos`).
 *  - setPosition and attachToParent place cell after cell, adding
 *    `cellPitch` to the x of a running ScreenSpritePos, with one extra 0x10
 *    before cell `gapIndex`.
 *  - Its callers make one per string: TaskCore's slot and item widgets (one
 *    per name, TaskCore__CreateSlotElements), ItemList's 26-character rows,
 *    TitleMenu's name field, TextEntry's `textRow`, ObjM's "Pause".
 *
 * The ctor chains to CharSprite's first (GetCharSpriteMethods()->ctor with
 * cell 0x20), so the id tree (0x1144 -> 0x11144) is the ctor chain. No class
 * derives from it, so it defines no FIELDS/SLOTS macros.
 *
 * The inherited slots it overrides forward to the cells: attachToParent,
 * detachFromParent, setDisplay and setColor walk the visible window
 * `cells[firstVisible .. firstVisible + visibleCount)`; setPosition and
 * finalize walk all `cellCount`. Three overrides take a parameter list their
 * slot does not have, and keep the inherited slot type:
 *  - reset (+0x040) takes the text; the ctor, which passes it, casts to
 *    TextRowResetFn (no code), as CharSprite's ctor does for its cell.
 *  - setCell (+0x0C4) is TextRow__SetCellAt(self, cell, index); a caller
 *    that passes the index casts to TextRowSetCellAtFn (TextEntry__SetCharAt).
 *  - setDisplay (+0x060) is TextRow__SetDisplay(self, on, result): the value
 *    it returns with no visible cell is the caller's untouched $a2.
 * attachToParent's position is a ScreenSpritePos where SceneNode's slot
 * types a LongVec3 offset, the cast ScreenSprite's banner describes.
 *
 * The object is 0xB8 bytes (New_TextRow). Its own fields start at +0x0A9,
 * inside CharSprite's word padding after `cellIndex`, which a flat expansion
 * of CHARSPRITE_FIELDS puts exactly there.
 */

typedef struct TextRow TextRow;
typedef struct TextRowMethods TextRowMethods;

#define TEXTROW_DEFAULT_PITCH 7 /* reset's cellPitch: pixels from one cell's x to the next */
#define TEXTROW_GAP_WIDTH 16    /* the extra x attachToParent adds before cell `gapIndex` */

/* CharSprite's slots, then this class's own. `tools/classtable.py
 * gTextRowMethods --vs gCharSpriteMethods` lists the overrides of the
 * inherited ones: TextRow__TextRow, __Finalize, __Reset, __AttachToParent,
 * __DetachFromParent, __SetDisplay, __SetColor, __SetPosition, __SetCellAt
 * and __NoOpGetCell (empty, at getCell). */
struct TextRowMethods {
    CHARSPRITE_SLOTS(TextRow, (TextRow * self, void *texture, s32 count, char *text));
    /* +0x0CC */ void (*setText)(TextRow *self, char *text); /* TextRow__SetText */
    /* +0x0D0 */ void (*slotD0)(void); /* TextRow__NoOpSlotD0, empty; never called */
    /* +0x0D4 */ void (*setCellPitch)(TextRow *self, s32 pitch); /* TextRow__SetCellPitch; reset passes 7 */
};

struct TextRow {
    CHARSPRITE_FIELDS(TextRowMethods);
    /* +0x0A9 */ u8 cellCount; /* the ctor's count: cells allocated; finalize releases them, setPosition walks them */
    /* +0x0AA */ u8 gapIndex; /* 0: none; else attachToParent adds 0x10 before this cell (TitleMenu's name field: 9) */
    /* +0x0AB */ u8 visibleCount; /* the window attach/detach/setDisplay/setColor walk; the ctor sets it to count (name field: 8) */
    /* +0x0AC */ u8 firstVisible; /* its first cell; the ctor sets 0 (name field: 4) */
    /* +0x0AD */ u8 padAD[3];
    /* +0x0B0 */ s32 cellPitch; /* setCellPitch; added to x between cells */
    /* +0x0B4 */ CharSprite **cells; /* cellCount New_CharSprite cells. The object is 0xB8 bytes (New_TextRow) */
};

extern TextRowMethods gTextRowMethods;
extern TextRowMethods *GetTextRowMethods(void); /* returns &gTextRowMethods */

/* The overrides whose parameter lists differ from their slot's (see the
 * banner), as a caller that passes the extra argument casts them. */
typedef void (*TextRowResetFn)(TextRow *self, char *text);
typedef void (*TextRowSetCellAtFn)(TextRow *self, s32 cell, s32 index);

/* The class's own methods, in address order. */
TextRow *New_TextRow(void *texture, s32 count, char *text);
void TextRow__TextRow(TextRow *self, void *texture, s32 count, char *text);
void TextRow__Finalize(TextRow *self);
void TextRow__Reset(TextRow *self, char *text);
void TextRow__AttachToParent(TextRow *self, SceneNode *parent, ScreenSpritePos *pos);
void TextRow__DetachFromParent(TextRow *self);
s32 TextRow__SetDisplay(TextRow *self, s32 on, s32 result);
void TextRow__SetColor(TextRow *self, ColorRgb *rgb);
void TextRow__SetPosition(TextRow *self, ScreenSpritePos *pos);
void TextRow__SetCellAt(TextRow *self, s32 cell, s32 index);
void TextRow__NoOpGetCell(void);
void TextRow__SetText(TextRow *self, char *text);
void TextRow__NoOpSlotD0(void);
void TextRow__SetCellPitch(TextRow *self, s32 pitch);

#endif
