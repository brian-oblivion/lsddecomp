/*
 * ROUND 42 CORRECTION (2026-09-15) -- READ BEFORE ANY "BLOCKED" LINE BELOW:
 * every claim in this comment that a function is BLOCKED by `gp_rel`,
 * `nop_mflo_mfhi` or `addiu_at` is STALE.  All three constructs are RESOLVED
 * by pinned maspsx flags (CLAUDE.md, "Open toolchain blockers");
 * `tools/nearmiss.py` reports them tagged (RESOLVED-not-a-blocker) and counts
 * none of them.  Any "do NOT spend attempts on these" directive below is
 * therefore RETRACTED: those functions are ordinary matching work, and most
 * carry a mechanism-correct partial derivation already.  The rest of this
 * comment still stands -- only the blocker verdicts are withdrawn.
 * Screen: `python3 tools/nearmiss.py`, round 43 (2026-09-15).
 *
 * class_3bb8c_s -- functions 44..53 of the class_3bb8c remainder,
 * 0x46D20..0x475F0 (10 functions).  Carved round 21 (2026-09-06).
 *
 * Blocker census at carve time (four screens, canonical shell forms), fixed
 * up per the round 22 Gate 1 screen: `addiu_at` was RESOLVED in round 21 (see
 * docs/research/addiu-at-blocker.md) and is no longer a blocker at all -- the
 * 3 functions previously counted against it are workable, and their stub
 * reports were retired. Only `gp_rel` remains live in this unit, on
 * func_80056520, func_80056640 and func_80056D18 -- each still carries a
 * current stall report and none of the three is touched here.
 *
 * MATCHED this round: func_80056718, func_80056794, func_800567D4,
 * func_80056858, func_80056B8C -- byte-exact, see docs/match-reports/.
 *
 * func_80056BBC was attempted (the addiu_at hit in it is the now-resolved
 * construct, not a real blocker) and got to a one-instruction residue -- a
 * retail dead store (`li $a1, 1`) that never gets read on either branch it
 * precedes, the same "redundant move" class MATCHING-GUIDE.md already
 * documents as permuter territory. ~8000 permuter iterations did not find a
 * zero; restored to INCLUDE_ASM per the hard rule. See its match report.
 *
 * func_800569A8 is still untouched (its only screen hit is the same
 * now-resolved addiu_at construct; it remains fresh ground).
 *
 * This slice spans (at least) parts of the same class as the neighbouring
 * `class_3bb8c_o` slice: `self` here is the SAME kind of node as that unit's
 * `LinkOwnerObj` (a 5-element `arr84` link array is confirmed via
 * func_80056D18's occupancy of it), extended with more fields this unit
 * actually reads (+0x054 dispatch state, +0x064/+0x068/+0x06C/+0x070/+0x074/
 * +0x078 and a 2-element +0x07C array). Kept as this unit's OWN local view,
 * `LinkNode`/`LinkNodeMethods` -- `class_3bb8c_o.c`'s `LinkOwnerObj` is not
 * this unit's to edit, and per the multiple-independent-local-views
 * convention there is no reason a fresh view here should match its fields.
 * Confirmed with a straight read of this unit's own asm/nonmatchings .s files,
 * cross-checked against the (read-only, not edited) disassembly of the three
 * still-blocked functions in this same unit, which occupy the same self and
 * establish the +0x084 5-element array and the class_3bb8c_o.c call targets.
 */

#include "common.h"

/* ------------------------------------------------------------------ *
 * Shared node type for this unit. `self` in every function below (and the
 * three still gp_rel-blocked ones left as INCLUDE_ASM) is the same kind of
 * node: it owns a small fixed vtable (LinkNodeMethods) and is ALSO the
 * element type of its own two child arrays, `arr7C` (2 elements) and
 * `arr84` (5 elements, the same array class_3bb8c_o.c's `LinkOwnerObj`
 * already established under its own local name).
 * ------------------------------------------------------------------ */

typedef struct LinkNode LinkNode;

typedef struct Vec3S {
    s32 x, y, z;
} Vec3S;

