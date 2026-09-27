/*
 * code_3770c -- CdStream, a BasicClass subclass that drives streamed CD-XA
 * playback over libcd (StSetRing/StFreeRing/CdSync/CdControl) and libspu
 * (SpuSetCommonAttr for the CD audio mix). Carved from psyq_3770c on
 * 2026-09-25 (FINISHING-PLAN revision 18): counted as Psy-Q SDK by segment
 * name, but tools/gameinsdk.py measured it as game code (a method-table entry
 * beside game methods, contiguous with them, no Sony fingerprint).
 * 0x3770C..0x38110 (vram 0x80046F0C..0x80047910), all 18 methods of
 * gCdStreamMethods plus their helpers, matched round 82 (no INCLUDE_ASM
 * left) and named round 82. The class is declared once, in
 * include/CdStream.h (track 4, round 87).
 *
 * One global `gActiveCdStream` is the single active stream: most methods
 * are no-ops unless `self` is that pointer, so only one CdStream streams
 * at a time. The state machine (CdStream.state) runs Open -> Seek(1) ->
 * StartRead(2) -> Stop(4) -> Restart(0, re-seeks) and Close tears it down
 * from any state.
 */
#include "common.h"
#include "BasicClass.h"
#include "CdStream.h"

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

void *func_800270B8(void); /* code_171e0.c: the data directory string */
extern s32 D_8008A94C;
extern char gCdStreamVersionSuffix[]; /* ";1" */
extern void StSetStream(u32 mode, u32 start_frame, u32 end_frame, void (*func1)(), void (*func2)());

extern void *BMemPMgrAlloc(s32 size);

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

CdStream *New_CdStream(s32 speed, s32 fps, s32 arg3) {
    CdStream *obj = BMemPMgrAlloc(0x5C);

    if (obj != NULL) {
        Get_vtable_CdStream()->ctor(obj, speed, fps, arg3);
        return obj;
    }
    return NULL;
}

void CdStream__CdStream(CdStream *self, u32 speed, s32 fps, s32 arg3) {
    Get_vtable_BasicClass()->ctor((BasicClass *)self);
    self->methods = Get_vtable_CdStream();
    self->speed = speed;
    self->muted = 0;
    self->bytesPerFrame = (((speed < 4) ? 300 : 150) / fps / 2) * 2054;
    self->unk3C = arg3;
    self->ring = NULL;
    self->onStreamEnd = NULL;
    self->onFrameReady = NULL;
    self->onSeekDone = NULL;
    self->state = 0;
}

void CdStream__Finalize(CdStream *self) {
    self->methods->close(self);
    Get_vtable_BasicClass()->finalize((BasicClass *)self);
}

void CdStream__SetRing(CdStream *self, u32 *ring, u32 size) {
    if (self->state == 0) {
        StSetRing(ring, size >> 11);
        self->ring = ring;
    }
}

s32 CdStream__Open(CdStream *self, char *name, s32 tries) {
    char path[0x20];
    s32 n;

    n = tries;
    if (self->state == 0) {
        if (self->ring == NULL) {
            return 1;
        }
        if (gActiveCdStream != NULL) {
            return 0;
        }
        path[0] = '\\';
        strcpy(&path[1], func_800270B8());
        strcat(path, name);
        strcat(path, gCdStreamVersionSuffix);
        while (CdSearchFile(&self->file, path) == 0) {
            if (n >= 0 && --tries < 0) {
                return 1;
            }
        }
        self->totalFrames = self->file.size / self->bytesPerFrame;
        D_8008A94C = SetupCdStreamAudio(self);
        gActiveCdStream = self;
        self->methods->seek(self, self->file.pos);
        return 0;
    }
    return 1;
}

