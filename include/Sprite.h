#ifndef SPRITE_H
#define SPRITE_H

#include "Class6B5CC.h"

/*
 * Sprite -- the base of the game's 2-D sprites (class id 0x44, method table
 * gSpriteMethods): a Class6B5CC subclass that draws a libgs GsSPRITE embedded
 * at +0x064 instead of its GsDOBJ2 model. Viewport__DrawNode hands
 * `self + 0x64` to GsSortSprite for every class whose id's low byte is 0x44:
 * screen-space for 0x144 (ScreenSprite and below), otherwise world-space,
 * projected from the inherited coordinate. Methods in src/code_322b4.c; four
 * classes derive from it (`typeviews.py --tree`): ScreenSprite (0x144, the
 * screen-space sprite, include/ScreenSprite.h), CharSprite (0x1144, one 8x8
 * font character, include/CharSprite.h), TextRow (0x11144) and gClass879C4Methods (0x1F44,
 * class_3bb8c_p/q/t).
 *
 * The texture is bound by reset (+0x040), which the ctor calls with its own
 * arguments: `texture` is a TimImage (code_2bb9c.c: its GsIMAGE is at +0x02C,
 * which reset keeps in `image`), `rect` the texture cell, and InitGsSprite
 * fills the GsSPRITE from the two: tpage from GetTPage(pmode & 3, abr, px,
 * py), clut from the image, w/h and u/v from the cell, rgb 0x80, the pivot
 * (mx, my) at the cell's centre and scale 1.0.
 *
 * The inherited slots that act on the GsDOBJ2 in Class6B5CC act on the
 * GsSPRITE here: setDisplay/setSemiTrans/setSemiTransRate edit
 * sprite.attribute (bit 31 inverted, bit 30, bits 28-29: GsDOFF, GsALON,
 * the semitrans rate), updateRotation writes sprite.rotate.
 *
 * Not settled here: the ctor passes reset FIVE arguments after self and
 * reset reads three, where Class6B5CC's reset takes none (Class879C4's
 * ctor, round 87, is void: its +0x040 occupant sets no $v0). The slot keeps
 * Class6B5CC's type (it is that class's to change), so a C call through it
 * with arguments needs a cast until Class6B5CC's slot is retyped.
 *
 * The object is 0xA0 bytes (New_Sprite).
 */

typedef struct Sprite Sprite;
typedef struct SpriteMethods SpriteMethods;
typedef struct SpriteRgb SpriteRgb;
typedef struct SpriteRect SpriteRect;
typedef struct SpriteGs SpriteGs;

/* The GsIMAGE a TimImage describes its TIM into (defined in code_2bb9c.c);
 * only its address is kept here. */
struct GsIMAGE;

/* A 3-byte colour. All-s8 members give it alignment 1, which is what makes
 * Sprite__SetColor's whole-struct copy compile to lb,lb,lb then sb,sb,sb
 * (DECOMPILATION_LEARNINGS, the 3-byte all-s8 struct idiom). */
struct SpriteRgb {
    s8 r, g, b;
};

/* A texture cell: 16-bit origin in the texture page, 32-bit extent. The
 * ctor's `rect`, copied to `rect` by reset; gClass879C4Cells (Class879C4's two
 * cells) and D_8006ED40 (CharSprite's 8x8 cell origin) are these. */
struct SpriteRect {
    /* +0x000 */ u16 u;
    /* +0x002 */ u16 v;
    /* +0x004 */ s32 w;
    /* +0x008 */ s32 h;
};

/* libgs GsSPRITE, 0x24 bytes (LIBGS.H), with r,g,b as one SpriteRgb. */
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
    /* +0x014 */ SpriteRgb rgb;
    /* +0x017 */ u8 pad17;
    /* +0x018 */ s16 mx;
    /* +0x01A */ s16 my;
    /* +0x01C */ s16 scalex;
    /* +0x01E */ s16 scaley;
    /* +0x020 */ s32 rotate;    /* updateRotation (Sprite__UpdateRotation): 4096 per degree */
};

/* Class6B5CC's slots, then this class's own. Occupants in gSpriteMethods
 * named at each own slot; `tools/classtable.py gSpriteMethods --vs
 * gClass6B5CCMethods` lists the overrides of the inherited ones (Sprite__Sprite,
 * Sprite__Reset, Sprite__UpdateRotation, Sprite__SetDisplay,
 * Sprite__SetSemiTrans, Sprite__SetSemiTransRate, Sprite__Update). */
#define SPRITE_SLOTS(Self, CtorParams)                                                             \
    CLASS6B5CC_SLOTS(Self, CtorParams);                                                            \
    /* +0x0B8 */ void (*setColor)(Self *self, SpriteRgb *rgb) /* Sprite__SetColor; gTextRowMethods: TextRow__SetColor */

#define SPRITE_FIELDS(Methods)                                                                     \
    CLASS6B5CC_FIELDS(Methods);                                                                    \
    /* +0x044 */ u8 pad44[4];                                                                      \
    /* +0x048 */ struct GsIMAGE *image; /* reset: &texture->tim (the TimImage's +0x02C) */          \
    /* +0x04C */ SpriteRect rect;       /* reset: a copy of the ctor's cell */                     \
    /* +0x058 */ s32 unk58;             /* zeroed by reset; Class879C4__UpdateScale: non-zero scales unk5C/unk60 instead of the sprite */ \
    /* +0x05C */ s32 unk5C;             /* Class879C4__UpdateScale: times the x ratio when unk58 != 0 */ \
    /* +0x060 */ s32 unk60;             /* Class879C4__UpdateScale: times the y ratio when unk58 != 0 */ \
    /* +0x064 */ SpriteGs sprite;       /* InitGsSprite fills it; Viewport__DrawNode sorts it */    \
    /* +0x088 */ u8 pad88[0xA0 - 0x88]  /* the object is 0xA0 bytes (New_Sprite); ScreenSprite's own fields start at +0x0A0 */

struct SpriteMethods {
    SPRITE_SLOTS(Sprite, (Sprite *self, void *texture, s32 abr, SpriteRect *rect, void *arg4, s32 arg5));
};

struct Sprite {
    SPRITE_FIELDS(SpriteMethods);
};

extern SpriteMethods gSpriteMethods;
extern SpriteMethods *GetSpriteMethods(void); /* returns &gSpriteMethods */

/* The class's own methods, in address order. */
Sprite *New_Sprite(void *texture, s32 abr, SpriteRect *rect, void *arg3, s32 arg4);
void *Sprite__Sprite(Sprite *self, void *texture, s32 abr, SpriteRect *rect, void *arg4, s32 arg5);
void Sprite__Reset(Sprite *self, void *texture, s32 abr, SpriteRect *rect);
void InitGsSprite(SpriteGs *sprite, s32 abr, SpriteRect *rect, struct GsIMAGE *image);
void Sprite__UpdateRotation(Sprite *self, s32 set, WholeFrac_d294 *table);
s32 Sprite__SetDisplay(Sprite *self, s32 on);
s32 Sprite__SetSemiTrans(Sprite *self, s32 on);
s32 Sprite__SetSemiTransRate(Sprite *self, s32 rate);
void Sprite__Update(Sprite *self, void *sender, s32 event);
void Sprite__SetColor(Sprite *self, SpriteRgb *rgb);

#endif