typedef struct LinkNodeMethods {
    u8 pad0[0x44];
    void (*slot44)(LinkNode *self, s32 flag, s32 val);      /* +0x044 */
    void (*slot48)(LinkNode *self, s32 flag, void *arg);      /* +0x048 */
    void (*slot4C)(LinkNode *self, void *arg1, void *arg2);     /* +0x04C */
    u8 pad50[0x60 - 0x50];
    void (*slot60)(LinkNode *self, s32 arg1);                     /* +0x060 */
    void (*slot64)(LinkNode *self, s32 arg1);                       /* +0x064 */
    void (*slot68)(LinkNode *self, s32 arg1);                         /* +0x068 */
    u8 pad6C[0xB8 - 0x6C];
    void (*slotB8)(LinkNode *self, void *arg1);                          /* +0x0B8 */
    void (*slotBC)(LinkNode *self, void *arg1);                          /* +0x0BC, called on each
                                                                             arr7C child by
                                                                             func_800569A8 */
} LinkNodeMethods;

struct LinkNode {
    LinkNodeMethods *methods; /* +0x000 */
    u8 pad4[0x14 - 0x4];         /* +0x004 .. +0x013, unknown */
    s32 *unk14;                    /* +0x014, zeroed by func_800569A8 on
                                       every exit path */
    u8 pad18[0x20 - 0x18];           /* +0x018 .. +0x01F, unknown */
    s32 unk20;                     /* +0x020, forwarded to func_8001E770 */
    s32 unk24;                       /* +0x024, bounded < 0x1F5 and used as a
                                         modulus dividend by func_800569A8 */
    u8 pad28[0x54 - 0x28];              /* +0x028 .. +0x053, unknown */
    s32 unk54;                         /* +0x054, a dispatch "state" selector */
    Vec3S unk58;                     /* +0x058, added into the passed-in Vec3
                                         (arg1/arg2) by func_80056520/
                                         func_80056640 before dispatch */
    s32 unk64;                         /* +0x064 */
    void *unk68;                         /* +0x068, ptr to an object whose
                                             first field is a signed s16 */
    s32 unk6C;                              /* +0x06C, an index (0..4) into
                                                D_800877F8 */
    s32 unk70;                                 /* +0x070, an index into
                                                   D_8008780C */
    void *unk74;                                  /* +0x074 */
    void *unk78;                                     /* +0x078 */
    LinkNode *arr7C[2];                                 /* +0x07C..+0x083 */
    LinkNode *arr84[5];                                    /* +0x084..+0x097,
                                                                same array as
                                                                class_3bb8c_o.c's
                                                                LinkOwnerObj::arr84 */
};

extern void func_80056DF8(void *self);   /* class_3bb8c_o.c, LinkOwnerObj* */
extern void func_80056F28(void *self);   /* class_3bb8c_o.c, LinkOwnerObj* */
extern void func_800573A8(void *self, Vec3S *v);   /* class_3bb8c_t.c */
extern void func_80056D18(void *self, s32 a1, s32 a2, void *tbl); /* below */
extern s32 D_80087844[];
extern s32 D_8008785C[];
extern s32 D_80087868[];
extern s32 D_80087874[];
extern Vec3S D_80087880;

void func_80056B8C(LinkNode *self);
void func_80056858(LinkNode *self, s32 reuse);
void func_80056794(Vec3S *dst, Vec3S *a, Vec3S *b);
void func_800567D4(LinkNode *self, void *arg1, void *arg2, s32 arg3, void *arg4);
extern void func_8001E770(void *self, s32 arg); /* established, code_55dd4.h */

/* func_80056DF0/func_80056E1C/func_80056E44 are defined in class_3bb8c_o.c
 * (own their addresses, own local view "LinkOwnerObj"/"LinkElemObj") with
 * signatures of 0 or 1 pointer argument. Every call site in THIS unit still
 * sets up a second argument register that those bodies never read (retail's
 * own caller-side view evidently did not carry a narrower prototype either)
 * -- so these are declared here the same old-style (unprototyped) way,
 * which is what lets this file's calls pass the extra dead argument without
 * a parameter-count mismatch against their real, narrower definitions
 * elsewhere. func_80056BBC is this same idiom but for a function defined
 * later IN THIS FILE (func_80056520 calls it, ROM-earlier than its own
 * definition). func_800569A8 is likewise defined later in this file, and
 * func_80056640 forwards a dead second argument to it the same way. */
extern void func_80056DF0();
extern void func_80056E1C();
extern void func_80056E44();
extern void func_80056BBC();
extern void func_800569A8();

/* class_3bb8c_p.c; fully prototyped since every call site here uses all
 * three arguments for real. */
extern void *func_80057C94(void *arg1, void *arg2, void *arg3);

