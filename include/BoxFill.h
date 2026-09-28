#ifndef BOXFILL_H
#define BOXFILL_H

#include "SceneNode.h"

/*
 * BoxFill -- a flat-coloured screen rectangle (class id 0x64, method table
 * gBoxFillMethods): a SceneNode subclass holding one libgs GsBOXF.
 * Methods in src/ui/ScreenWidgets.c (New_BoxFill, the ctor, Reset) and
 * src/ui/ScreenWidgets.c (the rest, up to GetBoxFillMethods). The name is for
 * what the class does, and the evidence is this:
 *  - Viewport__DrawNode (ViewportDraw.c) takes its own path for a node whose
 *    class-id low byte is 0x64, i.e. this class and everything below it:
 *    it computes the GsBOXF's x/y from `posX`/`posY` (as a percentage of
 *    half the screen width/height while `relative` is set, as pixels
 *    otherwise) and calls GsSortBoxFill(&box, ot, pri) with `pri`.
 *  - The class's own slots set exactly those: setColor (the GsBOXF r,g,b),
 *    setPosition, setSize (w, h), setPri, and attachAbsolute, which attaches
 *    and clears `relative`. Its setDisplay/setSemiTransOn/setSemiTransRate
 *    overrides are SceneNode's GetSetBitField accessors at the same bit
 *    positions (31, 30, 28..29), over the GsBOXF attribute instead of the
 *    GsDOBJ2 one.
 *  - Its users: TaskCore's listView (the frame behind a scrolled list:
 *    attached at the list's position, then setSize(40, rows * 12)),
 *    GraphRoom's 100 plotted dots, and the style decoration boxes of
 *    the style layer (ApplyStyleDecorationIfSet makes its box semi-transparent:
 *    setSemiTransOn(1), setSemiTransRate(0)).
 *
 * Ctor chain: BoxFill__BoxFill calls GetSceneNodeMethods()->ctor first, so
 * the id parent (0x4) is the ctor-chain parent. One class derives from it,
 * FadeBox (gFadeBoxMethods, 0x164, a colour fade over the box;
 * include/FadeBox.h), whose ctor calls this one's first
 * (FadeBox__FadeBox: GetBoxFillMethods()->ctor).
 *
 * Overrides whose parameter list differs from the inherited slot keep the
 * slot's type; a caller reaching the
 * override through the slot casts to the typedef below it (no code):
 *  - +0x040 reset: BoxFill__Reset takes the ctor's (size, color, pri) and
 *    initialises the box from them; the ctor calls it through
 *    BoxFillResetFn.
 *  - +0x04C attachToParent: BoxFill__AttachToParent's third argument is a
 *    screen position (a BoxFillPos) where the slot, SceneNode's, types a
 *    LongVec3 offset; it attaches with a NULL offset, then setPosition.
 *    Callers cast to BoxFillAttachToParentFn (the style layer
 *    in ObjMStyleActor.c, Task) or, through a SceneNode pointer, cast the argument
 *    (Viewport__SetFadeBox). BoxFill__AttachAbsolute calls it with FOUR
 *    arguments through an unprototyped pointer (see its match report).
 * The ctor itself returns nothing where SceneNode's slot returns `void *`;
 * every caller ignores the value.
 *
 * The object is 0x6C bytes (New_BoxFill); FadeBox's own fields start at
 * +0x06C.
 */

typedef struct BoxFill BoxFill;
typedef struct BoxFillMethods BoxFillMethods;
typedef struct BoxFillSize BoxFillSize;
typedef struct BoxFillPos BoxFillPos;

/* Bit positions in `boxAttribute`, the GsBOXF attribute word, that the
 * setDisplay/setSemiTransOn/setSemiTransRate overrides set through
 * GetSetBitField: libgs's GsDOFF, GsALON and the 2-bit GsAZERO..GsATHREE
 * rate, the same positions as SceneNode's GsDOBJ2 attribute. */
#define BOXFILL_ATTR_RATE_SHIFT 28 /* semitransparency rate, 2 bits */
#define BOXFILL_ATTR_ALON_SHIFT 30 /* GsALON: semitransparency on */
#define BOXFILL_ATTR_DOFF_SHIFT 31 /* GsDOFF: display off */

