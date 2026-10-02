#ifndef VARIANT_SPRITE_H
#define VARIANT_SPRITE_H

#include "sprite.h"

/**
 * @file variant_sprite.h
 * @brief VariantSprite, a world-space Sprite whose variant, 0 or 1, picks its
 *        texture cell and CLUT: the sprites StyleEffect carries.
 *
 * Methods in src/world/variant_sprite.c, New_VariantSprite through
 * GetVariantSpriteMethods.
 */

typedef struct VariantSprite VariantSprite;
typedef struct VariantSpriteMethods VariantSpriteMethods;

/**
 * @brief VariantSprite's slots: Sprite's, then three of its own, which hold
 * empty functions and which nothing calls.
 *
 * Overrides of inherited slots: the ctor, +0x040 reset
 * (VariantSprite__SetVariantClut), +0x048 updateScale
 * (VariantSprite__UpdateScale) and +0x098 update (VariantSprite__Update).
 * Two take a different parameter list from the slot they fill, so the slot
 * keeps the inherited type: reset takes a variant where SceneNode's slot
 * takes none and returns nothing where it returns `void *` (the ctor calls
 * it through VariantSpriteResetFn); updateScale reads its table as two ratio
 * pairs where SceneNode's reads three.
 */
/* clang-format off */
#define VARIANTSPRITE_SLOTS(Self, CtorParams)                                                      \
    SPRITE_SLOTS(Self, CtorParams);                                                                \
    /* +0x0BC */ void (*slotBC)(void); /* @see VariantSprite__NoOpSlotBC; never called */          \
    /* +0x0C0 */ void (*slotC0)(void); /* @see VariantSprite__NoOpSlotC0; never called */          \
    /* +0x0C4 */ void (*slotC4)(void)  /* @see VariantSprite__NoOpSlotC4; never called */
/* clang-format on */

/**
 * @brief VariantSprite's fields: Sprite's, then the variant and one word the
 * ctor clears.
 */
/* clang-format off */
#define VARIANTSPRITE_FIELDS(Methods)                                                              \
    SPRITE_FIELDS(Methods);                                                                        \
    /* +0x0A0 */ s32 variant; /* the ctor's first argument, 0 or 1 (SetVariantClut); never read */ \
    /* +0x0A4 */ s32 unusedA4    /* zeroed by the ctor; nothing else reads or writes it */
/* clang-format on */

/** @brief VariantSprite's method table (see VARIANTSPRITE_SLOTS). */
struct VariantSpriteMethods {
    VARIANTSPRITE_SLOTS(VariantSprite,
                        (VariantSprite * self, s32 variant, void *resetArg, void *texture));
};

/**
 * @brief A world-space sprite in two variants (class id 0x1F44), and the
 * variant is the whole of what it adds to Sprite: `variant` picks the
 * texture cell the Sprite ctor binds (sVariantSpriteCells, two adjacent
 * 16x16 cells) and the CLUT the reset slot then points the GsSPRITE at
 * (sVariantSpriteClut, two adjacent 16-colour rows at VRAM y 511). Its
 * class id's low 12 bits are not 0x144, so Viewport__DrawNode projects it
 * from its coordinate like any Sprite, not by the screen-space path
 * ScreenSprite takes. No class derives from it.
 *
 * Who makes it: only StyleEffect (include/style_effect.h), five per effect
 * of the two sprite kinds, in StyleEffect__SpawnSprites, as
 * New_VariantSprite(variant, 0, sStyleEffectTim) with `variant` 0 on every
 * path, so variant 1's cell and CLUT are never selected. StyleEffect drives
 * them only through inherited slots: attachToParent, setColor, updateScale,
 * setDisplay, the semi-transparency pair, and sprite.rotate directly.
 *
 * Lifecycle: the ctor runs Sprite's ctor with the texture and the variant's
 * cell, installs this table, clears `unusedA4` and calls reset with the
 * variant, which records it and replaces the CLUT Sprite's reset took from
 * the texture.
 */
struct VariantSprite {
    VARIANTSPRITE_FIELDS(VariantSpriteMethods);
}; /* 0xA8 bytes: New_VariantSprite */

/** @brief The reset slot as the ctor calls it (VariantSprite__SetVariantClut). */
typedef void (*VariantSpriteResetFn)(VariantSprite *self, s32 variant);

/** @brief VariantSprite's method table (see VariantSpriteMethods). */
extern VariantSpriteMethods gVariantSpriteMethods;

/**
 * @brief Returns VariantSprite's method table.
 * @return &gVariantSpriteMethods.
 */
extern VariantSpriteMethods *GetVariantSpriteMethods(void);

/**
 * @brief Allocates a VariantSprite from the BMemPMgr pool and constructs it.
 * @param variant  0 or 1: the texture cell and CLUT.
 * @param resetArg Handed on to Sprite__Reset, which does not read it.
 * @param texture  The TimImage the sprite draws from.
 * @return The new sprite, or NULL when the pool is exhausted.
 */
VariantSprite *New_VariantSprite(s32 variant, void *resetArg, void *texture);

/**
 * @brief Constructor (slot +0x008): Sprite's ctor with the variant's cell,
 * this class's table, then reset with the variant. Returns nothing: it ends
 * in that call.
 * @param self     The object being constructed.
 * @param variant  0 or 1.
 * @param resetArg Handed on to Sprite__Reset, which does not read it.
 * @param texture  The TimImage the sprite draws from.
 */
void VariantSprite__VariantSprite(VariantSprite *self, s32 variant, void *resetArg, void *texture);

/**
 * @brief reset (slot +0x040): records the variant and points the GsSPRITE's
 * CLUT at that variant's row.
 * @param self    The sprite.
 * @param variant 0 or 1.
 */
void VariantSprite__SetVariantClut(VariantSprite *self, s32 variant);

/**
 * @brief updateScale (slot +0x048): turns two num/den ratios, x then y, into
 * 20.12 fixed point for the GsSPRITE's scalex/scaley, or, while Sprite's
 * accumulateScale is set, multiplies accumScaleX/Y by them instead.
 * @param self   The sprite.
 * @param set    Not read.
 * @param ratios Two Ratio16 pairs (callers' tables hold three; the third is
 *               not read).
 */
void VariantSprite__UpdateScale(VariantSprite *self, s32 set, Ratio16 *ratios);

/**
 * @brief update (slot +0x098): does nothing, as Sprite__Update does.
 * @param self   The sprite.
 * @param sender The FrameClock (unused).
 * @param event  The event code (unused).
 */
void VariantSprite__Update(VariantSprite *self, void *sender, s32 event);

/** @brief Slot +0x0BC: does nothing; never called. */
void VariantSprite__NoOpSlotBC(void);

/** @brief Slot +0x0C0: does nothing; never called. */
void VariantSprite__NoOpSlotC0(void);

/** @brief Slot +0x0C4: does nothing; never called. */
void VariantSprite__NoOpSlotC4(void);

#endif
