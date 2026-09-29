#ifndef SVM_DATA_H
#define SVM_DATA_H

#include "common.h"

/**
 * @file svm_data.h
 * @brief libsnd's voice manager tables: SvmVoice (_svm_voice, the state of
 *        each of the 24 voices), SvmSreg (_svm_sreg_buf, the shadow of each
 *        voice's SPU registers) with its dirty bits, and the SPU's own
 *        register block, SpuRegs.
 *
 * Sony's, not the game's. libsnd/vmanager.o (Psy-Q disc 3.5) defines the
 * bss block _svm_sreg_buf +0x000, _svm_sreg_dirty +0x180, _svm_voice
 * +0x198, _svm_envx_ptr +0x678, _svm_envx_hist +0x67C, so _svm_voice's
 * 0x4E0 bytes are 24 voices x 0x34 (ending exactly at _svm_envx_ptr), and
 * _svm_sreg_buf's 0x180 are 24 x 0x10.
 *
 * No libsnd internal header ships on any SDK disc, so Sony's struct tags
 * are unknown: each type is named for Sony's variable, and each field for
 * what libsnd's functions do with it, with the argument names of the
 * <libsnd.h> calls that store it (SsUtKeyOn, SpuVmKeyOn, SeAutoVol).
 */

/**
 * @brief One voice's state in the voice manager: what it plays (VAG, note,
 * program, tone, the sequence that keyed it), how it was keyed (volume, pan,
 * priority, age for the allocator) and its automatic volume and pan ramps.
 * SpuVmInit resets all 24; the key-on paths (SpuVmKeyOn, SsUtKeyOn,
 * SsUtKeyOnV) fill one that SpuVmAlloc picks, and the key-off paths free it.
 *
 * Field types are what the accessors need: the width every accessor agrees
 * on, and the signedness most of them read. SePitchBend reads otherwise
 * (+0x0C unsigned, +0x10 and +0x14 as bytes) through value casts at the
 * site, (u16)v.note and (u8)v.progIndex. SpuVmFlush walks envx as u16 for
 * its pointer stride.
 */
typedef struct SvmVoice {
    /* +0x00 */ s16 vag; /**< The tone's VAG number (VagAtr.vag) at key-on, 0xFF for the noise generator (and after SpuVmInit/SsUtAllKeyOff); key-off paths clear it. */
    /* +0x02 */ u16 age; /**< 0x18 at init; key-on clears it; SpuVmAlloc adds 1 to every voice's and takes the oldest as the tie-break. */
    /* +0x04 */ s16 pitch; /**< SpuVmKeyOnNow stores the pitch it keys on with, vmNoiseOn2 stores 10; every key-off path clears it. */
    /* +0x06 */ u16 envx; /**< SpuVmFlush copies the voice's SPU envx register into it each tick (0 = died); SpuVmAlloc's secondary key. */
    /* +0x08 */ s16 vol; /**< SpuVmKeyOn's vol * 127 / the channel volume (SEQ key-ons only); SpuVmSetVol scales it by vol / 127. */
    /* +0x0A */ u8 pan;  /**< SpuVmKeyOn's pan; 0x40 (centre) at init. */
    u8 pad0B;
    /* +0x0C */ s16 note; /**< The key-on's note; SePitchBend adds the bend to it and passes it to note2pitch2. */
    /* +0x0E */ s16 seq; /**< The packed sequence number that keyed it (SpuVmKeyOn's first argument); 0x21 for SsUtKeyOn/SsUtKeyOnV and sound effects, 0xFF when free. */
    /* +0x10 */ s16 progIndex; /**< The program's index into _svm_pg, and of its tones' block in _svm_tn (read as a byte by SePitchBend). */
    /* +0x12 */ s16 prog;      /**< The key-on's prog; key-off and SpuVmPBVoice match on it. */
    /* +0x14 */ s16 tone; /**< The tone within the program (read as a byte by SePitchBend), 0xFF when free. */
    /* +0x16 */ s16 vabId; /**< The key-on's vabId; key-off and SpuVmPBVoice match on it. */
    /* +0x18 */ s16 prior; /**< The tone's priority (VagAtr.prior), SpuVmAlloc's first key. */
    u8 pad1A;
    /* +0x1B */ u8 keyState; /**< 1 keyed on, 2 noise, 0 off. */

    /* +0x1C..+0x26: the SeAutoVol/SetAutoVol ramp. */
    /* +0x1C */ s16 autoVolActive; /**< Nonzero while the ramp runs; SeAutoVol sets it, SetAutoVol clears it when autoVolValue reaches autoVolTarget. */
    /* +0x1E */ s16 autoVolStep;   /**< Per-tick increment/decrement applied to autoVolValue. */
    /* +0x20 */ s16 autoVolInterval; /**< Ticks between steps (0 = every tick). */
    /* +0x22 */ s16 autoVolCountdown; /**< Countdown to the next step, reloaded from autoVolInterval. */
    /* +0x24 */ s16 autoVolValue;     /**< Running ramp value, SeAutoVol's "from". */
    /* +0x26 */ s16 autoVolTarget; /**< Value the ramp clamps to, SeAutoVol's "to". */

    /* +0x28..+0x32: the SeAutoPan/SetAutoPan ramp, same mechanics. */
    /* +0x28 */ s16 autoPanActive; /**< Nonzero while the ramp runs; SeAutoPan sets it, SetAutoPan clears it when autoPanValue reaches autoPanTarget. */
    /* +0x2A */ s16 autoPanStep;   /**< Per-tick increment/decrement applied to autoPanValue. */
    /* +0x2C */ s16 autoPanInterval; /**< Ticks between steps (0 = every tick). */
    /* +0x2E */ s16 autoPanCountdown; /**< Countdown to the next step, reloaded from autoPanInterval. */
    /* +0x30 */ s16 autoPanValue;     /**< Running ramp value, SeAutoPan's "from". */
    /* +0x32 */ s16 autoPanTarget; /**< Value the ramp clamps to, SeAutoPan's "to". */
} SvmVoice;                        /* 0x34 */

