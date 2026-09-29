/*
 * SceneNode's methods (include/scene_node.h documents each), in the order
 * of gSceneNodeMethods' slots +0x008 to +0x0B4, then the table getter, the
 * methods in no slot, and the free vector, bit-field and box-clipping
 * helpers they call. The method table and the Reset inputs close the file.
 *
 * The link test runs across several of them. A sender, on a hull event,
 * fetches its model's hull, rotates it by its world matrix and notifies its
 * parents with it (NotifyWithHull, GetModelHull, TransformAndNotifyParents).
 * A SceneNode receiving that runs TryAttachNearby on the sender
 * (DispatchLinkCommand): the hull, brought into the receiver's frame
 * (ComposeAndApplyRotation), must overlap the model's bounds
 * (CheckBoundsOverlap) and hit one of its faces (RaycastHullAgainstFaces,
 * over ClipSegmentToBox and BisectSegmentToBox). On a hit the two record
 * each other as linkTarget.
 */
#include "common.h"
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include "scene_node.h"
#include "pad.h"
#include "frame_clock.h"
#include "tmd_model.h"
#include "actor.h"
#include "bmem_pmgr.h"

/* The identity inputs SceneNode__Reset hands to updateRotation and
 * updateScale: three Ratio16s each, {0/1, 0/1, 0/1} and {1/1, 1/1, 1/1}. */
extern Ratio16 sRotationZero[3];
extern Ratio16 sSceneNodeScaleOne[3];

/* Degrees in a full turn. UpdateRotation's inputs are degrees, and a degree
 * count in 20.12 fixed point divided by this is the angle in 4096ths of a
 * turn (ONE to the turn), GsCOORD2PARAM.rotate's unit; FaceTarget converts
 * ratan2's angles back to degrees with it. */
#define DEGREES_PER_TURN 360

SceneNode *New_SceneNode(void) {
    SceneNode *obj;

    obj = BMemPMgrAlloc(sizeof(SceneNode));
    if (obj == NULL) {
        return NULL;
    }
    if (GetSceneNodeMethods()->ctor(obj) != NULL) {
        return obj;
    }
    BMemPMgrFree(obj);
    return NULL;
}

void *SceneNode__SceneNode(SceneNode *self) {
    GsCOORD2PARAM *param;

    self->coord2 = BMemPMgrAlloc(sizeof(GsCOORDINATE2));
    if (self->coord2 == NULL) {
        return NULL;
    }
    param = BMemPMgrAlloc(sizeof(GsCOORD2PARAM));
    self->coord2->param = param;
    if (param == NULL) {
        BMemPMgrFree(self->coord2);
        return NULL;
    }
    GetBasicClassMethods()->ctor((BasicClass *)self);
    self->methods = GetSceneNodeMethods();
    self->model = NULL;
    self->tmd = 0;
    self->parent = NULL;
    self->coord2->super = NULL;
    self->methods->reset(self);
    return self;
}

void SceneNode__Finalize(SceneNode *self) {
    self->methods->detachFromParent(self);
    self->methods->detachAttachedChildren(self);
    self->methods->finalizeHook(self, 0);
    BMemPMgrFree(self->coord2->param);
    BMemPMgrFree(self->coord2);
    GetBasicClassMethods()->finalize((BasicClass *)self);
}

void SceneNode__AddChild(SceneNode *self, BasicClass *child) {
    GetBasicClassMethods()->addChild((BasicClass *)self, child);
    if ((child->methods->header & CLASS_ID_ROOT_MASK) == TMDMODEL_CLASS_ID) {
        SceneNode__LinkModel(self, child);
    }
}

void SceneNode__RemoveChild(SceneNode *self, BasicClass *child) {
    if ((child->methods->header & CLASS_ID_ROOT_MASK) == TMDMODEL_CLASS_ID) {
        SceneNode__UnlinkModel(self);
    }
    GetBasicClassMethods()->removeChild((BasicClass *)self, child);
}

void SceneNode__RemoveAllChildren(SceneNode *self) {
    SceneNode__UnlinkModel(self);
    GetBasicClassMethods()->removeAllChildren((BasicClass *)self);
}

/* The base onNotify first, then by the SENDER's class: a Pad's event goes to
 * onPadEvent, a FrameClock's to update, another SceneNode's to
 * dispatchLinkCommand. */
void SceneNode__OnNotify(SceneNode *self, BasicClass *sender, s32 event) {
    s32 tag;

    GetBasicClassMethods()->onNotify((BasicClass *)self, sender, event);
    tag = sender->methods->header & CLASS_ID_ROOT_MASK;
    if (tag == PAD_CLASS_ID) {
        self->methods->onPadEvent(self, sender, event);
    } else if (tag == FRAMECLOCK_CLASS_ID) {
        self->methods->update(self, sender, event);
    } else if (tag == SCENENODE_CLASS_ID) {
        self->methods->dispatchLinkCommand(self, sender, event);
    }
}

/* No rotation, unit scale, tick and attribute zero. */
void SceneNode__Reset(SceneNode *self) {
    self->tick = 0;
    self->attribute = 0;
    GsInitCoordinate2(NULL, self->coord2);
    self->methods->updateRotation(self, 1, sRotationZero);
    self->methods->updateScale(self, 1, sSceneNodeScaleOne);
    self->coord2->flg = 1;
}

/* table is Ratio16[3], degrees about x, y and z. set: store them as
 * coord2->param->rotate; else add them, wrapping at a full turn. Either way
 * coord2->flg is cleared so libgs recomputes the matrix. */
void SceneNode__UpdateRotation(SceneNode *self, s32 set, void *table) {
    Ratio16 *ratios = table;
    s32 angles[3];
    GsCOORD2PARAM *param;
    s16 *next;

    angles[0] = RatioToFixed12(&ratios[0]);
    angles[1] = RatioToFixed12(&ratios[1]);
    angles[2] = RatioToFixed12(&ratios[2]);
    /* MATCHING: all three calls, then all three divisions */
    angles[0] /= DEGREES_PER_TURN;
    angles[1] /= DEGREES_PER_TURN;
    angles[2] /= DEGREES_PER_TURN;
    param = self->coord2->param;
    /* MATCHING: `next` taken before the branch, and the loop's copy-then-advance walk */
    next = &param->rotate.vx;
    if (set) {
        param->rotate.vx = angles[0];
        param->rotate.vy = angles[1];
        param->rotate.vz = angles[2];
    } else {
        s32 i;
        s16 *cur;

        for (i = 0; i < 3; i++) {
            cur = next;
            next++;
            *cur = (*cur + angles[i]) % ONE;
        }
    }
    self->coord2->flg = 0;
}

