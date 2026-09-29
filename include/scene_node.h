#ifndef SCENE_NODE_H
#define SCENE_NODE_H

#include "common.h"
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include "basic_class.h"
#include "tmd_model.h"
#include "draw_system.h"

/**
 * @file scene_node.h
 * @brief SceneNode, the base class of everything the game positions and
 *        draws, and the vector, bit-field and box-clipping helpers its
 *        methods use.
 *
 * Hierarchy: attachToParent sets `parent` and points coord2->super at the
 * parent's coordinate; detachFromParent clears both. The parent chain is
 * what ComposeAndApplyRotation and RaycastVertical walk.
 *
 * Transform: updateRotation and updateScale take three Ratio16s (degrees,
 * or a scale ratio) and set or add them into coord2->param, clearing
 * coord2->flg so libgs recomputes the matrix. The attribute setters
 * (SetDisplay through SetBackClip) each set one field of GsDOBJ2.attribute,
 * named after its libgs.h bit, and return the old value.
 *
 * Messages: onNotify dispatches on the SENDER's class id nibble: a Pad (2)
 * to onPadEvent, a FrameClock (5) to update, another SceneNode (4) to
 * dispatchLinkCommand, which on a hull event runs tryAttachNearby (a
 * bounds test of the sender's hull against this node) and on
 * SCENENODE_EVENT_LINKED records the sender as `linkTarget`. notifyWithHull
 * is the sending side: it transforms the model's hull into world space and
 * notifies the parents.
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

/** SceneNode's class id (gSceneNodeMethods word +0x000). A single nibble, so
 * `(header & CLASS_ID_ROOT_MASK) == SCENENODE_CLASS_ID` tests for it or a subclass. */
#define SCENENODE_CLASS_ID 0x4

/**
 * The link test's events, between two SceneNodes (SceneNode__NotifyWithHull
 * sends, SceneNode__DispatchLinkCommand receives). A node notifies its
 * parents with its hull in notifyVerts on an event in HULL_FIRST..HULL_LAST,
 * which SceneNode treats alike; a receiver whose model the hull touches
 * answers the sender with LINKED. Actor adds its move events after these
 * (include/actor.h).
 */
enum SceneNodeLinkEvent {
    SCENENODE_EVENT_HULL_FIRST = 2, /**< first hull event: run the link test */
    SCENENODE_EVENT_HULL_LAST = 3,  /**< last hull event: run the link test */
    SCENENODE_EVENT_LINKED = 4      /**< the receiver's answer: the hull touched its model */
};

/**
 * Three 32-bit components (Sony's "long vector" without VECTOR's pad word;
 * arrays of it have a 0xC stride): positions, offsets and translations.
 */
typedef struct LongVec3 {
    s32 x; /**< x component */
    s32 y; /**< y component */
    s32 z; /**< z component */
} LongVec3;

typedef struct Ratio16 Ratio16;

/**
 * A ratio of two s16s, num / den, which RatioToFixed12 turns into 20.12
 * fixed point. updateRotation and updateScale take three (sRotationZero,
 * sSceneNodeScaleOne); every producer in the class sets den to 1.
 */
struct Ratio16 {
    s16 num; /**< numerator */
    s16 den; /**< denominator; never 0 */
};

/**
 * SceneNode's method slots, for SceneNodeMethods and every subclass's table
 * to expand first. Each slot is named for its occupant in gSceneNodeMethods,
 * whose prototype below documents it; a subclass overrides some of them
 * (its own header lists which). CtorParams is the subclass's ctor parameter
 * list; SceneNode's ctor returns `void *` (self, or NULL on failure).
 */