s32 SetupCdStreamAudio(CdStream *self) {
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

void CdStream__Close(CdStream *self) {
    CdStream *cur;

    if (self->state != 0) {
        cur = gActiveCdStream;
        if (cur == self) {
            cur->methods->stop(cur);
            cur->state = 0;
            gActiveCdStream = NULL;
        }
    }
}

void CdStream__Seek(CdStream *self, u8 *pos) {
    if (self->state != 2 && gActiveCdStream == self) {
        if (self->onSeekDone != NULL) {
            CdSyncCallback(OnCdSeekComplete);
            CdControlF(0x15, pos);
        } else {
            while (CdControl(0x15, pos, 0) == 0) {
            }
        }
        self->state = 1;
    }
}

void OnCdSeekComplete(u8 status, u8 *result) {
    if (gActiveCdStream != NULL && status == 2) {
        CdSyncCallback(NULL);
        if (gActiveCdStream->onSeekDone != NULL) {
            gActiveCdStream->onSeekDone(gActiveCdStream->cbArg);
        }
    }
}

void CdStream__StartRead(CdStream *self, u32 startFrame, s32 frameCount) {
    u32 mode;

    if (self->state == 1 && gActiveCdStream == self) {
        mode = 0x140;
        if (self->speed < 4) {
            mode = 0x1C0;
        }
        if (frameCount != 0) {
            self->totalFrames = frameCount;
        }
        self->lastFrame = 0;
        StSetStream(0, startFrame, -1, 0, 0);
        self->methods->mute(self);
        while (CdControl(2, self->file.pos, 0) == 0 || CdRead2(mode) == 0) {
        }
        self->methods->demute(self);
        self->state = 2;
    }
}

void CdStream__Stop(CdStream *self) {
    if (self->state == 2 && gActiveCdStream == self) {
        self->methods->mute(self);
        self->methods->clearRing(self);
        self->methods->unsetRing(self);
        while (CdControl(9, 0, 0) == 0) {
        }
        self->state = 4;
    }
}

void CdStream__Restart(CdStream *self) {
    CdStream *cur;

    if (self->state == 4) {
        cur = gActiveCdStream;
        if (cur == self) {
            cur->state = 0;
            cur->methods->seek(cur, cur->file.pos);
        }
    }
}

void CdStream__NoOpSlot5C(CdStream *self) {}

void CdStream__NoOpSlot60(CdStream *self) {}

void CdStream__Mute(CdStream *self) {
    if (self->muted == 0 && gActiveCdStream == self) {
        while (CdControl(0xB, 0, 0) == 0) {
        }
        self->muted = 1;
    }
}

void CdStream__Demute(CdStream *self) {
    if (self->muted != 0 && gActiveCdStream == self) {
        while (CdControl(0xC, 0, 0) == 0) {
        }
        self->muted = 0;
    }
}

s32 CdStream__GetNextFrame(CdStream *self, u32 **addr, u32 *frame, s32 tries) {
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
            CdStream__ReleaseFrame(self, *addr, *frame);
            CdStream__OnStreamEnd(self);
            return -1;
        }
        self->lastFrame = n;
    }
    CdStream__ReleaseFrame(self, *addr, *frame);
    return 1;
}

void CdStream__ReleaseFrame(CdStream *self, u32 *base, u32 frame) {
    if (self->onFrameReady != NULL) {
        self->onFrameReady(self->cbArg);
        self->methods->freeRing(self, base);
    }
}

void CdStream__OnStreamEnd(CdStream *self) {
    if (self->onStreamEnd != NULL) {
        self->onFrameReady(self->cbArg);
        self->methods->close(self);
    }
}

u32 CdStream__FreeRing(CdStream *self, u32 *base) {
    return StFreeRing(base);
}

void CdStream__UnsetRing(CdStream *self) {
    StUnSetRing();
}

void CdStream__ClearRing(CdStream *self) {
    StClearRing();
}

int CdStream__Sync(CdStream *self, int mode) {
    return CdSync(mode, self->cdResult);
}

void CdStream__NoOpSlot7C(CdStream *self) {}

CdStreamMethods *Get_vtable_CdStream(void) {
    return &gCdStreamMethods;
}
