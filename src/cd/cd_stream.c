/*
 * CdStream's methods: one streamed CD file (an FMV's sectors) read through
 * libcd's streaming library (StSetRing, StSetStream, StGetNext, StFreeRing)
 * with the CD audio routed into the SPU mix. The class, its fields and its
 * slots are declared in include/cd_stream.h; MoviePlayer is the one holder.
 *
 * One global, sActiveCdStream, is the stream that owns the drive: open sets
 * it, close clears it, and seek, startRead, stop, restart, mute and demute do
 * nothing unless `self` is that stream, so only one CdStream streams at a
 * time. `state` (enum CdStreamState) runs open -> SEEKING -> startRead ->
 * READING -> stop -> STOPPED -> restart -> IDLE and seeks again; close tears
 * the stream down from any state.
 */
#include "common.h"
#include <libcd.h>
#include <libspu.h>
#include "basic_class.h"
#include "cd_stream.h"
#include "cd_driver.h" /* CD_SECTOR_SHIFT */
#include "bmem_pmgr.h"
#include <strings.h>
#include "data_source.h"

extern CdStream *sActiveCdStream; /* the stream that owns the drive, or NULL */

/* The ctor: a drive speed below this is double speed. */
#define CDSTREAM_DOUBLE_SPEED_BELOW 4
/* CD-ROM sectors read per second at double and at normal speed. */
#define CD_SECTORS_PER_SECOND_2X 300
#define CD_SECTORS_PER_SECOND_1X 150
/* The ctor's multiplier: bytesPerFrame is half a frame's sectors times
 * this. Not CD_SECTOR_SIZE (2048); what the extra 6 bytes count is not
 * established. */
#define CDSTREAM_FRAME_UNIT 2054
/* open: "\<data directory><name>;1" must fit. */
#define CDSTREAM_PATH_SIZE 32
/* SetupCdStreamAudio: master volume and CD input volume, each the top of
 * its libspu range (master -0x4000..0x3FFF, CD -0x8000..0x7FFF). */
#define CDSTREAM_MASTER_VOLUME 0x3FFF
#define CDSTREAM_CD_VOLUME 0x7FFF
/* startRead: stream with XA-ADPCM on, at double or normal speed. */
#define CDSTREAM_MODE_2X (CdlModeStream | CdlModeSpeed | CdlModeRT)
#define CDSTREAM_MODE_1X (CdlModeStream | CdlModeRT)
/* getNextFrame's poll count when `tries` is negative. */
#define CDSTREAM_NEXT_FRAME_TRIES 8388608

extern s32 sCdStreamAudioMixSet;
extern char sCdStreamVersionSuffix[]; /* ";1" */

/* MATCHING: the ctor call and return sit inside `if (obj != NULL)`; an early
 * `return NULL` adds a jump. */
CdStream *New_CdStream(s32 cdSpeed, s32 fps, s32 reserved) {
    CdStream *obj = BMemPMgrAlloc(sizeof(CdStream));

    if (obj != NULL) {
        GetCdStreamMethods()->ctor(obj, cdSpeed, fps, reserved);
        return obj;
    }
    return NULL;
}

/* MATCHING: the sectors-a-second choice stays an inline conditional; a local
 * lets cc1 hoist its load and reorder the stores. */
void CdStream__CdStream(CdStream *self, u32 cdSpeed, s32 fps, s32 reserved) {
    GetBasicClassMethods()->ctor((BasicClass *)self);
    self->methods = GetCdStreamMethods();
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
    GetBasicClassMethods()->finalize((BasicClass *)self);
}

void CdStream__SetRing(CdStream *self, u32 *ring, u32 size) {
    if (self->state == CDSTREAM_IDLE) {
        StSetRing(ring, size >> CD_SECTOR_SHIFT);
        self->ring = ring;
    }
}

/* Only while idle and with a ring set: looks up "\<data directory><name>;1",
 * retrying up to `tries` more times (forever if negative), counts the file's frames,
 * sets up the CD audio mix and seeks to the file. Returns 0 once seeking, or
 * when another stream owns the drive; 1 otherwise.
 * MATCHING: the whole body nests in `if (idle) { ...; return 0; } return 1;`;
 * flat early returns merge or reorder the two `return 1` paths. */
