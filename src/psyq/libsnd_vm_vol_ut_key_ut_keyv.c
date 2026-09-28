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
#include "libsnd_internal.h"

/* The SPU register block, 0x1F801C00, indexed in halfwords as
 * libsnd_ut_ako.c's SsUtAllKeyOff indexes it.
 * MATCHING: not volatile here; volatile moves SsUtKeyOff's second store out
 * of its branch delay slot. */
extern u16 *_svm_sreg;

/* The SPU's noise-mode enable register pair (NON, 0x1F801D94/0x1F801D96:
 * one bit per voice, voices 0-15 then 16-23), as halfword indices from
 * that base. */
#define SPU_NOISE_ON_LO (0x194 / 2)
#define SPU_NOISE_ON_HI (0x196 / 2)

/* _svm_cur + 0x0C, the tone within the program (the rest of _svm_cur is in
 * libsnd_internal.h). MATCHING: volatile here; plain changes SsUtKeyOnV's
 * NON_MATCHING body by a word. */
extern volatile u8 D_8008EA18;

#ifdef NON_MATCHING
/* NON_MATCHING: 315/324 words, 9 words short. Residue: one GCC CSE
 * decision on the `_svm_tn[_svm_voice[i].tone]` address plus two
 * loop-invariant hoists (docs/match-reports/SpuVmSetVol.md). */
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

    if (spuVmMaxVoice != 0) {
        i = 0;
        do {
            if (_svm_voice[i].seq == (s16)a0) {
                s32 t0 = _svm_voice[i].prog;
                if (t0 == (s16)a2 && _svm_voice[i].vabId == (s16)a1) {
                    u8 e968FromD998 = _svm_pg[_svm_voice[i].progIndex].mvol;
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

                    lvl0 = _svm_voice[i].vol * (u16)a3 / 127;
                    prio = lvl0 * 0x3FFF;
                    lvl1 = _svm_vh->mvol * prio / 16129;

                    if (e968FromD998 != e968FromT0) {
                        lvl1b = lvl1 * e968FromT0;
                    } else {
                        lvl1b = lvl1 * e968FromD998;
                    }

                    e978c = _svm_tn[_svm_voice[i].tone].vol;
                    lvl1c = lvl1b * e978c;
                    lvl2 = lvl1c / 16129;

                    pan1 = (lvl2 * e->unk74) / 127;
                    pan2 = (lvl2 * e->unk76) / 127;

                    e978d = _svm_tn[_svm_voice[i].tone].pan;
                    if (e978d < 0x40) {
                        pan2 = (pan2 * e978d) / 63;
                    } else {
                        pan1 = (pan1 * (0x7F - e978d)) / 63;
                    }

                    e968d = _svm_pg[_svm_voice[i].progIndex].mpan;
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
                    if (_svm_stereo_mono == 1) {
                        if (pan1 < pan2) {
                            pan1 = pan2;
                        } else {
                            pan2 = pan1;
                        }
                        pan1sq = pan1 * pan1;
                    }
                    pan2sq = pan2 * pan2;

                    _svm_sreg_buf[i].volL = (u16)(pan1sq / 16383);
                    _svm_sreg_buf[i].volR = (u16)(pan2sq / 16383);

                    result++;
                    _svm_sreg_dirty[i] |= SVM_SREG_DIRTY_VOL;
                }
            }
            i++;
        } while (i < spuVmMaxVoice);
    }
    return result;
}
#else
INCLUDE_ASM("asm/nonmatchings/psyq/libsnd_vm_vol_ut_key_ut_keyv", SpuVmSetVol);
#endif
#ifdef NON_MATCHING
/* NON_MATCHING: 252/252 words, length exact. Residue: the busy-lock
 * guard's branch polarity, and more not yet characterised
 * (docs/match-reports/SsUtKeyOn.md). */
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

    rec = &_svm_tn[D_8008EA18 + D_8008EA13 * 16];
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
    if ((u8)result == spuVmMaxVoice) {
        goto fail;
    }
    __asm__("");
    D_8008EA26 = (u8)result;
    __asm__("");
    _svm_voice[(u8)result].seq = 0x21;
    __asm__("");
    _svm_voice[(u8)result].vabId = p0;
    __asm__("");
    _svm_voice[(u8)result].prog = p1;
    __asm__("");
    _svm_voice[(u8)result].progIndex = D_8008EA13;
    __asm__("");
    _svm_voice[(u8)result].vag = D_8008EA24;
    __asm__("");
    pending18 = D_8008EA18;
    _svm_voice[(u8)result].note = p3;
    _svm_voice[(u8)result].keyState = 1;
    __asm__("");
    _svm_voice[(u8)result].age = 0;
    __asm__("");
    _svm_voice[(u8)result].tone = pending18;

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
INCLUDE_ASM("asm/nonmatchings/psyq/libsnd_vm_vol_ut_key_ut_keyv", SsUtKeyOn);
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
    if (_svm_voice[idx].vabId != p1 || _svm_voice[idx].prog != p2 || _svm_voice[idx].tone != p3 ||
        _svm_voice[idx].note != p4) {
        goto fail;
    }
    if (_svm_voice[idx].vag == 0xFF) {
        _svm_voice[(u8)idx].keyState = 0;
        _svm_voice[(u8)idx].pitch = 0;
        _svm_sreg[SPU_NOISE_ON_LO] = 0;
        _svm_sreg[SPU_NOISE_ON_HI] = 0;
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
        _svm_voice[chan].keyState = 0;
        _svm_voice[chan].pitch = 0;
        _svm_voice[chan].vag = 0;
        _svm_okof1 = mask0 | _svm_okof1;
        _svm_okof2 |= mask1;
        _svm_okon1 &= ~_svm_okof1;
        _svm_okon2 &= ~_svm_okof2;
    }
    _snd_ev_flag = 0;
    return 0;

