#ifndef SCENENODE_H
#define SCENENODE_H

#include "BasicClass.h"
#include "TmdModel.h"

/*
 * SceneNode -- a positioned 3D scene object (class id 0x4, method table
 * gSceneNodeMethods): a BasicClass subclass wrapping a libgs GsDOBJ2 and its
 * GsCOORDINATE2 (the identification is in include/code_d294.h's PSY-Q block).
 * Methods in src/code_d294.c, code_d294_b.c and code_d294_c.c; sixteen classes
 * derive from it (`typeviews.py --tree`), among them gActorMethods (0x34, the
 * base of Class65650, Entity and DreamSys), Class86AA0 (0x24), gBoxFillMethods
 * (0x64) and LightRig (0x14, include/LightRig.h, the base of Class866E8).
 *
 * Objects form a transform hierarchy: attachToParent sets `parent` and points
 * the coordinate's `super` at the parent's coordinate, and the parent chain is
 * walked through `parent` (ComposeAndApplyRotation, func_8001E7BC). The
 * inherited onNotify is split by the SENDER's class nibble: a Pad (2) goes to
 * onPadEvent, a FrameClock (5) to update, another SceneNode (4) to
 * dispatchLinkCommand, which on event 2/3 runs tryAttachNearby and on event 4
 * records the sender as `linkTarget`.
 *
 * The object is 0x44 bytes (New_SceneNode). Its constructor returns self, or
 * NULL when an allocation fails, so its SLOTS expand BASICCLASS_SLOTS_R with a
 * `void *` ctor.
 */

typedef struct SceneNode SceneNode;
typedef struct SceneNodeMethods SceneNodeMethods;
typedef struct SceneNodeSub14 SceneNodeSub14;
typedef struct SceneNodeSub44 SceneNodeSub44;
typedef struct S16Quad_d294 S16Quad_d294;

/* A plain s16 quad (libgs SVECTOR). All-s16 members give it alignment 2,
 * which is what makes SceneNode__GetRotMatrix's whole-struct copy compile to
 * lwl/lwr (DECOMPILATION_LEARNINGS, the all-s8/s16 struct idiom). */
struct S16Quad_d294 {
    s16 x; /* +0x000 */
    s16 y; /* +0x002 */
    s16 z; /* +0x004 */
    s16 w; /* +0x006, never written by SceneNode__GetRotMatrix's negate path */
};

/* A plain 3-word vector: SceneNode__AttachToParent's optional offset, and the
 * shape of the coordinate's translations. */
typedef struct LongVec3 {
    s32 x;
    s32 y;
    s32 z;
} LongVec3;

/* One `{whole, frac}` entry of the three-entry angle/scale tables
 * updateRotation and updateScale take (ROTATION_ZERO, SCALE_ONE) and
 * SceneNode__GetRotationDegrees fills; RatioToFixed12 reads one. */
typedef struct Ratio16 Ratio16;

struct Ratio16 {
    s16 whole;
    s16 frac;
};

/* GsCOORD2PARAM, 0x28 bytes: the ctor's second allocation. */
struct SceneNodeSub44 {
    s32 scaleX; /* +0x000, GsCOORD2PARAM.scale.vx */
    s32 scaleY; /* +0x004, scale.vy */
    s32 scaleZ; /* +0x008, scale.vz */
    u8 padC[0x010 - 0x00C];
    S16Quad_d294 rotate;     /* +0x010, GsCOORD2PARAM.rotate (4096 per turn) */
    u8 pad18[0x028 - 0x018]; /* +0x018, trans (VECTOR); no accessor */
};

