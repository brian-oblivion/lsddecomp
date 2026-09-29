#ifndef SCREEN_SPRITE_H
#define SCREEN_SPRITE_H

#include "sprite.h"

/**
 * @file screen_sprite.h
 * @brief ScreenSprite, the sprite placed in screen space by a position and a
 *        pivot anchor.
 *
 * Viewport__DrawNode takes a separate path for
 * `(tag & CLASS_ID_LEVEL3_MASK) == SCREENSPRITE_CLASS_ID`, i.e. this class and
 * everything below it: the GsSPRITE's x/y are
 * `screenPos` read as a percentage of half the screen width/height from the
 * centre, plus the pivot (mx, my), with no projection. Every other 0x44
 * sprite is projected from its coordinate.
 *
 * The class's two own slots set exactly those: setPosition (+0x0BC) stores
 * `screenPos`, setPivotAnchor (+0x0C0) moves (mx, my). Its attachToParent
 * override attaches with a zero 3-D offset and hands its third argument to
 * setPosition instead; the callers pass pairs like (-70, -60) and (-100, -60)
 * (sCardIconPos, sItemListPanelPos), percentages.
 *
 * Not settled here: the ctor occupant returns nothing where the slot, from
 * SceneNode, returns `void *`; and the attachToParent override's third
 * argument is a ScreenSpritePos where the slot, SceneNode's, types it
 * `LongVec3 *offset`, so a call through the slot with a position needs a
 * cast. Both slots are SceneNode's to change.
 */

typedef struct ScreenSprite ScreenSprite;
typedef struct ScreenSpriteMethods ScreenSpriteMethods;
typedef struct ScreenSpritePos ScreenSpritePos;

/** ScreenSprite's class id (gScreenSpriteMethods word +0x000). Three nibbles,
 * so `(header & CLASS_ID_LEVEL3_MASK) == SCREENSPRITE_CLASS_ID` is its is-kind-of test
 * (Viewport__DrawNode). */
#define SCREENSPRITE_CLASS_ID 0x144

/** A screen position: percent of half the screen width/height, from the
 * centre (Viewport__DrawNode). */
struct ScreenSpritePos {
    s32 x; /**< percent of half the screen width, from the centre */
    s32 y; /**< percent of half the screen height, from the centre */
};

/**
 * setPivotAnchor's argument: where (mx, my) moves on the sprite's cell.
 * LEFT/RIGHT move mx only, TOP/BOTTOM my only; any other value does nothing.
 */
enum ScreenSpriteAnchor {
    SCREENSPRITE_ANCHOR_CENTRE = 0, /**< (w / 2, h / 2) */
    SCREENSPRITE_ANCHOR_LEFT = 1,   /**< mx = 0 */
    SCREENSPRITE_ANCHOR_RIGHT = 2,  /**< mx = w */
    SCREENSPRITE_ANCHOR_TOP = 3,    /**< my = 0 */
    SCREENSPRITE_ANCHOR_BOTTOM = 4  /**< my = h */
};

/**
 * Sprite's slots, then ScreenSprite's own, for ScreenSpriteMethods and every
 * subclass's table to expand first. gScreenSpriteMethods overrides the ctor,
 * reset and attachToParent with the ScreenSprite__ methods below.
 */
/* clang-format off */
#define SCREENSPRITE_SLOTS(Self, CtorParams)                                                       \
    SPRITE_SLOTS(Self, CtorParams);                                                                \
    /* +0x0BC */ void (*setPosition)(Self *self, ScreenSpritePos *pos); /* ScreenSprite__SetPosition; gTextRowMethods: TextRow__SetPosition */ \
    /* +0x0C0 */ void (*setPivotAnchor)(Self *self, u32 anchor)         /* ScreenSprite__SetPivotAnchor */
/* clang-format on */

