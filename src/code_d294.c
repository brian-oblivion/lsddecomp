/*
 * code_d294 -- SceneNode (include/SceneNode.h), part 1 of 3: the occupants
 * of slots +0x000 to +0x070 of gSceneNodeMethods. Part 2 is code_d294_b.c,
 * part 3 code_d294_c.c; their shared helpers are declared in code_d294.h.
 *
 * Lifecycle: New_SceneNode, the ctor (which allocates the node's
 * GsCOORDINATE2 and its GsCOORD2PARAM, and fails when either allocation
 * does) and Finalize (detach from parent and children, free both).
 *
 * Children: the BasicClass child-list overrides, which also link a TmdModel
 * child into the node's GsDOBJ2 when it is added and unlink it when it is
 * removed; OnNotify, which dispatches on the sender's class id; and the
 * walk over the children attached to this node (GetNextAttachedChild).
 *
 * Transform: Reset (identity), UpdateRotation and UpdateScale (set or add
 * three Ratio16s into the GsCOORD2PARAM), and attach to and detach from a
 * parent's coordinate.
 *
 * Attribute: the first five setters over GsDOBJ2.attribute, each replacing
 * one libgs field through GetSetBitField and returning its old value.
 */
#include "common.h"
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include "code_d294.h"
#include "Pad.h"
#include "FrameClock.h"
#include "TmdModel.h"
#include "Actor.h"

/* UpdateRotation's divisor: its inputs are degrees, and a degree count in
 * 20.12 fixed point divided by 360 is the angle in 4096ths of a turn (ONE to
 * the turn), GsCOORD2PARAM.rotate's unit. */
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
    Get_vtable_BasicClass()->ctor((BasicClass *)self);
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
    self->methods->slot5C(self, 0);
    BMemPMgrFree(self->coord2->param);
    BMemPMgrFree(self->coord2);
    Get_vtable_BasicClass()->finalize((BasicClass *)self);
}

void SceneNode__AddChild(SceneNode *self, BasicClass *child) {
    Get_vtable_BasicClass()->addChild((BasicClass *)self, child);
    if ((child->methods->header & CLASS_ID_ROOT_MASK) == TMDMODEL_CLASS_ID) {
        SceneNode__LinkModel(self, child);
    }
}

void SceneNode__RemoveChild(SceneNode *self, BasicClass *child) {
    if ((child->methods->header & CLASS_ID_ROOT_MASK) == TMDMODEL_CLASS_ID) {
        SceneNode__UnlinkModel(self);
    }
    Get_vtable_BasicClass()->removeChild((BasicClass *)self, child);
}

void SceneNode__RemoveAllChildren(SceneNode *self) {
    SceneNode__UnlinkModel(self);
    Get_vtable_BasicClass()->removeAllChildren((BasicClass *)self);
}

/* The base onNotify first, then by the SENDER's class: a Pad's event goes to
 * onPadEvent, a FrameClock's to update, another SceneNode's to
 * dispatchLinkCommand. */
