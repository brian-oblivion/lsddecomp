/*
 * class_3bb8c_p -- functions 74..93 of the 113-function `class_3bb8c_n`
 * remainder, 0x47CC4..0x485BC (vram 0x800574C4..0x80057DBC).  Carved round 17
 * (2026-09-04), immediately behind `class_3bb8c_o`.
 *
 * Blocker census, three-grep screen run per function at carve time:
 * 20 of 20 clean -- no gp_rel, no addiu-$at, no nop_mflo_mfhi anywhere in the
 * slice.  This is the cleanest window found in the whole executable this
 * round.  Four of the 20 are two-word leaves; splat matched two of them
 * itself, so the queue below is 18.
 *
 * Owns NO switch jump table -- zero `jtbl_` references in the slice -- so no
 * rodata sub-slot is attached to this unit.
 *
 * EXPECT THIS SLICE TO SPAN MORE THAN ONE CLASS.  It is cut at ROM addresses,
 * not at class boundaries, and round 15 measured three of five such slices
 * spanning two or more vtables.  Identify each class with tools/classtable.py
 * rather than assuming the unit has one.  A class that spans a carve boundary
 * is also the normal reason two units name the same table -- see the
 * multiple-independent-local-views convention in CLAUDE.md before deciding
 * whether your view of one belongs in include/class_3bb8c.h or here.
 */
#include "common.h"
#include "DreamSys.h"

/* Two-element s16 array -- func_800574C4 and func_800574FC each write one
 * element (index 0 and 1 respectively) via a plain `sh` through a pointer
 * computed as %hi/%lo of `D_8008ABA4 + 2*index`, so splat's single-word
 * dlabel is really this 2-element array, not a lone s32 (round 2026-09-04).
 * Not referenced anywhere else in the repo (checked with grep), so this is
 * this unit's own reading -- kept local rather than added to a shared
 * header. */
extern s16 D_8008ABA4[2];

void func_80057534(DreamSys *self, s16 *slot, s32 val, void *extra, volatile s32 count);

void func_800574C4(DreamSys *self, s32 val, void *extra) {
    func_80057534(self, &D_8008ABA4[0], val, extra, 7);
}

void func_800574FC(DreamSys *self, s32 val, void *extra) {
    func_80057534(self, &D_8008ABA4[1], val, extra, 8);
}

/* `count` is `volatile` so it stays a stack reference reloaded at its one use
 * site, rather than being promoted to a callee-saved register across the
 * intervening func_80057444 call -- confirmed with a standalone reproducer
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
void func_80057534(DreamSys *self, s16 *slot, s32 val, void *extra, volatile s32 count) {
    s16 val16 = (s16) val;
    *slot = val16;
    self->field_0x48 = val16;
    self->vt->func_80057444(self, &D_8008ABA4[0]);
    *slot = 0;
    if (extra != NULL) {
        self->vt->func_80058B08(self, count);
    }
}

void func_80057618(DreamSys *self, void (*callback)(DreamSys *, s32, void *), s32 val, void *extra);

void func_800575B0(DreamSys *self, s32 val, void *extra) {
    func_80057618(self, self->vt->func_8005748C, val, extra);
}

void func_800575E0(DreamSys *self, s32 val, void *extra) {
    func_80057618(self, self->vt->func_800574C4, val, extra);
}

void func_80057610(void) {
}

s32 func_80057668(DreamSys *self);

void func_80057618(DreamSys *self, void (*callback)(DreamSys *, s32, void *), s32 val, void *extra) {
    self->unk_0x28 = NULL;
    callback(self, val, extra);
    if (self->unk_0x28 == NULL) {
        func_80057668(self);
    }
}

/* A 12-byte {s16,s16,s32,s32} query/result record. Built by this unit's
 * own func_80057784 (still queued) into caller-supplied buffers, and
 * walked as an array (stride 0xC) by func_80057954. Field meaning beyond
 * shape unconfirmed; kept opaque. */
