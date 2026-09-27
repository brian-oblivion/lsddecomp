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
 * (SpuVmSetVol .. SsUtKeyOffV).
 *
 * WHY THIS UNIT EXISTS, IN TWO STEPS, BOTH IN ROUND 34.
 *   1. `libsnd/vm_prog.o` (Psy-Q 3.6 -- the only disc carrying the module)
 *      covers 0x20FF0..0x21180: SpuVmSetProgVol, SpuVmGetProgVol,
 *      SpuVmSetProgPan, SpuVmGetProgPan, all four previously MATCHED as C.
 *      That split the old code_179d8_j into [c][o][c] and created this file.
 *   2. `libsnd/ut_pb.o` (Psy-Q 3.6 only, 0x90) covers 0x221B4..0x22244:
 *      `SsUtPitchBend`, which was `func_800319B4`, also previously MATCHED.
 *      That split THIS file again into [c][o][c], and everything from
 *      SsUtChangePitch on moved to `src/code_179d8_j_c.c`.
 * Reclassifying five matched functions out of the game count across the two
 * steps is the correction CLAUDE.md asks for, not a regression; their C is
 * DELETED, not commented out.
 *
 * `D_8008EA22`'S OWN COMMENT WAS WRONG AND IS CORRECTED HERE (round 36): it
 * used to claim `SsUtPitchBend` and the deleted `SpuVmGetSeqRVol` were its
 * only readers in this family, and dropped the extern on that basis. That
 * was never true of this file's own two remaining stalls -- SsUtKeyOn
 * and SsUtKeyOnV both WRITE it (`D_8008EA22 = 0x21;`) -- it just went
 * unnoticed because both were still INCLUDE_ASM and nothing failed to
 * link. Declared again below.
 *
 * NO RODATA ATTACH IS OWNED BY ANY UNIT IN THIS FAMILY, and that is measured,
 * not assumed: the old code_179d8_mid_c monolith contains zero `jtbl_` and
 * zero `.word .L` across its whole extent, and the splat yaml's rodata slot
 * list names none of `code_179d8_j`, `_j_b` or `_j_c`.  Unlike round 33's
 * libsnd_ssinit_libapi_counter there was nothing to move, and a link failure of the form
 * `undefined reference to '.L8003....'` would mean something else.
 *
 * DECLARATIONS: this file carries its own copy of what its functions use,
 * split out of the old shared block.  Keep it that way -- do NOT create a
 * shared code_179d8*.h.  The sibling slices are staffed independently and a
 * shared header is what makes their merges collide; see
 * `python3 tools/headercontention.py`.  Several externs below are read only by
 * functions still carried as INCLUDE_ASM (the `SsUtKeyOn` scratch globals
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
#include <libsnd.h>
#include "SvmData.h"

/* libsnd vmanager's _svm_tn (pinned at this address): the current VAB's
 * tone table, 16 VagAtr per program, indexed prog * 16 + tone. */
extern VagAtr *D_8008E978;

/* Base pointer for a table of 0x10-byte entries, indexed by a 0..0x17
 * id.  Only the two leading s16 fields this unit's own accessors touch
 * are named. */
typedef struct EntryDAD4 {
    s16 unk0; /* +0x0 */
    s16 unk2; /* +0x2 */
    s16 unk4; /* +0x4 -- read by SsUtKeyOff, entry index 25 only */
    s16 unk6; /* +0x6 -- read by SsUtKeyOff, entry index 25 only */
    u8 pad8[0x10 - 0x8];
} EntryDAD4;

extern EntryDAD4 *D_8006DAD4;

/* Reentrancy lock, same identifier/type as the sibling reading in
 * Sony's `SsSeqCalledTbyT` (`libsnd/sscall`, linked since round 34; it was
 * code_179d8_i.c's matched func_80033738) -- "if already busy, return/skip;
 * set; ...; clear before returning" guarding a per-channel operation. */
extern s32 _snd_ev_flag;

/* "Currently selected channel" scratch globals: written as a side
 * effect and then re-read from the global (not from the parameter) by
 * the same and sibling functions -- same idiom as this file's own
 * D_8008EA22 above. */
extern volatile u16 D_8008EA26;
extern volatile u8 D_8008EA18;
extern u16 D_8008EA22;

/* libsnd vmanager's _svm_pg (pinned at this address): the current VAB's
 * program table, indexed by program number. */
extern ProgAtr *D_8008E968;

/* SsUtKeyOn's own scratch globals -- a "start channel" setup
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

extern s32 SpuVmAlloc(void);
extern void SpuVmDoAllocate(void);
extern void vmNoiseOn(s32 a0);
extern s32 note2pitch2(u16 a0, u16 a1);
extern void SpuVmKeyOnNow(s32 a0, u16 a1);
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

/* Shared with vmNoiseOn/SpuVmKeyOnNow in code_179d8_l.c (same
 * two-level entry table, same blend-cascade shape); this unit's own
 * reduced view, per the project's per-unit-local-view convention --
 * only the two fields SpuVmSetVol itself touches are named. */
typedef struct {
    u8 pad0[0x74];
    u16 unk74;
    u16 unk76;
    u8 pad78[0xAC - 0x78];
} D800902E8Entry;

extern D800902E8Entry *_ss_score[];

/* libsnd vmanager's _svm_vh (pinned at this address): the current VAB's
 * header. */
extern VabHdr *D_8008E970;

extern s16 D_8008E8C0;

#ifdef NON_MATCHING
/* NON_MATCHING: 315/324 words, 9 words short; raw word-match 10/324,
 * insertions 76 / deletions 76 (re-measured round 70, unchanged since
 * round 62). Residue: one GCC CSE decision on the
 * `D_8008E978[_svm_voice[i].unk14]` address plus two loop-invariant
 * hoists (docs/match-reports/SpuVmSetVol.md). Hand-derived. */
s32 SpuVmSetVol(s32 a0, s32 a1, s32 a2, s32 a3, u16 a4) {
    D800902E8Entry *e;
    u8 i;
    s32 result;
    u32 pan1;
    u32 pan2;

    result = 0;
    e = &_ss_score[a0 & 0xFF][(a0 & 0xFF00) >> 8];
    SpuVmVSetUp((s16)a1, (s16)a2);
    D_8008EA22 = (u16)a0;

    if (D_8008E9D0 != 0) {
        i = 0;
        do {
            if (_svm_voice[i].unk0E == (s16)a0) {
                s32 t0 = _svm_voice[i].unk12;
                if (t0 == (s16)a2 && _svm_voice[i].unk16 == (s16)a1) {
                    u8 e968FromD998 = D_8008E968[_svm_voice[i].unk10].mvol;
                    u8 e968FromT0 = D_8008E968[t0].mvol;
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

                    lvl0 = _svm_voice[i].unk08 * (u16)a3 / 127;
                    prio = lvl0 * 0x3FFF;
                    lvl1 = D_8008E970->mvol * prio / 16129;

                    if (e968FromD998 != e968FromT0) {
                        lvl1b = lvl1 * e968FromT0;
                    } else {
                        lvl1b = lvl1 * e968FromD998;
                    }

                    e978c = D_8008E978[_svm_voice[i].unk14].vol;
                    lvl1c = lvl1b * e978c;
                    lvl2 = lvl1c / 16129;

                    pan1 = (lvl2 * e->unk74) / 127;
                    pan2 = (lvl2 * e->unk76) / 127;

                    e978d = D_8008E978[_svm_voice[i].unk14].pan;
                    if (e978d < 0x40) {
                        pan2 = (pan2 * e978d) / 63;
                    } else {
                        pan1 = (pan1 * (0x7F - e978d)) / 63;
                    }

                    e968d = D_8008E968[_svm_voice[i].unk10].mpan;
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

                    _svm_sreg_buf[i].unk0 = (u16)(pan1sq / 16383);
                    _svm_sreg_buf[i].unk2 = (u16)(pan2sq / 16383);

                    result++;
                    _svm_sreg_dirty[i] |= 3;
                }
            }
            i++;
        } while (i < D_8008E9D0);
    }
    return result;
}
#else
INCLUDE_ASM("asm/nonmatchings/code_179d8_j_b", SpuVmSetVol);
#endif
#ifdef NON_MATCHING
/* NON_MATCHING: 252/252 words, length exact; raw word-match 65/252,
 * insertions 11 / deletions 11 (re-measured round 70). Residue: the
 * busy-lock guard's branch polarity, with the rest not re-characterised
 * since `--nop-at-expansion` closed the old length gap
 * (docs/match-reports/SsUtKeyOn.md). Hand-derived. */
s16 SsUtKeyOn(s16 p0, s16 p1, s16 p2, s16 p3, s16 p4, s16 p5, s16 p6) {
    ProgAtr *slot;
    VagAtr *rec;
    s32 result;
    u16 note;
    u8 pending18;

    if (_snd_ev_flag == 1) {
        goto fail_nolock;
    }
    _snd_ev_flag = 1;
    if (SpuVmVSetUp(p0, p1) != 0) {
        goto fail;
    }
    D_8008EA22 = 0x21;
    D_8008EA0E = (u8)p3;
    D_8008EA0F = (u8)p4;
    D_8008EA18 = (u8)p2;
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
    D_8008EA16 = slot[p1].mvol;
    D_8008EA17 = slot[p1].mpan;
    D_8008EA0C = slot[p1].tones;

    rec = &D_8008E978[D_8008EA18 + D_8008EA13 * 16];
    D_8008EA1B = rec->prior;
    note = rec->vag;
    D_8008EA24 = note;
    D_8008EA19 = rec->vol;
    D_8008EA1A = rec->pan;
    D_8008EA1C = rec->center;
    D_8008EA1D = rec->shift;
    D_8008EA20 = rec->mode;
    D_8008EA1E = rec->min;
    D_8008EA1F = rec->max;

    if ((s16)note == 0) {
        goto fail;
    }
    result = (s32)(u8)SpuVmAlloc();
    if ((u8)result == D_8008E9D0) {
        goto fail;
    }
    __asm__("");
    D_8008EA26 = (u8)result;
    __asm__("");
    _svm_voice[(u8)result].unk0E = 0x21;
    __asm__("");
    _svm_voice[(u8)result].unk16 = p0;
    __asm__("");
    _svm_voice[(u8)result].unk12 = p1;
    __asm__("");
    _svm_voice[(u8)result].unk10 = D_8008EA13;
    __asm__("");
    _svm_voice[(u8)result].unk00 = D_8008EA24;
    __asm__("");
    pending18 = D_8008EA18;
    _svm_voice[(u8)result].unk0C = p3;
    _svm_voice[(u8)result].unk1B = 1;
    __asm__("");
    _svm_voice[(u8)result].unk02 = 0;
    __asm__("");
    _svm_voice[(u8)result].unk14 = pending18;

    SpuVmDoAllocate();
    if ((s16)D_8008EA24 == 0xFF) {
        vmNoiseOn((u8)result);
    } else {
        s32 ret = note2pitch2((u16)p3, (u16)p4);
        SpuVmKeyOnNow(1, (u16)ret);
    }
    _snd_ev_flag = 0;
    return (u8)result;

fail:
    _snd_ev_flag = 0;
fail_nolock:
    return -1;
}
#else
INCLUDE_ASM("asm/nonmatchings/code_179d8_j_b", SsUtKeyOn);
#endif

s16 SsUtKeyOff(s16 idx, s16 p1, s16 p2, s16 p3, s16 p4) {
    u16 chan;
    u32 mask0;
    u16 mask1;

    if (_snd_ev_flag == 1) {
        goto fail_nolock;
    }
    _snd_ev_flag = 1;
    if ((u16)idx >= 0x18) {
        goto fail;
    }
    if (_svm_voice[idx].unk16 != p1 || _svm_voice[idx].unk12 != p2 || _svm_voice[idx].unk14 != p3 ||
        _svm_voice[idx].unk0C != p4) {
        goto fail;
    }
    if (_svm_voice[idx].unk00 == 0xFF) {
        _svm_voice[(u8)idx].unk1B = 0;
        _svm_voice[(u8)idx].unk04 = 0;
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
        _svm_voice[chan].unk1B = 0;
        _svm_voice[chan].unk04 = 0;
        _svm_voice[chan].unk00 = 0;
        D_80090C60 = mask0 | D_80090C60;
        D_80090C64 |= mask1;
        D_8008E228 &= ~D_80090C60;
        D_8008E22C &= ~D_80090C64;
    }
    _snd_ev_flag = 0;
    return 0;

fail:
    _snd_ev_flag = 0;
fail_nolock:
    return -1;
}

#ifdef NON_MATCHING
/* NON_MATCHING: 248/253 words, 5 words short (re-measured round 70).
 * Residue: the busy-lock guard's branch polarity and a second guard
 * flip; the 5-word gap is not re-characterised since
 * `--nop-at-expansion` closed 11 of the old 16
 * (docs/match-reports/SsUtKeyOnV.md). Hand-derived. */
s16 SsUtKeyOnV(s16 idx, s16 p0, s16 p1, s16 p2, s16 p3, s16 p4, s16 p5, s16 p6) {
    VagAtr *rec;
    u16 note;
    u8 pending18;

    if (_snd_ev_flag == 1) {
        return -1;
    }
    _snd_ev_flag = 1;
    if ((u16)idx >= 0x18) {
        goto fail;
    }
    if (SpuVmVSetUp(p0, p1) != 0) {
        goto fail;
    }
    D_8008EA22 = 0x21;
    D_8008EA0E = (u8)p3;
    D_8008EA0F = (u8)p4;
    D_8008EA18 = (u8)p2;
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

    D_8008EA16 = D_8008E968[p1].mvol;
    D_8008EA17 = D_8008E968[p1].mpan;
    D_8008EA0C = D_8008E968[p1].tones;

    rec = &D_8008E978[D_8008EA18 + D_8008EA13 * 16];
    D_8008EA1B = rec->prior;
    note = rec->vag;
    D_8008EA24 = note;
    D_8008EA19 = rec->vol;
    D_8008EA1A = rec->pan;
    D_8008EA1C = rec->center;
    D_8008EA1D = rec->shift;
    D_8008EA20 = rec->mode;
    D_8008EA1E = rec->min;
    D_8008EA1F = rec->max;

    if ((s16)note == 0) {
        goto fail;
    }
    __asm__("");
    D_8008EA26 = idx;
    __asm__("");
    _svm_voice[idx].unk0E = 0x21;
    __asm__("");
    _svm_voice[idx].unk16 = p0;
    __asm__("");
    _svm_voice[idx].unk12 = p1;
    __asm__("");
    _svm_voice[idx].unk10 = D_8008EA13;
    __asm__("");
    _svm_voice[idx].unk00 = D_8008EA24;
    __asm__("");
    pending18 = D_8008EA18;
    _svm_voice[idx].unk0C = p3;
    _svm_voice[idx].unk1B = 1;
    __asm__("");
    _svm_voice[idx].unk02 = 0;
    __asm__("");
    _svm_voice[idx].unk14 = pending18;
    SpuVmDoAllocate();
    if ((s16)D_8008EA24 == 0xFF) {
        vmNoiseOn((u8)idx);
    } else {
        s32 ret = note2pitch2((u16)p3, (u16)p4);
        SpuVmKeyOnNow(1, (u16)ret);
    }
    _snd_ev_flag = 0;
    return idx;

fail:
    _snd_ev_flag = 0;
    return -1;
}
#else
INCLUDE_ASM("asm/nonmatchings/code_179d8_j_b", SsUtKeyOnV);
#endif

/* The "release channel" twin of SsUtKeyOff's else-branch above: same
 * _snd_ev_flag lock, same (mask0, mask1) split of a 0..0x17 channel across two
 * 16-bit mask words, same three per-channel field clears, same mask update.
 * MATCHED round 62 by writing it in exactly that sibling's idiom -- direct
 * global expressions with NO cached locals. Four earlier rounds carried four
 * cached locals here (old60/old64/e228/e22c) and filed the result as an
 * unreachable register-identity stall; the caching was the whole residue.
 * The only structural difference from the sibling is that the lock is
 * released BEFORE the mask block rather than after it (retail's
 * `sw zero, _snd_ev_flag` sits at 0x80031950, between the D_8008E228 load and
 * the first `or`). See docs/match-reports/SsUtKeyOffV.md. */
s16 SsUtKeyOffV(s16 idx) {
    u16 chan;
    u32 mask0;
    u16 mask1;

    if (_snd_ev_flag == 1) {
        goto fail_nolock;
    }
    _snd_ev_flag = 1;
    if ((u16)idx >= 0x18) {
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
    _svm_voice[chan].unk1B = 0;
    _svm_voice[chan].unk04 = 0;
    _svm_voice[chan].unk00 = 0;
    _snd_ev_flag = 0;
    D_80090C60 = mask0 | D_80090C60;
    D_80090C64 |= mask1;
    D_8008E228 &= ~D_80090C60;
    D_8008E22C &= ~D_80090C64;
    return 0;

fail:
    _snd_ev_flag = 0;
fail_nolock:
    return -1;
}
