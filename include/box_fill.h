/**
 * @file box_fill.h
 * @brief BoxFill, a flat-coloured screen rectangle drawn as one libgs GsBOXF.
 *
 * Declares the BoxFill class (object, method table, and the slot and field
 * macros its subclass expands), the size and position pairs its methods
 * take, and its attribute-bit and class-id constants. Methods are defined
 * in src/ui/box_fill.c.
 */
#ifndef BOX_FILL_H
#define BOX_FILL_H

#include "scene_node.h"

typedef struct BoxFill BoxFill;
typedef struct BoxFillMethods BoxFillMethods;
typedef struct BoxFillSize BoxFillSize;
typedef struct BoxFillPos BoxFillPos;

/** @name GsBOXF attribute bits
 * Bit positions in `boxAttribute` that the setDisplay, setSemiTransOn and
 * setSemiTransRate overrides set through GetSetBitField: libgs's GsDOFF,
 * GsALON and the 2-bit GsAZERO..GsATHREE rate, the same positions as
 * SceneNode's GsDOBJ2 attribute.
 * @{ */
#define BOXFILL_ATTR_RATE_SHIFT 28 /**< semitransparency rate, 2 bits */
#define BOXFILL_ATTR_ALON_SHIFT 30 /**< GsALON: semitransparency on */
#define BOXFILL_ATTR_DOFF_SHIFT 31 /**< GsDOFF: display off */
/** @} */

/** setSemiTransRate's argument for a libgs GsAZERO..GsATHREE rate (GsAONE
 * adds the box to what is behind it, GsATWO subtracts it), unshifted. */
#define BOXFILL_SEMITRANS_RATE(gsRate) ((gsRate) >> BOXFILL_ATTR_RATE_SHIFT)

/** BoxFill's class id (gBoxFillMethods word +0x000). Two nibbles, so
 * `(header & CLASS_ID_LEVEL2_MASK) == BOXFILL_CLASS_ID` is its is-kind-of test, true for
 * FadeBox (0x164) too (Viewport__DrawNode). */
#define BOXFILL_CLASS_ID 0x64

/**
 * @brief The box's size in pixels, width then height.
 *
 * The ctor's (and Reset's) size argument, setSize's, and FadeBox's
 * pushPosition's. Only the low halfword of each word is kept, in the u16
 * boxW/boxH. Callers pass two-word arrays and pairs of their own
 * (sListViewSize, sGraphPointSize, sStyleDecorBoxSize, and style_layer.c's
 * BoxFillSize copied from sStyleDecorSizeW), so New_BoxFill and the ctor
 * slot take `void *` and setSize `s32 *`. The same layout as BoxFillPos,
 * which is a position.
 */
struct BoxFillSize {
    s32 w; /**< +0x000: width in pixels */
    s32 h; /**< +0x004: height in pixels */
};

/**
 * @brief The box's screen position.
 *
 * Taken by setPosition, attachToParent (third argument), attachAbsolute and
 * FadeBox's pushPosition, and copied whole into posX/posY: percent of half
 * the screen width/height while `relative` is set, pixels after
 * attachAbsolute. ScreenSprite's ScreenSpritePos has the same layout but is
 * always a percentage.
 */
struct BoxFillPos {
    s32 x; /**< +0x000: x, percent or pixels (see BoxFill's `relative`) */
    s32 y; /**< +0x004: y, percent or pixels */
};

/* The box's colour, GsBOXF r, g, b, is a ColorRgb (include/draw_system.h):
 * what setColor copies into `color` (or adds to it when overwrite is 0), and
 * the ctor's (and Reset's) colour argument. The field and the slot parameter
 * stay `u8 color[3]` and `void *rgb`. */

/** SceneNode's slots, then BoxFill's own. BoxFill overrides the inherited
 * +0x008 (BoxFill__BoxFill), +0x040 (BoxFill__Reset), +0x04C
 * (BoxFill__AttachToParent), +0x060 (BoxFill__SetDisplay), +0x064
 * (BoxFill__SetSemiTrans) and +0x068 (BoxFill__SetSemiTransRate). */
/* clang-format off */
#define BOXFILL_SLOTS(Self, CtorParams)                                                            \
    SCENENODE_SLOTS(Self, CtorParams);                                                            \
    /* +0x0B8 */ void (*setColor)(Self *self, s32 overwrite, void *rgb);  /* @see BoxFill__SetColor */ \
    /* +0x0BC */ void (*setPosition)(Self *self, BoxFillPos *pos);        /* @see BoxFill__SetPosition */ \
    /* +0x0C0 */ void (*setSize)(Self *self, s32 *size);                  /* @see BoxFill__SetSize */ \
    /* +0x0C4 */ void (*attachAbsolute)(Self *self, SceneNode *parent, BoxFillPos *pos, s32 attachArg); /* @see BoxFill__AttachAbsolute */ \
    /* +0x0C8 */ void (*setPri)(Self *self, s32 pri);                     /* @see BoxFill__SetPri */ \
    /* +0x0CC */ s32 (*setMask)(Self *self, s32 bits)                     /* @see BoxFill__SetMask */
