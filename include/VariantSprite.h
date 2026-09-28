#ifndef VARIANTSPRITE_H
#define VARIANTSPRITE_H

#include "Sprite.h"

/*
 * VariantSprite -- a world-space sprite (Sprite, include/Sprite.h) that comes
 * in two variants, and the variant is the whole of what it adds: `variant`,
 * 0 or 1, picks the texture cell the Sprite ctor binds (sVariantSpriteCells,
 * two adjacent 16x16 cells) and the CLUT the reset slot then points the
 * GsSPRITE at (sVariantSpriteClutX/Y, two adjacent 16-colour rows at
 * VRAM y 0x1FF). Class id 0x1F44: its low 12 bits are not 0x144, so
 * Viewport__DrawNode projects it from its coordinate like any Sprite, not
 * the screen-space path ScreenSprite takes. Method table
 * gVariantSpriteMethods, getter GetVariantSpriteMethods. Methods in
 * src/world/ObjMStyleActor.c, New_VariantSprite through the getter. No class
 * derives from it.
 *
 * Who makes it: only StyleEffect (include/StyleEffect.h), five per instance of
 * kinds 2 and 3, in StyleEffect__SpawnSprites, as New_VariantSprite(variant,
 * 0, sStyleEffectTim) with `variant` 0 on every path, so variant 1's cell and
 * CLUT are never selected. StyleEffect drives them only through inherited
 * slots: attachToParent, setColor, updateScale, setDisplay, the semitrans
 * pair, and sprite.rotate directly.
 *
 * Lifecycle.
 *   ctor(variant, resetArg, texture)  Sprite's ctor with texture, abr 0,
 *                  the variant's cell and resetArg (which Sprite's ctor hands
 *                  on to Sprite__Reset, which does not read it; the one
 *                  caller passes 0); this table; unkA4 cleared; then the
 *                  reset slot with the variant (below).
 *   +0x040 reset   VariantSprite__SetVariantClut: records `variant` and sets
 *                  sprite.cx/cy from the variant's CLUT row, replacing the
 *                  CLUT Sprite's reset took from the texture.
 *   +0x048 updateScale  VariantSprite__UpdateScale: two s16 num/den ratios
 *                  (x, y) into sprite.scalex/scaley as 20.12 fixed point,
 *                  or, while Sprite's accumulateScale is set, into accumScaleX/Y.
 *   +0x098 update  VariantSprite__Update, empty, as Sprite__Update is (its
 *                  own copy in ROM).
 *
 * Two overrides take a different parameter list from the slot they fill, so
 * the slot keeps the inherited type:
 *   +0x040 reset   takes a variant where SceneNode's slot takes none and
 *                  returns nothing where it returns `void *`; the ctor calls
 *                  it through VariantSpriteResetFn and, ending in that call,
 *                  returns nothing itself (ScreenSprite's ctor is the same).
 *   +0x048 updateScale  reads its table as two ratio pairs where
 *                  SceneNode's reads three.
 *
 * The object is 0xA8 bytes (New_VariantSprite); its own fields start at
 * +0x0A0.
 */

typedef struct VariantSprite VariantSprite;
typedef struct VariantSpriteMethods VariantSpriteMethods;

/* Sprite's slots, then this class's own three, which hold empty functions
 * and which nothing calls. `tools/classtable.py gVariantSpriteMethods --vs
 * gSpriteMethods` lists the inherited slots it overrides (see the banner). */
/* clang-format off */
#define VARIANTSPRITE_SLOTS(Self, CtorParams)                                                      \
    SPRITE_SLOTS(Self, CtorParams);                                                                \
    /* +0x0BC */ void (*slotBC)(void); /* VariantSprite__NoOpSlotBC, empty; never called */        \
    /* +0x0C0 */ void (*slotC0)(void); /* VariantSprite__NoOpSlotC0, empty; never called */        \
    /* +0x0C4 */ void (*slotC4)(void)  /* VariantSprite__NoOpSlotC4, empty; never called */
/* clang-format on */

/* clang-format off */
#define VARIANTSPRITE_FIELDS(Methods)                                                              \
    SPRITE_FIELDS(Methods);                                                                        \
    /* +0x0A0 */ s32 variant; /* the ctor's first argument, 0 or 1 (SetVariantClut); never read */ \
    /* +0x0A4 */ s32 unkA4    /* zeroed by the ctor; no other accessor. The object is 0xA8 bytes (New_VariantSprite) */
/* clang-format on */

struct VariantSpriteMethods {
    VARIANTSPRITE_SLOTS(VariantSprite,
                        (VariantSprite * self, s32 variant, void *resetArg, void *texture));
};

struct VariantSprite {
    VARIANTSPRITE_FIELDS(VariantSpriteMethods);
};

/* The reset slot's occupant as the ctor calls it (see the banner). */
typedef void (*VariantSpriteResetFn)(VariantSprite *self, s32 variant);

extern VariantSpriteMethods gVariantSpriteMethods;
extern VariantSpriteMethods *GetVariantSpriteMethods(void); /* returns &gVariantSpriteMethods */

/* The class's own methods, in address order. */
VariantSprite *New_VariantSprite(s32 variant, void *resetArg, void *texture);
void VariantSprite__VariantSprite(VariantSprite *self, s32 variant, void *resetArg, void *texture);
void VariantSprite__SetVariantClut(VariantSprite *self, s32 variant);
void VariantSprite__UpdateScale(VariantSprite *self, s32 set, Ratio16 *ratios);
void VariantSprite__Update(VariantSprite *self, void *sender, s32 event);
void VariantSprite__NoOpSlotBC(void);
void VariantSprite__NoOpSlotC0(void);
void VariantSprite__NoOpSlotC4(void);

#endif
