/* code_d294_b -- the second carve of the Class6B5CC segment (see
 * include/code_d294.h's own banner and code_d294_c.c's file header for the
 * class-identity derivation: Class6B5CC is this game's POSITIONED 3D OBJECT
 * base class, MEASURED onto Psy-Q's GsDOBJ2/GsCOORDINATE2/GsCOORD2PARAM).
 *
 * Covers method-table slots +0x074 through +0x0B4 (tools/classtable.py
 * gClass6B5CCMethods) -- the table's own LAST 17 slots. In ROM order: four more
 * self->unk10 bitfield accessors (the sibling family code_d294.c starts;
 * two renamed this round, `Class6B5CC__GetSetUnk10Flag7`/`Field9`, two
 * held back as `func_` -- proposed `Field0`/`Flag8` -- because their
 * symbol is comment-referenced from other units' own vtable census notes);
 * a rotation-matrix builder (`Class6B5CC__GetRotMatrix`, proposed
 * `Class6B5CC__GetRotMatrix`); a gated read-transform-notify chain
 * (`Class6B5CC__ReadUnk20Data` -> `Class6B5CC__NotifyIfUnk20Active` ->
 * `Class6B5CC__TransformAndNotifyParents`); two vtable no-op stubs
 * (`func_8001D6A4`/`D6AC`, kept `func_` per this class's own
 * `Class6B5CC__func_1d33c` no-op precedent); a command dispatcher over the same
 * "attach" state (`Class6B5CC__DispatchLinkCommand`, proposed `Class6B5CC__DispatchLinkCommand`);
 * a proximity-attach attempt (`Class6B5CC__TryAttachNearby`, MATCHED round
 * 76) that hands off to a rotation compose-and-
 * apply step (`Class6B5CC__ComposeAndApplyRotation`), a corner-list AABB
 * overlap test (`Class6B5CC__CheckBoundsOverlap`, MATCHED round 73), and a
 * plane-classification test (`Class6B5CC__ClassifyAgainstPlanes`, STALL, proposed
 * `Class6B5CC__ClassifyAgainstPlanes` -- the RESOLVED former `gp_rel`
 * blocker, ordinary matching work now); a third no-op stub
 * (`func_8001E49C`); a parent-list notify walk (`Class6B5CC__NotifyTaggedParents`, STALL,
 * proposed `Class6B5CC__NotifyTaggedParents` -- naming only, matching
 * this one is explicitly out of scope for this round); this unit's own
 * vtable getter (`GetClass6B5CCMethods`, proposed `GetClass6B5CCMethods`,
 * cross-unit); and a small free-function pair for segment/AABB clipping
 * (`ClipSegmentToBox`/`BisectSegmentToBox`, both MATCHED, no `self` at
 * all) that `Class6B5CC__CheckBoundsOverlap` and `Class6B5CC__ClassifyAgainstPlanes` build on.
 *
 * Two functions remain INCLUDE_ASM: `Class6B5CC__ClassifyAgainstPlanes`,
 * `Class6B5CC__NotifyTaggedParents` -- documented stalls, see
 * docs/match-reports/. (`Class6B5CC__CheckBoundsOverlap` matched round 73,
 * `Class6B5CC__TryAttachNearby` round 76.)
 */

#include "common.h"
#include "code_d294.h"

/* Sibling of Class6B5CC__SetDisplay/D374/D3A0/D3CC/D3F8 (code_d294.c): a thin
 * wrapper around GetSetBitField over &self->unk10, shift 0 width 3. Raw
 * pass-through value and raw pass-through result -- same shape as
 * Class6B5CC__SetSemiTrans/D3A0/D3F8 (no `== 0` on either side). */
u32 Class6B5CC__GetSetUnk10Field0(Class6B5CCObj *self, u32 a1) {
    return GetSetBitField(&self->attribute, 0, 3, a1);
}

/* Sibling of Class6B5CC__SetDisplay (the ONLY one of the five already-matched
 * self->unk10 bitfield accessors that both converts its input to a boolean
 * (`a1 == 0`) AND inverts its own result (`== 0`)). This function does
 * exactly that double-inversion, at shift 7 width 1, hence the same `s32`
 * return type as Class6B5CC__SetDisplay rather than the plain `u32` of the other
 * three siblings. */