typedef struct GridQuery {
    s16 unk0;
    s16 unk2;
    s32 unk4;
    s32 unk8;
} GridQuery;

/* A grid-bucket linked-list node: singly-linked chain at +0x38, the same
 * "self-typed next pointer" idiom already established for `EntryChildObj`
 * in include/class_3bb8c.h (an UNRELATED class, per this round's own
 * research -- convergent shape, not a shared type). Walked by
 * func_80057A18; matched against with func_80057B54 (already matched,
 * this unit, below). */
typedef struct GridElem {
    u8 pad00[0x38];
    struct GridElem *next;
} GridElem;

/* self->unk_0x4C's pointee dereferences to one of these via its own
 * `+0x4` field (a s16 flag at +0x2C, read by func_80057954, and a second
 * s16 at +0x32, read by func_80057784) and, when treated as
 * func_80057A18's 5th argument, a grid-array base pointer at `+0x10`.
 * Multiple independent call sites agree on this shape; kept opaque
 * beyond the fields actually read. */
typedef struct GridArrElemInner {
    u8 pad00[0x2C];
    s16 unk2C;
    u8 pad2E[0x32 - 0x2E];
    s16 unk32;
} GridArrElemInner;
typedef struct GridArrElem {
    u8 pad00[0x4];
    GridArrElemInner *unk4;
    u8 pad08[0x10 - 0x8];
    GridElem **unk10;
} GridArrElem;

/* Output buffer filled in by DreamSysUnk4CMethods::slot0x110 (see
 * include/DreamSys.h) and read back by this unit's own func_80057784.
 * Only the three fields actually touched are named. */
typedef struct LinkQueryBuf {
    u8 pad00[0x2];
    s8 unk2;
    s8 unk3;
    u8 pad04[0x24 - 0x4];
    GridArrElem *unk24;
    u8 pad28[0x30 - 0x28];
} LinkQueryBuf;

void *func_80057B54(void *arg0, void *arg1, void *arg2);
s32 func_80057784(DreamSys *self, GridQuery *arr1, GridArrElem **arr2, LinkQueryBuf *arg3, s32 arg4);
void *func_80057954(DreamSys *self, void *arg1, void *arg2, s32 count, GridQuery *arr1, GridArrElem **arr2);

s32 func_80057668(DreamSys *self) {
    LinkQueryBuf sp18;
    GridQuery sp48[3];
    /* No known field needs this gap; empirically required to reproduce
     * retail's exact stack layout for sp78/sp88 below (round 2026-09-04,
     * see this function's match report). */
    u8 pad48Tail[8];
    GridArrElem *sp78[3];
    DreamSysVec3 sp88;

    if (self->unk_0x4C != NULL) {
        void *pos = (u8 *) self->unk_0x14 + 0x18;

        if (self->unk_0x4C->methods->slot0x110(self->unk_0x4C, &sp18, pos) == 0) {
            s32 count = func_80057784(self, sp48, sp78, &sp18, 1);
            void *result = func_80057954(self, &sp88, pos, count, sp48, sp78);

            self->unk_0x28 = result;
            if (result != NULL) {
                self->vt->func_800573A8(self, &sp88);
                self->vt->func_80058B08(self, -1);
                return 1;
            }
            self->vt->func_80058B08(self, -2);
            return 0;
        }
    }
    return 0;
}

/* STALL -- see docs/match-reports/func_80057784.md. Best reached: 13/116
 * words, preserved there in #if 0 with full declarations. */
INCLUDE_ASM("asm/nonmatchings/class_3bb8c_p", func_80057784);

void *func_80057A18(DreamSys *self, void *arg1, void *arg2, GridQuery *query, GridArrElem *source);

/* Walks `count` entries of `arr1` (a `GridQuery[]`, stride 0xC) paired
 * element-for-element with `arr2` (a `GridArrElem *[]`, stride 4),
 * skipping any entry whose `GridArrElem` doesn't have its `+0x2C` flag
 * set, and calling `func_80057A18` on the rest; returns the first
 * non-NULL result, or NULL if every entry was skipped or came back empty
 * (round 2026-09-04). */