/* setSemiTransRate's argument: libgs's GsAZERO..GsATHREE rate (GsAONE adds
 * the box to what is behind it, GsATWO subtracts it), unshifted. */
#define BOXFILL_SEMITRANS_RATE(gsRate) ((gsRate) >> BOXFILL_ATTR_RATE_SHIFT)

/* BoxFill's class id (gBoxFillMethods word +0x000). Two nibbles, so
 * `(header & 0xFF) == BOXFILL_CLASS_ID` is its is-kind-of test, true for
 * FadeBox (0x164) too (Viewport__DrawNode). */
#define BOXFILL_CLASS_ID 0x64

/* The box's size in pixels, width then height: the ctor's (and Reset's)
 * size argument, setSize's and FadeBox's pushPosition's. Each word is stored
 * into the u16 boxW/boxH, which reads only its low halfword (lhu at +0x000
 * and +0x004). The callers pass two-word arrays and pairs of their own
 * (sListViewSize, gGraphPointSize, sStyleDecorBoxSize, ObjMStyleActor's
 * PairXY), so New_BoxFill and the ctor slot take `void *` and setSize
 * `s32 *`. The same layout as BoxFillPos, which is a position. */
struct BoxFillSize {
    s32 w; /* +0x000 */
    s32 h; /* +0x004 */
};

/* The box's screen position (setPosition, attachToParent's third argument,
 * attachAbsolute's; FadeBox's pushPosition): copied whole into posX/posY, so
 * percent of half the screen width/height while `relative` is set and pixels
 * after attachAbsolute. ScreenSprite's ScreenSpritePos has the same layout
 * but is always a percentage. */
struct BoxFillPos {
    s32 x; /* +0x000 */
    s32 y; /* +0x004 */
};

/* The box's colour, GsBOXF r, g, b: what setColor copies into `color` (or
 * adds to it when overwrite is 0), and the ctor's (and Reset's) colour
 * argument. Not Sony's CVECTOR, which is four bytes and unsigned.
 * MATCHING: signed, and exactly three bytes -- a whole-struct copy is three
 * lb/sb pairs (BoxFill__ApplyColor, GraphRoom__BuildGraphPoints). The field
 * and the slot parameter stay `u8 color[3]` and `void *rgb`. */
typedef struct BoxFillRgb {
    s8 r;
    s8 g;
    s8 b;
} BoxFillRgb;

/* SceneNode's slots, then this class's own. `tools/classtable.py
 * gBoxFillMethods --vs gSceneNodeMethods` lists the overrides of the
 * inherited ones: +0x008 (BoxFill__BoxFill), +0x040 (BoxFill__Reset), +0x04C
 * (BoxFill__AttachToParent), +0x060 (BoxFill__SetDisplay), +0x064
 * (BoxFill__SetSemiTrans), +0x068 (BoxFill__SetSemiTransRate). */
/* clang-format off */
#define BOXFILL_SLOTS(Self, CtorParams)                                                            \
    SCENENODE_SLOTS(Self, CtorParams);                                                            \
    /* +0x0B8 */ void (*setColor)(Self *self, s32 overwrite, void *rgb);  /* BoxFill__SetColor: copy the 3 bytes, or add them when overwrite is 0 */ \
    /* +0x0BC */ void (*setPosition)(Self *self, BoxFillPos *pos);        /* BoxFill__SetPosition: only while attached */ \
    /* +0x0C0 */ void (*setSize)(Self *self, s32 *size);                  /* BoxFill__SetSize: {w, h} words, low halves; only while attached */ \
    /* +0x0C4 */ void (*attachAbsolute)(Self *self, SceneNode *parent, BoxFillPos *pos, s32 attachArg); /* BoxFill__AttachAbsolute: attachToParent, relative = 0, stores attachArg */ \
    /* +0x0C8 */ void (*setPri)(Self *self, s32 pri);                     /* BoxFill__SetPri */          \
    /* +0x0CC */ s32 (*setMask)(Self *self, s32 bits)                     /* BoxFill__SetMask: mask = (1 << bits) - 1; Reset passes 13 */