/* clang-format off */
#define SCENENODE_SLOTS(Self, CtorParams) \
    BASICCLASS_SLOTS_R(Self, void *, CtorParams); \
    /* +0x040 */ void (*reset)(Self *self); /* SceneNode__Reset */ \
    /* +0x044 */ void (*updateRotation)(Self *self, s32 set, void *table); /* SceneNode__UpdateRotation */ \
    /* +0x048 */ void (*updateScale)(Self *self, s32 set, void *table); /* SceneNode__UpdateScale */ \
    /* +0x04C */ SceneNode *(*attachToParent)(Self *self, SceneNode *parent, LongVec3 *offset); /* SceneNode__AttachToParent */ \
    /* +0x050 */ SceneNode *(*detachFromParent)(Self *self); /* SceneNode__DetachFromParent */ \
    /* +0x054 */ void (*detachAttachedChildren)(Self *self); /* SceneNode__DetachAttachedChildren */ \
    /* +0x058 */ void (*getNextAttachedChild)(Self *self, SceneNode **entry, BasicClassListNode **cursor); /* SceneNode__GetNextAttachedChild */ \
    /* +0x05C */ void (*finalizeHook)(Self *self, s32 arg1); /* SceneNode__NoOpSlot5C; Finalize calls it with (self, 0); no subclass overrides it */ \
    /* +0x060 */ s32 (*setDisplay)(Self *self, s32 on); /* SceneNode__SetDisplay */ \
    /* +0x064 */ u32 (*setSemiTransOn)(Self *self, s32 on); /* SceneNode__SetSemiTrans; not setSemiTrans, which is <libgpu.h>'s macro */ \
    /* +0x068 */ u32 (*setSemiTransRate)(Self *self, u32 rate); /* SceneNode__SetSemiTransRate */ \
    /* +0x06C */ u32 (*setLighting)(Self *self, s32 on); /* SceneNode__SetLighting */ \
    /* +0x070 */ u32 (*setLightMode)(Self *self, u32 mode); /* SceneNode__SetLightMode */ \
    /* +0x074 */ u32 (*setLightDim)(Self *self, u32 value); /* SceneNode__SetLightDim */ \
    /* +0x078 */ s32 (*setUseZ)(Self *self, s32 on); /* SceneNode__SetUseZ */ \
    /* +0x07C */ u32 (*setSubdivision)(Self *self, u32 value); /* SceneNode__SetSubdivision */ \
    /* +0x080 */ s32 (*setBackClip)(Self *self, s32 on); /* SceneNode__SetBackClip */ \
    /* +0x084 */ void (*getRotMatrix)(Self *self, void *out, s32 invert); /* SceneNode__GetRotMatrix */ \
    /* +0x088 */ void (*notifyWithHull)(Self *self, s32 event); /* SceneNode__NotifyWithHull; overridden by Actor__NotifyMove, DreamSys__NotifyLinkAttempt */ \
    /* +0x08C */ void (*getModelHull)(Self *self, void *dest); /* SceneNode__GetModelHull */ \
    /* +0x090 */ void (*transformAndNotifyParents)(Self *self, TmdHull *verts, s32 event); /* SceneNode__TransformAndNotifyParents */ \
    /* +0x094 */ void (*onPadEvent)(Self *self, void *sender, s32 event); /* SceneNode__OnPadEvent (empty); overridden by DreamSys__OnPadEvent */ \
    /* +0x098 */ void (*update)(Self *self, void *sender, s32 event); /* SceneNode__Update (empty); overridden by Sprite__Update, Entity__Update, DreamSys__TimerTick */ \
    /* +0x09C */ void (*dispatchLinkCommand)(Self *self, void *sender, s32 event); /* SceneNode__DispatchLinkCommand */ \
    /* +0x0A0 */ void (*tryAttachNearby)(Self *self); /* SceneNode__TryAttachNearby, which also takes `other` (see its prototype) */ \
    /* +0x0A4 */ void (*composeAndApplyRotation)(Self *self, void *vec, void *dst, void *src, s32 count); /* SceneNode__ComposeAndApplyRotation */ \
    /* +0x0A8 */ s32 (*checkBoundsOverlap)(Self *self, void *corners, TmdVec3 *delta); /* SceneNode__CheckBoundsOverlap */ \
    /* +0x0AC */ s32 (*raycastHullAgainstFaces)(Self *self, void *hullHits, TmdVec3 *hitPoint, void *hull); /* SceneNode__RaycastHullAgainstFaces */ \
    /* +0x0B0 */ void (*slotB0)(void); /* SceneNode__NoOpSlotB0; never called */ \
    /* +0x0B4 */ void (*addToActorParents)(Self *self, void *node) /* SceneNode__AddToActorParents */
/* clang-format on */

