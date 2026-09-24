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
 * code_179d8_l -- FRONT half of what was the `code_179d8_mid_c` asm
 * remainder: functions 149..160 of the original 274-function code_179d8
 * monolith, 0x1D508..0x1ECD8 (vram 0x8002CD08..0x8002E4D8), 12 functions.
 * Carved round 24 (2026-09-08).  `code_179d8_m` is the back half; between
 * them they consume the remainder whole.
 *
 * WHY IT WAS UNCARVED, AND WHY THAT VERDICT IS DEAD.  The splat comment on
 * the old remainder called 0x1D508 "the addiu-$at dense heart of this
 * monolith (of its 51 functions only ~12 are clean)".  `addiu_at` was
 * RESOLVED in round 21 (maspsx `--addiu-at`;
 * docs/research/addiu-at-blocker.md), so that census measured an
 * obstruction that no longer exists.  Re-censused 2026-09-08 over all 24
 * functions of the remainder with the four screens, canonical shell forms
 * (`grep -A2` FORWARD for nop_mflo_mfhi): 21 of 24 CLEAN, zero gp_rel,
 * zero `jr $t2` trampolines.
 *
 * The remainder is UNIFORM in density now, so the cut between _l and _m is
 * a STAFFING cut (one unit per runner) and not a density cut.  All three
 * surviving blocked functions landed in this half:
 *
 * ROUND 44: THE THREE BELOW ARE ASSIGNABLE AND ARE THIS UNIT'S FRESH GROUND.
 * ~~BLOCKED on nop_mflo_mfhi, which is STILL OPEN -- do NOT spend attempts:~~
 *   func_8002CD08 (132w), SpuVmKeyOnNow (316w), vmNoiseOn (311w)
 * nop_mflo_mfhi was RESOLVED in round 42 (`--no-nop-mflo-mfhi`), so the
 * "do NOT spend attempts" directive above is WITHDRAWN.  All three are
 * never-attempted cold ground whose reports were marked REOPENED in round 44.
 * That blocker is an `mflo`/`mfhi` FOLLOWED WITHIN TWO INSTRUCTIONS BY a
 * `mult`/`div`; it is the one construct in docs/research/addiu-at-blocker.md
 * that round 21 did not fix.  Each has a stub report.
 *
 * 9 CLEAN of 12, cheapest first:
 *   func_8002E2F8    2w  <- already matched: splat emitted the empty C body
 *   func_8002E300    2w  <- itself.  Not work, and not yours to redo.
 *   note2pitch   47w   note2pitch2   64w   vmNoiseOn2  112w
 *   SePitchBend  112w   SeAutoVol  116w   SpuVmDoAllocate  143w
 *   SpuVmAlloc  167w
 *
 * SeAutoVol's opening `addu $t3, $a0, $zero` is REGISTER PRESSURE with
 * s16 argument narrowing, NOT a BIOS trampoline -- checked by hand at carve
 * time, because the `jr $t2` trampoline screen is blind to variants and a
 * trampoline-dense segment reads as the cleanest ground in the file while
 * being the least matchable (round 17, class_3bb8c_h).  It is also a
 * near-identical sibling of SeAutoPan in code_179d8_m: same prologue,
 * same narrowing shape, same early-out branch.  If you match one, say so in
 * the report -- the other unit's runner is deriving the same shape.
 *
 * Owns NO jump table: zero `jtbl_` references, and no rodata word anywhere
 * in the image points into 0x8002CD08..0x800300D0 (checked at carve time
 * both numerically and for symbolic `.word .L`), so no rodata attach.
 * Boundary checks both sides: no function has more than one
 * `addiu $sp, $sp, -N`, every one ends in its own `jr $ra`, zero `alabel`,
 * and the frameless ones open on their own arguments or on a global, never
 * on $sp.
 *
 * Expect this slice to span more than one class; identify each with
 * tools/classtable.py rather than assuming the unit has one.
 */
#include "common.h"

/* Matched round 73 -- docs/match-reports/func_8002CD08.md. */
typedef struct Obj179D8CD08 Obj179D8CD08;

