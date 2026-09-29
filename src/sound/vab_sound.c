/*
 * vab_sound.c -- two subjects in one file: PlacementGrid, then the
 * VAB sound backend (NullDriver, VabStreamObj and the SoundCueSet's init,
 * flush and per-tick service), with ReturnZero between them.
 *
 * PlacementGrid (include/placement_grid.h), New_PlacementGrid to
 * GetPlacementGridMethods: the model placements of one map chunk's 20 x 20
 * cells: its allocator, ctor, finalize, read-done flag, the processBuffer
 * occupant that turns one placement record per call into a CellPlacement and
 * its model, and the table getter. ReturnZero follows; nothing calls it or
 * points at it.
 *
 * The type LinkResource's getModel is called through,
 * PlacementGridGetModelFn, is this file's own. The three classes' method
 * tables end the file.
 */
#include "common.h"
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include "null_driver.h"
#include "placement_grid.h"
#include "link_resource.h"
#include "stage_map.h"
#include <libsnd.h>
#include "vab_stream_obj.h"
#include "sound_cue_set.h"
#include "bmem_pmgr.h"
#include <strings.h>
#include "wbgm.h"
#include "data_source.h"

/** @brief What `buffer` points at: 8 bytes nothing here reads, then each
 * cell's first record, one per cell of the chunk's 20 x 20 lattice,
 * row-major. A cell's further records are reached by the byte offset in
 * `next`. */
typedef struct PlacementGridBuffer {
    u8 pad0[8];
    PlacementGridRecord cells[STAGE_SLOT_LATTICE_CELLS]; /**< each cell's first record, row-major */
} PlacementGridBuffer;

/* LinkResource__GetModel as PlacementGrid__ResolveEntry calls it through
 * `linkResource`'s getModel (+0x080). The occupant reads only (self, index). */
/* MATCHING: all four arguments, as retail passes them, though the occupant reads two */
typedef s32 (*PlacementGridGetModelFn)(LinkResource *self, s32 model, s32 cell, CellPlacement *placement);

PlacementGrid *New_PlacementGrid(char *name) {
    PlacementGrid *self;
    PlacementGridMethods *table;

    self = BMemPMgrAlloc(sizeof(PlacementGrid));
    if (self != NULL) {
        table = GetPlacementGridMethods();
        table->ctor(self, name);
        return self;
    }
    return NULL;
}

void PlacementGrid__PlacementGrid(PlacementGrid *self, char *name) {
    GetActiveDataSourceMethods()->ctor((FileResource *)self);
    self->methods = GetPlacementGridMethods();
    self->linkResource = NULL;
    self->loaded = 0;
    if (name != NULL) {
        self->methods->requestLoadFile(self, name);
    }
}

void PlacementGrid__Finalize(PlacementGrid *self) {
    GetActiveDataSourceMethods()->finalize((FileResource *)self);
}

void PlacementGrid__OnRequestDone(PlacementGrid *self) {
    self->loaded = 1;
    GetActiveDataSourceMethods()->onRequestDone((FileResource *)self);
}

/* Fill `placement` from cell `cell`'s first record, or, when
 * placement->next is set, from the record it points at, and return the
 * record's model from linkResource. -1: the record is empty; 0: `cell` is
 * past the lattice, the caller's end of the walk. */
s32 PlacementGrid__ResolveEntry(PlacementGrid *self, CellPlacement *placement, s32 cell) {
    PlacementGridRecord *rec;
    LinkResource *link;
    s32 row;
    s32 col;
    s32 model;

    if (cell < STAGE_SLOT_LATTICE_CELLS) {
        if (placement->next != 0) {
            rec = (PlacementGridRecord *)((u8 *)self->buffer + placement->next);
            placement->chained = 1;
        } else {
            rec = &((PlacementGridBuffer *)self->buffer)->cells[cell];
            placement->chained = 0;
        }
        placement->next = rec->next;
        if (rec->present != 0) {
            row = cell / STAGE_CHUNK_CELLS;
            col = cell - row * STAGE_CHUNK_CELLS;
            placement->x = (col << STAGE_CELL_SHIFT) + STAGE_CELL_SIZE / 2;
            placement->y = (s32)rec->y << STAGE_CELL_SHIFT;
            placement->z = (row << STAGE_CELL_SHIFT) + STAGE_CELL_SIZE / 2;
            placement->rotY = rec->rotY * ANGLE_DEG(90);
            placement->unk2C = rec->unk1;
            placement->cellFlags = rec->cellFlags;
            model = rec->model;
            placement->model = model;
            link = self->linkResource;
            return ((PlacementGridGetModelFn)link->methods->getModel)(link, model, cell, placement);
        }
        return -1;
    }
    return 0;
}