/**
 * SceneNode's fields, for SceneNode and every subclass's object to expand
 * first, so a subclass reads them flat. +0x010..+0x01F is Sony's GsDOBJ2
 * (attribute, coord2, tmd, id), which SceneNode__LinkModel hands to
 * GsLinkObject4 as one struct.
 */
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
    /* +0x028 */ SceneNode *linkTarget; /* dispatchLinkCommand's SCENENODE_EVENT_LINKED sender; TryAttachNearby's hit */ \
    /* +0x02C */ s32 hitMask; /* RaycastHullAgainstFaces: one bit per model bounds box (own) or hull box (the other's) */ \
    /* +0x030 */ TmdHull *notifyVerts; /* TransformAndNotifyParents's hull, set only while the parents are notified */ \
    /* +0x034 */ u16 unk34; /* zeroed by GridCell's ctor */ \
    /* +0x036 */ u16 flags36; /* bit 0x80 tested by StageMap's NotifyGridCell; zeroed by GridCell's ctor */ \
    /* +0x038 */ void *nextInCell; /* StageMap's grid-cell chain; zeroed by GridCell's ctor */ \
    /* +0x03C */ u8 pad3C[8] /* the object is 0x44 bytes (New_SceneNode) */
/* clang-format on */

/** SceneNode's method table: BasicClass's slots, then SCENENODE_SLOTS. */
struct SceneNodeMethods {
    SCENENODE_SLOTS(SceneNode, (SceneNode * self));
};

/**
 * SceneNode: a node in libgs's transform hierarchy, and the base of every
 * positioned thing the game draws. Class id 0x4 (SCENENODE_CLASS_ID), table
 * gSceneNodeMethods, parent BasicClass; methods in src/graphics/scene_node.c.
 * Subclasses (Actor, Sprite, LightRig, BoxFill and more) expand
 * SCENENODE_FIELDS and SCENENODE_SLOTS first.
 *
 * An instance embeds a GsDOBJ2 at +0x010 and may hold a TmdModel child whose
 * TMD it draws. Lifecycle: New_SceneNode allocates 0x44 bytes and runs the
 * ctor, which allocates the GsCOORDINATE2 and its GsCOORD2PARAM and returns
 * self, or NULL when either allocation fails (hence BASICCLASS_SLOTS_R with a
 * `void *` ctor); reset then gives it an identity transform. Finalize
 * detaches it from its parent and its children and frees both blocks.
 */
struct SceneNode {
    SCENENODE_FIELDS(SceneNodeMethods);
};

/** SceneNode's method table (class id 0x4). */
extern SceneNodeMethods gSceneNodeMethods;

/**
 * @brief The SceneNode method table, through which subclasses reach the base
 *        methods.
 * @return &gSceneNodeMethods.
 */
extern SceneNodeMethods *GetSceneNodeMethods(void);

/* The occupants of gSceneNodeMethods, in slot order, then the class's
 * non-slot methods. A subclass reaches the base ones through
 * GetSceneNodeMethods() and upcasts. */

/**
 * @brief Allocates a SceneNode and runs its ctor through the table.
 * @return The new node, or NULL when the allocation or the ctor fails (the
 *         allocation is freed then).
 */
SceneNode *New_SceneNode(void);

/**
 * @brief Constructor (slot +0x008): allocates the node's GsCOORDINATE2 and its
 *        GsCOORD2PARAM, runs BasicClass's ctor, installs gSceneNodeMethods,
 *        clears parent, model and tmd, then calls reset.
 * @param self The node.
 * @return self, or NULL when either coordinate block cannot be allocated
 *         (the first is freed again then).
 */
void *SceneNode__SceneNode(SceneNode *self);

/**
 * @brief Finalizer (slot +0x00C): detaches the node from its parent and from
 *        the children attached to it, calls finalizeHook(self, 0), frees
 *        both coordinate blocks, then runs BasicClass's finalize.
 * @param self The node.
 */
void SceneNode__Finalize(SceneNode *self);

/**
 * @brief Adds a child (BasicClass's addChild); a TmdModel child also becomes
 *        the node's model (SceneNode__LinkModel).
 * @param self The node.
 * @param child The child to add.
 */
void SceneNode__AddChild(SceneNode *self, BasicClass *child);

/**
 * @brief Removes a child (BasicClass's removeChild); removing a TmdModel
 *        child first unlinks the model (SceneNode__UnlinkModel).
 * @param self The node.
 * @param child The child to remove.
 */
void SceneNode__RemoveChild(SceneNode *self, BasicClass *child);