s32 Class6B5CC__GetSetUnk10Flag7(Class6B5CCObj *self, s32 a1) {
    return GetSetBitField(&self->attribute, 7, 1, a1 == 0) == 0;
}

/* Same family as Class6B5CC__GetSetUnk10Field0, shift 9 width 3. Raw pass-through. */
u32 Class6B5CC__GetSetUnk10Field9(Class6B5CCObj *self, u32 a1) {
    return GetSetBitField(&self->attribute, 9, 3, a1);
}

/* Same family as Class6B5CC__GetSetUnk10Flag7: double-inversion shape, shift 8 width 1. */
s32 Class6B5CC__GetSetUnk10Flag8(Class6B5CCObj *self, s32 a1) {
    return GetSetBitField(&self->attribute, 8, 1, a1 == 0) == 0;
}

/* self->unk14->unk44 is a 0x28-byte heap block whose +0x10 holds an
 * S16Quad_d294 (see include/code_d294.h). a2 selects between negating x/y/z
 * into a local copy (the 4th short left uninitialised, exactly as retail's
 * own negate path never stores to it) or copying the quad verbatim, then
 * forwards the result -- plus a1, passed straight through -- to the PsyQ
 * helper RotMatrix. */
void Class6B5CC__GetRotMatrix(Class6B5CCObj *self, s32 a1, s32 a2) {
    S16Quad_d294 buf;
    S16Quad_d294 *src = &self->coord2->param->rotate;

    if (a2) {
        buf.x = -src->x;
        buf.y = -src->y;
        buf.z = -src->z;
    } else {
        buf = *src;
    }
    RotMatrix(&buf, a1);
}

/* a1 gates a small range (2 <= a1 < 4). When self->unk20 is set and
 * func_8001F3A4(self->unk20) reports true, fills a stack buffer through
 * this class's own +0x8C slot (Class6B5CC__ReadUnk20Data, already matched in this
 * unit -- fills it via func_8001F51C(self->unk20, dest)) then forwards
 * that same buffer, retyped as a GenericCountList_d294, into +0x90
 * (Class6B5CC__TransformAndNotifyParents, also already matched in this unit), with the original
 * a1 passed through as Class6B5CC__TransformAndNotifyParents's own a2. */
void Class6B5CC__NotifyIfUnk20Active(Class6B5CCObj *self, s32 a1) {
    /* Sized to reproduce retail's own frame (0x58): Class6B5CC__ReadUnk20Data's own
     * target (func_8001F51C, PsyQ, asm/psyq_fa50.s, not
     * decompiled here) fills fields out past +0x32 of its own `dest`
     * argument, so the true destination struct is bigger than the 8 bytes
     * GenericCountList_d294 alone would reserve -- not derived beyond its
     * size, since the field layout past what Class6B5CC__TransformAndNotifyParents itself reads
     * (+0x0/+0x4) is PsyQ-internal. */
    u8 buf[0x38];

    if (a1 >= 4) {
        return;
    }
    if (a1 < 2) {
        return;
    }
    if (self->unk20 == NULL) {
        return;
    }
    if (!func_8001F3A4(self->unk20)) {
        return;
    }
    self->methods->readUnk20Data(self, buf);
    self->methods->transformAndNotifyParents(self, (GenericCountList_d294 *)buf, a1);
}

/* Forwards self->unk20 (still opaque, retyped `void *` this round -- see
 * include/code_d294.h) and its own 2nd argument straight through to
 * func_8001F51C, untouched. func_8001F51C's own body (psyq_fa50.s)
 * has no deliberate return value -- see the extern's own comment -- so this
 * wrapper is void, not `return func_8001F51C(...)`. */
void Class6B5CC__ReadUnk20Data(Class6B5CCObj *self, void *dest) {
    func_8001F51C(self->unk20, dest);
}

