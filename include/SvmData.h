#ifndef SVMDATA_H
#define SVMDATA_H

#include "common.h"

/*
 * SvmData.h -- libsnd vmanager's per-voice bss tables: _svm_sreg_buf,
 * _svm_sreg_dirty and _svm_voice.
 *
 * SvmVoice -- one record of libsnd's per-voice table _svm_voice.
 *
 * Sony's, not the game's. libsnd/vmanager.o (Psy-Q disc 3.5) defines the
 * bss block _svm_sreg_buf +0x000, _svm_sreg_dirty +0x180, _svm_voice
 * +0x198, _svm_envx_ptr +0x678, _svm_envx_hist +0x67C, so _svm_voice's
 * 0x4E0 bytes are 24 voices x 0x34 (ending exactly at _svm_envx_ptr).
 *
 * The type name is derived from Sony's VARIABLE name, because Sony's own
 * struct tag is unknown: no libsnd internal header ships on any SDK disc.
 * For the same reason every field is named by its OFFSET only, and no
 * game name goes on any field: only libsnd functions read it.
 *
 * Field types are what the accessors need: the width every accessor agrees
 * on, and the signedness most of them read. SePitchBend reads otherwise
 * (+0x0C unsigned, +0x10 and +0x14 as bytes) through value casts at the
 * site, (u16)v.unk0C and (u8)v.unk10. MATCHING: an address cast,
 * *(u8 *)&v.unk10, grows SePitchBend's frame by 8 bytes. SpuVmFlush walks
 * +0x06 as u16 for its pointer stride.
 *
 * asm/ names this table's fields by address (D_8008D98A, D_8008D98C, ...
 * at a 0x34 stride); the linker resolves those and this struct to the same
 * bytes.
 */
typedef struct SvmVoice {
    s16 unk00; /* +0x00 -- 0xFF after SpuVmInit/SsUtAllKeyOff; key-on stores D_8008EA24; SsUtKeyOff tests == 0xFF; key-off paths clear it */
    u16 unk02; /* +0x02 -- 0x18 at init; key-on clears it; SpuVmAlloc adds 1 to every voice's and uses it as the tie-break key */
    s16 unk04; /* +0x04 -- SpuVmKeyOnNow stores its a1, vmNoiseOn2 stores 10; every key-off path clears it */
    u16 unk06; /* +0x06 -- SpuVmFlush copies an SPU voice register into it each tick (0 = idle); SpuVmAlloc's secondary key */
    s16 unk08; /* +0x08 -- level scaled by SpuVmSetVol (x vol / 127) */
    u8 unk0A;  /* +0x0A -- byte, 0x40 at init; SpuVmKeyOn stores its 6th argument */
    u8 pad0B;
    s16 unk0C; /* +0x0C -- key-on argument; SePitchBend adds the bend to it and passes it to note2pitch2 */
    s16 unk0E; /* +0x0E -- 0x21 after SsUtKeyOn/SsUtKeyOnV, 0xFF when free */
    s16 unk10; /* +0x10 -- index into _svm_pg (read as a byte by SePitchBend) */
    s16 unk12; /* +0x12 -- compared against a key-off argument */
    s16 unk14; /* +0x14 -- index into _svm_tn (read as a byte by SePitchBend), 0xFF when free */
    s16 unk16; /* +0x16 -- compared against a key-off argument */
    s16 unk18; /* +0x18 -- SpuVmAlloc's priority, loaded from D_8008EA1B */
    u8 pad1A;
    u8 unk1B; /* +0x1B -- byte state: 1 keyed on, 2 noise, 0 off */
    /* +0x1C..+0x26: the SeAutoVol/SetAutoVol ramp. */
    s16 unk1C; /* +0x1C -- nonzero while the ramp runs; SeAutoVol sets it, SetAutoVol clears it when +0x24 reaches +0x26 */
    s16 unk1E; /* +0x1E -- per-tick increment/decrement applied to +0x24 */
    s16 unk20; /* +0x20 -- ticks between steps (0 = every tick) */
    s16 unk22; /* +0x22 -- countdown to the next step, reloaded from +0x20 */
    s16 unk24; /* +0x24 -- running ramp value, SeAutoVol's "from" */
    s16 unk26; /* +0x26 -- value the ramp clamps to, SeAutoVol's "to" */
    /* +0x28..+0x32: the SeAutoPan/SetAutoPan ramp, same mechanics. */
    s16 unk28; /* +0x28 -- nonzero while the ramp runs; SeAutoPan sets it, SetAutoPan clears it when +0x30 reaches +0x32 */
    s16 unk2A; /* +0x2A -- per-tick increment/decrement applied to +0x30 */
    s16 unk2C; /* +0x2C -- ticks between steps (0 = every tick) */
    s16 unk2E; /* +0x2E -- countdown to the next step, reloaded from +0x2C */
    s16 unk30; /* +0x30 -- running ramp value, SeAutoPan's "from" */
    s16 unk32; /* +0x32 -- value the ramp clamps to, SeAutoPan's "to" */
} SvmVoice;    /* 0x34 */