/**
 * @brief Unlinks the model, then removes every child (BasicClass's
 *        removeAllChildren).
 * @param self The node.
 */
void SceneNode__RemoveAllChildren(SceneNode *self);

/**
 * @brief Message handler: BasicClass's onNotify, then by the sender's class
 *        id: a Pad's event to onPadEvent, a FrameClock's to update, another
 *        SceneNode's to dispatchLinkCommand.
 * @param self The node.
 * @param sender The object that sent the event.
 * @param event The event.
 */
void SceneNode__OnNotify(SceneNode *self, BasicClass *sender, s32 event);

/**
 * @brief Identity transform: tick and attribute to 0, GsInitCoordinate2 on
 *        coord2, rotation set to zero and scale to one (updateRotation and
 *        updateScale), and coord2->flg left at 1.
 * @param self The node.
 */
void SceneNode__Reset(SceneNode *self);

/**
 * @brief Sets or adds a rotation: three Ratio16s, degrees about x, y and z,
 *        converted to 4096ths of a turn. Added angles wrap at a full turn.
 *        Clears coord2->flg so libgs recomputes the matrix.
 * @param self The node.
 * @param set Non-zero to store the angles, 0 to add them to the current ones.
 * @param table Ratio16[3]: the x, y and z angles in degrees.
 */
void SceneNode__UpdateRotation(SceneNode *self, s32 set, void *table);

/**
 * @brief Sets or adds a scale: three Ratio16s, the x, y and z scale as ratios
 *        (1/1 is unit scale), stored as 20.12 fixed point in
 *        coord2->param->scale. Clears coord2->flg.
 * @param self The node.
 * @param set Non-zero to store the scale, 0 to add it to the current one.
 * @param table Ratio16[3]: the x, y and z scale.
 */
void SceneNode__UpdateScale(SceneNode *self, s32 set, void *table);

/**
 * @brief Attaches the node under `parent` when it has no parent yet: records
 *        it, chains coord2 under the parent's, adds the node to the parent's
 *        children and sets the offset from the parent. Does nothing when the
 *        node already has a parent.
 * @param self The node.
 * @param parent The node to attach to.
 * @param offset The offset from the parent, or NULL for zero.
 * @return self.
 */
SceneNode *SceneNode__AttachToParent(SceneNode *self, SceneNode *parent, LongVec3 *offset);

/**
 * @brief Detaches the node from its parent, if it has one: removes it from the
 *        parent's children and clears parent and coord2->super.
 * @param self The node.
 * @return self.
 */
SceneNode *SceneNode__DetachFromParent(SceneNode *self);

/**
 * @brief Detaches every child that is a SceneNode attached to this node
 *        (walked with getNextAttachedChild).
 * @param self The node.
 */
void SceneNode__DetachAttachedChildren(SceneNode *self);

/**
 * @brief Iterator over the node's children that are SceneNodes attached to it
 *        (their parent is self). Start with *child NULL; each call advances.
 * @param self The node.
 * @param child In: the previous child, or NULL to start. Out: the next one,
 *        or NULL when there are no more.
 * @param cursor The walk's position in self->children; NULL once the list is
 *        done.
 */
void SceneNode__GetNextAttachedChild(SceneNode *self, SceneNode **child, BasicClassListNode **cursor);

/**
 * @brief Slot +0x05C: empty. Finalize calls it with (self, 0), and no subclass
 *        overrides it.
 */
void SceneNode__NoOpSlot5C(void);

/**
 * @brief Shows or hides the node: writes !on to GsDOFF.
 * @param self The node.
 * @param on Non-zero to display the node.
 * @return Non-zero when the node was displayed before.
 */
s32 SceneNode__SetDisplay(SceneNode *self, s32 on);

/**
 * @brief Turns semitransparency on or off (GsALON).
 * @param self The node.
 * @param on Non-zero for semitransparency on.
 * @return The old GsALON bit.
 */
u32 SceneNode__SetSemiTrans(SceneNode *self, s32 on);

/**
 * @brief Sets the semitransparency rate field (2 bits, GsAZERO..GsATHREE).
 * @param self The node.
 * @param rate The new rate, 0..3.
 * @return The old rate.
 */
u32 SceneNode__SetSemiTransRate(SceneNode *self, u32 rate);

