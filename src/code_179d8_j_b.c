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
 * code_179d8_j_b -- the MIDDLE third of the old code_179d8_j slice, after
 * round 34 (2026-09-12) linked TWO Sony objects into what used to be one unit.
 * Now 0x21180..0x221B4 (vram 0x80030980..0x800319B4), five functions
 * (func_80030980 .. func_80031890).
 *
 * WHY THIS UNIT EXISTS, IN TWO STEPS, BOTH IN ROUND 34.
 *   1. `libsnd/vm_prog.o` (Psy-Q 3.6 -- the only disc carrying the module)
 *      covers 0x20FF0..0x21180: SpuVmSetProgVol, SpuVmGetProgVol,
 *      SpuVmSetProgPan, SpuVmGetProgPan, all four previously MATCHED as C.
 *      That split the old code_179d8_j into [c][o][c] and created this file.
 *   2. `libsnd/ut_pb.o` (Psy-Q 3.6 only, 0x90) covers 0x221B4..0x22244:
 *      `SsUtPitchBend`, which was `func_800319B4`, also previously MATCHED.
 *      That split THIS file again into [c][o][c], and everything from
 *      func_80031A44 on moved to `src/code_179d8_j_c.c`.
 * Reclassifying five matched functions out of the game count across the two
 * steps is the correction CLAUDE.md asks for, not a regression; their C is
 * DELETED, not commented out.
 *
 * `D_8008EA22`'S OWN COMMENT WAS WRONG AND IS CORRECTED HERE (round 36): it
 * used to claim `SsUtPitchBend` and the deleted `func_80030648` were its
 * only readers in this family, and dropped the extern on that basis. That
 * was never true of this file's own two remaining stalls -- func_80030E90
 * and func_8003149C both WRITE it (`D_8008EA22 = 0x21;`) -- it just went
 * unnoticed because both were still INCLUDE_ASM and nothing failed to
 * link. Declared again below.
 *
 * NO RODATA ATTACH IS OWNED BY ANY UNIT IN THIS FAMILY, and that is measured,
 * not assumed: the old code_179d8_mid_c monolith contains zero `jtbl_` and
 * zero `.word .L` across its whole extent, and the splat yaml's rodata slot
 * list names none of `code_179d8_j`, `_j_b` or `_j_c`.  Unlike round 33's
 * code_179d8_c_b there was nothing to move, and a link failure of the form
 * `undefined reference to '.L8003....'` would mean something else.
 *
 * DECLARATIONS: this file carries its own copy of what its functions use,
 * split out of the old shared block.  Keep it that way -- do NOT create a
 * shared code_179d8*.h.  The sibling slices are staffed independently and a
 * shared header is what makes their merges collide; see
 * `python3 tools/headercontention.py`.  Several externs below are read only by
 * functions still carried as INCLUDE_ASM (the `func_80030E90` scratch globals
 * in particular); they are knowledge about those functions, not dead code, and
 * were re-homed here deliberately rather than dropped.
 *
 * BLOCKER PROFILE: screen with `python3 tools/nearmiss.py`, never by
 * re-implementing the greps and never for `addiu_at` (resolved round 21).
 * `nearmiss.py` runs `tools/sdkstalls.py` for you, and round 34 is why that
 * matters here: all five functions the two splits gave back to Sony were on
 * the old unit's carve-time WORKABLE list, screened clean on every blocker,
 * and were unmatchable by construction.
 *
 * Expect this slice to span more than one class; identify each with
 * tools/classtable.py rather than assuming the unit has one.  Keep every
 * function in strict ROM-address order.
 */

#include "common.h"

/* 0x20-byte-stride record indexed by `D_8008EA18 + D_8008EA13*16`
 * (func_80030E90's own computed index, not a channel id). Every field
 * this unit's own accessor touches is named; offsets are exact (read
 * from func_80030E90's own lbu/lhu immediates), field names are not. */
typedef struct {
    u8 unk0;  /* +0x0 */
    u8 unk1;  /* +0x1 */
    u8 unk2;  /* +0x2 */
    u8 unk3;  /* +0x3 */
    u8 unk4;  /* +0x4 */
    u8 unk5;  /* +0x5 */
    u8 unk6;  /* +0x6 */
    u8 unk7;  /* +0x7 */
    u8 pad8[0x16 - 0x8];
    u16 unk16; /* +0x16 */
    u8 pad18[0x20 - 0x18];
} RecordE978;
extern RecordE978 *D_8008E978;