typedef struct {
    u8 pad0[0x80];
    s32 (*slot80)(Obj179D8CD08 *self, s32 arg1, s32 arg2, s32 arg3);
    void (*slot84)(Obj179D8CD08 *self, s32 handle);
    u8 pad88[0x9C - 0x88];
    void (*slot9C)(Obj179D8CD08 *self, s32 arg1);
} Obj179D8CD08Methods;

struct Obj179D8CD08 {
    Obj179D8CD08Methods *methods;
};

typedef struct {
    s32 result;
    s32 word0;
    s32 word1;
    s32 word2;
    s32 word3;
} Entry179D8CD08;

typedef struct S179D8CD08 S179D8CD08;

struct S179D8CD08 {
    s32 unk0;
    s32 unk4;
    s32 unk8;
    void (*callback)(s32 arg0, S179D8CD08 *self);
    s32 unk10;
    s32 unk14;
    Entry179D8CD08 entries[3];
};

void func_8002CD08(Obj179D8CD08 *a0, S179D8CD08 *a1) {
    s32 i;
    Entry179D8CD08 *e;
    s32 rem1;
    s32 rem2;
    s32 note;

    if (a1->unk0 > 0) {
        i = 0;
        e = &a1->entries[0];
        do {
            i++;
            e->word0 = -1;
            e->word1 = 0;
            e->word2 = 0x7F;
            e->word3 = 0x40;
            e++;
        } while (i < 3);

        a1->unk10 = 0;
        if (a1->callback != NULL) {
            a1->callback(a1->unk8, a1);
        }

        if (a1->unk10 >= 0) {
            e = &a1->entries[0];
            i = 0;
            do {
                if (e->word0 >= 0) {
                    if (e->result >= 0) {
                        a0->methods->slot84(a0, e->result);
                    }
                    a0->methods->slot9C(a0, e->word1);
                    note = e->word0 * 16;
                    rem1 = e->word2 - (e->word2 / a1->unk14) * a1->unk10;
                    rem2 = e->word3 - (e->word3 / a1->unk14) * a1->unk10;
                    e->result = a0->methods->slot80(a0, note, rem1, rem2);
                } else if (e->word0 == -2 && e->result >= 0) {
                    a0->methods->slot84(a0, e->result);
                }
                i++;
                e++;
            } while (i < 3);
        }
        a1->unk4++;
    }
}


#ifdef NON_MATCHING
/* NON_MATCHING: 167/167 words, length exact (74/167 raw word-match; funcdiff
 * insertions/deletions 16/16). Residue: a systematic register rotation
 * (t3/t0/a2/a3 family) running through nearly the whole function, visible
 * from the very first instruction (docs/match-reports/SpuVmAlloc.md).
 * Hand-derived. */
extern u8 D_8008D9A3[];
extern u8 D_8008D98E[];
extern u8 D_8008D98A[];
extern u8 D_8008D9A0[];
extern u8 D_8008D988[];
extern u8 D_8008E9D0;
extern u8 D_8008EA1B;
extern void SpuSetNoiseVoice(s32 a0, s32 a1);