/* table is Ratio16[3], the x, y and z scale. set: store them as
 * coord2->param->scale; else add them. Clears coord2->flg. */
void SceneNode__UpdateScale(SceneNode *self, s32 set, void *table) {
    Ratio16 *ratios = table;
    s32 sx, sy, sz;
    GsCOORD2PARAM *param;

    sx = RatioToFixed12(&ratios[0]);
    sy = RatioToFixed12(&ratios[1]);
    sz = RatioToFixed12(&ratios[2]);
    param = self->coord2->param;
    if (set) {
        /* MATCHING: the (s16) casts, on both paths */
        param->scale.vx = (s16)sx;
        param->scale.vy = (s16)sy;
        param->scale.vz = (s16)sz;
    } else {
        param->scale.vx += (s16)sx;
        param->scale.vy += (s16)sy;
        param->scale.vz += (s16)sz;
    }
    self->coord2->flg = 0;
}

/* Only when the node has no parent: record `parent`, chain coord2 under the
 * parent's, add the node to the parent's children and set its offset from
 * the parent (zero when `offset` is NULL). */
SceneNode *SceneNode__AttachToParent(SceneNode *self, SceneNode *parent, LongVec3 *offset) {
    GsCOORDINATE2 *coord2;

    if (self->parent == NULL) {
        self->parent = parent;
        /* MATCHING: coord2 through a local, read again after addChild */
        coord2 = self->coord2;
        coord2->super = parent->coord2;
        parent->methods->addChild(parent, (BasicClass *)self);
        coord2 = self->coord2;
        if (offset != NULL) {
            coord2->coord.t[0] = offset->x;
            coord2->coord.t[1] = offset->y;
            coord2->coord.t[2] = offset->z;
        } else {
            coord2->coord.t[0] = 0;
            coord2->coord.t[1] = 0;
            coord2->coord.t[2] = 0;
        }
        self->coord2->flg = 0;
    }
    return self;
}

SceneNode *SceneNode__DetachFromParent(SceneNode *self) {
    SceneNode *parent;

    parent = self->parent;
    if (parent != NULL) {
        parent->methods->removeChild(parent, (BasicClass *)self);
        self->coord2->super = NULL;
        self->parent = NULL;
    }
    return self;
}

void SceneNode__DetachAttachedChildren(SceneNode *self) {
    SceneNode *child = NULL;
    BasicClassListNode *cursor;

    do {
        self->methods->getNextAttachedChild(self, &child, &cursor);
        if (child != NULL) {
            child->methods->detachFromParent(child);
        }
    } while (cursor);
}

/* An iterator over self's children that are SceneNodes attached to self
 * (their parent is self). Start with *child NULL; each call leaves the next
 * one in *child, or NULL when the list is done (*cursor NULL). */
void SceneNode__GetNextAttachedChild(SceneNode *self, SceneNode **child, BasicClassListNode **cursor) {
    do {
        if (*child == NULL) {
            *cursor = self->children;
        }
        GetNextBasicClass((BasicClass **)child, cursor);
        if (*child != NULL && ((*child)->methods->header & CLASS_ID_ROOT_MASK) == SCENENODE_CLASS_ID &&
            (*child)->parent == self) {
            return;
        }
    } while (*cursor != NULL);
    *child = NULL;
}

/* Slot +0x05C: empty, and no subclass table overrides it. Finalize calls it
 * with (self, 0). */
void SceneNode__NoOpFinalizeHook(void) {}

/* GsDOFF is display-off, so `on` is written inverted and the old bit is
 * returned inverted: nonzero means the node was displayed. */
s32 SceneNode__SetDisplay(SceneNode *self, s32 on) {
    return GetSetBitField(&self->attribute, ATTR_DOFF_SHIFT, 1, on == 0) == 0;
}

u32 SceneNode__SetSemiTrans(SceneNode *self, s32 on) {
    return GetSetBitField(&self->attribute, ATTR_ALON_SHIFT, 1, on != 0);
}

u32 SceneNode__SetSemiTransRate(SceneNode *self, u32 rate) {
    return GetSetBitField(&self->attribute, ATTR_ABR_SHIFT, 2, rate);
}

/* GsLOFF is lighting-off: `on` is written inverted; returns the old GsLOFF. */
u32 SceneNode__SetLighting(SceneNode *self, s32 on) {
    return GetSetBitField(&self->attribute, ATTR_LOFF_SHIFT, 1, on == 0);
}

u32 SceneNode__SetLightMode(SceneNode *self, u32 mode) {
    return GetSetBitField(&self->attribute, ATTR_LIGHTMODE_SHIFT, 3, mode);
}

/* TryAttachNearby's range: the other node is tested only when its world
 * position is within this distance of this node's on each axis. */
#define ATTACH_AXIS_RANGE 16384

/* RaycastHullAgainstFaces: an edge hit counts only above this height (the
 * hit point's y above the face box's minimum, TmdModel__RaycastFaces); so
 * does a centre-line hit while sHitHeightGate is set. */
#define HIT_HEIGHT_THRESHOLD 512

/* Sets the GsLDIM field to `value` and returns the old field. */
u32 SceneNode__SetLightDim(SceneNode *self, u32 value) {
    return GetSetBitField(&self->attribute, ATTR_LDIM_SHIFT, 3, value);
}

/* Sets GsZIGNR to !on (on: the node is Z-sorted) and returns whether it was
 * on before. SetBackClip is the same shape over GsNBACKC. */
s32 SceneNode__SetUseZ(SceneNode *self, s32 on) {
    return GetSetBitField(&self->attribute, ATTR_ZIGNR_SHIFT, 1, on == 0) == 0;
}

/* Sets the GsDIV field to `value` and returns the old field. */
u32 SceneNode__SetSubdivision(SceneNode *self, u32 value) {
    return GetSetBitField(&self->attribute, ATTR_DIV_SHIFT, 3, value);
}

