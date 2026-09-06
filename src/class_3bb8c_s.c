/*
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

typedef struct LinkNodeMethods {
    u8 pad0[0x44];
    void (*slot44)(LinkNode *self, s32 flag, s32 val);      /* +0x044 */
    void (*slot48)(LinkNode *self, s32 flag, void *arg);      /* +0x048 */
    void (*slot4C)(LinkNode *self, void *arg1, void *arg2);     /* +0x04C */
    u8 pad50[0x60 - 0x50];
    void (*slot60)(LinkNode *self, s32 arg1);                     /* +0x060 */
    void (*slot64)(LinkNode *self);                                 /* +0x064 */
    void (*slot68)(LinkNode *self, s32 arg1);                         /* +0x068 */
    u8 pad6C[0xB8 - 0x6C];
    void (*slotB8)(LinkNode *self, void *arg1);                          /* +0x0B8 */
} LinkNodeMethods;

struct LinkNode {
    LinkNodeMethods *methods; /* +0x000 */
    u8 pad4[0x20 - 0x4];         /* +0x004 .. +0x01F, unknown */
    s32 unk20;                     /* +0x020, forwarded to func_8001E770 */
    u8 pad24[0x54 - 0x24];           /* +0x024 .. +0x053, unknown */
    s32 unk54;                         /* +0x054, a dispatch "state" selector */
    u8 pad58[0x64 - 0x58];           /* +0x058 .. +0x063, unknown */
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

typedef struct Vec3S {
    s32 x, y, z;
} Vec3S;

extern void func_80056DF8(void *self);   /* class_3bb8c_o.c, LinkOwnerObj* */
extern void func_80056F28(void *self);   /* class_3bb8c_o.c, LinkOwnerObj* */

void func_80056B8C(LinkNode *self);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_s", func_80056520);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_s", func_80056640);

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

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_s", func_80056858);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_s", func_800569A8);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_s", func_80056B8C);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_s", func_80056BBC);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_s", func_80056D18);
