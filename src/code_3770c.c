/*
 * code_3770c -- CdStreamObj, a BasicClass subclass that drives streamed CD-XA
 * playback over libcd (StSetRing/StFreeRing/CdSync/CdControl) and libspu
 * (SpuSetCommonAttr for the CD audio mix). Carved from psyq_3770c on
 * 2026-09-25 (FINISHING-PLAN revision 18): counted as Psy-Q SDK by segment
 * name, but tools/gameinsdk.py measured it as game code (a method-table entry
 * beside game methods, contiguous with them, no Sony fingerprint).
 * 0x3770C..0x38110 (vram 0x80046F0C..0x80047910), all 18 methods of
 * gCdStreamObjMethods plus their helpers, matched round 82 (no INCLUDE_ASM
 * left) and named round 82 (this pass).
 *
 * One global `gActiveCdStreamObj` is the single active stream: most methods
 * are no-ops unless `self` is that pointer, so only one CdStreamObj streams
 * at a time. The state machine (CdStreamObj.state) runs Open -> Seek(1) ->
 * StartRead(2) -> Stop(4) -> Restart(0, re-seeks) and Close tears it down
 * from any state.
 */
#include "common.h"
#include "BasicClass.h"

/* Local view of the gCdStreamObjMethods class. Only the fields this unit's
 * matched methods touch are named; no other unit references this struct or
 * either of its two file-local globals. */
typedef struct CdStreamObj CdStreamObj;
typedef struct CdStreamObjMethods CdStreamObjMethods;

struct CdStreamObjMethods {
    BASICCLASS_SLOTS(CdStreamObj, (CdStreamObj *self, s32 arg1, s32 arg2, s32 arg3));
    /* +0x040 */ void (*setRing)(CdStreamObj *self, u32 *ring, u32 size); /* CdStreamObj__SetRing */
    /* +0x044 */ void *open;                                            /* CdStreamObj__Open */
    /* +0x048 */ void (*close)(CdStreamObj *self);                       /* CdStreamObj__Close */
    /* +0x04C */ void (*seek)(CdStreamObj *self, u8 *seekLoc);              /* CdStreamObj__Seek */
    /* +0x050 */ void *startRead;                                            /* CdStreamObj__StartRead */
    /* +0x054 */ void (*stop)(CdStreamObj *self);                       /* CdStreamObj__Stop */
    /* +0x058 */ void (*restart)(CdStreamObj *self);                       /* CdStreamObj__Restart */
    /* +0x05C */ void (*slot5C)(CdStreamObj *self);                       /* CdStreamObj__func_800475C8, empty */
    /* +0x060 */ void (*slot60)(CdStreamObj *self);                       /* CdStreamObj__func_800475D0, empty */
    /* +0x064 */ void (*mute)(CdStreamObj *self);                         /* CdStreamObj__Mute */
    /* +0x068 */ void (*demute)(CdStreamObj *self);                       /* CdStreamObj__Demute */
    /* +0x06C */ void *getNextFrame;                                            /* CdStreamObj__GetNextFrame */
    /* +0x070 */ u32 (*freeRing)(CdStreamObj *self, u32 *base);           /* CdStreamObj__FreeRing */
    /* +0x074 */ void (*unsetRing)(CdStreamObj *self);                    /* CdStreamObj__UnsetRing */
    /* +0x078 */ void (*clearRing)(CdStreamObj *self);                    /* CdStreamObj__ClearRing */
    /* +0x07C */ void (*slot7C)(CdStreamObj *self);                       /* CdStreamObj__func_800478F8, empty */
};

struct CdStreamObj {
    BASICCLASS_FIELDS(CdStreamObjMethods);
    /* +0x00C */ u8 seekLoc[0x24 - 0x0C];  /* CdlLOC-shaped buffer passed to seek/CdControl */
    /* +0x024 */ u8 cdResult[8];      /* CdSync result buffer (CdStreamObj__Sync) */
    /* +0x02C */ s32 state;           /* 0 idle, 1 seeking, 2 reading, 4 stopped */
    /* +0x030 */ s32 muted;           /* CdStreamObj__Mute / CdStreamObj__Demute */
    /* +0x034 */ s32 speed;           /* < 4 selects read mode 0x1C0, else 0x140; also the ctor's rate divisor select */
    /* +0x038 */ s32 bytesPerFrame;   /* ctor: (speed ? 300 : 150) / arg2 / 2 * 2054; divides file size into totalFrames */
    /* +0x03C */ s32 unk3C;           /* ctor: = arg3; no further use in this unit */
    /* +0x040 */ s32 totalFrames;     /* file size / bytesPerFrame (CdStreamObj__Open); overridable by StartRead's arg2 */
    /* +0x044 */ void *cbArg;         /* argument passed to onFrameReady/onStreamEnd/onSeekDone */
    /* +0x048 */ void (*onFrameReady)(void *arg);  /* called when a frame's ring buffer is ready to release */
    /* +0x04C */ void (*onStreamEnd)(void *arg);   /* tested (never itself called) to gate the stream-end path */
    /* +0x050 */ u32 *ring;           /* StSetRing's ring_addr (CdStreamObj__SetRing) */
    /* +0x054 */ void (*onSeekDone)(void *arg);    /* called by OnCdSeekComplete when the async seek finishes */
    /* +0x058 */ s32 lastFrame;       /* last frame index seen by GetNextFrame */
};