/* Sets GsNBACKC to !on (on: back faces are clipped) and returns whether it
 * was on before. */
s32 SceneNode__SetBackClip(SceneNode *self, s32 on) {
    return GetSetBitField(&self->attribute, ATTR_NBACKC_SHIFT, 1, on == 0) == 0;
}

/* out = RotMatrix of the node's rotation angles (GsCOORD2PARAM.rotate), or
 * of their negation when `invert` is set. The negated copy leaves `pad`
 * unset; RotMatrix does not read it. */
void SceneNode__GetRotMatrix(SceneNode *self, MATRIX *out, s32 invert) {
    SVECTOR angles;
    SVECTOR *rotate = &self->coord2->param->rotate;

    if (invert) {
        angles.vx = -rotate->vx;
        angles.vy = -rotate->vy;
        angles.vz = -rotate->vz;
    } else {
        angles = *rotate;
    }
    RotMatrix(&angles, out);
}

/* The sending side of the link test: for SCENENODE_EVENT_HULL_FIRST and
 * _LAST only, and only when
 * the node has a model with bounds, fetches the model's hull through
 * getModelHull and passes it and the event to transformAndNotifyParents.
 * Other events are dropped here; overrides handle their own before calling
 * this (Actor__NotifyMove, DreamSys__NotifyLinkAttempt). */
void SceneNode__NotifyWithHull(SceneNode *self, s32 event) {
    TmdHull hull;

    if (event > SCENENODE_EVENT_HULL_LAST) {
        return;
    }
    if (event < SCENENODE_EVENT_HULL_FIRST) {
        return;
    }
    if (self->model == NULL) {
        return;
    }
    if (!TmdModel__GetBoundsCount(self->model)) {
        return;
    }
    self->methods->getModelHull(self, &hull);
    self->methods->transformAndNotifyParents(self, &hull, event);
}

/* dest = the linked model's hull: its bounding box as eight corners
 * (TmdModel__GetHull). */
void SceneNode__GetModelHull(SceneNode *self, void *dest) {
    TmdModel__GetHull(self->model, dest);
}

/* Rotates the hull's corners in place by the node's world matrix, clears
 * linkTarget and hitMask, and notifies the parents with `event` while
 * notifyVerts points at the hull; a receiver's TryAttachNearby reads it
 * from there. */
void SceneNode__TransformAndNotifyParents(SceneNode *self, TmdHull *verts, s32 event) {
    ApplyMatrixToSVArray(verts->v, verts->v, verts->count * HULL_BOX_CORNERS, &self->coord2->workm);
    self->linkTarget = NULL;
    self->hitMask = 0;
    self->notifyVerts = verts;
    self->methods->notifyParents(self, event);
    self->notifyVerts = NULL;
}

void SceneNode__OnPadEvent(void) {}

void SceneNode__Update(void) {}

/* The receiving side of the link test, reached from OnNotify for a
 * SceneNode sender. The two hull events run tryAttachNearby on the sender
 * (which arrives as the caller's untouched second argument; see the slot).
 * SCENENODE_EVENT_LINKED is TryAttachNearby's answer: the sender that found this node records
 * itself here as linkTarget. */
void SceneNode__DispatchLinkCommand(SceneNode *self, void *sender, s32 event) {
    switch (event) {
        case SCENENODE_EVENT_HULL_FIRST:
        case SCENENODE_EVENT_HULL_LAST:
            self->methods->tryAttachNearby(self);
            break;
        case SCENENODE_EVENT_LINKED:
            self->linkTarget = sender;
            break;
    }
}

/* Tests whether `other`'s hull (its notifyVerts, set while it notifies)
 * touches this node's model. The world positions must lie within
 * ATTACH_AXIS_RANGE of each other on every axis; the hull and the offset
 * are then rotated into this node's frame (composeAndApplyRotation), and
 * the hull, moved by the offset, must overlap the model's bounds
 * (checkBoundsOverlap) and hit one of its faces (raycastHullAgainstFaces,
 * which fills other->hitMask). On a hit each node records the other: this
 * one here, `other` on the SCENENODE_EVENT_LINKED sent to it. A node with no parent has
 * no world position, and the code does not guard that case. */
void SceneNode__TryAttachNearby(SceneNode *self, SceneNode *other) {
    LongVec3 *otherPos;
    LongVec3 *selfPos;
    LongVec3 offset;
    TmdVec3 delta;
    s32 mag;
    MATRIX unused; /* MATCHING: never read; it gives the stack frame these 32 bytes */
    TmdHull hull;

    if (self->model == NULL) {
        return;
    }
    if (!TmdModel__GetBoundsCount(self->model)) {
        return;
    }

    otherPos = (other->parent != NULL) ? (LongVec3 *)other->coord2->workm.t : NULL;
    offset = *otherPos;

    selfPos = (self->parent != NULL) ? (LongVec3 *)self->coord2->workm.t : NULL;
    offset.x = offset.x - selfPos->x;
    offset.y = offset.y - selfPos->y;
    offset.z = offset.z - selfPos->z;

    /* MATCHING: each axis as gotos, and -x as ~x + 1; if/else is longer. */
    if (offset.x < 0) {
        goto x_neg;
    }
    if (offset.x <= ATTACH_AXIS_RANGE) {
        goto x_done;
    }
    return;
x_neg:
    mag = ~offset.x + 1;
    if (mag > ATTACH_AXIS_RANGE) {
        return;
    }
x_done:
    if (offset.y < 0) {
        goto y_neg;
    }
    if (offset.y <= ATTACH_AXIS_RANGE) {
        goto y_done;
    }
    return;
y_neg:
    mag = ~offset.y + 1;
    if (mag > ATTACH_AXIS_RANGE) {
        return;
    }
y_done:
    if (offset.z < 0) {
        goto z_neg;
    }
    if (offset.z <= ATTACH_AXIS_RANGE) {
        goto z_done;
    }
    return;
z_neg:
    mag = ~offset.z + 1;
    if (mag > ATTACH_AXIS_RANGE) {
        return;
    }
z_done:

    delta.x = offset.x;
    delta.y = offset.y;
    delta.z = offset.z;

    hull.count = other->notifyVerts->count;
    { /* MATCHING: the cached pointer sets the order of the two loads */
        TmdHull *otherHull = other->notifyVerts;
        self->methods->composeAndApplyRotation(self, &delta, hull.v, otherHull->v,
                                               hull.count * HULL_BOX_CORNERS);
    }

    if (!self->methods->checkBoundsOverlap(self, &hull, &delta)) {
        return;
    }
    if (!self->methods->raycastHullAgainstFaces(self, &other->hitMask, &delta, &hull)) {
        return;
    }

    self->linkTarget = other;
    other->methods->onNotify(other, self, SCENENODE_EVENT_LINKED);
}

