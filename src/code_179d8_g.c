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
 * code_179d8_g -- functions 83..99 of the original 274-function code_179d8
 * monolith, 0x1AB78..0x1C440 (vram 0x8002A378..0x8002BC40).  Carved round 17
 * (2026-09-04) off the back of what was then `code_179d8_mid`, the three
 * functions in front of this slice. Those three were left as "all addiu-$at
 * blocked, so there is nothing left to staff there"; re-censused round 24
 * (2026-09-08) that is FALSE -- CD_sync (161w), CD_ready (180w)
 * and CD_cw (282w) are ALL THREE blocker-clean now that `addiu_at`
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
 *   CD_readsync (174w)  func_8002B640 (186w)  func_8002B94C (189w)
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
 * AND A SECOND ROUND-32 CORRECTION, made the same day as the one above:
 * func_8002B640 and func_8002B94C are NOT GAME CODE AT ALL. Both lie fully
 * inside `libcd/iso9660.o` (Psy-Q 3.3), an object already placed in
 * config/psyq-objects.txt and verified against retail. No C matches them;
 * the correct disposition is conversion per docs/SDK-OBJECTS-GUIDE.md.
 * Only CD_readsync of the three is real game ground.
 *
 * ROUND 34 (head): CONVERTED. The unit's last three functions -- CdSearchFile
 * (func_8002B640), _cmp (func_8002B928, which had been matched as C) and
 * CD_newmedia (func_8002B94C) -- are linked from `lib/libcd/iso9660.o`, which
 * runs on into code_179d8_d (CD_searchdir, CD_cachefile, cd_read and a WEAK
 * memcpy). The unit is now 0x1AB78..0x1BE40 (vram 0x8002A378..0x8002B640),
 * 14 functions. The iso9660-only declarations that used to sit below (the
 * CD_* diagnostic strings, the directory-cache views, UWord) went with them.
 *
 * Note what happened here, because it is the reason this comment now
 * carries three verdicts: round 24 reopened all three as free ground and
 * round 32's first pass "corrected" that to near-misses -- both times
 * without asking whether Sony owned them. `python3 tools/sdkstalls.py`
 * answers that in one command and did not exist until round 32.
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
                                 * CD_readsync -- addiu_at was RESOLVED round 21, dead cause */
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
extern s32 D_8006D8DC[10];   /* first of 10 consecutive words zeroed by a pointer walk;
                          * D_8006D8E0..D_8006D900 are the other nine, each
                          * already individually named -- not a real array. */
extern s32 D_8006D8E0;
extern s32 D_8006D8E4;
extern volatile s32 D_8006D8E8;
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
extern s32 CD_cw(s32 arg0, s32 arg1, s32 arg2, s32 arg3); /* defined in code_179d8_n */
extern s32 CD_sync(s32 arg0, s32 arg1);                   /* defined in code_179d8_n, per code_179d8_b.c */
extern s32 getintr(void);                                /* code_179d8_b.c, MATCHED round 70
                                                                   (libcd getintr by its strings) */
extern s32 CheckCallback(void);                                /* lib/libetc/intr.o -- trivial
                                                                   (u16)D_8006C272 getter */
/* Still INCLUDE_ASM in THIS unit (not yet converted) -- INCLUDE_ASM leaves no
 * C-level prototype of its own, so callers within this file need one. */
extern s32 func_8002AA6C(void);
extern s32 CD_datasync(s32 arg0);

/* Forward declarations: taken by address before their own ROM-order definition
 * further down this file (CD_initintr/CD_init hand callback to
 * InterruptCallback as a thread entry; func_8002AA6C hands cb_read to
 * D_8006D600 as a callback). */
void callback(void);
void cb_read(s32 arg0, s32 arg1);

s32 CD_vol(u8 *arg0)
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

void CD_shell(void)
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
            CD_cw(1, 0, 0, 0);
        }

        while (CD_cw(0x16, D_8006D908, 0, 0)) {
            CD_cw(1, 0, 0, 0);
            puts(D_80010A50);
        }

        D_8006D5FC = saved;
        D_8006D904 = D_8006D614;
    }
}

void CD_flush(void)
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

s32 CD_initvol(void)
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

