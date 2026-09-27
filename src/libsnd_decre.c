/*
 * libsnd_decre -- Snd_decrescendo, libsnd's per-tick volume fade-out for one
 * sequence, the mirror of Snd_crescendo (libsnd_cres.c). Sony's code carried
 * as disassembly: the linked libsnd objects call it by this name
 * (config/psyq-objects.ld), and the libsnd/decre object that holds it on the
 * 3.0 and 3.3 discs is a different build (0x474 and 0x4B0 bytes of text
 * against retail's 0x328; 3.5/3.6 rename it _SsSndDecrescendo), so it cannot
 * be linked. Its record is Sony's _ss_score entry, include/SsScore.h.
 *
 * The unit's edges are Sony objects on both sides: libsnd/tempo before it,
 * libsnd/replay after it.
 */

#include "common.h"
#include "SsScore.h"

/* libsnd's sequence-volume accessors, defined in code_179d8_j.c. The access
 * number packs the SEQ/SEP access in the low byte and the sequence in the
 * high byte. */
extern s32 SpuVmSetSeqVol(s16 a0, u16 a1, u16 a2, s32 a3);
extern s32 SpuVmGetSeqVol(s32 p0, s16 *out1, s16 *out2);

/* One tick of the fade Snd_SetDecres (libsnd/vol) started on
 * _ss_score[a0][a1]: every unk42 ticks (or unk42's magnitude per tick, when
 * negative) the sequence volume falls, until the steps or ticks run out or
 * the volume reaches 0, which clears the record's flag 0x20.
 * STALL: register/stack allocation (retail's frame is 0x40, this body's 0x38);
 * the derivation and both measured bodies are in
 * docs/match-reports/Snd_decrescendo.md. */
#if 0
void Snd_decrescendo(s16 a0, s16 a1)
{
    SsScore **row = &_ss_score[a0];
    s32 off = a1 * sizeof(SsScore);
    SsScore *p = (SsScore *)((u8 *)*row + off);
    s32 c42 = p->unk42;
    s32 cnt = p->unk98 - 1;
    s16 pk;
    u16 sp10, sp12;
    u16 lo, hi;

    p->unk98 = cnt;
    do {
    if (c42 > 0) {
        if ((u32)cnt % (u32)c42 != 0) {
            goto tailFinal;
        }
        if (p->unk3E <= 0) {
            goto tailCheck;
        }
        p->unk40 -= 1;
        if (p->unk40 < 0) {
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
        if (p->unk3E <= 0) {
            goto tailCheck;
        }
        p->unk40 += c42;
        if (p->unk40 < 0) {
            goto negHandler;
        }
        pk = (s16)(a0 | (a1 << 8));
        SpuVmGetSeqVol(pk, (s16 *)&sp10, (s16 *)&sp12);
        if ((s32)sp10 < -(s32)p->unk42) {
            goto clearHandler;
        }
        if ((s32)sp12 < -(s32)p->unk42) {
            goto clearHandler;
        }
        lo = sp10 + p->unk42;
        hi = sp12 + p->unk42;
    }
    } while (0);
    SpuVmSetSeqVol(pk, lo, hi, 0);
    goto tailCheck;

clearHandler:
    SpuVmSetSeqVol((s16)(a0 | (a1 << 8)), 0, 0, 0);
    _ss_score[a0][a1].unk90 &= ~0x20;
    goto tailCheck;

negHandler:
    SpuVmSetSeqVol((s16)(a0 | (a1 << 8)), 0, 0, 0);
    ((SsScore *)((u8 *)*row + off))->unk90 &= ~0x20;

tailCheck:
    if (p->unk98 == 0 || p->unk40 == 0) {
        _ss_score[a0][a1].unk90 &= ~0x20;
    }

tailFinal:
    SpuVmGetSeqVol((s16)(a0 | (a1 << 8)), &p->unk78, &p->unk7A);
}
#endif

INCLUDE_ASM("asm/nonmatchings/libsnd_decre", Snd_decrescendo);
