/*
 * libsnd_vm_vol_ut_key_ut_keyv -- five functions of Sony's libsnd voice
 * manager, carried as C: SpuVmSetVol, SsUtKeyOn, SsUtKeyOff, SsUtKeyOnV
 * and SsUtKeyOffV. On the 3.0/3.3/3.5 discs all five sit in one object,
 * vmanager.o; on the 3.6 disc they are three, vm_vol.o (SpuVmSetVol),
 * ut_key.o (SsUtKeyOn, SsUtKeyOff) and ut_keyv.o (SsUtKeyOnV, SsUtKeyOffV),
 * in this order, and the file is named for those three because its
 * neighbours are 3.6-only split modules. Retail's build of them is on no
 * SDK disc, so they never placed as objects; progress.py counts these
 * functions as library by address, and they keep Sony's names and
 * <libsnd.h>'s prototypes.
 *
 * What decided its edges (python3 tools/tuboundary.py): the placed object
 * libsnd/vm_prog precedes it ("start edge possible") and the placed object
 * libsnd/ut_pb follows it ("start edge possible"), so there is nothing to
 * merge with. Inside, every edge is "boundary possible": the binary neither
 * proves nor forbids a file boundary. PARKED: the content says three files
 * (after SpuVmSetVol and after SsUtKeyOff, the 3.6 module edges), but that
 * split is a new carve, not a merge or rename, so the file keeps its carve
 * edges and is named for all three modules.
 *
 *   SpuVmSetVol  rescales every voice one sequence plays on a given VAB and
 *                program: voice level x the VAB's, the program's and the
 *                tone's volumes x the sequence's L/R volume (SsScore),
 *                panned by the tone, the program and the caller, into the
 *                voices' shadow volume registers (_svm_sreg_buf), marked
 *                dirty.
 *   SsUtKeyOn    Sony's utility key-on: stages the program's and tone's
 *                attributes (_svm_pg, _svm_tn) in the vmanager's scratch
 *                globals, allocates a voice, fills its _svm_voice record and
 *                keys it on (vmNoiseOn for a noise tone, vag 0xFF).
 *   SsUtKeyOff   keys off the voice SsUtKeyOn returned, if the voice still
 *                holds the same VAB/program/tone/note: a noise voice clears
 *                the SPU's noise-mode enable, any other sets its bit in the
 *                pending key-off mask and drops the pending key-offs from
 *                the key-on mask.
 *   SsUtKeyOnV   SsUtKeyOn on a caller-chosen voice.
 *   SsUtKeyOffV  keys off a voice unconditionally.
 *
 * The four SsUt functions take the _snd_ev_flag lock and return -1 while
 * it is held. SpuVmSetVol, SsUtKeyOn and SsUtKeyOnV are carried as INCLUDE_ASM
 * with their best readable body under NON_MATCHING.
 */

#include "common.h"
#include <libsnd.h>
#include "SsScore.h"
#include "SvmData.h"

/* libsnd vmanager's _svm_tn (pinned at this address): the current VAB's
 * tone table, 16 VagAtr per program, indexed prog * 16 + tone. */
extern VagAtr *D_8008E978;

/* Holds the base of the SPU register block, 0x1F801C00, indexed in
 * halfwords as code_179d8_p.c's SsUtAllKeyOff indexes it.
 * MATCHING: not volatile here; volatile moves SsUtKeyOff's second store out
 * of its branch delay slot. */
extern u16 *D_8006DAD4;

/* The SPU's noise-mode enable register pair (NON, 0x1F801D94/0x1F801D96:
 * one bit per voice, voices 0-15 then 16-23), as halfword indices from
 * that base. */
#define SPU_NOISE_ON_LO (0x194 / 2)
#define SPU_NOISE_ON_HI (0x196 / 2)

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
extern ProgAtr *_svm_pg;

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

/* libsnd vmanager's _svm_vh (pinned at this address): the current VAB's
 * header. */
extern VabHdr *_svm_vh;

extern s16 D_8008E8C0;

#ifdef NON_MATCHING
/* NON_MATCHING: 315/324 words, 9 words short; raw word-match 10/324,
 * insertions 76 / deletions 76 (re-measured round 70, unchanged since
 * round 62). Residue: one GCC CSE decision on the
 * `D_8008E978[_svm_voice[i].unk14]` address plus two loop-invariant
 * hoists (docs/match-reports/SpuVmSetVol.md). Hand-derived. */
s32 SpuVmSetVol(s32 a0, s32 a1, s32 a2, s32 a3, u16 a4) {
    SsScore *e;
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
                    u8 e968FromD998 = _svm_pg[_svm_voice[i].unk10].mvol;
                    u8 e968FromT0 = _svm_pg[t0].mvol;
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
                    lvl1 = _svm_vh->mvol * prio / 16129;

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

                    e968d = _svm_pg[_svm_voice[i].unk10].mpan;
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
INCLUDE_ASM("asm/nonmatchings/libsnd_vm_vol_ut_key_ut_keyv", SpuVmSetVol);
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

    slot = _svm_pg;
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
INCLUDE_ASM("asm/nonmatchings/libsnd_vm_vol_ut_key_ut_keyv", SsUtKeyOn);
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
        D_8006DAD4[SPU_NOISE_ON_LO] = 0;
        D_8006DAD4[SPU_NOISE_ON_HI] = 0;
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

    D_8008EA16 = _svm_pg[p1].mvol;
    D_8008EA17 = _svm_pg[p1].mpan;
    D_8008EA0C = _svm_pg[p1].tones;

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
INCLUDE_ASM("asm/nonmatchings/libsnd_vm_vol_ut_key_ut_keyv", SsUtKeyOnV);
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
