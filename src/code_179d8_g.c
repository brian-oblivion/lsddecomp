/*
 * code_179d8_g -- functions 83..99 of the original 274-function code_179d8
 * monolith, 0x1AB78..0x1C440 (vram 0x8002A378..0x8002BC40).  Carved round 17
 * (2026-09-04) off the back of `code_179d8_mid`, which keeps that name for
 * the three functions still in front of this slice (all three addiu-$at
 * blocked, so there is nothing left to staff there).
 *
 * Blocker census, three-grep screen run per function at carve time:
 * 14 of the 17 clean, zero trivial leaves.  These are BIG bodies -- 196,
 * 223, 186 and 189 instructions among them -- so this unit is smaller in
 * count and considerably larger in work than 17 suggests.  Budget fewer
 * functions per pass here than in a leaf-heavy unit.
 *
 * BLOCKED, stub reports already filed, do NOT spend attempts on these:
 *   addiu_at: func_8002AEE0 (174 insn), func_8002B640 (186 insn),
 *             func_8002B94C (189 insn)
 *
 * Owns NO switch jump table -- zero `jtbl_` references in the slice -- so no
 * rodata sub-slot is attached to this unit.
 */
#include "common.h"

/* code_179d8_g -- this window's globals continue code_179d8_b's reading:
 * plain scalar/pointer driver state, not object fields (no classtable.py
 * hit near D_8006D5FC..D_8006D934). This unit's own extern declarations,
 * kept local per the project's multiple-independent-local-views convention
 * -- see code_179d8_b.c's header comment for why no shared header. */

extern s32 D_8006D8A4;
extern s32 D_8006D5FC;
extern s32 D_8006D600;
extern s32 D_8006D604;
extern s32 D_8006D60C;
extern s32 D_8006D610;
extern s32 D_8006D614;
extern u8 D_8006D618;
extern u8 D_8006D619;
extern u8 D_8006D61A;
extern u8 D_8006D61C;
extern u8 D_8006D61D;
extern s32 D_8006D620[4];      /* BLOCKED in func_8002AEE0 (addiu_at); read-only here */
extern s32 D_8006D6A0[];       /* lookup table, indexed by a byte field << 2 */

extern volatile u8 *D_8006D8C0;
extern volatile u8 *D_8006D8C4;
extern volatile u8 *D_8006D8C8;
extern volatile u8 *D_8006D8CC;
extern volatile s32 *D_8006D8D0;
extern volatile u16 *D_8006D8D4;   /* HW register block; offsets are byte offsets */
extern u8 D_8006D8D8[2];
extern u8 D_8006D8D9;
extern volatile u8 D_8006D8DA;
extern s32 D_8006D8DC;   /* first of 10 consecutive words zeroed by a pointer walk;
                          * D_8006D8E0..D_8006D900 are the other nine, each
                          * already individually named -- not a real array. */
extern s32 D_8006D8E0;
extern s32 D_8006D8E4;
extern s32 D_8006D8E8;
extern volatile s32 D_8006D8EC;
extern volatile s32 D_8006D8F0;
extern volatile s32 D_8006D8F4;
extern s32 D_8006D8F8;
extern s32 D_8006D8FC;
extern s32 D_8006D900;
extern s32 D_8006D904;
extern u8 D_8008B3CC[];
extern u8 D_8008B3D4[];
extern s32 D_8008B3E4;
extern s32 D_8008B3E8;
extern s32 D_8008B3EC;
extern u8 D_80010AE0[];
extern u8 D_80010984[];
extern u8 D_80010994[];
extern u8 D_80010A40[];
extern u8 D_80010A50[];
extern u8 D_80010A94[];
extern u8 D_80010AA0[];
extern u8 D_8006D908[];
extern u8 D_8006D90C[];
extern volatile s32 *D_8006D924;
extern volatile s32 *D_8006D928;
extern volatile s32 *D_8006D92C;
extern volatile s32 *D_8006D930;
extern volatile s32 *D_8006D934;

