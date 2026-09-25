#ifndef CLASS6B5CC_H
#define CLASS6B5CC_H

#include "BasicClass.h"

/*
 * Class6B5CC -- a positioned 3D scene object (class id 0x4, method table
 * gClass6B5CCMethods): a BasicClass subclass wrapping a libgs GsDOBJ2 and its
 * GsCOORDINATE2 (the identification is in include/code_d294.h's PSY-Q block).
 * Methods in src/code_d294.c, code_d294_b.c and code_d294_c.c; sixteen classes
 * derive from it (`typeviews.py --tree`), among them gActorMethods (0x34, the
 * base of Class65650, Entity and DreamSys), Class86AA0 (0x24), D_8006EAC0
 * (0x64) and D_8006EFAC (0x14, the base of Class866E8).
 *
 * Objects form a transform hierarchy: attachToParent sets `parent` and points
 * the coordinate's `super` at the parent's coordinate, and the parent chain is
 * walked through `parent` (ComposeAndApplyRotation, func_8001E7BC). The
 * inherited onNotify is split by the SENDER's class nibble: a Pad (2) goes to
 * onPadEvent, a D_8006EF50 (5) to update, another Class6B5CC (4) to
 * dispatchLinkCommand, which on event 2/3 runs tryAttachNearby and on event 4
 * records the sender as `linkTarget`.
 *
 * The object is 0x44 bytes (New_Class6B5CC). Its constructor returns self, or
 * NULL when an allocation fails, so its SLOTS expand BASICCLASS_SLOTS_R with a
 * `void *` ctor.
 */

typedef struct Class6B5CC Class6B5CC;
typedef struct Class6B5CCMethods Class6B5CCMethods;
typedef struct Class6B5CCSub14 Class6B5CCSub14;
typedef struct Class6B5CCSub44 Class6B5CCSub44;
typedef struct S16Quad_d294 S16Quad_d294;
typedef struct GenericCountList_d294 GenericCountList_d294;

/* A plain s16 quad (libgs SVECTOR). All-s16 members give it alignment 2,
 * which is what makes Class6B5CC__GetRotMatrix's whole-struct copy compile to
 * lwl/lwr (DECOMPILATION_LEARNINGS, the all-s8/s16 struct idiom). */
struct S16Quad_d294 {
    s16 x;  /* +0x000 */
    s16 y;  /* +0x002 */
    s16 z;  /* +0x004 */
    s16 w;  /* +0x006, never written by Class6B5CC__GetRotMatrix's negate path */
};

/* A plain 3-word vector: Class6B5CC__AttachToParent's optional offset, and the
 * shape of the coordinate's translations. */
typedef struct Vec3_d294 {
    s32 x;
    s32 y;
    s32 z;
} Vec3_d294;

/* A 6-byte all-s16 vector (alignment 2: whole-value copies are lwl/lwr). */
typedef struct Vec3S16_d294 {
    s16 x;
    s16 y;
    s16 z;
} Vec3S16_d294;

/* One `{whole, frac}` entry of the three-entry angle/scale tables
 * updateRotation and updateScale take (ROTATION_ZERO, SCALE_ONE) and
 * Class6B5CC__GetRotationDegrees fills; RatioToFixed12 reads one. */
typedef struct WholeFrac_d294 WholeFrac_d294;
struct WholeFrac_d294 {
    s16 whole;
    s16 frac;
};

/* GsCOORD2PARAM, 0x28 bytes: the ctor's second allocation. */
struct Class6B5CCSub44 {
    s32 scaleX;  /* +0x000, GsCOORD2PARAM.scale.vx */
    s32 scaleY;  /* +0x004, scale.vy */
    s32 scaleZ;  /* +0x008, scale.vz */
    u8 padC[0x010 - 0x00C];
    S16Quad_d294 rotate;  /* +0x010, GsCOORD2PARAM.rotate (4096 per turn) */
    u8 pad18[0x028 - 0x018];  /* +0x018, trans (VECTOR); no accessor */
};

