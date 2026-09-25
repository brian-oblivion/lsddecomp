#ifndef BOXFILL_H
#define BOXFILL_H

#include "Class6B5CC.h"

/*
 * BoxFill -- a flat-coloured screen rectangle (class id 0x64, method table
 * gBoxFillMethods): a Class6B5CC subclass holding one libgs GsBOXF.
 * Methods in src/code_2cc8c_e.c (New_BoxFill, the ctor, Reset) and
 * src/code_2cc8c_f.c (the rest, up to GetBoxFillMethods). The name is for
 * what the class does, and the evidence is this:
 *  - Viewport__DrawNode (code_2864.c) takes its own path for a node whose
 *    class-id low byte is 0x64, i.e. this class and everything below it:
 *    it computes the GsBOXF's x/y from `posX`/`posY` (as a percentage of
 *    half the screen width/height while `relative` is set, as pixels
 *    otherwise) and calls GsSortBoxFill(&box, ot, pri) with `pri`.
 *  - The class's own slots set exactly those: setColor (the GsBOXF r,g,b),
 *    setPosition, setSize (w, h), setPri, and attachAbsolute, which attaches
 *    and clears `relative`. Its setDisplay/setSemiTrans/setSemiTransRate
 *    overrides are Class6B5CC's GetSetBitField accessors at the same bit
 *    positions (31, 30, 28..29), over the GsBOXF attribute instead of the
 *    GsDOBJ2 one.
 *  - Its users: TaskCore's listView (the frame behind a scrolled list:
 *    attached at the list's position, then setSize(40, rows * 12)),
 *    GraphRoomObj's 100 plotted dots, and the style decoration boxes of
 *    class_3bb8c_m/_n (semi-transparent: setSemiTrans(1), rate 0).
 *
 * Ctor chain: BoxFill__BoxFill calls GetClass6B5CCMethods()->ctor first, so
 * the id parent (0x4) is the ctor-chain parent. One class derives from it,
 * Class6E99C (D_8006E99C, 0x164, a colour fade over the box; its views are
 * its own, in include/code_2cc8c.h), whose ctor calls this one's first
 * (Class6E99C__Class6E99C: GetBoxFillMethods()->ctor).
 *
 * Overrides whose parameter list differs from the inherited slot keep the
 * slot's type (FINISHING-PLAN track 4 step 6); a caller reaching the
 * override through the slot casts to the typedef below it (no code):
 *  - +0x040 reset: BoxFill__Reset takes the ctor's (size, color, pri) and
 *    initialises the box from them; the ctor calls it through
 *    BoxFillResetFn.
 *  - +0x04C attachToParent: BoxFill__AttachToParent's third argument is a
 *    screen position (a Pair32E99C) where the slot, Class6B5CC's, types a
 *    Vec3_d294 offset; it attaches with a NULL offset, then setPosition.
 *    Callers cast to BoxFillAttachToParentFn (class_3bb8c_m/_n,
 *    code_2cc8c_b) or, through a Class6B5CC pointer, cast the argument
 *    (Viewport__SetSubHandle). BoxFill__AttachAbsolute calls it with FOUR
 *    arguments through an unprototyped pointer (see its match report).
 * The ctor itself returns nothing where Class6B5CC's slot returns `void *`;
 * every caller ignores the value.
 *
 * The object is 0x6C bytes (New_BoxFill); Class6E99C's own fields start at
 * +0x06C.
 */

typedef struct BoxFill BoxFill;
typedef struct BoxFillMethods BoxFillMethods;
typedef struct SkipShort2 SkipShort2;
typedef struct Pair32E99C Pair32E99C;

/* The ctor's (and Reset's) size argument: two words of which only the low
 * halfwords are read (lhu at +0x000 and +0x004), into boxW and boxH. The
 * callers pass s32 pairs of their own types (class_3bb8c_n's PairXY,
 * D_8008A8E8[2]), so New_BoxFill and the ctor slot take `void *` and the
 * occupants read it as this. */
struct SkipShort2 {
    s16 x;             /* +0x000, the width */
    u8 pad2[0x004 - 0x002];
    s16 y;             /* +0x004, the height */
};

/* A two-word screen position (setPosition, attachToParent's third argument):
 * copied whole into posX/posY. Class6E99C's position stack and D_8006EB90's
 * layout loops use the same record. */
struct Pair32E99C {
    s32 a; /* +0x000, x */
    s32 b; /* +0x004, y */
};

/* Class6B5CC's slots, then this class's own. `tools/classtable.py
 * gBoxFillMethods --vs gClass6B5CCMethods` lists the overrides of the
 * inherited ones: +0x008 (BoxFill__BoxFill), +0x040 (BoxFill__Reset), +0x04C
 * (BoxFill__AttachToParent), +0x060 (BoxFill__SetDisplay), +0x064
 * (BoxFill__SetSemiTrans), +0x068 (BoxFill__SetSemiTransRate). */
