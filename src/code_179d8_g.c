/*
 * code_179d8_g -- functions 83..99 of the original 274-function code_179d8
 * monolith, 0x1AB78..0x1C440 (vram 0x8002A378..0x8002BC40).  Carved round 17
 * (2026-09-04) off the back of what was then `code_179d8_mid`, the three
 * functions in front of this slice. Those three were left as "all addiu-$at
 * blocked, so there is nothing left to staff there"; re-censused round 24
 * (2026-09-08) that is FALSE -- func_800299BC (161w), func_80029C40 (180w)
 * and func_80029F10 (282w) are ALL THREE blocker-clean now that `addiu_at`
 * is resolved. Round 26 (2026-09-09) acted on that and CARVED them as the
 * C unit `code_179d8_n`; no `code_179d8_mid` segment exists any more.
 *
 * Blocker census, three-grep screen run per function at carve time:
 * 14 of the 17 clean, zero trivial leaves.  These are BIG bodies -- 196,
 * 223, 186 and 189 instructions among them -- so this unit is smaller in
 * count and considerably larger in work than 17 suggests.  Budget fewer
 * functions per pass here than in a leaf-heavy unit.
 *
 * NO LONGER BLOCKED -- all three of this unit's blocked functions were
 * blocked on `addiu_at` ALONE, and `addiu_at` was RESOLVED in round 21
 * (maspsx `--addiu-at`; docs/research/addiu-at-blocker.md). Re-screened with
 * `python3 tools/nearmiss.py` on 2026-09-08 (round 24):
 *   func_8002AEE0 (174w)  func_8002B640 (186w)  func_8002B94C (189w)
 * The previous version of this comment read "BLOCKED, stub reports already
 * filed, do NOT spend attempts on these" -- a stale DIRECTIVE over free
 * ground.
 *
 * ROUND 32 (2026-09-12) CORRECTION -- that reopening WORKED, and the
 * "FRESH and assignable / their stub reports are already gone" wording it
 * left behind is now stale in the OPPOSITE direction. All three have since
 * been attempted and all three carry full worked stall reports (174/174
 * length-exact at 153 words; 3 words short; 2 words long respectively).
 * They are near-misses, NOT cold ground: read
 * docs/match-reports/<func>.md before spending an attempt, or you will
 * re-derive several hundred lines of someone else's derivation. Verified
 * by `tools/nearmiss.py` and by the presence of the report files, not by
 * reading this comment.
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
extern s32 D_8006D620[4];      /* read-only here; formerly noted BLOCKED (addiu_at) in
                                 * func_8002AEE0 -- addiu_at was RESOLVED round 21, dead cause */
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
extern u8 D_80010AD8[];
extern u8 D_80010984[];
extern u8 D_80010994[];
extern u8 D_80010A40[];
extern u8 D_80010A50[];
extern u8 D_80010A94[];
extern u8 D_80010AA0[];
extern u8 D_80010AAC[];
extern u8 D_80010ABC[];
extern u8 D_8006D908[];
extern u8 D_8006D90C[];
extern volatile s32 *D_8006D924;
extern volatile s32 *D_8006D928;
extern volatile s32 *D_8006D92C;
extern volatile s32 *D_8006D930;
extern volatile s32 *D_8006D934;

/* Still INCLUDE_ASM elsewhere -- not this unit's to carve. */
extern void ResetCallback(void);                              /* lib/libetc/intr.o */
extern void (*InterruptCallback(s32 arg0, void (*callback)(void)))(void); /* lib/libetc/intr.o, per code_179d8_c.c */
extern s32 VSync(s32 arg0);                            /* asm/psyq_15d04.s */
extern void puts(const char *arg0);                   /* asm/psyq_15d04.s */
extern void printf(const char *fmt, ...);                /* Psy-Q printf wrapper */
extern s32 func_80029F10(s32 arg0, s32 arg1, s32 arg2, s32 arg3); /* defined in code_179d8_n */
extern s32 func_800299BC(s32 arg0, s32 arg1);                   /* defined in code_179d8_n, per code_179d8_b.c */
extern s32 func_80029478(void);                                /* asm/nonmatchings/code_179d8_b;
                                                                   addiu_at RESOLVED round 21, no longer blocked */
extern s32 func_8002C0AC(const char *arg0, const char *arg1, s32 arg2); /* asm/nonmatchings/code_179d8_d */
extern s32 CheckCallback(void);                                /* lib/libetc/intr.o -- trivial
                                                                   (u16)D_8006C272 getter */
extern s32 func_8002BC40(s32 id, char *name);                  /* code_179d8_d.c, matched: looks up
                                                                   `name` under directory id `id` in
                                                                   D_8008B9F4[], returns 1-based index
                                                                   or -1 */
