#ifndef SVMDATA_H
#define SVMDATA_H

#include "common.h"

/*
 * SvmData.h -- libsnd vmanager's per-voice bss tables: _svm_sreg_buf,
 * _svm_sreg_dirty and _svm_voice. (Round 86 created this file as
 * SvmVoice.h for _svm_voice alone and renamed it when the other two
 * joined.)
 *
 * SvmVoice -- one record of libsnd's per-voice table _svm_voice.
 *
 * Sony's, not the game's. libsnd/vmanager.o (Psy-Q disc 3.5) defines the
 * bss block _svm_sreg_buf +0x000, _svm_sreg_dirty +0x180, _svm_voice
 * +0x198, _svm_envx_ptr +0x678, _svm_envx_hist +0x67C. Anchored at
 * 0x8008D7F0 every one of them lands, so _svm_voice = 0x8008D988 and its
 * 0x4E0 bytes are 24 voices x 0x34 (ending exactly at _svm_envx_ptr).
 *
 * The type name is derived from Sony's VARIABLE name, because Sony's own
 * struct tag is unknown: no libsnd internal header ships on any SDK disc.
 * For the same reason every field is named by its OFFSET only. Per
 * FINISHING-PLAN's Sony rule (track 3, "Naming rules") NO game name goes
 * on any field of this struct: only libsnd functions read it. A comment
 * describing a field's MECHANICS is fine and is kept below.
 *
 * Field types are what the accessors need (track 4b step 1): the width
 * every accessor agrees on, and the signedness most of them read. The one
 * accessor that reads otherwise, SePitchBend (+0x0C unsigned, +0x10 and
 * +0x14 as bytes), uses VALUE casts at the site -- (u16)v.unk0C,
 * (u8)v.unk10 -- which are byte-exact; the address-cast spelling
 * *(u8 *)&v.unk10 is not (it grows that function's frame by 8 bytes).
 * No unit keeps a signedness view. SpuVmFlush keeps a u16 walk type over
 * +0x06, for its pointer stride, not for signedness.
 *
 * Earlier rounds split this table into one splat symbol per field
 * (D_8008D98A, D_8008D98C, ... at a 0x34 stride) and gave twelve of them
 * game names; round 86 (alpha, track 2) merged them back into this one
 * table. splat still prints the per-address auto-symbols in asm/ (the
 * table lies past the global segment's vram range, so splat never hands
 * _svm_voice's size to spimdisasm); the linker resolves both spellings to
 * the same bytes.
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
 * D_8006DAD4 (the SPU voice register block) for every voice whose
 * _svm_sreg_dirty byte has bits set. Same provenance and naming rule as
 * SvmVoice: libsnd/vmanager.o bss +0x000, 24 voices x 0x10, 0x8008D7F0;
 * fields by offset only.
 */
typedef struct SvmSreg {
    s16 unk0; /* +0x0 -- SpuVmFlush copies +0x0/+0x2 out when dirty bit 0x1 is set */
    s16 unk2; /* +0x2 */
    u16 unk4; /* +0x4 -- note2pitch2 result (SePitchBend, SpuVmPBVoice); copied out on dirty bit 0x4 */
    u16 unk6; /* +0x6 -- copied out on dirty bit 0x8 */
    s16 unk8; /* +0x8 -- SsUtChangeADSR stores its p4; +0x8/+0xA copied out on dirty bit 0x10 */
    s16 unkA; /* +0xA -- SsUtChangeADSR stores its p5 */
    u8 padC[0x10 - 0xC];
} SvmSreg; /* 0x10 */

extern SvmSreg _svm_sreg_buf[]; /* 24 voices */
extern u8 _svm_sreg_dirty[];    /* 24 voices: which _svm_sreg_buf fields SpuVmFlush must copy out */

/*
 * SpuVoiceRegs, SpuRegs -- the PS1 SPU's own register block at 0x1F801C00,
 * which libsnd/vmanager.o's first .data word (D_8006DAD4) points at: 24
 * voices of 0x10 bytes, then the global key-on/key-off/noise/reverb bit
 * masks. Unlike the bss tables above these are hardware registers, so the
 * fields carry the hardware's names; only registers some accessor touches
 * are named. Units declare D_8006DAD4 themselves, each with the pointee
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