void *func_80057954(DreamSys *self, void *arg1, void *arg2, s32 count, GridQuery *arr1, GridArrElem **arr2) {
    s32 i;

    for (i = 0; i < count;) {
        GridArrElem *elem = *arr2;
        i++;
        if (elem->unk4->unk2C != 0) {
            void *result = func_80057A18(self, arg1, arg2, arr1, elem);
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
 * at `source->unk_0x10`, `query->unk8` rows by `query->unk4` columns,
 * starting at row `query->unk2`, column `query->unk0` (each row is 0x50
 * bytes = 20 bucket-head pointers; each column step is one bucket-head
 * pointer, 4 bytes). For each bucket, tries `func_80057B54` against the
 * head first, then each linked element in turn (`->next`), returning the
 * first one `func_80057B54` accepts (non-NULL); NULL if the whole window
 * comes up empty. `self` (this function's own first argument) is read
 * from `a0` in the disassembly but never touched by the body -- present
 * only to match its caller's calling convention (round 2026-09-04). */
void *func_80057A18(DreamSys *self, void *arg1, void *arg2, GridQuery *query, GridArrElem *source) {
    s32 row, col;
    GridElem **bucket;

    bucket = (GridElem **) ((u8 *) source->unk10 + query->unk2 * 0x50 + query->unk0 * 4);
    for (row = 0; row < query->unk8; row++) {
        for (col = 0; col < query->unk4; col++) {
            GridElem *node;

            if (func_80057B54(*bucket, arg1, arg2) != NULL) {
                return *bucket;
            }
            for (node = (*bucket)->next; node != NULL; node = node->next) {
                if (func_80057B54(node, arg1, arg2) != NULL) {
                    return node;
                }
            }
            bucket++;
        }
        bucket = (GridElem **) ((u8 *) bucket - (query->unk4 * 4 + 0x50));
    }
    return NULL;
}

extern s32 func_8001E7BC(void);

void *func_80057B54(void *arg0, void *arg1, void *arg2) {
    if (arg0 != NULL) {
        if (func_8001E7BC() != 0) {
            return arg0;
        }
    }
    return NULL;
}

/* This unit's own local view of func_8001E57C()'s return, matching only
 * the one slot this unit's own functions dispatch through directly (the
 * OTHER slot these functions use, +0xA0, is reached through the object's
 * OWN vtable instead -- see include/DreamSys.h's `slotA0`) -- per the
 * project's established "per-call-site signature" precedent
 * (include/code_d294.h's own file banner; that header's `Class6B5CCMethods`
 * types this SAME slot with a different argument count for ITS OWN call
 * sites, which is fine because the actual callee ignores unused trailing
 * register arguments). Kept local rather than added to code_d294.h. */
typedef struct DreamSysBasicSlots {
    u8 pad00[0x9C];
    void (*slot9C)(DreamSys *self, void *arg1, s32 count);
} DreamSysBasicSlots;
extern DreamSysBasicSlots *func_8001E57C(void);

void func_80057B90(DreamSys *self, void *arg1, s32 count) {
    func_8001E57C()->slot9C(self, arg1, count);
    if (count < 9) {
        if (count >= 5) {
            self->vt->slotA0(self, arg1, count);
        }
    }
}

void func_80057C14(DreamSys *self, void *arg1, s32 count) {
    func_8001E57C()->slot9C(self, arg1, count);
}

void func_80057C6C(DreamSys *self, s16 val) {
    self->field_0x48 = val;
}

void func_80057C74(void) {
}

void func_80057C7C(DreamSys *self, void *extra) {
    self->unk_0x54 = extra;
}

/* The shared intermediate base-class table -- see include/DreamSys.h's
 * `DreamSysBaseMethods` comment and include/code_55dd4.h's own independent
 * view (`D800878D4Methods`) of the SAME table. This unit's own extern,
 * typed to match the return type `func_80057C84` already carried in
 * include/DreamSys.h (round 2026-09-04). */
extern DreamSysBaseMethods D_800878D4;

DreamSysBaseMethods *func_80057C84(void) {
    return &D_800878D4;
}

extern void *func_80017B34(s32 size);

/* The class allocated below, table D_800879C4 (49 slots, uncarved --
 * lives in the still-monolithic asm/class_3bb8c_q.s, this unit's
 * immediate successor per class_3bb8c_o.c's own file banner). Only the
 * ctor slot (+0x008) is needed here, resolved via `tools/classtable.py
 * D_800879C4` to this unit's own `func_80057D10` (see its own report).
 * `func_80057F58` is a plain no-argument getter for `&D_800879C4` --
 * confirmed by reading its own body directly in asm/class_3bb8c_q.s,
 * which is otherwise off limits (uncarved ground, not this unit's). */
typedef struct D_800879C4Obj D_800879C4Obj;
typedef struct D_800879C4Methods {
    u8 pad00[0x8];
    D_800879C4Obj *(*ctor)(D_800879C4Obj *self, void *arg1, void *arg2, void *arg3);
    /* +0x00C..+0x03C not yet needed by this unit. */
    u8 pad0C[0x40 - 0xC];
    /* This class's OWN slot, resolved via `tools/classtable.py
     * D_800879C4`: `func_80057DBC`, the FIRST function of this unit's
     * successor `class_3bb8c_q` -- out of this unit/runner's range.
     * Tail-called by this unit's own `func_80057D10` (its own ctor, see
     * that function's report) as (self, arg1) once construction is
     * otherwise complete (round 2026-09-04). */
    void *(*slot40)(D_800879C4Obj *self, s32 arg1);
} D_800879C4Methods;
extern D_800879C4Methods *func_80057F58(void);

/* This unit's own view of a D_800879C4 instance -- only the vtable
 * pointer (set by this unit's own ctor, `func_80057D10`) and `+0xA4`
 * (also written by that ctor) are named; the rest is opaque. */
struct D_800879C4Obj {
    D_800879C4Methods *methods;
    u8 pad04[0xA4 - 0x4];
    s32 unk_0xA4;
};

void *func_80057C94(void *arg1, void *arg2, void *arg3) {
    void *obj = func_80017B34(0xA8);
    if (obj != NULL) {
        func_80057F58()->ctor(obj, arg1, arg2, arg3);
        return obj;
    }
    return NULL;
}

/* 2-element, 0xC-byte-stride table -- address-of only here (never
 * dereferenced by this function), forwarded as the ctor call's `arg3`.
 * Field layout unconfirmed beyond the stride; kept opaque. */
typedef struct D_80087A8CEntry {
    u8 unk[0xC];
} D_80087A8CEntry;
extern D_80087A8CEntry D_80087A8C[2];

/* Another uncarved-ground getter (asm/psyq_memset.s, of all places --
 * splat's segmentation, not a meaningful grouping): plain no-argument,
 * `return &D_8006EE1C;`. Only the ctor slot is needed; the callee ignores
 * this call's "arguments" (this function's OWN a0..a3, left untouched in
 * registers from entry -- the "per-call-site signature" precedent again,
 * see func_80057B54's report), so the C call site takes none either. */
typedef struct D_8006EE1CMethods {
    u8 pad00[0x8];
    void *(*ctor)(void *self, void *arg1, s32 arg2, void *arg3, void *arg4, s32 arg5);
} D_8006EE1CMethods;
extern D_8006EE1CMethods *func_800422BC(void);

void *func_80057D10(D_800879C4Obj *self, s32 arg1, void *arg2, void *arg3) {
    func_800422BC()->ctor(self, arg3, 0, &D_80087A8C[arg1], arg2, 0);
    self->methods = func_80057F58();
    self->unk_0xA4 = 0;
    return self->methods->slot40(self, arg1);
}
