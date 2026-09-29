#ifndef SPRITE_H
#define SPRITE_H

#include "scene_node.h"

/**
 * @file sprite.h
 * @brief Sprite, the base class of the game's 2-D sprites, and the texture
 *        cell and GsSPRITE layouts it draws with.
 *
 * Viewport__DrawNode hands a Sprite's embedded GsSPRITE (`sprite`, +0x064)
 * to GsSortSprite for every class whose id's low byte is 0x44: screen-space
 * for 0x144 (ScreenSprite and below), otherwise world-space, projected from
 * the inherited coordinate.
 *
 * The texture is bound by reset (+0x040), which the ctor calls with its own
 * arguments: `texture` is a TimImage (include/tim_image.h: its GsIMAGE is at
 * +0x02C, which reset keeps in `image`), `rect` the texture cell, and
 * InitGsSprite fills the GsSPRITE from the two.
 *
 * The inherited slots that act on the GsDOBJ2 in SceneNode act on the
 * GsSPRITE here: setDisplay/setSemiTransOn/setSemiTransRate edit
 * sprite.attribute (bit 31 inverted, bit 30, bits 28-29: GsDOFF, GsALON,
 * the semitransparency rate), updateRotation writes sprite.rotate.
 *
 * Not settled here: the ctor passes reset FIVE arguments after self and
 * Sprite__Reset reads three, where SceneNode's reset slot takes none. The
 * slot keeps SceneNode's type (it is that class's to change), so a C call
 * through it with arguments needs a cast (the ctor's SpriteResetFn) until
 * SceneNode's slot is retyped.
 *
 * `image` and InitGsSprite's `tim` are <libgs.h>'s GsIMAGE, so an includer
 * takes Sony's headers first (`common.h`, <libgte.h>, <libgpu.h>, <libgs.h>).
 */

typedef struct Sprite Sprite;
typedef struct SpriteMethods SpriteMethods;
typedef struct SpriteRect SpriteRect;
typedef struct SpriteGs SpriteGs;

/** Sprite's class id (gSpriteMethods word +0x000). Two nibbles, so
 * `(header & CLASS_ID_LEVEL2_MASK) == SPRITE_CLASS_ID` is its is-kind-of test, true for
 * every subclass too: ScreenSprite (0x144) and VariantSprite (0x1F44)
 * (Viewport__DrawNode). */
#define SPRITE_CLASS_ID 0x44

/**
 * A texture cell: 16-bit origin in the texture page, 32-bit extent. The
 * ctor's `rect`, copied to `rect` by reset; sVariantSpriteCells (VariantSprite's two
 * cells) and sCharSpriteCellRect (CharSprite's 8x8 cell origin) are these.
 */
struct SpriteRect {
    /* +0x000 */ u16 u; /**< left edge in the texture page */
    /* +0x002 */ u16 v; /**< top edge in the texture page */
    /* +0x004 */ s32 w; /**< width in texels */
    /* +0x008 */ s32 h; /**< height in texels */
};

/**
 * libgs's GsSPRITE, 0x24 bytes, field for field, with r, g, b as one
 * ColorRgb so Sprite__SetColor copies the colour whole, and `attribute` a
 * u32 so GetSetBitField takes its address as it is. Sony's own GsSPRITE
 * would cost a cast at each of those four sites to save the two in
 * Viewport__DrawNode.
 */
