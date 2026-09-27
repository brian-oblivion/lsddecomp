/*
 * code_179d8_e -- the SPU/VAB sound-streaming backend (`gActiveDataSource ==
 * 0x23`, confirmed round 52 against alpha's own naming of that global in
 * code_171e0.c) and its small mood/context-tagged sound-cue queue.
 *
 * functions 120..148 of the original 274-function code_179d8 monolith,
 * 0x1CC08..0x1D508 (vram 0x8002C408..0x8002CD08).  Carved round 17
 * (2026-09-04) out of what the yaml called `code_179d8_mid_b`; the remainder
 * behind it is now `code_179d8_mid_c`.
 *
 * Owns NO switch jump table -- zero `jtbl_` references anywhere in the slice
 * -- so no rodata sub-slot is attached to this unit.
 *
 * CLASS FRAMEWORK (established round 17, sharpened round 43 and round 52).
 * `tools/classtable.py --scan` hits two real class tables in this unit's own
 * globals: `gVabDriverMethods` (29 slots, header word 0x00000023) and
 * `gVabStreamObjMethods` (39 slots, header word 0x00000A03).  They share the
 * same BasicClass tail (slots +0x10..+0x38) -- two classes off the same
 * base, not one subclassing the other.
 *
 * `gVabDriverMethods`'s header word (0x23) is not a coincidence: it is
 * exactly the value `code_171e0.c`'s `gActiveDataSource` compares against to
 * select this backend (the other value, 0x13, selects the CD-ROM read
 * driver, `gVabDriverMethods`'s sibling `gCdDriverMethods` in `code_179d8_q.c`).
 * `code_171e0.c`'s own `GetActiveDataSourceMethods` returns `GetVabDriverMethods()` (this
 * unit) exactly when `gActiveDataSource == 0x23`, and `VabStreamObj`'s own
 * constructor/close (below) chain their base-class calls through that
 * accessor's return.  ROUND 87 CORRECTION (track 4): that does not make
 * gVabDriverMethods VabStreamObj's base.  Both are FileResource subclasses
 * (ids 0x23 and 0xA03, parent 0x3); VabStreamObj chains to whichever driver
 * is active, as every data source does.  VabDriver is now declared once, in
 * `include/VabDriver.h`: its ctor/dtor (`code_179d8_d.c`) and the eleven
 * interface slots it overrides, six of them defined in this unit
 * (`VabDriver__Read`/`LoadFile`/`RunRequestQueue`/`RequestLoadFile`/
 * `StopService`/`CancelRequests`), are all empty no-ops.
 *
 * `gVabStreamObjMethods` is the real work. It is the class VabStreamObj,
 * declared once in `include/VabStreamObj.h` since round 87 (track 4). That
 * header's banner describes the load sequence and the slots. Two older
 * readings here were wrong. `VabStreamObj__AdvanceLoadState` (was `Update`)
 * is not a per-frame poll: it is the setFlag slot, and the CD driver calls
 * it when a request completes. The "null in retail" slots +0x58 and +0x6C
 * (loadFile, requestLoadFile) are filled from the active driver by
 * SetActiveDataSource.
 *
 * `InitSoundCueSet`/`FlushSoundCueSet` are unrelated free functions (NOT
 * `gVabStreamObjMethods` vtable slots -- checked, absent from its slot list)
 * operating on the 3-voice `SoundCueSet` (include/SoundCueSet.h) that `Entity`/`DreamSys`/
 * `class_3bb8c_n` embed and tag with their own context (round 52: Entity.c
 * passes `this->moodIndex + 1` as the tag).  `FlushSoundCueSet` dispatches
 * each populated slot's stored index through the ACTIVE stream object's own
 * `VabStreamObj__StopVoice` slot, so the queue is a backend-agnostic front
 * door onto whichever data source `gActiveDataSource` currently selects, not
 * something owned by `VabStreamObj` itself.
 *
 * `GetVabUseVSyncCallback` and `GetVabDriverMode`/`SetVabDriverMode` are this
 * backend's own implementations of the same generic driver-mode interface
 * `code_171e0.c` dispatches on `gActiveDataSource` -- confirmed round 52 by
 * that unit's own substitution (`func_80026FAC`/`func_80026F34`/
 * `func_80026FE8` call `GetCdDriverMode`/`SetCdDriverMode`/`GetCdUseVSyncCallback`
 * when `gActiveDataSource == 0x13`, else these).  `GetVabUseVSyncCallback` itself
 * stays unnamed: its only paired counterpart, `GetCdUseVSyncCallback`, is still
 * unnamed too, so there's nothing to name it AS a stand-in for.
 */
