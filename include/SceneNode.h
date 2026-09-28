#ifndef SCENENODE_H
#define SCENENODE_H

#include "BasicClass.h"
#include "TmdModel.h"

/*
 * SceneNode -- a node in libgs's transform hierarchy, and the base of every
 * positioned thing the game draws (class id 0x4, table gSceneNodeMethods,
 * parent BasicClass). An instance embeds a GsDOBJ2 at +0x010 (attribute,
 * coord2, tmd, id: New_SceneNode's ctor allocates the 0x50-byte GsCOORDINATE2
 * and its 0x28-byte GsCOORD2PARAM, and SceneNode__LinkModel hands &attribute
 * to GsLinkObject4) and may hold a TmdModel child whose TMD it draws.
 *
 * Lifecycle: New_SceneNode allocates 0x44 bytes and runs the ctor, which
 * returns self or NULL when either coordinate block cannot be allocated
 * (hence BASICCLASS_SLOTS_R with a `void *` ctor), then reset gives it an
 * identity transform. Finalize detaches it from its parent and its children
 * and frees both blocks.
 *
 * Hierarchy: attachToParent sets `parent` and points coord2->super at the
 * parent's coordinate; detachFromParent clears both. The parent chain is
 * what ComposeAndApplyRotation and RaycastVertical walk.
 *
 * Transform: updateRotation and updateScale take three Ratio16s (degrees, or
 * a scale ratio) and set or add them into coord2->param, clearing coord2->flg
 * so libgs recomputes the matrix. The attribute setters (SetDisplay through
 * SetBackClip) each set one field of GsDOBJ2.attribute, named after its
 * libgs.h bit, and return the old value.
 *
 * Messages: onNotify dispatches on the SENDER's class id nibble: a Pad (2)
 * to onPadEvent, a FrameClock (5) to update, another SceneNode (4) to
 * dispatchLinkCommand, which on event 2 or 3 runs tryAttachNearby (a
 * bounds test of the sender's hull against this node) and on event 4
 * records the sender as `linkTarget`. notifyWithHull is the sending side:
 * it transforms the model's hull into world space and notifies the parents.
 *
 * Methods: src/graphics/SceneNode.c. Subclasses:
 * `python3 tools/plan.py classes` (Actor, Sprite, LightRig, BoxFill and
 * more); they expand SCENENODE_FIELDS and SCENENODE_SLOTS first.
 *
 * The coordinate blocks are Sony's own types: coord2 is a GsCOORDINATE2
 * (the offset from the parent in coord.t, the composed world matrix in
 * workm, whose workm.t is the world position), and coord2->param its
 * GsCOORD2PARAM (scale, the SVECTOR rotate in 4096ths of a turn, trans).
 * So an includer takes Sony's headers first: `common.h`, <libgte.h>,
 * <libgpu.h>, <libgs.h>.
 */

typedef struct SceneNode SceneNode;
typedef struct SceneNodeMethods SceneNodeMethods;

/* SceneNode's class id (gSceneNodeMethods word +0x000). A single nibble, so
 * `(header & CLASS_ID_ROOT_MASK) == SCENENODE_CLASS_ID` tests for it or a subclass. */
#define SCENENODE_CLASS_ID 0x4

/* Three 32-bit components (Sony's "long vector" without VECTOR's pad word;
 * arrays of it have a 0xC stride): positions, offsets and translations. */
typedef struct LongVec3 {
    s32 x;
    s32 y;
    s32 z;
} LongVec3;

/* A ratio of two s16s, num / den, which RatioToFixed12 turns into 20.12
 * fixed point. updateRotation and updateScale take three (sRotationZero,
 * sSceneNodeScaleOne); every producer in the class sets den to 1. */
typedef struct Ratio16 Ratio16;

struct Ratio16 {
    s16 num;
    s16 den;
};

/* The slots, each named for the occupant in gSceneNodeMethods; `python3
 * tools/classtable.py <subclass table> --vs gSceneNodeMethods` lists a
 * subclass's overrides. */