extern CdStreamObjMethods gCdStreamObjMethods;  /* the class's method table */
CdStreamObjMethods *Get_vtable_CdStreamObj(void);
void OnCdSeekComplete(u8 status, u8 *result);
void CdStreamObj__ReleaseFrame(CdStreamObj *self, u32 *base, u32 frame);
void CdStreamObj__OnStreamEnd(CdStreamObj *self);
s32 SetupCdStreamAudio(CdStreamObj *self);

/* LIBCD.H */
extern void StSetRing(u32 *ring_addr, u32 ring_size);
extern void StClearRing(void);
extern void StUnSetRing(void);
extern u32 StFreeRing(u32 *base);
extern int CdSync(int mode, u8 *result);
extern void *CdSyncCallback(void (*func)(u8 status, u8 *result));
extern int CdControl(u8 com, u8 *param, u8 *result);
extern int CdControlF(u8 com, u8 *param);
extern int CdRead2(u32 mode);
extern int StGetNext(u32 **addr, u32 **header);
extern void *CdSearchFile(void *fp, char *name);

/* libc2 (Sony's, linked) */
extern char *strcpy(char *dest, char *src);
extern char *strcat(char *dest, char *src);

void *func_800270B8(void);   /* code_171e0.c: the data directory string */
extern s32 D_8008A94C;
extern char D_8008A954[];    /* ";1" */
extern void StSetStream(u32 mode, u32 start_frame, u32 end_frame, void (*func1)(), void (*func2)());

extern void *BMemPMgrAlloc(s32 size);
extern CdStreamObj *gActiveCdStreamObj;  /* the active stream object */

/* LIBSPU.H */
typedef struct {
    s16 left;
    s16 right;
} SpuVolume;

typedef struct {
    SpuVolume volume;
    s32 reverb;
    s32 mix;
} SpuExtAttr;

typedef struct {
    u32 mask;
    SpuVolume mvol;
    SpuVolume mvolmode;
    SpuVolume mvolx;
    SpuExtAttr cd;
    SpuExtAttr ext;
} SpuCommonAttr;

extern void SpuSetCommonAttr(SpuCommonAttr *attr);