/* Base pointer for a table of 0x10-byte entries, indexed by a 0..0x17
 * id.  Only the two leading s16 fields this unit's own accessors touch
 * are named. */
typedef struct EntryDAD4 {
    s16 unk0; /* +0x0 */
    s16 unk2; /* +0x2 */
    s16 unk4; /* +0x4 -- read by func_80031280, entry index 25 only */
    s16 unk6; /* +0x6 -- read by func_80031280, entry index 25 only */
    u8 pad8[0x10 - 0x8];
} EntryDAD4;
extern EntryDAD4 *D_8006DAD4;

/* Reentrancy lock, same identifier/type as the sibling reading in
 * Sony's `SsSeqCalledTbyT` (`libsnd/sscall`, linked since round 34; it was
 * code_179d8_i.c's matched func_80033738) -- "if already busy, return/skip;
 * set; ...; clear before returning" guarding a per-channel operation. */
extern s32 D_8008E934;

/* "Currently selected channel" scratch globals: written as a side
 * effect and then re-read from the global (not from the parameter) by
 * the same and sibling functions -- same idiom as this file's own
 * D_8008EA22 above. */
extern volatile u16 D_8008EA26;
extern volatile u8 D_8008EA18;
extern u16 D_8008EA22;

/* Base pointer for a table of 0x10-byte slots, indexed by the same <0x18
 * channel space func_80030E90 validates via SpuVmVSetUp. This unit's own
 * reduced view: only the fields func_80030E90 itself touches are named.
 * The sibling accessors that used to share this typedef (SpuVmSetProgVol
 * and friends, code_179d8_j.c's old `SlotE968`) are Sony's own object as
 * of round 34's split and never touched offset 0; func_80030E90 does, so
 * this unit's own copy of the type names it (see func_80030864.md, the
 * matched sibling's report, for the +0x1/+0x4 fields' provenance). */
typedef struct SlotE968 {
    u8 unk0; /* +0x0 */
    u8 unk1; /* +0x1 */
    u8 pad2[0x4 - 0x2];
    u8 unk4; /* +0x4 */
    u8 pad5[0x10 - 0x5];
} SlotE968;
extern SlotE968 *D_8008E968;

/* func_80030E90's own scratch globals -- a "start channel" setup
 * routine that stages its parameters and a couple of table lookups
 * into a block of one/two-byte globals before registering a new
 * active-channel record.  Offsets are exact (this unit's own field
 * accesses); names are opaque placeholders per the reduced-local-view
 * convention. */
extern u8 D_8008EA0C;
extern u8 D_8008EA0E;
extern u8 D_8008EA0F;
extern u8 D_8008EA10;
extern u8 D_8008EA11;
extern u8 D_8008EA13;
extern u8 D_8008EA16;
extern u8 D_8008EA17;
extern u8 D_8008EA19;
extern u8 D_8008EA1A;
extern u8 D_8008EA1B;
extern u8 D_8008EA1C;
extern u8 D_8008EA1D;
extern u8 D_8008EA1E;
extern u8 D_8008EA1F;
extern u8 D_8008EA20;
extern u16 D_8008EA24;

extern s32 func_8002CF18(void);
extern void func_8002D6A4(void);
extern void func_8002D8E0(s32 a0);
extern s32 func_8002E038(u16 a0, u16 a1);
extern void func_8002D1B4(s32 a0, u16 a1);
extern s32 SpuVmVSetUp(s16 a0, s16 a1);

/* Loop bound for a small table of active "objects" (screen/slot
 * pairs); see code_179d8_i.c's D_80090B68/6C for the sibling reading of
 * an analogous count. */
extern u8 D_8008E9D0;

/* A pair of 16-bit bitmasks split across a 0..0x1F channel space
 * (low 16 channels in the first word, next 16 in the second), each
 * paired with an "active mask" word that is cleared wherever the
 * channel mask bit is set. */
extern u16 D_80090C60;
extern u16 D_80090C64;
extern u16 D_8008E228;
extern u16 D_8008E22C;

/* 52 (0x34)-byte-stride channel-configuration record, referenced by
 * several unrelated top-level symbols 2 bytes apart (D_8008D994,
 * D_8008D996, D_8008D99A, D_8008D99C, D_8008D99E, ...) -- each function
 * in this unit only touches the one or two fields it actually reads,
 * per this project's reduced-local-view convention. Only the leading
 * s16 is named; every array below shares this one shape. */
