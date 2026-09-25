/*
 * class_3bb8c_p -- vram 0x800574C4..0x80057DBC, carved round 17
 * (2026-09-04), immediately behind class_3bb8c_o. Two DreamSys-method
 * families plus one unrelated constructor:
 *
 *  - A local-offset family (DreamSys__ApplyOffsetSlot0/1, the shared
 *    DreamSys__ApplyOffsetSlotAndNotify helper, and the
 *    DreamSys__DispatchOffsetSlot0/SlotC4 + DreamSys__ApplyOffsetOrFindNearby
 *    pair): writes one component of a shared local-space offset buffer,
 *    applies it as a rotated positional nudge via the inherited
 *    BaseObjO__ApplyRotatedVec14, resets that component, and falls back to
 *    a grid-based nearby-link search (DreamSys__FindNearbyLink,
 *    DreamSys__BuildLinkQueries, DreamSys__ScanLinkCandidates,
 *    DreamSys__ScanGridWindow, DreamSys__AcceptGridElem) when the offset
 *    alone did not attach to a link (DreamSys::unk_0x28).
 *  - A link-command dispatch pair (DreamSys__DispatchLinkCommand[AndTryAttach])
 *    forwarding through the inherited Class6B5CC base table and, for a
 *    count in [5,9), the object's own inherited TryAttachNearby slot.
 *  - DreamSys__SetLastOffsetValue/SetPendingExtra/GetBaseMethods: plain
 *    setters/getter.
 *  - New_D800879C4 + D800879C4__D800879C4: allocator and constructor for
 *    an unrelated, still-uncarved sibling class (table D_800879C4, in
 *    class_3bb8c_q.s).
 *
 * No stalls: DreamSys__BuildLinkQueries, the last one, matched in round 75
 * (2-argument method call, see its report). No switch jump table in this slice, and no gp_rel/addiu_at/
 * nop_mflo_mfhi anywhere in it (all three are resolved toolchain
 * constructs anyway, CLAUDE.md "Open toolchain blockers").
 */
#include "common.h"
#include "DreamSys.h"
#include "Sprite.h"

/* Two-element s16 array -- DreamSys__ApplyOffsetSlot0 and DreamSys__ApplyOffsetSlot1 each write one
 * element (index 0 and 1 respectively) via a plain `sh` through a pointer
 * computed as %hi/%lo of `D_8008ABA4 + 2*index`, so splat's single-word
 * dlabel is really this 2-element array, not a lone s32 (round 2026-09-04).
 * Not referenced anywhere else in the repo (checked with grep), so this is
 * this unit's own reading -- kept local rather than added to a shared
 * header. */
extern s16 D_8008ABA4[2];

void DreamSys__ApplyOffsetSlotAndNotify(DreamSys *self, s16 *slot, s32 val, void *extra, volatile s32 count);

void DreamSys__ApplyOffsetSlot0(DreamSys *self, s32 val, void *extra) {
    DreamSys__ApplyOffsetSlotAndNotify(self, &D_8008ABA4[0], val, extra, 7);
}

void DreamSys__ApplyOffsetSlot1(DreamSys *self, s32 val, void *extra) {
    DreamSys__ApplyOffsetSlotAndNotify(self, &D_8008ABA4[1], val, extra, 8);
}

/* `count` is `volatile` so it stays a stack reference reloaded at its one use
 * site, rather than being promoted to a callee-saved register across the
 * intervening BaseObjO__ApplyRotatedVec14 call -- confirmed with a standalone reproducer
 * through the pinned toolchain: dropping `volatile` grows the frame by one
 * callee-saved register (s3) and changes 0x1c/0x20 byte offsets throughout,
 * which is not what retail does (round 2026-09-04).
 *
 * `val` arrives as `s32` (its callers forward an incoming register with no
 * conversion -- typing it `s16` here made the CALLERS re-sign-extend it on
 * every call, which retail does not do), but the two stores below are
 * genuinely 16-bit (`sh`). Truncating once into a local `s16` and storing
 * THAT (rather than truncating `val` twice inline) is what reproduces
 * retail's callee-saved register assignment for `slot`/`extra`
 * (confirmed with a standalone reproducer: inline truncation swaps which
 * of s0/s1 holds which, round 2026-09-04). */