/* GsCOORDINATE2, 0x50 bytes: the ctor's first allocation. */
struct SceneNodeSub14 {
    s32 flg; /* +0x000, 0 = recompute (updateRotation/updateScale/attachToParent clear it) */
    u8 pad04[0x018 - 0x004]; /* +0x004, coord.m */
    s32 tx;                  /* +0x018, coord.t[0]: attachToParent's offset */
    s32 ty;                  /* +0x01C, coord.t[1] */
    s32 tz;                  /* +0x020, coord.t[2] */
    u8 unk24[0x038 - 0x024]; /* +0x024, workm (address only: the matrix TransformAndNotifyParents applies) */
    s32 unk38[3];            /* +0x038, workm.t: the world position, indexed per axis */
    SceneNodeSub44 *param; /* +0x044, GsCOORDINATE2.param */
    SceneNodeSub14 *super; /* +0x048, the parent's coordinate: set by attachToParent, cleared by detachFromParent */
    SceneNodeSub14 *sub; /* +0x04C, GsCOORDINATE2.sub; no accessor */
};

/* Occupants in gSceneNodeMethods named at each slot; `tools/classtable.py
 * <subclass table> --vs gSceneNodeMethods` lists a subclass's overrides. */
/* clang-format off */
#define SCENENODE_SLOTS(Self, CtorParams)                                                         \
    BASICCLASS_SLOTS_R(Self, void *, CtorParams);                                                  \
    /* +0x040 */ void (*reset)(Self *self);                                /* SceneNode__Reset */  \
    /* +0x044 */ void (*updateRotation)(Self *self, s32 set, void *table); /* SceneNode__UpdateRotation: Ratio16[3] degrees; set or add */ \
    /* +0x048 */ void (*updateScale)(Self *self, s32 set, void *table);    /* SceneNode__UpdateScale */ \
    /* +0x04C */ SceneNode *(*attachToParent)(Self *self, SceneNode *parent, LongVec3 *offset); /* SceneNode__AttachToParent */ \
    /* +0x050 */ SceneNode *(*detachFromParent)(Self *self);              /* SceneNode__DetachFromParent */ \
    /* +0x054 */ void (*detachAttachedChildren)(Self *self);               /* SceneNode__DetachAttachedChildren */ \
    /* +0x058 */ void (*getNextAttachedChild)(Self *self, SceneNode **entry, BasicClassListNode **cursor); /* SceneNode__GetNextAttachedChild */ \
    /* +0x05C */ void (*slot5C)(Self *self, s32 arg1);                     /* SceneNode__func_1d33c, empty; Finalize passes (self, 0) */ \
    /* +0x060 */ s32 (*setDisplay)(Self *self, s32 on);                    /* SceneNode__SetDisplay */ \
    /* +0x064 */ u32 (*setSemiTrans)(Self *self, s32 on);                  /* SceneNode__SetSemiTrans */ \
    /* +0x068 */ u32 (*setSemiTransRate)(Self *self, u32 rate);            /* SceneNode__SetSemiTransRate */ \
    /* +0x06C */ u32 (*setLighting)(Self *self, s32 on);                   /* SceneNode__SetLighting */ \
    /* +0x070 */ u32 (*setLightMode)(Self *self, u32 mode);                /* SceneNode__SetLightMode */ \
    /* +0x074 */ u32 (*getSetUnk10Field0)(Self *self, u32 value);          /* SceneNode__GetSetUnk10Field0 */ \
    /* +0x078 */ s32 (*getSetUnk10Flag7)(Self *self, s32 on);              /* SceneNode__GetSetUnk10Flag7 */ \
    /* +0x07C */ u32 (*getSetUnk10Field9)(Self *self, u32 value);          /* SceneNode__GetSetUnk10Field9 */ \
    /* +0x080 */ s32 (*getSetUnk10Flag8)(Self *self, s32 on);              /* SceneNode__GetSetUnk10Flag8 */ \
    /* +0x084 */ void (*getRotMatrix)(Self *self, void *out, s32 invert);  /* SceneNode__GetRotMatrix: RotMatrix of the (negated) rotation into a MATRIX */ \
    /* +0x088 */ void (*notifyIfUnk20Active)(Self *self, s32 event);       /* SceneNode__NotifyIfUnk20Active */ \
    /* +0x08C */ void (*readUnk20Data)(Self *self, void *dest);            /* SceneNode__ReadUnk20Data */ \
    /* +0x090 */ void (*transformAndNotifyParents)(Self *self, TmdHull *verts, s32 event); /* SceneNode__TransformAndNotifyParents */ \
    /* +0x094 */ void (*onPadEvent)(Self *self, void *sender, s32 event);  /* func_8001D6A4, empty; onNotify's Pad (2) case; DreamSys__OnPadEvent */ \
    /* +0x098 */ void (*update)(Self *self, void *sender, s32 event);      /* func_8001D6AC, empty; onNotify's FrameClock (5) case; Entity__Update, DreamSys__TimerTick */ \
    /* +0x09C */ void (*dispatchLinkCommand)(Self *self, void *sender, s32 event); /* SceneNode__DispatchLinkCommand; onNotify's SceneNode (4) case */ \
    /* +0x0A0 */ void (*tryAttachNearby)(Self *self);                      /* SceneNode__TryAttachNearby; its 2nd parameter arrives as the caller's untouched $a1 */ \
    /* +0x0A4 */ void (*composeAndApplyRotation)(Self *self, void *vec, void *dst, void *src, s32 count); /* SceneNode__ComposeAndApplyRotation */ \
    /* +0x0A8 */ s32 (*checkBoundsOverlap)(Self *self, void *corners, TmdVec3 *delta); /* SceneNode__CheckBoundsOverlap */ \
    /* +0x0AC */ s32 (*classifyAgainstPlanes)(Self *self, void *outFlag, TmdVec3 *delta, void *corners); /* SceneNode__ClassifyAgainstPlanes */ \
    /* +0x0B0 */ void (*slotB0)(void);                                     /* func_8001E49C, empty; never called */ \
    /* +0x0B4 */ void (*notifyTaggedParents)(Self *self, void *node)       /* SceneNode__NotifyTaggedParents */