/* Composes getRotMatrix(invert) of the node and of every ancestor, applies
 * it to `count` corners (src into dst) and, when `vec` is non-NULL, to that
 * one vector in place. */
void SceneNode__ComposeAndApplyRotation(SceneNode *self, void *vec, void *dst, void *src, s32 count) {
    MATRIX parentRot;
    MATRIX rot;
    SceneNode *parent;

    self->methods->getRotMatrix(self, &rot, 1);

    parent = self->parent;
    if (parent != NULL) {
        do {
            parent->methods->getRotMatrix(parent, &parentRot, 1);
            MulMatrix2(&parentRot, &rot);
            parent = parent->parent;
        } while (parent != NULL);
    }

    ApplyMatrixToSVArray(dst, src, count, &rot);
    if (vec != NULL) {
        ApplyMatrixToSVArray(vec, vec, 1, &rot);
    }
}

/* Moves every corner of the hull by `delta`, in place, and returns whether
 * the box around the moved corners overlaps the box around all of the
 * model's bounds records. */
/* MATCHING: every min/max a ternary stored back each iteration; a min compares with `>` */
s32 SceneNode__CheckBoundsOverlap(SceneNode *self, void *corners, TmdVec3 *delta) {
    TmdHull *hull;
    TmdVec3 *corner;
    TmdVec3 *cornerEnd;
    TmdBox hullBox;
    TmdBox *grow;
    TmdBox *bounds;
    TmdBox *boundsEnd;
    s32 boundsCount;
    s32 overlap;
    TmdBox modelBox;

    hull = corners;
    corner = hull->v;
    corner->x += delta->x;
    corner->y += delta->y;
    cornerEnd = corner + hull->count * HULL_BOX_CORNERS;
    corner->z += delta->z;
    hullBox.min = *corner;
    hullBox.max = *corner;
    grow = &hullBox;
    for (corner++; corner < cornerEnd; corner++) {
        corner->x += delta->x;
        corner->y += delta->y;
        corner->z += delta->z;
        grow->min.x = (grow->min.x > corner->x) ? corner->x : grow->min.x;
        grow->min.y = (grow->min.y > corner->y) ? corner->y : grow->min.y;
        grow->min.z = (grow->min.z > corner->z) ? corner->z : grow->min.z;
        grow->max.x = (grow->max.x < corner->x) ? corner->x : grow->max.x;
        grow->max.y = (grow->max.y < corner->y) ? corner->y : grow->max.y;
        grow->max.z = (grow->max.z < corner->z) ? corner->z : grow->max.z;
    }

    TmdModel__UpdateBoundsBuffer(self->model);
    bounds = TmdModel__GetBoundsBuffer(self->model, 0);
    boundsCount = TmdModel__GetBoundsCount(self->model);
    modelBox = *bounds;
    boundsEnd = bounds + boundsCount;
    for (bounds++; bounds < boundsEnd; bounds++) {
        modelBox.min.x = (modelBox.min.x > bounds->min.x) ? bounds->min.x : modelBox.min.x;
        modelBox.min.y = (modelBox.min.y > bounds->min.y) ? bounds->min.y : modelBox.min.y;
        modelBox.min.z = (modelBox.min.z > bounds->min.z) ? bounds->min.z : modelBox.min.z;
        modelBox.max.x = (modelBox.max.x < bounds->max.x) ? bounds->max.x : modelBox.max.x;
        modelBox.max.y = (modelBox.max.y < bounds->max.y) ? bounds->max.y : modelBox.max.y;
        modelBox.max.z = (modelBox.max.z < bounds->max.z) ? bounds->max.z : modelBox.max.z;
    }

    overlap = 0;
    if (!(hullBox.min.z > modelBox.max.z) && !(hullBox.max.z < modelBox.min.z) &&
        !(hullBox.min.x > modelBox.max.x) && !(hullBox.max.x < modelBox.min.x) &&
        !(hullBox.min.y > modelBox.max.y)) {
        overlap = !(hullBox.max.y < modelBox.min.y);
    }
    return overlap;
}

extern s32 sHitHeightGate;

/* Ray-casts segments of the hull through the model's faces, each only
 * against the bounds records its segment crosses (ClipSegmentToBox).
 * First the centre line, from the centre of the hull's first face to the
 * centre of its second: a hit sets bit i of hitMask for bounds record i
 * (above HIT_HEIGHT_THRESHOLD only, while sHitHeightGate is set), and
 * *hullHits = 1. With no centre-line hit, the edges joining corners 1 and 2
 * of each box's first face to the second face are cast: a hit above
 * HIT_HEIGHT_THRESHOLD sets bit i of hitMask and bit k of *hullHits for box
 * k. Returns whether anything was hit. TmdModel__RaycastFaces writes the
 * nearest hit point to hitPoint. */