extern SvmVoice _svm_voice[]; /* 24 voices */

/*
 * SvmSreg -- one record of libsnd's _svm_sreg_buf, the per-voice shadow
 * of the SPU voice registers that SpuVmFlush copies out through
 * _svm_sreg (the SPU voice register block) for every voice whose
 * _svm_sreg_dirty byte has bits set. Same provenance as SvmVoice:
 * libsnd/vmanager.o bss +0x000, 24 voices x 0x10. Each field is named for
 * the SpuVoiceRegs register SpuVmFlush copies it to.
 */
typedef struct SvmSreg {
    s16 volL;  /* +0x0 -- SpuVoiceRegs.volL's shadow */
    s16 volR;  /* +0x2 -- volR's */
    u16 pitch; /* +0x4 -- pitch's: note2pitch2's result (SePitchBend, SpuVmPBVoice) */
    u16 addr;  /* +0x6 -- addr's */
    s16 adsr1; /* +0x8 -- adsr1's: SsUtChangeADSR stores its p4 */
    s16 adsr2; /* +0xA -- adsr2's: SsUtChangeADSR stores its p5 */
    u8 padC[0x10 - 0xC];
} SvmSreg; /* 0x10 */

extern SvmSreg _svm_sreg_buf[]; /* 24 voices */
extern u8 _svm_sreg_dirty[];    /* 24 voices: which _svm_sreg_buf fields SpuVmFlush must copy out */

/* _svm_sreg_dirty's bits, one per SvmSreg field. SpuVmFlush tests VOL_L
 * (and copies volL and volR), PITCH, ADDR and ADSR1 (copying adsr1 and
 * adsr2); VOL_R and ADSR2 are set with their pair but never tested. */
#define SVM_SREG_DIRTY_VOL_L 0x01
#define SVM_SREG_DIRTY_VOL_R 0x02
#define SVM_SREG_DIRTY_PITCH 0x04
#define SVM_SREG_DIRTY_ADDR 0x08
#define SVM_SREG_DIRTY_ADSR1 0x10
#define SVM_SREG_DIRTY_ADSR2 0x20
#define SVM_SREG_DIRTY_VOL (SVM_SREG_DIRTY_VOL_L | SVM_SREG_DIRTY_VOL_R)
#define SVM_SREG_DIRTY_ADSR (SVM_SREG_DIRTY_ADSR1 | SVM_SREG_DIRTY_ADSR2)

/*
 * SpuVoiceRegs, SpuRegs -- the PS1 SPU's own register block at 0x1F801C00,
 * which libsnd/vmanager.o's first .data word (_svm_sreg) points at: 24
 * voices of 0x10 bytes, then the global key-on/key-off/noise/reverb bit
 * masks. Unlike the bss tables above these are hardware registers, so the
 * fields carry the hardware's names; only registers some accessor touches
 * are named. Units declare _svm_sreg themselves, each with the pointee
 * spelling its bodies match against.
 */
typedef struct {
    s16 volL;  /* +0x0 -- left volume */
    s16 volR;  /* +0x2 -- right volume */
    s16 pitch; /* +0x4 -- sample rate; 0x1000 plays at 44.1 kHz */
    s16 addr;  /* +0x6 -- start address in sound RAM, in 8-byte units */
    s16 adsr1; /* +0x8 */
    s16 adsr2; /* +0xA */
    u16 envx;  /* +0xC -- current envelope level; 0 once the voice has died */
    u8 padE[0x10 - 0xE];
} SpuVoiceRegs;

typedef struct {
    SpuVoiceRegs voice[24]; /* +0x000 */
    u8 pad180[0x188 - 0x180];
    u16 keyOn[2];  /* +0x188 -- voices 0-15, 16-23: a set bit keys the voice on */
    u16 keyOff[2]; /* +0x18C -- a set bit releases the voice */
    u8 pad190[0x194 - 0x190];
    u16 noiseOn[2];  /* +0x194 -- a set bit plays the voice from the noise generator */
    u16 reverbOn[2]; /* +0x198 -- a set bit sends the voice to the reverb */
} SpuRegs;

#endif
