#include "common.h"
#include "code_8220.h"
#include "gte.h"

void func_80019774(void *dst, s32 flag)
{
    if (flag) {
        gte_stsxy3_ft4(dst);
    } else {
        char *p = (char *)dst + 0x20;

        gte_stsxy2(p);
    }
}

void func_8001979C(void *dst, s32 flag)
{
    if (flag) {
        gte_stsxy3_gt4(dst);
    } else {
        char *p = (char *)dst + 0x2c;

        gte_stsxy2(p);
    }
}

#ifdef NON_MATCHING
/* NON_MATCHING: 46/54 words, length exact. Residue: register identity
 * ($a2 vs $a1 for the OT high-byte mask, cascading to the second reload's
 * register) plus a missing load-delay-slot filler `addiu $v0,$s1,0x14`
 * (docs/match-reports/func_800197C4.md). Hand-derived. */
void func_800197C4(void *arg0, void *arg1) {
    if (*(s32 *)((u8 *)arg1 + 0x78) == 0) {
        ((OtTag *)arg0)->addr = (*(OtTag **)((u8 *)arg1 + 0x30))->addr;
        (*(OtTag **)((u8 *)arg1 + 0x30))->addr = (u32)arg0;
    } else {
        func_8001A380(D_8008ACD0, arg1, (u8 *)arg0 + 0x4, 0, 0, 0);
        func_8001A3EC((PolyVtx **)((u8 *)arg1 + 0x88), (PolyVtx **)((u8 *)arg1 + 0xA4),
                      (PolyUV4 *)((u8 *)arg0 + 0x8), (PolyUV4 *)((u8 *)arg0 + 0xC),
                      (PolyUV4 *)((u8 *)arg0 + 0x10));
        RCpolyF3(arg0, D_8008ACD0);
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/code_8220_c", func_800197C4);
#endif

/* A 2-s16 pair (alignment 2, not 4) -- see func_8001A268's stall report for
 * why this is needed even at accidentally-4-aligned offsets. */
typedef struct {
    s16 x, y;
} Vec2s16_98;

#ifdef NON_MATCHING
/* NON_MATCHING: 76/84 words, length exact. Residue: family-shared register
 * identity ($a2 vs $a1, duplicated OT mask) plus a missing delay-slot
 * filler `addiu $v0,$s1,0x1c` (docs/match-reports/func_8001989C.md).
 * This function's own self/prim register-swap sub-residue is closed by a
 * permuter-found lead (do/while(0) wrap + partial vtx0 caching), reviewed
 * and confirmed semantically equivalent to the disassembly, not scorer-
 * exploiting UB. Hand-derived (with a reviewed permuter-found lever). */
void func_8001989C(void *arg0, void *arg1) {
    u8 *self = (u8 *)arg0;
    u8 *prim = (u8 *)arg1;
    u8 *vtx0;

    if (*(s32 *)(prim + 0x78) == 0) {
        do {
            ((OtTag *)self)->addr = (*(OtTag **)(prim + 0x30))->addr;
            (*(OtTag **)(prim + 0x30))->addr = (u32)self;
        } while (0);
    } else {
        func_8001A380(D_8008ACD0, prim, self + 0x4, 0, 0, 0);
        func_8001A3EC((PolyVtx **)(prim + 0x88), (PolyVtx **)(prim + 0xA4),
                      (PolyUV4 *)(self + 0x8), (PolyUV4 *)(self + 0x10),
                      (PolyUV4 *)(self + 0x18));

        vtx0 = *(u8 **)(prim + 0x88);
        *(u16 *)(vtx0 + 0xA) = *(u8 *)(self + 0xF);
        *(u16 *)(*(u8 **)(prim + 0x8C) + 0xA) = *(u8 *)(self + 0xF);
        *(u16 *)(*(u8 **)(prim + 0x90) + 0xA) = *(u8 *)(self + 0x17);

        *(Vec2s16_98 *)(*(u8 **)(prim + 0x88) + 0xC) = *(Vec2s16_98 *)(self + 0x4);
        *(Vec2s16_98 *)(*(u8 **)(prim + 0x8C) + 0xC) = *(Vec2s16_98 *)(self + 0xC);
        *(Vec2s16_98 *)(*(u8 **)(prim + 0x90) + 0xC) = *(Vec2s16_98 *)(self + 0x14);

        RCpolyG3(self, D_8008ACD0);
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/code_8220_c", func_8001989C);
#endif

#ifdef NON_MATCHING
/* NON_MATCHING: 70/78 words, length exact. Residue: register identity
 * ($a2 vs $a1 for the OT high-byte mask) plus a missing load-delay-slot
 * filler `addiu $v0,$s1,0x20` (docs/match-reports/func_800199EC.md).
 * Hand-derived. */
void func_800199EC(void *arg0, void *arg1) {
    if (*(s32 *)((u8 *)arg1 + 0x78) == 0) {
        ((OtTag *)arg0)->addr = (*(OtTag **)((u8 *)arg1 + 0x30))->addr;
        (*(OtTag **)((u8 *)arg1 + 0x30))->addr = (u32)arg0;
    } else {
        func_8001A380(D_8008ACD0, arg1, (u8 *)arg0 + 0x4, 1, *(u16 *)((u8 *)arg0 + 0xE), *(u16 *)((u8 *)arg0 + 0x16));
        func_8001A3EC((PolyVtx **)((u8 *)arg1 + 0x88), (PolyVtx **)((u8 *)arg1 + 0xA4),
                      (PolyUV4 *)((u8 *)arg0 + 0x8), (PolyUV4 *)((u8 *)arg0 + 0x10),
                      (PolyUV4 *)((u8 *)arg0 + 0x18));

        *(u16 *)(*(u8 **)((u8 *)arg1 + 0x88) + 0xA) = *(u16 *)((u8 *)arg0 + 0x1E);
        *(u16 *)(*(u8 **)((u8 *)arg1 + 0x8C) + 0xA) = *(u16 *)((u8 *)arg0 + 0x1E);
        *(u16 *)(*(u8 **)((u8 *)arg1 + 0x90) + 0xA) = *(u16 *)((u8 *)arg0 + 0x1E);
        *(u16 *)(*(u8 **)((u8 *)arg1 + 0x88) + 0x8) = *(u16 *)((u8 *)arg0 + 0xC);
        *(u16 *)(*(u8 **)((u8 *)arg1 + 0x8C) + 0x8) = *(u16 *)((u8 *)arg0 + 0x14);
        *(u16 *)(*(u8 **)((u8 *)arg1 + 0x90) + 0x8) = *(u16 *)((u8 *)arg0 + 0x1C);

        RCpolyFT3(arg0, D_8008ACD0);
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/code_8220_c", func_800199EC);
#endif

#ifdef NON_MATCHING
/* NON_MATCHING: 48/56 words, length exact. Residue: register identity
 * ($a2 vs $a1 for the OT high-byte mask, cascading register renames)
 * plus a missing load-delay-slot filler `addiu $v0,$s1,0x18`
 * (docs/match-reports/func_80019B24.md). Hand-derived. */
void func_80019B24(void *arg0, void *arg1) {
    if (*(s32 *)((u8 *)arg1 + 0x78) == 0) {
        ((OtTag *)arg0)->addr = (*(OtTag **)((u8 *)arg1 + 0x30))->addr;
        (*(OtTag **)((u8 *)arg1 + 0x30))->addr = (u32)arg0;
    } else {
        func_8001A380(D_8008AEE8, arg1, (u8 *)arg0 + 0x4, 0, 0, 0);
        func_8001A4C0((u8 *)arg1 + 0x94, (u8 *)arg1 + 0xA4, (u8 *)arg0 + 0x8,
                      (u8 *)arg0 + 0xC, (u8 *)arg0 + 0x10, (u8 *)arg0 + 0x14);
        RCpolyF4(arg0, D_8008AEE8);
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/code_8220_c", func_80019B24);
#endif

/* A 2-s16 pair (alignment 2, not 4) -- see func_8001A268's stall report for
 * why this is needed even at accidentally-4-aligned offsets. */
typedef struct {
    s16 x, y;
} Vec2s16_C04;

#ifdef NON_MATCHING
/* NON_MATCHING: 88/96 words, length exact. Residue: register identity
 * ($a2 vs $a1 for the OT mask) plus a missing load-delay-slot filler
 * `addiu $v0,$s1,0x24` (docs/match-reports/func_80019C04.md).
 * Hand-derived. */
void func_80019C04(void *arg0, void *arg1) {
    u8 *self = (u8 *)arg0;
    u8 *prim = (u8 *)arg1;

    if (*(s32 *)(prim + 0x78) == 0) {
        ((OtTag *)self)->addr = (*(OtTag **)(prim + 0x30))->addr;
        (*(OtTag **)(prim + 0x30))->addr = (u32)self;
    } else {
        func_8001A380(D_8008AEE8, prim, self + 0x4, 0, 0, 0);
        func_8001A4C0(prim + 0x94, prim + 0xA4, self + 0x8, self + 0x10,
                      self + 0x18, self + 0x20);

        *(u16 *)(*(u8 **)(prim + 0x94) + 0xA) = *(u8 *)(self + 0xF);
        *(u16 *)(*(u8 **)(prim + 0x98) + 0xA) = *(u8 *)(self + 0xF);
        *(u16 *)(*(u8 **)(prim + 0x9C) + 0xA) = *(u8 *)(self + 0x17);
        *(u16 *)(*(u8 **)(prim + 0xA0) + 0xA) = *(u8 *)(self + 0x1F);

        *(Vec2s16_C04 *)(*(u8 **)(prim + 0x94) + 0xC) = *(Vec2s16_C04 *)(self + 0x4);
        *(Vec2s16_C04 *)(*(u8 **)(prim + 0x98) + 0xC) = *(Vec2s16_C04 *)(self + 0xC);
        *(Vec2s16_C04 *)(*(u8 **)(prim + 0x9C) + 0xC) = *(Vec2s16_C04 *)(self + 0x14);
        *(Vec2s16_C04 *)(*(u8 **)(prim + 0xA0) + 0xC) = *(Vec2s16_C04 *)(self + 0x1C);

        RCpolyG4(self, D_8008AEE8);
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/code_8220_c", func_80019C04);
#endif

#ifdef NON_MATCHING
/* NON_MATCHING: 80/88 words, length exact. Residue: register identity
 * ($a2 vs $a1 for the OT high-byte mask) plus a missing load-delay-slot
 * filler `addiu $v0,$s1,0x28` = align_up_4(last touched self field +
 * width) (docs/match-reports/func_80019D84.md). Hand-derived. */
void func_80019D84(void *arg0, void *arg1) {
    if (*(s32 *)((u8 *)arg1 + 0x78) == 0) {
        ((OtTag *)arg0)->addr = (*(OtTag **)((u8 *)arg1 + 0x30))->addr;
        (*(OtTag **)((u8 *)arg1 + 0x30))->addr = (u32)arg0;
    } else {
        func_8001A380(D_8008AEE8, arg1, (u8 *)arg0 + 0x4, 1, *(u16 *)((u8 *)arg0 + 0xE), *(u16 *)((u8 *)arg0 + 0x16));
        func_8001A4C0((u8 *)arg1 + 0x94, (u8 *)arg1 + 0xA4, (u8 *)arg0 + 0x8,
                      (u8 *)arg0 + 0x10, (u8 *)arg0 + 0x18, (u8 *)arg0 + 0x20);

        *(u16 *)(*(u8 **)((u8 *)arg1 + 0x94) + 0xA) = *(u16 *)((u8 *)arg0 + 0x1E);
        *(u16 *)(*(u8 **)((u8 *)arg1 + 0x98) + 0xA) = *(u16 *)((u8 *)arg0 + 0x1E);
        *(u16 *)(*(u8 **)((u8 *)arg1 + 0x9C) + 0xA) = *(u16 *)((u8 *)arg0 + 0x1E);
        *(u16 *)(*(u8 **)((u8 *)arg1 + 0xA0) + 0xA) = *(u16 *)((u8 *)arg0 + 0x1E);
        *(u16 *)(*(u8 **)((u8 *)arg1 + 0x94) + 0x8) = *(u16 *)((u8 *)arg0 + 0xC);
        *(u16 *)(*(u8 **)((u8 *)arg1 + 0x98) + 0x8) = *(u16 *)((u8 *)arg0 + 0x14);
        *(u16 *)(*(u8 **)((u8 *)arg1 + 0x9C) + 0x8) = *(u16 *)((u8 *)arg0 + 0x1C);
        *(u16 *)(*(u8 **)((u8 *)arg1 + 0xA0) + 0x8) = *(u16 *)((u8 *)arg0 + 0x24);

        RCpolyFT4(arg0, D_8008AEE8);
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/code_8220_c", func_80019D84);
#endif

/* A 2-s16 pair (alignment 2, not 4) -- see func_8001A268's stall report for
 * why this is needed even at accidentally-4-aligned offsets. */
typedef struct {
    s16 x, y;
} Vec2s16_EE4;

#ifdef NON_MATCHING
/* NON_MATCHING: 88/96 words, length exact. Residue: register identity
 * ($a2 vs $a1 for the OT mask) plus a missing load-delay-slot filler
 * `addiu $v0,$s1,0x28` = align_up_4(last touched self field + width)
 * (docs/match-reports/func_80019EE4.md). Hand-derived. */
void func_80019EE4(void *arg0, void *arg1) {
    u8 *self = (u8 *)arg0;
    u8 *prim = (u8 *)arg1;

    if (*(s32 *)(prim + 0x78) == 0) {
        ((OtTag *)self)->addr = (*(OtTag **)(prim + 0x30))->addr;
        (*(OtTag **)(prim + 0x30))->addr = (u32)self;
    } else {
        func_8001A380(D_8008ACD0, prim, self + 0x4, 1, *(u16 *)(self + 0xE), *(u16 *)(self + 0x1A));
        func_8001A3EC((PolyVtx **)(prim + 0x88), (PolyVtx **)(prim + 0xA4),
                      (PolyUV4 *)(self + 0x8), (PolyUV4 *)(self + 0x14),
                      (PolyUV4 *)(self + 0x20));

        *(u16 *)(*(u8 **)(prim + 0x88) + 0xA) = *(u16 *)(self + 0x26);
        *(u16 *)(*(u8 **)(prim + 0x8C) + 0xA) = *(u16 *)(self + 0x26);
        *(u16 *)(*(u8 **)(prim + 0x90) + 0xA) = *(u16 *)(self + 0x26);

        *(Vec2s16_EE4 *)(*(u8 **)(prim + 0x88) + 0xC) = *(Vec2s16_EE4 *)(self + 0x4);
        *(Vec2s16_EE4 *)(*(u8 **)(prim + 0x8C) + 0xC) = *(Vec2s16_EE4 *)(self + 0x10);
        *(Vec2s16_EE4 *)(*(u8 **)(prim + 0x90) + 0xC) = *(Vec2s16_EE4 *)(self + 0x1C);

        *(u16 *)(*(u8 **)(prim + 0x88) + 0x8) = *(u16 *)(self + 0xC);
        *(u16 *)(*(u8 **)(prim + 0x8C) + 0x8) = *(u16 *)(self + 0x18);
        *(u16 *)(*(u8 **)(prim + 0x90) + 0x8) = *(u16 *)(self + 0x24);

        RCpolyGT3(self, D_8008ACD0);
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/code_8220_c", func_80019EE4);
#endif

/* A 2-s16 pair (alignment 2, not 4) -- see func_8001A268's stall report for
 * why this is needed even at accidentally-4-aligned offsets. */
typedef struct {
    s16 x, y;
} Vec2s16_A64;

#ifdef NON_MATCHING
/* NON_MATCHING: 104/112 words, length exact. Residue: register identity
 * ($a2 vs $a1 for the OT mask) plus a missing load-delay-slot filler
 * `addiu $v0,$s1,0x34` = align_up_4(last touched self field + width)
 * (docs/match-reports/func_8001A064.md). Hand-derived. */
void func_8001A064(void *arg0, void *arg1) {
    u8 *self = (u8 *)arg0;
    u8 *prim = (u8 *)arg1;

    if (*(s32 *)(prim + 0x78) == 0) {
        ((OtTag *)self)->addr = (*(OtTag **)(prim + 0x30))->addr;
        (*(OtTag **)(prim + 0x30))->addr = (u32)self;
    } else {
        func_8001A380(D_8008AEE8, prim, self + 0x4, 1, *(u16 *)(self + 0xE), *(u16 *)(self + 0x1A));
        func_8001A4C0(prim + 0x94, prim + 0xA4, self + 0x8, self + 0x14,
                      self + 0x20, self + 0x2C);

        *(u16 *)(*(u8 **)(prim + 0x94) + 0xA) = *(u16 *)(self + 0x26);
        *(u16 *)(*(u8 **)(prim + 0x98) + 0xA) = *(u16 *)(self + 0x26);
        *(u16 *)(*(u8 **)(prim + 0x9C) + 0xA) = *(u16 *)(self + 0x32);
        *(u16 *)(*(u8 **)(prim + 0xA0) + 0xA) = *(u16 *)(self + 0x32);

        *(Vec2s16_A64 *)(*(u8 **)(prim + 0x94) + 0xC) = *(Vec2s16_A64 *)(self + 0x4);
        *(Vec2s16_A64 *)(*(u8 **)(prim + 0x98) + 0xC) = *(Vec2s16_A64 *)(self + 0x10);
        *(Vec2s16_A64 *)(*(u8 **)(prim + 0x9C) + 0xC) = *(Vec2s16_A64 *)(self + 0x1C);
        *(Vec2s16_A64 *)(*(u8 **)(prim + 0xA0) + 0xC) = *(Vec2s16_A64 *)(self + 0x28);

        *(u16 *)(*(u8 **)(prim + 0x94) + 0x8) = *(u16 *)(self + 0xC);
        *(u16 *)(*(u8 **)(prim + 0x98) + 0x8) = *(u16 *)(self + 0x18);
        *(u16 *)(*(u8 **)(prim + 0x9C) + 0x8) = *(u16 *)(self + 0x24);
        *(u16 *)(*(u8 **)(prim + 0xA0) + 0x8) = *(u16 *)(self + 0x30);

        RCpolyGT4(self, D_8008AEE8);
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/code_8220_c", func_8001A064);
#endif

void func_8001A224(void *arg0, void *arg1, s32 kind)
{
    u8 *src = (u8 *)arg1 + 0x18;
    u8 *dst0 = (u8 *)arg0;
    u8 *dst1 = (kind == 4) ? (u8 *)arg1 + 0xF0 : (u8 *)arg1 + 0xA8;

    while (kind-- > 0) {
        *(void **)dst1 = src;
        *(void **)dst0 = src;
        src += 0x18;
        dst1 += 4;
        dst0 += 4;
    }
}

#ifdef NON_MATCHING
/* NON_MATCHING: 53/70 words, length exact. Residue: retail's
 * `addiu $sp,$sp,-0x20` frame adjustment is scheduled into the loop-skip
 * branch's delay slot (word 17) instead of the ordinary prologue position
 * (word 0); a pure placement difference with no data dependency, not
 * register identity (docs/match-reports/func_8001A268.md). Hand-derived. */
/* A 2-s16 pair (alignment 2, not 4) -- forces the unaligned lwl/lwr whole-
 * struct copy retail uses for prim->0x60 -> prim->0x74 -> prim->0x70 even
 * though those particular offsets are accidentally 4-aligned; the compiler
 * only knows the DECLARED alignment of the type, not the runtime address.
 * Same idiom as FlashbackRotation (include/DreamSys.h) and func_8004B38C. */
typedef struct {
    s16 x, y;
} Vec2s16_268;

void func_8001A268(void *arg0, s32 count)
{
    u8 *self = (u8 *)arg0;
    s16 *xp, *yp, *end;

    *(Vec2s16_268 *)(self + 0x74) = *(Vec2s16_268 *)(self + 0x60);
    *(Vec2s16_268 *)(self + 0x70) = *(Vec2s16_268 *)(self + 0x74);

    xp = (s16 *)(self + 0x64);
    yp = (s16 *)(self + 0x66);
    end = (s16 *)(self + 0x5C + (count << 2));

    for (; xp < end; xp += 2, yp += 2) {
        if (*xp < *(s16 *)(self + 0x70)) {
            *(s16 *)(self + 0x70) = *xp;
        }
        if (*yp < *(s16 *)(self + 0x72)) {
            *(s16 *)(self + 0x72) = *yp;
        }
        if (*(s16 *)(self + 0x74) < *xp) {
            *(s16 *)(self + 0x74) = *xp;
        }
        if (*(s16 *)(self + 0x76) < *yp) {
            *(s16 *)(self + 0x76) = *yp;
        }
    }

    if (*(s16 *)(self + 0x74) - *(s16 *)(self + 0x70) >= 0x101) {
        *(s32 *)(self + 0x78) = 1;
    }
    if (*(s16 *)(self + 0x76) - *(s16 *)(self + 0x72) >= 0x101) {
        *(s32 *)(self + 0x78) = 1;
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/code_8220_c", func_8001A268);
#endif

extern s32 D_8008A824;
extern s32 D_8008A828;
extern s32 D_80090C18;
extern s32 D_8008A830;
extern s32 D_8008A834;

void func_8001A380(void *arg0, void *arg1, PolyUV4 *arg2, s32 arg3, u16 arg4, u16 arg5)
{
    u8 *dst = (u8 *)arg0;
    u8 *prim = (u8 *)arg1;
    s32 val;
    s32 code;
    s32 code2;

    if (D_8008A830) {
        val = D_8008A834;
    } else {
        val = D_80090C18;
    }
    code = D_8008A824;
    code2 = D_8008A828;

    *(s32 *)dst = val;
    *(s32 *)(dst + 0x4) = code;
    *(s32 *)(dst + 0x8) = code2;

    if (arg3 != 0) {
        *(u16 *)(dst + 0xC) = arg4;
        *(u16 *)(dst + 0xE) = arg5;
    }

    *(PolyUV4 *)(dst + 0x10) = *arg2;
    *(s32 *)(dst + 0x14) = *(s32 *)(prim + 0x30);
}

/*
 * Copies three unaligned 8-byte fields (src[i]->xy -> dst[i]->xy) and three
 * unaligned 4-byte fields (*uv0/*uv1/*uv2 -> dst[i]->uv). Retail does each
 * with lwl/lwr + swl/swr, and the idiom that reproduces that is the struct
 * types themselves: PolyXY8 and PolyUV4 are ALL-s16, so their alignment is
 * 2, and a whole-struct assignment of an alignment-2 type is what GCC 2.6.3
 * emits as the unaligned pair. One stray s32 member and the copy becomes
 * aligned lw/sw and stops matching (DECOMPILATION_LEARNINGS, "A struct
 * whose members are all s8/s16 has alignment 2"). Round 13 first matched
 * this as a whole-function __asm__ transcription; the head reworked it into
 * these six assignments, byte-exact, and CLAUDE.md HARD RULE 6 cites it as
 * the example of "hard to type" not being "no C form".
 */
void func_8001A3EC(PolyVtx **dst, PolyVtx **src, PolyUV4 *uv0, PolyUV4 *uv1,
                   PolyUV4 *uv2) {
    dst[0]->xy = src[0]->xy;
    dst[1]->xy = src[1]->xy;
    dst[2]->xy = src[2]->xy;
    dst[0]->uv = *uv0;
    dst[1]->uv = *uv1;
    dst[2]->uv = *uv2;
}


void func_8001A4C0(PolyVtx **dst, PolyVtx **src, PolyUV4 *uv0, PolyUV4 *uv1,
                   PolyUV4 *uv2, PolyUV4 *uv3) {
    func_8001A3EC(dst, src, uv0, uv1, uv2);
    dst[3]->xy = src[3]->xy;
    dst[3]->uv = *uv3;
}

extern s32 D_8008A830;
extern s32 D_8008A834;

void func_8001A54C(s32 arg0, s32 arg1)
{
    D_8008A830 = arg0;
    if (arg0) {
        D_8008A834 = arg1;
    }
}
