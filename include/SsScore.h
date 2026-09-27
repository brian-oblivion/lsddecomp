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
 * Only the fields a unit including this header accesses are declared.
 */
typedef struct SsScore {
    u8 unk0; /* +0x00 -- _SsSndNextSep's second argument when GetMetaEvent stops the sequence (unk3C is the first) */
    u8 pad1[0x4 - 0x1];
    u8 *unk4; /* +0x04 -- read cursor into the SEQ event stream: every seqread decoder advances it */
    u8 *unk8; /* +0x08 -- start of the event stream: GetMetaEvent's End of Track rewinds unk4 (and unkC) to it */
    u8 *unkC; /* +0x0C -- loop point: ContNrpn2's NRPN 0x14 saves unk4 here and its 0x1E rewinds unk4 to it */
    u8 unk10; /* +0x10 -- loop count taken: ContNrpn1/ContDataEntry set it when they store unk28; ContNrpn2 clears it when the loop ends */
    u8 unk11; /* +0x11 -- MIDI running status: GetSeqData stores each status byte's high nibble (0xFF for 0xF0, meta) and reuses it for a data byte */
    u8 unk12; /* +0x12 -- the MIDI channel of the event being played: GetSeqData stores a status byte's low nibble; unk17, unk2C and unk4E are indexed by it */
    u8 unk13; /* +0x13 -- RPN LSB: ContRpn1 (CC100) stores it; ContDataEntry selects on it with unk14; ContResetAll clears it */
    u8 unk14; /* +0x14 -- RPN MSB: ContRpn2 (CC101) stores it; ContResetAll clears it */
    u8 unk15; /* +0x15 -- NRPN LSB: ContNrpn1 (CC98) stores it outside loop NRPNs; ContDataEntry hands it to Snd_setVabAttr as the attribute selector */
    u8 unk16; /* +0x16 -- NRPN MSB: ContNrpn2 (CC99) stores it; 0x14 marks a loop start, 0x1E a loop end, 0x28 routes ContNrpn1 to the mark callback */
    u8 unk17[0x10]; /* +0x17 -- per-channel pan: CC10 stores it and ContResetAll sets 0x40; NoteOn and CC7/CC11 pass it to SpuVmKeyOn/SpuVmSetVol */
    u8 unk27; /* +0x27 -- loop open: ContNrpn2's NRPN 0x14 sets it to 1; End of Track clears it */
    u8 unk28; /* +0x28 -- loop count: ContNrpn1/ContDataEntry store the data value after a loop start; ContNrpn2's 0x1E counts it down (0x7F and up loop forever) */
    u8 unk29; /* +0x29 -- RPN bytes received: ContRpn1/ContRpn2 count up; ContDataEntry acts at 2 and resets it */
    u8 unk2A; /* +0x2A -- NRPN bytes received: ContNrpn1/ContNrpn2 count up; ContDataEntry acts at 2 and resets it */
    u8 unk2B; /* +0x2B -- GetMetaEvent clears it when it stops the sequence */
    u8 unk2C[0x10]; /* +0x2C -- per-channel program: SetProgramChange stores it and ContResetAll resets it to the channel number; the program handed to SsUtGet/SetVagAtr and SpuVm* */
    u8 unk3C; /* +0x3C -- _SsSndNextSep's first argument when GetMetaEvent stops the sequence; 0xFF skips the call */
    u8 pad3D[0x3E - 0x3D];
    s16 unk3E; /* +0x3E -- volume change of the running crescendo: Snd_setvol_data (libsnd/vol) stores its signed vol argument; Snd_crescendo steps only while it is > 0 */
    s16 unk40; /* +0x40 -- volume steps left: Snd_setvol_data seeds it with vol; Snd_crescendo counts it toward 0 and stops the fade there */
    s16 unk42; /* +0x42 -- fade rate: Snd_setvol_data stores v_time / |vol| (> 0: ticks per 1-step) or -(|vol| / v_time) (< 0: steps per tick) */
    u8 pad44[0x46 - 0x44];
    s16 unk46; /* +0x46 -- play count: End of Track rewinds while unk48 is below it, and forever when it is 0 */
    u16 unk48; /* +0x48 -- plays so far: End of Track counts it up and compares it (signed) with unk46 */
    s16 unk4A; /* +0x4A -- the Set Tempo meta event's rate recompute multiplies it with unk8C */
    s16 unk4C; /* +0x4C -- VAB id: CC0 (bank select) stores it; the vab id handed to SsUtGet/SetProgAtr/VagAtr and SpuVm* */
    s16 unk4E[0x10]; /* +0x4E -- per-channel volume, one per MIDI channel: CC7 stores it and ContResetAll sets 0x7F; SpuVmKeyOn divides a note's velocity * 127 by it */
    s16 unk6E; /* +0x6E -- SeqPlay calls per tick, counted down: Set Tempo stores the same value as unk70, or -1 when one call covers unk70 ticks */
    s16 unk70; /* +0x70 -- ticks per SeqPlay call while unk6E is -1, else unk6E's reload value: Set Tempo derives it from VBLANK_MINUS, unk4A and unk8C */
    u8 pad72[0x74 - 0x72];
    u16 unk74; /* +0x74 -- left volume: SpuVmSetSeqVol stores its voll (clamped to 0x7F); SpuVmSetVol scales a voice's left level by it / 127; NoteOn ignores the event (no key on or off) while it is 0 */
    u16 unk76; /* +0x76 -- right volume: SpuVmSetSeqVol stores its volr (clamped to 0x7F); SpuVmSetVol scales the right level by it / 127 */
    s16 unk78; /* +0x78 -- left volume as of the last fade tick: Snd_crescendo reads unk74 into it through SpuVmGetSeqVol */
    s16 unk7A; /* +0x7A -- right volume as of the last fade tick: the same for unk76 */
    u8 pad7C[0x80 - 0x7C];
    s32 unk80; /* +0x80 -- ticks played: ReadDeltaValue adds every delta time; End of Track zeroes it */
    u8 pad84[0x88 - 0x84];
    s32 unk88; /* +0x88 -- ticks to the next event: each event handler stores ReadDeltaValue's result; SeqPlay counts it down by unk70 */
    s32 unk8C; /* +0x8C -- tempo in beats per minute: Set Tempo stores 60000000 / its microseconds per quarter note */
    s32 unk90; /* +0x90 -- state flags: Snd_SetCres sets 0x10 (crescendo running) and clears 0x20; Snd_crescendo clears 0x10 when the fade ends; Snd_setvol_data does nothing while 0x4 or 0x100 is set; GetMetaEvent's stop clears 0x1/0x2/0x8 and sets 0x4/0x200 */
    u8 pad94[0x98 - 0x94];
    s32 unk98; /* +0x98 -- fade ticks left: Snd_setvol_data stores v_time (in unk94 too); Snd_crescendo decrements it once per tick */
    u8 pad9C[0xA8 - 0x9C];
    s16 unkA8; /* +0xA8 -- NoteOn stores the velocity of each key on */
    u8 padAA[0xAC - 0xAA];
} SsScore; /* 0xAC == SS_SEQ_TABSIZ */

extern SsScore *_ss_score[];

#endif
