#ifndef SS_SCORE_H
#define SS_SCORE_H

#include "common.h"

/**
 * @file ss_score.h
 * @brief SsScore, libsnd's per-sequence play state, and _ss_score, the
 *        table of them.
 *
 * Sony's, not the game's. No libsnd internal header ships on any SDK disc,
 * so Sony's struct tag is unknown: the type is named for Sony's variable
 * _ss_score, as SvmVoice is for _svm_voice (svm_data.h), and each field is
 * named for what the libsnd functions that read it (the only readers) do
 * with it, in the SEQ format's MIDI terms. Only the fields some function
 * here accesses are declared.
 */

/**
 * @brief One sequence's play state: its read cursor in the SEQ event
 * stream, the MIDI controller state per channel, the loop and play counts,
 * the tempo, the volume and its fade.
 *
 * _ss_score holds one pointer per open SEQ/SEP access, each at an array of
 * SsScore, one per sequence in that access, so a sequence is reached as
 * `_ss_score[access][seq]`. The record is 0xAC bytes, which is <libsnd.h>'s
 * SS_SEQ_TABSIZ: the per-sequence size of the table the application hands
 * libsnd through SsSetTableSize(table, s_max, t_max).
 * SeqPlay (libsnd_seqread.c) plays it a tick at a time, and Snd_crescendo
 * and Snd_decrescendo fade its volume.
 */