extern s32 func_8002BCEC(s32 id);                               /* asm/nonmatchings/code_179d8_d, still
                                                                   INCLUDE_ASM there; signature inferred
                                                                   from this function's call site only */

/* CdSearchFile diagnostics (confirmed via asm/data/120C.rodata.s) */
extern u8 D_80010AEC[];   /* "%s: path level (%d) error\n" */
extern u8 D_80010B08[];   /* "%s: dir was not found\n" */
extern u8 D_80010B20[];   /* "CdSearchFile: disc error\n" */
extern u8 D_80010B3C[];   /* "CdSearchFile: searching %s...\n" */
extern u8 D_80010B5C[];   /* "%s:  found\n" */
extern u8 D_80010B68[];   /* "%s: not found\n" */

extern s32 D_8006D93C;
extern s32 D_8006D608;
extern u8 D_8008B3F8[];   /* CD directory-entry table; the copy in func_8002B640 reads from
                            * (D_8008B3F8 - 8) for the matched record while the flag byte and
                            * name compare read D_8008B3F8 itself at the same stride (0x18) --
                            * see that function's report for why the two views are offset. */

extern s32 func_8002BFA8(void *p0, void *p1, void *p2);        /* code_179d8_d.c, matched */
extern void func_8002C014(char *dest, char *src, s32 count);   /* code_179d8_d.c, matched */
extern u8 D_8008CFF0[];   /* PVD (primary volume descriptor) read buffer, this function's own scratch */
extern u8 D_8008D07C[];   /* byte-packed field inside the PVD -- root dir record, read unaligned */
extern s32 D_8006D938;

/* This unit's own local view of code_179d8_d.c's Entry8008B9F4 (id/unk4/name,
 * stride 0x2C) -- kept LOCAL per the project's multiple-independent-local-
 * views convention, not shared via a header. func_8002BC40 (code_179d8_d.c)
 * only ever READS `.id`; this function is the one that WRITES `.id`/`.unk4`/
 * `.name` in the first place, from CD directory-record data. */
typedef struct {
    s32 id;
    s32 unk4;
    char name[0x24];
} Entry8008B9F4View;
extern Entry8008B9F4View D_8008B9F4[0x80];
extern s32 D_8008B9F0[];  /* a SEPARATE table immediately before D_8008B9F4, same 0x2C stride --
                            * NOT part of Entry8008B9F4 (func_8002BC40's established, matched
                            * reading puts the struct's own first field, `.id`, AT D_8008B9F4
                            * itself); holds this function's own 1-based record ordinal. */
extern u8 D_8008D7F0[];   /* address-only use here (an upper-bound check on the scan cursor);
                            * code_179d8_j.c/_m.c have their own typed local views of this
                            * symbol for their own purposes -- not shared, per convention. */

/* A 4-byte, alignment-1 view used only to force the unaligned lwl/lwr +
 * swl/swr load/store shape this function's two misaligned-word accesses
 * need -- same idiom CLAUDE.md documents for an all-byte struct whose
 * computed alignment is less than a word's. */
typedef struct {
    u8 b0, b1, b2, b3;
} UWord;

/* CD_newmedia diagnostics (confirmed via asm/data/120C.rodata.s) */
extern u8 D_80010B78[];   /* "CD_newmedia: Read error in cd_read(PVD)\n" */
extern u8 D_80010BA4[];   /* "CD001" -- ISO9660 standard identifier */
extern u8 D_80010BAC[];   /* "CD_newmedia: Disc format error in cd_read(PVD)\n" */
extern u8 D_80010BDC[];   /* "CD_newmedia: Read error (PT:%08x)\n" */
extern u8 D_80010C00[];   /* "CD_newmedia: sarching dir..\n" [sic] */
extern u8 D_80010C20[];   /* "\t%08x,%04x,%04x,%s\n" */
extern u8 D_80010C34[];   /* "CD_newmedia: %d dir entries found\n" */

/* Still INCLUDE_ASM in THIS unit (not yet converted) -- INCLUDE_ASM leaves no
 * C-level prototype of its own, so callers within this file need one. */
extern s32 func_8002AA6C(void);
extern s32 func_8002B198(s32 arg0);
extern s32 func_8002B94C(void);

/* Forward declarations: taken by address before their own ROM-order definition
 * further down this file (func_8002A6EC/func_8002A75C hand func_8002B3F4 to
 * InterruptCallback as a thread entry; func_8002AA6C hands func_8002B4D4 to
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
                puts(D_80010A40);
            }
            counter++;
            func_80029F10(1, 0, 0, 0);
        }

        while (func_80029F10(0x16, D_8006D908, 0, 0)) {
            func_80029F10(1, 0, 0, 0);
            puts(D_80010A50);
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
    ResetCallback();
    InterruptCallback(2, func_8002B3F4);
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