/* clang-format on */

/* clang-format off */
#define SCENENODE_FIELDS(Methods)                                                                 \
    BASICCLASS_FIELDS(Methods);                                                                    \
    /* +0x00C */ SceneNode *parent;      /* attachToParent/detachFromParent; the chain ComposeAndApplyRotation walks */ \
    /* +0x010 */ u32 attribute;           /* GsDOBJ2.attribute: the SetDisplay/GetSetBitField family's packed word */ \
    /* +0x014 */ SceneNodeSub14 *coord2; /* GsDOBJ2.coord2: the ctor's 0x50-byte GsCOORDINATE2 */ \
    /* +0x018 */ s32 tmd;                 /* GsDOBJ2.tmd: LinkModel copies the model's +0x10, UnlinkModel clears it */ \
    /* +0x01C */ s32 id;                  /* GsDOBJ2.id; no accessor */                            \
    /* +0x020 */ void *model;             /* the gTmdModelMethods (class 9) child LinkModel linked; NULL when none */ \
    /* +0x024 */ s32 tick;                /* zeroed by Reset; gClass876FCMethods's update increments it */ \
    /* +0x028 */ SceneNode *linkTarget;  /* dispatchLinkCommand's event-4 sender; TryAttachNearby's hit */ \
    /* +0x02C */ s32 hitMask;             /* ClassifyAgainstPlanes: one bit per plane (own) or corner group (the other's) */ \
    /* +0x030 */ TmdHull *notifyVerts; /* TransformAndNotifyParents's vertices, set only while the parents are notified */ \
    /* +0x034 */ u16 unk34;               /* zeroed by Class86AA0's ctor */                        \
    /* +0x036 */ u16 flags36;             /* bit 0x80 tested by Class866E8's NotifyGridCell; zeroed by Class86AA0's ctor */ \
    /* +0x038 */ void *nextInCell;        /* Class866E8's grid-cell chain; zeroed by Class86AA0's ctor */ \
    /* +0x03C */ u8 pad3C[8]              /* the object is 0x44 bytes (New_SceneNode) */
/* clang-format on */

struct SceneNodeMethods {
    SCENENODE_SLOTS(SceneNode, (SceneNode * self));
};

struct SceneNode {
    SCENENODE_FIELDS(SceneNodeMethods);
};