typedef struct {
    s16 unk0; /* +0x0 */
    u8 pad2[0x34 - 0x2];
} Rec34D994;
extern Rec34D994 D_8008D994[];
extern Rec34D994 D_8008D990[];
extern Rec34D994 D_8008D996[];
extern Rec34D994 D_8008D998[];
extern Rec34D994 D_8008D99A[];
extern Rec34D994 D_8008D99C[];
extern Rec34D994 D_8008D99E[];

/* Same 0x34 stride, byte-sized "in use" flag at offset 0 (retail always
 * clears it with `sb`, never `sh`). */
typedef struct {
    u8 unk0; /* +0x0 */
    u8 pad1[0x34 - 0x1];
} Rec34Byte;
extern Rec34Byte D_8008D9A3[];

/* Same 0x34 stride, halfword field at offset 0 (retail always clears it
 * with `sh`). Three independent arrays share this shape (D_8008D988,
 * D_8008D98A and D_8008D98C, each 2 bytes apart in the data section --
 * same "several unrelated top-level symbols" convention as the
 * Rec34D994 group above). D_8008D988's field is read signed
 * (func_80031280 compares it against 0xFF with `lh`, not `lhu`). */
typedef struct {
    s16 unk0; /* +0x0 */
    u8 pad2[0x34 - 0x2];
} Rec34Half;
extern Rec34Half D_8008D988[];
extern Rec34Half D_8008D98A[];
extern Rec34Half D_8008D98C[];

/* Shared with func_8002D8E0/func_8002D1B4 in code_179d8_l.c (same
 * two-level entry table, same blend-cascade shape); this unit's own
 * reduced view, per the project's per-unit-local-view convention --
 * only the two fields func_80030980 itself touches are named. */
typedef struct {
    u8 pad0[0x74];
    u16 unk74;
    u16 unk76;
    u8 pad78[0xAC - 0x78];
} D800902E8Entry;
extern D800902E8Entry *D_800902E8[];

typedef struct {
    u8 pad0[0x18];
    u8 unk18; /* +0x18 */
} ObjE970;
extern ObjE970 *D_8008E970;

extern s16 D_8008E8C0;
extern u8 D_8008D7F0[];
extern u8 D_8008D970[];

#ifdef NON_MATCHING
/* NON_MATCHING: 315/324 words, 9 words short as of round 62 (re-measured
 * round 70, unchanged -- this residue is a GCC 2.6.3 CSE decision on the
 * `D_8008E978[D_8008D99C[i].unk0]` address, not the load-delay-nop
 * construct round 63's `--nop-at-expansion` flag resolved, so that flag
 * does not touch this function). Raw word-match 10/324, insertions 76 /
 * deletions 76. Round 50's earlier 324/324 "length-exact" body is
 * FALSIFIED at ins101/del101 -- more structurally wrong despite the
 * matching word count -- and is not used here; this is round 62's
 * better-characterized body. See docs/match-reports/func_80030980.md.
 * Hand-derived. */
