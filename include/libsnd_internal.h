#ifndef LIBSND_INTERNAL_H
#define LIBSND_INTERNAL_H

/**
 * @file libsnd_internal.h
 * @brief The declarations libsnd's own modules share: the voice manager's
 *        functions and globals (libsnd/vmanager and its siblings), its
 *        current key-on request (_svm_cur), the sequencer tick, and the
 *        sound system's state flags.
 *
 * Sony's, not the game's. No libsnd internal header ships on any SDK disc,
 * so this one is the project's: each prototype has its definition's types,
 * and a function whose callers disagree with its definition says so on its
 * declaration. Sony's public API stays in <libsnd.h>.
 *
 * A sequence is named by one packed number, `seqSepNo`: the SEQ/SEP access
 * in the low byte, the sequence within it in the high byte, reaching
 * `_ss_score[access][seq]` (ss_score.h). A key-on or key-off is not written
 * to the SPU when it is requested: it is collected in the _svm_okon and
 * _svm_okof masks, which SpuVmFlush writes out once per tick.
 */
#include "common.h"
#include <libsnd.h>
#include "ss_score.h"
#include "svm_data.h"

/**
 * @brief Plays one tick of sequence `_ss_score[access][seq]`: counts its
 * delta time down and decodes every event that falls due.
 * @param access The SEQ/SEP access.
 * @param seq    The sequence within it.
 */
void SeqPlay(s16 access, s16 seq);

/**
 * @brief Picks the voice for _svm_cur's request: a voice that is off and
 * silent if there is one, else the playing voice of lowest priority not
 * above the request's, ties going to the quietest envelope and then the
 * oldest.
 * Every voice ages by one and the chosen one's age restarts at 0 with the
 * request's priority; a chosen noise voice has the noise generator turned
 * off.
 *
 * Declared without a parameter list: the voice manager's own calls pass an
 * argument (0 or 0xFF), SsUtKeyOn and SsUtKeyOnV pass none, and the
 * definition reads none.
 * @return The voice, or spuVmMaxVoice when every voice outranks the request.
 */
s32 SpuVmAlloc(); /* arity-ok: callers pass 0, 0xFF or nothing; the definition reads no argument */

/**
 * @brief Keys _svm_cur's voice on at `pitch`: its shadow volumes from the
 * request's volume, the VAB's master volume, the program's and tone's
 * volumes and the sequence's, panned by the tone, the program and the
 * request (mono when _svm_stereo_mono is 1); then marks it to key on at the
 * next flush, and to reverb when the tone's mode bit 2 is set.
 * @param unused SpuVmKeyOn passes its match count; not read.
 * @param pitch  The SPU pitch to play at (note2pitch's).
 */
void SpuVmKeyOnNow(s32 unused, s32 pitch);

/**
 * @brief Programs _svm_cur's voice with its tone before the key-on: clears
 * the voice's bit in the dead-envelope history, sets its SPU start address
 * from the VAB's VAG table and its ADSR from the tone's (adsr2 plus
 * _svm_damper), and marks those registers dirty.
 */
void SpuVmDoAllocate(void);

/**
 * @brief Keys `voice` on the noise generator for _svm_cur's request (a tone
 * whose VAG is 0xFF), at the request's volumes scaled by the sequence's.
 * @param voice The voice SpuVmAlloc picked.
 */
void vmNoiseOn(s32 voice);

/**
 * @brief The SPU pitch of `note` with fine tune `fine` on _svm_cur's tone:
 * semitones from the tone's centre note, looked up in the pitch table at
 * octave 5 and shifted to the note's octave; the fine tune plus the tone's
 * shift is in 8ths of a table step and carries into the next semitone.
 * @param note The MIDI note.
 * @param fine The fine tune, 0 to 127.
 * @return The SPU pitch (0x1000 plays the sample at its own rate).
 */
s32 note2pitch2(s32 note, s32 fine);

/**
 * @brief Resets the voice manager: every voice, its shadow registers and
 * its SPU registers, the VAB table, the reverb depth to 0x3FFF, then one
 * SpuVmFlush.
 * @param voices The voices to manage, at most 24.
 */
void SpuVmInit(s32 voices);

/**
 * @brief Allocates a voice at top priority (0x7F) and keys it on the noise
 * generator at volumes `volL`/`volR`.
 * @param volL  Left volume.
 * @param volR  Right volume.
 * @param adsr1 Passed to the noise key-on, which does not read it.
 * @param adsr2 Passed to the noise key-on, which does not read it.
 */