/* Copies a1's own count*8 elements into self->unk14->unk24 (via
 * ApplyMatrixToSVArray, both its src and dest args are &a1->unk4 -- computed once,
 * copied, per the disassembly), zeroes unk28/unk2C, stashes a1 into unk30
 * for the duration of a single self->methods->slot30(self, a2) dispatch
 * (an inherited BasicClass slot, not this unit's own code), then clears
 * unk30 again. */
void Class6B5CC__TransformAndNotifyParents(Class6B5CCObj *self, GenericCountList_d294 *a1, s32 a2) {
    ApplyMatrixToSVArray(&a1->unk4, &a1->unk4, a1->unk0 * 8, &self->coord2->unk24);
    self->unk28 = 0;
    self->unk2C = 0;
    self->unk30 = a1;
    self->methods->notifyParents(self, a2);
    self->unk30 = NULL;
}

void func_8001D6A4(void) {
}

void func_8001D6AC(void) {
}

/* a2 selects one of three behaviors: 2 or 3 dispatches through the vtable
 * (self->methods->slotA0), exactly 4 stores a1 into self->unk28, and
 * anything else (< 2 or > 4) is a no-op. */
void Class6B5CC__DispatchLinkCommand(Class6B5CCObj *self, s32 a1, s32 a2) {
    switch (a2) {
    case 2:
    case 3:
        self->methods->tryAttachNearby(self);
        break;
    case 4:
        self->unk28 = a1;
        break;
    }
}

/* The corner list this function builds and hands to +0xA8/+0xAC: the count
 * header and eight corners are ONE local (count at sp+0x50, corners at
 * sp+0x54); round 76. Same layout as CornerList_d294 with the array made
 * explicit. */
typedef struct AttachCornerList_d294b {
    s32 count;
    Vec3S16_d294 v[8];
} AttachCornerList_d294b;

/* Range-checks `other` against `self` (each axis of position difference
 * must fit in +/-0x4000), then hands off to three vtable slots
 * (+0xA4 = Class6B5CC__ComposeAndApplyRotation, +0xA8 = Class6B5CC__CheckBoundsOverlap, +0xAC = Class6B5CC__ClassifyAgainstPlanes)
 * with the resulting Vec3S16 difference, before registering `other` into
 * self->unk28 and notifying it via its own +0x038 slot. */
void Class6B5CC__TryAttachNearby(Class6B5CCObj *self, GenericObj_d294 *other) {
    Vec3_d294 *posA;
    Vec3_d294 *posB;
    Vec3_d294 diffRaw;
    Vec3S16_d294 diff;
    s32 abs;
    u8 unused[0x20]; /* sp+0x30, never referenced; reserves retail's slot */
    AttachCornerList_d294b list;

    if (self->unk20 == NULL) {
        return;
    }
    if (!func_8001F3A4(self->unk20)) {
        return;
    }

    posA = (other->parent != NULL) ? (Vec3_d294 *)other->coord2->unk38 : NULL;
    diffRaw = *posA;

    posB = (self->parent != NULL) ? (Vec3_d294 *)self->coord2->unk38 : NULL;
    diffRaw.x = diffRaw.x - posB->x;
    diffRaw.y = diffRaw.y - posB->y;
    diffRaw.z = diffRaw.z - posB->z;

    if (diffRaw.x < 0) {
        goto x_neg;
    }
    if (diffRaw.x < 0x4001) {
        goto x_done;
    }
    return;
x_neg:
    abs = ~diffRaw.x + 1;
    if (abs >= 0x4001) {
        return;
    }
x_done:
    if (diffRaw.y < 0) {
        goto y_neg;
    }
    if (diffRaw.y < 0x4001) {
        goto y_done;
    }
    return;
y_neg:
    abs = ~diffRaw.y + 1;
    if (abs >= 0x4001) {
        return;
    }
y_done:
    if (diffRaw.z < 0) {
        goto z_neg;
    }
    if (diffRaw.z < 0x4001) {
        goto z_done;
    }
    return;
z_neg:
    abs = ~diffRaw.z + 1;
    if (abs >= 0x4001) {
        return;
    }
z_done:

    diff.x = diffRaw.x;
    diff.y = diffRaw.y;
    diff.z = diffRaw.z;

    list.count = other->unk30->unk0;
    {
        GenericCountList_d294 *countList = other->unk30;
        self->methods->composeAndApplyRotation(self, &diff, list.v, &countList->unk4, list.count * 8);
    }

    if (!self->methods->checkBoundsOverlap(self, &list, &diff)) {
        return;
    }
    if (!self->methods->classifyAgainstPlanes(self, other->unk2C, &diff, &list)) {
        return;
    }

    self->unk28 = other;
    other->methods->onNotify(other, self, 4);
}

