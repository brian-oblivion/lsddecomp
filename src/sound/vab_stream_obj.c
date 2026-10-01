/*
 * VabStreamObj's methods (include/vab_stream_obj.h, whose class
 * documentation describes the load sequence and libsnd's set-up: one VAB
 * sound bank, loaded through the active data source and played through
 * libsnd), in ROM order: the allocator, ctor and finalize, the load steps
 * (AdvanceLoadState, OnBodyReady, LoadVagAttrs), then playTone, stopVoice,
 * mute and unmute, three empty slots and setPitchOffset, ending with its
 * getter GetVabStreamObjMethods; then GetOpenVabCount and
 * GetSsTicksPerSecond. Its method table closes the file. A (void *) entry
 * in it is a method whose declared type differs from its slot's, usually
 * one inherited from a parent class and declared on the parent's type.
 */
#include "common.h"
#include <libsnd.h>
#include "vab_stream_obj.h"
#include "bmem_pmgr.h"
#include <strings.h>
#include "wbgm.h"
#include "data_source.h"

/* libsnd set-up, done once and undone when the last bank closes: SsInit and
 * the size table; the tick mode; SsStart and the master volume. All start
 * at 0, nothing set up. */
static s32 sVabSizeTableInited SDATA = 0;
static s32 sVabStreamInited SDATA = 0;
static s32 sVabVolumeInited SDATA = 0;
static s32 sOpenVabCount SDATA = 0; /* VabStreamObjs constructed and not yet finalized */
/* The .VH buffer, kept from the header state until LoadVagAttrs takes it
 * back as the object's buffer. */
static void *sPendingVabBuffer SDATA = NULL;
static s32 sSsTicksPerSecond SDATA = 0; /* the SsSetTickMode rate, for callers timing in ticks */

/* A bank's two files: its header and its body. */
static char sVabHeaderSuffix[] SDATA = ".VH";
static char sVabBodySuffix[] SDATA = ".VB";

VabStreamObj *New_VabStreamObj(char *path) {
    VabStreamObj *self;

    self = BMemPMgrAlloc(sizeof(VabStreamObj));
    if (self != NULL) {
        GetVabStreamObjMethods()->ctor(self, path);
        return self;
    }
    return NULL;
}

/* The "<base>.VH"/"<base>.VB" path buffers the ctor and AdvanceLoadState
 * build on the stack. */
#define VAB_PATH_SIZE 32

/* SsSetMVol's level for both channels, set once when the first bank's
 * attributes are loaded (Sony's maximum is 127). */
#define VAB_MASTER_VOLUME 120

void VabStreamObj__VabStreamObj(VabStreamObj *self, char *path) {
    void *buf;
    char vhPath[VAB_PATH_SIZE];

    GetActiveDataSourceMethods()->ctor((FileResource *)self);
    self->methods = GetVabStreamObjMethods();
    self->vagAttrPool = NULL;
    self->progVagTable = NULL;
    self->vabId = 0;
    self->muted = 0;
    self->methods->setPitchOffset(self, 0);
    self->attrsReady = 0;
    self->bodyTransferPending = 0;
    self->baseFilename = NULL;
    if (sVabSizeTableInited == 0) {
        SsInit();
        sVabSizeTableInited = 1;
        SsSetTableSize(GetSsSizeTableBuf(), 2, 1); /* two scores of one track */
    }
    if (sVabStreamInited == 0) {
        sSsTicksPerSecond = 60; /* the rate SS_TICK60 sets */
        SsSetTickMode(SS_TICK60);
        sVabStreamInited = 1;
    }
    sOpenVabCount++;
    if (path != NULL) {
        buf = BMemPMgrAlloc(strlen(path) + 1);
        if (buf != NULL) {
            self->baseFilename = buf;
            strcpy(buf, path);
            BuildFileName(vhPath, buf, NULL, sVabHeaderSuffix);
            self->loadState = VABSTREAM_LOAD_HEADER;
            self->methods->requestLoadFile(self, vhPath);
        }
    }
}

void VabStreamObj__Finalize(VabStreamObj *self) {
    SsVabClose(self->vabId);
    if (--sOpenVabCount < 0) {
        sOpenVabCount = 0;
    }
    if (sOpenVabCount == 0 && IsWBgmActive() == 0) {
        sVabSizeTableInited = 0;
        sVabVolumeInited = 0;
        sVabStreamInited = 0;
        SsEnd();
        SsQuit();
    }
    BMemPMgrFree(self->vagAttrPool);
    BMemPMgrFree(self->progVagTable);
    BMemPMgrFree(self->baseFilename);
    GetActiveDataSourceMethods()->finalize((FileResource *)self);
}