void SpuVmNoiseOnWithAdsr(s32 volL, s32 volR, s32 adsr1, s32 adsr2);

/** @brief Releases every noise voice and clears the SPU's noise mask. */
void SpuVmNoiseOff(void);

/**
 * @brief SpuVmNoiseOnWithAdsr with the default envelope (0x80FF, 0x5FC8).
 * @param volL Left volume.
 * @param volR Right volume.
 */
void SpuVmNoiseOn(s32 volL, s32 volR);

/**
 * @brief Bends `voice`'s pitch when it is playing sequence `seq`, VAB
 * `vabId` and program `prog`: `bend` is 0 to 127 centred on 0x40, scaled
 * to the tone's pbmax semitones up or pbmin down.
 * @param voice The voice to test.
 * @param seq   The packed sequence number it must be playing.
 * @param vabId The VAB it must be playing.
 * @param prog  The program it must be playing.
 * @param bend  The bend, 0x40 for none.
 * @return 1 when the voice matched and was bent, else 0.
 */
s16 SpuVmPBVoice(s16 voice, s16 seq, s16 vabId, s16 prog, u16 bend);
/* SpuVmPitchBend is not here: its definition narrows its parameters to s16,
 * and SetPitchBend (libsnd/seqread) passes its first argument unnarrowed,
 * so seqread declares it locally. */

/**
 * @brief Starts a volume ramp on `voice` from `from` to `to` over
 * `duration` ticks, which SpuVmFlush steps (SetAutoVol) once per tick.
 * @param voice    The voice.
 * @param from     The start volume.
 * @param to       The volume the ramp ends at.
 * @param duration The ramp's length in ticks.
 */
void SeAutoVol(s16 voice, s16 from, s16 to, s16 duration);

/**
 * @brief Starts a pan ramp on `voice`, as SeAutoVol does for the volume.
 * @param voice    The voice.
 * @param from     The start pan.
 * @param to       The pan the ramp ends at.
 * @param duration The ramp's length in ticks.
 */
void SeAutoPan(s16 voice, s16 from, s16 to, s16 duration);

/**
 * @brief The voice manager's once-a-tick flush: records which voices'
 * envelopes have died, releases voices silent across that whole history
 * (unless _svm_auto_kof_mode is set), steps the volume and pan ramps,
 * copies the dirty shadow registers to the SPU and writes the key-off,
 * key-on and reverb masks.
 */
void SpuVmFlush(void);

/**
 * @brief Keys on `note` of VAB `vabId`'s program `prog` for a sequence: one
 * allocated voice per tone of the program whose note range holds the note.
 * A `vol` of 0 calls SpuVmKeyOff instead.
 * @param seqSepNo The packed sequence number, or 0x21 for a sound effect.
 * @param vabId    The VAB.
 * @param prog     The program.
 * @param note     The MIDI note.
 * @param vol      The velocity, 0 to 127.
 * @param pan      The pan, 0 to 127, 64 centre.
 * @return The voices keyed, four bits each, the last in the low nibble;
 *         0 for a key-off; -1 when the VAB or program is not valid.
 */
s32 SpuVmKeyOn(s32 seqSepNo, s16 vabId, s16 prog, u16 note, u16 vol, u16 pan);

/**
 * @brief Releases every voice playing `note` of VAB `vabId`'s program
 * `prog` for sequence `seqSepNo`.
 * @param seqSepNo The packed sequence number, or 0x21 for a sound effect.
 * @param vabId    The VAB.
 * @param prog     The program.
 * @param note     The MIDI note.
 * @return The number of voices released.
 */
s32 SpuVmKeyOff(s16 seqSepNo, s16 vabId, s16 prog, u16 note);

/**
 * @brief Keys on a sound effect: SpuVmKeyOn under the sound-effect sequence
 * number 0x21, with the louder of the two volumes as the velocity and their
 * ratio as the pan.
 * @param vabId  The VAB.
 * @param prog   The program.
 * @param note   The MIDI note.
 * @param unused Not read.
 * @param volL   Left volume.
 * @param volR   Right volume.
 * @return SpuVmKeyOn's result.
 */
s32 SpuVmSeKeyOn(s32 vabId, s32 prog, s32 note, s32 unused, u16 volL, u16 volR);