/* Three globals a class_3bb8c_o.c ctor-shaped function (func_80056F5C)
 * captures once from its own three pointer-typed parameters -- D_8008ACA4 is
 * some other object (first field a methods pointer, called through a new
 * +0x080 slot below), D_8008ACA8 is forwarded opaquely to func_80057C94 as
 * its own third argument, and D_8008ACAC's pointee has a lookup field at
 * +0x018 that func_80056520/func_80056640 snapshot/diff via D_8008ACB0. */
typedef struct {
    u8 pad0[0x80];
    s32 (*slot80)(void *self, s32 arg);
} D_8008ACA4Methods;
typedef struct {
    D_8008ACA4Methods *methods;
} D_8008ACA4Obj;
extern D_8008ACA4Obj *D_8008ACA4;
extern void *D_8008ACA8;
extern void *D_8008ACAC;
extern s32 D_8008ACB0;
extern s32 D_8008AB98[];

void func_80056520(LinkNode *self, void *arg1, Vec3S *arg2) {
    Vec3S local;
    s32 state;

    D_8008ACB0 = *(s32 *)((u8 *)D_8008ACAC + 0x18);
    func_80056794(&local, arg2, &self->unk58);
    func_800567D4(self, arg1, &local, self->unk64, self->unk68);

    state = self->unk54;
    if (state < 2) {
        s32 ret = D_8008ACA4->methods->slot80(D_8008ACA4, D_8008AB98[state]);
        func_8001E770(self, ret);
        state = self->unk54;
    }

    switch (state) {
    case 0:
        func_80056858(self, 0);
        break;
    case 2:
        func_80056BBC(self, 0);
        break;
    case 3:
        func_80056E1C(self, 0);
        break;
    default:
        break;
    }
}

void func_80056640(LinkNode *self, void *arg1) {
    Vec3S local;

    func_80056794(&local, (Vec3S *)arg1, &self->unk58);
    local.y += *(s32 *)((u8 *)D_8008ACAC + 0x18) - D_8008ACB0;
    self->methods->slotB8(self, &local);

    switch (self->unk54) {
    case 0:
        func_800569A8(self, arg1);
        break;
    case 2:
        func_80056DF0(self, arg1);
        break;
    case 3:
        func_80056E44(self, arg1);
        break;
    default:
        break;
    }
}

/* func_80056718 -- dispatch on self->unk54, one of three such handlers in
 * this slice (func_80056520/func_80056640 are the other two, each mapping
 * the same state values to a DIFFERENT set of callees -- consistent with
 * three separate per-phase handlers, e.g. update/draw/free, sharing one
 * state field). */
void func_80056718(LinkNode *self) {
    switch (self->unk54) {
    case 0:
        func_80056B8C(self);
        break;
    case 2:
        func_80056DF8(self);
        break;
    case 3:
        func_80056F28(self);
        break;
    default:
        break;
    }
}

/* Plain Vec3 add: dst = a + b. Frameless -- no self/vtable involved. */
void func_80056794(Vec3S *dst, Vec3S *a, Vec3S *b) {
    dst->x = a->x + b->x;
    dst->y = a->y + b->y;
    dst->z = a->z + b->z;
}

/* Forwards straight through to a child's own slot4C/slot44/slot48, using
 * whatever the caller already set up in arg1/arg2 (an outer node pointer and
 * an accumulator Vec3, respectively -- see func_80056858's own two call
 * sites) plus its own arg3/arg4. */
void func_800567D4(LinkNode *self, void *arg1, void *arg2, s32 arg3, void *arg4) {
    self->methods->slot4C(self, arg1, arg2);
    self->methods->slot44(self, 1, arg3);
    self->methods->slot48(self, 1, arg4);
}

/* A rand()-free Vec3 accumulate/attach helper: for each of self's two
 * arr7C slots, fold a table-driven contribution into a local Vec3 (either
 * into .x, scaled by *self->unk68, or straight into .y, depending on
 * self->unk6C), then either forward it to an existing child's slotB8 or
 * spin up a brand new child via func_80056FE4/func_8001E770/func_800567D4. */
extern void *func_80056FE4(void);        /* class_3bb8c_o.c, New_X allocator */
extern void func_8001E770(void *self, s32 arg); /* established, code_55dd4.h */
extern Vec3S D_800877EC;
extern s32 D_800877F8[];

void func_80056858(LinkNode *self, s32 reuse) {
    Vec3S accum;
    LinkNode **p;
    s32 i;
    s32 count = self->unk6C;

    if (count == 0) {
        return;
    }
    accum = D_800877EC;
    p = self->arr7C;
    for (i = 0; i < 2; i++, p++) {
        if (count < 3) {
            accum.x += *(s16 *)self->unk68 * D_800877F8[count];
        } else {
            accum.y += D_800877F8[count];
        }
        if (reuse) {
            LinkNode *child = *p;
            child->methods->slotB8(child, &accum);
        } else {
            LinkNode *child = func_80056FE4();
            *p = child;
            func_8001E770(child, self->unk20);
            func_800567D4(*p, self, &accum, self->unk64, self->unk68);
        }
    }
}