void CD_initintr(void)
{
    s32 *p;
    s32 i;

    D_8006D600 = 0;
    D_8006D5FC = 0;
    D_8006D610 = 0;
    D_8006D60C = 0;
    p = D_8006D8DC;
    for (i = 9; i != -1; i--) {
        *p = 0;
        p++;
    }
    ResetCallback();
    InterruptCallback(2, callback);
}

#ifdef NON_MATCHING
/* NON_MATCHING: 171/196 words, length exact. Residue: pure list-scheduling
 * (round-25 --debug breakdown: Reorderings: 3, Register Differences: 0) --
 * retail splits CD_cw(1,0,0,0)'s argument materialization from its
 * own a3/jal by ~90 bytes; neither call position tried reproduces the split
 * (docs/match-reports/CD_init.md). Hand-derived. */
s32 CD_init(void)
{
    s32 *p;
    s32 i;
    volatile u8 *q;
    s32 saved;
    s32 counter;

    puts(D_80010A94);
    printf(D_80010AA0, D_8006D90C);

    D_8006D61D = 0;
    D_8006D61C = 0;
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
    InterruptCallback(2, callback);

    *D_8006D8C0 = 1;
    while (*D_8006D8CC & 7) {
        *D_8006D8C0 = 1;
        *D_8006D8CC = 7;
        *D_8006D8C8 = 7;
    }

    CD_cw(1, 0, 0, 0);

    D_8006D8DA = 0;
    q = &D_8006D8D9;
    D_8006D61C = 0;
    *q = D_8006D8DA;
    __asm__("");
    D_8006D8D8[0] = 2;
    *D_8006D8C0 = 0;
    *D_8006D8CC = 0;
    *D_8006D8D0 = 0x1325;

    counter = 0;
    if (D_8006D60C & 0x10) {
        CD_cw(1, 0, 0, 0);
    }

    if (D_8006D904 < D_8006D614) {
        saved = D_8006D5FC;
        D_8006D5FC = 0;

        while (D_8006D60C & 0x10) {
            if ((u8)counter == 0) {
                puts(D_80010A40);
            }
            counter++;
            CD_cw(1, 0, 0, 0);
        }

        while (CD_cw(0x16, D_8006D908, 0, 0)) {
            CD_cw(1, 0, 0, 0);
            puts(D_80010A50);
        }

        D_8006D5FC = saved;
        D_8006D904 = D_8006D614;
    }

    if (CD_cw(0xA, 0, 0, 0) != 0) {
        return -1;
    }
    if (CD_cw(0xC, 0, 0, 0) != 0) {
        return -1;
    }
    return -(CD_sync(0, 0) != 2);
}
#else
INCLUDE_ASM("asm/nonmatchings/code_179d8_g", CD_init);
#endif

#ifdef NON_MATCHING
/* NON_MATCHING: 215/223 words, length exact. Residue: two small isolated
 * clusters -- a loop-setup scheduling swap at 0x8002AABC (p2 computed from
 * $a0 before vs. after the move into $s5) and a register-identity swap in
 * the final D_8006D8F4=-1 block at 0x8002ADAC -- neither reachable by any
 * reorder or spelling variant tried (docs/match-reports/func_8002AA6C.md).
 * Hand-derived structure (rounds 17-39); the n/saved throwaway-sink reuse
 * a few lines below is a permuter find (round 41) reviewed here and
 * confirmed sound (both are freshly written on every path before their
 * next read) against a rejected sibling candidate that hoisted a value
 * across a loop boundary unsoundly. */