extern SvmVoice _svm_voice[]; /**< The voice manager's voices, 24. */

/**
 * @brief One voice's shadow of its SPU voice registers, in _svm_sreg_buf.
 * The voice manager writes a voice's volume, pitch, address and envelope
 * here and sets the matching _svm_sreg_dirty bits; once a tick SpuVmFlush
 * copies the dirty fields out to the SPU (SpuRegs::voice) through _svm_sreg.
 * Each field is named for the SpuVoiceRegs register it shadows.
 */
typedef struct SvmSreg {
    /* +0x0 */ s16 volL;  /**< SpuVoiceRegs.volL's shadow. */
    /* +0x2 */ s16 volR;  /**< volR's. */
    /* +0x4 */ u16 pitch; /**< pitch's: note2pitch2's result (SePitchBend, SpuVmPBVoice). */
    /* +0x6 */ u16 addr;  /**< addr's. */
    /* +0x8 */ s16 adsr1; /**< adsr1's: SsUtChangeADSR stores its p4. */
    /* +0xA */ s16 adsr2; /**< adsr2's: SsUtChangeADSR stores its p5. */
    u8 padC[0x10 - 0xC];
} SvmSreg; /* 0x10 */

extern SvmSreg _svm_sreg_buf[]; /**< The voices' register shadows, 24. */
extern u8 _svm_sreg_dirty[]; /**< One byte per voice (24): the SVM_SREG_DIRTY_* bits of the _svm_sreg_buf fields SpuVmFlush must copy out. */

/** @name Voice register dirty bits
 * _svm_sreg_dirty's bits, one per SvmSreg field. SpuVmFlush tests VOL_L
 * (and copies volL and volR), PITCH, ADDR and ADSR1 (copying adsr1 and
 * adsr2); VOL_R and ADSR2 are set with their pair but never tested. @{ */
#define SVM_SREG_DIRTY_VOL_L 0x01 /**< volL changed */
#define SVM_SREG_DIRTY_VOL_R 0x02 /**< volR changed */
#define SVM_SREG_DIRTY_PITCH 0x04 /**< pitch changed */
#define SVM_SREG_DIRTY_ADDR 0x08  /**< the start address changed */
#define SVM_SREG_DIRTY_ADSR1 0x10 /**< adsr1 changed */
#define SVM_SREG_DIRTY_ADSR2 0x20 /**< adsr2 changed */
/** both volumes changed */
#define SVM_SREG_DIRTY_VOL (SVM_SREG_DIRTY_VOL_L | SVM_SREG_DIRTY_VOL_R)
/** both envelope words changed */
#define SVM_SREG_DIRTY_ADSR (SVM_SREG_DIRTY_ADSR1 | SVM_SREG_DIRTY_ADSR2)

/** @} */

/**
 * @brief One SPU voice's hardware registers, 0x10 bytes, as SpuRegs::voice
 * lays out 24 of them. Hardware registers, so the fields carry the
 * hardware's names; only registers some accessor touches are named.
 */
typedef struct {
    /* +0x0 */ s16 volL;  /**< Left volume. */
    /* +0x2 */ s16 volR;  /**< Right volume. */
    /* +0x4 */ s16 pitch; /**< Sample rate; 0x1000 plays at 44.1 kHz. */
    /* +0x6 */ s16 addr;  /**< Start address in sound RAM, in 8-byte units. */
    /* +0x8 */ s16 adsr1; /**< Envelope attack, decay and sustain level. */
    /* +0xA */ s16 adsr2; /**< Envelope sustain rate and release. */
    /* +0xC */ u16 envx;  /**< Current envelope level; 0 once the voice has died. */
    u8 padE[0x10 - 0xE];
} SpuVoiceRegs;

/**
 * @brief The PS1 SPU's register block at 0x1F801C00, which libsnd's
 * _svm_sreg points at: 24 voices of 0x10 bytes, then the global key-on,
 * key-off, noise and reverb bit masks, each two halfwords (voices 0-15,
 * then 16-23). Units declare _svm_sreg themselves, each with the pointee
 * type its bodies read it through.
 */
typedef struct {
    /* +0x000 */ SpuVoiceRegs voice[24]; /**< The 24 voices' registers. */
    u8 pad180[0x188 - 0x180];
    /* +0x188 */ u16 keyOn[2];  /**< Voices 0-15, 16-23: a set bit keys the voice on. */
    /* +0x18C */ u16 keyOff[2]; /**< A set bit releases the voice. */
    u8 pad190[0x194 - 0x190];
    /* +0x194 */ u16 noiseOn[2];  /**< A set bit plays the voice from the noise generator. */
    /* +0x198 */ u16 reverbOn[2]; /**< A set bit sends the voice to the reverb. */
} SpuRegs;

#endif