PlacementGridMethods *GetPlacementGridMethods(void) {
    return &gPlacementGridMethods;
}

s32 ReturnZero(void) {
    return 0;
}

void NullDriver__NullDriver(void) {}

void NullDriver__Destroy(void) {}

void NullDriver__NoOpSlot40(void) {
    /* MATCHING: retail reserves a 64-byte frame it never touches. */
    char unused[64];
}

void NullDriver__Open(void) {
    /* MATCHING: the same unused 64-byte frame. */
    char unused[64];
}

void NullDriver__Close(void) {}

void NullDriver__Seek(void) {}

void NullDriver__NoOpSlot50(void) {}

/*
 * The VAB sound backend: the rest of NullDriver's empty slots and its mode
 * accessors (include/null_driver.h), the VabStreamObj class
 * (include/vab_stream_obj.h, whose class documentation describes the load
 * sequence and libsnd's set-up), and the SoundCueSet functions
 * (include/sound_cue_set.h).
 *
 * NullDriver is the data source data_source.c selects when it is not reading
 * the CD. GetNullDriverMode, SetNullDriverMode and
 * GetNullDriverUseVSyncCallback answer the queries data_source.c's
 * GetActiveDataSource* functions otherwise forward to CdDriver: they keep
 * the two mode words and report no VSync callback.
 */

/* ".VH" and ".VB". */
extern char sVabHeaderSuffix[];
extern char sVabBodySuffix[];

/* SetNullDriverMode's two words, read back by GetNullDriverMode. */
extern s32 sNullDriverMode;
extern s32 sNullDriverModeArg;

/* libsnd set-up, done once and undone when the last bank closes: SsInit and
 * the size table; the tick mode; SsStart and the master volume. */
extern s32 sVabSizeTableInited;
extern s32 sVabStreamInited;
extern s32 sVabVolumeInited;
extern s32 sOpenVabCount;     /* VabStreamObjs constructed and not yet finalized */
extern s32 sSsTicksPerSecond; /* the SsSetTickMode rate, for callers timing in ticks */
/* The .VH buffer, kept from the header state until LoadVagAttrs takes it
 * back as the object's buffer. */
extern void *sPendingVabBuffer;

s32 NullDriver__Read(void) {
    return 0;
}

void NullDriver__LoadFile(void) {}

void NullDriver__RunRequestQueue(void) {}

void NullDriver__RequestLoadFile(void) {}

void NullDriver__StopService(void) {}

void NullDriver__CancelRequests(void) {}

NullDriverMethods *GetNullDriverMethods(void) {
    return &gNullDriverMethods;
}

s32 GetNullDriverMode(s32 *outMode2) {
    if (outMode2 != NULL) {
        *outMode2 = sNullDriverModeArg;
    }
    return sNullDriverMode;
}

s32 SetNullDriverMode(s32 async, s32 mode2) {
    sNullDriverMode = async;
    sNullDriverModeArg = mode2;
    return 1;
}

s32 GetNullDriverUseVSyncCallback(void) {
    return 0;
}

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

/* PlayTone's packed index: program << VAB_TONE_BITS | tone. */
#define VAB_TONE_BITS 4
#define VAB_TONES_PER_PROG 16

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

s32 InitSoundCueSet(VabStreamObj *sound, SoundCueSet *set, s32 tag, void *owner,
                    SoundCueCallbackFn callback) {
    SoundCueSlot *slot;
    s32 count;
    s32 sentinel;

    if (set->tag != 0) {
        return 0;
    }
    slot = set->slots;
    sentinel = VAB_NO_VOICE;
    count = ARRAY_COUNT(set->slots) - 1;
    set->tag = tag;
    set->owner = owner;
    set->callback = callback;
    do {
        slot->voice = sentinel;
        count--;
        slot++;
    } while (count >= 0);
    set->tick = 0;
    set->attenuationSteps = SOUND_CUE_ATTENUATION_STEPS;
    return 1;
}

void FlushSoundCueSet(VabStreamObj *sound, SoundCueSet *set) {
    s32 i;
    SoundCueSlot *slot;

    slot = set->slots;
    for (i = 0; i < ARRAY_COUNT(set->slots); i++) {
        if (slot->voice >= 0) {
            slot->voice = sound->methods->stopVoice(sound, slot->voice);
        }
        slot++;
    }
    set->tag = 0;
}