fail:
    _snd_ev_flag = 0;
fail_nolock:
    return -1;
}

#ifdef NON_MATCHING
/* NON_MATCHING: 248/253 words, 5 words short. Residue: the busy-lock
 * guard's branch polarity and a second guard flip
 * (docs/match-reports/SsUtKeyOnV.md). */
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

    rec = &_svm_tn[D_8008EA18 + D_8008EA13 * 16];
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
    _svm_voice[idx].seq = 0x21;
    __asm__("");
    _svm_voice[idx].vabId = p0;
    __asm__("");
    _svm_voice[idx].prog = p1;
    __asm__("");
    _svm_voice[idx].progIndex = D_8008EA13;
    __asm__("");
    _svm_voice[idx].vag = D_8008EA24;
    __asm__("");
    pending18 = D_8008EA18;
    _svm_voice[idx].note = p3;
    _svm_voice[idx].keyState = 1;
    __asm__("");
    _svm_voice[idx].age = 0;
    __asm__("");
    _svm_voice[idx].tone = pending18;
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
INCLUDE_ASM("asm/nonmatchings/psyq/libsnd_vm_vol_ut_key_ut_keyv", SsUtKeyOnV);
#endif

/* The "release channel" twin of SsUtKeyOff's else-branch above: same
 * _snd_ev_flag lock, same (mask0, mask1) split of a 0..0x17 channel across two
 * 16-bit mask words, same three per-channel field clears, same mask update.
 * MATCHING: written in that sibling's idiom, direct global expressions with
 * no cached locals. Unlike the sibling it releases the lock before the mask
 * block rather than after it. */
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
    _svm_voice[chan].keyState = 0;
    _svm_voice[chan].pitch = 0;
    _svm_voice[chan].vag = 0;
    _snd_ev_flag = 0;
    _svm_okof1 = mask0 | _svm_okof1;
    _svm_okof2 |= mask1;
    _svm_okon1 &= ~_svm_okof1;
    _svm_okon2 &= ~_svm_okof2;
    return 0;

fail:
    _snd_ev_flag = 0;
fail_nolock:
    return -1;
}
