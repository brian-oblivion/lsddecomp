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

extern s32 D_8008AB68;
extern s32 D_8008AB6C;
extern s32 D_8008AB70;
extern s32 D_8008AB74;
extern void *New_ClassEAC0(void *a0, void *a1, s32 a2);
extern void *D_8008E10C[];
extern s32 D_8008AC7C;

typedef struct ObjSlot4C ObjSlot4C;
typedef struct ObjSlot4CMethods ObjSlot4CMethods;
struct ObjSlot4CMethods {
    u8 pad4C[0x4C];
    void (*slot4C)(ObjSlot4C *self, void *arg1, void *arg2); /* +0x04C */
};
struct ObjSlot4C {
    ObjSlot4CMethods *methods; /* +0x000 */
};

typedef struct ObjSlotAC ObjSlotAC;
typedef struct ObjSlotACMethods ObjSlotACMethods;
struct ObjSlotACMethods {
    u8 padAC[0xAC];
    void *(*slotAC)(ObjSlotAC *self); /* +0x0AC */
};
struct ObjSlotAC {
    ObjSlotACMethods *methods; /* +0x000 */
};

/* Local view: D_8008AB68/D_8008AB6C and D_8008AB70/D_8008AB74 are two
 * adjacent 8-byte pairs, and this unit copies each into a local pair as a
 * WHOLE-STRUCT assignment rather than field by field.  That is not a style
 * choice -- it is load-bearing.  A BLKmode set makes gcc 2.6.3's cse.c call
 * invalidate_memory(), dropping every cached memory value, which is what
 * produces retail's otherwise inexplicable reload of D_8008AB50 for the
 * `== 2` test and its reload of the pair's second word right after writing
 * it.  Written as two scalar stores, neither reload appears and the body is
 * several words short.  Round 61; see docs/match-reports/func_80054850.md. */
typedef struct PairXY PairXY;
struct PairXY {
    s32 x; /* +0x000 */
    s32 y; /* +0x004 */
};

/* STALL, 17/86 words (best), 1 word SHORT (map-measured 85 words), first
 * real diff at word 15 (0x4508C / vram 0x8005488C) -- see
 * docs/match-reports/func_80054850.md.  Round 61 REVISIT: 12/86 and 6 short
 * -> 17/86 and 1 short.  Residue is one redundant `move` retail emits and
 * two scheduling reorderings.  Preserved near-miss body: */
#if 0
void func_80054850(void) {
    PairXY paramA;
    PairXY paramB;
    s32 i;
    s32 s1;
    void **arr;
    void **wp;
    void *obj;
    ObjSlotAC *self2;
    void *result;

    if (D_8008AB50 == 0) {
        return;
    }
    paramA = *(PairXY *) &D_8008AB68;
    if (D_8008AB50 == 2) {
        paramA.y += 0x1E;
    }
    paramB = *(PairXY *) &D_8008AB70;
    i = 1;
    s1 = 3;
    obj = New_ClassEAC0(&paramB, (void *) D_8008AC8C, 0x1FFF);
    __asm__("");
    arr = D_8008E10C;
    wp = arr + 1;
    *arr = obj;
    do {
        obj = New_ClassEAC0(&paramB, (void *) (s1 + D_8008AC8C), 0x1FFF);
        *wp = obj;
        wp++;
        ((ObjSlot4C *) obj)->methods->slot4C(obj, arr[0], &paramA);
        s1 += 3;
        paramA.y += 3;
        paramB.y -= 7;
        i++;
    } while (i < 0x12);

    self2 = *(ObjSlotAC **) (D_8008AC7C + 0xC);
    result = self2->methods->slotAC(self2);
    ((ObjSlot4C *) D_8008E10C[0])->methods->slot4C(D_8008E10C[0], result, &paramA);
}
#endif
INCLUDE_ASM("asm/nonmatchings/class_3bb8c_n", func_80054850);

typedef struct ObjAC7CSub ObjAC7CSub;
typedef struct ObjAC7CSubMethods ObjAC7CSubMethods;
struct ObjAC7CSubMethods {
    u8 pad64[0x64];
    void (*slot64)(ObjAC7CSub *self, void *arg1); /* +0x064 */
};
struct ObjAC7CSub {
    ObjAC7CSubMethods *methods; /* +0x000 */
    u8 pad4[0x18 - 0x4];
    s32 field18; /* +0x018 */
    u8 pad1C[0x24 - 0x1C];
    s32 field24; /* +0x024 */
};