/* clang-format on */

/** SceneNode's fields, then BoxFill's own; the object is 0x6C bytes. */
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
    /* +0x060 */ u16 boxW;         /* GsBOXF.w: Reset and setSize; FadeBox's pushPosition saves it */ \
    /* +0x062 */ u16 boxH;         /* GsBOXF.h */                                                  \
    /* +0x064 */ u8 color[3];      /* GsBOXF.r, g, b: setColor (Reset's default sBoxFillDefaultColor) */ \
    /* +0x067 */ u8 pad67;                                                                         \
    /* +0x068 */ s32 mask          /* setMask; read only by FadeBox's configure (maskPerTick) */
/* clang-format on */

/**
 * @brief BoxFill's method table: SceneNode's slots, then BoxFill's own.
 */
struct BoxFillMethods {
    BOXFILL_SLOTS(BoxFill, (BoxFill * self, void *size, void *color, s32 pri));
};

/**
 * @brief A flat-coloured screen rectangle: a SceneNode holding one libgs GsBOXF.
 *
 * Class id 0x64 (BOXFILL_CLASS_ID), method table gBoxFillMethods, parent
 * SceneNode: BoxFill__BoxFill runs SceneNode's ctor first. Methods in
 * src/ui/box_fill.c. One class derives from it, FadeBox (0x164,
 * include/fade_box.h), whose ctor runs this one's first.
 *
 * Drawing: Viewport__DrawNode takes its own path for a node whose class-id
 * low byte is 0x64. It computes the GsBOXF's x/y from `posX`/`posY` (as a
 * percentage of half the screen width/height while `relative` is set, as
 * pixels otherwise) and calls GsSortBoxFill with `pri`. The class's own
 * slots set exactly those: setColor (the GsBOXF r, g, b), setPosition,
 * setSize (w, h), setPri, and attachAbsolute, which attaches and clears
 * `relative`. Its setDisplay/setSemiTransOn/setSemiTransRate overrides set
 * the GsBOXF attribute's bits 31, 30 and 28..29 through GetSetBitField, the
 * positions SceneNode uses in its GsDOBJ2 attribute.
 *
 * Users: TaskCore's listView (the frame behind a scrolled list: attached at
 * the list's position, then setSize(40, rows * 12)), GraphRoom's 100 plotted
 * dots, and the style layer's decoration boxes (ApplyStyleDecorationIfSet
 * makes its box semi-transparent: setSemiTransOn(1), setSemiTransRate(0)).
 *
 * Overrides whose parameter list differs from the inherited slot keep the
 * slot's type, and a caller casts:
 *  - +0x040 reset: BoxFill__Reset takes the ctor's (size, color, pri); the
 *    ctor calls it through BoxFillResetFn.
 *  - +0x04C attachToParent: BoxFill__AttachToParent's third argument is a
 *    BoxFillPos screen position where SceneNode's slot types a LongVec3
 *    offset. Callers cast to BoxFillAttachToParentFn (the style layer in
 *    style_layer.c, task_core.c) or, through a SceneNode pointer, cast the
 *    argument (Viewport__SetFadeBox). BoxFill__AttachAbsolute passes it a
 *    fourth argument, which it ignores.
 * The ctor returns nothing where SceneNode's slot returns `void *`; every
 * caller ignores the value.
 *
 * The object is 0x6C bytes (New_BoxFill); FadeBox's own fields start at
 * +0x06C.
 */
struct BoxFill {
    BOXFILL_FIELDS(BoxFillMethods);
};

/** BoxFill's method table (class id BOXFILL_CLASS_ID). */
extern BoxFillMethods gBoxFillMethods;

/**
 * @brief Returns BoxFill's method table.
 * @return &gBoxFillMethods.
 */
extern BoxFillMethods *GetBoxFillMethods(void);

/** The +0x040 reset slot's occupant, BoxFill__Reset, as the ctor calls it. */
typedef void (*BoxFillResetFn)(BoxFill *self, BoxFillSize *size, void *color, s32 pri);
/** The +0x04C attachToParent slot's occupant, BoxFill__AttachToParent, as a
 * caller passing a BoxFillPos casts it. */
typedef void (*BoxFillAttachToParentFn)(BoxFill *self, SceneNode *parent, BoxFillPos *pos);

/* The class's own methods, in ROM order (box_fill.c). A subclass
 * reaches the base ones through GetBoxFillMethods() and upcasts. */