/* Fills buf1 from self's own +0x84 slot, then folds in every node of the
 * self->unkC list (each node's own +0x84 slot combined into buf1 via
 * MulMatrix2) before using buf1 as ApplyMatrixToSVArray's own "out" argument,
 * twice: once for (arg2, arg3, count), once more for (arg1, arg1, 1) when
 * arg1 is non-NULL. */
void Class6B5CC__ComposeAndApplyRotation(Class6B5CCObj *self, void *arg1, void *arg2, void *arg3, s32 count) {
    u8 buf2[0x20];
    u8 buf1[0x20];
    UnkOwner_d294 *node;

    self->methods->getRotMatrix(self, buf1, 1);

    node = self->parent;
    if (node != NULL) {
        do {
            node->methods->slot84(node, buf2, 1);
            MulMatrix2(buf2, buf1);
            node = node->next;
        } while (node != NULL);
    }

    ApplyMatrixToSVArray(arg2, arg3, count, buf1);
    if (arg1 != NULL) {
        ApplyMatrixToSVArray(arg1, arg1, 1, buf1);
    }
}

/* Offsets arg1's corner list by `d` and grows a box `mm` over the moved
 * corners, grows a second box `box` over the model's own bounds records
 * (func_8001F50C's array), and returns 1 if the two boxes overlap on all
 * three axes. Each running min/max is a ternary stored back unconditionally
 * (retail stores every field every iteration), and the source compares
 * with `>` for a min so the slt operands load in retail's order. */
s32 Class6B5CC__CheckBoundsOverlap(Class6B5CCObj *self, void *arg1, Vec3S16_d294 *d) {
    CornerList_d294 *list;
    Vec3S16_d294 *v;
    Vec3S16_d294 *end;
    BoundsBox_d294 mm;
    BoundsBox_d294 *b;
    BoundsBox_d294 *p;
    BoundsBox_d294 *end2;
    s32 n;
    s32 ret;
    BoundsBox_d294 box;

    list = (CornerList_d294 *)arg1;
    v = &list->hdr;
    v->x += d->x;
    v->y += d->y;
    end = v + list->count * 8;
    v->z += d->z;
    mm.lo = *v;
    mm.hi = *v;
    b = &mm;
    for (v++; v < end; v++) {
        v->x += d->x;
        v->y += d->y;
        v->z += d->z;
        b->lo.x = (b->lo.x > v->x) ? v->x : b->lo.x;
        b->lo.y = (b->lo.y > v->y) ? v->y : b->lo.y;
        b->lo.z = (b->lo.z > v->z) ? v->z : b->lo.z;
        b->hi.x = (b->hi.x < v->x) ? v->x : b->hi.x;
        b->hi.y = (b->hi.y < v->y) ? v->y : b->hi.y;
        b->hi.z = (b->hi.z < v->z) ? v->z : b->hi.z;
    }

    func_8001F4E4(self->unk20);
    p = (BoundsBox_d294 *)func_8001F50C(self->unk20, 0);
    n = func_8001F3A4(self->unk20);
    box = *p;
    end2 = p + n;
    for (p++; p < end2; p++) {
        box.lo.x = (box.lo.x > p->lo.x) ? p->lo.x : box.lo.x;
        box.lo.y = (box.lo.y > p->lo.y) ? p->lo.y : box.lo.y;
        box.lo.z = (box.lo.z > p->lo.z) ? p->lo.z : box.lo.z;
        box.hi.x = (box.hi.x < p->hi.x) ? p->hi.x : box.hi.x;
        box.hi.y = (box.hi.y < p->hi.y) ? p->hi.y : box.hi.y;
        box.hi.z = (box.hi.z < p->hi.z) ? p->hi.z : box.hi.z;
    }

    ret = 0;
    if (!(mm.lo.z > box.hi.z) && !(mm.hi.z < box.lo.z) && !(mm.lo.x > box.hi.x) &&
        !(mm.hi.x < box.lo.x) && !(mm.lo.y > box.hi.y)) {
        ret = !(mm.hi.y < box.lo.y);
    }
    return ret;
}