/**
 * @brief Turns lighting on or off: writes !on to GsLOFF.
 * @param self The node.
 * @param on Non-zero for lighting on.
 * @return The old GsLOFF bit (non-zero when lighting was off).
 */
u32 SceneNode__SetLighting(SceneNode *self, s32 on);

/**
 * @brief Sets the light-mode field (3 bits: GsFOG, GsMATE, GsLLMOD).
 * @param self The node.
 * @param mode The new field.
 * @return The old field.
 */
u32 SceneNode__SetLightMode(SceneNode *self, u32 mode);

/**
 * @brief Sets the light-dimming field (3 bits, GsLDIM0..GsLDIM7).
 * @param self The node.
 * @param value The new field.
 * @return The old field.
 */
u32 SceneNode__SetLightDim(SceneNode *self, u32 value);

/**
 * @brief Turns Z sorting on or off: writes !on to GsZIGNR.
 * @param self The node.
 * @param on Non-zero for the node to be Z-sorted.
 * @return Non-zero when it was Z-sorted before.
 */
s32 SceneNode__SetUseZ(SceneNode *self, s32 on);

/**
 * @brief Sets the polygon subdivision field (3 bits, GsDIV1..GsDIV5).
 * @param self The node.
 * @param value The new field.
 * @return The old field.
 */
u32 SceneNode__SetSubdivision(SceneNode *self, u32 value);

/**
 * @brief Turns back-face clipping on or off: writes !on to GsNBACKC.
 * @param self The node.
 * @param on Non-zero for back faces to be clipped.
 * @return Non-zero when they were clipped before.
 */
s32 SceneNode__SetBackClip(SceneNode *self, s32 on);

/**
 * @brief The rotation matrix of the node's angles (GsCOORD2PARAM.rotate), or
 *        of their negation, through RotMatrix.
 * @param self The node.
 * @param out The matrix to fill.
 * @param invert Non-zero to negate each angle first.
 */
void SceneNode__GetRotMatrix(SceneNode *self, MATRIX *out, s32 invert);

/**
 * @brief The sending side of the link test. For a hull event only
 *        (SCENENODE_EVENT_HULL_FIRST..HULL_LAST), and only when the node has
 *        a model with bounds, fetches the model's hull (getModelHull) and
 *        hands it to transformAndNotifyParents; other events are dropped.
 *        Overrides handle their own events first, then call this.
 * @param self The node.
 * @param event The event.
 */
void SceneNode__NotifyWithHull(SceneNode *self, s32 event);

/**
 * @brief The linked model's hull: its bounding boxes as eight corners each
 *        (TmdModel__GetHull).
 * @param self The node; must have a model.
 * @param dest The TmdHull to fill.
 */
void SceneNode__GetModelHull(SceneNode *self, void *dest);

/**
 * @brief Rotates the hull's corners in place by the node's world matrix,
 *        clears linkTarget and hitMask, and notifies the parents with `event`
 *        while notifyVerts points at the hull (a receiver's TryAttachNearby
 *        reads it there).
 * @param self The node.
 * @param verts The hull, in the node's model space; rotated in place.
 * @param event The event to send.
 */
void SceneNode__TransformAndNotifyParents(SceneNode *self, TmdHull *verts, s32 event);

/**
 * @brief Slot +0x094 (onPadEvent): empty. The slot passes (self, sender,
 *        event); DreamSys__OnPadEvent overrides it.
 */
void SceneNode__OnPadEvent(void);

/**
 * @brief Slot +0x098 (update), a FrameClock's event: empty. The slot passes
 *        (self, sender, event); Sprite__Update, Entity__Update and
 *        DreamSys__TimerTick override it.
 */
void SceneNode__Update(void);

/**
 * @brief The receiving side of the link test, reached from OnNotify for a
 *        SceneNode sender. A hull event runs tryAttachNearby on the sender;
 *        SCENENODE_EVENT_LINKED records the sender as linkTarget.
 * @param self The node.
 * @param sender The SceneNode that sent the event.
 * @param event The event.
 */
void SceneNode__DispatchLinkCommand(SceneNode *self, void *sender, s32 event);