typedef struct ObjSlotB8B8 ObjSlotB8B8;
typedef struct ObjSlotB8B8Methods ObjSlotB8B8Methods;
struct ObjSlotB8B8Methods {
    u8 padB8[0xB8];
    void (*slotB8)(ObjSlotB8B8 *self, s32 arg1, void *arg2); /* +0x0B8 */
    void (*slotBC)(ObjSlotB8B8 *self, void *arg1); /* +0x0BC */
};
struct ObjSlotB8B8 {
    ObjSlotB8B8Methods *methods; /* +0x000 */
};

/* MATCHED round 61 (bravo), first attempt, after the BLKmode-struct-copy
 * lever found on func_80054850 -- see docs/match-reports/func_800549A8.md.
 * `rgb` is written only at [0..2] (by func_80054B1C); its declared size of 8
 * is inferred from the STACK LAYOUT (it occupies sp+0x10..0x17, with `pos`
 * at sp+0x18), not from any access. */
void func_800549A8(void) {
    ObjAC7CSub *self;
    s32 delta;
    s32 shift;
    u8 rgb[8];
    PairXY pos;
    s32 srcOfs;
    s32 i;
    void **wp;
    ObjSlotB8B8 *obj;

    if (D_8008AB50 == 0) {
        return;
    }
    self = *(ObjAC7CSub **) (D_8008AC7C + 0xC);
    delta = self->field18 - self->field24;
    shift = (delta / 600) * 3;
    if (shift <= 0) {
        return;
    }
    pos = *(PairXY *) &D_8008AB68;
    i = 0;
    if (D_8008AB50 == 2) {
        pos.y += 0x1E;
    }
    wp = D_8008E10C;
    srcOfs = 0;
    pos.y += shift * 3;
    do {
        func_80054B1C(rgb, (u8 *) (srcOfs + D_8008AC8C), shift);
        obj = (ObjSlotB8B8 *) *wp;
        obj->methods->slotB8(obj, 1, rgb);
        obj = (ObjSlotB8B8 *) *wp;
        i++;
        srcOfs += 3;
        obj->methods->slotBC(obj, &pos);
        pos.y += 3;
        wp++;
    } while (i < 0x12);
    func_80054B1C(rgb, (u8 *) D_8008AC90, shift);
    self->methods->slot64(self, rgb);
}

void func_80054B1C(u8 *dst, u8 *src, s32 delta) {
    dst[0] = src[0] - delta;
    dst[1] = src[1] - delta;
    dst[2] = src[2] + delta;
}

extern void ReleaseBasicClassArray(void **array, s32 count);
extern s32 D_8008AB50;
extern void *D_8008E10C[];

void func_80054B50(void) {
    if (D_8008AB50 != 0) {
        ReleaseBasicClassArray(D_8008E10C, 0x12);
        D_8008AB50 = 0;
    }
}

extern s32 D_8008AC80;
extern s32 D_8008AC7C;
extern void BaseObjO__func_56f5c(s32 arg0, void *arg1, s32 arg2, s32 arg3);
extern s32 rand(void);
extern s8 D_80087324[];
extern s32 D_8008AC88;
extern void *D_8008E0C8[];
extern void *func_80054DA4(void *arg0, s32 arg1, void *arg2);
extern void **func_80054F30(void **arg0, s32 arg1, void *arg2);
extern void **func_80054FD8(void **arg0, void *arg1);
extern void *func_8005511C(void *arg0, void *arg1);

void func_80054B84(void *arg0) {
    s32 base;
    s32 val;
    s32 count;
    void **filled;

    if (D_8008AC80 < 0) {
        return;
    }
    base = D_8008AC7C;
    BaseObjO__func_56f5c(D_8008AC80, (void *) *(s32 *) (base + 4), *(s32 *) (base + 8), *(s32 *) (base + 0xC));
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
        ReleaseBasicClassArray(D_8008E0C8, D_8008AC88);
    }
}

