/*
 * MoviePlayer's methods (include/movie_player.h: CD-streamed, MDEC-decoded
 * FMV), in ROM order: the allocator, ctor and finalize, the frame buffers'
 * setup and release, play, rewind and their start requests, the per-tick
 * advance, abort, the frame pull and strip draw, pollActive and
 * decodeFrame, with OnMdecStripDone, the MDEC's callback, then
 * WaitFrameReady and setAutoPlay, ending with its getter
 * GetMoviePlayerMethods; its method table closes the file. A (void *)
 * entry in it is a method inherited from BasicClass and declared on its
 * type.
 */
#include "common.h"
#include <libgte.h>
#include <libgpu.h>
#include <libpress.h>
#include "movie_player.h"
#include "bmem_pmgr.h"
#include "cd_driver.h"

/* Set by the first ctor, which DecDCTReset(0)s the MDEC. */
static s32 sMdecInitialized SDATA = 0;
/* The playing movie, or NULL: play sets it, pollActive clears it. */
static MoviePlayer *sActiveMoviePlayer SDATA = NULL;
/* Play's clearImage colour: black. */
static u8 sMovieClearColor[4] SDATA = {0, 0, 0, 0};
/* PollActive's call count, from 1. */
static s32 sMoviePollCounter SDATA = 1;

/* MoviePlayer's stream and decode geometry. */
#define MOVIE_FPS 15                          /* New_CdStream's fps */
#define MOVIE_RING_SIZE (36 * CD_SECTOR_SIZE) /* the sector ring handed to setRing */
#define MOVIE_STRIP_W 16                      /* one DecDCTout strip's pixel width */
#define MOVIE_OPEN_TRIES 100                  /* CdStream open's tries */
#define MOVIE_FRAME_TRIES 8388608             /* CdStream getNextFrame's tries */
#define MOVIE_SYNC_HEIGHT 128       /* frames shorter than this DrawSync before each strip */
#define MOVIE_KEEP_ACTIVE_POLLS 100 /* pollActive's stop interval while keepActive */

/* Allocate and construct a MoviePlayer; NULL, the object freed, when the
 * ctor fails (it returns nonzero). */
MoviePlayer *New_MoviePlayer(DrawRect *frame, s32 cdSpeed, s32 external) {
    MoviePlayer *obj = BMemPMgrAlloc(sizeof(MoviePlayer));

    if (obj != NULL) {
        if (GetMoviePlayerMethods()->ctor(obj, frame, cdSpeed, external) == 0) {
            return obj;
        }
        BMemPMgrFree(obj);
    }
    return NULL;
}

/* ctor (+0x008): a CdStream and the decode buffers (1 when either cannot be
 * had), the MDEC reset by the first player built, its output callback
 * OnMdecStripDone, and auto-play on. */
s32 MoviePlayer__MoviePlayer(MoviePlayer *self, DrawRect *frame, s32 cdSpeed, s32 external) {
    GetBasicClassMethods()->ctor((BasicClass *)self);
    self->methods = GetMoviePlayerMethods();
    self->stream = New_CdStream(cdSpeed, MOVIE_FPS, 0);
    if (self->stream != NULL) {
        if (MoviePlayer__InitFrame(self, frame, external) == 0) {
            if (sMdecInitialized == 0) {
                DecDCTReset(0);
            }
            sMdecInitialized = 1;
            DecDCToutCallback(OnMdecStripDone);
            self->stream->methods->setRing(self->stream, self->ring, MOVIE_RING_SIZE);
            self->pendingStart = 0;
            self->methods->setAutoPlay(self, 1);
            return 0;
        }
    }
    return 1;
}

/* finalize (+0x00C): release the stream, detach and reset the MDEC, free
 * the buffers. */
void MoviePlayer__Finalize(MoviePlayer *self) {
    self->stream = self->stream->methods->release(self->stream);
    DecDCToutCallback(NULL);
    DecDCTReset(0);
    MoviePlayer__FreeFrameBuffers(self);
    GetBasicClassMethods()->finalize((BasicClass *)self);
}

/* Unless `external` (the caller's buffers), allocate the two frame
 * buffers, the ring and the strip; 1, with what was had freed, when one
 * cannot be. Then take `frame`, and the first strip at its left edge. */
s32 MoviePlayer__InitFrame(MoviePlayer *self, DrawRect *frame, s32 external) {
    s32 size;
    s32 unused[2]; /* MATCHING: never used; it gives retail's 0x28-byte stack */

    self->external = external;
    if (external == 0) {
        self->strip = NULL;
        self->frames[1] = NULL;
        self->frames[0] = NULL;
        self->ring = NULL;
        size = frame->w * frame->h * 2 + 4096;
        if ((self->frames[0] = BMemPMgrAlloc(size)) == NULL) {
            goto fail;
        }
        if ((self->frames[1] = BMemPMgrAlloc(size)) == NULL) {
            goto fail;
        }
        if ((self->ring = BMemPMgrAlloc(MOVIE_RING_SIZE)) == NULL) {
            goto fail;
        }
        if ((self->strip = BMemPMgrAlloc(frame->h * MOVIE_STRIP_W * sizeof(u16))) == NULL) {
            goto fail;
        }
    }
    self->stripRect = *frame;
    self->frame = self->stripRect;
    self->stripRect.w = MOVIE_STRIP_W;
    self->stripSize = (self->stripRect.h * MOVIE_STRIP_W) >> 1; /* 16-bit pixels, in words */
    return 0;
/* MATCHING: every failed allocation jumps here; one || chain with the
 * cleanup in its body lays the cleanup out before the frame setup. */
fail:
    MoviePlayer__FreeFrameBuffers(self);
    return 1;
}