/* clang-format off */
#define SCENENODE_SLOTS(Self, CtorParams) \
    BASICCLASS_SLOTS_R(Self, void *, CtorParams); \
    /* +0x040 */ void (*reset)(Self *self); /* SceneNode__Reset: identity transform */ \
    /* +0x044 */ void (*updateRotation)(Self *self, s32 set, void *table); /* SceneNode__UpdateRotation: Ratio16[3] degrees; set or add */ \
    /* +0x048 */ void (*updateScale)(Self *self, s32 set, void *table); /* SceneNode__UpdateScale: Ratio16[3]; set or add */ \
    /* +0x04C */ SceneNode *(*attachToParent)(Self *self, SceneNode *parent, LongVec3 *offset); /* SceneNode__AttachToParent */ \
    /* +0x050 */ SceneNode *(*detachFromParent)(Self *self); /* SceneNode__DetachFromParent */ \
    /* +0x054 */ void (*detachAttachedChildren)(Self *self); /* SceneNode__DetachAttachedChildren */ \
    /* +0x058 */ void (*getNextAttachedChild)(Self *self, SceneNode **entry, BasicClassListNode **cursor); /* SceneNode__GetNextAttachedChild */ \
    /* +0x05C */ void (*slot5C)(Self *self, s32 arg1); /* SceneNode__NoOpSlot5C; Finalize passes (self, 0) */ \
    /* +0x060 */ s32 (*setDisplay)(Self *self, s32 on); /* SceneNode__SetDisplay: GsDOFF */ \
    /* +0x064 */ u32 (*setSemiTransOn)(Self *self, s32 on); /* SceneNode__SetSemiTrans: GsALON; not setSemiTrans, which is <libgpu.h>'s macro */ \
    /* +0x068 */ u32 (*setSemiTransRate)(Self *self, u32 rate); /* SceneNode__SetSemiTransRate: GsAZERO..GsATHREE */ \
    /* +0x06C */ u32 (*setLighting)(Self *self, s32 on); /* SceneNode__SetLighting: GsLOFF */ \
    /* +0x070 */ u32 (*setLightMode)(Self *self, u32 mode); /* SceneNode__SetLightMode: GsFOG, GsMATE, GsLLMOD */ \
    /* +0x074 */ u32 (*setLightDim)(Self *self, u32 value); /* SceneNode__SetLightDim: GsLDIM0..7 */ \
    /* +0x078 */ s32 (*setUseZ)(Self *self, s32 on); /* SceneNode__SetUseZ: GsZIGNR */ \
    /* +0x07C */ u32 (*setSubdivision)(Self *self, u32 value); /* SceneNode__SetSubdivision: GsDIV1..5 */ \
    /* +0x080 */ s32 (*setBackClip)(Self *self, s32 on); /* SceneNode__SetBackClip: GsNBACKC */ \
    /* +0x084 */ void (*getRotMatrix)(Self *self, void *out, s32 invert); /* SceneNode__GetRotMatrix: RotMatrix of the (negated) rotation into a MATRIX */ \
    /* +0x088 */ void (*notifyWithHull)(Self *self, s32 event); /* SceneNode__NotifyWithHull; Actor__NotifyMove, DreamSys__NotifyLinkAttempt */ \
    /* +0x08C */ void (*getModelHull)(Self *self, void *dest); /* SceneNode__GetModelHull: TmdModel__GetHull into dest */ \
    /* +0x090 */ void (*transformAndNotifyParents)(Self *self, TmdHull *verts, s32 event); /* SceneNode__TransformAndNotifyParents */ \
    /* +0x094 */ void (*onPadEvent)(Self *self, void *sender, s32 event); /* SceneNode__OnPadEvent, empty; DreamSys__OnPadEvent */ \
    /* +0x098 */ void (*update)(Self *self, void *sender, s32 event); /* SceneNode__Update, empty; Sprite__Update, Entity__Update, DreamSys__TimerTick */ \
    /* +0x09C */ void (*dispatchLinkCommand)(Self *self, void *sender, s32 event); /* SceneNode__DispatchLinkCommand */ \
    /* +0x0A0 */ void (*tryAttachNearby)(Self *self); /* SceneNode__TryAttachNearby; its 2nd parameter arrives as the caller's untouched $a1 */ \
    /* +0x0A4 */ void (*composeAndApplyRotation)(Self *self, void *vec, void *dst, void *src, s32 count); /* SceneNode__ComposeAndApplyRotation */ \
    /* +0x0A8 */ s32 (*checkBoundsOverlap)(Self *self, void *corners, TmdVec3 *delta); /* SceneNode__CheckBoundsOverlap */ \
    /* +0x0AC */ s32 (*raycastHullAgainstFaces)(Self *self, void *hullHits, TmdVec3 *hitPoint, void *hull); /* SceneNode__RaycastHullAgainstFaces */ \
    /* +0x0B0 */ void (*slotB0)(void); /* SceneNode__NoOpSlotB0; never called */ \
    /* +0x0B4 */ void (*addToActorParents)(Self *self, void *node) /* SceneNode__AddToActorParents */