void DreamSys__ApplyOffsetSlotAndNotify(DreamSys *self, s16 *slot, s32 val, void *extra, volatile s32 count) {
    s16 val16 = (s16) val;
    *slot = val16;
    self->lastOffsetValue = val16;
    self->vt->BaseObjO__ApplyRotatedVec14(self, &D_8008ABA4[0]);
    *slot = 0;
    if (extra != NULL) {
        self->vt->DreamSys__NotifyLinkAttempt(self, count);
    }
}

void DreamSys__ApplyOffsetOrFindNearby(DreamSys *self, void (*callback)(DreamSys *, s32, void *), s32 val, void *extra);

void DreamSys__DispatchOffsetSlotC4(DreamSys *self, s32 val, void *extra) {
    DreamSys__ApplyOffsetOrFindNearby(self, self->vt->BaseObjO__func_5748c, val, extra);
}

void DreamSys__DispatchOffsetSlot0(DreamSys *self, s32 val, void *extra) {
    DreamSys__ApplyOffsetOrFindNearby(self, self->vt->DreamSys__ApplyOffsetSlot0, val, extra);
}

void DreamSys__NoOpSlotD8(void) {
}

s32 DreamSys__FindNearbyLink(DreamSys *self);

void DreamSys__ApplyOffsetOrFindNearby(DreamSys *self, void (*callback)(DreamSys *, s32, void *), s32 val, void *extra) {
    self->linkTarget = NULL;
    callback(self, val, extra);
    if (self->linkTarget == NULL) {
        DreamSys__FindNearbyLink(self);
    }
}

/* A 12-byte {s16,s16,s32,s32} query/result record. Built by this unit's
 * own DreamSys__BuildLinkQueries into caller-supplied buffers, and
 * walked as an array (stride 0xC) by DreamSys__ScanLinkCandidates. Describes a
 * rectangular window of the grid DreamSys__ScanGridWindow walks: `startCol`/
 * `startRow` is the window's origin bucket, `numCols`/`numRows` its extent --
 * confirmed directly from that function's own body (round 57 naming pass). */
typedef struct GridQuery {
    s16 startCol;
    s16 startRow;
    s32 numCols;
    s32 numRows;
} GridQuery;

/* A grid-bucket linked-list node: singly-linked chain at +0x38, the same
 * "self-typed next pointer" idiom already established for `EntryChildObj`
 * in include/class_3bb8c.h (an UNRELATED class, per this round's own
 * research -- convergent shape, not a shared type). Walked by
 * DreamSys__ScanGridWindow; matched against with DreamSys__AcceptGridElem (already matched,
 * this unit, below). */
typedef struct GridElem {
    u8 pad00[0x38];
    struct GridElem *next;
} GridElem;

/* self->linkMgr's pointee dereferences to one of these via its own
 * `+0x4` field (a s16 flag at +0x2C, read by DreamSys__ScanLinkCandidates, and a second
 * s16 at +0x32, read by DreamSys__BuildLinkQueries) and, when treated as
 * DreamSys__ScanGridWindow's 5th argument, a grid-array base pointer at `+0x10`.
 * Multiple independent call sites agree on this shape; kept opaque
 * beyond the fields actually read. */
typedef struct GridArrElemInner {
    u8 pad00[0x2C];
    /* Gates whether DreamSys__ScanLinkCandidates processes this array
     * entry at all (`if (elem->info->enabled != 0)`) -- confirmed
     * directly from that function's own body. */
    s16 enabled;
    u8 pad2E[0x32 - 0x2E];
    s16 unk32;
} GridArrElemInner;
typedef struct GridArrElem {
    u8 pad00[0x4];
    /* Per-entry descriptor -- only its `enabled` flag is read by this
     * unit's matched code. */
    GridArrElemInner *info;
    u8 pad08[0x10 - 0x8];
    /* The grid-array base pointer DreamSys__ScanGridWindow indexes as
     * `(GridElem **)` (confirmed directly from that function's own
     * body). */
    GridElem **buckets;
} GridArrElem;

