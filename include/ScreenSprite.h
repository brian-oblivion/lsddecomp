#ifndef SCREENSPRITE_H
#define SCREENSPRITE_H

#include "Sprite.h"

/*
 * ScreenSprite -- the sprite placed in screen space (class id 0x144, method
 * table gScreenSpriteMethods): Sprite's direct subclass, adding a screen
 * position and a pivot anchor. Methods in src/graphics/Sprite.c. The name is for
 * what the class does, and the evidence is this:
 *  - Viewport__DrawNode (ViewportDraw.c) takes a separate path for
 *    `(tag & 0xFFF) == 0x144`, i.e. this class and everything below it: the
 *    GsSPRITE's x/y are `screenPos` read as a percentage of half the screen
 *    width/height from the centre, plus the pivot (mx, my), with no
 *    projection. Every other 0x44 sprite is projected from its coordinate.
 *  - The class's two own slots set exactly those: setPosition (+0x0BC)
 *    stores `screenPos`, setPivotAnchor (+0x0C0) moves (mx, my). Its
 *    attachToParent override attaches with a zero 3-D offset and hands its
 *    third argument to setPosition instead; the callers pass pairs like
 *    (-70, -60) and (-100, -60) (sCardIconPos, sItemListPanelPos), percentages.
 *
 * The ctor chains to Sprite's first (GetSpriteMethods()->ctor with abr 0 and
 * a NULL fourth argument), and CharSprite's ctor chains to this one, so the
 * id tree (0x44 -> 0x144 -> 0x1144) is the ctor chain. Two classes derive
 * from it (`typeviews.py --tree`): CharSprite (0x1144, one 8x8 font
 * character, include/CharSprite.h, own fields from +0x0A8), which expands
 * these macros, and TextRow (0x11144, below CharSprite, include/TextRow.h),
 * which expands CharSprite's.
 *
 * Not settled here: the ctor occupant returns nothing where the slot, from
 * SceneNode, returns `void *`; and the attachToParent override's third
 * argument is a ScreenSpritePos where the slot, SceneNode's, types it
 * `LongVec3 *offset`, so a call through the slot with a position needs a
 * cast. Both slots are SceneNode's to change.
 *
 * The object is 0xA8 bytes (New_ScreenSprite).
 */

typedef struct ScreenSprite ScreenSprite;
typedef struct ScreenSpriteMethods ScreenSpriteMethods;
typedef struct ScreenSpritePos ScreenSpritePos;

/* ScreenSprite's class id (gScreenSpriteMethods word +0x000). Three nibbles,
 * so `(header & 0xFFF) == SCREENSPRITE_CLASS_ID` is its is-kind-of test
 * (Viewport__DrawNode). */
#define SCREENSPRITE_CLASS_ID 0x144

/* A screen position: percent of half the screen width/height, from the
 * centre (Viewport__DrawNode). */
struct ScreenSpritePos {
    s32 x;
    s32 y;
};

/* setPivotAnchor's argument: where (mx, my) moves on the sprite's cell.
 * LEFT/RIGHT move mx only, TOP/BOTTOM my only; any other value does nothing. */
enum ScreenSpriteAnchor {
    SCREENSPRITE_ANCHOR_CENTRE = 0, /* (w / 2, h / 2) */
    SCREENSPRITE_ANCHOR_LEFT = 1,   /* mx = 0 */
    SCREENSPRITE_ANCHOR_RIGHT = 2,  /* mx = w */
    SCREENSPRITE_ANCHOR_TOP = 3,    /* my = 0 */
    SCREENSPRITE_ANCHOR_BOTTOM = 4  /* my = h */
};

/* Sprite's slots, then this class's own. `tools/classtable.py
 * gScreenSpriteMethods --vs gSpriteMethods` lists the overrides of the
 * inherited ones (ScreenSprite__ScreenSprite, ScreenSprite__Reset,
 * ScreenSprite__AttachToParent). */
/* clang-format off */
#define SCREENSPRITE_SLOTS(Self, CtorParams)                                                       \
    SPRITE_SLOTS(Self, CtorParams);                                                                \
    /* +0x0BC */ void (*setPosition)(Self *self, ScreenSpritePos *pos); /* ScreenSprite__SetPosition; gTextRowMethods: TextRow__SetPosition */ \
    /* +0x0C0 */ void (*setPivotAnchor)(Self *self, u32 anchor)         /* ScreenSprite__SetPivotAnchor: 0 centre, 1 left, 2 right, 3 top, 4 bottom */
/* clang-format on */

/* clang-format off */
#define SCREENSPRITE_FIELDS(Methods)                                                               \
    SPRITE_FIELDS(Methods);                                                                        \
    /* +0x0A0 */ ScreenSpritePos screenPos /* setPosition; Viewport__DrawNode places the sprite from it. The object is 0xA8 bytes (New_ScreenSprite): CharSprite's own fields start at +0x0A8 */
/* clang-format on */

struct ScreenSpriteMethods {
    SCREENSPRITE_SLOTS(ScreenSprite,
                       (ScreenSprite * self, void *texture, SpriteRect *rect, s32 resetWord));
};

struct ScreenSprite {
    SCREENSPRITE_FIELDS(ScreenSpriteMethods);
};

extern ScreenSpriteMethods gScreenSpriteMethods;
extern ScreenSpriteMethods *GetScreenSpriteMethods(void); /* returns &gScreenSpriteMethods */

/* The class's own methods, in address order. */
ScreenSprite *New_ScreenSprite(void *texture, SpriteRect *rect, s32 resetWord);
void ScreenSprite__ScreenSprite(ScreenSprite *self, void *texture, SpriteRect *rect, s32 resetWord);
void ScreenSprite__Reset(ScreenSprite *self);
void ScreenSprite__AttachToParent(ScreenSprite *self, SceneNode *parent, ScreenSpritePos *pos);
void ScreenSprite__SetPosition(ScreenSprite *self, ScreenSpritePos *pos);
void ScreenSprite__SetPivotAnchor(ScreenSprite *self, u32 anchor);

#endif