s32 SpuVmAlloc(void)
{
    s32 chosen;
    u16 bestSec;
    s32 found;
    s32 bestTer;
    s32 bestIdx;
    u32 idx;
    s32 threshold;
    s32 pri;
    u32 newSec;
    u32 count;
    u8 *p988;

    chosen = 0x63;
    bestSec = 0xFFFF;
    found = 0;
    bestTer = 0;
    bestIdx = 0x63;
    threshold = D_8008EA1B;

    for (idx = 0; (u8) idx < D_8008E9D0; idx++) {
        if (D_8008D9A3[(u8) idx * 0x34] != 0
            || *(u16 *)(D_8008D98E + (u8) idx * 0x34) != 0) {
            pri = *(s16 *)(D_8008D9A0 + (u8) idx * 0x34);
            if (pri < (s32)(u16) threshold) {
                threshold = pri;
                bestIdx = idx;
                bestSec = *(u16 *)(D_8008D98E + (u8) idx * 0x34);
                bestTer = *(u16 *)(D_8008D98A + (u8) idx * 0x34);
                found = 1;
            } else if (pri == (s32)(u16) threshold) {
                found++;
                newSec = *(u16 *)(D_8008D98E + (u8) idx * 0x34);
                if (newSec < bestSec) {
                    bestTer = *(u16 *)(D_8008D98A + (u8) idx * 0x34);
                    bestSec = newSec;
                    bestIdx = idx;
                } else if (newSec == bestSec) {
                    if (bestTer < (s16) *(u16 *)(D_8008D98A + (u8) idx * 0x34)) {
                        bestTer = (s16) *(u16 *)(D_8008D98A + (u8) idx * 0x34);
                        bestIdx = idx;
                    }
                }
            }
        } else {
            chosen = idx;
        }
    }

    if ((u8) chosen == 0x63) {
        if ((u8) found != 0) {
            chosen = bestIdx;
        } else {
            chosen = D_8008E9D0;
        }
    }

    count = D_8008E9D0;
    if ((u8) chosen < count) {
        if (count != 0) {
            p988 = D_8008D988;
            for (idx = 0; (u8) idx < count; idx++) {
                *(u16 *)(p988 + (u8) idx * 0x34 + 2) =
                    *(u16 *)(D_8008D98A + (u8) idx * 0x34) + 1;
            }
        }
        *(u16 *)(D_8008D98A + (u8) chosen * 0x34) = 0;
        *(s16 *)(D_8008D9A0 + (u8) chosen * 0x34) = D_8008EA1B;
        if (D_8008D9A3[(u8) chosen * 0x34] == 2) {
            SpuSetNoiseVoice(0, 0xFFFFFF);
        }
    }
    return (u8) chosen;
}
#else
INCLUDE_ASM("asm/nonmatchings/code_179d8_l", SpuVmAlloc);
#endif

/* Shared with vmNoiseOn below (same two-level entry table, same
 * blend-cascade shape); declared once here since SpuVmKeyOnNow is
 * ROM-earlier, reused there rather than redeclared. */
typedef struct {
    u8 pad0[0x74];
    u16 unk74;
    u16 unk76;
    u8 pad78[0xAC - 0x78];
} D800902E8Entry;
extern D800902E8Entry *D_800902E8[];

extern u8 D_8008EA16;
extern u8 D_8008EA19;
extern u8 D_8008EA17;
extern u8 D_8008EA11;
extern u8 D_8008EA1A;
extern s16 D_8008E8C0;
extern u16 D_8008EA22;
extern u8 D_8008EA20;
extern u8 D_8008D970[];
extern u8 D_8008D98C[];
extern u8 D_8008D9A3[];
extern u16 D_8008E228;
extern u16 D_8008E22C;
extern u16 D_80090C60;
extern u16 D_80090C64;
extern u16 D_8008E230;
extern u16 D_8008E234;

/* STALL -- see docs/match-reports/SpuVmKeyOnNow.md. Best body reached
 * (332/316 built words, 16 words LONG; 33/316 raw word-match, drift-
 * affected) preserved there in #if 0. */
#ifdef NON_MATCHING
/* NON_MATCHING: 316/316 words, length exact (201/316 raw word-match; funcdiff
 * insertions/deletions 46/46). Residue: frame SIZE only -- addiu sp,sp,-8
 * against retail's -0x10 -- plus this unit's documented register-identity
 * class (docs/match-reports/SpuVmKeyOnNow.md). Hand-derived, plus one
 * permuter hoist (round 65: pan1sq/16383 computed before pan2sq), reviewed
 * as a pure reordering and kept. */
/* Object holding a per-note "priority"-ish scale byte at +0x18; only field
 * this function needs. */
typedef struct {
    u8 pad0[0x18];
    u8 unk18; /* +0x18 */
} ObjE970;
extern ObjE970 *D_8008E970;

extern u8 D_8008EA10;
/* NOT volatile, and declared as an incomplete ARRAY on purpose: the array
 * spelling is what makes GCC 2.6.3 materialise the address once into a GPR
 * and spend one word per read, which is retail. Retail's five reloads come
 * from ordinary CSE invalidation by the stores between them. */
extern s16 D_8008EA26[];

/* Independent 0x10-byte-stride s16 array. */
extern s16 D_8008D7F4[];