/* Output buffer filled in by DreamSysUnk4CMethods::queryLinkAtPos (see
 * include/DreamSys.h) and read back by this unit's own DreamSys__BuildLinkQueries.
 * Only the three fields actually touched are named. `queryCol`/`queryRow`
 * feed straight into GridQuery::startCol/startRow (DreamSys__BuildLinkQueries,
 * matched round 75; round 57 naming pass). */
typedef struct LinkQueryBuf {
    u8 pad00[0x2];
    s8 queryCol;
    s8 queryRow;
    u8 pad04[0x24 - 0x4];
    GridArrElem *source;
    u8 pad28[0x30 - 0x28];
} LinkQueryBuf;

void *DreamSys__AcceptGridElem(void *arg0, void *arg1, void *arg2);
s32 DreamSys__BuildLinkQueries(DreamSys *self, GridQuery *arr1, GridArrElem **arr2, LinkQueryBuf *arg3, s32 arg4);
void *DreamSys__ScanLinkCandidates(DreamSys *self, void *arg1, void *arg2, s32 count, GridQuery *arr1, GridArrElem **arr2);

s32 DreamSys__FindNearbyLink(DreamSys *self) {
    LinkQueryBuf sp18;
    GridQuery sp48[3];
    /* No known field needs this gap; empirically required to reproduce
     * retail's exact stack layout for sp78/sp88 below (round 2026-09-04,
     * see this function's match report). */
    u8 pad48Tail[8];
    GridArrElem *sp78[3];
    DreamSysVec3 sp88;

    if (self->linkMgr != NULL) {
        void *pos = (u8 *) self->unk_0x14 + 0x18;

        if (self->linkMgr->methods->queryLinkAtPos(self->linkMgr, &sp18, pos) == 0) {
            s32 count = DreamSys__BuildLinkQueries(self, sp48, sp78, &sp18, 1);
            void *result = DreamSys__ScanLinkCandidates(self, &sp88, pos, count, sp48, sp78);

            self->linkTarget = result;
            if (result != NULL) {
                self->vt->BaseObjO__AddVec14(self, &sp88);
                self->vt->DreamSys__NotifyLinkAttempt(self, -1);
                return 1;
            }
            self->vt->DreamSys__NotifyLinkAttempt(self, -2);
            return 0;
        }
    }
    return 0;
}

s32 DreamSys__BuildLinkQueries(DreamSys *self, GridQuery *arr1, GridArrElem **arr2, LinkQueryBuf *arg3, s32 arg4) {
    s32 f2 = arg3->queryCol;
    s32 f3 = arg3->queryRow;
    s32 numCols;
    s32 numRows;
    s32 col;
    s32 row;
    s32 idx;

    numRows = !(arg4 & 1) ? ++arg4 : arg4;
    numCols = numRows;
    col = f2;
    row = f3;
    idx = 1;
    if (arg4 == 1) {
        DreamSysUnk4CObj *unk4C;
        DreamSysUnk4C68Obj *unk68;
        GridArrElem *src;
        s16 s3;
        s32 pos;

        arr1[0].startCol = col;
        arr1[0].startRow = row;
        arr1[0].numCols = numRows;
        arr1[0].numRows = numRows;
        src = arg3->source;
        arr2[0] = src;
        unk4C = self->linkMgr;
        unk68 = unk4C->unk_0x68;
        if (unk68->unk_0x4 != idx) {
            return 1;
        }
        s3 = src->info->unk32;
        pos = s3 + 1;
        if (pos < unk68->unk_0x2) {
            arr2[1] = unk4C->methods->getGridArrElemAt(unk4C, pos);
            idx = 2;
            arr1[1] = arr1[0];
        }
        pos = s3 - 1;
        if (pos >= 0) {
            arr2[idx] = unk4C->methods->getGridArrElemAt(unk4C, pos);
            arr1[idx] = arr1[0];
            idx++;
        }
        return idx;
    }

    if (col == 0) {
        numCols = numRows - 1;
    } else {
        col--;
    }
    if (f2 == 0x13) {
        numCols--;
    }
    if (f3 == 0x13) {
        numRows--;
    } else {
        row++;
    }
    arr1[0].startCol = col;
    if (f3 == 0) {
        numRows--;
    }
    arr1[0].startRow = row;
    arr1[0].numCols = numCols;
    arr1[0].numRows = numRows;
    arr2[0] = arg3->source;
    return 1;
}

