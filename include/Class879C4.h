#ifndef CLASS879C4_H
#define CLASS879C4_H

#include "Sprite.h"

/*
 * Class879C4 -- a two-variant world-space sprite (class id 0x1F44, method
 * table gClass879C4Methods): Sprite's subclass (its ctor chains to
 * GetSpriteMethods()->ctor first). Methods in src/class_3bb8c_p.c (New_,
 * ctor), src/class_3bb8c_q.c (SetVariantClut, UpdateScale) and
 * src/class_3bb8c_t.c (four empty leaves, the getter). No class derives
 * from it. Class876FC (class_3bb8c_s.c) builds five per instance
 * (Class876FC__SpawnSprites) and drives them through inherited slots only.
 *
 * `variant` (0 or 1, the ctor's first argument) picks both the texture cell
 * the Sprite ctor binds (gClass879C4Cells[variant]) and, through the reset
 * slot the ctor calls last, the CLUT row (gClass879C4ClutX/Y[variant]).
 * The name stays the table's address: what the sprites are in the game is
 * not established.
 *
 * Settled from the bytes (round 87, track 4):
 *  - +0x040 reset: the occupant, Class879C4__SetVariantClut, takes a
 *    variant where the slot (Class6B5CC's) takes none, so the ctor calls it
 *    through Class879C4ResetFn (a cast, no code). It returns nothing, and
 *    the ctor ends in that call without setting $v0, so the ctor returns
 *    nothing either (ScreenSprite's precedent), where the slot, from
 *    Class6B5CC, returns `void *`. New_Class879C4 does not read it.
 *  - +0x048 updateScale: the occupant reads `table` as two s16 num/den
 *    ratio pairs (x, y) where Class6B5CC's reads three.
 *
 * The object is 0xA8 bytes (New_Class879C4).
 */

typedef struct Class879C4 Class879C4;
typedef struct Class879C4Methods Class879C4Methods;

/* Sprite's slots, then this class's own. `tools/classtable.py
 * gClass879C4Methods --vs gSpriteMethods` lists the overrides of the
 * inherited ones (Class879C4__Class879C4, Class879C4__SetVariantClut at
 * reset, Class879C4__UpdateScale, Class879C4__Update). The three own slots
 * hold empty functions and nothing calls them. */
/* clang-format off */
#define CLASS879C4_SLOTS(Self, CtorParams)                                                         \
    SPRITE_SLOTS(Self, CtorParams);                                                                \
    /* +0x0BC */ void (*slotBC)(void); /* Class879C4__func_57f40, empty; never called */           \
    /* +0x0C0 */ void (*slotC0)(void); /* Class879C4__func_57f48, empty; never called */           \
    /* +0x0C4 */ void (*slotC4)(void)  /* Class879C4__func_57f50, empty; never called */
/* clang-format on */

/* clang-format off */
#define CLASS879C4_FIELDS(Methods)                                                                 \
    SPRITE_FIELDS(Methods);                                                                        \
    /* +0x0A0 */ s32 variant; /* SetVariantClut: the ctor's first argument, 0 or 1 */             \
    /* +0x0A4 */ s32 unkA4    /* zeroed by the ctor; no other accessor. The object is 0xA8 bytes (New_Class879C4) */
/* clang-format on */

struct Class879C4Methods {
    CLASS879C4_SLOTS(Class879C4, (Class879C4 * self, s32 variant, void *arg2, void *texture));
};

struct Class879C4 {
    CLASS879C4_FIELDS(Class879C4Methods);
};

/* The reset slot's occupant as the ctor calls it (see the banner). */
typedef void (*Class879C4ResetFn)(Class879C4 *self, s32 variant);

extern Class879C4Methods gClass879C4Methods;
extern Class879C4Methods *GetClass879C4Methods(void); /* returns &gClass879C4Methods */

/* The class's own methods, in address order. */
Class879C4 *New_Class879C4(s32 variant, void *arg2, void *texture);
void Class879C4__Class879C4(Class879C4 *self, s32 variant, void *arg2, void *texture);
void Class879C4__SetVariantClut(Class879C4 *self, s32 variant);
void Class879C4__UpdateScale(Class879C4 *self, s32 set, s16 *ratios);
void Class879C4__Update(Class879C4 *self, void *sender, s32 event);
void Class879C4__func_57f40(void);
void Class879C4__func_57f48(void);
void Class879C4__func_57f50(void);

#endif
