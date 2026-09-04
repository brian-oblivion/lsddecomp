/*
 * code_179d8_e -- functions 120..148 of the original 274-function code_179d8
 * monolith, 0x1CC08..0x1D508 (vram 0x8002C408..0x8002CD08).  Carved round 17
 * (2026-09-04) out of what the yaml called `code_179d8_mid_b`; the remainder
 * behind it is now `code_179d8_mid_c`.
 *
 * Blocker census, three-grep screen run per function at carve time:
 * 21 of the 29 clean.  READ THAT WITH CARE -- ten of the 29 are two-word
 * `jr $ra; nop` leaves and splat matched eight of them itself, so the real
 * queue is the 21 INCLUDE_ASMs below, of which eight are blocked.
 *
 * BLOCKED, stub reports already filed, do NOT spend attempts on these:
 *   gp_rel: func_8002C448, func_8002C468, func_8002C4E0, func_8002C638,
 *           func_8002C6FC, func_8002C890, func_8002CC1C, func_8002CC28
 * That is a cluster, not a scatter: these functions are the accessors for
 * one band of small-data globals (D_8008A8B0..D_8008A8CC), which is exactly
 * the shape the gp-relative blocker takes.  The bodies AROUND them that do
 * not touch that band are clean.
 *
 * Owns NO switch jump table -- zero `jtbl_` references anywhere in the slice
 * -- so no rodata sub-slot is attached to this unit.
 *
 * CORRECTION (runner echo, round 17): the assignment inherited a blanket "not
 * class-framework code" note from code_179d8_b/_d's sibling findings, but
 * that finding does NOT hold for THIS slice.  `tools/classtable.py --scan`
 * hits both D_8006D9BC (29 slots, header 0x00000023) and D_8006DA34 (39
 * slots, header 0x00000A03) as real class tables.  func_8002C824's and
 * func_8002CC84's self objects load `*self` (offset 0, the methods pointer)
 * and dereference table slots at exactly the offsets `classtable.py D_8006DA34`
 * prints for func_8002C890 (+0x7C) and func_8002CB18 (+0x84) -- direct
 * confirmation this is genuine self->methods->slotN(self, ...) dispatch, not
 * driver code.  D_8006D9BC and D_8006DA34 share the same BasicClass tail
 * (slots +0x10..+0x38), so they are two related classes off the same base --
 * plausibly a PS1 SPU/VAB sound-streaming object (func_8002C4E0, blocked,
 * calls SsUtGetVabHdr on `self`).  Do expect a `this` pointer in this slice;
 * the sibling slices' finding is neighbourhood-scoped, not monolith-wide.
 */
#include "common.h"

/* ------------------------------------------------------------------------
 * SoundObj class framework (D_8006D9BC / D_8006DA34).  Only the slots and
 * fields THIS unit's functions actually touch are named; everything else is
 * opaque padding, per this project's local-reading convention (see
 * code_179d8_d.c's Table6D940/Obj6D940 for the same idiom applied to a
 * neighbouring class).  Kept LOCAL to this file -- no shared header, so a
 * sibling slice's independent reading of the same tables cannot collide with
 * this one on merge.
 * ------------------------------------------------------------------------ */
typedef struct ObjDA34 ObjDA34;

/* D_8006D9BC's own methods table.  Only +0x054 is a slot this unit defines. */
typedef struct TableD9BC {
    u8 pad000[0x054];
    s32 (*slot54)(void); /* func_8002C408 */
    u8 pad058[0x078 - 0x058];
} TableD9BC;
extern TableD9BC D_8006D9BC;

/* D_8006DA34's own methods table -- the class ObjDA34 below dispatches
 * through.  Only the slots this unit's own functions call or are assigned to
 * are named. */
typedef struct TableDA34 {
    u8 pad000[0x008];
    void (*slot08)(void *self, s32 arg1); /* func_8002C4E0, BLOCKED (gp_rel) -- this unit's own new_class_da34 dispatch */
    u8 pad00C[0x078 - 0x00C];
    s32 (*slot78)(ObjDA34 *self, s32 arg1); /* func_8002C824 */
    s32 (*slot7C)(ObjDA34 *self);           /* func_8002C890, BLOCKED (gp_rel) */
    s32 (*slot84)(ObjDA34 *self, s32 arg1); /* func_8002CB18, arg1 is s16-truncated by the callee */
    u8 pad088[0x09C - 0x088];
} TableDA34;
extern TableDA34 D_8006DA34;

/* One "chunk" entry inside a self->unk50 sub-array, stride 0x20.  Only the
 * two bytes func_8002CA3C itself reads are named. */
typedef struct Chunk179D8E {
    u8 pad0[0x4];
    u8 unk4;
    u8 unk5;
    u8 pad6[0x20 - 0x6];
} Chunk179D8E;

/* self for the D_8006DA34-dispatched methods in this unit.  Only fields this
 * unit's own functions touch are named -- see the header-comment correction
 * above for how this was established. */
struct ObjDA34 {
    TableDA34 *methods;    /* +0x000 */
    u8 pad004[0x050 - 0x004];
    Chunk179D8E **unk50;   /* +0x050, array of per-`hi` pointers into unk4C's pool */
    s16 unk54;             /* +0x054 */
    s16 unk56;              /* +0x056, boolean-ish flag */
    s16 unk58;              /* +0x058 */
    u16 unk5A;               /* +0x05A */
    u8 pad05C[0x060 - 0x05C];
    s32 unk60;               /* +0x060 */
};

