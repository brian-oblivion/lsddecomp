#ifndef CDSTREAM_H
#define CDSTREAM_H

#include "BasicClass.h"

/*
 * CdStream -- one streamed CD file (an FMV's sectors) read through Sony's
 * libcd streaming library (StSetRing/StSetStream/StGetNext/StFreeRing), class
 * id 0x40, method table gCdStreamMethods, a direct BasicClass subclass.
 * Methods in src/code_3770c.c. The one holder is MoviePlayer (gMoviePlayerMethods,
 * src/code_33808.c), whose ctor builds one with New_CdStream(arg2, 15, 0)
 * into its +0x060 and drives it through the slots below.
 *
 * One stream at a time. `gActiveCdStream` is the stream that owns the drive:
 * open sets it, close clears it, and seek/startRead/stop/restart/mute/demute
 * are no-ops for any other object. `state` runs 0 idle -> open ->
 * seek (1) -> startRead (2) -> stop (4) -> restart (back to 0 and re-seek);
 * close tears it down from any state.
 *
 * `speed` < 4 means double speed: the ctor then counts 300 sectors a second
 * (else 150) and startRead reads in mode 0x1C0 (CdlModeStream | CdlModeSpeed
 * | CdlModeRT; else 0x140). `bytesPerFrame` is (sectors a second / fps / 2
 * * 2) * 2054, and open divides the file size by it into `totalFrames`.
 *
 * The callback words (+0x048, +0x04C, +0x054) are cleared by the ctor and
 * written nowhere else: New_CdStream's one caller is MoviePlayer's ctor and
 * gActiveCdStream is read only in code_3770c, so the object reaches no other
 * code, and MoviePlayer only calls slots. It hands its callback to slot
 * +0x07C instead, whose occupant is empty.
 *
 * NO FIELDS/SLOTS MACROS: no class lies below 0x40 (`typeviews.py --tree`).
 */

typedef struct CdStream CdStream;
typedef struct CdStreamMethods CdStreamMethods;

/* Sony's CdlFILE (include/psyq/libcd.h, 24 bytes), which CdSearchFile fills.
 * Spelled here so this header does not bring in LIBCD.H's prototypes, which
 * the units declare locally. `pos` is the CdlLOC that seek and CdlSetloc take. */
typedef struct CdStreamFile {
    /* +0x00 */ u8 pos[4];       /* CdlLOC: minute, second, sector, track */
    /* +0x04 */ u32 size;        /* file size in bytes */
    /* +0x08 */ char name[16];
} CdStreamFile;

struct CdStreamMethods {
    /* ctor: New_CdStream passes (speed, fps, arg3); MoviePlayer (arg2, 15, 0). */
    BASICCLASS_SLOTS(CdStream, (CdStream *self, u32 speed, s32 fps, s32 arg3));
    /* +0x040 */ void (*setRing)(CdStream *self, u32 *ring, u32 size);    /* CdStream__SetRing: StSetRing(ring, size / 2048) while idle */
    /* +0x044 */ s32 (*open)(CdStream *self, char *name, s32 tries);      /* CdStream__Open: 0 once open and seeking, 1 when not */
    /* +0x048 */ void (*close)(CdStream *self);                            /* CdStream__Close */
    /* +0x04C */ void (*seek)(CdStream *self, u8 *pos);                    /* CdStream__Seek: CdlSeekL */
    /* +0x050 */ void (*startRead)(CdStream *self, u32 startFrame, s32 frameCount); /* CdStream__StartRead: frameCount 0 keeps totalFrames */
    /* +0x054 */ void (*stop)(CdStream *self);                             /* CdStream__Stop: CdlPause */
    /* +0x058 */ void (*restart)(CdStream *self);                          /* CdStream__Restart: back to idle and re-seek */
    /* +0x05C */ void (*slot5C)(CdStream *self);                           /* CdStream__NoOpSlot5C; no caller */
    /* +0x060 */ void (*slot60)(CdStream *self);                           /* CdStream__NoOpSlot60; no caller */
    /* +0x064 */ void (*mute)(CdStream *self);                             /* CdStream__Mute: CdlMute */
    /* +0x068 */ void (*demute)(CdStream *self);                           /* CdStream__Demute: CdlDemute */
    /* +0x06C */ s32 (*getNextFrame)(CdStream *self, u32 **addr, u32 *frame, s32 tries); /* CdStream__GetNextFrame: 1 frame, 0 none, -1 end */
    /* +0x070 */ u32 (*freeRing)(CdStream *self, u32 *base);               /* CdStream__FreeRing: StFreeRing */
    /* +0x074 */ void (*unsetRing)(CdStream *self);                        /* CdStream__UnsetRing: StUnSetRing */
    /* +0x078 */ void (*clearRing)(CdStream *self);                        /* CdStream__ClearRing: StClearRing */
    /* +0x07C: the occupant, CdStream__NoOpSlot7C, is empty and takes self
     * only; the parameters are the callers'. MoviePlayer__Stop passes
     * (MoviePlayer__MarkStopped, player) and MoviePlayer__Abort (0, 0), in
     * $a1/$a2, so a narrower slot would drop those argument loads. */
    /* +0x07C */ void (*slot7C)(CdStream *self, void (*fn)(), void *arg);
};                                   /* 31 slots, 0x80 bytes */