s32 func_80030980(s32 a0, s32 a1, s32 a2, s32 a3, u16 a4) {
    D800902E8Entry *e;
    u8 i;
    s32 result;
    u32 pan1;
    u32 pan2;

    result = 0;
    e = &D_800902E8[a0 & 0xFF][(a0 & 0xFF00) >> 8];
    SpuVmVSetUp((s16)a1, (s16)a2);
    D_8008EA22 = (u16)a0;

    if (D_8008E9D0 != 0) {
        i = 0;
        do {
            if (D_8008D996[i].unk0 == (s16)a0) {
                s32 t0 = D_8008D99A[i].unk0;
                if (t0 == (s16)a2 && D_8008D99E[i].unk0 == (s16)a1) {
                    u8 e968FromD998 = D_8008E968[D_8008D998[i].unk0].unk1;
                    u8 e968FromT0 = D_8008E968[t0].unk1;
                    s32 lvl0;
                    s32 prio;
                    s32 lvl1;
                    u32 lvl1b;
                    u32 lvl1c;
                    u32 lvl2;
                    u8 e978c;
                    u8 e978d;
                    u8 e968d;
                    u32 pan1sq;
                    u32 pan2sq;
                    s32 off16;

                    lvl0 = D_8008D990[i].unk0 * (u16)a3 / 127;
                    prio = lvl0 * 0x3FFF;
                    lvl1 = D_8008E970->unk18 * prio / 16129;

                    if (e968FromD998 != e968FromT0) {
                        lvl1b = lvl1 * e968FromT0;
                    } else {
                        lvl1b = lvl1 * e968FromD998;
                    }

                    e978c = D_8008E978[D_8008D99C[i].unk0].unk2;
                    lvl1c = lvl1b * e978c;
                    lvl2 = lvl1c / 16129;

                    pan1 = (lvl2 * e->unk74) / 127;
                    pan2 = (lvl2 * e->unk76) / 127;

                    e978d = D_8008E978[D_8008D99C[i].unk0].unk3;
                    if (e978d < 0x40) {
                        pan2 = (pan2 * e978d) / 63;
                    } else {
                        pan1 = (pan1 * (0x7F - e978d)) / 63;
                    }

                    e968d = D_8008E968[D_8008D998[i].unk0].unk4;
                    if (e968d < 0x40) {
                        pan2 = (pan2 * e968d) / 63;
                    } else {
                        pan1 = (pan1 * (0x7F - e968d)) / 63;
                    }

                    if ((u8)a4 < 0x40) {
                        pan2 = (pan2 * (u8)a4) / 63;
                    } else {
                        pan1 = (pan1 * (0x7F - (u8)a4)) / 63;
                    }

                    pan1sq = pan1 * pan1;
                    if (D_8008E8C0 == 1) {
                        if (pan1 < pan2) {
                            pan1 = pan2;
                        } else {
                            pan2 = pan1;
                        }
                        pan1sq = pan1 * pan1;
                    }
                    pan2sq = pan2 * pan2;

                    off16 = i << 4;
                    *(u16 *)(D_8008D7F0 + off16) = (u16)(pan1sq / 16383);
                    *(u16 *)(D_8008D7F0 + off16 + 2) = (u16)(pan2sq / 16383);

                    result++;
                    D_8008D970[i] |= 3;
                }
            }
            i++;
        } while (i < D_8008E9D0);
    }
    return result;
}
#else
INCLUDE_ASM("asm/nonmatchings/code_179d8_j_b", func_80030980);
#endif
#ifdef NON_MATCHING
/* NON_MATCHING: 252/252 words, LENGTH-EXACT as of round 70 -- the
 * round-63 `--nop-at-expansion` maspsx flag closed the 9/11-word
 * load-delay-nop gap this report's round-62 section attributed to a
 * below-cc1 blocker (see docs/match-reports/func_80030E90.md); that
 * blocker is RESOLVED and this title is no longer "TOOLCHAIN-BLOCKED".
 * Re-measured round 70: raw word-match 65/252, insertions 11 / deletions
 * 11 (was 241/252 11-short, raw 36/252, ins22/del22 before the flag).
 * Residue: the busy-lock guard polarity (retail `bne` NEAR, ours `beq`
 * FAR) plus whatever the flag's extra nops reshuffled elsewhere in the
 * function -- not yet re-characterized past the raw figures. See
 * docs/match-reports/func_80030E90.md's "Round 70 re-measure" section.
 * Hand-derived. */
s32 func_80030E90(s16 p0, s16 p1, s16 p2, s16 p3, u16 p4, s16 p5, s16 p6)
{
    SlotE968 *slot;
    RecordE978 *rec;
    s32 result;
    u16 note;
    u8 pending18;

    if (D_8008E934 == 1) {
        goto fail_nolock;
    }
    D_8008E934 = 1;
    if (SpuVmVSetUp(p0, p1) != 0) {
        goto fail;
    }
    D_8008EA22 = 0x21;
    D_8008EA0E = (u8) p3;
    D_8008EA0F = (u8) p4;
    D_8008EA18 = (u8) p2;
    if (p5 == p6) {
        D_8008EA11 = 0x40;
        D_8008EA10 = p5;
    } else if (p6 < p5) {
        D_8008EA10 = p5;
        D_8008EA11 = (p6 << 6) / p5;
    } else {
        D_8008EA10 = p6;
        D_8008EA11 = 0x7F - ((p5 << 6) / p6);
    }

    slot = D_8008E968;
    D_8008EA16 = slot[p1].unk1;
    D_8008EA17 = slot[p1].unk4;
    D_8008EA0C = slot[p1].unk0;

    rec = &D_8008E978[D_8008EA18 + D_8008EA13 * 16];
    D_8008EA1B = rec->unk0;
    note = rec->unk16;
    D_8008EA24 = note;
    D_8008EA19 = rec->unk2;
    D_8008EA1A = rec->unk3;
    D_8008EA1C = rec->unk4;
    D_8008EA1D = rec->unk5;
    D_8008EA20 = rec->unk1;
    D_8008EA1E = rec->unk6;
    D_8008EA1F = rec->unk7;

    if ((s16) note == 0) {
        goto fail;
    }
    result = (s32)(u8) func_8002CF18();
    if ((u8) result == D_8008E9D0) {
        goto fail;
    }
    __asm__("");
    D_8008EA26 = (u8) result;
    __asm__("");
    D_8008D996[(u8) result].unk0 = 0x21;
    __asm__("");
    D_8008D99E[(u8) result].unk0 = p0;
    __asm__("");
    D_8008D99A[(u8) result].unk0 = p1;
    __asm__("");
    D_8008D998[(u8) result].unk0 = D_8008EA13;
    __asm__("");
    D_8008D988[(u8) result].unk0 = D_8008EA24;
    __asm__("");
    pending18 = D_8008EA18;
    D_8008D994[(u8) result].unk0 = p3;
    D_8008D9A3[(u8) result].unk0 = 1;
    __asm__("");
    D_8008D98A[(u8) result].unk0 = 0;
    __asm__("");
    D_8008D99C[(u8) result].unk0 = pending18;

    func_8002D6A4();
    if ((s16) D_8008EA24 == 0xFF) {
        func_8002D8E0((u8) result);
    } else {
        s32 ret = func_8002E038((u16) p3, p4);
        func_8002D1B4(1, (u16) ret);
    }
    D_8008E934 = 0;
    return (u8) result;

fail:
    D_8008E934 = 0;
fail_nolock:
    return -1;
}
#else
INCLUDE_ASM("asm/nonmatchings/code_179d8_j_b", func_80030E90);
#endif

