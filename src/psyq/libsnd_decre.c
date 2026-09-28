/*
 * libsnd_decre -- Sony's libsnd/decre module, carried as disassembly
 * because no SDK disc carries the build retail linked.
 *
 * Snd_decrescendo is libsnd's per-tick volume fade-out for one sequence,
 * the mirror of Snd_crescendo (libsnd_cres.c): the linked libsnd objects
 * call it by this name (config/psyq-objects.ld). Its record is Sony's
 * _ss_score entry, include/ss_score.h.
 *
 * Which object (nm over sdk/work/<disc>/elf/libsnd): decre.o on 3.0, 3.3
 * and 3.5, decres.o on 3.6, with this one function as its only text
 * symbol -- Snd_decrescendo on 3.0 and 3.3, renamed _SsSndDecrescendo on
 * 3.5 and 3.6. None is retail's build: its text is 0x474 (3.0), 0x4B0
 * (3.3) and 0x2AC (3.5, 3.6) bytes against retail's 0x328, so it cannot be
 * linked.
 */

#include "common.h"
#include "libsnd_internal.h"

/* One tick of the fade Snd_SetDecres (libsnd/vol) started on
 * _ss_score[a0][a1]: every fadeRate ticks (or fadeRate's magnitude per tick, when
 * negative) the sequence volume falls, until the steps or ticks run out or
 * the volume reaches 0, which clears the record's flag 0x20. */
#ifdef NON_MATCHING
/* NON_MATCHING: 200 words against retail's 202; register and stack
 * allocation, retail's frame 0x40 against this body's 0x38
 * (docs/match-reports/Snd_decrescendo.md). */
void Snd_decrescendo(s16 a0, s16 a1) {
    SsScore **row = &_ss_score[a0];
    s32 off = a1 * sizeof(SsScore);
    SsScore *p = (SsScore *)((u8 *)*row + off);
    s32 c42 = p->fadeRate;
    s32 cnt = p->fadeTicksLeft - 1;
    s16 pk;
    u16 sp10, sp12;
    u16 lo, hi;

    p->fadeTicksLeft = cnt;
    do {
        if (c42 > 0) {
            if ((u32)cnt % (u32)c42 != 0) {
                goto tailFinal;
            }
            if (p->fadeDelta <= 0) {
                goto tailCheck;
            }
            p->fadeStepsLeft -= 1;
            if (p->fadeStepsLeft < 0) {
                goto negHandler;
            }
            pk = (s16)(a0 | (a1 << 8));
            SpuVmGetSeqVol(pk, (s16 *)&sp10, (s16 *)&sp12);
            if (sp10 == 0) {
                goto clearHandler;
            }
            if (sp12 == 0) {
                goto clearHandler;
            }
            lo = sp10 + (u16)-1;
            hi = sp12 + (u16)-1;
        } else {
            if (p->fadeDelta <= 0) {
                goto tailCheck;
            }
            p->fadeStepsLeft += c42;
            if (p->fadeStepsLeft < 0) {
                goto negHandler;
            }
            pk = (s16)(a0 | (a1 << 8));
            SpuVmGetSeqVol(pk, (s16 *)&sp10, (s16 *)&sp12);
            if ((s32)sp10 < -(s32)p->fadeRate) {
                goto clearHandler;
            }
            if ((s32)sp12 < -(s32)p->fadeRate) {
                goto clearHandler;
            }
            lo = sp10 + p->fadeRate;
            hi = sp12 + p->fadeRate;
        }
    } while (0);
    SpuVmSetSeqVol(pk, lo, hi, 0);
    goto tailCheck;

clearHandler:
    SpuVmSetSeqVol((s16)(a0 | (a1 << 8)), 0, 0, 0);
    _ss_score[a0][a1].flags &= ~0x20;
    goto tailCheck;

negHandler:
    SpuVmSetSeqVol((s16)(a0 | (a1 << 8)), 0, 0, 0);
    ((SsScore *)((u8 *)*row + off))->flags &= ~0x20;

tailCheck:
    if (p->fadeTicksLeft == 0 || p->fadeStepsLeft == 0) {
        _ss_score[a0][a1].flags &= ~0x20;
    }

tailFinal:
    SpuVmGetSeqVol((s16)(a0 | (a1 << 8)), &p->fadeVolL, &p->fadeVolR);
}
#else
INCLUDE_ASM("asm/nonmatchings/psyq/libsnd_decre", Snd_decrescendo);
#endif
