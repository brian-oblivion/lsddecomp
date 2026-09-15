/*
 * class_3bb8c_n -- functions 0..22 of the old 113-function class_3bb8c
 * remainder, 0x44F14..0x46288.  23 functions, 1245 words.
 * Carved round 45 (2026-09-15) by the head.
 *
 * STAFFED IN ROUND 46.  Carved round 45 to BANK ground, carrying the banked
 * marker `progress.py` keys on; the round-46 head removed it when staffing a
 * runner here, as that header instructed.  If this unit is ever un-staffed
 * with functions left over, put the marker back -- `banked` and `fresh` are
 * different claims and only the marker distinguishes them.
 *
 * DO NOT WRITE THAT MARKER PHRASE OUT IN FULL ANYWHERE IN THIS FILE, even to
 * quote it or to say it was removed.  `progress.py` tests
 * `"<phrase>" in text` over the whole unit source -- a bare substring, with
 * no line anchor and no notion of quoting -- so a sentence ABOUT the marker
 * re-banks the unit exactly as the marker itself would.  The round-46 head
 * did this on its first edit and the `banked` column did not move; the
 * phrase is spelled out in `tools/progress.py` and in docs/PARALLEL-RUNS.md,
 * which is where to go read it.
 *
 * WHY IT WAS UNCARVED UNTIL NOW, and why that reason is dead.  The splat
 * yaml called this segment "the gp_rel-densest ground in the executable",
 * censused it at 3 of 23 clean in round 26, and ended with a directive:
 * "Not worth a runner until the gp-relative blocker moves."  It moved --
 * round 42, maspsx `--gp-symbols`, pinned in the Makefile, whole image
 * byte-exact (CLAUDE.md, "Open toolchain blockers").  `tools/uncarved.py`
 * measures this segment at **23 of 23 blocker-clean**.  Both the census and
 * the directive are retracted in the yaml entry; if you find either quoted
 * anywhere else, it is stale.
 *
 * Carve-time screens, four per function over the live segment: 23 of 23
 * clean; zero `jr $t2` BIOS trampolines; zero `jtbl_` references; zero
 * `alabel`; zero non-`.L` alt-entry labels; zero `.word .L`; no function
 * with more than one `addiu $sp, $sp, -N` prologue.  So NO rodata attach --
 * every `%hi` here is a named dlabel in the D_80087xxx class-table region,
 * which a standalone data segment resolves.  Sizes run 13w..111w with no
 * trivial leaves: every one is a real body.
 *
 * EXPECT THIS SLICE TO SPAN MORE THAN ONE CLASS.  It is cut at ROM
 * addresses, not at class boundaries.  Identify each with
 * `tools/classtable.py`, never by counting slots -- and note the game is
 * plain C with a hand-rolled class framework, not C++.
 *
 * This unit includes include/class_3bb8c.h, which eleven other units also
 * include.  Whoever is staffed here should be the ONLY runner in the
 * class_3bb8c block that round, or the head should price the contention
 * with `python3 tools/headercontention.py` first.
 */

#include "common.h"

/* Local view only, not the shared header: `D_8008AC94` is already established
 * as a `LocalM4D0Obj *` in `src/class_3bb8c_m.c` (round 15, own local type),
 * with named slots at +0x04C/+0x064/+0x068.  This function dispatches +0x004
 * instead, a slot that unit never names -- kept as its own minimal local
 * view rather than importing that unit's type (multiple-independent-local-
 * views convention; class_3bb8c_m.c is not this unit's to edit). */
typedef struct ObjAB54 ObjAB54;
typedef struct ObjAB54Methods ObjAB54Methods;
struct ObjAB54Methods {
    u8 pad0[0x4];
    void (*slot4)(ObjAB54 *self); /* +0x004 */
};
struct ObjAB54 {
    ObjAB54Methods *methods; /* +0x000 */
};

extern const u8 *D_8008AB54;
extern s32 D_8008AC94;

void func_80054714(void) {
    if (D_8008AB54 != 0) {
        ((ObjAB54 *) D_8008AC94)->methods->slot4((ObjAB54 *) D_8008AC94);
        D_8008AB54 = 0;
    }
}

extern s32 D_8008AC74;
extern s32 D_8008AC6C;
extern s8 D_800873DC[];
extern s32 D_8008AC80;
extern s8 D_800873D8[];
extern s32 D_8008AC84;
extern s32 D_800873C8[];
extern s32 D_8008AC90;
extern u8 D_8008726C[];
extern u8 D_800872C4[];
extern s32 D_8008AC8C;
extern u8 D_80087234[];
extern s32 D_8008AB50;

