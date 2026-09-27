#ifndef VABSTREAMOBJ_H
#define VABSTREAMOBJ_H

#include "FileResource.h"

/*
 * VabStreamObj -- one VAB sound bank, loaded from disc through the active
 * data source and played through Sony's libsnd (class id 0xA03, method table
 * gVabStreamObjMethods). It is a FileResource subclass and a sibling of the
 * drivers (VabDriver 0x23, the CD driver 0x13), not derived from either. Like
 * every data source, its ctor and finalize chain to the ACTIVE driver's
 * (GetActiveDataSourceMethods). Methods in src/code_179d8_d.c.
 *
 * Loading. The ctor copies the base path (`baseFilename`) and asks the
 * driver for "<base>.VH" (requestLoadFile, +0x06C), with `loadState` set to
 * VABSTREAM_LOAD_HEADER. The driver calls setFlag (+0x064,
 * VabStreamObj__AdvanceLoadState) when a request completes. In the header
 * state, with CD_FLAG_LOAD_FILE_DONE (0x200) in `flags`, it opens the
 * header (SsVabOpenHead) and loads "<base>.VB" (loadFile, +0x058) in
 * VABSTREAM_LOAD_BODY. In that state it transfers the body (SsVabTransBody)
 * and calls slot78, VabStreamObj__OnBodyReady. That waits on SsVabTransCompleted and then
 * runs loadVagAttrs (+0x07C), which caches the bank's VagAtr records per
 * program. +0x058 and +0x06C are NULL in the static table.
 * SetActiveDataSource fills them from the active driver
 * (GetVabStreamObjMethods is in gDataSourceClientGetters).
 *
 * Playing. playTone(index, vol, endVol) keys on program index >> 4, tone
 * index & 0xF, at the tone's centre note plus `pitchOffset`. It returns the
 * voice, or -1. stopVoice(voice) keys one voice off, or all voices when
 * voice >= 24. The holders keep the object as whatever their own field type
 * is (TaskCore::sound, TimedTask::sound, DreamSys::soundObj, WBgm::vab) and
 * cast to VabStreamObj * where they call through it. The SoundCueSet queue
 * (InitSoundCueSet/FlushSoundCueSet/ServiceSoundCueSet) takes this object
 * as its first argument, but those are free functions, not methods.
 *
 * NO FIELDS/SLOTS MACROS: no class lies below 0xA03 (`typeviews.py --tree`).
 */

typedef struct VabStreamObj VabStreamObj;
typedef struct VabStreamObjMethods VabStreamObjMethods;

/* The two bytes of Sony's VagAtr (include/psyq/libsnd.h, 32 bytes) that
 * PlayTone reads, at VagAtr's own offsets. Kept reduced so this header does
 * not bring in LIBSND.H's prototypes, which the units declare locally. */
typedef struct VabStreamVagAtr {
    /* +0x00 */ u8 pad0[0x4];
    /* +0x04 */ u8 center; /* VagAtr::center, the tone's centre note */
    /* +0x05 */ u8 shift;  /* VagAtr::shift, centre-note fine tune */
    /* +0x06 */ u8 pad6[0x20 - 0x6];
} VabStreamVagAtr;

/* The two fields of Sony's VabHdr (32 bytes) that LoadVagAttrs reads. */
typedef struct VabStreamVabHdr {
    /* +0x00 */ u8 pad0[0x12];
    /* +0x12 */ u16 ts; /* program count */
    /* +0x14 */ u16 vs; /* VAG count */
    /* +0x16 */ u8 pad16[0x20 - 0x16];
} VabStreamVabHdr;

/* VabStreamObj's FileResource::loadState, advanced by
 * VabStreamObj__AdvanceLoadState. The values are the game's own; nothing
 * uses 2 to 5. */
enum VabStreamLoadState {
    VABSTREAM_LOAD_IDLE = 0,   /* nothing requested */
    VABSTREAM_LOAD_HEADER = 1, /* waiting for "<base>.VH" */
    VABSTREAM_LOAD_BODY = 6    /* waiting for "<base>.VB" */
};