/* Local view only: `func_800557DC` (defined later in this unit, in strict
 * ROM order) takes one of these two per-slot objects. `unk0` is address-
 * taken then chased for a single byte at +0x6 (toggled there); `unk14` is
 * only ever address-taken, as an embedded sub-object handed to
 * `FlushSoundCueSet`/`func_8002CD08` (same discard-return caveat as
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

extern u8 D_800871C8[];
extern s32 D_80087328[];
extern u8 *D_8008E0B4;
extern s32 D_8008E0BC;
extern s32 D_8008E0A4;
extern void func_80055258(void *arg0, void *arg1);
extern void func_80055410(void *arg0, void *arg1);
extern void *func_80056320(void *arg0, void *arg1, void *arg2, void *arg3);

/* STALL, 93/99 words (length matches, 0x18C, re-measured round 48; earlier
 * round 47 report recorded 87/99), whole-function arg0/arg1/arg2
 * register-colour rotation (s2/s5/s4) -- see
 * docs/match-reports/func_80054DA4.md. Round 48: check 3 confirms AGREE
 * (permuter scaffold: Insertions 0, Deletions 0, Reorderings 0, pure
 * Stack/Register-field residue -- matches the in-tree rebuild's pure
 * word-level register-field diffs). Preserved near-miss body: */
#if 0
void *func_80054DA4(void *arg0, s32 arg1, void *arg2) {
    void **arr;
    s32 i;
    s32 t3;
    void (*fp)(void *, void *);

    arr = (void **) arg0;
    D_8008E0BC = rand() % 7;
    D_8008E0B4 = (u8 *) D_800871C8 + ((u32) rand() % 5) * 12;
    t3 = (u32) rand() % 5;
    if (t3 != 0) {
        t3 = D_80087328[t3];
    }
    fp = func_80055410;
    if (D_8008AC74 % 7 != 0) {
        fp = func_80055258;
    }
    for (i = 0; i < arg1; i++) {
        fp(arg2, (void *) t3);
        *arr = func_80056320((void *) 0, &D_8008E0A4, (void *) D_8008AB4C, arg2);
        arr++;
    }
    return (void *) arr;
}
#endif
INCLUDE_ASM("asm/nonmatchings/class_3bb8c_n", func_80054DA4);

extern s32 D_80087330;
extern u8 D_80087204[];

void **func_80054F30(void **arg0, s32 arg1, void *arg2) {
    s32 i;
    s32 val;

    val = D_80087330;
    D_8008E0B4 = D_80087204;
    for (i = 0; i < arg1; i++) {
        func_80055258(arg2, (void *) val);
        *arg0 = func_80056320((void *) 1, &D_8008E0A4, (void *) D_8008AB4C, arg2);
        arg0++;
    }
    return arg0;
}

extern s32 D_80087330;
extern void func_80055258(void *arg0, void *arg1);
extern s32 D_8008E0C0[];
extern u8 *D_8008E0B0;
extern u8 D_80087174[];
extern s32 D_8008E0A8;
extern s32 D_8008E0AC;
extern u8 D_8008721C[];

/* STALL, 78/81 words, length EXACT (no drift), first real diff at word 69
 * (0x458EC / vram 0x800550EC) -- see docs/match-reports/func_80054FD8.md.
 * Round 61 REVISIT: 38/81 -> 78/81. Round 47's "pure arg0/arg1 register-
 * colour swap, ZERO drift" verdict was wrong on both counts; the equal
 * length was two defects cancelling. Residue is now ONE instruction's
 * placement: the `*q = D_80087174` store must sink below `lw a2` and
 * `move a3` into the jal's delay slot, and cannot because a store through
 * a pointer is an opaque MEM that gcc 2.6.3's scheduler will not let the
 * gp-relative load hoist across. Preserved near-miss body: */
/* STALL, 79/81 words, length EXACT, insertions 0 / deletions 0, first real
 * diff at word 60 (0x458C8 / vram 0x800550C8) -- see
 * docs/match-reports/func_80054FD8.md.  Round 61 REVISIT: 38/81 -> 79/81.
 * Round 47's "pure arg0/arg1 register-colour swap, ZERO drift" verdict was
 * wrong on both counts; the equal length was two defects cancelling.  What
 * is left is 2 words of genuine register identity: the else branch's
 * computed value sits in $a2 here and in $v1 in retail, because `t`'s single
 * pseudo (deliberately shared -- splitting it into two variables costs 4
 * words) coalesces with the third-argument register.  Preserved near-miss
 * body: */
