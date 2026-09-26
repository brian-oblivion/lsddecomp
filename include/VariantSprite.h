#ifndef VARIANTSPRITE_H
#define VARIANTSPRITE_H

#include "Sprite.h"

/*
 * VariantSprite -- a two-variant world-space sprite (class id 0x1F44, method
 * table gVariantSpriteMethods): Sprite's subclass (its ctor chains to
 * GetSpriteMethods()->ctor first). Methods in src/class_3bb8c_p.c (New_,
 * ctor), src/class_3bb8c_q.c (SetVariantClut, UpdateScale) and
 * src/class_3bb8c_t.c (four empty leaves, the getter). No class derives
 * from it. Class876FC (class_3bb8c_s.c) builds five per instance
 * (Class876FC__SpawnSprites) and drives them through inherited slots only.
 *
 * `variant` (0 or 1, the ctor's first argument) picks both the texture cell
 * the Sprite ctor binds (gVariantSpriteCells[variant]) and, through the reset
 * slot the ctor calls last, the CLUT row (gVariantSpriteClutX/Y[variant]).
 * The name stays the table's address: what the sprites are in the game is
 * not established.
 *
 * Settled from the bytes (round 87, track 4):
 *  - +0x040 reset: the occupant, VariantSprite__SetVariantClut, takes a
 *    variant where the slot (SceneNode's) takes none, so the ctor calls it
 *    through VariantSpriteResetFn (a cast, no code). It returns nothing, and
 *    the ctor ends in that call without setting $v0, so the ctor returns
 *    nothing either (ScreenSprite's precedent), where the slot, from
 *    SceneNode, returns `void *`. New_VariantSprite does not read it.
 *  - +0x048 updateScale: the occupant reads `table` as two s16 num/den
 *    ratio pairs (x, y) where SceneNode's reads three.
 *
 * The object is 0xA8 bytes (New_VariantSprite).
 */

typedef struct VariantSprite VariantSprite;
typedef struct VariantSpriteMethods VariantSpriteMethods;

/* Sprite's slots, then this class's own. `tools/classtable.py
 * gVariantSpriteMethods --vs gSpriteMethods` lists the overrides of the
 * inherited ones (VariantSprite__VariantSprite, VariantSprite__SetVariantClut at
 * reset, VariantSprite__UpdateScale, VariantSprite__Update). The three own slots
 * hold empty functions and nothing calls them. */
/* clang-format off */
#define VARIANTSPRITE_SLOTS(Self, CtorParams)                                                         \
    SPRITE_SLOTS(Self, CtorParams);                                                                \
    /* +0x0BC */ void (*slotBC)(void); /* VariantSprite__func_57f40, empty; never called */           \
    /* +0x0C0 */ void (*slotC0)(void); /* VariantSprite__func_57f48, empty; never called */           \
    /* +0x0C4 */ void (*slotC4)(void)  /* VariantSprite__func_57f50, empty; never called */
/* clang-format on */

/* clang-format off */
#define VARIANTSPRITE_FIELDS(Methods)                                                                 \
    SPRITE_FIELDS(Methods);                                                                        \
    /* +0x0A0 */ s32 variant; /* SetVariantClut: the ctor's first argument, 0 or 1 */             \
    /* +0x0A4 */ s32 unkA4    /* zeroed by the ctor; no other accessor. The object is 0xA8 bytes (New_VariantSprite) */
/* clang-format on */

struct VariantSpriteMethods {
    VARIANTSPRITE_SLOTS(VariantSprite, (VariantSprite * self, s32 variant, void *arg2, void *texture));
};

struct VariantSprite {
    VARIANTSPRITE_FIELDS(VariantSpriteMethods);
};

/* The reset slot's occupant as the ctor calls it (see the banner). */
typedef void (*VariantSpriteResetFn)(VariantSprite *self, s32 variant);

extern VariantSpriteMethods gVariantSpriteMethods;
extern VariantSpriteMethods *GetVariantSpriteMethods(void); /* returns &gVariantSpriteMethods */

/* The class's own methods, in address order. */
VariantSprite *New_VariantSprite(s32 variant, void *arg2, void *texture);
void VariantSprite__VariantSprite(VariantSprite *self, s32 variant, void *arg2, void *texture);
void VariantSprite__SetVariantClut(VariantSprite *self, s32 variant);
void VariantSprite__UpdateScale(VariantSprite *self, s32 set, s16 *ratios);
void VariantSprite__Update(VariantSprite *self, void *sender, s32 event);
void VariantSprite__func_57f40(void);
void VariantSprite__func_57f48(void);
void VariantSprite__func_57f50(void);

#endif