void SpuVmKeyOnNow(s32 a0, s32 a1) {
    D800902E8Entry *e;
    s32 prio;
    s32 lvl0;
    u32 lvl1;
    u32 pan1;
    u32 pan2;
    u32 pan1sq;
    u32 pan2sq;
    s16 pan1out;
    s32 chanIdx;
    s32 lowBit;
    s32 highBit;

    prio = D_8008E970->unk18 * 0x3FFF;
    lvl0 = D_8008EA10 * prio / 16129;
    lvl1 = (u32)lvl0 * D_8008EA16 * D_8008EA19 / 16129;

    chanIdx = D_8008EA26[0] * 8;

    e = &D_800902E8[D_8008EA22 & 0xFF][D_8008EA22 >> 8];
    pan1 = lvl1;
    pan2 = lvl1;
    if ((s16)D_8008EA22 != 0x21) {
        pan1 = lvl1 * e->unk74 / 127;
        pan2 = lvl1 * e->unk76 / 127;
    }

    if ((u8)D_8008EA1A < 0x40) {
        pan2 = (pan2 * D_8008EA1A) / 63;
    } else {
        pan1 = (pan1 * (0x7F - D_8008EA1A)) / 63;
    }

    if ((u8)D_8008EA17 < 0x40) {
        pan2 = (pan2 * D_8008EA17) / 63;
    } else {
        pan1 = (pan1 * (0x7F - D_8008EA17)) / 63;
    }

    if ((u8)D_8008EA11 < 0x40) {
        pan2 = (D_8008EA11 * pan2) / 63;
    } else {
        pan1 = (pan1 * (0x7F - D_8008EA11)) / 63;
    }

    if (D_8008E8C0 == 1) {
        if (pan1 < pan2) {
            pan1 = pan2;
        } else {
            pan2 = pan1;
        }
    }
    pan1sq = pan1 * pan1;
    pan1out = (s16)(pan1sq / 16383);
    pan2sq = pan2 * pan2;

    D_8008D7F4[(u16)chanIdx] = (s16)a1;
    D_8008D7F4[(u16)chanIdx - 2] = pan1out;
    D_8008D7F4[(u16)chanIdx - 1] = (s16)(pan2sq / 16383);

    D_8008D970[D_8008EA26[0]] |= 7;
    *(u16 *)(D_8008D98C + D_8008EA26[0] * 0x34) = (s16)a1;
    *(u8 *)(D_8008D9A3 + D_8008EA26[0] * 0x34) = 1;

    if (D_8008EA26[0] < 0x10) {
        lowBit = 1 << D_8008EA26[0];
        highBit = 0;
    } else {
        lowBit = 0;
        highBit = 1 << (D_8008EA26[0] - 0x10);
    }

    if (D_8008EA20 & 4) {
        D_8008E230 = lowBit | D_8008E230;
        D_8008E234 = highBit | D_8008E234;
    } else {
        D_8008E230 = D_8008E230 & ~lowBit;
        D_8008E234 = D_8008E234 & ~highBit;
    }

    D_8008E228 = lowBit | D_8008E228;
    D_8008E22C = highBit | D_8008E22C;
    D_80090C60 = D_80090C60 & ~D_8008E228;
    D_80090C64 = D_80090C64 & ~D_8008E22C;
}
#else
INCLUDE_ASM("asm/nonmatchings/code_179d8_l", SpuVmKeyOnNow);
#endif

extern u8 D_8008EA13;
extern u8 D_8008EA18;

/* Shared with note2pitch2 below (same base pointer, same table); this
 * function needs the +0x10/+0x12 halfwords too, so the struct is declared
 * once here (ROM-address order: SpuVmDoAllocate precedes note2pitch2) and
 * reused there rather than redeclared -- see docs/match-reports/SpuVmDoAllocate.md. */
typedef struct {
    u8 unk0[4];
    u8 unk4;
    u8 unk5;
    u8 unk6[6];
    u8 unk12;
    u8 unk13;
    u8 pad14[2];
    u16 unk16; /* +0x10 */
    u16 unk18; /* +0x12 */
    u8 pad20[0x20 - 20];
} D8008E978Entry;
extern D8008E978Entry *D_8008E978;

INCLUDE_ASM("asm/nonmatchings/code_179d8_l", SpuVmDoAllocate);