/* clang-format on */

/* clang-format off */
#define BOXFILL_FIELDS(Methods)                                                                    \
    SCENENODE_FIELDS(Methods);                                                                    \
    /* +0x044 */ s32 pri;          /* setPri, the ctor's third argument; DrawNode's GsSortBoxFill pri (its low halfword) */ \
    /* +0x048 */ s32 relative;     /* 1 from Reset, 0 from attachAbsolute: DrawNode reads posX/posY as percent of half the screen while set */ \
    /* +0x04C */ s32 attachArg;    /* zeroed by Reset; attachAbsolute's fourth argument; no reader */ \
    /* +0x050 */ s32 posX;         /* setPosition; DrawNode's source for boxX */                   \
    /* +0x054 */ s32 posY;         /* setPosition; DrawNode's source for boxY */                   \
    /* +0x058 */ u32 boxAttribute; /* GsBOXF.attribute: the setDisplay/setSemiTransOn/setSemiTransRate bits */ \
    /* +0x05C */ s16 boxX;         /* GsBOXF.x: zeroed by Reset, written by DrawNode */            \
    /* +0x05E */ s16 boxY;         /* GsBOXF.y */                                                  \
    /* +0x060 */ u16 boxW;         /* GsBOXF.w: Reset and setSize (FadeBox reads it lhu) */    \
    /* +0x062 */ u16 boxH;         /* GsBOXF.h */                                                  \
    /* +0x064 */ u8 color[3];      /* GsBOXF.r, g, b: setColor (Reset's default gBoxFillDefaultColor) */     \
    /* +0x067 */ u8 pad67;                                                                         \
    /* +0x068 */ s32 mask          /* setMask. The object is 0x6C bytes (New_BoxFill) */
/* clang-format on */

struct BoxFillMethods {
    BOXFILL_SLOTS(BoxFill, (BoxFill * self, void *size, void *color, s32 pri));
};

struct BoxFill {
    BOXFILL_FIELDS(BoxFillMethods);
};

extern BoxFillMethods gBoxFillMethods;
extern BoxFillMethods *GetBoxFillMethods(void); /* returns &gBoxFillMethods */

/* +0x040's and +0x04C's occupants, as a caller reaching them through the
 * inherited slots casts them (see the banner). */
typedef void (*BoxFillResetFn)(BoxFill *self, BoxFillSize *size, void *color, s32 pri);
typedef void (*BoxFillAttachToParentFn)(BoxFill *self, SceneNode *parent, BoxFillPos *pos);

/* The class's own methods, in ROM order (ScreenWidgets, then ScreenWidgets).
 * A subclass reaches the base ones through GetBoxFillMethods() and upcasts. */
BoxFill *New_BoxFill(void *size, void *color, s32 pri);
void BoxFill__BoxFill(BoxFill *self, BoxFillSize *size, void *color, s32 pri);
void BoxFill__Reset(BoxFill *self, BoxFillSize *size, void *color, s32 pri);
void BoxFill__AttachToParent(BoxFill *self, SceneNode *parent, BoxFillPos *pos);
s32 BoxFill__SetDisplay(BoxFill *self, s32 on);
s32 BoxFill__SetSemiTrans(BoxFill *self, s32 on);
s32 BoxFill__SetSemiTransRate(BoxFill *self, s32 rate);
void BoxFill__SetColor(BoxFill *self, s32 overwrite, u8 *rgb);
void BoxFill__ApplyColor(BoxFill *self, u8 *dst, u8 *src, s32 overwrite);
void BoxFill__SetPosition(BoxFill *self, BoxFillPos *pos);
void BoxFill__SetSize(BoxFill *self, s32 *size);
void BoxFill__AttachAbsolute(BoxFill *self, SceneNode *parent, BoxFillPos *pos, s32 attachArg);
void BoxFill__SetPri(BoxFill *self, s32 pri);
s32 BoxFill__SetMask(BoxFill *self, s32 bits);

#endif