s32 func_8002AA6C(void)
{
    s32 n;
    s32 *tmp;
    s32 *pRetry;
    volatile s32 *p2;
    s32 saved;
    s32 counter;
    volatile u8 *q;
    u8 buf;

    tmp = D_8006D8DC;
    n = *tmp;
    D_8006D600 = 0;
    D_8006D5FC = 0;
    *tmp = n - 1;
    __asm__("");

    if (n > 0) {
        pRetry = tmp;
        p2 = pRetry + 4;
        do {
            if (*pRetry < 7) {
                counter = 0;
                puts(D_80010AAC);
                printf(D_80010ABC, *pRetry, D_8006D618, D_8006D619, D_8006D61A);

                if (D_8006D904 < D_8006D614) {
                    saved = D_8006D5FC;
                    D_8006D5FC = 0;

                    while (D_8006D60C & 0x10) {
                        if ((u8)counter == 0) {
                            puts(D_80010A40);
                        }
                        counter++;
                        CD_cw(1, 0, 0, 0);
                    }

                    while (CD_cw(0x16, D_8006D908, 0, 0)) {
                        CD_cw(1, 0, 0, 0);
                        puts(D_80010A50);
                    }

                    D_8006D5FC = saved;
                    D_8006D904 = D_8006D614;
                }

                if (CD_cw(9, 0, 0, 0) != 0) {
                    goto tail;
                }
                if (CD_cw(2, (s32)&D_8006D618, 0, 0) != 0) {
                    goto tail;
                }
            }

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

            {
                s32 v0 = p2[0];
                buf = (u8)v0;
                n = ((u8)v0 != D_8006D61C);
                if (n) {
                    saved = (s32)&buf;
                    if (CD_cw(0xE, saved, 0, 0) != 0) {
                        goto tail;
                    }
                }
            }

            D_8006D600 = (s32)cb_read;
            p2[-1] = p2[-2];
            CD_cw(6, 0, 0, 1);
            p2[2] = p2[-3];
            p2[3] = VSync(-1) + 0x1E0;
            return p2[2];

        tail:
            tmp = D_8006D8DC;
            n = *tmp;
            *tmp = n - 1;
            __asm__("");
        } while (n > 0);
    }

    {
        volatile s32 *pF4 = &D_8006D8F4;
        *pF4 = -1;
        return *pF4;
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/code_179d8_g", func_8002AA6C);
#endif

s32 CD_readm(s32 arg0, s32 arg1, s32 arg2)
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
    D_8006D8DC[0] = 8;
    D_8006D8FC = D_8006D5FC;
    D_8006D900 = D_8006D600;

    if (D_8006D60C & 0xE0) {
        CD_cw(9, 0, 0, 0);
    }
    CD_sync(0, 0);
    return -(func_8002AA6C() < 1);
}

s32 CD_readsync(s32 arg0, s32 arg1)
{
    s32 now;
    s32 old;
    s32 flags;
    s32 *pEC;
    u8 status;
    s32 *p6A0;
    u8 *p8D8;
    u8 *p8D9;
    s32 *pF8;
    u8 *dst;
    u8 *src;
    s32 i;
    s32 idx0;
    s32 idx1;
    s32 result;

    now = VSync(-1);
    p6A0 = D_8006D6A0;
    p8D8 = D_8006D8D8;
    p8D9 = &D_8006D8D8[1];
    pF8 = &D_8006D8DC[7];

    D_8008B3E4 = now + 0x1E0;
    D_8008B3E8 = 0;
    D_8008B3EC = (s32)D_80010AD8;

    for (;;) {
        now = VSync(-1);
        if (D_8008B3E4 < now) {
            goto timeout;
        }
        old = D_8008B3E8;
        D_8008B3E8 = old + 1;
        if (0x1E0000 >= old) {
            goto success;
        }

    timeout:
        puts(D_80010984);
        idx0 = p8D8[0];
        idx1 = p8D8[1];
        __asm__("");
        /* &D_8008B3EC routed through a local pointer -- forces the same
         * unfolded lui/addiu addressing retail uses for this argument;
         * a plain `D_8008B3EC` reference here compiles FOLDED instead.
         * See docs/match-reports/CD_readsync.md's round-36 entry. */
        pEC = &D_8008B3EC;
        printf(D_80010994, *pEC, D_8006D620[D_8006D61D],
               p6A0[idx0], p6A0[idx1]);
        CD_flush();
        result = -1;
        goto after_diag;

    success:
        result = 0;

    after_diag:
        if (result != 0) {
            return result;
        }
        if (CheckCallback() != 0) {
            status = (u8)(*D_8006D8C0 & 3);
            for (;;) {
                flags = getintr();
                if (flags == 0) {
                    break;
                }
                if ((flags & 4) && D_8006D600 != 0) {
                    ((void (*)(s32, u8 *))D_8006D600)(p8D9[0], D_8008B3D4);
                }
                if ((flags & 2) && D_8006D5FC != 0) {
                    ((void (*)(s32, u8 *))D_8006D5FC)(p8D8[0], D_8008B3CC);
                }
            }
            /* ------------------------------------------------------------
             * KNOWN-BAD CONSTRUCT, KEPT ONLY BECAUSE IT IS BYTE-EXACT.
             * DO NOT COPY THIS SHAPE INTO ANOTHER FUNCTION.
             *
             * `p6A0` and `pF8` are both addresses of globals, so the
             * condition is a TAUTOLOGY and both arms are IDENTICAL. GCC
             * 2.6.3 cross-jumps the two arms back into the single `sb`
             * retail has, so this compiles to no extra instruction -- its
             * whole effect is to perturb register allocation, forcing
             * `status` into $s1 across the inner loop. Worth 12 words:
             * 162/174 without it, 174/174 with it.
             *
             * docs/PARALLEL-RUNS.md Gate 3 names duplicate-arm forms
             * alongside UB as the signature of an EXHAUSTED class rather
             * than a solution, and says a permuter zero is a LEAD to be
             * translated into idiomatic C and re-verified. Round 39's head
             * tried eleven such translations and none reached 174/174 --
             * every declaration- and assignment-order permutation of the
             * four pointer locals, `status` retyped to s32, a real
             * (non-tautological) guard, a re-masked store, and an explicit
             * live-range extension. All tabulated in the match report.
             *
             * THE MIS-MODELLING LEAD IS TESTED AND THE ANSWER IS SPLIT
             * (round 40, head). The round-39 form of this comment said a
             * tautological null check is what a mis-modelled global looks
             * like, and asked whether either operand is really a POINTER
             * global. Measured from the DATA, not from attempts:
             *
             *   - D_8006D6A0 is a fixed 8-element table of rodata string
             *     addresses (asm/data/5DDFC.data.s:121). An array.
             *   - D_8006D8F8 is one zero word that the sibling
             *     cb_read stores VSync()'s return into
             *     (cb_read.s:49-51). An s32 timestamp.
             *
             * Neither is a pointer global, so the condition CANNOT become
             * an honest null test by that route. That half is closed.
             *
             * A DIFFERENT mis-modelling was real and IS now corrected:
             * `pF8[-1]` (used four times below) reaches D_8006D8F4 by
             * negative indexing off D_8006D8F8, which nobody writes -- the
             * ten consecutive words ARE one array, as the zeroing walk in
             * CD_readm already implied. They are now declared
             * `s32 D_8006D8DC[10]` and indexed, and that model is
             * BYTE-IDENTICAL (174/174, whole image green).
             *
             * BUT IT DOES NOT DISSOLVE THIS CONSTRUCT. Under the corrected
             * model, removing the construct still scores 162/174 -- the
             * same figure as before -- and every residual diff is a pure
             * register swap ($s2/$s3, $a0/$a2, $v0/$v1). So the 12 words
             * are REGISTER ALLOCATION, not data modelling, and the data
             * model was never what this construct was standing in for.
             * Do not re-run the data-modelling axis; it is measured.
             * ------------------------------------------------------------ */
            if (p6A0 || pF8) {
                *D_8006D8C0 = status;
            } else {
                *D_8006D8C0 = status;
            }
        }

        dst = (u8 *)arg1;
        src = D_8008B3D4;
        if (dst != 0) {
            for (i = 7; i != -1; i--) {
                *dst = *src;
                src++;
                dst++;
            }
        }

        if (VSync(-1) > pF8[0] + 0x3C) {
            func_8002AA6C();
        }
        if (pF8[-1] == 0) {
            CD_datasync(0);
        }
        if (arg0 != 0 || pF8[-1] <= 0) {
            break;
        }
    }

    return pF8[-1];
}

#ifdef NON_MATCHING
/* NON_MATCHING: 49/91 words, length exact. Residue: register identity
 * (the three hoisted pointers p620/p6A0/p8D8 land in different
 * callee-saved registers than retail's $s3/$s1/$s0) (docs/match-reports/
 * CD_datasync.md). Structure is hand-derived; the diagnostic call's
 * `ok =` sink is a permuter find (round 36), reviewed as a semantically
 * inert dead-store reuse and oracle-confirmed. */
s32 CD_datasync(s32 arg0)
{
    s32 now;
    s32 ok;
    s32 *p620;
    u8 *p8D8;
    s32 *p6A0;

    D_8008B3E4 = VSync(-1) + 0x1E0;
    p620 = D_8006D620;
    p6A0 = D_8006D6A0;
    p8D8 = D_8006D8D8;
    D_8008B3E8 = 0;
    D_8008B3EC = (s32)D_80010AE0;

    for (;;) {
        now = VSync(-1);
        ok = 1;
        if (D_8008B3E4 < now) {
            ok = 0;
        } else {
            D_8008B3E8 = D_8008B3E8 + 1;
            if (0x1E0000 < D_8008B3E8) {
                ok = 0;
            }
        }
        if (!ok) {
            puts(D_80010984);
            /* retail reuses the (dead, about-to-be-overwritten) `ok` slot as
             * the register target for this last argument's value -- a fresh
             * local here compiles worse (45/91 vs 49/91); see this report's
             * round-36 entry. */
            printf(D_80010994, p8D8[0], p6A0[p8D8[1]], p620[D_8006D61D],
                   ok = p6A0[p8D8[0]]);
            CD_flush();
            return -1;
        }
        if ((*D_8006D934 & 0x1000000) == 0) {
            return 0;
        }
        if (arg0 == 0) {
            continue;
        }
        return 1;
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/code_179d8_g", CD_datasync);
#endif

s32 CD_getsector(s32 arg0, s32 arg1)
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

#ifdef NON_MATCHING
/* NON_MATCHING: 30/56 words, length 1 short. Residue: instruction-selection
 * (retail computes &D_8006D8D8 unfolded inside the loop; this folds it)
 * (docs/match-reports/callback.md). Hand-derived. */
void callback(void)
{
    u8 status;
    s32 flags;
    s32 handler;
    u8 *pd9;

    status = (*D_8006D8C0) & 3;
    pd9 = &D_8006D8D9;

    for (;;) {
        flags = getintr();
        if (flags == 0) {
            break;
        }
        if (flags & 4) {
            handler = D_8006D600;
            if (handler != 0) {
                ((void (*)(s32, u8 *))handler)(*pd9, D_8008B3D4);
            }
        }
        if (flags & 2) {
            if (D_8006D5FC != 0) {
                ((void (*)(s32, u8 *))D_8006D5FC)(D_8006D8D8[0], D_8008B3CC);
            }
        }
    }
    *D_8006D8C0 = status;
}
#else
INCLUDE_ASM("asm/nonmatchings/code_179d8_g", callback);
#endif

void cb_read(s32 arg0, s32 arg1)
{
    volatile s32 *p;
    s32 code;
    s32 dummy;
    volatile s32 *new_var;

    if (arg0 != 1) {
        goto elseBranch;
    }
    p = &D_8006D8F4;
    if (*p <= 0) {
        goto shared;
    }
    CD_getsector(D_8006D8E8, D_8006D8F0);
    D_8006D8E8 = D_8006D8E8 + D_8006D8F0 * 4;
    *p = *p - 1;
    dummy = *p;
    (void)dummy;
    goto shared;
elseBranch:
    {
        volatile s32 *p2 = &D_8006D8F4;
        *p2 = -1;
    }
shared:
    {
        volatile s32 *pF8 = &D_8006D8F8;
        *pF8 = VSync(-1);
    }

    if (D_8006D8F4 < 0 && D_8006D8DC[0] > 0) {
        func_8002AA6C();
    }

    if ((*(new_var = &D_8006D8F4)) <= 0) {
        D_8006D5FC = D_8006D8FC;
        D_8006D600 = D_8006D900;
        CD_cw(9, 0, 0, 0);
        if (D_8006D604 != 0) {
            if ((*new_var) == 0) {
                code = 2;
            } else {
                code = 5;
            }
            ((void (*)(s32, s32))D_8006D604)(code, arg1);
        }
    }
}