#define BOXFILL_SLOTS(Self, CtorParams)                                                            \
    CLASS6B5CC_SLOTS(Self, CtorParams);                                                            \
    /* +0x0B8 */ void (*setColor)(Self *self, s32 overwrite, void *rgb);  /* BoxFill__SetColor: copy the 3 bytes, or add them when overwrite is 0 */ \
    /* +0x0BC */ void (*setPosition)(Self *self, Pair32E99C *pos);        /* BoxFill__SetPosition: only while attached */ \
    /* +0x0C0 */ void (*setSize)(Self *self, s32 *size);                  /* BoxFill__SetSize: {w, h} words, low halves; only while attached */ \
    /* +0x0C4 */ void (*attachAbsolute)(Self *self, Class6B5CC *parent, Pair32E99C *pos, s32 arg3); /* BoxFill__AttachAbsolute: attachToParent, relative = 0, unk4C = arg3 */ \
    /* +0x0C8 */ void (*setPri)(Self *self, s32 pri);                     /* BoxFill__SetPri */          \
    /* +0x0CC */ s32 (*setMask)(Self *self, s32 bits)                     /* BoxFill__SetMask: mask = (1 << bits) - 1; Reset passes 13 */

#define BOXFILL_FIELDS(Methods)                                                                    \
    CLASS6B5CC_FIELDS(Methods);                                                                    \
    /* +0x044 */ s32 pri;          /* setPri, the ctor's third argument; DrawNode's GsSortBoxFill pri (its low halfword) */ \
    /* +0x048 */ s32 relative;     /* 1 from Reset, 0 from attachAbsolute: DrawNode reads posX/posY as percent of half the screen while set */ \
    /* +0x04C */ s32 unk4C;        /* zeroed by Reset; attachAbsolute's fourth argument; no reader */ \
    /* +0x050 */ s32 posX;         /* setPosition; DrawNode's source for boxX */                   \
    /* +0x054 */ s32 posY;         /* setPosition; DrawNode's source for boxY */                   \
    /* +0x058 */ u32 boxAttribute; /* GsBOXF.attribute: the setDisplay/setSemiTrans/setSemiTransRate bits */ \
    /* +0x05C */ s16 boxX;         /* GsBOXF.x: zeroed by Reset, written by DrawNode */            \
    /* +0x05E */ s16 boxY;         /* GsBOXF.y */                                                  \
    /* +0x060 */ u16 boxW;         /* GsBOXF.w: Reset and setSize (Class6E99C reads it lhu) */    \
    /* +0x062 */ u16 boxH;         /* GsBOXF.h */                                                  \
    /* +0x064 */ u8 color[3];      /* GsBOXF.r, g, b: setColor (Reset's default D_8008A924) */     \
    /* +0x067 */ u8 pad67;                                                                         \
    /* +0x068 */ s32 mask          /* setMask. The object is 0x6C bytes (New_BoxFill) */

struct BoxFillMethods {
    BOXFILL_SLOTS(BoxFill, (BoxFill *self, void *size, void *color, s32 pri));
};

struct BoxFill {
    BOXFILL_FIELDS(BoxFillMethods);
};

extern BoxFillMethods gBoxFillMethods;
extern BoxFillMethods *GetBoxFillMethods(void); /* returns &gBoxFillMethods */

/* +0x040's and +0x04C's occupants, as a caller reaching them through the
 * inherited slots casts them (see the banner). */
typedef void (*BoxFillResetFn)(BoxFill *self, SkipShort2 *size, void *color, s32 pri);
typedef void (*BoxFillAttachToParentFn)(BoxFill *self, Class6B5CC *parent, Pair32E99C *pos);

/* The class's own methods, in ROM order (code_2cc8c_e, then code_2cc8c_f).
 * A subclass reaches the base ones through GetBoxFillMethods() and upcasts. */
BoxFill *New_BoxFill(void *size, void *color, s32 pri);
void BoxFill__BoxFill(BoxFill *self, SkipShort2 *size, void *color, s32 pri);
void BoxFill__Reset(BoxFill *self, SkipShort2 *size, void *color, s32 pri);
void BoxFill__AttachToParent(BoxFill *self, Class6B5CC *parent, Pair32E99C *pos);
s32 BoxFill__SetDisplay(BoxFill *self, s32 on);
s32 BoxFill__SetSemiTrans(BoxFill *self, s32 on);
s32 BoxFill__SetSemiTransRate(BoxFill *self, s32 rate);
void BoxFill__SetColor(BoxFill *self, s32 overwrite, u8 *rgb);
void BoxFill__ApplyColor(BoxFill *self, u8 *dst, u8 *src, s32 overwrite);
void BoxFill__SetPosition(BoxFill *self, Pair32E99C *pos);
void BoxFill__SetSize(BoxFill *self, s32 *size);
void BoxFill__AttachAbsolute(BoxFill *self, Class6B5CC *parent, Pair32E99C *pos, s32 arg3);
void BoxFill__SetPri(BoxFill *self, s32 pri);
s32 BoxFill__SetMask(BoxFill *self, s32 bits);

#endif