/* Unless `external`, free InitFrame's buffers. */
void MoviePlayer__FreeFrameBuffers(MoviePlayer *self) {
    if (self->external == 0) {
        BMemPMgrFree(self->frames[0]);
        BMemPMgrFree(self->frames[1]);
        BMemPMgrFree(self->ring);
        BMemPMgrFree(self->strip);
    }
}

/* play (+0x040): unless a movie is already active, open `name` on the
 * stream (1 when it will not open) and become the active movie, its frame
 * area cleared to black. */
s32 MoviePlayer__Play(MoviePlayer *self, char *name, s32 frameCount, s32 keepActive, s32 loops) {
    DrawSystem *ds;

    if (sActiveMoviePlayer == NULL) {
        if (self->autoPlay != 0) {
            MoviePlayer__RequestStart(self);
        }
        self->frameCount = frameCount;
        if (self->stream->methods->open(self->stream, name, MOVIE_OPEN_TRIES) == 0) {
            sActiveMoviePlayer = self;
            self->haveFrame = 0;
            self->frameIndex = 0;
            self->frameDone = 1;
            self->streamEnded = 0;
            self->finished = 0;
            self->keepActive = keepActive;
            self->loops = loops;
            ds = GetDrawSystem();
            ds->methods->clearImage(ds, sMovieClearColor, &self->frame);
            return 0;
        }
        return 1;
    }
    return 0;
}

void MoviePlayer__RequestStart(MoviePlayer *self) {
    self->pendingStart = 1;
}

/* rewind (+0x044): when active, reset the frame state and restart the stream
 * (its setEndCallback, handed RequestRestart, is empty). */
void MoviePlayer__Rewind(MoviePlayer *self) {
    MoviePlayer *cur = sActiveMoviePlayer;

    if (cur == self) {
        cur->haveFrame = 0;
        cur->frameIndex = 0;
        cur->frameDone = 1;
        cur->streamEnded = 0;
        cur->finished = 0;
        cur->stream->methods->setEndCallback(cur->stream, MoviePlayer__RequestRestart, cur);
        cur->started = 0;
        cur->stream->methods->restart(cur->stream);
    }
}

void MoviePlayer__RequestRestart(MoviePlayer *self) {
    self->pendingStart = -1;
}

/* advance (+0x048), StreamTask's per-tick call: when active, start the
 * stream reading on a pending start (on a restart, muting it once `loops`
 * runs out), else decode once started. */
/* The original has no return statement when this player is not the active
 * one, or is neither starting nor started: the caller gets whatever result
 * was left over. For an inactive player that is non-zero (StreamTask__Update's
 * call leaves the slot's own address there), so StreamTask takes the movie as
 * done and fades out. Neither starting nor started, it is the 0 just read
 * from `started`. */
s32 MoviePlayer__Advance(MoviePlayer *self) {
    MoviePlayer *cur = sActiveMoviePlayer;

    if (cur == self) {
        if (cur->pendingStart == 0) {
            if (cur->started == 0) {
                goto out;
            }
        } else {
            cur->stream->methods->startRead(cur->stream, 1, cur->frameCount);
            if (cur->pendingStart < 0) {
                if (cur->loops == 0 || --cur->loops == 0) {
                    cur->stream->methods->mute(cur->stream);
                }
            }
            self->pendingStart = 0;
            self->started = 1;
            return 0;
        }
        return cur->methods->decodeFrame(cur);
    }
out:; /* MATCHING: no return value on this path, as the original */
}

/* abort (+0x04C): when active, close the stream and finish. */
void MoviePlayer__Abort(MoviePlayer *self) {
    MoviePlayer *cur = sActiveMoviePlayer;

    if (cur == self) {
        cur->streamEnded = 1;
        cur->keepActive = 0;
        cur->stream->methods->close(cur->stream);
        cur->finished = 1;
        if (cur->started == 0) {
            cur->stream->methods->setEndCallback(cur->stream, 0, 0);
            cur->started = 1;
            cur->finished = 1;
        }
    }
}

void MoviePlayer__NoOpSlot50(void) {}

void MoviePlayer__NoOpSlot54(void) {}

/* pullFrame (+0x058): VLC-decode the stream's next frame into the other
 * frame buffer; 1 when there is none yet. The stream's end stops it. */