struct CdStream {
    BASICCLASS_FIELDS(CdStreamMethods);
    /* +0x00C */ CdStreamFile file;             /* CdSearchFile (open); file.pos goes to seek and CdlSetloc */
    /* +0x024 */ u8 cdResult[8];                /* CdSync result buffer (CdStream__Sync) */
    /* +0x02C */ s32 state;                     /* 0 idle, 1 seeking, 2 reading, 4 stopped */
    /* +0x030 */ s32 muted;                     /* mute / demute */
    /* +0x034 */ s32 speed;                     /* the ctor's; < 4 is double speed */
    /* +0x038 */ s32 bytesPerFrame;             /* the ctor's, from speed and fps */
    /* +0x03C */ s32 unk3C;                     /* the ctor's arg3; nothing reads it */
    /* +0x040 */ s32 totalFrames;               /* file.size / bytesPerFrame (open), or startRead's frameCount */
    /* +0x044 */ void *cbArg;                   /* the argument onFrameReady and onSeekDone are called with; never written */
    /* +0x048 */ void (*onFrameReady)(void *arg); /* called before a frame's sectors go back to the ring */
    /* +0x04C */ void (*onStreamEnd)(void *arg);  /* only tested: set, OnStreamEnd calls onFrameReady and closes */
    /* +0x050 */ u32 *ring;                     /* setRing's ring; open fails without one */
    /* +0x054 */ void (*onSeekDone)(void *arg); /* set: seek goes asynchronous and OnCdSeekComplete calls this */
    /* +0x058 */ s32 lastFrame;                 /* the last frame number getNextFrame saw */
};                                   /* 0x5C bytes: New_CdStream */

extern CdStreamMethods gCdStreamMethods;
extern CdStreamMethods *Get_vtable_CdStream(void);  /* returns &gCdStreamMethods */
extern CdStream *gActiveCdStream;                    /* the stream that owns the drive, or NULL */

CdStream *New_CdStream(s32 speed, s32 fps, s32 arg3);
void CdStream__CdStream(CdStream *self, u32 speed, s32 fps, s32 arg3);
void CdStream__Finalize(CdStream *self);
void CdStream__SetRing(CdStream *self, u32 *ring, u32 size);
s32 CdStream__Open(CdStream *self, char *name, s32 tries);
void CdStream__Close(CdStream *self);
void CdStream__Seek(CdStream *self, u8 *pos);
void CdStream__StartRead(CdStream *self, u32 startFrame, s32 frameCount);
void CdStream__Stop(CdStream *self);
void CdStream__Restart(CdStream *self);
void CdStream__NoOpSlot5C(CdStream *self);
void CdStream__NoOpSlot60(CdStream *self);
void CdStream__Mute(CdStream *self);
void CdStream__Demute(CdStream *self);
s32 CdStream__GetNextFrame(CdStream *self, u32 **addr, u32 *frame, s32 tries);
u32 CdStream__FreeRing(CdStream *self, u32 *base);
void CdStream__UnsetRing(CdStream *self);
void CdStream__ClearRing(CdStream *self);
void CdStream__NoOpSlot7C(CdStream *self);

/* Not slots: code_3770c's helpers. */
s32 SetupCdStreamAudio(CdStream *self);                          /* open: the CD audio mix (SpuSetCommonAttr) */
void OnCdSeekComplete(u8 status, u8 *result);                    /* seek's CdSyncCallback */
void CdStream__ReleaseFrame(CdStream *self, u32 *base, u32 frame); /* onFrameReady, then freeRing */
void CdStream__OnStreamEnd(CdStream *self);
int CdStream__Sync(CdStream *self, int mode);                   /* CdSync(mode, cdResult); no caller */

#endif
