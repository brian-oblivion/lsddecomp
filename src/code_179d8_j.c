/*
 * code_179d8_j -- eight functions of Sony's libsnd voice manager
 * (libsnd/vmanager), carried as C because retail's build of vmanager is on
 * no SDK disc, so it never placed as an object. progress.py counts them as
 * library by address, and they keep Sony's names.
 *
 *   SpuVmSeKeyOn    keys on a sound effect: SpuVmKeyOn on the SE pseudo-
 *                   sequence, with the louder of the two channel volumes as
 *                   the volume and their ratio as the pan.
 *   SpuVmSeKeyOff   keys off a sound effect SpuVmSeKeyOn started.
 *   KeyOnCheck      vmanager's empty internal hook.
 *   SpuVmSetSeqVol  sets one sequence's L/R volume (SsScore unk74/unk76,
 *                   clamped to 127) and rescales the voices it is playing.
 *   SpuVmGetSeqVol, SpuVmGetSeqLVol, SpuVmGetSeqRVol
 *                   read that volume back.
 *   SpuVmSeqKeyOff  keys off every voice one sequence is playing.
 *
 * A sequence is named by one packed number: the SEQ/SEP access in the low
 * byte, the sequence within it in the high byte, reaching
 * _ss_score[access][seq] (include/SsScore.h). Each volume accessor
 * records it in D_8008EA22, vmanager's current-sequence global
 * (SpuVmGetSeqLVol records only the access byte).
 *
 * Edges (python3 tools/tuboundary.py): code_179d8_m before it ("start edge
 * possible"), the placed object libsnd/vm_prog after it; every edge inside
 * is "boundary possible".
 */

#include "common.h"
#include "SsScore.h"

/* vmanager internals, absent from <libsnd.h>; signatures read from the
 * registers each call site loads. */
extern s32 SpuVmKeyOn(s32 seqSepNo, s16 vabId, s16 prog, u16 note, u16 vol, u16 pan);
extern s32 SpuVmKeyOff(s32 seqSepNo, s16 vabId, s16 prog, u16 note);

/* The packed number of the sequence vmanager is working on. */
extern u16 D_8008EA22;

/* The pseudo-sequence number sound effects are keyed on and off under;
 * libsnd's SE paths store it in D_8008EA22 and in a voice's owner field. */
#define SPUVM_SE_SEQ 0x21

/* Keys on note of vabId's program prog as a sound effect. The pan is 64
 * (centre) for equal volumes and leans toward the louder side by the ratio
 * of the quieter to the louder, 64ths of the way; the fourth argument is
 * unused. */
s32 SpuVmSeKeyOn(s32 vabId, s32 prog, s32 note, s32 unused, u16 volL, u16 volR) {
    u16 vol;
    u16 pan;

    if (volL == volR) {
        pan = 64;
        vol = volL;
    } else if (volR < volL) {
        vol = volL;
        pan = (volR << 6) / volL;
    } else {
        vol = volR;
        pan = 127 - ((volL << 6) / volR);
    }
    return SpuVmKeyOn(SPUVM_SE_SEQ, (s16)vabId, (s16)prog, (u16)note, vol, pan);
}

s32 SpuVmSeKeyOff(s16 vabId, s16 prog, u16 note) {
    return SpuVmKeyOff(SPUVM_SE_SEQ, vabId, prog, note);
}

void KeyOnCheck(void) {}

INCLUDE_ASM("asm/nonmatchings/code_179d8_j", SpuVmSetSeqVol);

/* Returns the packed number, read back through D_8008EA22. */
s32 SpuVmGetSeqVol(s32 seqSepNo, s16 *volL, s16 *volR) {
    SsScore *seqs = _ss_score[(u8)seqSepNo];
    s16 *cur = (s16 *)&D_8008EA22;

    *cur = (s16)seqSepNo;
    *volL = seqs[(seqSepNo & 0xFF00) >> 8].unk74;
    *volR = seqs[(seqSepNo & 0xFF00) >> 8].unk76;
    return *cur;
}

s32 SpuVmGetSeqLVol(s32 seqSepNo) {
    s32 access = seqSepNo & 0xFF;
    SsScore *seqs = _ss_score[access];
    s32 seq = (seqSepNo & 0xFF00) >> 8;

    /* MATCHING: keeps the D_8008EA22 store after the index arithmetic;
     * without it GCC hoists the store to the top. */
    __asm__("");
    D_8008EA22 = access;
    /* MATCHING: s16, for retail's sign-extending lh of the u16 field. */
    return (s16)seqs[seq].unk74;
}

s32 SpuVmGetSeqRVol(s32 seqSepNo) {
    SsScore *seqs = _ss_score[(u8)seqSepNo];

    /* MATCHING: keeps the _ss_score load above the D_8008EA22 store;
     * without it the load sinks below the store and the index arithmetic. */
    __asm__("");
    D_8008EA22 = seqSepNo;
    /* MATCHING: s16, for retail's sign-extending lh of the u16 field. */
    return (s16)seqs[(seqSepNo & 0xFF00) >> 8].unk76;
}

INCLUDE_ASM("asm/nonmatchings/code_179d8_j", SpuVmSeqKeyOff);
