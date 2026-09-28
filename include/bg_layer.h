/**
 * @file bg_layer.h
 * @brief BgLayer, the scene node that draws a tile map as one libgs GsBG
 *        background, and its method table.
 */
#ifndef BG_LAYER_H
#define BG_LAYER_H

#include "scene_node.h"

struct TileMap;

typedef struct BgLayer BgLayer;
typedef struct BgLayerMethods BgLayerMethods;

/** BgLayer's class id (gBgLayerMethods word +0x000). Two nibbles, so
 * `(header & 0xFF) == BGLAYER_CLASS_ID` is its is-kind-of test
 * (Viewport__DrawNode). */
#define BGLAYER_CLASS_ID 0x54

/**
 * @brief BgLayer's method table, gBgLayerMethods: SceneNode's slots, four of
 *        them overridden, then two of its own.
 *
 * The overrides are +0x008 ctor (BgLayer__BgLayer), +0x040 reset
 * (BgLayer__Reset), +0x044 updateRotation (BgLayer__UpdateRotation) and
 * +0x048 updateScale (BgLayer__UpdateScale). The ctor and reset keep
 * SceneNode's slot types: New_BgLayer ignores the ctor's return value, and
 * reset takes more parameters than the inherited slot declares, so its one
 * caller, the ctor, casts it to BgLayerResetFn.
 */
struct BgLayerMethods {
    SCENENODE_SLOTS(BgLayer, (BgLayer * self, struct TileMap *src, s32 mode));
    /* +0x0B8 */ void (*setColor)(BgLayer *self, s32 enable, ColorRgb *rgb); /**< @see BgLayer__SetColor */
    /* +0x0BC */ void (*slotBC)(void); /**< @see BgLayer__NoOp (empty; no known caller) */
};

/** @brief BgLayer__Reset's own parameter list, which the inherited +0x040
 *         slot does not carry: BgLayer__BgLayer casts `methods->reset` to it. */
typedef void (*BgLayerResetFn)(BgLayer *self, struct TileMap *src, s32 mode);

/**
 * @brief A background layer: a SceneNode (class id 0x54) whose own fields are
 *        exactly libgs's GsBG, laid over a TileMap's GsMAP.
 *
 * Parent SceneNode (its ctor chains to SceneNode's first); no class derives
 * from it. Methods in src/graphics/graphics_resources.c. The object is 0x68
 * bytes (New_BgLayer).
 *
 * Viewport__DrawNode (src/graphics/viewport_draw.c) passes a BgLayer's GsBG,
 * from `bgAttribute` on, to GsSortBg. Its one owner is TaskCore
 * (src/app/task.c): TaskCore__TaskCore builds one over its TileMap
 * (New_BgLayer(tileMap, 1)), OnInit attaches it to the LightRig and sets its
 * colour, OnDeinit detaches it, Finalize releases it, and the fades
 * (TaskCore__TickFadeIn, TaskCore__TickFadeOut) call setColor every frame.
 */
struct BgLayer {
    SCENENODE_FIELDS(BgLayerMethods);
    /* +0x044 */ u32 bgAttribute; /**< GsBG.attribute: 8-bit CLUT (layer sized to the map) or 15-bit direct (320 x 240), set by BgLayer__Reset; SceneNode's +0x010 (GsDOBJ2.attribute) already has the plain name */
    /* +0x048 */ s16 x;           /**< GsBG.x: screen position, 0 at reset */
    /* +0x04A */ s16 y;           /**< GsBG.y: 0 at reset */
    /* +0x04C */ s16 w;           /**< GsBG.w: the layer's width in pixels */
    /* +0x04E */ s16 h;           /**< GsBG.h: its height */
    /* +0x050 */ s16 scrollx;     /**< GsBG.scrollx: 0 at reset */
    /* +0x052 */ s16 scrolly;     /**< GsBG.scrolly: 0 at reset */
    /* +0x054 */ ColorRgb color; /**< GsBG r, g, b as one ColorRgb, copied whole: sBgLayerDefaultColor at reset, then setColor */
    /* +0x057 */ u8 pad57;
    /* +0x058 */ void *map; /**< GsBG.map: the source TileMap's GsMAP (&src->map) */
    /* +0x05C */ s16 mx;    /**< GsBG.mx: the pivot's x, w / 2 */
    /* +0x05E */ s16 my;    /**< GsBG.my: the pivot's y, h / 2 */
    /* +0x060 */ s16 scalex; /**< GsBG.scalex, 20.12: ONE at reset, at most 30000 through updateScale */
    /* +0x062 */ s16 scaley; /**< GsBG.scaley, as scalex */
    /* +0x064 */ s32 rotate; /**< GsBG.rotate, 20.12: 0 at reset, then updateRotation */
};

