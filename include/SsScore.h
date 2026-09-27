#ifndef SSSCORE_H
#define SSSCORE_H

#include "common.h"

/*
 * SsScore.h -- libsnd's per-sequence play state, the records behind Sony's
 * _ss_score.
 *
 * Sony's, not the game's. _ss_score (pinned in config/psyq-objects.ld) is an
 * array of pointers, one per open SEQ/SEP access; each points at an array of
 * SsScore, one per sequence in that access, so a sequence is reached as
 * _ss_score[access][seq]. The record is 0xAC bytes, which is <libsnd.h>'s
 * SS_SEQ_TABSIZ: the per-sequence size of the table the application hands
 * libsnd through SsSetTableSize(table, s_max, t_max).
 *
 * The type name is derived from Sony's VARIABLE name, as SvmVoice's is from
 * _svm_voice (include/SvmData.h): no libsnd internal header ships on any SDK
 * disc, so Sony's own struct tag is unknown. For the same reason fields are
 * named by OFFSET only, with their mechanics in a comment; only libsnd
 * functions read this record.
 *
 * Only the fields a unit including this header accesses are declared. Other
 * units still carry their own reduced views of the same record
 * (libsnd_decre/_j/_k's Entry90902E8, code_179d8_l's local SsScore); their
 * fields join this definition as those units move onto it.
 */
typedef struct SsScore {
    u8 pad0[0x12];
    u8 unk12; /* +0x12 -- the MIDI channel of the event being played: SpuVmKeyOn indexes unk4E by it */
    u8 pad13[0x3E - 0x13];
    s16 unk3E; /* +0x3E -- volume change of the running crescendo: Snd_setvol_data (libsnd/vol) stores its signed vol argument; Snd_crescendo steps only while it is > 0 */
    s16 unk40; /* +0x40 -- volume steps left: Snd_setvol_data seeds it with vol; Snd_crescendo counts it toward 0 and stops the fade there */
    s16 unk42; /* +0x42 -- fade rate: Snd_setvol_data stores v_time / |vol| (> 0: ticks per 1-step) or -(|vol| / v_time) (< 0: steps per tick) */
    u8 pad44[0x4E - 0x44];
    s16 unk4E[0x10]; /* +0x4E -- per-channel volume, one per MIDI channel: SpuVmKeyOn divides a note's velocity * 127 by it */
    u8 pad6E[0x74 - 0x6E];
    u16 unk74; /* +0x74 -- left volume: SpuVmSetSeqVol stores its voll (clamped to 0x7F); SpuVmSetVol scales a voice's left level by it / 127 */
    u16 unk76; /* +0x76 -- right volume: SpuVmSetSeqVol stores its volr (clamped to 0x7F); SpuVmSetVol scales the right level by it / 127 */
    s16 unk78; /* +0x78 -- left volume as of the last fade tick: Snd_crescendo reads unk74 into it through SpuVmGetSeqVol */
    s16 unk7A; /* +0x7A -- right volume as of the last fade tick: the same for unk76 */
    u8 pad7C[0x90 - 0x7C];
    s32 unk90; /* +0x90 -- state flags: Snd_SetCres sets 0x10 (crescendo running) and clears 0x20; Snd_crescendo clears 0x10 when the fade ends; Snd_setvol_data does nothing while 0x4 or 0x100 is set */
    u8 pad94[0x98 - 0x94];
    s32 unk98; /* +0x98 -- fade ticks left: Snd_setvol_data stores v_time (in unk94 too); Snd_crescendo decrements it once per tick */
    u8 pad9C[0xAC - 0x9C];
} SsScore; /* 0xAC == SS_SEQ_TABSIZ */

extern SsScore *_ss_score[];

#endif