s32 func_80031280(s16 idx, s16 p1, s16 p2, s16 p3, s16 p4)
{
    u16 chan;
    u32 mask0;
    u16 mask1;

    if (D_8008E934 == 1) {
        goto fail_nolock;
    }
    D_8008E934 = 1;
    if ((u16) idx >= 0x18) {
        goto fail;
    }
    if (D_8008D99E[idx].unk0 != p1
     || D_8008D99A[idx].unk0 != p2
     || D_8008D99C[idx].unk0 != p3
     || D_8008D994[idx].unk0 != p4) {
        goto fail;
    }
    if (D_8008D988[idx].unk0 == 0xFF) {
        D_8008D9A3[(u8) idx].unk0 = 0;
        D_8008D98C[(u8) idx].unk0 = 0;
        D_8006DAD4[25].unk4 = 0;
        D_8006DAD4[25].unk6 = 0;
    } else {
        D_8008EA26 = idx;
        chan = D_8008EA26;
        if (chan < 0x10) {
            mask0 = 1 << chan;
            mask1 = 0;
        } else {
            mask0 = 0;
            mask1 = 1 << (chan - 0x10);
        }
        D_8008D9A3[chan].unk0 = 0;
        D_8008D98C[chan].unk0 = 0;
        D_8008D988[chan].unk0 = 0;
        D_80090C60 = mask0 | D_80090C60;
        D_80090C64 |= mask1;
        D_8008E228 &= ~D_80090C60;
        D_8008E22C &= ~D_80090C64;
    }
    D_8008E934 = 0;
    return 0;

fail:
    D_8008E934 = 0;
fail_nolock:
    return -1;
}

#ifdef NON_MATCHING
/* NON_MATCHING: 248/253 words, 5 words short as of round 70 -- the
 * round-63 `--nop-at-expansion` maspsx flag narrowed this from 16 words
 * short (11 of its 16 missing words were the same below-cc1 load-delay
 * nop func_80030E90's report documents, 11 sites here vs that function's
 * 9); that blocker is RESOLVED and this title is no longer
 * "TOOLCHAIN-BLOCKED". Re-measured round 70 via build/lsdde.map (raw
 * word-match and ins/del are drift-contaminated while the length gap is
 * nonzero, per CLAUDE.md's "address drift" guard -- see
 * docs/match-reports/func_8003149C.md's "Round 70 re-measure" section for
 * the upper-bound figures). Residue: the remaining 5-word gap plus the
 * two guard-polarity residues this report already documents, not yet
 * re-characterized past the length figure. Hand-derived. */
