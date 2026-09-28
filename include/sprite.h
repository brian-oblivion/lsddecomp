#ifndef SPRITE_H
#define SPRITE_H

#include "scene_node.h"

/*
 * Sprite -- the base of the game's 2-D sprites (class id 0x44, method table
 * gSpriteMethods): a SceneNode subclass that draws a libgs GsSPRITE embedded
 * at +0x064 instead of its GsDOBJ2 model. Viewport__DrawNode hands
 * `self + 0x64` to GsSortSprite for every class whose id's low byte is 0x44:
 * screen-space for 0x144 (ScreenSprite and below), otherwise world-space,
 * projected from the inherited coordinate. Methods in src/graphics/sprite.c; four
 * classes derive from it (`typeviews.py --tree`): ScreenSprite (0x144, the
 * screen-space sprite, include/screen_sprite.h), CharSprite (0x1144, one 8x8
 * font character, include/char_sprite.h), TextRow (0x11144) and gVariantSpriteMethods (0x1F44,
 * src/world/dream_scene.c).
 *
 * The texture is bound by reset (+0x040), which the ctor calls with its own
 * arguments: `texture` is a TimImage (include/tim_image.h: its GsIMAGE is at +0x02C,
 * which reset keeps in `image`), `rect` the texture cell, and InitGsSprite
 * fills the GsSPRITE from the two: tpage from GetTPage(pmode & 3, abr, px,
 * py), clut from the image, w/h and u/v from the cell, rgb 0x80, the pivot
 * (mx, my) at the cell's centre and scale 1.0.
 *
 * The inherited slots that act on the GsDOBJ2 in SceneNode act on the
 * GsSPRITE here: setDisplay/setSemiTransOn/setSemiTransRate edit
 * sprite.attribute (bit 31 inverted, bit 30, bits 28-29: GsDOFF, GsALON,
 * the semitrans rate), updateRotation writes sprite.rotate.
 *
 * Not settled here: the ctor passes reset FIVE arguments after self and
 * reset reads three, where SceneNode's reset takes none (VariantSprite's
 * ctor is void: its +0x040 occupant sets no $v0). The slot keeps
 * SceneNode's type (it is that class's to change), so a C call through it
 * with arguments needs a cast until SceneNode's slot is retyped.
 *
 * `image` and InitGsSprite's `tim` are <libgs.h>'s GsIMAGE, so an includer
 * takes Sony's headers first (`common.h`, <libgte.h>, <libgpu.h>, <libgs.h>).
 *
 * The object is 0xA0 bytes (New_Sprite).
 */

typedef struct Sprite Sprite;
typedef struct SpriteMethods SpriteMethods;
typedef struct SpriteRect SpriteRect;
typedef struct SpriteGs SpriteGs;

/* Sprite's class id (gSpriteMethods word +0x000). Two nibbles, so
 * `(header & 0xFF) == SPRITE_CLASS_ID` is its is-kind-of test, true for
 * every subclass too: ScreenSprite (0x144) and VariantSprite (0x1F44)
 * (Viewport__DrawNode). */
#define SPRITE_CLASS_ID 0x44

/* A texture cell: 16-bit origin in the texture page, 32-bit extent. The
 * ctor's `rect`, copied to `rect` by reset; sVariantSpriteCells (VariantSprite's two
 * cells) and sCharSpriteCellRect (CharSprite's 8x8 cell origin) are these. */
struct SpriteRect {
    /* +0x000 */ u16 u;
    /* +0x002 */ u16 v;
    /* +0x004 */ s32 w;
    /* +0x008 */ s32 h;
};

/* libgs GsSPRITE, 0x24 bytes, field for field, with r,g,b as one ColorRgb.
 * Kept local rather than Sony's GsSPRITE: Sprite__SetColor copies the colour
 * as one ColorRgb (the whole-struct copy is what matches), and
 * GetSetBitField takes `attribute` as a u32 *, where Sony's is unsigned long;
 * Sony's type would cost a cast at each of those four sites to save the two
 * in Viewport__DrawNode. */
struct SpriteGs {
    /* +0x000 */ u32 attribute; /* GetSetBitField's word: bit 31 GsDOFF, 30 GsALON, 28-29 rate, 24-25 colour mode */
    /* +0x004 */ s16 x;
    /* +0x006 */ s16 y;
    /* +0x008 */ u16 w;
    /* +0x00A */ u16 h;
    /* +0x00C */ u16 tpage;
    /* +0x00E */ u8 u;
    /* +0x00F */ u8 v;
    /* +0x010 */ s16 cx;
    /* +0x012 */ s16 cy;
    /* +0x014 */ ColorRgb rgb;
    /* +0x017 */ u8 pad17;
    /* +0x018 */ s16 mx;
    /* +0x01A */ s16 my;
    /* +0x01C */ s16 scalex;
    /* +0x01E */ s16 scaley;
    /* +0x020 */ s32 rotate; /* updateRotation (Sprite__UpdateRotation): 4096 per degree */
};