/* MATCHING: the test of the always-0 `hit`, the gate's two bit-setting arms, the centre loop's pointers */
s32 SceneNode__RaycastHullAgainstFaces(SceneNode *self, s32 *hullHits, TmdVec3 *hitPoint, TmdHull *hull) {
    TmdVec3 center[2];
    TmdVec3 *c;
    TmdVec3 *opposite;
    s32 boundsCount;
    s32 i;
    TmdBox *bounds;
    s32 hit;
    s32 hullCount;
    TmdVec3 *corner;
    s32 k;
    s32 j;
    s32 nearest;
    s32 height;
    u8 pad[24]; /* MATCHING: never read; it gives the stack frame these 24 bytes */

    nearest = DIST_NONE;

    corner = hull->v;
    opposite = hull->v + 2; /* the diagonally opposite corner of the same face */
    for (c = center; c < &center[2]; c++) {
        c->x = (corner->x + opposite->x) >> 1;
        c->y = (corner->y + opposite->y) >> 1;
        c->z = (corner->z + opposite->z) >> 1;
        corner += HULL_FACE_CORNERS;
        opposite += HULL_FACE_CORNERS;
    }

    self->hitMask = 0;
    boundsCount = TmdModel__GetBoundsCount(self->model);
    hit = 0;
    for (i = 0; i < boundsCount; i++) {
        bounds = TmdModel__GetBoundsBuffer(self->model, i);
        if (ClipSegmentToBox(NULL, bounds, &center[0], &center[1])) {
            if (TmdModel__RaycastFaces(self->model, &nearest, hitPoint, &height, &center[0], &center[1])) {
                if (sHitHeightGate == 0) {
                    self->hitMask |= 1 << i;
                } else if (height > HIT_HEIGHT_THRESHOLD) {
                    self->hitMask |= 1 << i;
                }
            }
        }
    }

    if (self->hitMask != 0) {
        *hullHits = 1;
        if (hit != 0) {
            return 2;
        }
        return 1;
    }

    *hullHits = 0;
    hullCount = hull->count;
    for (i = 0; i < boundsCount; i++) {
        bounds = TmdModel__GetBoundsBuffer(self->model, i);
        corner = hull->v;
        for (k = 0; k < hullCount; k++) {
            for (j = 0; j < HULL_FACE_CORNERS; j++) {
                if (j == 1 || j == 2) {
                    if (ClipSegmentToBox(NULL, bounds, corner, corner + HULL_FACE_CORNERS)) {
                        if (TmdModel__RaycastFaces(self->model, &nearest, hitPoint, &height, corner,
                                                   corner + HULL_FACE_CORNERS)) {
                            if (height > HIT_HEIGHT_THRESHOLD) {
                                self->hitMask |= 1 << i;
                                *hullHits |= 1 << k;
                            }
                        }
                    }
                }
                corner++;
            }
            corner += HULL_FACE_CORNERS;
        }
    }

    return (*hullHits != 0);
}

/* Clips p1..p2 against `box` (enum ClipResult). With one end inside,
 * BisectSegmentToBox writes the crossing to `out` when it is non-NULL. With
 * both ends outside and not on the same side of any face, the segment is
 * halved and each half tried in turn, until it can no longer be halved. */
/* MATCHING: the final if (mid.y) with two identical arms; a plain return is longer */
s32 ClipSegmentToBox(TmdVec3 *out, TmdBox *box, TmdVec3 *p1, TmdVec3 *p2) {
    u8 code1;
    u8 code2;
    TmdVec3 mid;

    code1 = CalcBoxOutcode(box, p1);
    code2 = CalcBoxOutcode(box, p2);

    if (code1 == 0) {
        if (code2 != 0) {
            goto shared_test;
        }
        return CLIP_INSIDE;
    }
    if (code2 != 0) {
        goto shared_test;
    }
    if (out != NULL) {
        BisectSegmentToBox(out, box, p2, p1);
    }
    return CLIP_P2_INSIDE;

shared_test:
    if (code1 != 0) {
        goto combined;
    }
    if (out != NULL) {
        BisectSegmentToBox(out, box, p1, p2);
    }
    return CLIP_P1_INSIDE;

combined:
    if ((code1 & code2) != 0) {
        return CLIP_MISS;
    }

    mid.x = (p1->x + p2->x) >> 1;
    mid.y = (p1->y + p2->y) >> 1;
    mid.z = (p1->z + p2->z) >> 1;

    if (p1->x == mid.x && p1->y == mid.y && p1->z == mid.z) {
        return CLIP_MISS;
    }
    if (p2->x == mid.x && p2->y == mid.y && p2->z == mid.z) {
        return CLIP_MISS;
    }

    {
        s32 result = ClipSegmentToBox(out, box, p1, &mid);
        if (result != 0) {
            return result;
        }
    }
    {
        s32 result = ClipSegmentToBox(out, box, &mid, p2);
        if (result != 0) {
            return result;
        }
    }
    if (mid.y) {
        return CLIP_MISS;
    } else {
        return CLIP_MISS;
    }
}

/* Halves the segment from `near` (inside the box) to `far` (outside) until
 * the midpoint equals an end, and leaves the last midpoint in `out`. A
 * midpoint outside the box replaces `far`, one inside replaces `near`; each
 * goes into its own buffer, so neither end overwrites the other. The
 * outcode is CalcBoxOutcode's, computed inline. */
void BisectSegmentToBox(TmdVec3 *out, TmdBox *box, TmdVec3 *near, TmdVec3 *far) {
    TmdVec3 insideBuf;
    TmdVec3 outsideBuf;
    TmdVec3 *dst;
    u8 outcode;

    for (;;) {
        out->x = (near->x + far->x) >> 1;
        out->y = (near->y + far->y) >> 1;
        out->z = (near->z + far->z) >> 1;

        if (out->x == near->x && out->y == near->y && out->z == near->z) {
            return;
        }
        if (out->x == far->x && out->y == far->y && out->z == far->z) {
            return;
        }

        outcode = 0;
        if (box->max.x < out->x) {
            outcode = OUTCODE_X_MAX;
        } else if (out->x < box->min.x) {
            outcode = OUTCODE_X_MIN;
        }
        if (box->max.y < out->y) {
            outcode |= OUTCODE_Y_MAX;
        } else if (out->y < box->min.y) {
            outcode |= OUTCODE_Y_MIN;
        }
        if (box->max.z < out->z) {
            outcode |= OUTCODE_Z_MAX;
        } else if (out->z < box->min.z) {
            outcode |= OUTCODE_Z_MIN;
        }

        if (outcode != 0) {
            dst = &outsideBuf;
            far = dst;
        } else {
            dst = &insideBuf;
            near = dst;
        }
        *dst = *out;
    }
}

/* Slot +0x0B0: empty, and nothing calls it. */
void SceneNode__NoOpSlotB0(void) {}

/* Adds this node as a child (addChild) to every parent of `node` that is
 * an Actor. */
/* MATCHING: two nested do/while loops, not a goto loop, so SCENENODE_CLASS_ID is set up once,
 * before both */