/* Per-`unk70`-slot table (same index space `func_80056BBC` reads through
 * D_80087844/D_8008785C/D_80087868/D_80087874); used both as a "channel
 * active" guard (nonzero test) and as a divisor for the two modulus checks
 * below. */
extern s32 D_8008780C[];
/* A Vec3S "base offset" constant, added (via its .z only) to a per-child
 * accumulator before being forwarded to each arr7C child's slotB8. */
extern Vec3S D_8008782C;
/* Opaque table handle, forwarded unchanged to slot44 for self and for each
 * arr7C child -- passed as a plain s32 per the slot's established
 * signature (see func_800567D4's use of the same slot with an s32
 * argument), not dereferenced anywhere in this function. */
extern s32 D_80087838[];

#if 0
/* round 44 (2026-09-15): best-reached body, 117/121 words, NOT byte-exact.
 * See docs/match-reports/func_800569A8.md for the residue and what was
 * tried. Kept here per the hard rule -- restore this ahead of any future
 * attempt rather than re-deriving from scratch. */
void func_800569A8(LinkNode *self)
{
    s32 idx;
    s32 *tab70;
    s32 *tab70b;
    s32 accumOffset;
    s32 divq;
    s32 modend;
    LinkNode **p;
    s32 i;

    idx = self->unk70;
    if (self->unk6C != 0) {
        tab70 = &D_8008780C[idx];
        if (*tab70 != 0 && (u32) self->unk24 >= 0x1F5) {
            p = self->arr7C;
            self->methods->slot44(self, 0, (s32) D_80087838);

            i = 0;
            tab70b = tab70;
            accumOffset = 0;
            for (; i < 2; i++) {
                Vec3S local = D_8008782C;
                local.z += accumOffset + *tab70b;
                (*p)->methods->slotBC(*p, &local);
                accumOffset += 3;
                (*p)->methods->slot44(*p, 0, (s32) D_80087838);
                p++;
            }

            divq = 24500 / D_8008780C[idx];
            modend = self->unk24;
            if (divq >= 0) {
                if ((u32) modend % (u32) divq == 0) {
                    func_80056858(self, 1);
                }
            } else {
                u32 adivq = ~divq + 1;
                if ((u32) modend % adivq == 0) {
                    func_80056858(self, 1);
                }
            }
        }
    }
    *self->unk14 = 0;
}
#endif

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_s", func_800569A8);

extern void func_800183DC(void **array, s32 count);

/* self->unk6C-guarded release of the fixed 2-element arr7C array. */
void func_80056B8C(LinkNode *self) {
    if (self->unk6C != 0) {
        func_800183DC((void **)self->arr7C, 2);
    }
}

void func_80056BBC(LinkNode *self) {
    s32 parity = rand() % 2;
    void *tblOrNull = parity ? NULL : D_80087868;
    LinkNode *child;
    void *arg;

    func_80056D18(self, 0, 0, tblOrNull);

    if (self->unk70 >= 2) {
        LinkNodeMethods *m;

        child = self->arr84[1];
        D_80087880.x = D_80087844[self->unk70];
        func_800573A8(child, &D_80087880);
        m = child->methods;
        arg = (self->unk78 != NULL) ? self->unk78 : self->unk74;
        m->slotB8(child, arg);
    } else {
        child = self->arr84[1];
        child->methods->slot64(child, 1);
        child->methods->slot68(child, 0);
        child->methods->slot48(child, 1, (parity != 0) ? D_8008785C : D_80087874);
    }

    self->arr84[2]->methods->slot60(self->arr84[2], 0);
}

void func_80056D18(void *self, s32 a1, s32 a2, void *tbl) {
    LinkNode **p = (LinkNode **)((u8 *)self + 0x84);
    LinkNode *node;
    s32 i;

    for (i = 0; i < 5; i++, p++) {
        node = func_80057C94((void *)a2, 0, D_8008ACA8);
        *p = node;
        node->methods->slot4C(node, self, 0);
        (*p)->methods->slotB8(*p, ((LinkNode *)self)->unk74);
        if (tbl != 0) {
            (*p)->methods->slot48(*p, 1, tbl);
        }
    }
}
