/*
 * libsnd_cres -- Sony's libsnd/cres module, carried as C because no SDK disc
 * carries the build retail linked.
 *
 * Snd_crescendo is libsnd's per-tick volume fade for one sequence: the
 * linked libsnd objects call it by this name (config/psyq-objects.ld). Its
 * record is Sony's _ss_score entry, include/SsScore.h.
 *
 * Which object (nm over sdk/work/<disc>/elf/libsnd): cres.o on every disc,
 * with this one function as its only text symbol -- Snd_crescendo on 3.0 and
 * 3.3, renamed _SsSndCrescendo on 3.5 and 3.6. None is retail's build: its
 * text is 0x474 (3.0), 0x57C (3.3) and 0x2DC (3.5, 3.6) bytes against
 * retail's 0x3C0, so it cannot be linked.
 *
 * The unit's edges are Sony objects on both sides: libsnd/vm_doff before it,
 * libsnd/stop after it.
 */
#include "common.h"
#include "SsScore.h"

/* libsnd's sequence-volume accessors, defined in libsnd_vmanager.c. The access
 * number packs the SEQ/SEP access in the low byte and the sequence in the
 * high byte. MATCHING: x/y are u16 (retail masks them andi 0xFFFF). */
extern s32 SpuVmSetSeqVol(s16 a0, u16 a1, u16 a2, s32 a3);
extern s32 SpuVmGetSeqVol(s32 a0, s16 *out1, s16 *out2);

/* One tick of the fade Snd_SetCres (libsnd/vol) started on
 * _ss_score[a0][a1]: every unk42 ticks (or unk42's magnitude per tick, when
 * negative) the sequence volume rises, until the steps or ticks run out or
 * the volume reaches 0x7F, which clears the record's crescendo flag 0x10.
 * MATCHING: a0/a1 are s16, which gives retail's move a3,a0 ... move s5,a3. */
void Snd_crescendo(s16 a0, s16 a1) {
    SsScore **arr;
    SsScore *entry;
    s16 thresh;
    u16 sp10;
    u16 sp12;

    entry = &_ss_score[a0][a1];
    arr = &_ss_score[a0];
    entry->unk98 = entry->unk98 - 1;
    /* MATCHING: unk42 is read at each use, never copied to a local (retail's lh a2 + move v1,a2) */
    if (entry->unk42 > 0) {
        if ((u32)entry->unk98 % (u32)entry->unk42 != 0) {
            goto end;
        }
        if (entry->unk3E > 0) {
            entry->unk40 = entry->unk40 - 1;
            if (entry->unk40 >= 0) {
                SpuVmGetSeqVol((s16)(a0 | (a1 << 8)), (s16 *)&sp10, (s16 *)&sp12);
                if ((sp10 + 1) < 0x80 && (sp12 + 1) < 0x80) {
                    SpuVmSetSeqVol((s16)(a0 | (a1 << 8)), sp10 + 1, sp12 + 1, 0);
                } else {
                    SpuVmSetSeqVol((s16)(a0 | (a1 << 8)), 0x7F, 0x7F, 0);
                    _ss_score[a0][a1].unk90 &= ~0x10;
                }
            } else {
                SpuVmSetSeqVol((s16)(a0 | (a1 << 8)), 0x7F, 0x7F, 0);
                (*arr)[a1].unk90 &= ~0x10;
            }
        }
    } else {
        if (entry->unk3E > 0) {
            entry->unk40 = entry->unk40 + entry->unk42;
            SpuVmGetSeqVol((s16)(a0 | (a1 << 8)), (s16 *)&sp10, (s16 *)&sp12);
            if (entry->unk40 >= 0) {
                thresh = entry->unk42;
                if ((sp10 - thresh) < 0x80 && (sp12 - thresh) < 0x80) {
                    SpuVmSetSeqVol((s16)(a0 | (a1 << 8)), sp10 - thresh, sp12 - thresh, 0);
                } else {
                    SpuVmSetSeqVol((s16)(a0 | (a1 << 8)), 0x7F, 0x7F, 0);
                    _ss_score[a0][a1].unk90 &= ~0x10;
                }
            } else {
                SpuVmSetSeqVol((s16)(a0 | (a1 << 8)), 0x7F, 0x7F, 0);
                (*arr)[a1].unk90 &= ~0x10;
            }
        }
    }
    if (entry->unk98 == 0 || entry->unk40 == 0) {
        _ss_score[a0][a1].unk90 &= ~0x10;
    }
end:
    SpuVmGetSeqVol((s16)(a0 | (a1 << 8)), &entry->unk78, &entry->unk7A);
}
