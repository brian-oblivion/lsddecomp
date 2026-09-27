/*
 * code_d294_b -- SceneNode (include/SceneNode.h), part 2 of 3: slots +0x074
 * to +0x0B4. The last four attribute setters; GetRotMatrix; the hull
 * notification chain (NotifyWithHull fills the model's TmdHull through
 * GetModelHull and hands it to TransformAndNotifyParents); the empty
 * onPadEvent/update defaults; DispatchLinkCommand and the proximity test it
 * runs (TryAttachNearby, with ComposeAndApplyRotation, CheckBoundsOverlap
 * and ClassifyAgainstPlanes); NotifyTaggedParents; the table getter; and
 * the free segment-against-box clippers the bounds tests use
 * (ClipSegmentToBox, BisectSegmentToBox).
 */

#include "common.h"
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include "code_d294.h"
#include "TmdModel.h"
#include "Actor.h"

/* Sibling of SceneNode__SetDisplay/D374/D3A0/D3CC/D3F8 (code_d294.c): a thin
 * wrapper around GetSetBitField over &self->unk10, shift 0 width 3. Raw
 * pass-through value and raw pass-through result -- same shape as
 * SceneNode__SetSemiTrans/D3A0/D3F8 (no `== 0` on either side). */
u32 SceneNode__SetLightDim(SceneNode *self, u32 value) {
    return GetSetBitField(&self->attribute, 0, 3, value);
}

/* Sibling of SceneNode__SetDisplay (the ONLY one of the five already-matched
 * self->unk10 bitfield accessors that both converts its input to a boolean
 * (`a1 == 0`) AND inverts its own result (`== 0`)). This function does
 * exactly that double-inversion, at shift 7 width 1, hence the same `s32`
 * return type as SceneNode__SetDisplay rather than the plain `u32` of the other
 * three siblings. */
s32 SceneNode__SetUseZ(SceneNode *self, s32 on) {
    return GetSetBitField(&self->attribute, 7, 1, on == 0) == 0;
}

/* Same family as SceneNode__SetLightDim, shift 9 width 3. Raw pass-through. */
u32 SceneNode__SetSubdivision(SceneNode *self, u32 value) {
    return GetSetBitField(&self->attribute, 9, 3, value);
}

/* Same family as SceneNode__SetUseZ: double-inversion shape, shift 8 width 1. */
s32 SceneNode__SetBackClip(SceneNode *self, s32 on) {
    return GetSetBitField(&self->attribute, 8, 1, on == 0) == 0;
}

/* coord2->param is a 0x28-byte GsCOORD2PARAM whose +0x10 holds the SVECTOR
 * rotation. a2 selects between negating vx/vy/vz into a local copy (pad left
 * uninitialised, exactly as retail's own negate path never stores to it) or
 * copying the vector verbatim, then forwards the result -- plus a1, passed
 * straight through -- to the PsyQ helper RotMatrix. MATCHING: SVECTOR's
 * all-short members give it alignment 2, which is what makes the
 * whole-struct copy compile to lwl/lwr. */
void SceneNode__GetRotMatrix(SceneNode *self, s32 out, s32 invert) {
    SVECTOR angles;
    SVECTOR *rotate = &self->coord2->param->rotate;

    if (invert) {
        angles.vx = -rotate->vx;
        angles.vy = -rotate->vy;
        angles.vz = -rotate->vz;
    } else {
        angles = *rotate;
    }
    /* Cast: SceneNode.h types `out` as s32. */
    RotMatrix(&angles, (MATRIX *)out);
}

/* a1 gates a small range (2 <= a1 < 4). When self->model is set and
 * TmdModel__GetBoundsCount(self->model) reports true, fills a stack buffer through
 * this class's own +0x8C slot (SceneNode__GetModelHull, already matched in this
 * unit -- fills it via TmdModel__GetHull(self->model, dest)) then forwards
 * that same buffer, retyped as a TmdHull, into +0x90
 * (SceneNode__TransformAndNotifyParents, also already matched in this unit), with the original
 * a1 passed through as SceneNode__TransformAndNotifyParents's own a2. */