void SceneNode__AddToActorParents(SceneNode *self, void *node) {
    SceneNode *parent;
    void *cursor;

    parent = NULL;
    do {
        do {
            BasicClass__GetNextParentRef(node, (BasicClass **)&parent, (BasicClassListNode **)&cursor);
            if (parent != NULL && (parent->methods->header & CLASS_ID_ROOT_MASK) == SCENENODE_CLASS_ID) {
                goto found;
            }
        } while (cursor != NULL);
        parent = NULL;
    found:
        if (parent != NULL && (u8)parent->methods->header == ACTOR_CLASS_ID) {
            parent->methods->addChild(parent, (BasicClass *)self);
        }
    } while (cursor != NULL);
}

/* The table getter: subclasses reach the base methods through it. */
SceneNodeMethods *GetSceneNodeMethods(void) {
    return &gSceneNodeMethods;
}

/* How far RaycastVertical's ray reaches from its origin, first along -y and
 * then along +y, in the model's own units. */
#define RAYCAST_PROBE_LENGTH 1024

/* dst = `src`, three s16s in the node's own frame, rotated by the node's
 * rotation and widened to s32: the same offset in the parent's frame.
 * getRotMatrix's 0 asks for the un-negated angles (local to parent). */
void SceneNode__RotateLocalVector(SceneNode *self, LongVec3 *dst, s16 *src) {
    MATRIX rot;

    self->methods->getRotMatrix(self, &rot, 0);
    dst->x = src[0];
    dst->y = src[1];
    dst->z = src[2];
    ApplyMatrixToLVArray(dst, dst, 1, &rot);
}

/* dst = `src` rotated by the node's rotation (as RotateLocalVector) plus the
 * node's world position, coord2->workm.t. A node with no parent reads the
 * position through NULL. */
/* MATCHING: the parent test is repeated per axis; testing it once changes the code */
void SceneNode__LocalOffsetToWorldPos(SceneNode *self, s32 *dst, s32 *src, s32 unused) {
    MATRIX rot;
    long *worldPos;

    self->methods->getRotMatrix(self, &rot, 0);
    ApplyMatrixToLVArray(dst, src, 1, &rot);

    worldPos = self->parent != 0 ? self->coord2->workm.t : 0;
    dst[0] = dst[0] + worldPos[0];

    worldPos = self->parent != 0 ? self->coord2->workm.t : 0;
    dst[1] = dst[1] + worldPos[1];

    worldPos = self->parent != 0 ? self->coord2->workm.t : 0;
    dst[2] = dst[2] + worldPos[2];
}

/* out[i] = GsCOORD2PARAM.rotate's angle i (ONE to the turn) in degrees, over
 * 1: `* 45 >> 9` is `* 360 / ONE` reduced by 8, rounding down. The Ratio16[3]
 * shape FaceTarget builds and updateRotation takes. */
void SceneNode__GetRotationDegrees(SceneNode *self, Ratio16 *out) {
    GsCOORD2PARAM *src;

    src = self->coord2->param;
    out[0].num = src->rotate.vx * 45 >> 9;
    out[0].den = 1;
    out[1].num = src->rotate.vy * 45 >> 9;
    out[1].den = 1;
    out[2].num = src->rotate.vz * 45 >> 9;
    out[2].den = 1;
}

/* Keeps `model`, a TmdModel, as the node's model, puts its TMD object in
 * GsDOBJ2.tmd, and has GsLinkObject4 link object 0 of the TMD to the node's
 * GsDOBJ2 (which starts at `attribute`). */
/* MATCHING: the call re-reads self->model instead of using `model` */
void SceneNode__LinkModel(SceneNode *self, void *model) {
    self->model = model;
    self->tmd = (s32)((TmdModel *)model)->object;
    /* Casts: Sony types tmd_base as an address (`unsigned long`), and
     * scene_node.h spells the embedded GsDOBJ2 as four separate fields. */
    GsLinkObject4((u_long)((TmdModel *)self->model)->data->objects, (GsDOBJ2 *)&self->attribute, 0);
}

/* Clears what LinkModel set, GsDOBJ2.tmd and the model; no GS call. */
void SceneNode__UnlinkModel(SceneNode *self) {
    self->tmd = 0;
    self->model = 0;
}

extern void SubVec3S16(s32 *dest, s16 *from, s16 *to);

/* Casts a vertical ray through `target`, a world position, against the
 * node's model. On a hit, `offset` gets the hit point less `target`, both in
 * the node's own frame, and it returns 1; with no model or no hit, 0.
 *
 * A node with GsDOFF set and a parent first recomputes its world position,
 * workm.t, as its own coord.t plus every ancestor's coord.t (translation
 * only). `target` less that position is rotated into the node's frame by
 * composeAndApplyRotation, and the ray runs from there RAYCAST_PROBE_LENGTH
 * along -y, then, on a miss, along +y. */
/* MATCHING: one coord.t copy, the ternary twice per axis, one hit branch per probe, the workm.t test */
s32 SceneNode__RaycastVertical(SceneNode *self, s32 *offset, s32 *target) {
    long *worldPos;
    SVECTOR origin;
    SVECTOR end;
    SVECTOR hit;
    s32 best;
    GsCOORDINATE2 *node;
    SceneNode *cur;

    if (self->model != NULL) {
        if ((self->attribute & GsDOFF) && self->parent != NULL) {
            node = self->coord2;
            if (node->workm.t != NULL) {
                *(LongVec3 *)node->workm.t = *(LongVec3 *)node->coord.t;

                cur = self->parent;
                if (cur != NULL) {
                    do {
                        ((LongVec3 *)(self->parent != 0 ? self->coord2->workm.t : (long *)0))->x =
                            ((LongVec3 *)(self->parent != 0 ? self->coord2->workm.t : (long *)0))->x +
                            cur->coord2->coord.t[0];
                        ((LongVec3 *)(self->parent != 0 ? self->coord2->workm.t : (long *)0))->y =
                            ((LongVec3 *)(self->parent != 0 ? self->coord2->workm.t : (long *)0))->y +
                            cur->coord2->coord.t[1];
                        ((LongVec3 *)(self->parent != 0 ? self->coord2->workm.t : (long *)0))->z =
                            ((LongVec3 *)(self->parent != 0 ? self->coord2->workm.t : (long *)0))->z +
                            cur->coord2->coord.t[2];

                        cur = cur->parent;
                    } while (cur != NULL);
                }
            }
        }

        worldPos = self->parent != 0 ? self->coord2->workm.t : 0;
        end.vx = (u16)target[0] - (u16)worldPos[0];
        end.vy = (u16)target[1] - (u16)worldPos[1];
        end.vz = (u16)target[2] - (u16)worldPos[2];

        self->methods->composeAndApplyRotation(self, 0, &origin, &end, 1);

        end.vx = origin.vx;
        end.vy = (u16)origin.vy - RAYCAST_PROBE_LENGTH;
        end.vz = origin.vz;
        if (TmdModel__RaycastFaces(self->model, &best, (TmdVec3 *)&hit, NULL, (TmdVec3 *)&origin,
                                   (TmdVec3 *)&end)) {
            SubVec3S16(offset, &origin.vx, &hit.vx);
            return 1;
        }
        end.vy = (u16)origin.vy + RAYCAST_PROBE_LENGTH;
        if (TmdModel__RaycastFaces(self->model, &best, (TmdVec3 *)&hit, NULL, (TmdVec3 *)&origin,
                                   (TmdVec3 *)&end)) {
            SubVec3S16(offset, &origin.vx, &hit.vx);
            return 1;
        }
    }
    return 0;
}