#ifdef NON_MATCHING
/* NON_MATCHING: 29/199 words, length exact. Residue: whole-function
 * register-pressure interaction, not a simple two-value swap -- Part 1's
 * own induction-variable decomposition (four raw read pointers plus two
 * write pointers in retail, against the indexed mid[row].x/y/z form here)
 * competes for callee-saved registers with everything else in the function
 * (docs/match-reports/Class6B5CC__ClassifyAgainstPlanes.md). Hand-derived. */
extern s32 func_8001F8B8(void *arg0, s32 *arg1, Vec3S16_d294 *arg2, s32 *arg3, Vec3S16_d294 *arg4, Vec3S16_d294 *arg5);
extern s32 D_8008A838;

s32 Class6B5CC__ClassifyAgainstPlanes(Class6B5CCObj *self, s32 *outFlag, Vec3S16_d294 *diff, void *list) {
    Vec3S16_d294 mid[2];
    s16 *loPtr;
    s16 *hiPtr;
    s32 row;
    s32 count1;
    s32 i;
    Sixteen6_d294 *plane;
    s32 flag2;
    s32 cnt2;
    s32 j;
    u8 *rowBase;
    s32 k;
    s32 bitJ;
    s32 bitK;
    s32 m;
    s32 bigConst;
    s32 outWord;
    u8 pad[0x18];

    bigConst = 0x7FFFFFFF;

    loPtr = (s16 *)((u8 *)list + 4);
    hiPtr = (s16 *)((u8 *)list + 0x10);
    for (row = 0; row < 2; row++) {
        mid[row].x = (loPtr[0] + hiPtr[0]) >> 1;
        mid[row].y = (loPtr[1] + hiPtr[1]) >> 1;
        mid[row].z = (loPtr[2] + hiPtr[2]) >> 1;
        loPtr = (s16 *)((u8 *)loPtr + 0x18);
        hiPtr = (s16 *)((u8 *)hiPtr + 0x18);
    }

    count1 = func_8001F3A4(self->unk20);
    self->unk2C = 0;
    flag2 = 0;
    for (i = 0; i < count1; i++) {
        plane = func_8001F50C(self->unk20, i);
        if (ClipSegmentToBox(NULL, (BoundsBox_d294 *)plane, &mid[0], &mid[1])) {
            if (func_8001F8B8(self->unk20, &bigConst, diff, &outWord, &mid[0], &mid[1])) {
                if (D_8008A838 == 0 || outWord >= 0x201) {
                    self->unk2C |= (1 << i);
                }
            }
        }
    }

    if (self->unk2C != 0) {
        *outFlag = 1;
        if (flag2 != 0) {
            return 2;
        }
        return 1;
    }

    *outFlag = 0;
    cnt2 = *(s32 *)list;
    for (j = 0; j < count1; j++) {
        plane = func_8001F50C(self->unk20, j);
        bitJ = 1 << j;
        rowBase = (u8 *)list + 4;
        for (k = 0; k < cnt2; k++) {
            bitK = 1 << k;
            for (m = 0; m < 4; m++) {
                if (m == 1 || m == 2) {
                    u8 *rowM = rowBase;
                    u8 *rowMplus1 = rowBase + 0x18;
                    if (ClipSegmentToBox(NULL, (BoundsBox_d294 *)plane, (Vec3S16_d294 *)rowM, (Vec3S16_d294 *)rowMplus1)) {
                        if (func_8001F8B8(self->unk20, &bigConst, diff, &outWord, (Vec3S16_d294 *)rowM, (Vec3S16_d294 *)rowMplus1)) {
                            if (D_8008A838 == 0 || outWord >= 0x201) {
                                self->unk2C |= bitJ;
                                *outFlag |= bitK;
                            }
                        }
                    }
                }
                rowBase += 6;
            }
            rowBase += 0x18;
        }
    }

    return (*outFlag != 0);
}
#else
INCLUDE_ASM("asm/nonmatchings/code_d294_b", Class6B5CC__ClassifyAgainstPlanes);
#endif

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
s32 ClipSegmentToBox(Vec3S16_d294 *out, BoundsBox_d294 *box, Vec3S16_d294 *p1, Vec3S16_d294 *p2) {
    u8 r1;
    u8 r2;
    Vec3S16_d294 mid;

    r1 = CalcBoxOutcode(box, p1);
    r2 = CalcBoxOutcode(box, p2);

    if (r1 == 0) {
        if (r2 != 0) {
            goto shared_test;
        }
        return 1;
    }
    if (r2 != 0) {
        goto shared_test;
    }
    if (out != NULL) {
        BisectSegmentToBox(out, box, p2, p1);
    }
    return 3;

shared_test:
    if (r1 != 0) {
        goto combined;
    }
    if (out != NULL) {
        BisectSegmentToBox(out, box, p1, p2);
    }
    return 2;

combined:
    if ((r1 & r2) != 0) {
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
void BisectSegmentToBox(Vec3S16_d294 *out, BoundsBox_d294 *box, Vec3S16_d294 *near, Vec3S16_d294 *far) {
    Vec3S16_d294 buf0;
    Vec3S16_d294 buf1;
    Vec3S16_d294 *dst;
    u8 flags;

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

        flags = 0;
        if (box->hi.x < out->x) {
            flags = 8;
        } else if (out->x < box->lo.x) {
            flags = 4;
        }
        if (box->hi.y < out->y) {
            flags |= 2;
        } else if (out->y < box->lo.y) {
            flags |= 1;
        }
        if (box->hi.z < out->z) {
            flags |= 0x20;
        } else if (out->z < box->lo.z) {
            flags |= 0x10;
        }

        if (flags != 0) {
            dst = &buf1;
            far = dst;
        } else {
            dst = &buf0;
            near = dst;
        }
        *dst = *out;
    }
}

void func_8001E49C(void) {
}

#ifdef NON_MATCHING
/* NON_MATCHING: 48/54 words, length exact. Residue: register identity --
 * self and the literal tag value 4 land in $s1/$s2 swapped from retail's
 * own assignment (docs/match-reports/Class6B5CC__NotifyTaggedParents.md).
 * The preserved best-scoring body indirects self/node/the tag literal
 * through named locals and splits the tag comparison into its own
 * statement, both purely to influence register allocation; this body
 * writes the plain, direct form instead -- see the report for the
 * byte-shaped variant. Hand-derived. */
void Class6B5CC__NotifyTaggedParents(Class6B5CCObj *self, void *node) {
    GenericObj_d294 *entry;
    void *cursor;

    entry = NULL;
loop:
    BasicClass__GetNextParentRef(node, &entry, &cursor);
    if (entry == NULL) {
        goto check_cursor;
    }
    if ((entry->methods->header & 0xF) == 4) {
        goto dispatch;
    }
check_cursor:
    if (cursor != NULL) {
        goto loop;
    }
    entry = NULL;
dispatch:
    if (entry == NULL) {
        goto tail;
    }
    if (*(u8 *)entry->methods != 0x34) {
        goto tail;
    }
    entry->methods->slot10(entry, self);
tail:
    if (cursor != NULL) {
        goto loop;
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/code_d294_b", Class6B5CC__NotifyTaggedParents);
#endif

/* This unit's own no-argument vtable getter -- see the extended note on
 * gClass6B5CCMethods in include/code_d294.h and the file banner up top. */
Class6B5CCMethods *GetClass6B5CCMethods(void) {
    return &gClass6B5CCMethods;
}