s32 func_8003149C(s16 idx, s16 p0, s16 p1, s16 p2, u16 p3, u16 p4, s16 p5, s16 p6)
{
    RecordE978 *rec;
    u16 note;
    u8 pending18;

    if (D_8008E934 == 1) {
        return -1;
    }
    D_8008E934 = 1;
    if ((u16) idx >= 0x18) {
        goto fail;
    }
    if (SpuVmVSetUp(p0, p1) != 0) {
        goto fail;
    }
    D_8008EA22 = 0x21;
    D_8008EA0E = (u8) p3;
    D_8008EA0F = (u8) p4;
    D_8008EA18 = (u8) p2;
    if (p5 == p6) {
        D_8008EA11 = 0x40;
        D_8008EA10 = p5;
    } else if (p6 < p5) {
        D_8008EA10 = p5;
        D_8008EA11 = (p6 << 6) / p5;
    } else {
        D_8008EA10 = p6;
        D_8008EA11 = 0x7F - ((p5 << 6) / p6);
    }

    D_8008EA16 = D_8008E968[p1].unk1;
    D_8008EA17 = D_8008E968[p1].unk4;
    D_8008EA0C = D_8008E968[p1].unk0;

    rec = &D_8008E978[D_8008EA18 + D_8008EA13 * 16];
    D_8008EA1B = rec->unk0;
    note = rec->unk16;
    D_8008EA24 = note;
    D_8008EA19 = rec->unk2;
    D_8008EA1A = rec->unk3;
    D_8008EA1C = rec->unk4;
    D_8008EA1D = rec->unk5;
    D_8008EA20 = rec->unk1;
    D_8008EA1E = rec->unk6;
    D_8008EA1F = rec->unk7;

    if ((s16) note == 0) {
        goto fail;
    }
    __asm__("");
    D_8008EA26 = idx;
    __asm__("");
    D_8008D996[idx].unk0 = 0x21;
    __asm__("");
    D_8008D99E[idx].unk0 = p0;
    __asm__("");
    D_8008D99A[idx].unk0 = p1;
    __asm__("");
    D_8008D998[idx].unk0 = D_8008EA13;
    __asm__("");
    D_8008D988[idx].unk0 = D_8008EA24;
    __asm__("");
    pending18 = D_8008EA18;
    D_8008D994[idx].unk0 = p3;
    D_8008D9A3[idx].unk0 = 1;
    __asm__("");
    D_8008D98A[idx].unk0 = 0;
    __asm__("");
    D_8008D99C[idx].unk0 = pending18;
    func_8002D6A4();
    if ((s16) D_8008EA24 == 0xFF) {
        func_8002D8E0((u8) idx);
    } else {
        s32 ret = func_8002E038(p3, p4);
        func_8002D1B4(1, (u16) ret);
    }
    D_8008E934 = 0;
    return idx;

fail:
    D_8008E934 = 0;
    return -1;
}
#else
INCLUDE_ASM("asm/nonmatchings/code_179d8_j_b", func_8003149C);
#endif

/* The "release channel" twin of func_80031280's else-branch above: same
 * D_8008E934 lock, same (mask0, mask1) split of a 0..0x17 channel across two
 * 16-bit mask words, same three per-channel field clears, same mask update.
 * MATCHED round 62 by writing it in exactly that sibling's idiom -- direct
 * global expressions with NO cached locals. Four earlier rounds carried four
 * cached locals here (old60/old64/e228/e22c) and filed the result as an
 * unreachable register-identity stall; the caching was the whole residue.
 * The only structural difference from the sibling is that the lock is
 * released BEFORE the mask block rather than after it (retail's
 * `sw zero, D_8008E934` sits at 0x80031950, between the D_8008E228 load and
 * the first `or`). See docs/match-reports/func_80031890.md. */
s32 func_80031890(s16 idx)
{
    u16 chan;
    u32 mask0;
    u16 mask1;

    if (D_8008E934 == 1) {
        goto fail_nolock;
    }
    D_8008E934 = 1;
    if ((u16) idx >= 0x18) {
        goto fail;
    }
    D_8008EA26 = idx;
    chan = D_8008EA26;
    if (chan < 0x10) {
        mask0 = 1 << chan;
        mask1 = 0;
    } else {
        mask0 = 0;
        mask1 = 1 << (chan - 0x10);
    }
    D_8008D9A3[chan].unk0 = 0;
    D_8008D98C[chan].unk0 = 0;
    D_8008D988[chan].unk0 = 0;
    D_8008E934 = 0;
    D_80090C60 = mask0 | D_80090C60;
    D_80090C64 |= mask1;
    D_8008E228 &= ~D_80090C60;
    D_8008E22C &= ~D_80090C64;
    return 0;

fail:
    D_8008E934 = 0;
fail_nolock:
    return -1;
}