#if 0
void **func_80054FD8(void **arg0, void *arg1) {
    s32 t;
    s32 *p;
    u8 **q;

    func_80055258(arg1, (void *) D_80087330);
    if (D_8008AB50 != 0 && D_8008AC8C == (s32) D_8008726C) {
        D_8008E0A4 = 0xFFFF5000;
        D_8008E0A8 = -0x2000;
        D_8008E0AC = 0;
        D_8008E0C0[0] = (s32) (D_8008721C + 3);
    } else {
        p = &D_8008E0AC;
        if (*p > 0) {
            *p = -*p;
        }
        if (*p < -0x7800) {
            *p = -0x7800;
        }
        t = (s32) (D_8008721C + ((u32) rand() % 3) * 3);
        D_8008E0C0[0] = t;
    }
    t = D_8008AB4C;
    q = &D_8008E0B0;
    *q = D_80087174;
    *arg0 = func_80056320((void *) 3, (u8 *) q - 0xC, (void *) t, arg1);
    arg0++;
    return arg0;
}
#endif
INCLUDE_ASM("asm/nonmatchings/class_3bb8c_n", func_80054FD8);

extern s32 D_80087430;
extern u8 D_80087228[];
extern s32 D_8008E0C0[];
extern u8 *D_8008E0B0;
extern u8 D_80087174[];
extern s32 D_8008E0BC;

/* STALL, 16/79 words, 1 word short (78/79 built) -- see
 * docs/match-reports/func_8005511C.md. Round 48: check 3 confirms AGREE
 * (scaffold Insertions 12, Deletions 13, Reorderings 3 vs in-tree
 * rebuild's identical 16/79 with expected out-of-range drift from the
 * 1-word-short size). Preserved near-miss body: */
#if 0
void *func_8005511C(void *arg0, void *arg1) {
    s32 idx;
    s32 randval;
    s32 v0;
    s32 *slot;

    idx = (u32) rand() % 3;
    slot = D_8008E0C0;
    *slot = (s32) (D_80087228 + idx * 3);
    slot++;
    if (D_8008AC74 % 20 == 0) {
        v0 = 0;
    } else {
        v0 = D_80087430;
    }
    *slot = v0;
    func_80055258(arg1, (void *) D_80087330);
    D_8008E0B0 = D_80087174;
    randval = rand();
    D_8008E0BC = randval - (randval / 3) * 6;
    *(void **) arg0 = func_80056320((void *) 2, (u8 *) &D_8008E0B0 - 0xC, (void *) D_8008AB4C, arg1);
    return (u8 *) arg0 + 4;
}
#endif
INCLUDE_ASM("asm/nonmatchings/class_3bb8c_n", func_8005511C);

extern s32 D_8008E0A8;
extern s32 D_8008E0AC;
extern u8 *D_8008E0B0;
extern u8 D_80087174[];
extern s32 D_8008E0B8;

/* MATCHED round 64 (charlie), 110/110, ins 0 / del 0, one build.  The
 * round-46..48 residue (an extra callee-saved register caching
 * `D_8008E0A4`'s address, frame -0x18 -> -0x20) was NOT register identity:
 * it was the DECLARED TYPE of the global.  Declared `extern u8
 * D_8008E0A4[]` and written `*(s32 *) D_8008E0A4 = v`, the array decay is
 * an address-take VALUE that cc1 2.6.3's CSE promotes into a callee-saved
 * register across the intervening `rand()` calls.  Declared `extern s32
 * D_8008E0A4` and assigned BY NAME it emits retail's absolute
 * `lui $at, %hi / sw %lo($at)` fresh at each of the three accesses.
 * See docs/match-reports/func_80055258.md. */
void func_80055258(void *arg0, void *arg1) {
    if (arg1 == 0) {
        arg1 = (void *) D_80087328[rand() & 3];
    }
    D_8008E0A8 = (s32) arg1;
    D_8008E0A4 = (rand() % 23) << 11;
    if (rand() & 1) {
        D_8008E0A4 = -D_8008E0A4;
    }
    D_8008E0AC = (rand() % 23) << 11;
    if (rand() & 1) {
        D_8008E0AC = -D_8008E0AC;
    }
    D_8008E0B0 = D_80087174 + ((u32) rand() % 7) * 12;
    D_8008E0B8 = rand() % 5;
}

extern s32 D_8008732C;
extern s32 D_8008E0B8;

