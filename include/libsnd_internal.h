#ifndef LIBSND_INTERNAL_H
#define LIBSND_INTERNAL_H

/*
 * libsnd_internal.h -- the declarations libsnd's own modules share: the
 * voice manager's functions and globals (libsnd/vmanager and its siblings),
 * the sequencer tick, and the sound system's state flags.
 *
 * Sony's, not the game's. No libsnd internal header ships on any SDK disc,
 * so this one is the project's: each prototype has its definition's types,
 * and a function whose callers disagree with its definition says so on its
 * declaration. Sony's public API stays in <libsnd.h>.
 */
#include "common.h"
#include <libsnd.h>
#include "ss_score.h"
#include "svm_data.h"

/* libsnd/seqread: one tick of sequence _ss_score[access][seq]. */
void SeqPlay(s16 access, s16 seq);

/* libsnd/vmanager. */
/* MATCHING: declared without a parameter list. The voice manager's own
 * calls pass an argument (0 or 0xFF) and SsUtKeyOn/SsUtKeyOnV pass none;
 * the definition reads no argument register. */
s32 SpuVmAlloc();
void SpuVmKeyOnNow(s32 unused, s32 pitch);
void SpuVmDoAllocate(void);
void vmNoiseOn(s32 voice);
s32 note2pitch2(s32 note, s32 fine);
void SpuVmInit(s32 voices); /* the voices to manage, at most 24 */
void SpuVmNoiseOnWithAdsr(s32 a0, s32 a1, s32 a2, s32 a3);
void SpuVmNoiseOff(void);
void SpuVmNoiseOn(s32 a0, s32 a1);
s16 SpuVmPBVoice(s16 a0, s16 a1, s16 a2, s16 a3, u16 a4);
/* SpuVmPitchBend is not here: its definition narrows its parameters to s16,
 * and SetPitchBend (libsnd/seqread) passes its first argument unnarrowed,
 * so seqread declares it locally. */
void SeAutoVol(s16 voice, s16 from, s16 to, s16 duration);
void SeAutoPan(s16 a0, s16 a1, s16 a2, s16 a3);
void SpuVmFlush(void);
s32 SpuVmKeyOn(s32 seqSepNo, s16 vabId, s16 prog, u16 note, u16 vol, u16 pan);
s32 SpuVmKeyOff(s16 a0, s16 a1, s16 a2, u16 a3);
s32 SpuVmSeKeyOn(s32 vabId, s32 prog, s32 note, s32 unused, u16 volL, u16 volR);
s32 SpuVmSeKeyOff(s16 vabId, s16 prog, u16 note);
/* A sequence is named by one packed number, seqSepNo: the SEQ/SEP access in
 * the low byte, the sequence in the high byte. MATCHING: SpuVmSetSeqVol's
 * volumes are u16; Snd_crescendo's calls mask them with andi 0xFFFF. */
s32 SpuVmSetSeqVol(s16 seqSepNo, u16 volL, u16 volR, s32 a3);
s32 SpuVmGetSeqVol(s32 seqSepNo, s16 *volL, s16 *volR);
s32 SpuVmGetSeqLVol(s32 seqSepNo);
s32 SpuVmGetSeqRVol(s32 seqSepNo);
s32 SpuVmSeqKeyOff(s32 seqSepNo);

/* libsnd/vm_vol. */
s32 SpuVmSetVol(s32 a0, s32 a1, s32 a2, s32 a3, u16 a4);

/* Linked from Sony's objects: vm_vsu, vm_don, vm_doff, vm_prog. */
s32 SpuVmVSetUp(s16 a0, s16 a1);
void SpuVmDamperOn(void);
void SpuVmDamperOff(void);
s32 SpuVmSetProgVol(s16 a0, s16 a1, s32 a2);

/* The voice manager's globals. The key masks are two halfwords each,
 * voices 0-15 then 16-23; SpuVmFlush writes them to the SPU once per tick. */