/**
 * @brief Tests whether `other`'s hull (its notifyVerts) touches this node's
 *        model and, if so, links the two. The world positions must be within
 *        16384 of each other on every axis; the hull and the offset between
 *        them are then rotated into this node's frame
 *        (composeAndApplyRotation), and the moved hull must overlap the
 *        model's bounds (checkBoundsOverlap) and hit one of its faces
 *        (raycastHullAgainstFaces, which fills other->hitMask). On a hit this
 *        node's linkTarget becomes `other`, and `other` is sent
 *        SCENENODE_EVENT_LINKED.
 *
 * The slot is typed with self alone: SceneNode__DispatchLinkCommand calls it
 * with self only, and `other` is then the `sender` that DispatchLinkCommand
 * itself was given. Both nodes must have a parent (a world position); the
 * code does not guard that case.
 * @param self The receiving node.
 * @param other The node whose hull is tested.
 */
void SceneNode__TryAttachNearby(SceneNode *self, SceneNode *other);

/**
 * @brief Composes the node's negated-angle rotation (getRotMatrix with invert)
 *        with every ancestor's, each multiplied in on the left, and applies it
 *        to `count` corners and optionally to one more vector. TryAttachNearby
 *        and RaycastVertical bring world-space vectors into the node's frame
 *        with it.
 * @param self The node.
 * @param vec A TmdVec3 rotated in place, or NULL.
 * @param dst The TmdVec3 array to write.
 * @param src The TmdVec3 array to read; may equal dst.
 * @param count The number of corners in src.
 */
void SceneNode__ComposeAndApplyRotation(SceneNode *self, void *vec, void *dst, void *src, s32 count);

/**
 * @brief Moves every corner of the hull by `delta`, in place, and tests the
 *        box around the moved corners against the box around all of the
 *        model's bounds records (refreshed first, TmdModel__UpdateBoundsBuffer).
 * @param self The node; must have a model.
 * @param corners The TmdHull, moved in place.
 * @param delta The offset to add to every corner.
 * @return 1 when the two boxes overlap, else 0.
 */
s32 SceneNode__CheckBoundsOverlap(SceneNode *self, void *corners, TmdVec3 *delta);

/**
 * @brief Ray-casts segments of the hull through the model's faces
 *        (TmdModel__RaycastFaces), each only against the bounds records the
 *        segment crosses (ClipSegmentToBox). Clears and fills self->hitMask,
 *        one bit per bounds record hit.
 *
 * First the centre line, from the centre of the hull's first face to the
 * centre of its second: a hit counts (above a height of 512 only while
 * GetSetHitHeightGate's flag is set) and sets *hullHits to 1. With no
 * centre-line hit, the edges from corners 1 and 2 of each box's first face to
 * the second face are cast: a hit above a height of 512 sets bit k of
 * *hullHits for box k.
 * @param self The node; must have a model.
 * @param hullHits Out: 1 for a centre-line hit, else a mask of the hull
 *        boxes whose edges hit.
 * @param hitPoint Out: the nearest hit point.
 * @param hull The hull, in the node's frame.
 * @return Non-zero when anything was hit.
 */
s32 SceneNode__RaycastHullAgainstFaces(SceneNode *self, s32 *hullHits, TmdVec3 *hitPoint, TmdHull *hull);

/** @brief Slot +0x0B0: empty, and nothing calls it. */
void SceneNode__NoOpSlotB0(void);

/**
 * @brief Adds this node as a child of every SceneNode parent of `node` whose
 *        class id's low byte is ACTOR_CLASS_ID (an Actor or a class below it).
 * @param self The node to add.
 * @param node The BasicClass whose parents are walked.
 */
void SceneNode__AddToActorParents(SceneNode *self, void *node);

/**
 * @brief Rotates a vector from the node's own frame into its parent's: `src`
 *        rotated by the node's rotation and widened to s32.
 * @param self The node.
 * @param dst Out: the rotated vector.
 * @param src Three s16s in the node's frame.
 */
void SceneNode__RotateLocalVector(SceneNode *self, LongVec3 *dst, s16 *src);

/**
 * @brief Turns an offset in the node's own frame into a world position: `src`
 *        rotated by the node's rotation plus the node's world position
 *        (coord2->workm.t). The node must have a parent.
 * @param self The node.
 * @param dst Out: three s32s, the world position.
 * @param src Three s32s, the offset in the node's frame.
 * @param unused Not read; both callers pass 0.
 */
void SceneNode__LocalOffsetToWorldPos(SceneNode *self, s32 *dst, s32 *src, s32 unused);