void *DreamSys__ScanGridWindow(DreamSys *self, void *arg1, void *arg2, GridQuery *query, GridArrElem *source);

/* Walks `count` entries of `arr1` (a `GridQuery[]`, stride 0xC) paired
 * element-for-element with `arr2` (a `GridArrElem *[]`, stride 4),
 * skipping any entry whose `GridArrElem` doesn't have its `enabled` flag
 * set, and calling `DreamSys__ScanGridWindow` on the rest; returns the first
 * non-NULL result, or NULL if every entry was skipped or came back empty
 * (round 2026-09-04). */
void *DreamSys__ScanLinkCandidates(DreamSys *self, void *arg1, void *arg2, s32 count, GridQuery *arr1, GridArrElem **arr2) {
    s32 i;

    for (i = 0; i < count;) {
        GridArrElem *elem = *arr2;
        i++;
        if (elem->info->enabled != 0) {
            void *result = DreamSys__ScanGridWindow(self, arg1, arg2, arr1, elem);
            if (result != NULL) {
                return result;
            }
        }
        arr1 = (GridQuery *) ((u8 *) arr1 + 0xC);
        arr2++;
    }
    return NULL;
}

/* Scans a rectangular window of a grid of `GridElem` bucket lists, rooted
 * at `source->buckets`, `query->numRows` rows by `query->numCols` columns,
 * starting at row `query->startRow`, column `query->startCol` (each row is
 * 0x50 bytes = 20 bucket-head pointers; each column step is one bucket-head
 * pointer, 4 bytes). For each bucket, tries `DreamSys__AcceptGridElem` against the
 * head first, then each linked element in turn (`->next`), returning the
 * first one `DreamSys__AcceptGridElem` accepts (non-NULL); NULL if the whole window
 * comes up empty. `self` (this function's own first argument) is read
 * from `a0` in the disassembly but never touched by the body -- present
 * only to match its caller's calling convention (round 2026-09-04). */
void *DreamSys__ScanGridWindow(DreamSys *self, void *arg1, void *arg2, GridQuery *query, GridArrElem *source) {
    s32 row, col;
    GridElem **bucket;

    bucket = (GridElem **) ((u8 *) source->buckets + query->startRow * 0x50 + query->startCol * 4);
    for (row = 0; row < query->numRows; row++) {
        for (col = 0; col < query->numCols; col++) {
            GridElem *node;

            if (DreamSys__AcceptGridElem(*bucket, arg1, arg2) != NULL) {
                return *bucket;
            }
            for (node = (*bucket)->next; node != NULL; node = node->next) {
                if (DreamSys__AcceptGridElem(node, arg1, arg2) != NULL) {
                    return node;
                }
            }
            bucket++;
        }
        bucket = (GridElem **) ((u8 *) bucket - (query->numCols * 4 + 0x50));
    }
    return NULL;
}

/* The real signature, established when code_d294_c matched this function in
 * round 57: it is a Class6B5CC method taking (self, out, target). This unit
 * had long declared it `(void)` and called it with no arguments, which is
 * byte-identical here only because arg0-arg2 are already in $a0-$a2 -- the
 * byte oracle cannot see a wrong prototype. Spelled out so the forwarding is
 * visible; verified byte-exact. */
extern s32 func_8001E7BC(void *self, void *out, void *target);

void *DreamSys__AcceptGridElem(void *arg0, void *arg1, void *arg2) {
    if (arg0 != NULL) {
        if (func_8001E7BC(arg0, arg1, arg2) != 0) {
            return arg0;
        }
    }
    return NULL;
}


void DreamSys__DispatchLinkCommandAndTryAttach(DreamSys *self, void *arg1, s32 count) {
    GetClass6B5CCMethods()->dispatchLinkCommand((Class6B5CC *)self, arg1, count);
    if (count < 9) {
        if (count >= 5) {
            self->vt->tryAttachNearby(self, arg1, count);
        }
    }
}

void DreamSys__DispatchLinkCommand(DreamSys *self, void *arg1, s32 count) {
    GetClass6B5CCMethods()->dispatchLinkCommand((Class6B5CC *)self, arg1, count);
}