typedef struct SsScore {
    /* +0x00 */ u8 nextSepSeq; /**< _SsSndNextSep's second argument when GetMetaEvent stops the sequence (nextSepAccess is the first). */
    u8 pad1[0x4 - 0x1];
    /* +0x04 */ u8 *readPos; /**< Read cursor into the SEQ event stream: every seqread decoder advances it. */
    /* +0x08 */ u8 *trackStart; /**< Start of the event stream: GetMetaEvent's End of Track rewinds readPos (and loopPos) to it. */
    /* +0x0C */ u8 *loopPos; /**< Loop point: ContNrpn2's NRPN 0x14 saves readPos here and its 0x1E rewinds readPos to it. */
    /* +0x10 */ u8 loopCountSet; /**< Loop count taken: ContNrpn1/ContDataEntry set it when they store loopCount; ContNrpn2 clears it when the loop ends. */
    /* +0x11 */ u8 runningStatus; /**< MIDI running status: GetSeqData stores each status byte's high nibble (0xFF for 0xF0, meta) and reuses it for a data byte. */
    /* +0x12 */ u8 channel; /**< The MIDI channel of the event being played: GetSeqData stores a status byte's low nibble; pan, program and channelVol are indexed by it. */
    /* +0x13 */ u8 rpnLsb; /**< RPN LSB: ContRpn1 (CC100) stores it; ContDataEntry selects on it with rpnMsb; ContResetAll clears it. */
    /* +0x14 */ u8 rpnMsb; /**< RPN MSB: ContRpn2 (CC101) stores it; ContResetAll clears it. */
    /* +0x15 */ u8 nrpnLsb; /**< NRPN LSB: ContNrpn1 (CC98) stores it outside loop NRPNs; ContDataEntry hands it to Snd_setVabAttr as the attribute selector. */
    /* +0x16 */ u8 nrpnMsb; /**< NRPN MSB: ContNrpn2 (CC99) stores it; 0x14 marks a loop start, 0x1E a loop end, 0x28 routes ContNrpn1 to the mark callback. */
    /* +0x17 */ u8 pan[0x10]; /**< Per-channel pan: CC10 stores it and ContResetAll sets 0x40; NoteOn and CC7/CC11 pass it to SpuVmKeyOn/SpuVmSetVol. */
    /* +0x27 */ u8 loopOpen; /**< Loop open: ContNrpn2's NRPN 0x14 sets it to 1; End of Track clears it. */
    /* +0x28 */ u8 loopCount; /**< Loop count: ContNrpn1/ContDataEntry store the data value after a loop start; ContNrpn2's 0x1E counts it down (0x7F and up loop forever). */
    /* +0x29 */ u8 rpnBytes; /**< RPN bytes received: ContRpn1/ContRpn2 count up; ContDataEntry acts at 2 and resets it. */
    /* +0x2A */ u8 nrpnBytes; /**< NRPN bytes received: ContNrpn1/ContNrpn2 count up; ContDataEntry acts at 2 and resets it. */
    /* +0x2B */ u8 unk2B; /**< GetMetaEvent clears it when it stops the sequence; no reader in C. */
    /* +0x2C */ u8 program[0x10]; /**< Per-channel program: SetProgramChange stores it and ContResetAll resets it to the channel number; the program handed to SsUtGet/SetVagAtr and SpuVm*. */
    /* +0x3C */ u8 nextSepAccess; /**< _SsSndNextSep's first argument when GetMetaEvent stops the sequence; 0xFF skips the call. */
    u8 pad3D[0x3E - 0x3D];
    /* +0x3E */ s16 fadeDelta; /**< Volume change of the running crescendo: Snd_setvol_data (libsnd/vol) stores its signed vol argument; Snd_crescendo steps only while it is > 0. */
    /* +0x40 */ s16 fadeStepsLeft; /**< Volume steps left: Snd_setvol_data seeds it with vol; Snd_crescendo counts it toward 0 and stops the fade there. */
    /* +0x42 */ s16 fadeRate; /**< Fade rate: Snd_setvol_data stores v_time / |vol| (> 0: ticks per 1-step) or -(|vol| / v_time) (< 0: steps per tick). */
    u8 pad44[0x46 - 0x44];
    /* +0x46 */ s16 playCount; /**< Play count: End of Track rewinds while playsDone is below it, and forever when it is 0. */
    /* +0x48 */ u16 playsDone; /**< Plays so far: End of Track counts it up and compares it (signed) with playCount. */
    /* +0x4A */ s16 ticksPerBeat; /**< The Set Tempo meta event's rate recompute multiplies it with tempo. */
    /* +0x4C */ s16 vabId; /**< VAB id: CC0 (bank select) stores it; the vab id handed to SsUtGet/SetProgAtr/VagAtr and SpuVm*. */
    /* +0x4E */ s16 channelVol[0x10]; /**< Per-channel volume, one per MIDI channel: CC7 stores it and ContResetAll sets 0x7F; SpuVmKeyOn divides a note's velocity * 127 by it. */
    /* +0x6E */ s16 callsPerTick; /**< SeqPlay calls per tick, counted down: Set Tempo stores the same value as ticksPerCall, or -1 when one call covers ticksPerCall ticks. */
    /* +0x70 */ s16 ticksPerCall; /**< Ticks per SeqPlay call while callsPerTick is -1, else callsPerTick's reload value: Set Tempo derives it from VBLANK_MINUS, ticksPerBeat and tempo. */
    u8 pad72[0x74 - 0x72];
    /* +0x74 */ u16 volL; /**< Left volume: SpuVmSetSeqVol stores its voll (clamped to 0x7F); SpuVmSetVol scales a voice's left level by it / 127; NoteOn ignores the event (no key on or off) while it is 0. */
    /* +0x76 */ u16 volR; /**< Right volume: SpuVmSetSeqVol stores its volr (clamped to 0x7F); SpuVmSetVol scales the right level by it / 127. */
    /* +0x78 */ s16 fadeVolL; /**< Left volume as of the last fade tick: Snd_crescendo reads volL into it through SpuVmGetSeqVol. */
    /* +0x7A */ s16 fadeVolR; /**< Right volume as of the last fade tick: the same for volR. */
    u8 pad7C[0x80 - 0x7C];
    /* +0x80 */ s32 ticksPlayed; /**< Ticks played: ReadDeltaValue adds every delta time; End of Track zeroes it. */
    u8 pad84[0x88 - 0x84];
    /* +0x88 */ s32 deltaLeft; /**< Ticks to the next event: each event handler stores ReadDeltaValue's result; SeqPlay counts it down by ticksPerCall. */
    /* +0x8C */ s32 tempo; /**< Tempo in beats per minute: Set Tempo stores 60000000 / its microseconds per quarter note. */
    /* +0x90 */ s32 flags; /**< State flags: Snd_SetCres sets 0x10 (crescendo running) and clears 0x20; Snd_crescendo clears 0x10 when the fade ends; Snd_setvol_data does nothing while 0x4 or 0x100 is set; GetMetaEvent's stop clears 0x1/0x2/0x8 and sets 0x4/0x200. */
    u8 pad94[0x98 - 0x94];
    /* +0x98 */ s32 fadeTicksLeft; /**< Fade ticks left: Snd_setvol_data stores v_time (and at +0x94, undeclared); Snd_crescendo decrements it once per tick. */
    u8 pad9C[0xA8 - 0x9C];
    /* +0xA8 */ s16 lastVelocity; /**< NoteOn stores the velocity of each key on. */
    u8 padAA[0xAC - 0xAA];
} SsScore; /* 0xAC == SS_SEQ_TABSIZ */

/** @brief libsnd's sequence table: one pointer per open SEQ/SEP access, at
 * that access's SsScore array (see SsScore). */
extern SsScore *_ss_score[];

#endif