/**
 * @brief The node's rotation as three Ratio16s in whole degrees over 1 (the
 *        shape updateRotation takes), rounded down.
 * @param self The node.
 * @param out Ratio16[3]: the x, y and z angles.
 */
void SceneNode__GetRotationDegrees(SceneNode *self, Ratio16 *out);

/**
 * @brief Makes `model` the node's model: keeps it, puts its TMD object in
 *        GsDOBJ2.tmd, and links object 0 of its TMD to the node's GsDOBJ2
 *        (GsLinkObject4).
 * @param self The node.
 * @param model The TmdModel.
 */
void SceneNode__LinkModel(SceneNode *self, void *model);

/**
 * @brief Clears what LinkModel set, GsDOBJ2.tmd and the model; no libgs call.
 * @param self The node.
 */
void SceneNode__UnlinkModel(SceneNode *self);

/**
 * @brief Casts a vertical ray through `target` against the node's model, 1024
 *        units down and then, on a miss, 1024 up, in the node's frame.
 *
 * A node with GsDOFF set and a parent first recomputes its world position
 * (workm.t) as its own coord.t plus every ancestor's (translation only).
 * @param self The node; must have a parent.
 * @param offset Out, on a hit: three s32s, the hit point less `target`, in
 *        the node's frame.
 * @param target Three s32s, the world position the ray starts from.
 * @return 1 on a hit; 0 with no model or no hit.
 */
s32 SceneNode__RaycastVertical(SceneNode *self, s32 *offset, s32 *target);

/**
 * @brief Turns the node towards `target` (updateRotation, set): from the
 *        node's coord.t to the target's world position, yaw from
 *        ratan2(dx, dz) and pitch from ratan2(dz, dy) plus a quarter turn, as
 *        {pitch, yaw, 0} in degrees.
 * @param self The node.
 * @param target The node to face; must have a parent.
 * @param zeroPitch Non-zero to set the pitch to 0.
 * @param noHalfTurn 0 to add a half turn (180 degrees) to the yaw.
 * @param extraRotation Ratio16[3] added afterwards (updateRotation, add), or
 *        NULL.
 */
void SceneNode__FaceTarget(SceneNode *self, SceneNode *target, s32 zeroPitch, s32 noHalfTurn,
                           void *extraRotation);

/* SceneNode's free helpers, defined in src/graphics/scene_node.c and called there by
 * symbol: the rotation and scale inputs, the matrix-over-array transforms,
 * the attribute-word accessor and the box-clipping primitives. Sony's
 * functions (RotMatrix, MulMatrix2, ApplyMatrixLV, ratan2,
 * GsInitCoordinate2, GsLinkObject4) come from <libgte.h> and <libgs.h>. */

/**
 * @brief A Ratio16 as 20.12 fixed point, from the quotient and the remainder
 *        so that num * ONE cannot overflow.
 * @param pair The Ratio16.
 * @return num / den in 20.12 fixed point.
 */
extern s32 RatioToFixed12(void *pair);

/**
 * @brief dst[i] = m * src[i] over `count` TmdVec3s (6-byte s16 vectors),
 *        through Sony's ApplyMatrixSV.
 * @param dst The array to write.
 * @param src The array to read; may equal dst.
 * @param count The number of vectors.
 * @param m The matrix.
 */
extern void ApplyMatrixToSVArray(TmdVec3 *dst, TmdVec3 *src, s32 count, MATRIX *m);

/**
 * @brief dst[i] = m * src[i] over `count` LongVec3s, through Sony's
 *        ApplyMatrixLV.
 * @param dst The LongVec3 array to write.
 * @param src The LongVec3 array to read; may equal dst.
 * @param count The number of vectors.
 * @param m The MATRIX.
 */
void ApplyMatrixToLVArray(void *dst, void *src, s32 count, void *m);

/**
 * @brief Replaces the `width` bits at bit `shift` of *word with `value`. The
 *        attribute setters of SceneNode, Sprite and BoxFill wrap it.
 * @param word The word to edit.
 * @param shift The field's lowest bit.
 * @param width The field's width in bits.
 * @param value The new field contents (not masked).
 * @return The field's old contents.
 */
extern u32 GetSetBitField(u32 *word, s32 shift, s32 width, u32 value);

/**
 * @brief Tests whether each component of `b` is within `range` of `a`'s.
 * @param a Three s32s.
 * @param range The allowed difference, inclusive.
 * @param b Three s32s.
 * @return 1 when every component is within range, else 0.
 */