/* dest = to - from, per component, widening s16 to s32. */
void SubVec3S16(s32 *dest, s16 *from, s16 *to) {
    dest[0] = to[0] - from[0];
    dest[1] = to[1] - from[1];
    dest[2] = to[2] - from[2];
}

/* Turns the node towards `target`: from the node's coord.t to the target's
 * workm.t, yaw is ratan2(dx, dz) and pitch ratan2(dz, dy) plus a quarter
 * turn, a zero first argument passed as 1; both go to degrees and are set
 * with updateRotation as {pitch, yaw, 0}. `zeroPitch` clears the pitch,
 * `noHalfTurn == 0` adds a half turn to the yaw, and a non-NULL
 * `extraRotation` (Ratio16[3]) is then added. A target with no parent reads
 * its position through NULL. */
void SceneNode__FaceTarget(SceneNode *self, SceneNode *target, s32 zeroPitch, s32 noHalfTurn,
                           void *extraRotation) {
    long *pos;
    long *targetPos;
    s32 dx;
    s32 dy;
    s32 dz;
    Ratio16 out[3];

    pos = self->coord2->coord.t;
    targetPos = target->parent != 0 ? target->coord2->workm.t : 0;

    if (targetPos[0] != pos[0]) {
        dx = targetPos[0] - pos[0];
        dz = targetPos[2] - pos[2];
        out[1].num = ratan2(dx, dz);
    } else {
        dz = targetPos[2] - pos[2];
        out[1].num = ratan2(1, dz);
    }

    if (targetPos[2] != pos[2]) {
        dz = targetPos[2] - pos[2];
        dy = targetPos[1] - pos[1];
        out[0].num = ratan2(dz, dy);
    } else {
        dy = targetPos[1] - pos[1];
        out[0].num = ratan2(1, dy);
    }

    out[0].num = (out[0].num + ONE / 4) * DEGREES_PER_TURN / ONE;
    out[1].num = out[1].num * DEGREES_PER_TURN / ONE;

    out[2].num = 0;
    out[2].den = 1;
    out[1].den = 1;
    out[0].den = 1;
    if (zeroPitch != 0) {
        out[0].num = 0;
    }
    if (noHalfTurn == 0) {
        out[1].num = out[1].num + DEGREES_PER_TURN / 2;
    }

    self->methods->updateRotation(self, 1, out);
    if (extraRotation != 0) {
        self->methods->updateRotation(self, 0, extraRotation);
    }
}

/* `pair` (a Ratio16) as 20.12 fixed point, from the quotient and the
 * remainder so that num * ONE cannot overflow; the / and % share one division. */
s32 RatioToFixed12(void *pair) {
    Ratio16 *p;
    s32 whole, rem, frac;

    p = (Ratio16 *)pair;
    whole = p->num / p->den;
    rem = p->num % p->den;
    frac = rem * ONE / p->den;
    return whole * ONE + frac;
}

/* The OUTCODE_* bits of `point` against `box`, 0 when it is inside. The
 * callers mask the result to a byte. */
s32 CalcBoxOutcode(TmdBox *box, TmdVec3 *point) {
    s32 flags;

    flags = 0;
    if (box->max.x < point->x) {
        flags = OUTCODE_X_MAX;
    } else if (point->x < box->min.x) {
        flags = OUTCODE_X_MIN;
    }
    if (box->max.y < point->y) {
        flags |= OUTCODE_Y_MAX;
    } else if (point->y < box->min.y) {
        flags |= OUTCODE_Y_MIN;
    }
    if (box->max.z < point->z) {
        flags |= OUTCODE_Z_MAX;
    } else if (point->z < box->min.z) {
        flags |= OUTCODE_Z_MIN;
    }
    return flags;
}

/* Replaces the `width` bits at bit `shift` of *word with `value` and returns
 * the field's old contents. The attribute setters of SceneNode, Sprite and
 * BoxFill wrap it. */
/* MATCHING: the mask is built by a loop, not as (1 << width) - 1 */
u32 GetSetBitField(u32 *word, s32 shift, s32 width, u32 value) {
    u32 mask;
    s32 i;
    u32 old;

    mask = 1;
    for (i = 0; i < width; i++) {
        mask <<= 1;
    }
    mask -= 1;
    mask <<= shift;
    old = (*word & mask) >> shift;
    *word = *word & ~mask;
    *word = *word | (value << shift);
    return old;
}

/* Sony's, from libgte; this SDK's <libgte.h> does not declare it. */
extern SVECTOR *ApplyMatrixSV(MATRIX *m, SVECTOR *v0, SVECTOR *v1);

/* dst[i] = m * src[i] over `count` TmdVec3s, through ApplyMatrixSV (which
 * reads and writes an SVECTOR's first three). dst == src works in place. */