/* STALL, 25/87 words, 2 words long -- see docs/match-reports/func_80055410.md.
 * Signature widened from `void func_80055410(void)` (round 46) to two dead
 * void* params: func_80054DA4 dispatches this through a function pointer
 * shared with func_80055258 (which genuinely takes two args), and the ABI
 * slot is call-site-determined, not body-determined -- see CLAUDE.md's
 * "already-matched signature can be too narrow" lesson. Dead params cost
 * zero instructions in the callee, so the round-46 body is otherwise
 * untouched. Round 48: check 3 confirms AGREE (scaffold Insertions 3,
 * Deletions 1, Register 90 vs in-tree rebuild's identical 25/87 with
 * expected out-of-range drift from the 2-extra-word size). Round 48:
 * searched (not closed, 900s/136367 iterations, best score 100/850 base);
 * the score-100 candidate's literal transcription regressed to 5/87
 * in-tree (worse than 25/87) rather than the isolated scaffold's
 * improvement -- not applied. Preserved near-miss body: */
#if 0
void func_80055410(void *arg0, void *arg1) {
    s32 r;
    s32 mod3;

    rand();
    D_8008E0A8 = D_8008732C;
    r = rand();
    D_8008E0A4 = (r % 20) << 11;
    mod3 = D_8008AC74 % 3;
    D_8008E0AC = 0xA000;
    if (mod3 == 1) {
        D_8008E0AC = -0xA000;
    } else if (mod3 == 2) {
        D_8008E0AC = 0x800;
    }
    r = rand();
    D_8008E0B0 = D_80087174 + ((u32) r % 7) * 12;
    r = rand();
    D_8008E0B8 = r % 5;
}
#endif
INCLUDE_ASM("asm/nonmatchings/class_3bb8c_n", func_80055410);

extern s32 D_8008AC7C;
extern void *func_80055620(void *arg0, s32 *arg1, void *arg2);
extern s32 D_800874B0[];
extern s32 InitSoundCueSet(s32 arg0, void *arg1, s32 arg2, void *arg3, s32 arg4);

ObjN14 *func_8005556C(ObjN14 *arg0, s32 *arg1, void *arg2, void *arg3) {
    ObjN14Sub *sub;

    sub = (ObjN14Sub *) func_80055620(&arg0->unk4, &arg0->unk10, arg2);
    if (sub != 0) {
        arg0->unk0 = sub;
        InitSoundCueSet(*(s32 *) D_8008AC7C, &arg0->unk14, sub->unk6, arg0, D_800874B0[sub->unk6]);
        if (sub->unk6 == *arg1) {
            *arg1 = -sub->unk6;
        }
        sub->unk6 = -sub->unk6;
        return arg0;
    }
    return 0;
}

extern s32 D_8008AC6C;
extern s32 D_8008AC98;
extern u8 *D_800876B4[];
extern u8 D_800876EC[];
extern u8 D_800874EC[];
extern s32 D_80087474[];

/* Local view only: `D_8008AB4C`'s value is another "pointer stored as a
 * plain s32" (same idiom as `D_8008AC7C`), here treated as a "self" object
 * with a method table at offset 0, dispatched through slot +0x0E8. Moved
 * ahead of its original spot (just before func_800558F0) because
 * func_80055620, ROM-earlier, also dispatches through it. */
typedef struct ObjAB4C ObjAB4C;
typedef struct ObjAB4CMethods ObjAB4CMethods;
struct ObjAB4CMethods {
    u8 padE8[0xE8];
    void (*slotE8)(ObjAB4C *self, void *arg1, void *arg2); /* +0x0E8 */
};
struct ObjAB4C {
    ObjAB4CMethods *methods; /* +0x000 */
};

typedef struct Pos4 Pos4;
struct Pos4 {
    s16 hi;
    s16 lo;
};

typedef struct TabEntry TabEntry;
struct TabEntry {
    Pos4 head;
    s16 tail;
};

typedef struct EntrySlot EntrySlot;
struct EntrySlot {
    Pos4 pos;  /* +0x0 */
    u8 idx;    /* +0x4 */
    u8 pad5;   /* +0x5 */
    s8 count;  /* +0x6 */
    u8 pad7;   /* +0x7 */
};

typedef struct LocalBuf LocalBuf;
struct LocalBuf {
    Pos4 pos;
    TabEntry tab;
};

/* MATCHED round 48 (alpha), 111/111 -- see docs/match-reports/func_80055620.md
 * for the round 47 (bravo) recovery and the round 48 permuter lead that
 * closed it: the `if (n <= 0) goto fail;` early exit is redundant (the
 * `for (j = 0; j < n; ...)` loop already falls through to the same
 * `fail: return 0;` when n <= 0) and dropping it, plus writing the
 * `entry` pointer's address computation as `offset + (s32) base` instead
 * of `base + offset`, closed the last word (a pure commutative-operand
 * encoding-order residue in the `addu`). */