/* GsCOORDINATE2, 0x50 bytes: the ctor's first allocation. */
struct Class6B5CCSub14 {
    s32 flg;                    /* +0x000, 0 = recompute (updateRotation/updateScale/attachToParent clear it) */
    u8 pad04[0x018 - 0x004];    /* +0x004, coord.m */
    s32 tx;                     /* +0x018, coord.t[0]: attachToParent's offset */
    s32 ty;                     /* +0x01C, coord.t[1] */
    s32 tz;                     /* +0x020, coord.t[2] */
    u8 unk24[0x038 - 0x024];    /* +0x024, workm (address only: the matrix TransformAndNotifyParents applies) */
    s32 unk38[3];               /* +0x038, workm.t: the world position, indexed per axis */
    Class6B5CCSub44 *param;     /* +0x044, GsCOORDINATE2.param */
    Class6B5CCSub14 *super;     /* +0x048, the parent's coordinate: set by attachToParent, cleared by detachFromParent */
    Class6B5CCSub14 *sub;       /* +0x04C, GsCOORDINATE2.sub; no accessor */
};

/* A count and a vertex array (count * 8 Vec3S16_d294 corners from &unk4):
 * TransformAndNotifyParents's argument, held in `notifyVerts` while the
 * parents are notified. */
struct GenericCountList_d294 {
    s32 unk0;  /* +0x000, multiplied by 8 to form ApplyMatrixToSVArray's count */
    u8 unk4;   /* +0x004, address only: the first corner */
};

/* TryAttachNearby's transformed copy of a count list: eight corners per count. */
typedef struct AttachCornerList_d294b {
    s32 count;
    Vec3S16_d294 v[8];
} AttachCornerList_d294b;

/* Occupants in gClass6B5CCMethods named at each slot; `tools/classtable.py
 * <subclass table> --vs gClass6B5CCMethods` lists a subclass's overrides. */
#define CLASS6B5CC_SLOTS(Self, CtorParams)                                                         \
    BASICCLASS_SLOTS_R(Self, void *, CtorParams);                                                  \
    /* +0x040 */ void (*reset)(Self *self);                                /* Class6B5CC__Reset */  \
    /* +0x044 */ void (*updateRotation)(Self *self, s32 set, void *table); /* Class6B5CC__UpdateRotation: WholeFrac_d294[3] degrees; set or add */ \
    /* +0x048 */ void (*updateScale)(Self *self, s32 set, void *table);    /* Class6B5CC__UpdateScale */ \
    /* +0x04C */ Class6B5CC *(*attachToParent)(Self *self, Class6B5CC *parent, Vec3_d294 *offset); /* Class6B5CC__AttachToParent */ \
    /* +0x050 */ Class6B5CC *(*detachFromParent)(Self *self);              /* Class6B5CC__DetachFromParent */ \
    /* +0x054 */ void (*detachAttachedChildren)(Self *self);               /* Class6B5CC__DetachAttachedChildren */ \
    /* +0x058 */ void (*getNextAttachedChild)(Self *self, Class6B5CC **entry, BasicClassListNode **cursor); /* Class6B5CC__GetNextAttachedChild */ \
    /* +0x05C */ void (*slot5C)(Self *self, s32 arg1);                     /* Class6B5CC__func_1d33c, empty; Finalize passes (self, 0) */ \
    /* +0x060 */ s32 (*setDisplay)(Self *self, s32 on);                    /* Class6B5CC__SetDisplay */ \
    /* +0x064 */ u32 (*setSemiTrans)(Self *self, s32 on);                  /* Class6B5CC__SetSemiTrans */ \
    /* +0x068 */ u32 (*setSemiTransRate)(Self *self, u32 rate);            /* Class6B5CC__SetSemiTransRate */ \
    /* +0x06C */ u32 (*setLighting)(Self *self, s32 on);                   /* Class6B5CC__SetLighting */ \
    /* +0x070 */ u32 (*setLightMode)(Self *self, u32 mode);                /* Class6B5CC__SetLightMode */ \
    /* +0x074 */ u32 (*getSetUnk10Field0)(Self *self, u32 value);          /* Class6B5CC__GetSetUnk10Field0 */ \
    /* +0x078 */ s32 (*getSetUnk10Flag7)(Self *self, s32 on);              /* Class6B5CC__GetSetUnk10Flag7 */ \
    /* +0x07C */ u32 (*getSetUnk10Field9)(Self *self, u32 value);          /* Class6B5CC__GetSetUnk10Field9 */ \
    /* +0x080 */ s32 (*getSetUnk10Flag8)(Self *self, s32 on);              /* Class6B5CC__GetSetUnk10Flag8 */ \
    /* +0x084 */ void (*getRotMatrix)(Self *self, void *out, s32 invert);  /* Class6B5CC__GetRotMatrix: RotMatrix of the (negated) rotation into a MATRIX */ \
    /* +0x088 */ void (*notifyIfUnk20Active)(Self *self, s32 event);       /* Class6B5CC__NotifyIfUnk20Active */ \
    /* +0x08C */ void (*readUnk20Data)(Self *self, void *dest);            /* Class6B5CC__ReadUnk20Data */ \
    /* +0x090 */ void (*transformAndNotifyParents)(Self *self, GenericCountList_d294 *verts, s32 event); /* Class6B5CC__TransformAndNotifyParents */ \
    /* +0x094 */ void (*onPadEvent)(Self *self, void *sender, s32 event);  /* func_8001D6A4, empty; onNotify's Pad (2) case; DreamSys__ApplyLinkCommand */ \
    /* +0x098 */ void (*update)(Self *self, void *sender, s32 event);      /* func_8001D6AC, empty; onNotify's D_8006EF50 (5) case; Entity__Update, DreamSys__TimerTick */ \
    /* +0x09C */ void (*dispatchLinkCommand)(Self *self, void *sender, s32 event); /* Class6B5CC__DispatchLinkCommand; onNotify's Class6B5CC (4) case */ \
    /* +0x0A0 */ void (*tryAttachNearby)(Self *self);                      /* Class6B5CC__TryAttachNearby; its 2nd parameter arrives as the caller's untouched $a1 */ \
    /* +0x0A4 */ void (*composeAndApplyRotation)(Self *self, void *vec, void *dst, void *src, s32 count); /* Class6B5CC__ComposeAndApplyRotation */ \
    /* +0x0A8 */ s32 (*checkBoundsOverlap)(Self *self, void *corners, Vec3S16_d294 *delta); /* Class6B5CC__CheckBoundsOverlap */ \
    /* +0x0AC */ s32 (*classifyAgainstPlanes)(Self *self, void *outFlag, Vec3S16_d294 *delta, void *corners); /* Class6B5CC__ClassifyAgainstPlanes */ \
    /* +0x0B0 */ void (*slotB0)(void);                                     /* func_8001E49C, empty; never called */ \
    /* +0x0B4 */ void (*notifyTaggedParents)(Self *self, void *node)       /* Class6B5CC__NotifyTaggedParents */