/* Still INCLUDE_ASM elsewhere -- not this unit's to carve. */
extern void func_80024D10(void);                              /* asm/psyq_GsLinkObject4.s */
extern void (*func_80024D40(s32 arg0, void (*callback)(void)))(void); /* asm/psyq_GsLinkObject4.s, per code_179d8_c.c */
extern s32 func_80025900(s32 arg0);                            /* asm/psyq_15d04.s */
extern void func_80025AE4(const char *arg0);                   /* asm/psyq_15d04.s */
extern void func_80012C20(const char *fmt, ...);                /* Psy-Q printf wrapper */
extern s32 func_80029F10(s32 arg0, s32 arg1, s32 arg2, s32 arg3); /* asm/code_179d8_mid.s */
extern s32 func_800299BC(s32 arg0, s32 arg1);                   /* asm/code_179d8_mid.s, per code_179d8_b.c */
extern s32 func_80029478(void);                                /* asm/nonmatchings/code_179d8_b, BLOCKED (addiu_at) */
extern s32 func_8002C0AC(const char *arg0, const char *arg1, s32 arg2); /* asm/nonmatchings/code_179d8_d */

/* Still INCLUDE_ASM in THIS unit (not yet converted) -- INCLUDE_ASM leaves no
 * C-level prototype of its own, so callers within this file need one. */
extern s32 func_8002AA6C(void);

/* Forward declarations: taken by address before their own ROM-order definition
 * further down this file (func_8002A6EC/func_8002A75C hand func_8002B3F4 to
 * func_80024D40 as a thread entry; func_8002AA6C hands func_8002B4D4 to
 * D_8006D600 as a callback). */
void func_8002B3F4(void);
void func_8002B4D4(s32 arg0, s32 arg1);

s32 func_8002A378(u8 *arg0)
{
    *D_8006D8C0 = 2;
    *D_8006D8C8 = arg0[0];
    *D_8006D8CC = arg0[1];
    *D_8006D8C0 = 3;
    *D_8006D8C4 = arg0[2];
    *D_8006D8C8 = arg0[3];
    *D_8006D8CC = 0x20;
    return 0;
}

void func_8002A400(void)
{
    s32 saved;
    s32 counter = 0;

    if (D_8006D904 < D_8006D614) {
        saved = D_8006D5FC;
        D_8006D5FC = 0;

        while (D_8006D60C & 0x10) {
            if ((u8)counter == 0) {
                func_80025AE4(D_80010A40);
            }
            counter++;
            func_80029F10(1, 0, 0, 0);
        }

        while (func_80029F10(0x16, D_8006D908, 0, 0)) {
            func_80029F10(1, 0, 0, 0);
            func_80025AE4(D_80010A50);
        }

        D_8006D5FC = saved;
        D_8006D904 = D_8006D614;
    }
}

void func_8002A510(void)
{
    volatile u8 *q;

    *D_8006D8C0 = 1;
    while (*D_8006D8CC & 7) {
        *D_8006D8C0 = 1;
        *D_8006D8CC = 7;
        *D_8006D8C8 = 7;
    }

    D_8006D8DA = 0;
    q = &D_8006D8D9;
    D_8006D61C = 0;
    *q = D_8006D8DA;
    __asm__("");
    D_8006D8D8[0] = 2;
    *D_8006D8C0 = 0;
    *D_8006D8CC = 0;
    *D_8006D8D0 = 0x1325;
}

s32 func_8002A5F8(void)
{
    u8 buf[4];

    if (D_8006D8D4[0xDC] == 0 && D_8006D8D4[0xDD] == 0) {
        D_8006D8D4[0xC0] = 0x3FFF;
        D_8006D8D4[0xC1] = 0x3FFF;
    }
    D_8006D8D4[0xD8] = 0x3FFF;
    D_8006D8D4[0xD9] = 0x3FFF;
    D_8006D8D4[0xD5] = 0xC001;

    buf[2] = 0x80;
    buf[0] = 0x80;
    buf[3] = 0;
    buf[1] = 0;
    *D_8006D8C0 = 2;
    *D_8006D8C8 = buf[0];
    *D_8006D8CC = buf[1];
    *D_8006D8C0 = 3;
    *D_8006D8C4 = buf[2];
    *D_8006D8C8 = buf[3];
    *D_8006D8CC = 0x20;
    return 0;
}

