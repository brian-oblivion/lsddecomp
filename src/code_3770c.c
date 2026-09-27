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
#include <libcd.h>
#include <libspu.h>
#include "BasicClass.h"
#include "CdStream.h"
#include "CdDriver.h" /* CD_SECTOR_SHIFT */

/* The ctor: a drive speed below this is double speed. */
#define CDSTREAM_DOUBLE_SPEED_BELOW 4
/* CD-ROM sectors read per second at double and at normal speed. */
#define CD_SECTORS_PER_SECOND_2X 300
#define CD_SECTORS_PER_SECOND_1X 150
/* The ctor's multiplier: bytesPerFrame is half a frame's sectors times
 * this. Not CD_SECTOR_SIZE (2048); what the extra 6 bytes count is not
 * established. */
#define CDSTREAM_FRAME_UNIT 2054
/* open: "\\<data directory><name>;1" must fit. */
#define CDSTREAM_PATH_SIZE 32
/* SetupCdStreamAudio: master volume at half scale, CD input at full. */
#define CDSTREAM_MASTER_VOLUME 0x3FFF
#define CDSTREAM_CD_VOLUME 0x7FFF
/* startRead: stream with XA-ADPCM on, at double or normal speed. */
#define CDSTREAM_MODE_2X (CdlModeStream | CdlModeSpeed | CdlModeRT)
#define CDSTREAM_MODE_1X (CdlModeStream | CdlModeRT)
/* getNextFrame: polls when `tries` is negative. */
#define CDSTREAM_NEXT_FRAME_TRIES 8388608

/* Defined in other units: func_800270B8 (code_171e0.c) returns the data
 * directory's name; strcpy and strcat are Sony's libc2; BMemPMgrAlloc is the
 * game's heap allocator. */
void *func_800270B8(void);
extern char *strcpy(char *dest, char *src);
extern char *strcat(char *dest, char *src);
extern void *BMemPMgrAlloc(s32 size);

extern s32 gCdStreamAudioMixSet;
extern char gCdStreamVersionSuffix[]; /* ";1" */

CdStream *New_CdStream(s32 cdSpeed, s32 fps, s32 reserved) {
    CdStream *obj = BMemPMgrAlloc(sizeof(CdStream));

    if (obj != NULL) {
        Get_vtable_CdStream()->ctor(obj, cdSpeed, fps, reserved);
        return obj;
    }
    return NULL;
}

void CdStream__CdStream(CdStream *self, u32 cdSpeed, s32 fps, s32 reserved) {
    Get_vtable_BasicClass()->ctor((BasicClass *)self);
    self->methods = Get_vtable_CdStream();
    self->cdSpeed = cdSpeed;
    self->muted = 0;
    self->bytesPerFrame =
        (((cdSpeed < CDSTREAM_DOUBLE_SPEED_BELOW) ? CD_SECTORS_PER_SECOND_2X : CD_SECTORS_PER_SECOND_1X) /
         fps / 2) *
        CDSTREAM_FRAME_UNIT;
    self->reserved = reserved;
    self->ring = NULL;
    self->onStreamEnd = NULL;
    self->onFrameReady = NULL;
    self->onSeekDone = NULL;
    self->state = CDSTREAM_IDLE;
}

void CdStream__Finalize(CdStream *self) {
    self->methods->close(self);
    Get_vtable_BasicClass()->finalize((BasicClass *)self);
}

void CdStream__SetRing(CdStream *self, u32 *ring, u32 size) {
    if (self->state == CDSTREAM_IDLE) {
        StSetRing(ring, size >> CD_SECTOR_SHIFT);
        self->ring = ring;
    }
}

s32 CdStream__Open(CdStream *self, char *name, s32 tries) {
    char path[CDSTREAM_PATH_SIZE];
    s32 n;

    n = tries;
    if (self->state == CDSTREAM_IDLE) {
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
        while (CdSearchFile((CdlFILE *)&self->file, path) == 0) {
            if (n >= 0 && --tries < 0) {
                return 1;
            }
        }
        self->totalFrames = self->file.size / self->bytesPerFrame;
        gCdStreamAudioMixSet = SetupCdStreamAudio(self);
        gActiveCdStream = self;
        self->methods->seek(self, self->file.pos);
        return 0;
    }
    return 1;
}

