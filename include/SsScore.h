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
 * (code_179d8_f/_i/_k's Entry90902E8, code_179d8_l's SsScore,
 * code_179d8_m's Entry90902E8M); their fields join this definition as those
 * units move onto it.
 */
typedef struct SsScore {
    u8 pad0[0x74];
    u16 unk74; /* +0x74 -- left volume: SpuVmSetSeqVol stores its voll (clamped to 0x7F); SpuVmSetVol scales a voice's left level by it / 127 */
    u16 unk76; /* +0x76 -- right volume: SpuVmSetSeqVol stores its volr (clamped to 0x7F); SpuVmSetVol scales the right level by it / 127 */
    u8 pad78[0xAC - 0x78];
} SsScore; /* 0xAC == SS_SEQ_TABSIZ */

extern SsScore *_ss_score[];

#endif