CdStreamObj *New_CdStreamObj(s32 arg1, s32 arg2, s32 arg3) {
    CdStreamObj *obj = BMemPMgrAlloc(0x5C);

    if (obj != NULL) {
        Get_vtable_CdStreamObj()->ctor(obj, arg1, arg2, arg3);
        return obj;
    }
    return NULL;
}
void CdStreamObj__CdStreamObj(CdStreamObj *self, u32 arg1, s32 arg2, s32 arg3) {
    Get_vtable_BasicClass()->ctor((BasicClass *)self);
    self->methods = Get_vtable_CdStreamObj();
    self->speed = arg1;
    self->muted = 0;
    self->bytesPerFrame = (((arg1 < 4) ? 300 : 150) / arg2 / 2) * 2054;
    self->unk3C = arg3;
    self->ring = NULL;
    self->onStreamEnd = NULL;
    self->onFrameReady = NULL;
    self->onSeekDone = NULL;
    self->state = 0;
}
void CdStreamObj__Finalize(CdStreamObj *self) {
    self->methods->close(self);
    Get_vtable_BasicClass()->finalize((BasicClass *)self);
}
void CdStreamObj__SetRing(CdStreamObj *self, u32 *ring, u32 size) {
    if (self->state == 0) {
        StSetRing(ring, size >> 11);
        self->ring = ring;
    }
}
s32 CdStreamObj__Open(CdStreamObj *self, char *name, s32 tries) {
    char path[0x20];
    s32 n;

    n = tries;
    if (self->state == 0) {
        if (self->ring == NULL) {
            return 1;
        }
        if (gActiveCdStreamObj != NULL) {
            return 0;
        }
        path[0] = '\\';
        strcpy(&path[1], func_800270B8());
        strcat(path, name);
        strcat(path, D_8008A954);
        while (CdSearchFile(self->seekLoc, path) == 0) {
            if (n >= 0 && --tries < 0) {
                return 1;
            }
        }
        self->totalFrames = *(u32 *)&self->seekLoc[4] / self->bytesPerFrame;
        D_8008A94C = SetupCdStreamAudio(self);
        gActiveCdStreamObj = self;
        self->methods->seek(self, self->seekLoc);
        return 0;
    }
    return 1;
}
s32 SetupCdStreamAudio(CdStreamObj *self) {
    SpuCommonAttr attr;

    attr.mask = 0x2C3;
    attr.mvol.left = 0x3FFF;
    attr.mvol.right = 0x3FFF;
    attr.cd.volume.left = 0x7FFF;
    attr.cd.volume.right = 0x7FFF;
    attr.cd.mix = 1;
    SpuSetCommonAttr(&attr);
    return 1;
}
void CdStreamObj__Close(CdStreamObj *self) {
    CdStreamObj *cur;

    if (self->state != 0) {
        cur = gActiveCdStreamObj;
        if (cur == self) {
            cur->methods->stop(cur);
            cur->state = 0;
            gActiveCdStreamObj = NULL;
        }
    }
}
void CdStreamObj__Seek(CdStreamObj *self, u8 *seekLoc) {
    if (self->state != 2 && gActiveCdStreamObj == self) {
        if (self->onSeekDone != NULL) {
            CdSyncCallback(OnCdSeekComplete);
            CdControlF(0x15, seekLoc);
        } else {
            while (CdControl(0x15, seekLoc, 0) == 0) {
            }
        }
        self->state = 1;
    }
}
void OnCdSeekComplete(u8 status, u8 *result) {
    if (gActiveCdStreamObj != NULL && status == 2) {
        CdSyncCallback(NULL);
        if (gActiveCdStreamObj->onSeekDone != NULL) {
            gActiveCdStreamObj->onSeekDone(gActiveCdStreamObj->cbArg);
        }
    }
}
void CdStreamObj__StartRead(CdStreamObj *self, u32 startFrame, s32 arg2) {
    u32 mode;

    if (self->state == 1 && gActiveCdStreamObj == self) {
        mode = 0x140;
        if (self->speed < 4) {
            mode = 0x1C0;
        }
        if (arg2 != 0) {
            self->totalFrames = arg2;
        }
        self->lastFrame = 0;
        StSetStream(0, startFrame, -1, 0, 0);
        self->methods->mute(self);
        while (CdControl(2, self->seekLoc, 0) == 0 || CdRead2(mode) == 0) {
        }
        self->methods->demute(self);
        self->state = 2;
    }
}
void CdStreamObj__Stop(CdStreamObj *self) {
    if (self->state == 2 && gActiveCdStreamObj == self) {
        self->methods->mute(self);
        self->methods->clearRing(self);
        self->methods->unsetRing(self);
        while (CdControl(9, 0, 0) == 0) {
        }
        self->state = 4;
    }
}
void CdStreamObj__Restart(CdStreamObj *self) {
    CdStreamObj *cur;

    if (self->state == 4) {
        cur = gActiveCdStreamObj;
        if (cur == self) {
            cur->state = 0;
            cur->methods->seek(cur, cur->seekLoc);
        }
    }
}
void CdStreamObj__func_800475C8(CdStreamObj *self) {
}
void CdStreamObj__func_800475D0(CdStreamObj *self) {
}
void CdStreamObj__Mute(CdStreamObj *self) {
    if (self->muted == 0 && gActiveCdStreamObj == self) {
        while (CdControl(0xB, 0, 0) == 0) {
        }
        self->muted = 1;
    }
}
void CdStreamObj__Demute(CdStreamObj *self) {
    if (self->muted != 0 && gActiveCdStreamObj == self) {
        while (CdControl(0xC, 0, 0) == 0) {
        }
        self->muted = 0;
    }
}
s32 CdStreamObj__GetNextFrame(CdStreamObj *self, u32 **addr, u32 *frame, s32 tries) {
    u32 *header;
    u32 n;

    if (tries < 0) {
        tries = 0x800000;
    }
    while (StGetNext(addr, &header) != 0) {
        if (--tries < 0) {
            self->methods->freeRing(self, (u32 *)addr);
            return 0;
        }
    }
    n = header[2];
    *frame = n;
    if (self->totalFrames > 0) {
        if (n >= self->totalFrames || n < self->lastFrame) {
            if (n < self->lastFrame) {
                *frame = 0;
            }
            CdStreamObj__ReleaseFrame(self, *addr, *frame);
            CdStreamObj__OnStreamEnd(self);
            return -1;
        }
        self->lastFrame = n;
    }
    CdStreamObj__ReleaseFrame(self, *addr, *frame);
    return 1;
}
void CdStreamObj__ReleaseFrame(CdStreamObj *self, u32 *base, u32 frame) {
    if (self->onFrameReady != NULL) {
        self->onFrameReady(self->cbArg);
        self->methods->freeRing(self, base);
    }
}
void CdStreamObj__OnStreamEnd(CdStreamObj *self) {
    if (self->onStreamEnd != NULL) {
        self->onFrameReady(self->cbArg);
        self->methods->close(self);
    }
}
u32 CdStreamObj__FreeRing(CdStreamObj *self, u32 *base) {
    return StFreeRing(base);
}
void CdStreamObj__UnsetRing(CdStreamObj *self) {
    StUnSetRing();
}
void CdStreamObj__ClearRing(CdStreamObj *self) {
    StClearRing();
}
int CdStreamObj__Sync(CdStreamObj *self, int mode) {
    return CdSync(mode, self->cdResult);
}
void CdStreamObj__func_800478F8(CdStreamObj *self) {
}
CdStreamObjMethods *Get_vtable_CdStreamObj(void) {
    return &gCdStreamObjMethods;
}