/* D800902E8Entry, D_800902E8 and the blend-cascade globals
 * (D_8008EA16/17/19/1A/11/20/22, D_8008E8C0, D_8008E228/22C, D_80090C60/64,
 * D_8008E230/234, D_8008D970/98C/9A3) are already declared above, before
 * SpuVmKeyOnNow (ROM-earlier, same shapes) -- reused here, not redeclared. */
extern u8 D_8008EA0E;
extern u8 D_8008EA1C;
extern u16 *D_8006DAD4;
extern u8 D_8008D7F0[];
extern u8 D_8008D7F2[];
extern u8 D_8008E9D0;

/* STALL -- see docs/match-reports/vmNoiseOn.md. Best body reached
 * (309/311 built words, 2 words SHORT) preserved there in #if 0. */
INCLUDE_ASM("asm/nonmatchings/code_179d8_l", vmNoiseOn);

#ifdef NON_MATCHING
/* NON_MATCHING: 107/112 words, 5 words short. Residue: the a0/a3 role-swap
 * register-identity class (this unit's documented class) plus an 8-byte
 * frame retail allocates that this shape doesn't reach
 * (docs/match-reports/vmNoiseOn2.md). Hand-derived. The byte-shaped
 * body's order-only __asm__("") barrier is omitted here; it is in the report. */
extern u8 D_8008D98A[];

void vmNoiseOn2(s32 a0, s32 a1, s32 a2) {
    s32 a3;
    s32 off16;
    s32 v1;
    s32 lowBit;
    s32 highBit;
    s32 idx52;
    s32 li;
    s32 i;
    s32 n;

    a3 = a0;
    a0 = (u8)a0;
    off16 = a0 << 4;
    *(u16 *)(D_8008D7F2 + off16) = a2;
    v1 = D_8008D970[a0];
    *(u16 *)(D_8008D7F0 + off16) = a1;
    v1 |= 3;
    D_8008D970[a0] = v1;
    if ((u32)a0 < 16) {
        lowBit = 1 << a0;
        highBit = 0;
    } else {
        lowBit = 0;
        highBit = 1 << (a0 - 16);
    }

    idx52 = (u8)a3 * 52;
    n = D_8008E9D0;
    *(u16 *)(D_8008D98C + idx52) = 10;
    if (n != 0) {
        i = 0;
        do {
            li = (u16)i * 52;
            *(u8 *)(D_8008D9A3 + li) = *(u8 *)(D_8008D9A3 + li) & 1;
            i++;
        } while ((u16)i < D_8008E9D0);
    }
    idx52 = (u8)a3 * 52;
    *(u8 *)(D_8008D9A3 + idx52) = 2;

    *(u16 *)(D_8008D98A + idx52) = 0;
    D_8008E228 = lowBit | D_8008E228;
    D_8008E22C = highBit | D_8008E22C;
    D_80090C60 = D_80090C60 & ~D_8008E228;
    D_80090C64 = D_80090C64 & ~D_8008E22C;
    D_8006DAD4[0xCA] = lowBit;
    D_8006DAD4[0xCB] = highBit;
}
#else
INCLUDE_ASM("asm/nonmatchings/code_179d8_l", vmNoiseOn2);
#endif

extern u8 D_8008EA0E;
extern u8 D_8008EA1C;
extern u8 D_8008EA1D;
extern u16 D_8006DAD8[];

s32 note2pitch(void) {
    s32 a0;
    s32 q12;
    s16 rem12;
    u8 a2;
    u16 v1;

    a0 = (s16)(D_8008EA0E + 0x3C - D_8008EA1C);
    q12 = a0 / 12;
    a2 = D_8008EA1D >> 3;
    rem12 = a0 - q12 * 12;
    if (a2 >= 16) {
        a2 = 15;
    }
    v1 = D_8006DAD8[a2 + rem12 * 16];
    if ((s16)(q12 - 5) > 0) {
        v1 <<= (s16)(q12 - 5);
    } else if ((s16)(q12 - 5) < 0) {
        v1 = (u16)v1 >> -(s16)(q12 - 5);
    }
    return v1;
}

extern u8 D_8008EA13;
extern u8 D_8008EA18;