/* clang-format on */

/* clang-format off */
#define SCENENODE_FIELDS(Methods) \
    BASICCLASS_FIELDS(Methods); \
    /* +0x00C */ SceneNode *parent; /* attachToParent/detachFromParent; the chain ComposeAndApplyRotation walks */ \
    /* +0x010 */ u32 attribute; /* GsDOBJ2.attribute: the setters' packed word (libgs.h's attribute bits) */ \
    /* +0x014 */ GsCOORDINATE2 *coord2; /* GsDOBJ2.coord2: the ctor's 0x50-byte GsCOORDINATE2 */ \
    /* +0x018 */ s32 tmd; /* GsDOBJ2.tmd: LinkModel copies the model's +0x10, UnlinkModel clears it */ \
    /* +0x01C */ s32 id; /* GsDOBJ2.id; no accessor */ \
    /* +0x020 */ void *model; /* the TmdModel child LinkModel linked; NULL when none */ \
    /* +0x024 */ s32 tick; /* zeroed by Reset; gStyleEffectMethods's update increments it */ \
    /* +0x028 */ SceneNode *linkTarget; /* dispatchLinkCommand's event-4 sender; TryAttachNearby's hit */ \
    /* +0x02C */ s32 hitMask; /* RaycastHullAgainstFaces: one bit per model bounds box (own) or hull box (the other's) */ \
    /* +0x030 */ TmdHull *notifyVerts; /* TransformAndNotifyParents's hull, set only while the parents are notified */ \
    /* +0x034 */ u16 unk34; /* zeroed by GridCell's ctor */ \
    /* +0x036 */ u16 flags36; /* bit 0x80 tested by StageMap's NotifyGridCell; zeroed by GridCell's ctor */ \
    /* +0x038 */ void *nextInCell; /* StageMap's grid-cell chain; zeroed by GridCell's ctor */ \
    /* +0x03C */ u8 pad3C[8] /* the object is 0x44 bytes (New_SceneNode) */
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
void SceneNode__NoOpSlot5C(void);
s32 SceneNode__SetDisplay(SceneNode *self, s32 on);
u32 SceneNode__SetSemiTrans(SceneNode *self, s32 on);
u32 SceneNode__SetSemiTransRate(SceneNode *self, u32 rate);
u32 SceneNode__SetLighting(SceneNode *self, s32 on);
u32 SceneNode__SetLightMode(SceneNode *self, u32 mode);
u32 SceneNode__SetLightDim(SceneNode *self, u32 value);
s32 SceneNode__SetUseZ(SceneNode *self, s32 on);
u32 SceneNode__SetSubdivision(SceneNode *self, u32 value);
s32 SceneNode__SetBackClip(SceneNode *self, s32 on);
void SceneNode__GetRotMatrix(SceneNode *self, MATRIX *out, s32 invert);
void SceneNode__NotifyWithHull(SceneNode *self, s32 event);
void SceneNode__GetModelHull(SceneNode *self, void *dest);
void SceneNode__TransformAndNotifyParents(SceneNode *self, TmdHull *verts, s32 event);
void SceneNode__OnPadEvent(void);
void SceneNode__Update(void);
void SceneNode__DispatchLinkCommand(SceneNode *self, void *sender, s32 event);
void SceneNode__TryAttachNearby(SceneNode *self, SceneNode *other);
void SceneNode__ComposeAndApplyRotation(SceneNode *self, void *vec, void *dst, void *src, s32 count);
s32 SceneNode__CheckBoundsOverlap(SceneNode *self, void *corners, TmdVec3 *delta);
s32 SceneNode__RaycastHullAgainstFaces(SceneNode *self, s32 *hullHits, TmdVec3 *hitPoint, TmdHull *hull);
void SceneNode__NoOpSlotB0(void);
void SceneNode__AddToActorParents(SceneNode *self, void *node);