void ServiceSoundCueSet(VabStreamObj *sound, SoundCueSet *set) {
    s32 i;
    SoundCueSlot *slot;
    s32 vol;
    s32 endVol;
    s32 toneIndex;

    if (set->tag > 0) {
        /* MATCHING: both loops are do-whiles, entered without a test; a for loop adds
         * a test on entry. This one also steps its count first in the body. */
        i = 0;
        slot = &set->slots[0];
        do {
            i++;
            slot->program = SOUND_CUE_NONE;
            slot->octave = 0;
            slot->vol = SOUND_CUE_DEFAULT_VOL;
            slot->endVol = SOUND_CUE_DEFAULT_END_VOL;
            slot++;
        } while (i < ARRAY_COUNT(set->slots));

        set->attenuation = 0;
        if (set->callback != NULL) {
            set->callback(set->owner, set);
        }

        if (set->attenuation >= 0) {
            slot = &set->slots[0];
            i = 0;
            do {
                if (slot->program >= 0) {
                    if (slot->voice >= 0) {
                        sound->methods->stopVoice(sound, slot->voice);
                    }
                    sound->methods->setPitchOffset(sound, slot->octave);
                    toneIndex = slot->program * VAB_TONES_PER_PROG;
                    vol = slot->vol - (slot->vol / set->attenuationSteps) * set->attenuation;
                    endVol = slot->endVol - (slot->endVol / set->attenuationSteps) * set->attenuation;
                    slot->voice = sound->methods->playTone(sound, toneIndex, vol, endVol);
                } else if (slot->program == SOUND_CUE_STOP && slot->voice >= 0) {
                    sound->methods->stopVoice(sound, slot->voice);
                }
                i++;
                slot++;
            } while (i < ARRAY_COUNT(set->slots));
        }
        set->tick++;
    }
}

/* The file's three method tables, in the order the image keeps them. A
 * (void *) entry is a function whose declared type differs from its slot's:
 * a method inherited from a parent class and declared on the parent's type,
 * or an empty method declared (void). */

/* PlacementGrid (include/placement_grid.h): FileResource's table with the
 * ctor, finalize and onRequestDone, and processBuffer's occupant,
 * ResolveEntry. */
PlacementGridMethods gPlacementGridMethods = {
    /* +0x000 header */ PLACEMENTGRID_CLASS_ID,
    /* +0x004 release */ (void *)FileResource__Release,
    /* +0x008 ctor */ PlacementGrid__PlacementGrid,
    /* +0x00C finalize */ PlacementGrid__Finalize,
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
    /* +0x064 onRequestDone */ PlacementGrid__OnRequestDone,
    /* +0x068 runRequestQueue */ NULL,
    /* +0x06C requestLoadFile */ NULL,
    /* +0x070 stopService */ NULL,
    /* +0x074 cancelRequests */ NULL,
    /* +0x078 processBuffer */ PlacementGrid__ResolveEntry,
};

/* NullDriver (include/null_driver.h): FileResource's slots up to +0x074,
 * the eleven data-source slots all empty. */
NullDriverMethods gNullDriverMethods = {
    /* +0x000 header */ NULLDRIVER_CLASS_ID,
    /* +0x004 release */ (void *)FileResource__Release,
    /* +0x008 ctor */ (void *)NullDriver__NullDriver,
    /* +0x00C finalize */ (void *)NullDriver__Destroy,
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
    /* +0x040 slot40 */ NullDriver__NoOpSlot40,
    /* +0x044 open */ (void *)NullDriver__Open,
    /* +0x048 close */ (void *)NullDriver__Close,
    /* +0x04C seek */ (void *)NullDriver__Seek,
    /* +0x050 slot50 */ NullDriver__NoOpSlot50,
    /* +0x054 read */ (void *)NullDriver__Read,
    /* +0x058 loadFile */ (void *)NullDriver__LoadFile,
    /* +0x05C freeBuffer */ (void *)FileResource__FreeBuffer,
    /* +0x060 slot60 */ NoOp,
    /* +0x064 onRequestDone */ (void *)FileResource__OnRequestDone,
    /* +0x068 runRequestQueue */ NullDriver__RunRequestQueue,
    /* +0x06C requestLoadFile */ (void *)NullDriver__RequestLoadFile,
    /* +0x070 stopService */ (void *)NullDriver__StopService,
    /* +0x074 cancelRequests */ (void *)NullDriver__CancelRequests,
};

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