#include "common.h"
#include <libsnd.h>
#include "VabDriver.h"
#include "VabStreamObj.h"
#include "SoundCueSet.h"

/* Cross-unit calls into the still-uncarved code_179d8_tail monolith --
 * declared LOCAL to this unit, per-call-site typed, since none of them have
 * an established prototype anywhere yet. */
extern void *BMemPMgrAlloc(s32 size);

s32 VabDriver__Read(void) {
    return 0;
}

void VabDriver__LoadFile(void) {}

void VabDriver__RunRequestQueue(void) {}

void VabDriver__RequestLoadFile(void) {}

void VabDriver__StopService(void) {}

void VabDriver__CancelRequests(void) {}

VabDriverMethods *GetVabDriverMethods(void) {
    return &gVabDriverMethods;
}

extern s32 gVabDriverMode;
extern s32 gVabDriverModeArg;

s32 GetVabDriverMode(s32 *arg0) {
    if (arg0 != NULL) {
        *arg0 = gVabDriverModeArg;
    }
    return gVabDriverMode;
}

s32 SetVabDriverMode(s32 a, s32 b) {
    gVabDriverMode = a;
    gVabDriverModeArg = b;
    return 1;
}

s32 GetVabUseVSyncCallback(void) {
    return 0;
}

VabStreamObj *New_VabStreamObj(char *path) {
    VabStreamObj *self;

    self = BMemPMgrAlloc(0x64);
    if (self != NULL) {
        GetVabStreamObjMethods()->ctor(self, path);
        return self;
    }
    return NULL;
}

/* GetActiveDataSourceMethods (code_171e0.c) returns gVabDriverMethods when
 * gActiveDataSource is DATASOURCE_SPU and gCdDriverMethods otherwise; both
 * are FILERESOURCE_SLOTS tables, so it is typed FileResourceMethods * as in
 * every other unit that calls it. VabStreamObj's ctor and finalize chain
 * through its +0x008 ctor and +0x00C finalize (round 88; this unit's own
 * DriverBaseMethods view, slot08/slot0C, until then). */
extern FileResourceMethods *GetActiveDataSourceMethods(void);


/* Uncarved code_179d8_tail helpers this cluster calls. */
extern char *GetSsSizeTableBuf(void);
extern s32 IsWBgmActive(void);
extern void *BMemPMgrFree(void *ptr);
extern char *BuildFileName(char *dest, char *arg1, char *arg2, char *arg3);
extern s32 strlen(char *s);
extern char *strcpy(char *dest, char *src);

/* ".VH"/".VB" -- already emitted by splat in .sdata, referenced not
 * retyped (a literal here would duplicate the bytes and shift the image). */
extern const char gVabHeaderSuffix[];
extern const char gVabBodySuffix[];

extern s32 gVabSizeTableInited;
extern s32 gVabStreamInited;
extern s32 gVabVolumeInited;
extern s32 gOpenVabCount;
extern s32 gSsTicksPerSecond;
extern void *gPendingVabBuffer;

void VabStreamObj__VabStreamObj(VabStreamObj *self, char *path) {
    void *buf;
    char vhPath[0x20];

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
    if (gVabSizeTableInited == 0) {
        SsInit();
        gVabSizeTableInited = 1;
        SsSetTableSize(GetSsSizeTableBuf(), 2, 1);
    }
    if (gVabStreamInited == 0) {
        gSsTicksPerSecond = 0x3C;
        SsSetTickMode(1);
        gVabStreamInited = 1;
    }
    gOpenVabCount++;
    if (path != NULL) {
        buf = BMemPMgrAlloc(strlen(path) + 1);
        if (buf != NULL) {
            self->baseFilename = buf;
            strcpy(buf, path);
            BuildFileName(vhPath, buf, NULL, gVabHeaderSuffix);
            self->loadState = 1;
            self->methods->requestLoadFile(self, vhPath);
        }
    }
}

void VabStreamObj__Finalize(VabStreamObj *self) {
    SsVabClose(self->vabId);
    if (--gOpenVabCount < 0) {
        gOpenVabCount = 0;
    }
    if (gOpenVabCount == 0 && IsWBgmActive() == 0) {
        gVabSizeTableInited = 0;
        gVabVolumeInited = 0;
        gVabStreamInited = 0;
        SsEnd();
        SsQuit();
    }
    BMemPMgrFree(self->vagAttrPool);
    BMemPMgrFree(self->progVagTable);
    BMemPMgrFree(self->baseFilename);
    GetActiveDataSourceMethods()->finalize((FileResource *)self);
}