/* slot78 is FileResource's `void *slot78` (NULL there). This class's occupant,
 * VabStreamObj__OnBodyReady, is called through this typedef. That takes no
 * code (FINISHING-PLAN track 4 step 6). */
typedef s32 (*VabStreamObjOnBodyReadyFn)(VabStreamObj *self, s32 done);

struct VabStreamObjMethods {
    /* ctor: New_VabStreamObj passes the bank's base path (no extension). */
    FILERESOURCE_SLOTS(VabStreamObj, (VabStreamObj * self, char *path));
    /* +0x07C */ void (*loadVagAttrs)(VabStreamObj *self); /* VabStreamObj__LoadVagAttrs */
    /* +0x080 */ s32 (*playTone)(VabStreamObj *self, s32 index, s32 vol,
                                 s32 endVol); /* VabStreamObj__PlayTone: the voice, or -1 */
    /* +0x084 */ s32 (*stopVoice)(VabStreamObj *self, s32 voice); /* VabStreamObj__StopVoice: always -1 */
    /* +0x088 */ s32 (*mute)(VabStreamObj *self);                 /* VabStreamObj__Mute */
    /* +0x08C */ s32 (*unmute)(VabStreamObj *self);               /* VabStreamObj__Unmute */
    /* +0x090 */ void (*slot90)(void); /* VabStreamObj__NoOpSlot90; no caller */
    /* +0x094 */ void (*slot94)(void); /* VabStreamObj__NoOpSlot94; no caller */
    /* +0x098 */ void (*slot98)(void); /* VabStreamObj__NoOpSlot98; no caller */
    /* +0x09C */ void (*setPitchOffset)(VabStreamObj *self,
                                        s32 octave); /* VabStreamObj__SetPitchOffset: pitchOffset = octave * 12 - 24 */
}; /* 39 slots, 0xA0 bytes */

struct VabStreamObj {
    FILERESOURCE_FIELDS(VabStreamObjMethods); /* buffer: the loaded .VH, then the .VB; loadState is a VabStreamLoadState */
    /* +0x02C */ VabStreamVabHdr vabHdr;       /* SsUtGetVabHdr */
    /* +0x04C */ VabStreamVagAtr *vagAttrPool; /* vabHdr.vs records */
    /* +0x050 */ VabStreamVagAtr **progVagTable; /* vabHdr.ts pointers into vagAttrPool, one per program */
    /* +0x054 */ s16 vabId;      /* SsVabOpenHead / SsVabTransBody; WBgm opens its SEQ on it */
    /* +0x056 */ s16 muted;      /* Mute / Unmute */
    /* +0x058 */ u16 attrsReady; /* set by OnBodyReady; WBgm waits on it */
    /* +0x05A */ u16 bodyTransferPending; /* set when SsVabTransBody succeeds */
    /* +0x05C */ void *baseFilename; /* the ctor's copy of the path; freed once the .VB is requested */
    /* +0x060 */ s32 pitchOffset;    /* semitones added to a tone's centre note */
}; /* 0x64 bytes: New_VabStreamObj */

extern VabStreamObjMethods gVabStreamObjMethods;
extern VabStreamObjMethods *GetVabStreamObjMethods(void);

VabStreamObj *New_VabStreamObj(char *path);
void VabStreamObj__VabStreamObj(VabStreamObj *self, char *path);
void VabStreamObj__Finalize(VabStreamObj *self);
void VabStreamObj__AdvanceLoadState(VabStreamObj *self);
s32 VabStreamObj__OnBodyReady(VabStreamObj *self, s32 done);
void VabStreamObj__LoadVagAttrs(VabStreamObj *self);
s32 VabStreamObj__PlayTone(VabStreamObj *self, s32 index, s32 vol, s32 endVol);
s32 VabStreamObj__StopVoice(VabStreamObj *self, s32 voice);
s32 VabStreamObj__Mute(VabStreamObj *self);
s32 VabStreamObj__Unmute(VabStreamObj *self);
void VabStreamObj__NoOpSlot90(void);
void VabStreamObj__NoOpSlot94(void);
void VabStreamObj__NoOpSlot98(void);
void VabStreamObj__SetPitchOffset(VabStreamObj *self, s32 octave);

#endif