void VabStreamObj__AdvanceLoadState(VabStreamObj *self) {
    char path[VAB_PATH_SIZE];

    switch (self->loadState) {
        case VABSTREAM_LOAD_IDLE:
            break;
        case VABSTREAM_LOAD_HEADER:
            if (self->flags & CD_FLAG_LOAD_FILE_DONE) {
                self->vabId = SsVabOpenHead(self->buffer, -1);
                BuildFileName(path, self->baseFilename, NULL, sVabBodySuffix);
                sPendingVabBuffer = self->buffer;
                self->loadState = VABSTREAM_LOAD_BODY;
                self->buffer = NULL;
                self->methods->loadFile(self, path);
                if (self->baseFilename != NULL) {
                    BMemPMgrFree(self->baseFilename);
                    self->baseFilename = NULL;
                }
            }
            break;
        case VABSTREAM_LOAD_BODY:
            if (self->flags & CD_FLAG_LOAD_FILE_DONE) {
                self->vabId = SsVabTransBody(self->buffer, self->vabId);
                if (self->vabId != -1) {
                    self->bodyTransferPending = 1;
                    ((VabStreamObjOnBodyReadyFn)self->methods->processBuffer)(self, 1);
                }
            }
            break;
        default:
            break;
    }
}

s32 VabStreamObj__OnBodyReady(VabStreamObj *self, s32 done) {
    s32 result;

    result = 0;
    if (self->bodyTransferPending != 0) {
        if (done != 0) {
            SsVabTransCompleted(1);
            self->bodyTransferPending = 0;
            self->attrsReady = 1;
            self->methods->loadVagAttrs(self);
            result = 1;
        }
    }
    return result;
}

void VabStreamObj__LoadVagAttrs(VabStreamObj *self) {
    ProgAtr prog;
    VabStreamVagAtr *pool;
    s32 i;
    s32 j;
    s16 result;

    if (self->attrsReady == 0) {
        return;
    }
    self->methods->freeBuffer(self);
    self->buffer = sPendingVabBuffer;
    result = SsUtGetVabHdr(self->vabId, (VabHdr *)&self->vabHdr);
    if (result == -1) {
        return;
    }
    self->vagAttrPool = BMemPMgrAlloc(self->vabHdr.vs * sizeof(VabStreamVagAtr));
    if (self->vagAttrPool == NULL) {
        return;
    }
    self->progVagTable = BMemPMgrAlloc(self->vabHdr.ts * sizeof(VabStreamVagAtr *));
    if (self->progVagTable == NULL) {
        return;
    }
    pool = self->vagAttrPool;
    for (i = 0; i < self->vabHdr.ts; i++) {
        self->progVagTable[i] = pool;
        result = SsUtGetProgAtr(self->vabId, i, &prog);
        if (result == -1) {
            return;
        }
        for (j = 0; j < prog.tones; j++) {
            result = SsUtGetVagAtr(self->vabId, i, j, (VagAtr *)pool);
            if (result == -1) {
                return;
            }
            pool++;
        }
    }
    if (sVabVolumeInited == 0) {
        SsStart();
        SsSetMVol(VAB_MASTER_VOLUME, VAB_MASTER_VOLUME);
        sVabVolumeInited = 1;
    }
}

/* Key on the tone a packed index names, at vol, and ramp it to endVol.
 * Returns the voice, or VAB_NO_VOICE. */
s32 VabStreamObj__PlayTone(VabStreamObj *self, s32 index, s32 vol, s32 endVol) {
    s32 program;
    s32 tone;
    VabStreamVagAtr *entry;
    VabStreamVagAtr *row;
    s16 result;

    if (index >= 0) {
        program = index >> VAB_TONE_BITS;
        /* MATCHING: row is read before tone is worked out; the other way round,
         * tone simplifies to index & 0xF, which retail does not do. */
        row = self->progVagTable[program];
        tone = index - program * VAB_TONES_PER_PROG;
        entry = &row[tone];
        result = SsUtKeyOn(self->vabId, (s16)program, (s16)tone,
                           (s16)(entry->center + self->pitchOffset), entry->shift, (s16)vol, (s16)vol);
        if (result >= 0) {
            SsUtAutoVol(result, (s16)vol, (s16)endVol, 2);
            return result;
        }
    }
    return VAB_NO_VOICE;
}

/* The SPU's voice count: StopVoice keys off every voice for a voice number
 * past the last. */
#define SPU_VOICE_COUNT 24

s32 VabStreamObj__StopVoice(VabStreamObj *self, s32 voice) {
    if (voice < SPU_VOICE_COUNT) {
        SsUtKeyOffV(voice);
    } else {
        SsUtAllKeyOff(0);
    }
    return VAB_NO_VOICE;
}