s32 note2pitch2(s32 a0, s32 a1) {
    s32 origA0;
    s32 idx;
    s32 tblIdx;
    D8008E978Entry *e;
    s32 v0;
    s32 div8;
    u8 a2;
    s16 a3;
    s32 diff;
    s32 q12;
    s16 rem12;
    u16 v1;

    origA0 = a0;
    idx = D_8008EA18 + (D_8008EA13 << 4);
    e = &D_8008E978[idx];
    v0 = (u16)a1 + e->unk5;
    div8 = v0 / 8;
    a3 = div8;
    a2 = 0;
    if (div8 >= 16) {
        a2 = 1;
        a3 = div8 - 16;
    }
    diff = (s16)(a2 + (origA0 + 0x3C - e->unk4));
    q12 = diff / 12;
    rem12 = diff - q12 * 12;
    tblIdx = rem12 * 16;
    tblIdx = tblIdx + a3;
    v1 = D_8006DAD8[tblIdx];
    if ((s16)(q12 - 5) > 0) {
        v1 <<= (s16)(q12 - 5);
    } else if ((s16)(q12 - 5) < 0) {
        v1 = (u16)v1 >> -(s16)(q12 - 5);
    }
    return v1;
}

/* Matched round 73 -- docs/match-reports/SePitchBend.md. */
typedef struct {
    u8 unk0;
    u8 pad1[0x34 - 0x1];
} Rec34B_E138;
typedef struct {
    u16 unk0;
    u8 pad2[0x34 - 0x2];
} Rec34H_E138;
extern Rec34B_E138 D_8008D998[];
extern Rec34B_E138 D_8008D99C[];
extern Rec34H_E138 D_8008D994[];
extern s16 D_8008EA26[];
extern u8 D_8008D970[];

void SePitchBend(s32 chan, s32 bend) {
    s32 off;
    s32 prod;
    s32 q;
    s32 note;
    s32 fine;
    s16 b;
    s32 idx;
    u8 *p;

    off = (chan & 0xFF) * 8;
    if ((u32)(chan & 0xFF) < 24) {
        p = &D_8008EA13;
        *p = D_8008D998[(chan & 0xFF)].unk0;
        D_8008EA18 = D_8008D99C[(chan & 0xFF)].unk0;
        D_8008EA26[0] = (u8)chan;
        idx = D_8008EA18 + (*p << 4);
        b = bend;
        if (b >= 0) {
            prod = b * D_8008E978[idx].unk13;
            note = D_8008D994[(chan & 0xFF)].unk0 + prod / 127;
            fine = prod % 127;
        } else {
            q = (b * D_8008E978[idx].unk12) / 127;
            note = D_8008D994[(chan & 0xFF)].unk0 + q - 1;
            fine = q + 127;
        }
        ((u16 *)D_8008D7F0)[off + 2] = note2pitch2((u16)note, (u16)fine);
        D_8008D970[(chan & 0xFF)] |= 4;
    }
}


void func_8002E2F8(void) {
}

void func_8002E300(void) {
}

/* Matched round 73 -- docs/match-reports/SeAutoVol.md. Same body as
 * SeAutoPan (code_179d8_m) over the gVoiceEnv* family. */
typedef struct {
    s16 unk0;
    u8 pad2[0x34 - 0x2];
} Rec34Half_E308;
extern Rec34Half_E308 gVoiceEnvActive[];
extern Rec34Half_E308 gVoiceEnvStep[];
extern Rec34Half_E308 gVoiceEnvInterval[];
extern Rec34Half_E308 gVoiceEnvCountdown[];
extern Rec34Half_E308 gVoiceEnvAccum[];
extern Rec34Half_E308 gVoiceEnvLimit[];

void SeAutoVol(s16 voice, s16 from, s16 to, s16 duration) {
    s16 q;

    if (from == to) {
        return;
    }
    gVoiceEnvActive[voice].unk0 = 1;
    gVoiceEnvAccum[voice].unk0 = from;
    gVoiceEnvLimit[voice].unk0 = to;
    if ((from - to < 0 ? to - from : from - to) < duration) {
        q = duration / (from - to);
        gVoiceEnvStep[voice].unk0 = 1;
        gVoiceEnvInterval[voice].unk0 = q;
        gVoiceEnvCountdown[voice].unk0 = q;
    } else {
        q = (from - to) / duration;
        gVoiceEnvInterval[voice].unk0 = 0;
        gVoiceEnvStep[voice].unk0 = q;
    }
}