extern s32 IsVec3WithinRange(s32 *a, s32 range, s32 *b);

/**
 * @brief Sets the height gate SceneNode__RaycastHullAgainstFaces's centre-line
 *        test reads: while it is non-zero, a centre-line hit counts only
 *        above a height of 512, as the edge tests always require.
 *        ObjM__InitStyleAndWorld sets it for stage 0 and stages 3, 5 and 6.
 * @param value The new gate.
 * @return The old gate.
 */
extern s32 GetSetHitHeightGate(s32 value);

/** Bit positions in GsDOBJ2.attribute (include/psyq/libgs.h), the fields the
 * SceneNode attribute setters (src/graphics/scene_node.c) replace. */
#define ATTR_LDIM_SHIFT 0      /**< GsLDIM0..GsLDIM7, 3 bits */
#define ATTR_LIGHTMODE_SHIFT 3 /**< GsFOG|GsMATE|GsLLMOD, 3 bits */
#define ATTR_LOFF_SHIFT 6      /**< GsLOFF */
#define ATTR_ZIGNR_SHIFT 7     /**< GsZIGNR */
#define ATTR_NBACKC_SHIFT 8    /**< GsNBACKC */
#define ATTR_DIV_SHIFT 9       /**< GsDIV1..GsDIV5, 3 bits */
#define ATTR_ABR_SHIFT 28      /**< GsAZERO..GsATHREE, 2 bits */
#define ATTR_ALON_SHIFT 30     /**< GsALON */
#define ATTR_DOFF_SHIFT 31     /**< GsDOFF */

/**
 * ClipSegmentToBox's result: how the segment p1..p2 meets the box. The
 * link tests' segment-against-box clipping is CalcBoxOutcode (a point's
 * outcode), ClipSegmentToBox and BisectSegmentToBox (the boundary crossing).
 */
enum ClipResult {
    CLIP_MISS = 0,      /**< the segment misses the box */
    CLIP_INSIDE = 1,    /**< both ends are inside */
    CLIP_P1_INSIDE = 2, /**< only p1 is inside */
    CLIP_P2_INSIDE = 3  /**< only p2 is inside */
};

/** @name Box outcodes
 * CalcBoxOutcode's bits: per axis, MAX when the point is past the box's
 * maximum and MIN when it is before its minimum. @{ */
#define OUTCODE_Y_MIN 0x01 /**< below the box's minimum y */
#define OUTCODE_Y_MAX 0x02 /**< past the box's maximum y */
#define OUTCODE_X_MIN 0x04 /**< below the box's minimum x */
#define OUTCODE_X_MAX 0x08 /**< past the box's maximum x */
#define OUTCODE_Z_MIN 0x10 /**< below the box's minimum z */
#define OUTCODE_Z_MAX 0x20 /**< past the box's maximum z */
/** @} */

/**
 * @brief Finds where a segment crosses the box's boundary by halving it from
 *        the inside end towards the outside end until the midpoint equals an
 *        end.
 * @param out Out: the last midpoint, the crossing.
 * @param box The box.
 * @param near The end inside the box.
 * @param far The end outside the box.
 */
void BisectSegmentToBox(TmdVec3 *out, TmdBox *box, TmdVec3 *near, TmdVec3 *far);

/**
 * @brief A point's outcode against a box: the OUTCODE_* bits of each axis it
 *        lies outside on.
 * @param box The box.
 * @param point The point.
 * @return The OUTCODE_* bits, 0 when the point is inside; callers mask it to
 *         a byte.
 */
extern s32 CalcBoxOutcode(TmdBox *box, TmdVec3 *point);

/**
 * @brief Clips the segment p1..p2 against a box. With one end inside, writes
 *        the boundary crossing to `out` (BisectSegmentToBox). With both ends
 *        outside and not on the same side of any face, halves the segment
 *        and tries each half in turn until it can no longer be halved.
 * @param out Out: the crossing when exactly one end is inside, or NULL.
 * @param box The box.
 * @param p1 One end.
 * @param p2 The other end.
 * @return An enum ClipResult.
 */
s32 ClipSegmentToBox(TmdVec3 *out, TmdBox *box, TmdVec3 *p1, TmdVec3 *p2);

#endif