void SceneNode__RotateLocalVector(SceneNode *self, LongVec3 *dst, s16 *src);
/* `unused`: both callers pass 0 and the body never reads it. */
void SceneNode__LocalOffsetToWorldPos(SceneNode *self, s32 *dst, s32 *src, s32 unused);
void SceneNode__GetRotationDegrees(SceneNode *self, Ratio16 *out);
void SceneNode__LinkModel(SceneNode *self, void *model);
void SceneNode__UnlinkModel(SceneNode *self);
void SceneNode__FaceTarget(SceneNode *self, SceneNode *target, s32 yawOnly, s32 swapped, void *extra);


/* SceneNode's free helpers, defined in src/graphics/SceneNode.c and called there by
 * symbol: the rotation and scale inputs, the matrix-over-array transforms,
 * the attribute-word accessor and the box-clipping primitives. Sony's
 * functions (RotMatrix, MulMatrix2, ApplyMatrixLV, ratan2,
 * GsInitCoordinate2, GsLinkObject4) come from <libgte.h> and <libgs.h>. */
/* A Ratio16 (num / den) as 20.12 fixed point. updateRotation and updateScale
 * apply it to each of their three entries. */
extern s32 RatioToFixed12(void *pair);

/* dst[i] = m * src[i] over `count` elements, through Sony's ApplyMatrixSV
 * (6-byte s16 vectors) and ApplyMatrixLV (0xC-byte s32 vectors). The first
 * argument is the destination; dst == src transforms in place. */
extern void ApplyMatrixToSVArray(TmdVec3 *dst, TmdVec3 *src, s32 count, MATRIX *m);
void ApplyMatrixToLVArray(void *dst, void *src, s32 count, void *m);

/* Replaces the `width` bits at bit `shift` of *word with `value` and returns
 * the field's old contents. The attribute setters (SceneNode__SetDisplay and
 * the rest) are wrappers around it over GsDOBJ2.attribute. */
extern u32 GetSetBitField(u32 *word, s32 shift, s32 width, u32 value);

/* Bit positions in GsDOBJ2.attribute (include/psyq/libgs.h), the fields the
 * SceneNode attribute setters (src/graphics/SceneNode.c) replace. */
#define ATTR_LDIM_SHIFT 0      /* GsLDIM0..GsLDIM7, 3 bits */
#define ATTR_LIGHTMODE_SHIFT 3 /* GsFOG|GsMATE|GsLLMOD, 3 bits */
#define ATTR_LOFF_SHIFT 6      /* GsLOFF */
#define ATTR_ZIGNR_SHIFT 7     /* GsZIGNR */
#define ATTR_NBACKC_SHIFT 8    /* GsNBACKC */
#define ATTR_DIV_SHIFT 9       /* GsDIV1..GsDIV5, 3 bits */
#define ATTR_ABR_SHIFT 28      /* GsAZERO..GsATHREE, 2 bits */
#define ATTR_ALON_SHIFT 30     /* GsALON */
#define ATTR_DOFF_SHIFT 31     /* GsDOFF */

/* Segment-against-box clipping for the link tests. CalcBoxOutcode returns
 * a point's 6-bit outcode against a box (x 8/4, y 2/1, z 0x20/0x10; high bit
 * past max, low bit before min), unmasked. ClipSegmentToBox returns 0 when
 * p1..p2 misses the box, 1 when both ends are inside, 2 or 3 when only p1 or
 * only p2 is, writing the boundary crossing to `out` when non-NULL.
 * BisectSegmentToBox finds that crossing by halving from the inside point
 * `near` towards the outside point `far`. */
enum ClipResult {
    CLIP_MISS = 0,      /* the segment misses the box */
    CLIP_INSIDE = 1,    /* both ends are inside */
    CLIP_P1_INSIDE = 2, /* only p1 is inside */
    CLIP_P2_INSIDE = 3  /* only p2 is inside */
};

/* CalcBoxOutcode's bits: per axis, MAX when the point is past the box's
 * maximum and MIN when it is before its minimum. */
#define OUTCODE_Y_MIN 0x01
#define OUTCODE_Y_MAX 0x02
#define OUTCODE_X_MIN 0x04
#define OUTCODE_X_MAX 0x08
#define OUTCODE_Z_MIN 0x10
#define OUTCODE_Z_MAX 0x20

void BisectSegmentToBox(TmdVec3 *out, TmdBox *box, TmdVec3 *near, TmdVec3 *far);
extern s32 CalcBoxOutcode(TmdBox *box, TmdVec3 *point);
s32 ClipSegmentToBox(TmdVec3 *out, TmdBox *box, TmdVec3 *p1, TmdVec3 *p2);

#endif