/**
 * @brief Allocates a BoxFill and runs its ctor through the method table.
 * @param size  the box's BoxFillSize (width, height in pixels).
 * @param color three bytes r, g, b, or NULL for the default grey.
 * @param pri   the ordering-table priority GsSortBoxFill is given.
 * @return the new box, or NULL when the allocation fails.
 */
BoxFill *New_BoxFill(void *size, void *color, s32 pri);

/**
 * @brief Constructs a BoxFill: SceneNode's ctor, BoxFill's table, then reset.
 * @param self  the object to construct.
 * @param size  the box's size in pixels.
 * @param color three bytes r, g, b, or NULL for the default grey.
 * @param pri   the ordering-table priority.
 */
void BoxFill__BoxFill(BoxFill *self, BoxFillSize *size, void *color, s32 pri);

/**
 * @brief Resets the box's priority, positioning mode, size, colour and mask.
 *
 * Sets `relative`, clears `attachArg`, the attribute word and the GsBOXF
 * x/y, stores the size, then calls setColor (overwriting) and setMask(13).
 * @param self  the box.
 * @param size  the box's size in pixels.
 * @param color three bytes r, g, b, or NULL for sBoxFillDefaultColor (128, 128, 128).
 * @param pri   the ordering-table priority.
 */
void BoxFill__Reset(BoxFill *self, BoxFillSize *size, void *color, s32 pri);

/**
 * @brief Attaches an unattached box under `parent` and places it at `pos`.
 *
 * Does nothing when the box already has a parent. Attaches through
 * SceneNode's attachToParent with no offset, then calls setPosition.
 * @param self   the box.
 * @param parent the node to attach under.
 * @param pos    the box's screen position.
 */
void BoxFill__AttachToParent(BoxFill *self, SceneNode *parent, BoxFillPos *pos);

/**
 * @brief Shows or hides the box (the GsBOXF attribute's GsDOFF bit).
 * @param self the box.
 * @param on   nonzero to show the box.
 * @return nonzero when the box was shown before the call.
 */
s32 BoxFill__SetDisplay(BoxFill *self, s32 on);

/**
 * @brief Turns semi-transparency on or off (the GsALON bit).
 * @param self the box.
 * @param on   nonzero for semi-transparent.
 * @return the bit's previous value.
 */
s32 BoxFill__SetSemiTrans(BoxFill *self, s32 on);

/**
 * @brief Sets the semi-transparency rate (attribute bits 28..29).
 * @param self the box.
 * @param rate the rate, 0..3 (BOXFILL_SEMITRANS_RATE of a libgs GsA* value).
 * @return the previous rate.
 */
s32 BoxFill__SetSemiTransRate(BoxFill *self, s32 rate);

/**
 * @brief Sets the box's colour, or adds to it.
 * @param self      the box.
 * @param overwrite nonzero to copy `rgb`, zero to add it byte by byte.
 * @param rgb       three bytes r, g, b.
 */
void BoxFill__SetColor(BoxFill *self, s32 overwrite, u8 *rgb);

/**
 * @brief Copies three colour bytes from `src` to `dst`, or adds them.
 * @param self      the box (unused).
 * @param dst       the three bytes to write.
 * @param src       the three bytes r, g, b to copy or add.
 * @param overwrite nonzero to copy, zero to add byte by byte (each wraps at 256).
 */
void BoxFill__ApplyColor(BoxFill *self, u8 *dst, u8 *src, s32 overwrite);

/**
 * @brief Moves an attached box to `pos`; does nothing while it is unattached.
 * @param self the box.
 * @param pos  the new position, copied into posX/posY.
 */
void BoxFill__SetPosition(BoxFill *self, BoxFillPos *pos);

/**
 * @brief Resizes an attached box; does nothing while it is unattached.
 * @param self the box.
 * @param size a BoxFillSize {w, h}; the low halfword of each is kept.
 */
void BoxFill__SetSize(BoxFill *self, s32 *size);

/**
 * @brief Attaches the box with its position in pixels rather than percent.
 *
 * Calls attachToParent, then clears `relative` and stores `attachArg`.
 * @param self      the box.
 * @param parent    the node to attach under.
 * @param pos       the box's position in pixels.
 * @param attachArg stored in `attachArg`, which nothing reads.
 */
void BoxFill__AttachAbsolute(BoxFill *self, SceneNode *parent, BoxFillPos *pos, s32 attachArg);

/**
 * @brief Sets the ordering-table priority the box is drawn at.
 * @param self the box.
 * @param pri  the priority.
 */
void BoxFill__SetPri(BoxFill *self, s32 pri);

/**
 * @brief Sets `mask` to its low `bits` bits.
 * @param self the box.
 * @param bits the number of bits set (Reset passes 13).
 * @return the new mask, (1 << bits) - 1.
 */
s32 BoxFill__SetMask(BoxFill *self, s32 bits);

#endif