#define CLASS6B5CC_FIELDS(Methods)                                                                 \
    BASICCLASS_FIELDS(Methods);                                                                    \
    /* +0x00C */ Class6B5CC *parent;      /* attachToParent/detachFromParent; the chain ComposeAndApplyRotation walks */ \
    /* +0x010 */ u32 attribute;           /* GsDOBJ2.attribute: the SetDisplay/GetSetBitField family's packed word */ \
    /* +0x014 */ Class6B5CCSub14 *coord2; /* GsDOBJ2.coord2: the ctor's 0x50-byte GsCOORDINATE2 */ \
    /* +0x018 */ s32 tmd;                 /* GsDOBJ2.tmd: LinkModel copies the model's +0x10, UnlinkModel clears it */ \
    /* +0x01C */ s32 id;                  /* GsDOBJ2.id; no accessor */                            \
    /* +0x020 */ void *model;             /* the D_8006BEA0 (class 9) child LinkModel linked; NULL when none */ \
    /* +0x024 */ s32 tick;                /* zeroed by Reset; gClass876FCMethods's update increments it */ \
    /* +0x028 */ Class6B5CC *linkTarget;  /* dispatchLinkCommand's event-4 sender; TryAttachNearby's hit */ \
    /* +0x02C */ s32 hitMask;             /* ClassifyAgainstPlanes: one bit per plane (own) or corner group (the other's) */ \
    /* +0x030 */ GenericCountList_d294 *notifyVerts; /* TransformAndNotifyParents's vertices, set only while the parents are notified */ \
    /* +0x034 */ u16 unk34;               /* zeroed by Class86AA0's ctor */                        \
    /* +0x036 */ u16 flags36;             /* bit 0x80 tested by Class866E8's NotifyGridCell; zeroed by Class86AA0's ctor */ \
    /* +0x038 */ void *nextInCell;        /* Class866E8's grid-cell chain; zeroed by Class86AA0's ctor */ \
    /* +0x03C */ u8 pad3C[8]              /* the object is 0x44 bytes (New_Class6B5CC) */

struct Class6B5CCMethods {
    CLASS6B5CC_SLOTS(Class6B5CC, (Class6B5CC *self));
};

struct Class6B5CC {
    CLASS6B5CC_FIELDS(Class6B5CCMethods);
};