/** Sprite's fields, then ScreenSprite's own, for every subclass to expand first. */
/* clang-format off */
#define SCREENSPRITE_FIELDS(Methods)                                                               \
    SPRITE_FIELDS(Methods);                                                                        \
    /* +0x0A0 */ ScreenSpritePos screenPos /* setPosition; Viewport__DrawNode places the sprite from it. The object is 0xA8 bytes (New_ScreenSprite): CharSprite's own fields start at +0x0A8 */
/* clang-format on */

/** ScreenSprite's method table: SCREENSPRITE_SLOTS with its ctor parameters. */
struct ScreenSpriteMethods {
    SCREENSPRITE_SLOTS(ScreenSprite,
                       (ScreenSprite * self, void *texture, SpriteRect *rect, s32 resetWord));
};

/**
 * ScreenSprite: a Sprite drawn at a screen position rather than projected.
 * Class id 0x144 (SCREENSPRITE_CLASS_ID), table gScreenSpriteMethods, parent
 * Sprite, whose ctor it chains to first; methods in src/graphics/char_sprite.c.
 * Two classes derive from it: CharSprite (0x1144, one 8x8 font character,
 * include/char_sprite.h), which expands these macros, and TextRow (0x11144,
 * below CharSprite, include/text_row.h). The object is 0xA8 bytes
 * (New_ScreenSprite). It takes its position when attached (attachToParent);
 * setPosition and setPivotAnchor do nothing before that.
 */
struct ScreenSprite {
    SCREENSPRITE_FIELDS(ScreenSpriteMethods);
};

/** ScreenSprite's method table (class id 0x144). */
extern ScreenSpriteMethods gScreenSpriteMethods;

/**
 * @brief The ScreenSprite method table.
 * @return &gScreenSpriteMethods.
 */
extern ScreenSpriteMethods *GetScreenSpriteMethods(void);

/**
 * @brief Allocates a ScreenSprite and runs its ctor through the table.
 * @param texture The TimImage to draw from.
 * @param rect The texture cell.
 * @param resetWord Handed on to Sprite's ctor.
 * @return The new sprite, or NULL when the allocation fails.
 */
ScreenSprite *New_ScreenSprite(void *texture, SpriteRect *rect, s32 resetWord);

/**
 * @brief Constructor (slot +0x008): Sprite's ctor with abr 0 and a NULL
 *        resetArg, installs gScreenSpriteMethods, then calls reset.
 * @param self The sprite.
 * @param texture The TimImage to draw from.
 * @param rect The texture cell.
 * @param resetWord Handed on to Sprite's ctor.
 */
void ScreenSprite__ScreenSprite(ScreenSprite *self, void *texture, SpriteRect *rect, s32 resetWord);

/**
 * @brief Reset (slot +0x040): empty; the texture stays as Sprite's ctor bound it.
 * @param self The sprite.
 */
void ScreenSprite__Reset(ScreenSprite *self);

/**
 * @brief attachToParent (slot +0x04C): when not yet attached, attaches through
 *        Sprite's attachToParent with a zero offset, then hands `pos` to
 *        setPosition. Shared by gCharSpriteMethods.
 * @param self The sprite.
 * @param parent The node to attach to.
 * @param pos The screen position.
 */
void ScreenSprite__AttachToParent(ScreenSprite *self, SceneNode *parent, ScreenSpritePos *pos);

/**
 * @brief setPosition (slot +0x0BC): stores the screen position, once attached.
 * @param self The sprite.
 * @param pos The screen position, copied.
 */
void ScreenSprite__SetPosition(ScreenSprite *self, ScreenSpritePos *pos);

/**
 * @brief setPivotAnchor (slot +0x0C0): once attached, moves the pivot (mx, my)
 *        to the edge or centre `anchor` names.
 * @param self The sprite.
 * @param anchor An enum ScreenSpriteAnchor; other values do nothing.
 */
void ScreenSprite__SetPivotAnchor(ScreenSprite *self, u32 anchor);

#endif