extern VabHdr *_svm_vh;      /* the header of the VAB being played */
extern ProgAtr *_svm_pg;     /* that VAB's program attributes */
extern VagAtr *_svm_tn;      /* that VAB's tone attributes, 16 per program */
extern u8 spuVmMaxVoice;     /* voices the allocator may hand out */
extern s16 _svm_stereo_mono; /* 1: mono, both channels at the louder volume */
extern u16 _svm_okon1;       /* voices to key on at the next flush */
extern u16 _svm_okon2;
extern u16 _svm_okof1; /* voices to key off at the next flush */
extern u16 _svm_okof2;
extern u16 _svm_orev1; /* voices sent to reverb */
extern u16 _svm_orev2;

/*
 * _svm_cur (pinned in config/psyq-objects.ld, 0x20 bytes): the key-on
 * request the voice manager is working on, which the key-on paths
 * (SpuVmKeyOn, SsUtKeyOn, SsUtKeyOnV) fill before they call into the
 * allocator and the pitch helpers. Sony's struct type is not on any SDK
 * disc, so each byte is its own extern, named by address, with its offset
 * in _svm_cur. The three pans cut the right channel below 64 and the left
 * one above. +0x0C, the tone, is declared by each unit that reads it:
 * libsnd_vmanager.c reads it plain, libsnd_vm_vol_ut_key_ut_keyv.c volatile.
 */
extern u8 D_8008EA0C; /* +0x00: the program's tone count */
extern u8 D_8008EA0E; /* +0x02: the note to key */
extern u8 D_8008EA0F; /* +0x03: the note's fine tune */
extern u8 D_8008EA10; /* +0x04: volume, scaled by the VAB's mvol */
extern u8 D_8008EA11; /* +0x05: third pan */
extern u8 D_8008EA13; /* +0x07: program; a VAB gives each 16 tone slots in _svm_tn */
extern u8 D_8008EA16; /* +0x0A: first volume factor, out of 127 */
extern u8 D_8008EA17; /* +0x0B: second pan */
extern u8 D_8008EA19; /* +0x0D: second volume factor, out of 127 */
extern u8 D_8008EA1A; /* +0x0E: first pan */
extern u8 D_8008EA1B; /* +0x0F: priority, which SpuVmAlloc compares */
extern u8 D_8008EA1C; /* +0x10: the tone's centre note */
extern u8 D_8008EA1D; /* +0x11: the tone's fine tune, 8 per pitch-table step */
extern u8 D_8008EA1E; /* +0x12: the tone's lowest note (VagAtr min) */
extern u8 D_8008EA1F; /* +0x13: the tone's highest note (VagAtr max) */
extern u8 D_8008EA20; /* +0x14: mode bits; bit 2 sends the voice to reverb */
extern u16 D_8008EA22; /* +0x16: _ss_score index, access in the low byte; 0x21 from SsUtKeyOn/SsUtKeyOnV: no score */
extern u16 D_8008EA24; /* +0x18: the VAG number the tone plays, 0xFF for noise */
/* +0x1A: the voice being keyed. MATCHING: volatile; the key paths store it
 * and read it straight back, and without the qualifier cc1 drops the
 * reload. */
extern volatile u16 D_8008EA26;

/* libsnd/ssinit's globals. */
extern SsMarkCallbackProc _ss_MarkCallback[0x20][16]; /* SsSetMarkCallback's, one per (access, sequence);
                                                       * _SsInit clears them and ContNrpn1 calls them */
extern s32 _snd_openflag;
extern s32 _snd_ev_flag;       /* the sound system's reentrancy lock */
extern s32 _snd_video_mode;    /* the video mode SsSetTickMode reads, 0 or 1 */
extern s32 _snd_seq_no_tick;   /* 1: SsSetTickMode was given SS_NOTICK */
extern s32 _snd_seq_tick_mode; /* SsSetTickMode's mode, SS_NOTICK cleared */
extern s32 _snd_use_vsync_cb;  /* nonzero: _SsStart installed a vsync callback */
extern s32 _snd_use_interrupt_id;
extern s32 _snd_1per2;              /* nonzero: the sequencer runs on every second tick */
extern void (*_snd_vsync_cb)(void); /* the vsync callback _SsTrapIntrVSync chains to */
extern u32 VBLANK_MINUS;            /* the sequencer's tick rate, 50, 60, 120 or 240 */

#endif