s32 CdStream__Open(CdStream *self, char *name, s32 tries) {
    char path[CDSTREAM_PATH_SIZE];
    s32 n;

    n = tries; /* the sign test; `tries` is what counts down */
    if (self->state == CDSTREAM_IDLE) {
        if (self->ring == NULL) {
            return 1;
        }
        if (sActiveCdStream != NULL) {
            return 0;
        }
        path[0] = '\\';
        strcpy(&path[1], GetDataDirectory());
        strcat(path, name);
        strcat(path, sCdStreamVersionSuffix);
        while (CdSearchFile(&self->file, path) == 0) {
            if (n >= 0 && --tries < 0) {
                return 1;
            }
        }
        self->totalFrames = self->file.size / self->bytesPerFrame;
        sCdStreamAudioMixSet = SetupCdStreamAudio(self);
        sActiveCdStream = self;
        self->methods->seek(self, &self->file.pos);
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

/* MATCHING: every access goes through `cur`, the loaded global, not `self`. */
void CdStream__Close(CdStream *self) {
    CdStream *cur;

    if (self->state != CDSTREAM_IDLE) {
        cur = sActiveCdStream;
        if (cur == self) {
            cur->methods->stop(cur);
            cur->state = CDSTREAM_IDLE;
            sActiveCdStream = NULL;
        }
    }
}

/* With onSeekDone set the seek is asynchronous and OnCdSeekComplete reports
 * it; otherwise it blocks until the drive takes the command. */
void CdStream__Seek(CdStream *self, CdlLOC *pos) {
    if (self->state != CDSTREAM_READING && sActiveCdStream == self) {
        if (self->onSeekDone != NULL) {
            CdSyncCallback(OnCdSeekComplete);
            CdControlF(CdlSeekL, (u_char *)pos);
        } else {
            while (CdSeekL(pos) == 0) {
            }
        }
        self->state = CDSTREAM_SEEKING;
    }
}

void OnCdSeekComplete(u8 status, u8 *result) {
    if (sActiveCdStream != NULL && status == CdlComplete) {
        CdSyncCallback(NULL);
        if (sActiveCdStream->onSeekDone != NULL) {
            sActiveCdStream->onSeekDone(sActiveCdStream->cbArg);
        }
    }
}

void CdStream__StartRead(CdStream *self, u32 startFrame, s32 frameCount) {
    u32 mode;

    if (self->state == CDSTREAM_SEEKING && sActiveCdStream == self) {
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
        while (CdControl(CdlSetloc, (u_char *)&self->file.pos, 0) == 0 || CdRead2(mode) == 0) {
        }
        self->methods->demute(self);
        self->state = CDSTREAM_READING;
    }
}

void CdStream__Stop(CdStream *self) {
    if (self->state == CDSTREAM_READING && sActiveCdStream == self) {
        self->methods->mute(self);
        self->methods->clearRing(self);
        self->methods->unsetRing(self);
        while (CdPause() == 0) {
        }
        self->state = CDSTREAM_STOPPED;
    }
}

/* MATCHING: every access goes through `cur`, the loaded global, not `self`. */
void CdStream__Restart(CdStream *self) {
    CdStream *cur;

    if (self->state == CDSTREAM_STOPPED) {
        cur = sActiveCdStream;
        if (cur == self) {
            cur->state = CDSTREAM_IDLE;
            cur->methods->seek(cur, &cur->file.pos);
        }
    }
}

void CdStream__NoOpSlot5C(CdStream *self) {}

void CdStream__NoOpSlot60(CdStream *self) {}

void CdStream__Mute(CdStream *self) {
    if (self->muted == 0 && sActiveCdStream == self) {
        while (CdMute() == 0) {
        }
        self->muted = 1;
    }
}

void CdStream__Demute(CdStream *self) {
    if (self->muted != 0 && sActiveCdStream == self) {
        while (CdDeMute() == 0) {
        }
        self->muted = 0;
    }
}

/* Waits up to `tries` polls for the next frame's sectors. Returns 1 with the
 * frame in *addr and its number in *frame; 0 when none came (the ring is
 * freed); -1 at the end of the stream (a frame number past totalFrames, or
 * lower than the last one, which reports frame 0), after which the stream
 * closes if onStreamEnd is set. */
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

/* Tests onStreamEnd but calls onFrameReady. */
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

/* MATCHING: cd_stream.h's slot keeps its callers' (self, fn, arg); a narrower slot drops their argument loads */
void CdStream__NoOpSlot7C(CdStream *self) {}

CdStreamMethods *GetCdStreamMethods(void) {
    return &gCdStreamMethods;
}