void func_8002A6EC(void)
{
    s32 *p;
    s32 i;

    D_8006D600 = 0;
    D_8006D5FC = 0;
    D_8006D610 = 0;
    D_8006D60C = 0;
    p = &D_8006D8DC;
    for (i = 9; i != -1; i--) {
        *p = 0;
        p++;
    }
    func_80024D10();
    func_80024D40(2, func_8002B3F4);
}

INCLUDE_ASM("asm/nonmatchings/code_179d8_g", func_8002A75C);

INCLUDE_ASM("asm/nonmatchings/code_179d8_g", func_8002AA6C);

s32 func_8002ADE8(s32 arg0, s32 arg1, s32 arg2)
{
    s32 t;
    volatile s32 *p;

    p = &D_8006D8EC;
    *p = arg2;
    t = *p & 0x30;

    /* Retail keeps all three D_8006D8F0 stores as separate, unmerged blocks
     * (three distinct address computations -- two folded through $at, one
     * unfolded through a real GPR) instead of the single shared store GCC's
     * cross-jump/tail-merge pass produces from the equivalent if/else-if/else
     * or switch. The barrier on case1 and the local `volatile s32 *` pointers
     * on case3/case2's-neighbour below are what keeps each store distinct
     * enough that the merge heuristic can't unify them -- removing any one
     * of the three re-merges a pair and drops 4-24 bytes. This is a
     * scheduling/block-identity lever (order/selection), not a register-
     * identity fix: no operand constraint pins a register here. */
    if (t == 0) {
        goto case1;
    }
    if (t == 0x20) {
        goto case2;
    }
    goto case3;
case1:
    D_8006D8F0 = 0x200;
    __asm__("");
    goto join;
case2:
    D_8006D8F0 = 0x249;
    goto join;
case3:
    {
        volatile s32 *q3 = &D_8006D8F0;
        *q3 = 0x246;
    }
join:

    {
        volatile s32 *q4 = &D_8006D8E4;
        *q4 = arg0;
    }
    D_8006D8E0 = arg1;
    D_8006D8DC = 8;
    D_8006D8FC = D_8006D5FC;
    D_8006D900 = D_8006D600;

    if (D_8006D60C & 0xE0) {
        func_80029F10(9, 0, 0, 0);
    }
    func_800299BC(0, 0);
    return -(func_8002AA6C() < 1);
}

INCLUDE_ASM("asm/nonmatchings/code_179d8_g", func_8002AEE0);

INCLUDE_ASM("asm/nonmatchings/code_179d8_g", func_8002B198);

s32 func_8002B304(s32 arg0, s32 arg1)
{
    *D_8006D8C0 = 0;
    *D_8006D8CC = 0x80;
    *D_8006D924 = 0x20943;
    *D_8006D8D0 = 0x1323;
    *D_8006D928 |= 0x8000;
    *D_8006D92C = arg0;
    *D_8006D930 = arg1 | 0x10000;
    *D_8006D934 = 0x11000000;

    while (*D_8006D934 & 0x1000000) {
    }

    *D_8006D8D0 = 0x1325;
    return 0;
}

void func_8002B3E4(s32 arg0)
{
    D_8006D8A4 = arg0;
}

INCLUDE_ASM("asm/nonmatchings/code_179d8_g", func_8002B3F4);

INCLUDE_ASM("asm/nonmatchings/code_179d8_g", func_8002B4D4);

INCLUDE_ASM("asm/nonmatchings/code_179d8_g", func_8002B640);

s32 func_8002B928(const char *arg0, const char *arg1)
{
    return func_8002C0AC(arg0, arg1, 0xC) == 0;
}

INCLUDE_ASM("asm/nonmatchings/code_179d8_g", func_8002B94C);