/**
 * @brief Keys off a sound effect SpuVmSeKeyOn started.
 * @param vabId The VAB.
 * @param prog  The program.
 * @param note  The MIDI note.
 * @return The number of voices released.
 */
s32 SpuVmSeKeyOff(s16 vabId, s16 prog, u16 note);

/**
 * @brief Sets sequence `seqSepNo`'s volume (SsScore::volL/volR, clamped to
 * 0x7F) and rescales the voices it is playing.
 * @param seqSepNo The packed sequence number.
 * @param volL     Left volume, 0 to 127.
 * @param volR     Right volume, 0 to 127.
 * @param a3       Every caller passes 0.
 * @return A status no call here reads.
 */
s32 SpuVmSetSeqVol(s16 seqSepNo, u16 volL, u16 volR, s32 a3);

/**
 * @brief Reads sequence `seqSepNo`'s volume back.
 * @param seqSepNo The packed sequence number.
 * @param volL     Receives the left volume.
 * @param volR     Receives the right volume.
 * @return `seqSepNo`, as recorded in _svm_cur.
 */
s32 SpuVmGetSeqVol(s32 seqSepNo, s16 *volL, s16 *volR);

/**
 * @brief Reads sequence `seqSepNo`'s left volume.
 * @param seqSepNo The packed sequence number.
 * @return The left volume.
 */
s32 SpuVmGetSeqLVol(s32 seqSepNo);

/**
 * @brief Reads sequence `seqSepNo`'s right volume.
 * @param seqSepNo The packed sequence number.
 * @return The right volume.
 */
s32 SpuVmGetSeqRVol(s32 seqSepNo);

/**
 * @brief Keys off every voice sequence `seqSepNo` is playing.
 * @param seqSepNo The packed sequence number.
 * @return A status no call here reads.
 */
s32 SpuVmSeqKeyOff(s32 seqSepNo);

/**
 * @brief Rescales every voice sequence `a0` plays on VAB `a1`'s program
 * `a2` to volume `a3` and pan `a4`: the voice's level times the VAB's, the
 * program's and the tone's volumes and the sequence's left and right
 * volume, panned by the tone, the program and `a4`, into the shadow volume
 * registers, marked dirty.
 * @param a0 The packed sequence number.
 * @param a1 The VAB.
 * @param a2 The program.
 * @param a3 The channel volume, 0 to 127.
 * @param a4 The channel pan, 0 to 127.
 * @return The number of voices rescaled.
 */
s32 SpuVmSetVol(s32 a0, s32 a1, s32 a2, s32 a3, u16 a4);

/**
 * @brief Selects VAB `vabId` as the current VAB (_svm_vh, _svm_pg, _svm_tn)
 * for a key-on or volume change of program `prog`.
 * @param vabId The VAB.
 * @param prog  The program.
 * @return 0 when `vabId` is a usable VAB, else nonzero (the key paths then
 *         give up).
 */
s32 SpuVmVSetUp(s16 vabId, s16 prog);

/** @brief The sustain pedal down (MIDI CC64 at 64 or above). */
void SpuVmDamperOn(void);

/** @brief The sustain pedal up (MIDI CC64 below 64). */
void SpuVmDamperOff(void);

/**
 * @brief Sets VAB `vabId`'s program `prog` volume (MIDI CC11, expression).
 * @param vabId The VAB.
 * @param prog  The program.
 * @param vol   The volume, 0 to 127.
 * @return A status no call here reads.
 */
s32 SpuVmSetProgVol(s16 vabId, s16 prog, s32 vol);

/* The voice manager's globals. The key masks are two halfwords each,
 * voices 0-15 then 16-23; SpuVmFlush writes them to the SPU once per tick. */
extern VabHdr *_svm_vh;      /**< The header of the current VAB. */
extern ProgAtr *_svm_pg;     /**< The current VAB's program attributes. */
extern VagAtr *_svm_tn;      /**< The current VAB's tone attributes, 16 per program. */
extern u8 spuVmMaxVoice;     /**< The voices the allocator may hand out (SpuVmInit's). */
extern s16 _svm_stereo_mono; /**< 1: mono, both channels at the louder volume. */
extern u16 _svm_okon1;       /**< Voices 0-15 to key on at the next flush. */
extern u16 _svm_okon2;       /**< Voices 16-23 to key on at the next flush. */
extern u16 _svm_okof1;       /**< Voices 0-15 to key off at the next flush. */
extern u16 _svm_okof2;       /**< Voices 16-23 to key off at the next flush. */
extern u16 _svm_orev1;       /**< Voices 0-15 sent to reverb. */
extern u16 _svm_orev2;       /**< Voices 16-23 sent to reverb. */

