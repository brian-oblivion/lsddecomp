#ifndef BGLAYER_H
#define BGLAYER_H

#include "SceneNode.h"

/*
 * BgLayer -- class id 0x54, method table gBgLayerMethods: a SceneNode subclass
 * (its ctor chains to GetSceneNodeMethods()->ctor first, so the id tree
 * 0x4 -> 0x54 is the ctor chain) whose own fields, +0x044..+0x067, are
 * exactly libgs's GsBG (LIBGS.H: attribute, x, y, w, h, scrollx, scrolly,
 * r, g, b, map, mx, my, scalex, scaley, rotate). Methods in
 * src/GraphicsResources.c; no class derives from it.
 *
 * The name is round 83's, and the evidence is the GsBG: Viewport__DrawNode
 * (src/code_2864.c) passes a class-0x54 node's +0x044 to GsSortBg, and
 * BgLayer__Reset lays that GsBG over a map source's GsMAP. Its one outside
 * user is TaskCore (src/code_2c054.c): TaskCore__TaskCore builds one over its
 * TileMap (New_BgLayer(tileMap, 1)) into TaskCore::bgLayer, OnInit attaches
 * it to the scene root (unk14) and sets its colour, OnDeinit detaches it,
 * Finalize releases it, and the colour fades (TaskCore__TickColorFade,
 * TaskCore__TickFadeColor) call setColor every frame.
 *
 * SLOTS (`classtable.py gBgLayerMethods --vs gSceneNodeMethods`, 47 against 45):
 *  - +0x008 ctor, BgLayer__BgLayer(self, src, mode): SceneNode's, this
 *    table, then reset(self, src, mode). Returns nothing; the slot keeps
 *    SceneNode's `void *` ctor type, New_BgLayer ignoring the value (as
 *    include/GridCell.h);
 *  - +0x040 reset, BgLayer__Reset(self, src, mode): lays the GsBG over
 *    src's GsMAP. Its parameter list differs from the inherited slot's
 *    (self only), so the slot keeps SceneNode's type and the ctor, its one
 *    caller, casts to BgLayerResetFn (FINISHING-PLAN track 4 step 6);
 *  - +0x044 updateRotation (BgLayer__UpdateRotation) and +0x048 updateScale
 *    (BgLayer__UpdateScale): set or add, from the same Ratio16
 *    {num, den} ratio table SceneNode's reads; the GsBG's rotate from entry
 *    [2] (the z angle, the only one a 2D layer has), its scalex/scaley from
 *    entries [0] and [1];
 *  - two own slots: +0x0B8 setColor (BgLayer__SetColor: the GsBG's r, g, b,
 *    when `enable`; every TaskCore caller passes 1 and a u8[3] buffer) and
 *    +0x0BC slotBC (BgLayer__NoOp, empty; no known caller).
 *
 * FIELDS: the GsBG, +0x044..+0x067; the object is 0x68 bytes (New_BgLayer).
 * `bgAttribute` because SceneNode's +0x010 (GsDOBJ2.attribute) already has
 * the name. r, g, b are one signed three-byte struct (BgLayerRgb): retail
 * copies them lb/lb/lb, sb/sb/sb (BgLayer__SetColor, BgLayer__Reset), which
 * three u8 members would not give.
 *
 * The map source is a TileMap (gTileMapMethods, include/TileMap.h, round 88),
 * whose GsMAP starts at +0x02C. Only its tag is named here, as
 * include/TriggerWorld.h does for its descriptor.
 */

struct TileMap;

typedef struct BgLayer BgLayer;
typedef struct BgLayerMethods BgLayerMethods;

/* BgLayer's class id (gBgLayerMethods word +0x000). Two nibbles, so
 * `(header & 0xFF) == BGLAYER_CLASS_ID` is its is-kind-of test
 * (Viewport__DrawNode). */
#define BGLAYER_CLASS_ID 0x54

/* GsBG's r, g, b. Signed: see the banner. */
typedef struct BgLayerRgb {
    s8 r;
    s8 g;
    s8 b;
} BgLayerRgb;

struct BgLayerMethods {
    SCENENODE_SLOTS(BgLayer, (BgLayer * self, struct TileMap *src, s32 mode));
    /* +0x0B8 */ void (*setColor)(BgLayer *self, s32 enable, BgLayerRgb *rgb); /* BgLayer__SetColor */
    /* +0x0BC */ void (*slotBC)(void); /* BgLayer__NoOp, empty; no known caller */
};

/* BgLayer__Reset's own parameter list, which the inherited +0x040 slot does
 * not carry: BgLayer__BgLayer casts `methods->reset` to it. */
typedef void (*BgLayerResetFn)(BgLayer *self, struct TileMap *src, s32 mode);

struct BgLayer {
    SCENENODE_FIELDS(BgLayerMethods);
    /* +0x044 */ u32 bgAttribute; /* GsBG.attribute: 0x1000000 (sized to the map) or 0x2000000 (320 x 240), BgLayer__Reset */
    /* +0x048 */ s16 x;
    /* +0x04A */ s16 y;
    /* +0x04C */ s16 w;
    /* +0x04E */ s16 h;
    /* +0x050 */ s16 scrollx;
    /* +0x052 */ s16 scrolly;
    /* +0x054 */ BgLayerRgb color; /* GsBG r, g, b: gBgLayerDefaultColor at reset; setColor */
    /* +0x057 */ u8 pad57;
    /* +0x058 */ void *map; /* GsBG.map: the source's GsMAP (&src->map, +0x02C) */
    /* +0x05C */ s16 mx;    /* the pivot: w / 2, h / 2 */
    /* +0x05E */ s16 my;
    /* +0x060 */ s16 scalex; /* 20.12; 0x1000 at reset, clamped to 30000 by updateScale */
    /* +0x062 */ s16 scaley;
    /* +0x064 */ s32 rotate; /* GsBG.rotate, 20.12; updateRotation */
};

extern BgLayerMethods gBgLayerMethods;
extern BgLayerMethods *GetBgLayerMethods(void); /* returns &gBgLayerMethods */

/* The class's own methods, in address order. */
BgLayer *New_BgLayer(struct TileMap *src, s32 mode); /* BMemPMgrAlloc(0x68), then ctor */
void BgLayer__BgLayer(BgLayer *self, struct TileMap *src, s32 mode);
void BgLayer__Reset(BgLayer *self, struct TileMap *src, s32 mode);
void BgLayer__UpdateRotation(BgLayer *self, s32 set, Ratio16 *table);
void BgLayer__UpdateScale(BgLayer *self, s32 set, Ratio16 *table);
void BgLayer__SetColor(BgLayer *self, s32 enable, BgLayerRgb *rgb);
void BgLayer__NoOp(void);

#endif