void SceneNode__OnNotify(SceneNode *self, BasicClass *sender, s32 event) {
    s32 tag;

    Get_vtable_BasicClass()->onNotify((BasicClass *)self, sender, event);
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
    self->methods->updateRotation(self, 1, ROTATION_ZERO);
    self->methods->updateScale(self, 1, SCALE_ONE);
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
        /* MATCHING: coord2 through a local, reloaded after addChild */
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
void SceneNode__NoOpSlot5C(void) {}

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

/* ---- merged from code_d294_b ---- */

/*
 * code_d294_b -- SceneNode (include/SceneNode.h), part 2 of 3: the
 * occupants of slots +0x074 to +0x0B4, and the segment-against-box clippers
 * the link test uses. Part 1 is code_d294.c, part 3 code_d294_c.c.
 *
 * SetLightDim, SetUseZ, SetSubdivision and SetBackClip set the last four
 * fields of GsDOBJ2.attribute; GetRotMatrix makes the node's rotation, or
 * its negation, a MATRIX.
 *
 * The link test, both sides. A sender, on event 2 or 3, fetches its model's
 * hull (NotifyWithHull, GetModelHull), rotates it by its world matrix and
 * notifies its parents with it in notifyVerts (TransformAndNotifyParents).
 * A SceneNode receiving that runs TryAttachNearby on the sender
 * (DispatchLinkCommand): the hull, brought into the receiver's frame
 * (ComposeAndApplyRotation), must overlap the model's bounds
 * (CheckBoundsOverlap) and hit one of its faces (RaycastHullAgainstFaces).
 * On a hit the two record each other as linkTarget, the sender through the
 * event 4 the receiver sends back.
 *
 * Also: the empty onPadEvent and update defaults and slot +0x0B0,
 * AddToActorParents, the table getter, and ClipSegmentToBox and
 * BisectSegmentToBox (declared in include/code_d294.h).
 */


/* TryAttachNearby's range: the other node is tested only when its world
 * position is within this distance of this node's on each axis. */
#define ATTACH_AXIS_RANGE 16384

/* RaycastHullAgainstFaces: an edge hit counts only above this height (the
 * hit point's y above the face box's minimum, TmdModel__RaycastFaces); so
 * does a centre-line hit while gHitHeightGate is set. */
#define HIT_HEIGHT_THRESHOLD 512

/* Sets the GsLDIM field to `value` and returns the old field. The four
 * setters here are wrappers around GetSetBitField, like code_d294.c's. */
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

/* The sending side of the link test: for events 2 and 3 only, and only when
 * the node has a model with bounds, fetches the model's hull through
 * getModelHull and passes it and the event to transformAndNotifyParents.
 * Other events are dropped here; overrides handle their own before calling
 * this (Actor__NotifyMove, DreamSys__NotifyLinkAttempt). */
void SceneNode__NotifyWithHull(SceneNode *self, s32 event) {
    TmdHull hull;

    if (event >= 4) {
        return;
    }
    if (event < 2) {
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
 * SceneNode sender. Events 2 and 3 run tryAttachNearby on the sender (which
 * arrives as the caller's untouched second argument; see the slot). Event
 * 4 is TryAttachNearby's answer: the sender that found this node records
 * itself here as linkTarget. */
void SceneNode__DispatchLinkCommand(SceneNode *self, void *sender, s32 event) {
    switch (event) {
        case 2:
        case 3:
            self->methods->tryAttachNearby(self);
            break;
        case 4:
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
 * one here, `other` on the event 4 sent to it. A node with no parent has
 * no world position, and the code does not guard that case. */
void SceneNode__TryAttachNearby(SceneNode *self, SceneNode *other) {
    LongVec3 *otherPos;
    LongVec3 *selfPos;
    LongVec3 offset;
    TmdVec3 delta;
    s32 mag;
    MATRIX unused; /* MATCHING: never read; retail's frame has these 32 bytes */
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
    { /* MATCHING: the cached pointer orders the two loads as retail does */
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
    other->methods->onNotify(other, self, 4);
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
 * model's bounds records. MATCHING: every min/max is a ternary stored back
 * each iteration, and a min compares with `>`: retail stores every field
 * every time and loads the slt operands in this order. */
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

extern s32 gHitHeightGate;

/* Ray-casts segments of the hull through the model's faces, each only
 * against the bounds records its segment crosses (ClipSegmentToBox).
 * First the centre line, from the centre of the hull's first face to the
 * centre of its second: a hit sets bit i of hitMask for bounds record i
 * (above HIT_HEIGHT_THRESHOLD only, while gHitHeightGate is set), and
 * *hullHits = 1. With no centre-line hit, the edges joining corners 1 and 2
 * of each box's first face to the second face are cast: a hit above
 * HIT_HEIGHT_THRESHOLD sets bit i of hitMask and bit k of *hullHits for box
 * k. Returns whether anything was hit. TmdModel__RaycastFaces writes the
 * nearest hit point to hitPoint.
 *
 * MATCHING: `hit` is only ever 0, but retail still tests it; the gate is
 * two arms that each set the bit; the centre loop walks pointers. */
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
    u8 pad[24]; /* MATCHING: never read; retail's frame has 24 bytes here */

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
                if (gHitHeightGate == 0) {
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
 * halved and each half tried in turn, until it can no longer be halved.
 * MATCHING: the final `if (mid.y)` returns the same value on both arms; it
 * is what makes the second recursive result be tested in $v0. */
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
 * an Actor. MATCHING: the two nested do/while loops, not gotos: loop.c
 * then hoists SCENENODE_CLASS_ID into a saved register, as retail does. */
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