/*
 * _svm_cur (0x20 bytes): the key-on request the voice manager is working
 * on, which the key-on paths (SpuVmKeyOn, SsUtKeyOn, SsUtKeyOnV) fill before
 * they call into the allocator and the pitch helpers. Sony's struct type is
 * not on any SDK disc, so each byte is its own extern, named by address,
 * with its offset in _svm_cur. The three pans cut the right channel below
 * 64 and the left one above. +0x0C, the tone, is declared by each unit that
 * reads it.
 */
extern u8 D_8008EA0C;  /**< _svm_cur +0x00: the program's tone count. */
extern u8 D_8008EA0E;  /**< _svm_cur +0x02: the note to key. */
extern u8 D_8008EA0F;  /**< _svm_cur +0x03: the note's fine tune. */
extern u8 D_8008EA10;  /**< _svm_cur +0x04: volume, scaled by the VAB's mvol. */
extern u8 D_8008EA11;  /**< _svm_cur +0x05: third pan. */
extern u8 D_8008EA13;  /**< _svm_cur +0x07: program; a VAB gives each 16 tone slots in _svm_tn. */
extern u8 D_8008EA16;  /**< _svm_cur +0x0A: first volume factor, out of 127. */
extern u8 D_8008EA17;  /**< _svm_cur +0x0B: second pan. */
extern u8 D_8008EA19;  /**< _svm_cur +0x0D: second volume factor, out of 127. */
extern u8 D_8008EA1A;  /**< _svm_cur +0x0E: first pan. */
extern u8 D_8008EA1B;  /**< _svm_cur +0x0F: priority, which SpuVmAlloc compares. */
extern u8 D_8008EA1C;  /**< _svm_cur +0x10: the tone's centre note. */
extern u8 D_8008EA1D;  /**< _svm_cur +0x11: the tone's fine tune, 8 per pitch-table step. */
extern u8 D_8008EA1E;  /**< _svm_cur +0x12: the tone's lowest note (VagAtr min). */
extern u8 D_8008EA1F;  /**< _svm_cur +0x13: the tone's highest note (VagAtr max). */
extern u8 D_8008EA20;  /**< _svm_cur +0x14: mode bits; bit 2 sends the voice to reverb. */
extern u16 D_8008EA22; /**< _svm_cur +0x16: _ss_score index, access in the low byte; 0x21 from SsUtKeyOn/SsUtKeyOnV: no score. */
extern u16 D_8008EA24; /**< _svm_cur +0x18: the VAG number the tone plays, 0xFF for noise. */
extern volatile u16 D_8008EA26; /**< _svm_cur +0x1A: the voice being keyed; volatile, because the key paths store it and read it straight back. */

/* libsnd/ssinit's globals. */
extern SsMarkCallbackProc _ss_MarkCallback[0x20][16]; /**< SsSetMarkCallback's, one per (access, sequence); _SsInit clears them and ContNrpn1 calls them. */
extern s32 _snd_openflag;   /**< libsnd's record of the open SEQ/SEP accesses; _SsInit clears it. */
extern s32 _snd_ev_flag;    /**< The sound system's reentrancy lock. */
extern s32 _snd_video_mode; /**< The video mode SsSetTickMode reads, 0 or 1 (GetVideoMode's). */
extern s32 _snd_seq_no_tick;   /**< 1: SsSetTickMode was given SS_NOTICK. */
extern s32 _snd_seq_tick_mode; /**< SsSetTickMode's mode, SS_NOTICK cleared. */
extern s32 _snd_use_vsync_cb;  /**< Nonzero: _SsStart installed a vsync callback. */
extern s32 _snd_use_interrupt_id; /**< The interrupt _SsStart installed its handler on, -1 for none; SsEnd removes it. */
extern s32 _snd_1per2;              /**< Nonzero: the sequencer runs on every second tick. */
extern void (*_snd_vsync_cb)(void); /**< The vsync callback _SsTrapIntrVSync chains to. */
extern u32 VBLANK_MINUS;            /**< The sequencer's tick rate, 50, 60, 120 or 240. */

#endif