s32 SetupCdStreamAudio(CdStream *self) {
    SpuCommonAttr attr;

    attr.mask = SPU_COMMON_MVOLL | SPU_COMMON_MVOLR | SPU_COMMON_CDVOLL | SPU_COMMON_CDVOLR |
                SPU_COMMON_CDMIX;
    attr.mvol.left = CDSTREAM_MASTER_VOLUME;
    attr.mvol.right = CDSTREAM_MASTER_VOLUME;
    attr.cd.volume.left = CDSTREAM_CD_VOLUME;
    attr.cd.volume.right = CDSTREAM_CD_VOLUME;
    attr.cd.mix = SPU_ON;
    SpuSetCommonAttr(&attr);
    return 1;
}

void CdStream__Close(CdStream *self) {
    CdStream *cur;

    if (self->state != CDSTREAM_IDLE) {
        cur = gActiveCdStream;
        if (cur == self) {
            cur->methods->stop(cur);
            cur->state = CDSTREAM_IDLE;
            gActiveCdStream = NULL;
        }
    }
}

void CdStream__Seek(CdStream *self, u8 *pos) {
    if (self->state != CDSTREAM_READING && gActiveCdStream == self) {
        if (self->onSeekDone != NULL) {
            CdSyncCallback(OnCdSeekComplete);
            CdControlF(CdlSeekL, pos);
        } else {
            while (CdSeekL(pos) == 0) {
            }
        }
        self->state = CDSTREAM_SEEKING;
    }
}

void OnCdSeekComplete(u8 status, u8 *result) {
    if (gActiveCdStream != NULL && status == CdlComplete) {
        CdSyncCallback(NULL);
        if (gActiveCdStream->onSeekDone != NULL) {
            gActiveCdStream->onSeekDone(gActiveCdStream->cbArg);
        }
    }
}

void CdStream__StartRead(CdStream *self, u32 startFrame, s32 frameCount) {
    u32 mode;

    if (self->state == CDSTREAM_SEEKING && gActiveCdStream == self) {
        mode = CDSTREAM_MODE_1X;
        if (self->cdSpeed < CDSTREAM_DOUBLE_SPEED_BELOW) {
            mode = CDSTREAM_MODE_2X;
        }
        if (frameCount != 0) {
            self->totalFrames = frameCount;
        }
        self->lastFrame = 0;
        StSetStream(0, startFrame, -1, NULL, NULL);
        self->methods->mute(self);
        while (CdControl(CdlSetloc, self->file.pos, 0) == 0 || CdRead2(mode) == 0) {
        }
        self->methods->demute(self);
        self->state = CDSTREAM_READING;
    }
}

void CdStream__Stop(CdStream *self) {
    if (self->state == CDSTREAM_READING && gActiveCdStream == self) {
        self->methods->mute(self);
        self->methods->clearRing(self);
        self->methods->unsetRing(self);
        while (CdPause() == 0) {
        }
        self->state = CDSTREAM_STOPPED;
    }
}

void CdStream__Restart(CdStream *self) {
    CdStream *cur;

    if (self->state == CDSTREAM_STOPPED) {
        cur = gActiveCdStream;
        if (cur == self) {
            cur->state = CDSTREAM_IDLE;
            cur->methods->seek(cur, cur->file.pos);
        }
    }
}

void CdStream__NoOpSlot5C(CdStream *self) {}

void CdStream__NoOpSlot60(CdStream *self) {}

void CdStream__Mute(CdStream *self) {
    if (self->muted == 0 && gActiveCdStream == self) {
        while (CdMute() == 0) {
        }
        self->muted = 1;
    }
}

void CdStream__Demute(CdStream *self) {
    if (self->muted != 0 && gActiveCdStream == self) {
        while (CdDeMute() == 0) {
        }
        self->muted = 0;
    }
}

s32 CdStream__GetNextFrame(CdStream *self, u32 **addr, u32 *frame, s32 tries) {
    u32 *header;
    u32 n;

    if (tries < 0) {
        tries = CDSTREAM_NEXT_FRAME_TRIES;
    }
    while (StGetNext(addr, &header) != 0) {
        if (--tries < 0) {
            self->methods->freeRing(self, (u32 *)addr);
            return 0;
        }
    }
    n = ((StHEADER *)header)->frameCount;
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