void *func_80055620(void *arg0, s32 *arg1, void *arg2) {
    s32 j, n;
    u8 *base;
    EntrySlot *entry;
    LocalBuf buf;
    s32 d1, d2, dist;
    void *self;

    if (arg2 == 0) {
        goto fail;
    }
    base = D_800876B4[D_8008AC6C];
    n = D_800876EC[D_8008AC6C] - D_8008AC98;
    entry = (EntrySlot *) (D_8008AC98 * 8 + (s32) base);
    for (j = 0; j < n; j++, entry++) {
        D_8008AC98++;
        if (entry->count > 0) {
            buf.pos = entry->pos;
            buf.tab = *(TabEntry *) (D_800874EC + entry->idx * 6);
            self = (void *) D_8008AB4C;
            ((ObjAB4C *) self)->methods->slotE8((ObjAB4C *) self, arg0, &buf);
            d1 = *(s32 *) arg0 - *(s32 *) arg2;
            if (d1 < 0) {
                d1 = ~d1 + 1;
            }
            d2 = *(s32 *) ((u8 *) arg0 + 8) - *(s32 *) ((u8 *) arg2 + 8);
            if (d2 >= 0) {
                dist = d1 + d2;
            } else {
                dist = d1 - d2;
            }
            *arg1 = dist;
            if (dist < D_80087474[entry->count]) {
                return entry;
            }
        }
    }
fail:
    return 0;
}

extern s32 D_8008AC7C;
extern void FlushSoundCueSet(s32 arg0, void *arg1);

s32 func_800557DC(ObjN14 *arg0) {
    FlushSoundCueSet(*(s32 *) D_8008AC7C, &arg0->unk14);
    arg0->unk0->unk6 = -arg0->unk0->unk6;
    return 0;
}

extern s32 func_80055874(ObjN14 *arg0, void *arg1);
extern void func_8002CD08(s32 arg0, void *arg1);

s32 func_8005582C(ObjN14 *arg0, void *arg1, void *arg2) {
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

extern void func_80054850(void);
extern void func_800549A8(void);
extern void func_80055A24(void);
extern s32 D_8008AC70;
extern s32 D_8008AC98;
extern u8 D_8008E154[];
extern ObjN14 *func_8005556C(ObjN14 *arg0, s32 *arg1, void *arg2, void *arg3);
extern s32 func_8005582C(ObjN14 *arg0, void *arg1, void *arg2);

s32 func_800558F0(void *arg0, void *arg1, s32 arg2) {
    void *ctx;
    u8 buf[0x10];
    s32 i;

    ctx = 0;
    if (arg0 != 0) {
        ctx = buf;
        ((ObjAB4C *) D_8008AB4C)->methods->slotE8((ObjAB4C *) D_8008AB4C, ctx, arg0);
    }
    if (D_8008AC70++ == 0) {
        func_80054660();
        func_80054850();
        func_80054B84(ctx);
    }
    func_800549A8();
    func_80054C74(ctx);
    func_80055A24();
    D_8008AC98 = 0;
    for (i = 0; i < 2; i++) {
        if (D_8008AC9C[i] != 0) {
            if (func_8005582C(D_8008AC9C[i], ctx, arg1) == 0) {
                D_8008AC9C[i] = (ObjN14 *) func_800557DC(D_8008AC9C[i]);
            }
            /* INERT ON PURPOSE -- DO NOT DELETE. This pair is a semantic
             * no-op (`i` is the initialized loop counter, so nothing here is
             * an uninitialized read), and it exists solely because it
             * perturbs GCC 2.6.3's allocator back into retail's register
             * colours for `ctx`/`i`. Found by the permuter at iteration 4
             * and kept because the WHOLE-IMAGE SHA1 verifies with it, not
             * because the permuter's own scorer liked it (round 41: a
             * scorer zero is a lead, an `OK: build matches retail` is an
             * answer). Removing these two lines re-breaks func_800558F0. */
            i++;
            i--;
        } else {
            D_8008AC9C[i] = func_8005556C((ObjN14 *) (D_8008E154 + i * 0x68), &arg2, ctx, arg1);
        }
    }
    return arg2;
}

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