struct SpriteGs {
    /* +0x000 */ u32 attribute; /**< GetSetBitField's word: bit 31 GsDOFF, 30 GsALON, 28-29 rate, 24-25 colour mode */
    /* +0x004 */ s16 x;        /**< screen x (Viewport__DrawNode sets it each draw) */
    /* +0x006 */ s16 y;        /**< screen y */
    /* +0x008 */ u16 w;        /**< width, from the cell */
    /* +0x00A */ u16 h;        /**< height, from the cell */
    /* +0x00C */ u16 tpage;    /**< texture page (GetTPage) */
    /* +0x00E */ u8 u;         /**< cell's left edge in the page */
    /* +0x00F */ u8 v;         /**< cell's top edge in the page */
    /* +0x010 */ s16 cx;       /**< CLUT x, from the image */
    /* +0x012 */ s16 cy;       /**< CLUT y, from the image */
    /* +0x014 */ ColorRgb rgb; /**< brightness per channel; 128 is neutral */
    /* +0x017 */ u8 pad17;
    /* +0x018 */ s16 mx;     /**< pivot x within the cell */
    /* +0x01A */ s16 my;     /**< pivot y within the cell */
    /* +0x01C */ s16 scalex; /**< x scale, ONE = 1.0 */
    /* +0x01E */ s16 scaley; /**< y scale, ONE = 1.0 */
    /* +0x020 */ s32 rotate; /**< rotation, 4096 per degree (Sprite__UpdateRotation) */
};

/* Bit positions in SpriteGs.attribute, as libgs.h documents GsSPRITE's. */
#define SPRITE_ATTR_MODE_SHIFT 24 /**< colour mode, 2 bits: TIM_PMODE_DEPTH_MASK */
#define SPRITE_ATTR_RATE_SHIFT 28 /**< semitransparency rate, 2 bits (GsAZERO..GsATHREE) */
#define SPRITE_ATTR_ALON_SHIFT 30 /**< GsALON: semitransparency on */
#define SPRITE_ATTR_DOFF_SHIFT 31 /**< GsDOFF: display off */

/** The r, g, b InitGsSprite sets: 128 draws the texture at its own brightness. */
#define SPRITE_RGB_NEUTRAL 128

/**
 * SceneNode's slots, then Sprite's own, for SpriteMethods and every
 * subclass's table to expand first. gSpriteMethods overrides the ctor,
 * reset, updateRotation, setDisplay, setSemiTransOn, setSemiTransRate and
 * update with the Sprite__ methods below.
 */
/* clang-format off */
#define SPRITE_SLOTS(Self, CtorParams)                                                             \
    SCENENODE_SLOTS(Self, CtorParams);                                                            \
    /* +0x0B8 */ void (*setColor)(Self *self, ColorRgb *rgb) /* Sprite__SetColor; gTextRowMethods: TextRow__SetColor */
/* clang-format on */

/**
 * SceneNode's fields, then Sprite's own, for Sprite and every subclass's
 * object to expand first.
 */
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

/** Sprite's method table: SPRITE_SLOTS with Sprite's ctor parameters. */
struct SpriteMethods {
    SPRITE_SLOTS(Sprite, (Sprite * self, void *texture, s32 abr, SpriteRect *rect, void *resetArg,
                          s32 resetWord));
};

/**
 * Sprite: a SceneNode that draws a libgs GsSPRITE embedded at +0x064 instead
 * of its GsDOBJ2 model. Class id 0x44 (SPRITE_CLASS_ID), table gSpriteMethods,
 * parent SceneNode; methods in src/graphics/sprite.c. Four classes derive
 * from it: ScreenSprite (0x144, include/screen_sprite.h), CharSprite
 * (0x1144, include/char_sprite.h), TextRow (0x11144) and VariantSprite
 * (0x1F44, include/variant_sprite.h). New_Sprite allocates 0xA0 bytes; the
 * ctor binds the texture through reset.
 */
struct Sprite {
    SPRITE_FIELDS(SpriteMethods);
};

/** Sprite's method table (class id 0x44). */
extern SpriteMethods gSpriteMethods;

/**
 * @brief The Sprite method table.
 * @return &gSpriteMethods.
 */
extern SpriteMethods *GetSpriteMethods(void);

/**
 * @brief Allocates a Sprite and runs its ctor through the table.
 * @param texture The TimImage to draw from.
 * @param abr The semitransparency rate for the texture page (GetTPage).
 * @param rect The texture cell.
 * @param resetArg Handed on to reset; Sprite's does not read it.
 * @param resetWord Handed on to reset; Sprite's does not read it.
 * @return The new Sprite, or NULL when the allocation fails.
 */