/* MATCHING: each element is copied whole through an all-s16 struct local */
void ApplyMatrixToSVArray(TmdVec3 *dst, TmdVec3 *src, s32 count, MATRIX *m) {
    TmdVec3 *end;

    end = dst + count;
    while (dst < end) {
        TmdVec3 buf;

        buf = *src;
        ApplyMatrixSV(m, (SVECTOR *)&buf, (SVECTOR *)dst);
        src++;
        dst++;
    }
}

/* dst[i] = m * src[i] over `count` LongVec3s, through ApplyMatrixLV.
 * dst == src works in place. */
/* MATCHING: the dead six-argument call gives the function retail's stack room for
 * six outgoing arguments */
void ApplyMatrixToLVArray(void *dst, void *src, s32 count, void *m) {
    LongVec3 *end;

    end = (LongVec3 *)dst + count;
    while ((LongVec3 *)dst < end) {
        ApplyMatrixLV(m, src, dst);
        dst = (LongVec3 *)dst + 1;
        src = (LongVec3 *)src + 1;
    }
    if (0) {
        /* Cast to an unprototyped function type: libgte.h's prototype
         * rejects six arguments, and only the count matters here. */
        ((void (*)())ApplyMatrixLV)(m, src, dst, 0, 0, 0);
    }
}

/* 1 when each component of `b` is within `range` of `a`'s, else 0. */
/* MATCHING: the pointer bumps sit in the for's increment clause */
s32 IsVec3WithinRange(s32 *a, s32 range, s32 *b) {
    s32 i;

    for (i = 0; i < 3; i++, a++, b++) {
        if (*b < *a - range) {
            return 0;
        }
        if (*a + range < *b) {
            return 0;
        }
    }
    return 1;
}

/* Sets sHitHeightGate and returns its old value. While it is non-zero,
 * SceneNode__RaycastHullAgainstFaces's segment test accepts only a hit whose
 * height (TmdModel__RaycastFaces: above the face box's minimum y) is at least
 * 513, which its edge tests always require. ObjM__InitStyleAndWorld sets it
 * for stage 0 and stages 3, 5 and 6. */
s32 GetSetHitHeightGate(s32 value) {
    s32 old;

    old = sHitHeightGate;
    sHitHeightGate = value;
    return old;
}

/* SceneNode's method table (include/scene_node.h names each slot): the
 * inherited BasicClass slots, SceneNode's overrides of them, then its own.
 * A (void *) entry is a method whose declared parameters differ from the
 * slot's, usually a base method taking BasicClass *. */
SceneNodeMethods gSceneNodeMethods = {
    /* +0x000 header */ SCENENODE_CLASS_ID,
    /* +0x004 release */ (void *)BasicClass__Release,
    /* +0x008 ctor */ SceneNode__SceneNode,
    /* +0x00C finalize */ SceneNode__Finalize,
    /* +0x010 addChild */ SceneNode__AddChild,
    /* +0x014 removeChild */ SceneNode__RemoveChild,
    /* +0x018 removeAllChildren */ SceneNode__RemoveAllChildren,
    /* +0x01C getNextChild */ (void *)BasicClass__GetNextChild,
    /* +0x020 addParentRef */ (void *)BasicClass__AddParentRef,
    /* +0x024 removeParentRef */ (void *)BasicClass__RemoveParentRef,
    /* +0x028 clearParentRefs */ (void *)BasicClass__ClearParentRefs,
    /* +0x02C getNextParentRef */ (void *)BasicClass__GetNextParentRef,
    /* +0x030 notifyParents */ (void *)BasicClass__NotifyParents,
    /* +0x034 slot34 */ BasicClass__NoOpSlot34,
    /* +0x038 onNotify */ (void *)SceneNode__OnNotify,
    /* +0x03C slot3C */ NULL,
    /* +0x040 reset */ SceneNode__Reset,
    /* +0x044 updateRotation */ SceneNode__UpdateRotation,
    /* +0x048 updateScale */ SceneNode__UpdateScale,
    /* +0x04C attachToParent */ SceneNode__AttachToParent,
    /* +0x050 detachFromParent */ SceneNode__DetachFromParent,
    /* +0x054 detachAttachedChildren */ SceneNode__DetachAttachedChildren,
    /* +0x058 getNextAttachedChild */ SceneNode__GetNextAttachedChild,
    /* +0x05C finalizeHook */ (void *)SceneNode__NoOpFinalizeHook,
    /* +0x060 setDisplay */ SceneNode__SetDisplay,
    /* +0x064 setSemiTransOn */ SceneNode__SetSemiTrans,
    /* +0x068 setSemiTransRate */ SceneNode__SetSemiTransRate,
    /* +0x06C setLighting */ SceneNode__SetLighting,
    /* +0x070 setLightMode */ SceneNode__SetLightMode,
    /* +0x074 setLightDim */ SceneNode__SetLightDim,
    /* +0x078 setUseZ */ SceneNode__SetUseZ,
    /* +0x07C setSubdivision */ SceneNode__SetSubdivision,
    /* +0x080 setBackClip */ SceneNode__SetBackClip,
    /* +0x084 getRotMatrix */ (void *)SceneNode__GetRotMatrix,
    /* +0x088 notifyWithHull */ SceneNode__NotifyWithHull,
    /* +0x08C getModelHull */ SceneNode__GetModelHull,
    /* +0x090 transformAndNotifyParents */ SceneNode__TransformAndNotifyParents,
    /* +0x094 onPadEvent */ (void *)SceneNode__OnPadEvent,
    /* +0x098 update */ (void *)SceneNode__Update,
    /* +0x09C dispatchLinkCommand */ SceneNode__DispatchLinkCommand,
    /* +0x0A0 tryAttachNearby */ (void *)SceneNode__TryAttachNearby,
    /* +0x0A4 composeAndApplyRotation */ SceneNode__ComposeAndApplyRotation,
    /* +0x0A8 checkBoundsOverlap */ SceneNode__CheckBoundsOverlap,
    /* +0x0AC raycastHullAgainstFaces */ (void *)SceneNode__RaycastHullAgainstFaces,
    /* +0x0B0 slotB0 */ SceneNode__NoOpSlotB0,
    /* +0x0B4 addToActorParents */ SceneNode__AddToActorParents,
};

Ratio16 sRotationZero[3] = {{0, 1}, {0, 1}, {0, 1}};
Ratio16 sSceneNodeScaleOne[3] = {{1, 1}, {1, 1}, {1, 1}};