extern Class6B5CCMethods gClass6B5CCMethods;
extern Class6B5CCMethods *GetClass6B5CCMethods(void); /* returns &gClass6B5CCMethods */

/* The occupants of gClass6B5CCMethods, in slot order, then the class's
 * non-slot methods. A subclass reaches the base ones through
 * GetClass6B5CCMethods() and upcasts. */
Class6B5CC *New_Class6B5CC(void);
void *Class6B5CC__Class6B5CC(Class6B5CC *self);
void Class6B5CC__Finalize(Class6B5CC *self);
void Class6B5CC__AddChild(Class6B5CC *self, BasicClass *child);
void Class6B5CC__RemoveChild(Class6B5CC *self, BasicClass *child);
void Class6B5CC__RemoveAllChildren(Class6B5CC *self);
void Class6B5CC__OnNotify(Class6B5CC *self, BasicClass *sender, s32 event);
void Class6B5CC__Reset(Class6B5CC *self);
void Class6B5CC__UpdateRotation(Class6B5CC *self, s32 set, void *table);
void Class6B5CC__UpdateScale(Class6B5CC *self, s32 set, void *table);
Class6B5CC *Class6B5CC__AttachToParent(Class6B5CC *self, Class6B5CC *parent, Vec3_d294 *offset);
Class6B5CC *Class6B5CC__DetachFromParent(Class6B5CC *self);
void Class6B5CC__DetachAttachedChildren(Class6B5CC *self);
void Class6B5CC__GetNextAttachedChild(Class6B5CC *self, Class6B5CC **entry, BasicClassListNode **cursor);
void Class6B5CC__func_1d33c(void);
s32 Class6B5CC__SetDisplay(Class6B5CC *self, s32 on);
u32 Class6B5CC__SetSemiTrans(Class6B5CC *self, s32 on);
u32 Class6B5CC__SetSemiTransRate(Class6B5CC *self, u32 rate);
u32 Class6B5CC__SetLighting(Class6B5CC *self, s32 on);
u32 Class6B5CC__SetLightMode(Class6B5CC *self, u32 mode);
u32 Class6B5CC__GetSetUnk10Field0(Class6B5CC *self, u32 value);
s32 Class6B5CC__GetSetUnk10Flag7(Class6B5CC *self, s32 on);
u32 Class6B5CC__GetSetUnk10Field9(Class6B5CC *self, u32 value);
s32 Class6B5CC__GetSetUnk10Flag8(Class6B5CC *self, s32 on);
void Class6B5CC__GetRotMatrix(Class6B5CC *self, s32 out, s32 invert);
void Class6B5CC__NotifyIfUnk20Active(Class6B5CC *self, s32 event);
void Class6B5CC__ReadUnk20Data(Class6B5CC *self, void *dest);
void Class6B5CC__TransformAndNotifyParents(Class6B5CC *self, GenericCountList_d294 *verts, s32 event);
void func_8001D6A4(void);
void func_8001D6AC(void);
void Class6B5CC__DispatchLinkCommand(Class6B5CC *self, void *sender, s32 event);
void Class6B5CC__TryAttachNearby(Class6B5CC *self, Class6B5CC *other);
void Class6B5CC__ComposeAndApplyRotation(Class6B5CC *self, void *vec, void *dst, void *src, s32 count);
s32 Class6B5CC__CheckBoundsOverlap(Class6B5CC *self, void *corners, Vec3S16_d294 *delta);
s32 Class6B5CC__ClassifyAgainstPlanes(Class6B5CC *self, s32 *outFlag, Vec3S16_d294 *delta, AttachCornerList_d294b *corners);
void func_8001E49C(void);
void Class6B5CC__NotifyTaggedParents(Class6B5CC *self, void *node);

void Class6B5CC__RotateLocalVector(Class6B5CC *self, Vec3_d294 *dst, s16 *src);
void Class6B5CC__LocalOffsetToWorldPos(Class6B5CC *self, s32 *dst, s32 *src, s32 unused); /* both callers set $a3 = 0 (0x80059460, 0x8005CF7C); the body never reads it */
void Class6B5CC__GetRotationDegrees(Class6B5CC *self, WholeFrac_d294 *out);
void Class6B5CC__LinkModel(Class6B5CC *self, void *model);
void Class6B5CC__UnlinkModel(Class6B5CC *self);
void Class6B5CC__FaceTarget(Class6B5CC *self, Class6B5CC *target, s32 yawOnly, s32 swapped, void *extra);

#endif