s32 MoviePlayer__PullFrame(MoviePlayer *self) {
    u32 *data;
    s32 size;
    s32 r;

    if (self->streamEnded == 0) {
        r = self->stream->methods->getNextFrame(self->stream, &data, &size, MOVIE_FRAME_TRIES);
        if (r != 0) {
            if (size != 0) {
                self->frameIndex ^= 1;
                DecDCTvlc((u_long *)data, (u_long *)self->frames[self->frameIndex]);
            }
            self->stream->methods->freeRing(self->stream, data);
            if (r < 0) {
                self->streamEnded = 1;
                self->stream->methods->stop(self->stream);
            }
            return 0;
        }
    }
    return 1;
}

void MoviePlayer__NoOpSlot5C(void) {}

/* drawStrip (+0x060): upload the decoded strip and ask the MDEC for the
 * next, or, past the frame's right edge, mark the frame done. */
void MoviePlayer__DrawStrip(MoviePlayer *self) {
    DrawSystem *ds = GetDrawSystem();

    ds->methods->loadImage(ds, &self->stripRect, self->strip);
    self->stripRect.x += self->stripRect.w;
    if (self->stripRect.x < self->frame.x + self->frame.w) {
        if (self->stripRect.h < MOVIE_SYNC_HEIGHT) {
            DrawSync(0);
        }
        DecDCTout((u_long *)self->strip, self->stripSize);
    } else {
        self->frameDone = 1;
        self->stripRect.x = self->frame.x;
        self->stripRect.y = self->frame.y;
        if (self->streamEnded != 0) {
            self->finished = 1;
        }
    }
}

/* pollActive (+0x064), once the movie has finished: with keepActive, stay
 * active, rewinding the stream every MOVIE_KEEP_ACTIVE_POLLS calls (0);
 * otherwise no movie is active any more (1). */
s32 MoviePlayer__PollActive(MoviePlayer *self) {
    if (self->keepActive != 0) {
        if (sMoviePollCounter++ > MOVIE_KEEP_ACTIVE_POLLS) {
            sMoviePollCounter = 1;
            self->methods->rewind(self);
        }
        return 0;
    }
    sActiveMoviePlayer = NULL;
    return 1;
}

/* decodeFrame (+0x068): when active and not finished, hand the pulled
 * frame to the MDEC once the last one is drawn, and pull the next. */
/* The original has no return statement when another player is active: the
 * caller gets whatever result was left over. */
s32 MoviePlayer__DecodeFrame(MoviePlayer *self) {
    MoviePlayer *cur = sActiveMoviePlayer;

    if (cur == self) {
        if (cur->finished == 0) {
            if (cur->haveFrame != 0) {
                MoviePlayer__WaitFrameReady(cur);
                cur->frameDone = 0;
                if (cur->stripRect.h < MOVIE_SYNC_HEIGHT) {
                    DrawSync(0);
                }
                DecDCTin((u_long *)cur->frames[cur->frameIndex], 2);
                DecDCTout((u_long *)cur->strip, cur->stripSize);
            }
            self->haveFrame = self->methods->pullFrame(self) == 0;
            return 0;
        }
        return cur->methods->pollActive(cur);
    }
} /* MATCHING: no return when another player is active, as the original */

/* The MDEC's DecDCTout callback: drawStrip of sActiveMoviePlayer, when
 * there is one. */
void OnMdecStripDone(void) {
    if (sActiveMoviePlayer != NULL) {
        sActiveMoviePlayer->methods->drawStrip(sActiveMoviePlayer);
    }
}

/* Wait for drawStrip to finish the frame. */
void MoviePlayer__WaitFrameReady(MoviePlayer *self) {
    while (self->frameDone == 0) { /* MATCHING: not volatile, so retail reads it once and spins */
    }
}

/* setAutoPlay (+0x06C). */
void MoviePlayer__SetAutoPlay(MoviePlayer *self, s32 autoPlay) {
    self->autoPlay = autoPlay;
}

MoviePlayerMethods *GetMoviePlayerMethods(void) {
    return &gMoviePlayerMethods;
}

/* MoviePlayer: BasicClass's table, then play, rewind, the per-frame
 * advance and decode, and the poll the game loop runs. */
MoviePlayerMethods gMoviePlayerMethods = {
    /* +0x000 header */ MOVIEPLAYER_CLASS_ID,
    /* +0x004 release */ (void *)BasicClass__Release,
    /* +0x008 ctor */ MoviePlayer__MoviePlayer,
    /* +0x00C finalize */ MoviePlayer__Finalize,
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
    /* +0x040 play */ MoviePlayer__Play,
    /* +0x044 rewind */ MoviePlayer__Rewind,
    /* +0x048 advance */ MoviePlayer__Advance,
    /* +0x04C abort */ MoviePlayer__Abort,
    /* +0x050 slot50 */ MoviePlayer__NoOpSlot50,
    /* +0x054 slot54 */ MoviePlayer__NoOpSlot54,
    /* +0x058 pullFrame */ MoviePlayer__PullFrame,
    /* +0x05C slot5C */ MoviePlayer__NoOpSlot5C,
    /* +0x060 drawStrip */ MoviePlayer__DrawStrip,
    /* +0x064 pollActive */ MoviePlayer__PollActive,
    /* +0x068 decodeFrame */ MoviePlayer__DecodeFrame,
    /* +0x06C setAutoPlay */ MoviePlayer__SetAutoPlay,
};