void SceneNode__NotifyWithHull(SceneNode *self, s32 event) {
    /* Sized to reproduce retail's own frame (0x58): SceneNode__GetModelHull's own
     * target (TmdModel__GetHull, TmdModel) writes a TmdHull
     * (include/TmdModel.h: a count word and eight 6-byte corners, 0x34
     * bytes) into its `dest`, so the true destination struct is bigger than
     * the 8 bytes a count and one corner would reserve. */
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

/* Forwards self->model (the TmdModel, held as `void *` in SceneNode.h) and
 * its own 2nd argument straight through to TmdModel__GetHull, untouched.
 * TmdModel__GetHull (TmdModel) is void, so this wrapper is void too. */
void SceneNode__GetModelHull(SceneNode *self, void *dest) {
    TmdModel__GetHull(self->model, dest);
}

/* Copies a1's own count*8 elements into self->unk14->unk24 (via
 * ApplyMatrixToSVArray, both its src and dest args are &a1->unk4 -- computed once,
 * copied, per the disassembly), zeroes unk28/unk2C, stashes a1 into unk30
 * for the duration of a single self->methods->slot30(self, a2) dispatch
 * (an inherited BasicClass slot, not this unit's own code), then clears
 * unk30 again. */
void SceneNode__TransformAndNotifyParents(SceneNode *self, TmdHull *verts, s32 event) {
    ApplyMatrixToSVArray(verts->v, verts->v, verts->count * 8, &self->coord2->workm);
    self->linkTarget = 0;
    self->hitMask = 0;
    self->notifyVerts = verts;
    self->methods->notifyParents(self, event);
    self->notifyVerts = NULL;
}

void SceneNode__OnPadEvent(void) {}

void SceneNode__Update(void) {}

/* a2 selects one of three behaviors: 2 or 3 dispatches through the vtable
 * (self->methods->slotA0), exactly 4 stores a1 into self->linkTarget, and
 * anything else (< 2 or > 4) is a no-op. */
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

/* The corner list this function builds and hands to +0xA8/+0xAC is one
 * TmdHull local: the count header and the eight corners together. */

/* Range-checks `other` against `self` (each axis of position difference
 * must fit in +/-0x4000), then hands off to three vtable slots
 * (+0xA4 = SceneNode__ComposeAndApplyRotation, +0xA8 = SceneNode__CheckBoundsOverlap, +0xAC = SceneNode__RaycastHullAgainstFaces)
 * with the resulting Vec3S16 difference, before registering `other` into
 * self->linkTarget and notifying it via its own +0x038 slot. */
void SceneNode__TryAttachNearby(SceneNode *self, SceneNode *other) {
    LongVec3 *otherPos;
    LongVec3 *selfPos;
    LongVec3 offset;
    TmdVec3 delta;
    s32 mag;
    u8 unused[0x20]; /* sp+0x30, never referenced; reserves retail's slot */
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

    if (offset.x < 0) {
        goto x_neg;
    }
    if (offset.x < 0x4001) {
        goto x_done;
    }
    return;
x_neg:
    mag = ~offset.x + 1;
    if (mag >= 0x4001) {
        return;
    }
x_done:
    if (offset.y < 0) {
        goto y_neg;
    }
    if (offset.y < 0x4001) {
        goto y_done;
    }
    return;
y_neg:
    mag = ~offset.y + 1;
    if (mag >= 0x4001) {
        return;
    }
y_done:
    if (offset.z < 0) {
        goto z_neg;
    }
    if (offset.z < 0x4001) {
        goto z_done;
    }
    return;
z_neg:
    mag = ~offset.z + 1;
    if (mag >= 0x4001) {
        return;
    }
z_done:

    delta.x = offset.x;
    delta.y = offset.y;
    delta.z = offset.z;

    hull.count = other->notifyVerts->count;
    {
        TmdHull *otherHull = other->notifyVerts;
        self->methods->composeAndApplyRotation(self, &delta, hull.v, otherHull->v, hull.count * 8);
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

/* Fills buf1 from self's own +0x84 slot, then folds in every node of the
 * self->unkC list (each node's own +0x84 slot combined into buf1 via
 * MulMatrix2) before using buf1 as ApplyMatrixToSVArray's own "out" argument,
 * twice: once for (arg2, arg3, count), once more for (arg1, arg1, 1) when
 * arg1 is non-NULL. */
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

/* Offsets arg1's corner list by `d` and grows a box `mm` over the moved
 * corners, grows a second box `box` over the model's own bounds records
 * (TmdModel__GetBoundsBuffer's array), and returns 1 if the two boxes overlap on all
 * three axes. Each running min/max is a ternary stored back unconditionally
 * (retail stores every field every iteration), and the source compares
 * with `>` for a min so the slt operands load in retail's order. */
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
    cornerEnd = corner + hull->count * 8;
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

/* Tests the corner list against every model plane. Part 1 averages two
 * diagonal corner pairs into mid[0]/mid[1] and tests that segment against
 * each plane, setting bit i of self->hitMask on a hit; any hit returns at once.
 * Otherwise every 8-corner box k in `list` has its two vertical edges
 * (corner m against corner m+4, m = 1, 2) tested against every plane, and a
 * hit sets plane bit i in self->hitMask and box bit k in *outFlag. `hit` is
 * written only as 0, but retail still tests it. Round 76. Byte levers:
 * the gHitHeightGate gate is two arms that each set the bit, so loop.c sees two
 * equal constant-1 loads (savings 2) and hoists the 1 into $s1; Part 1 walks
 * `p`, `v` and `hi` as pointers. */
extern s32 gHitHeightGate;

s32 SceneNode__RaycastHullAgainstFaces(SceneNode *self, s32 *hullHits, TmdVec3 *delta, TmdHull *hull) {
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
    u8 pad[0x18];

    nearest = 0x7FFFFFFF;

    corner = hull->v;
    opposite = hull->v + 2;
    for (c = center; c < &center[2]; c++) {
        c->x = (corner->x + opposite->x) >> 1;
        c->y = (corner->y + opposite->y) >> 1;
        c->z = (corner->z + opposite->z) >> 1;
        corner += 4;
        opposite += 4;
    }

    self->hitMask = 0;
    boundsCount = TmdModel__GetBoundsCount(self->model);
    hit = 0;
    for (i = 0; i < boundsCount; i++) {
        bounds = TmdModel__GetBoundsBuffer(self->model, i);
        if (ClipSegmentToBox(NULL, bounds, &center[0], &center[1])) {
            if (TmdModel__RaycastFaces(self->model, &nearest, (TmdVec3 *)delta, &height,
                                       (TmdVec3 *)&center[0], (TmdVec3 *)&center[1])) {
                if (gHitHeightGate == 0) {
                    self->hitMask |= 1 << i;
                } else if (height >= 0x201) {
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
            for (j = 0; j < 4; j++) {
                if (j == 1 || j == 2) {
                    if (ClipSegmentToBox(NULL, bounds, corner, corner + 4)) {
                        if (TmdModel__RaycastFaces(self->model, &nearest, (TmdVec3 *)delta, &height,
                                                   (TmdVec3 *)corner, (TmdVec3 *)(corner + 4))) {
                            if (height >= 0x201) {
                                self->hitMask |= 1 << i;
                                *hullHits |= 1 << k;
                            }
                        }
                    }
                }
                corner++;
            }
            corner += 4;
        }
    }

    return (*hullHits != 0);
}

/* Round 41: MATCHED, 118/118, byte-exact. Round 20 got the CFG (a
 * tail-merge/shared-block dispatch, see the git history for the full
 * derivation) and the register mapping exactly right, leaving one
 * standalone residue: an extra `move v1,v0` before the SECOND recursive
 * call's result test, where retail tests $v0 directly. Closed by a
 * first-ever permuter search (`docs/match-reports/ClipSegmentToBox.md`,
 * "Round 41"): the tautological trailing `if (mid.y) return 0; else
 * return 0;` below is not meaningful control flow -- both arms return 0,
 * exactly like the plain `return 0;` it replaces -- but it changes
 * register pressure enough at the tail of the function that GCC 2.6.3
 * drops the otherwise-unavoidable `move v1,v0` and tests $v0 directly,
 * matching retail exactly. Kept because it is what's needed for
 * byte-exactness, not because it means anything; see the report for the
 * hand-lever history this replaced. */
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
        return 1;
    }
    if (code2 != 0) {
        goto shared_test;
    }
    if (out != NULL) {
        BisectSegmentToBox(out, box, p2, p1);
    }
    return 3;

shared_test:
    if (code1 != 0) {
        goto combined;
    }
    if (out != NULL) {
        BisectSegmentToBox(out, box, p1, p2);
    }
    return 2;

combined:
    if ((code1 & code2) != 0) {
        return 0;
    }

    mid.x = (p1->x + p2->x) >> 1;
    mid.y = (p1->y + p2->y) >> 1;
    mid.z = (p1->z + p2->z) >> 1;

    if (p1->x == mid.x && p1->y == mid.y && p1->z == mid.z) {
        return 0;
    }
    if (p2->x == mid.x && p2->y == mid.y && p2->z == mid.z) {
        return 0;
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
        return 0;
    } else {
        return 0;
    }
}

/* Bisects the segment [near, far] against `box` until the midpoint exactly
 * equals one endpoint, writing the running midpoint into `out` every
 * iteration (the caller's real result is whatever `*out` holds when this
 * returns). Each iteration computes an outcode (`flags`, matching the
 * project's already-confirmed `u8`-flags idiom -- an explicit `andi
 * $v0,$v1,0xFF` re-mask appears in retail wherever `flags` is read back)
 * from `box` against the midpoint; a non-zero outcode means the midpoint
 * overshot, so it becomes the new `far`, otherwise it becomes the new
 * `near` -- each written into one of two ping-pong stack buffers so the
 * OTHER endpoint's storage is never disturbed. */
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
            outcode = 8;
        } else if (out->x < box->min.x) {
            outcode = 4;
        }
        if (box->max.y < out->y) {
            outcode |= 2;
        } else if (out->y < box->min.y) {
            outcode |= 1;
        }
        if (box->max.z < out->z) {
            outcode |= 0x20;
        } else if (out->z < box->min.z) {
            outcode |= 0x10;
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

void SceneNode__NoOpSlotB0(void) {}

/* Walks node's parent refs. For each run it finds the next entry whose class
 * kind (low nibble of its method table's first word) is 4. If that entry's
 * tag byte is also 0x34, it calls the entry's +0x010 slot with self. Round 76:
 * the two nested do/while loops are real loops for loop.c, which hoists the
 * literal 4 into $s1. The goto form of earlier rounds had no loop notes, so
 * it needed a named `tag` and could not get retail's register order. */
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

/* This unit's own no-argument vtable getter -- see the extended note on
 * gSceneNodeMethods in include/code_d294.h and the file banner up top. */
SceneNodeMethods *GetSceneNodeMethods(void) {
    return &gSceneNodeMethods;
}