/* Bit positions in SpriteGs.attribute, as libgs.h documents GsSPRITE's. */
#define SPRITE_ATTR_MODE_SHIFT 24 /* colour mode, 2 bits: the TIM's pmode & 0x3 */
#define SPRITE_ATTR_RATE_SHIFT 28 /* semitransparency rate, 2 bits (GsAZERO..GsATHREE) */
#define SPRITE_ATTR_ALON_SHIFT 30 /* GsALON: semitransparency on */
#define SPRITE_ATTR_DOFF_SHIFT 31 /* GsDOFF: display off */

/* The r, g, b InitGsSprite sets: 128 draws the texture at its own brightness. */
#define SPRITE_RGB_NEUTRAL 128

/* SceneNode's slots, then this class's own. Occupants in gSpriteMethods
 * named at each own slot; `tools/classtable.py gSpriteMethods --vs
 * gSceneNodeMethods` lists the overrides of the inherited ones (Sprite__Sprite,
 * Sprite__Reset, Sprite__UpdateRotation, Sprite__SetDisplay,
 * Sprite__SetSemiTrans, Sprite__SetSemiTransRate, Sprite__Update). */
/* clang-format off */
#define SPRITE_SLOTS(Self, CtorParams)                                                             \
    SCENENODE_SLOTS(Self, CtorParams);                                                            \
    /* +0x0B8 */ void (*setColor)(Self *self, ColorRgb *rgb) /* Sprite__SetColor; gTextRowMethods: TextRow__SetColor */
/* clang-format on */

/* clang-format off */
#define SPRITE_FIELDS(Methods)                                                                     \
    SCENENODE_FIELDS(Methods);                                                                    \
    /* +0x044 */ u8 pad44[4];                                                                      \
    /* +0x048 */ GsIMAGE *image;        /* reset: &texture->tim (the TimImage's +0x02C) */          \
    /* +0x04C */ SpriteRect rect;       /* reset: a copy of the ctor's cell */                     \
    /* +0x058 */ s32 accumulateScale;   /* zeroed by reset; VariantSprite__UpdateScale: non-zero scales accumScaleX/Y instead of the sprite */ \
    /* +0x05C */ s32 accumScaleX;       /* VariantSprite__UpdateScale: times the x ratio (20.12) while accumulateScale != 0 */ \
    /* +0x060 */ s32 accumScaleY;       /* ... and this times the y ratio; no other accessor of either */ \
    /* +0x064 */ SpriteGs sprite;       /* InitGsSprite fills it; Viewport__DrawNode sorts it */    \
    /* +0x088 */ u8 pad88[0xA0 - 0x88]  /* the object is 0xA0 bytes (New_Sprite); ScreenSprite's own fields start at +0x0A0 */
/* clang-format on */

struct SpriteMethods {
    SPRITE_SLOTS(Sprite, (Sprite * self, void *texture, s32 abr, SpriteRect *rect, void *resetArg,
                          s32 resetWord));
};

struct Sprite {
    SPRITE_FIELDS(SpriteMethods);
};

extern SpriteMethods gSpriteMethods;
extern SpriteMethods *GetSpriteMethods(void); /* returns &gSpriteMethods */

/* The class's own methods, in address order. */
Sprite *New_Sprite(void *texture, s32 abr, SpriteRect *rect, void *resetArg, s32 resetWord);
void *Sprite__Sprite(Sprite *self, void *texture, s32 abr, SpriteRect *rect, void *resetArg,
                     s32 resetWord);
void Sprite__Reset(Sprite *self, void *texture, s32 abr, SpriteRect *rect);
void InitGsSprite(SpriteGs *sprite, s32 abr, SpriteRect *rect, GsIMAGE *tim);
void Sprite__UpdateRotation(Sprite *self, s32 set, Ratio16 *table);
s32 Sprite__SetDisplay(Sprite *self, s32 on);
s32 Sprite__SetSemiTrans(Sprite *self, s32 on);
s32 Sprite__SetSemiTransRate(Sprite *self, s32 rate);
void Sprite__Update(Sprite *self, void *sender, s32 event);
void Sprite__SetColor(Sprite *self, ColorRgb *rgb);

#endif