void *func_80054758(void) {
    s32 sum;
    s32 kind;
    s32 divisor;
    s32 remainder;
    s8 *result;
    s32 b3;
    s32 b2;
    u8 *tab;

    sum = D_8008AC74 + D_8008AC6C;
    kind = D_800873DC[sum & 0xF];
    D_8008AC80 = kind;
    divisor = D_800873D8[kind];
    remainder = sum % divisor;
    D_8008AC84 = remainder;
    result = (s8 *) D_800873C8[kind] + remainder * 4;
    if (kind == 0) {
        b3 = result[3];
        D_8008AC90 = (s32) (D_800872C4 + b3 * 3);
        b2 = result[2];
        tab = D_8008726C;
        if (b2 != 0x12) {
            tab = D_80087234;
        }
        D_8008AC8C = (s32) tab;
        if (remainder < 4) {
            D_8008AB50 = 1;
        } else if (remainder < 6) {
            D_8008AB50 = 2;
        }
    }
    return result;
}

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_n", func_80054850);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_n", func_800549A8);

void func_80054B1C(u8 *dst, u8 *src, s32 delta) {
    dst[0] = src[0] - delta;
    dst[1] = src[1] - delta;
    dst[2] = src[2] + delta;
}

extern void func_800183DC(void **array, s32 count);
extern s32 D_8008AB50;
extern void *D_8008E10C[];

void func_80054B50(void) {
    if (D_8008AB50 != 0) {
        func_800183DC(D_8008E10C, 0x12);
        D_8008AB50 = 0;
    }
}

extern s32 D_8008AC80;
extern s32 D_8008AC7C;
extern void func_80056F5C(s32 arg0, void *arg1, s32 arg2, s32 arg3);
extern s32 rand(void);
extern s8 D_80087324[];
extern s32 D_8008AC88;
extern void *D_8008E0C8[];
extern void *func_80054DA4(void *arg0, s32 arg1, void *arg2);
extern void **func_80054F30(void **arg0, s32 arg1, void *arg2);
extern void func_80054FD8(void *arg0, void *arg1);
extern void func_8005511C(void *arg0, void *arg1);

void func_80054B84(void *arg0) {
    s32 base;
    s32 val;
    s32 count;
    void **filled;

    if (D_8008AC80 < 0) {
        return;
    }
    base = D_8008AC7C;
    func_80056F5C(D_8008AC80, (void *) *(s32 *) (base + 4), *(s32 *) (base + 8), *(s32 *) (base + 0xC));
    val = D_80087324[rand() & 3];
    count = (D_8008AC80 == 2) ? 0x10 - val : 0;
    D_8008AC88 = val + count;
    filled = (void **) func_80054DA4(D_8008E0C8, val, arg0);
    filled = func_80054F30(filled, count, arg0);
    if (D_8008AC80 == 0) {
        func_80054FD8(filled, arg0);
    } else if (D_8008AC80 == 2) {
        func_8005511C(filled, arg0);
    } else {
        return;
    }
    D_8008AC88 = D_8008AC88 + 1;
}

/* Local view: array elements at D_8008E0C8 are objects with a method table
 * pointer at offset 0, dispatched here through slot +0xEC as
 * slotEC(self, arg1) -- mirrors the ObjAB54 pattern above. */
typedef struct ObjE0C8 ObjE0C8;
typedef struct ObjE0C8Methods ObjE0C8Methods;
struct ObjE0C8Methods {
    u8 padEC[0xEC];
    void (*slotEC)(ObjE0C8 *self, void *arg1); /* +0x0EC */
};
struct ObjE0C8 {
    ObjE0C8Methods *methods; /* +0x000 */
};

extern s32 D_8008AC80;
extern s32 D_8008AC88;
extern void *D_8008E0C8[];

void func_80054C74(void *arg0) {
    s32 i;
    ObjE0C8 *obj;

    if (D_8008AC80 < 0) {
        return;
    }
    for (i = 0; i < D_8008AC88; i++) {
        obj = (ObjE0C8 *) D_8008E0C8[i];
        obj->methods->slotEC(obj, arg0);
    }
}

extern s32 D_8008AC80;
extern s32 D_8008AC88;
extern void *D_8008E0C8[];

void func_80054CFC(void) {
    if (D_8008AC80 >= 0) {
        func_800183DC(D_8008E0C8, D_8008AC88);
    }
}

/* Local view only: `func_800557DC` (defined later in this unit, in strict
 * ROM order) takes one of these two per-slot objects. `unk0` is address-
 * taken then chased for a single byte at +0x6 (toggled there); `unk14` is
 * only ever address-taken, as an embedded sub-object handed to
 * `func_8002CC84`/`func_8002CD08` (same discard-return caveat as
 * `include/Entity.h`'s `unk9C` -- a field only ever address-taken carries
 * no evidence about its own declared type). */
typedef struct ObjN14Sub ObjN14Sub;
struct ObjN14Sub {
    u8 pad0[0x6];
    s8 unk6; /* +0x006 */
};