/** BgLayer's method table. */
extern BgLayerMethods gBgLayerMethods;

/**
 * @brief Returns BgLayer's method table.
 * @return &gBgLayerMethods.
 */
extern BgLayerMethods *GetBgLayerMethods(void);

/**
 * @brief Allocates a BgLayer from the pool and constructs it over a tile map.
 * @param src  The tile map whose GsMAP the layer draws.
 * @param mode 0 to size the layer to the map with an 8-bit CLUT, 1 for a
 *             320 x 240 layer of 15-bit texels (TaskCore's).
 * @return The new layer, or NULL when the pool is exhausted.
 */
BgLayer *New_BgLayer(struct TileMap *src, s32 mode);

/**
 * @brief Constructor (slot +0x008): SceneNode's, then reset over `src`.
 * @param self The object to construct.
 * @param src  The tile map to draw.
 * @param mode The layer mode, as New_BgLayer's.
 */
void BgLayer__BgLayer(BgLayer *self, struct TileMap *src, s32 mode);

/**
 * @brief Slot +0x040: lays the GsBG over `src`'s map at the origin,
 *        unscrolled, unscaled and unrotated, pivoting on its centre, in the
 *        default colour.
 * @param self The layer.
 * @param src  The tile map whose GsMAP it draws; mode 0 sizes the layer to it.
 * @param mode 0: sized to the map, 8-bit CLUT; 1: 320 x 240, 15-bit direct.
 *             Any other value leaves the attribute and size as they were.
 */
void BgLayer__Reset(BgLayer *self, struct TileMap *src, s32 mode);

/**
 * @brief Slot +0x044: sets or adds to the layer's rotation from a ratio table.
 * @param self  The layer.
 * @param set   Nonzero to replace the rotation, zero to add to it.
 * @param table SceneNode's per-axis ratio table; entry [2], the z angle, is
 *              read as num / den in 20.12.
 */
void BgLayer__UpdateRotation(BgLayer *self, s32 set, Ratio16 *table);

/**
 * @brief Slot +0x048: sets or adds to the layer's x and y scale from a ratio
 *        table, clamped to 30000 (20.12).
 *
 * When setting, an axis whose ratio has a zero divisor gets ONE. When adding,
 * a sum past the clamp becomes the clamp, or 1 when that axis's ratio had a
 * negative term.
 * @param self The layer.
 * @param set  Nonzero to replace the scale, zero to add to it.
 * @param src  SceneNode's per-axis ratio table; entries [0] (x) and [1] (y)
 *             are read.
 */
void BgLayer__UpdateScale(BgLayer *self, s32 set, Ratio16 *src);

/**
 * @brief Slot +0x0B8: takes `rgb` as the layer's colour.
 * @param self   The layer.
 * @param enable Nonzero to copy `rgb`; zero does nothing.
 * @param rgb    The new colour.
 */
void BgLayer__SetColor(BgLayer *self, s32 enable, ColorRgb *rgb);

/** @brief Slot +0x0BC: does nothing. */
void BgLayer__NoOp(void);

#endif