void VabStreamObj__AdvanceLoadState(VabStreamObj *self) {
    char path[0x20];

    switch (self->loadState) {
        case 0:
            break;
        case 1:
            if (self->flags & 0x200) {
                self->vabId = SsVabOpenHead(self->buffer, -1);
                BuildFileName(path, self->baseFilename, NULL, gVabBodySuffix);
                gPendingVabBuffer = self->buffer;
                self->loadState = 6;
                self->buffer = NULL;
                self->methods->loadFile(self, path);
                if (self->baseFilename != NULL) {
                    BMemPMgrFree(self->baseFilename);
                    self->baseFilename = NULL;
                }
            }
            break;
        case 6:
            if (self->flags & 0x200) {
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
    self->buffer = gPendingVabBuffer;
    result = SsUtGetVabHdr(self->vabId, (VabHdr *)&self->vabHdr);
    if (result == -1) {
        return;
    }
    self->vagAttrPool = BMemPMgrAlloc(self->vabHdr.vs << 5);
    if (self->vagAttrPool == NULL) {
        return;
    }
    self->progVagTable = BMemPMgrAlloc(self->vabHdr.ts << 2);
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
    if (gVabVolumeInited == 0) {
        SsStart();
        SsSetMVol(0x78, 0x78);
        gVabVolumeInited = 1;
    }
}

/* Resolve a packed program/tone index, look up its VagAtr and key it on.
 * `prog` is loaded BEFORE `lo` is computed on purpose: GCC 2.6.3's combine
 * folds `index - (index >> 4) * 16` into `index & 0xF` whenever `hi`'s
 * FIRST use is the multiply (flow.c links a set only to its next use), and
 * retail kept the unfolded sll+subu (docs/match-reports/VabStreamObj__PlayTone.md). */
s32 VabStreamObj__PlayTone(VabStreamObj *self, s32 index, s32 vol, s32 endVol) {
    s32 hi;
    s32 lo;
    VabStreamVagAtr *entry;
    VabStreamVagAtr *prog;
    s16 result;

    if (index >= 0) {
        hi = index >> 4;
        prog = self->progVagTable[hi];
        lo = index - hi * 16;
        entry = &prog[lo];
        result = SsUtKeyOn(self->vabId, (s16)hi, (s16)lo, (s16)(entry->center + self->pitchOffset),
                           entry->shift, (s16)vol, (s16)vol);
        if (result >= 0) {
            SsUtAutoVol(result, (s16)vol, (s16)endVol, 2);
            return result;
        }
    }
    return -1;
}

/* The PS1 SPU's own hardware voice count -- the boundary VabStreamObj__StopVoice
 * checks `index` against. */
#define SPU_VOICE_COUNT 0x18

s32 VabStreamObj__StopVoice(VabStreamObj *self, s32 voice) {
    if (voice < SPU_VOICE_COUNT) {
        SsUtKeyOffV(voice);
    } else {
        SsUtAllKeyOff(0);
    }
    return -1;
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
        /* MATCHING: retail returns what SsSetMute leaves in $v0 (SpuSetMute's
         * result); <libsnd.h> declares it void, so the call is cast. */
        result = ((s32 (*)(char))SsSetMute)(0);
        self->muted = 0;
    }
    return result;
}

void VabStreamObj__NoOpSlot90(void) {}

void VabStreamObj__NoOpSlot94(void) {}

void VabStreamObj__NoOpSlot98(void) {}

void VabStreamObj__SetPitchOffset(VabStreamObj *self, s32 octave) {
    self->pitchOffset = octave * 12 - 0x18;
}

VabStreamObjMethods *GetVabStreamObjMethods(void) {
    return &gVabStreamObjMethods;
}

extern s32 gOpenVabCount;

s32 GetOpenVabCount(void) {
    return gOpenVabCount;
}

extern s32 gSsTicksPerSecond;

s32 func_8002CC28(void) {
    return gSsTicksPerSecond;
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
    sentinel = -1;
    count = 2;
    set->tag = tag;
    set->owner = owner;
    set->callback = callback;
    do {
        slot->voice = sentinel;
        count--;
        slot++;
    } while (count >= 0);
    set->tick = 0;
    set->attenuationSteps = 10;
    return 1;
}

void FlushSoundCueSet(VabStreamObj *self, SoundCueSet *set) {
    s32 i;
    SoundCueSlot *slot;

    slot = set->slots;
    for (i = 0; i < 3; i++) {
        if (slot->voice >= 0) {
            slot->voice = self->methods->stopVoice(self, slot->voice);
        }
        slot++;
    }
    set->tag = 0;
}