void DreamSys__SetLastOffsetValue(DreamSys *self, s16 val) {
    self->lastOffsetValue = val;
}

void DreamSys__NoOpSlotE8(void) {
}

void DreamSys__SetPendingExtra(DreamSys *self, void *extra) {
    self->pendingExtra = extra;
}

/* The shared intermediate base-class table -- see include/DreamSys.h's
 * `DreamSysBaseMethods` comment and include/code_55dd4.h's own independent
 * view (`D800878D4Methods`) of the SAME table. This unit's own extern,
 * typed to match the return type `DreamSys__GetBaseMethods` already carried in
 * include/DreamSys.h (round 2026-09-04). */
extern DreamSysBaseMethods D_800878D4;

DreamSysBaseMethods *DreamSys__GetBaseMethods(void) {
    return &D_800878D4;
}

extern void *BMemPMgrAlloc(s32 size);

/* The class allocated below, table D_800879C4 (49 slots, uncarved --
 * lives in the still-monolithic asm/class_3bb8c_q.s, this unit's
 * immediate successor per class_3bb8c_o.c's own file banner). Only the
 * ctor slot (+0x008) is needed here, resolved via `tools/classtable.py
 * D_800879C4` to this unit's own `D800879C4__D800879C4` (see its own report).
 * `func_80057F58` is a plain no-argument getter for `&D_800879C4` --
 * confirmed by reading its own body directly in asm/class_3bb8c_q.s,
 * which is otherwise off limits (uncarved ground, not this unit's). Type
 * names drop the underscore after `D` per this project's convention for
 * an as-yet-unnamed class (see `include/code_55dd4.h`'s `D800878D4Methods`,
 * round 57 naming pass). */
typedef struct D800879C4Obj D800879C4Obj;
typedef struct D800879C4Methods {
    u8 pad00[0x8];
    D800879C4Obj *(*ctor)(D800879C4Obj *self, void *arg1, void *arg2, void *arg3);
    /* +0x00C..+0x03C not yet needed by this unit. */
    u8 pad0C[0x40 - 0xC];
    /* This class's OWN slot, resolved via `tools/classtable.py
     * D_800879C4`: `D800879C4__SetVariantClut`, the FIRST function of this unit's
     * successor `class_3bb8c_q` -- out of this unit/runner's range.
     * Tail-called by this unit's own `D800879C4__D800879C4` (its own ctor, see
     * that function's report) as (self, arg1) once construction is
     * otherwise complete (round 2026-09-04). Named `postConstruct` in the
     * round 57 naming pass; renamed after its occupant in round 79. */
    void *(*setVariantClut)(D800879C4Obj *self, s32 arg1);
} D800879C4Methods;
extern D800879C4Methods *func_80057F58(void);

/* This unit's own view of a D_800879C4 instance -- only the vtable
 * pointer (set by this unit's own ctor, `D800879C4__D800879C4`) and `+0xA4`
 * (also written by that ctor) are named; the rest is opaque. */
struct D800879C4Obj {
    D800879C4Methods *methods;
    u8 pad04[0xA4 - 0x4];
    s32 unk_0xA4;
};

void *New_D800879C4(void *arg1, void *arg2, void *arg3) {
    void *obj = BMemPMgrAlloc(0xA8);
    if (obj != NULL) {
        func_80057F58()->ctor(obj, arg1, arg2, arg3);
        return obj;
    }
    return NULL;
}

/* D800879C4's two texture cells, forwarded as the Sprite ctor's `rect`
 * (Sprite__Reset copies it into Sprite.rect): u,v = (0x00,0x20) and
 * (0x10,0x20), 16x16. */
extern SpriteRect D_80087A8C[2];

/* The base-class ctor, Sprite__Sprite, through GetSpriteMethods()
 * (include/Sprite.h); `self` is upcast to the Sprite it derives from. */

void *D800879C4__D800879C4(D800879C4Obj *self, s32 arg1, void *arg2, void *arg3) {
    GetSpriteMethods()->ctor((Sprite *)self, arg3, 0, &D_80087A8C[arg1], arg2, 0);
    self->methods = func_80057F58();
    self->unk_0xA4 = 0;
    return self->methods->setVariantClut(self, arg1);
}
