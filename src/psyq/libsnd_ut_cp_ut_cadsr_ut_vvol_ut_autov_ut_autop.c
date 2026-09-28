/*
 * libsnd_ut_cp_ut_cadsr_ut_vvol_ut_autov_ut_autop -- eight of Sony's libsnd
 * per-voice utility calls, carried as C because no SDK disc carries the
 * build retail linked. tools/progress.py counts every function here as
 * library, so each keeps Sony's name and <libsnd.h>'s prototype.
 *
 * Which objects (nm over sdk/work/<disc>/elf/libsnd): on the 3.0, 3.3 and
 * 3.5 discs all eight sit in one object, vmanager.o; on the 3.6 disc they
 * are five, in retail's order: ut_cp.o (SsUtChangePitch), ut_cadsr.o
 * (SsUtChangeADSR), ut_vvol.o (SsUtGetDetVVol, SsUtSetDetVVol, SsUtGetVVol,
 * SsUtSetVVol), ut_autov.o (SsUtAutoVol) and ut_autop.o (SsUtAutoPan). The
 * file is named for those five because retail's libsnd is the split build:
 * the 3.6-only libsnd/ut_pb object is linked right before it. None of the
 * five is retail's build (3.6's text is 0x140, 0xD8, 0x1DC, 0x64 and 0x64
 * bytes against retail's 0x160, 0xF4, 0x1FC, 0x54 and 0x54), so none can
 * be linked.
 *
 *   - SsUtChangePitch: re-pitches a keyed-on voice (still assembly).
 *   - SsUtChangeADSR: if the voice still plays the given vab/program/note,
 *     writes the two ADSR words into its shadow registers and marks them dirty.
 *   - SsUtGetDetVVol / SsUtSetDetVVol: read the voice's left/right volume
 *     from the SPU registers / write it to the shadow registers, unscaled.
 *   - SsUtGetVVol / SsUtSetVVol: the same in 0..127 units (x or / 129).
 *   - SsUtAutoVol / SsUtAutoPan: start a volume or pan slide (SeAutoVol,
 *     SeAutoPan).
 *
 * Every call takes a voice number 0..23 and returns -1 for anything else.
 * The shadow registers are include/svm_data.h's _svm_sreg_buf, flushed to
 * the SPU by SpuVmFlush; _svm_sreg points at the SPU register block itself
 * (svm_data.h's SpuRegs).
 */

#include "common.h"
#include "libsnd_internal.h"

extern SpuRegs *_svm_sreg;

/* STALL -- docs/match-reports/SsUtChangePitch.md (84/88 words). */
INCLUDE_ASM("asm/nonmatchings/psyq/libsnd_ut_cp_ut_cadsr_ut_vvol_ut_autov_ut_autop", SsUtChangePitch);

/* MATCHING: `dead` only makes GCC reserve retail's unused 8-byte frame,
 * which puts the stack arguments at 0x18/0x1C($sp); nothing else may be added. */
s16 SsUtChangeADSR(s16 idx, s16 p1, s16 p2, s16 p3, u16 p4, u16 p5) {
    s32 dead[2];

    if ((u16)idx < 0x18) {
        if (_svm_voice[idx].vabId != p1) {
            return -1;
        }
        if (_svm_voice[idx].prog != p2) {
            return -1;
        }
        if (_svm_voice[idx].note != p3) {
            return -1;
        }
        _svm_sreg_buf[idx].adsr1 = p4;
        _svm_sreg_buf[idx].adsr2 = p5;
        _svm_sreg_dirty[idx] |= SVM_SREG_DIRTY_ADSR;
        return 0;
    }
    if (0) {
        dead[0] = 1;
    }
    return -1;
}

s16 SsUtGetDetVVol(s16 idx, s16 *out1, s16 *out2) {
    if ((u16)idx < 0x18) {
        /* MATCHING: `_svm_sreg->voice[idx]` swaps the addu's operands. */
        *out1 = (_svm_sreg->voice + idx)->volL;
        *out2 = (_svm_sreg->voice + idx)->volR;
        return 0;
    }
    return -1;
}

s16 SsUtSetDetVVol(s16 idx, s16 p1, s16 p2) {
    /* MATCHING: an unused 8-byte array reproduces retail's untouched frame. */
    s32 unused[2];

    if ((u16)idx < 0x18) {
        _svm_sreg_buf[idx].volR = p2;
        _svm_sreg_dirty[idx] |= SVM_SREG_DIRTY_VOL;
        _svm_sreg_buf[idx].volL = p1;
        return 0;
    }
    return -1;
}

s16 SsUtGetVVol(s16 idx, s16 *out1, s16 *out2) {
    SpuVoiceRegs *e;
    s16 f0, f2;

    if ((u16)idx < 0x18) {
        e = &_svm_sreg->voice[idx];
        f0 = e->volL;
        f2 = e->volR;
        *out1 = f0 / 129;
        *out2 = f2 / 129;
        return 0;
    }
    return -1;
}

s16 SsUtSetVVol(s16 idx, s16 p1, s16 p2) {
    /* MATCHING: as in SsUtSetDetVVol. */
    s32 unused[2];
    s16 t1, t2;

    if ((u16)idx < 0x18) {
        t1 = p1 * 129;
        t2 = p2 * 129;
        _svm_sreg_buf[idx].volR = t2;
        _svm_sreg_dirty[idx] |= SVM_SREG_DIRTY_VOL;
        _svm_sreg_buf[idx].volL = t1;
        return 0;
    }
    return -1;
}

s16 SsUtAutoVol(s16 p0, s16 p1, s16 p2, s16 p3) {
    if ((u16)p0 < 0x18) {
        SeAutoVol(p0, p1, p2, p3);
        return 0;
    }
    return -1;
}

s16 SsUtAutoPan(s16 p0, s16 p1, s16 p2, s16 p3) {
    if ((u16)p0 < 0x18) {
        SeAutoPan(p0, p1, p2, p3);
        return 0;
    }
    return -1;
}