typedef struct ObjN14 ObjN14;
struct ObjN14 {
    ObjN14Sub *unk0; /* +0x000 */
    s32 unk4; /* +0x004 */
    u8 pad8[0x4];
    s32 unkC; /* +0x00C */
    s32 unk10; /* +0x010 */
    s32 unk14; /* +0x014 */
};

extern s32 func_800557DC(ObjN14 *arg0);

extern s32 D_8008AB4C;
extern ObjN14 *D_8008AC9C[2];

void func_80054D30(void) {
    s32 i;

    func_80054714();
    func_80054B50();
    func_80054CFC();
    for (i = 0; i < 2; i++) {
        D_8008AC9C[i] = (ObjN14 *) func_800557DC(D_8008AC9C[i]);
    }
    if (D_8008AB4C != 0) {
        D_8008AB4C = 0;
    }
}

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_n", func_80054DA4);

extern s32 D_80087330;
extern u8 D_80087204[];
extern u8 D_8008E0A4[];
extern u8 *D_8008E0B4;
extern void *func_80055258(void *arg0, void *arg1);
extern void *func_80056320(void *arg0, void *arg1, void *arg2, void *arg3);

void **func_80054F30(void **arg0, s32 arg1, void *arg2) {
    s32 i;
    s32 val;

    val = D_80087330;
    D_8008E0B4 = D_80087204;
    for (i = 0; i < arg1; i++) {
        func_80055258(arg2, (void *) val);
        *arg0 = func_80056320((void *) 1, D_8008E0A4, (void *) D_8008AB4C, arg2);
        arg0++;
    }
    return arg0;
}

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_n", func_80054FD8);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_n", func_8005511C);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_n", func_80055258);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_n", func_80055410);

extern s32 D_8008AC7C;
extern void *func_80055620(s32 *arg0, s32 *arg1);
extern s32 D_800874B0[];
extern s32 func_8002CC34(s32 arg0, void *arg1, s32 arg2, void *arg3, s32 arg4);

ObjN14 *func_8005556C(ObjN14 *arg0, s32 *arg1) {
    ObjN14Sub *sub;

    sub = (ObjN14Sub *) func_80055620(&arg0->unk4, &arg0->unk10);
    if (sub != 0) {
        arg0->unk0 = sub;
        func_8002CC34(*(s32 *) D_8008AC7C, &arg0->unk14, sub->unk6, arg0, D_800874B0[sub->unk6]);
        if (sub->unk6 == *arg1) {
            *arg1 = -sub->unk6;
        }
        sub->unk6 = -sub->unk6;
        return arg0;
    }
    return 0;
}

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_n", func_80055620);

extern s32 D_8008AC7C;
extern void func_8002CC84(s32 arg0, void *arg1);

s32 func_800557DC(ObjN14 *arg0) {
    func_8002CC84(*(s32 *) D_8008AC7C, &arg0->unk14);
    arg0->unk0->unk6 = -arg0->unk0->unk6;
    return 0;
}

extern s32 func_80055874(ObjN14 *arg0, void *arg1);
extern void func_8002CD08(s32 arg0, void *arg1);

s32 func_8005582C(ObjN14 *arg0, void *arg1) {
    if (func_80055874(arg0, arg1) != 0) {
        func_8002CD08(*(s32 *) D_8008AC7C, &arg0->unk14);
        return 1;
    }
    return 0;
}

extern s32 D_80087474[];

s32 func_80055874(ObjN14 *arg0, void *arg1) {
    s32 dx, dy, dist;
    s8 idx;

    if (arg1 == 0) {
        return 0;
    }
    dx = arg0->unk4 - *(s32 *) arg1;
    if (dx < 0) {
        dx = ~dx + 1;
    }
    dy = arg0->unkC - *(s32 *) ((u8 *) arg1 + 0x8);
    if (dy >= 0) {
        dist = dx + dy;
    } else {
        dist = dx - dy;
    }
    arg0->unk10 = dist;
    idx = arg0->unk0->unk6;
    if (dist < D_80087474[-idx]) {
        dist = 1;
        return dist;
    }
    return 0;
}

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_n", func_800558F0);

extern s32 D_8008AC6C;
extern void func_8003B624(void *arg0, s32 arg1, void *arg2);
extern s32 D_80087444[];
extern s32 D_80087450[];
extern s32 D_8008745C[];
extern s32 D_80087468[];

void func_80055A24(void) {
    void *a0, *a2;
    s32 a1;

    if (D_8008AC6C == 2) {
        a0 = D_80087444;
        a2 = D_80087450;
        a1 = 1;
    } else if ((u32) (D_8008AC6C - 3) < 3) {
        a1 = 1;
        a0 = D_8008745C;
        a2 = D_80087468;
    } else {
        return;
    }
    func_8003B624(a0, a1, a2);
}