/* Cross-unit calls into the still-uncarved code_179d8_tail monolith --
 * declared LOCAL to this unit, per-call-site typed, since none of them have
 * an established prototype anywhere yet. */
extern void *func_80017B34(s32 size);
extern s16 func_80030E90(s16 a0, s16 hi, s16 lo, s16 a3, s32 b5, s32 argA, s32 argB);
extern void func_80031E94(s16 a0, s16 a1, s16 a2, s32 a3);
extern void func_80031890(s16 index);
extern void func_80031F3C(s32 arg0);
extern void func_8003370C(s32 arg0);
extern s32 func_800336CC(s32 arg0);

/* func_8002CC34's own "obj" (its `arg1`) -- a small slot-table object,
 * unrelated to ObjDA34 (this function is NOT a D_8006DA34 vtable slot; its
 * only callers pass a plain heap/stack struct pointer).  Only the fields
 * this unit's own function touches are named. */
typedef struct Slot179D8ECC34 {
    s32 unk0; /* -1 = free/sentinel after init */
    u8 pad4[0x14 - 0x4];
} Slot179D8ECC34;

typedef struct ObjCC34 {
    s32 unk0;   /* +0x00, guard: 0 = uninitialized */
    s32 unk4;   /* +0x04 */
    void *unk8; /* +0x08 */
    s32 unkC;   /* +0x0C */
    u8 pad10[0x14 - 0x10];
    s32 unk14;  /* +0x14 */
    Slot179D8ECC34 arr[3]; /* +0x18 */
} ObjCC34;

/* Forward declaration: func_8002CC0C is defined later in this file (ROM
 * order), but func_8002C480 (earlier in ROM order) calls it. */
TableDA34 *func_8002CC0C(void);

s32 func_8002C408(void) {
    return 0;
}

void func_8002C410(void) {
}

void func_8002C418(void) {
}

void func_8002C420(void) {
}

void func_8002C428(void) {
}

void func_8002C430(void) {
}

TableD9BC *func_8002C438(void) {
    return &D_8006D9BC;
}

INCLUDE_ASM("asm/nonmatchings/code_179d8_e", func_8002C448);

INCLUDE_ASM("asm/nonmatchings/code_179d8_e", func_8002C468);

s32 func_8002C478(void) {
    return 0;
}

void *func_8002C480(s32 arg0) {
    void *self;

    self = func_80017B34(0x64);
    if (self != NULL) {
        func_8002CC0C()->slot08(self, arg0);
        return self;
    }
    return NULL;
}

INCLUDE_ASM("asm/nonmatchings/code_179d8_e", func_8002C4E0);

INCLUDE_ASM("asm/nonmatchings/code_179d8_e", func_8002C638);

INCLUDE_ASM("asm/nonmatchings/code_179d8_e", func_8002C6FC);

s32 func_8002C824(ObjDA34 *self, s32 arg1) {
    s32 result;

    result = 0;
    if (self->unk5A != 0) {
        if (arg1 != 0) {
            func_8003370C(1);
            self->unk5A = 0;
            self->unk58 = 1;
            self->methods->slot7C(self);
            result = 1;
        }
    }
    return result;
}

INCLUDE_ASM("asm/nonmatchings/code_179d8_e", func_8002C890);

INCLUDE_ASM("asm/nonmatchings/code_179d8_e", func_8002CA3C);

s32 func_8002CB18(ObjDA34 *self, s32 index) {
    if (index < 0x18) {
        func_80031890(index);
    } else {
        func_80031F3C(0);
    }
    return -1;
}

s32 func_8002CB58(ObjDA34 *self) {
    s32 flag;

    flag = self->unk56;
    if (flag == 0) {
        func_800336CC(1);
        flag = 1;
        self->unk56 = flag;
    }
    return flag;
}

s32 func_8002CB9C(ObjDA34 *self) {
    s32 flag;

    flag = self->unk56;
    if (flag != 0) {
        flag = func_800336CC(0);
        self->unk56 = 0;
    }
    return flag;
}

void func_8002CBDC(void) {
}

void func_8002CBE4(void) {
}

void func_8002CBEC(void) {
}

void func_8002CBF4(ObjDA34 *self, s32 arg1) {
    self->unk60 = arg1 * 12 - 0x18;
}

TableDA34 *func_8002CC0C(void) {
    return &D_8006DA34;
}

INCLUDE_ASM("asm/nonmatchings/code_179d8_e", func_8002CC1C);

INCLUDE_ASM("asm/nonmatchings/code_179d8_e", func_8002CC28);

s32 func_8002CC34(void *unused, ObjCC34 *obj, s32 arg2, void *arg3, s32 arg4) {
    Slot179D8ECC34 *slot;
    s32 count;
    s32 sentinel;

    if (obj->unk0 != 0) {
        return 0;
    }
    slot = obj->arr;
    sentinel = -1;
    count = 2;
    obj->unk0 = arg2;
    obj->unk8 = arg3;
    obj->unkC = arg4;
    do {
        slot->unk0 = sentinel;
        count--;
        slot++;
    } while (count >= 0);
    obj->unk4 = 0;
    obj->unk14 = 10;
    return 1;
}

INCLUDE_ASM("asm/nonmatchings/code_179d8_e", func_8002CC84);