s32 VabStreamObj__Mute(VabStreamObj *self) {
    s32 flag;

    flag = self->muted;
    if (flag == 0) {
        SsSetMute(1);
        flag = 1;
        self->muted = flag;
    }
    return flag;
}

s32 VabStreamObj__Unmute(VabStreamObj *self) {
    s32 result;

    result = self->muted;
    if (result != 0) {
        /* MATCHING: retail returns what SsSetMute returns (SpuSetMute's result),
         * though <libsnd.h> declares it void, so the call is cast. */
        result = ((s32 (*)(char))SsSetMute)(0);
        self->muted = 0;
    }
    return result;
}

void VabStreamObj__NoOpSlot90(void) {}

void VabStreamObj__NoOpSlot94(void) {}

void VabStreamObj__NoOpSlot98(void) {}

/* setPitchOffset's argument is an octave: 2 plays a tone at its centre
 * note, each step away shifts it an octave. */
#define SEMITONES_PER_OCTAVE 12

void VabStreamObj__SetPitchOffset(VabStreamObj *self, s32 octave) {
    self->pitchOffset = octave * SEMITONES_PER_OCTAVE - 2 * SEMITONES_PER_OCTAVE;
}

VabStreamObjMethods *GetVabStreamObjMethods(void) {
    return &gVabStreamObjMethods;
}

s32 GetOpenVabCount(void) {
    return sOpenVabCount;
}

s32 GetSsTicksPerSecond(void) {
    return sSsTicksPerSecond;
}

/* VabStreamObj (include/vab_stream_obj.h): FileResource's table with the
 * ctor, finalize, the load-state step as onRequestDone and OnBodyReady as
 * processBuffer, then its voice slots. */
VabStreamObjMethods gVabStreamObjMethods = {
    /* +0x000 header */ VABSTREAMOBJ_CLASS_ID,
    /* +0x004 release */ (void *)FileResource__Release,
    /* +0x008 ctor */ VabStreamObj__VabStreamObj,
    /* +0x00C finalize */ VabStreamObj__Finalize,
    /* +0x010 addChild */ (void *)BasicClass__AddChild,
    /* +0x014 removeChild */ (void *)BasicClass__RemoveChild,
    /* +0x018 removeAllChildren */ (void *)BasicClass__RemoveAllChildren,
    /* +0x01C getNextChild */ (void *)BasicClass__GetNextChild,
    /* +0x020 addParentRef */ (void *)BasicClass__AddParentRef,
    /* +0x024 removeParentRef */ (void *)BasicClass__RemoveParentRef,
    /* +0x028 clearParentRefs */ (void *)BasicClass__ClearParentRefs,
    /* +0x02C getNextParentRef */ (void *)BasicClass__GetNextParentRef,
    /* +0x030 notifyParents */ (void *)BasicClass__NotifyParents,
    /* +0x034 slot34 */ BasicClass__NoOpSlot34,
    /* +0x038 onNotify */ (void *)BasicClass__OnNotify,
    /* +0x03C slot3C */ NULL,
    /* +0x040 slot40 */ NULL,
    /* +0x044 open */ NULL,
    /* +0x048 close */ NULL,
    /* +0x04C seek */ NULL,
    /* +0x050 slot50 */ NULL,
    /* +0x054 read */ NULL,
    /* +0x058 loadFile */ NULL,
    /* +0x05C freeBuffer */ (void *)FileResource__FreeBuffer,
    /* +0x060 slot60 */ NoOp,
    /* +0x064 onRequestDone */ VabStreamObj__AdvanceLoadState,
    /* +0x068 runRequestQueue */ NULL,
    /* +0x06C requestLoadFile */ NULL,
    /* +0x070 stopService */ NULL,
    /* +0x074 cancelRequests */ NULL,
    /* +0x078 processBuffer */ VabStreamObj__OnBodyReady,
    /* +0x07C loadVagAttrs */ VabStreamObj__LoadVagAttrs,
    /* +0x080 playTone */ VabStreamObj__PlayTone,
    /* +0x084 stopVoice */ VabStreamObj__StopVoice,
    /* +0x088 mute */ VabStreamObj__Mute,
    /* +0x08C unmute */ VabStreamObj__Unmute,
    /* +0x090 slot90 */ VabStreamObj__NoOpSlot90,
    /* +0x094 slot94 */ VabStreamObj__NoOpSlot94,
    /* +0x098 slot98 */ VabStreamObj__NoOpSlot98,
    /* +0x09C setPitchOffset */ VabStreamObj__SetPitchOffset,
};