Sprite *New_Sprite(void *texture, s32 abr, SpriteRect *rect, void *resetArg, s32 resetWord);

/**
 * @brief Constructor (slot +0x008): SceneNode's ctor, installs gSpriteMethods,
 *        then calls reset with every argument.
 * @param self The sprite.
 * @param texture The TimImage to draw from.
 * @param abr The semitransparency rate for the texture page.
 * @param rect The texture cell.
 * @param resetArg Handed on to reset.
 * @param resetWord Handed on to reset.
 */
void Sprite__Sprite(Sprite *self, void *texture, s32 abr, SpriteRect *rect, void *resetArg, s32 resetWord);

/**
 * @brief Reset (slot +0x040): binds the texture and cell and rebuilds the
 *        GsSPRITE (InitGsSprite); clears accumulateScale.
 * @param self The sprite.
 * @param texture The TimImage; its GsIMAGE becomes `image`.
 * @param abr The semitransparency rate for the texture page.
 * @param rect The texture cell, copied to `rect`.
 */
void Sprite__Reset(Sprite *self, void *texture, s32 abr, SpriteRect *rect);

/**
 * @brief Fills a GsSPRITE from a texture image and a cell: colour mode from
 *        the image's pmode, tpage from GetTPage(mode, abr, px, py), the CLUT
 *        from the image, size and u, v from the cell, the pivot at the cell's
 *        centre, neutral colour (SPRITE_RGB_NEUTRAL), scale 1.0, no rotation
 *        and position (0, 0).
 * @param sprite The GsSPRITE to fill.
 * @param abr The semitransparency rate for the texture page.
 * @param rect The texture cell.
 * @param tim The texture's GsIMAGE.
 */
void InitGsSprite(SpriteGs *sprite, s32 abr, SpriteRect *rect, GsIMAGE *tim);

/**
 * @brief updateRotation (slot +0x044): a sprite turns about z only. Sets or
 *        adds table[2], in degrees, to the GsSPRITE's rotate (4096 per
 *        degree); table[0] and table[1] are not read.
 * @param self The sprite.
 * @param set Non-zero to store the angle, 0 to add it.
 * @param table Ratio16[3]; entry 2 is the angle in degrees.
 */
void Sprite__UpdateRotation(Sprite *self, s32 set, Ratio16 *table);

/**
 * @brief setDisplay (slot +0x060): shows or hides the sprite; writes !on to
 *        the GsSPRITE's GsDOFF bit.
 * @param self The sprite.
 * @param on Non-zero to display it.
 * @return Non-zero when it was displayed before.
 */
s32 Sprite__SetDisplay(Sprite *self, s32 on);

/**
 * @brief setSemiTransOn (slot +0x064): semitransparency on or off (GsALON).
 * @param self The sprite.
 * @param on Non-zero for on.
 * @return The old GsALON bit.
 */
s32 Sprite__SetSemiTrans(Sprite *self, s32 on);

/**
 * @brief setSemiTransRate (slot +0x068): the semitransparency rate, 2 bits.
 * @param self The sprite.
 * @param rate The new rate, 0..3 (GsAZERO..GsATHREE).
 * @return The old rate.
 */
s32 Sprite__SetSemiTransRate(Sprite *self, s32 rate);

/**
 * @brief update (slot +0x098), a FrameClock's event: empty. Every sprite
 *        class's table but VariantSprite's holds it.
 * @param self The sprite.
 * @param sender The FrameClock.
 * @param event Its event.
 */
void Sprite__Update(Sprite *self, void *sender, s32 event);

/**
 * @brief setColor (slot +0x0B8): copies the colour into the GsSPRITE's r, g, b.
 *        Shared by every sprite class's table but TextRow's.
 * @param self The sprite.
 * @param rgb The colour; 128 per channel is neutral.
 */
void Sprite__SetColor(Sprite *self, ColorRgb *rgb);

#endif