extern SceneNodeMethods gSceneNodeMethods;
extern SceneNodeMethods *GetSceneNodeMethods(void); /* returns &gSceneNodeMethods */

/* The occupants of gSceneNodeMethods, in slot order, then the class's
 * non-slot methods. A subclass reaches the base ones through
 * GetSceneNodeMethods() and upcasts. */
SceneNode *New_SceneNode(void);
void *SceneNode__SceneNode(SceneNode *self);
void SceneNode__Finalize(SceneNode *self);
void SceneNode__AddChild(SceneNode *self, BasicClass *child);
void SceneNode__RemoveChild(SceneNode *self, BasicClass *child);
void SceneNode__RemoveAllChildren(SceneNode *self);
void SceneNode__OnNotify(SceneNode *self, BasicClass *sender, s32 event);
void SceneNode__Reset(SceneNode *self);
void SceneNode__UpdateRotation(SceneNode *self, s32 set, void *table);
void SceneNode__UpdateScale(SceneNode *self, s32 set, void *table);
SceneNode *SceneNode__AttachToParent(SceneNode *self, SceneNode *parent, LongVec3 *offset);
SceneNode *SceneNode__DetachFromParent(SceneNode *self);
void SceneNode__DetachAttachedChildren(SceneNode *self);
void SceneNode__GetNextAttachedChild(SceneNode *self, SceneNode **entry, BasicClassListNode **cursor);
void SceneNode__func_1d33c(void);
s32 SceneNode__SetDisplay(SceneNode *self, s32 on);
u32 SceneNode__SetSemiTrans(SceneNode *self, s32 on);
u32 SceneNode__SetSemiTransRate(SceneNode *self, u32 rate);
u32 SceneNode__SetLighting(SceneNode *self, s32 on);
u32 SceneNode__SetLightMode(SceneNode *self, u32 mode);
u32 SceneNode__GetSetUnk10Field0(SceneNode *self, u32 value);
s32 SceneNode__GetSetUnk10Flag7(SceneNode *self, s32 on);
u32 SceneNode__GetSetUnk10Field9(SceneNode *self, u32 value);
s32 SceneNode__GetSetUnk10Flag8(SceneNode *self, s32 on);
void SceneNode__GetRotMatrix(SceneNode *self, s32 out, s32 invert);
void SceneNode__NotifyIfUnk20Active(SceneNode *self, s32 event);
void SceneNode__ReadUnk20Data(SceneNode *self, void *dest);
void SceneNode__TransformAndNotifyParents(SceneNode *self, TmdHull *verts, s32 event);
void func_8001D6A4(void);
void func_8001D6AC(void);
void SceneNode__DispatchLinkCommand(SceneNode *self, void *sender, s32 event);
void SceneNode__TryAttachNearby(SceneNode *self, SceneNode *other);
void SceneNode__ComposeAndApplyRotation(SceneNode *self, void *vec, void *dst, void *src, s32 count);
s32 SceneNode__CheckBoundsOverlap(SceneNode *self, void *corners, TmdVec3 *delta);
s32 SceneNode__ClassifyAgainstPlanes(SceneNode *self, s32 *outFlag, TmdVec3 *delta, TmdHull *corners);
void func_8001E49C(void);
void SceneNode__NotifyTaggedParents(SceneNode *self, void *node);

void SceneNode__RotateLocalVector(SceneNode *self, LongVec3 *dst, s16 *src);
void SceneNode__LocalOffsetToWorldPos(SceneNode *self, s32 *dst, s32 *src,
                                      s32 unused); /* both callers set $a3 = 0 (0x80059460, 0x8005CF7C); the body never reads it */
void SceneNode__GetRotationDegrees(SceneNode *self, Ratio16 *out);
void SceneNode__LinkModel(SceneNode *self, void *model);
void SceneNode__UnlinkModel(SceneNode *self);
void SceneNode__FaceTarget(SceneNode *self, SceneNode *target, s32 yawOnly, s32 swapped, void *extra);

#endif
