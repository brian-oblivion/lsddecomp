#ifndef MOVIEPLAYER_H
#define MOVIEPLAYER_H

#include "basic_class.h"
#include "cd_stream.h"
#include "DrawSystem.h"

/*
 * MoviePlayer -- CD-streamed, MDEC-decoded FMV playback (class id 0x70,
 * method table gMoviePlayerMethods, a direct BasicClass subclass). Methods in
 * src/graphics/GraphicsResources.c; the object is 0x6C bytes (New_MoviePlayer). The one
 * holder is StreamTask (`player`, include/StreamTask.h, src/app/Task.c),
 * which builds it with New_MoviePlayer(GetDefaultMovieFrame(), 0, 0)
 * and calls +0x06C setAutoPlay, +0x040 play, +0x048 advance, +0x04C abort
 * and +0x004 release.
 *
 * The pipeline, measured from the methods:
 *   - the ctor builds a CdStream (New_CdStream(cdSpeed, MOVIE_FPS, 0)) at `stream`
 *     and hands it the 0x12000-byte `ring` (CdStream setRing);
 *   - pullFrame takes the next frame's sectors from the stream
 *     (getNextFrame), flips `frameIndex` and VLC-decodes them into
 *     frames[frameIndex] (DecDCTvlc), then gives the sectors back (freeRing);
 *   - decodeFrame, once a frame is pulled (`haveFrame`), waits for the
 *     previous frame's last strip (`frameDone`, WaitFrameReady), feeds
 *     frames[frameIndex] to the MDEC (DecDCTin) and asks for the first strip
 *     (DecDCTout into `strip`), then pulls the next frame;
 *   - the MDEC's output callback (OnMdecStripDone) runs the ACTIVE player's
 *     drawStrip: upload `strip` at `stripRect` (DrawSystem loadImage), step
 *     stripRect.x by its 16-pixel width, and either request the next strip
 *     or, past the frame's right edge, rewind and set `frameDone`.
 * One movie plays at a time: `sActiveMoviePlayer` is the player play made
 * active; rewind, advance, abort and decodeFrame do nothing for any other
 * object, and pollActive clears it when the movie is over.
 *
 * NO FIELDS/SLOTS MACROS: no class lies below 0x70 (`typeviews.py --tree`).
 */

typedef struct MoviePlayer MoviePlayer;
typedef struct MoviePlayerMethods MoviePlayerMethods;

struct MoviePlayerMethods {
    /* ctor: 0 on success, 1 when the stream or a buffer cannot be had
     * (New_MoviePlayer frees the object on nonzero). `frame` is the frame
     * rectangle, `speed` goes to New_CdStream, `external` nonzero means the
     * caller owns the buffers (InitFrame allocates none). */
    BASICCLASS_SLOTS_R(MoviePlayer, s32, (MoviePlayer * self, DrawRect *frame, s32 speed, s32 external));
    /* +0x040 */ s32 (*play)(MoviePlayer *self, char *name, s32 frameCount, s32 keepActive,
                             s32 loops); /* MoviePlayer__Play: 0 started (or another movie is active), 1 open failed */
    /* +0x044 */ void (*rewind)(MoviePlayer *self); /* MoviePlayer__Rewind: reset, hand the stream RequestRestart, restart it */
    /* +0x048 */ s32 (*advance)(MoviePlayer *self); /* MoviePlayer__Advance: StreamTask's per-tick call; nonzero when done */
    /* +0x04C */ void (*abort)(MoviePlayer *self); /* MoviePlayer__Abort: close the stream, finish */
    /* +0x050 */ void (*slot50)(void);             /* MoviePlayer__NoOpSlot50; no caller */
    /* +0x054 */ void (*slot54)(void);             /* MoviePlayer__NoOpSlot54; no caller */
    /* +0x058 */ s32 (*pullFrame)(MoviePlayer *self); /* MoviePlayer__PullFrame: 0 pulled (or end reached), 1 no data */
    /* +0x05C */ void (*slot5C)(void);                 /* MoviePlayer__NoOpSlot5C; no caller */
    /* +0x060 */ void (*drawStrip)(MoviePlayer *self); /* MoviePlayer__DrawStrip: OnMdecStripDone's call */
    /* +0x064 */ s32 (*pollActive)(MoviePlayer *self); /* MoviePlayer__PollActive: 1 once the movie is over */
    /* +0x068 */ s32 (*decodeFrame)(MoviePlayer *self); /* MoviePlayer__DecodeFrame: advance's tail call */
    /* +0x06C */ void (*setAutoPlay)(MoviePlayer *self, s32 autoPlay); /* MoviePlayer__SetAutoPlay */
}; /* 0x70 bytes */

struct MoviePlayer {
    BASICCLASS_FIELDS(MoviePlayerMethods);
    /* +0x00C */ s32 external; /* the ctor's: nonzero, InitFrame allocates and FreeFrameBuffers frees nothing */
    /* +0x010 */ u32 *ring; /* 0x12000 bytes, the stream's sector ring (setRing) */
    /* +0x014 */ u32 *frames[2]; /* w * h * 2 + 0x1000 bytes each: pullFrame's DecDCTvlc output, decodeFrame's DecDCTin input */
    /* +0x01C */ u32 *strip; /* h * 32 bytes: one 16-pixel column, DecDCTout's output, drawStrip's upload */
    /* +0x020 */ DrawRect frame; /* the ctor's frame rectangle; play clears it (clearImage), drawStrip's right edge and rewind point */
    /* +0x02C */ DrawRect stripRect; /* `frame` with w = 16, stepped right by drawStrip. Its s32 w is read as a halfword there
                                      * (lhu +0x030: cc1 narrows the load, byte-verified). Its h (+0x034) is the frame height,
                                      * which drawStrip and decodeFrame test against 0x80 before a DrawSync */
    /* +0x038 */ s32 stripSize; /* h * 16 / 2: DecDCTout's size in words */
    /* +0x03C */ s32 frameIndex; /* which frames[] pullFrame decoded last; play and rewind reset it */
    /* +0x040 */ s32 haveFrame; /* decodeFrame: pullFrame returned 0, so frames[frameIndex] waits for the MDEC */
    /* +0x044 */ s32 finished; /* drawStrip at a frame's end once streamEnded, and abort; decodeFrame then returns pollActive */
    /* +0x048 */ s32 streamEnded; /* pullFrame when getNextFrame returns < 0, and abort; pullFrame then pulls nothing */
    /* +0x04C */ s32 frameDone; /* drawStrip after a frame's last strip; WaitFrameReady spins on it; play and rewind set it */
    /* +0x050 */ s32 pendingStart; /* RequestStart 1, RequestRestart -1; advance starts the stream reading (startRead) while nonzero,
                                      * counting `loops` down when negative, then clears it. The ctor clears it */
    /* +0x054 */ s32 keepActive; /* play's keepActive (StreamTask::keepActive); while set, pollActive does not finish but calls rewind
                                      * every 100 polls; abort clears it */
    /* +0x058 */ s32 loops;      /* play's; advance mutes the stream when it runs out */
    /* +0x05C */ s32 frameCount; /* play's arg2; advance's startRead(1, frameCount) */
    /* +0x060 */ CdStream *stream; /* the ctor's New_CdStream; finalize releases it */
    /* +0x064 */ s32 started; /* advance, after startRead; abort; rewind clears it. With pendingStart clear, advance decodes only once set */
    /* +0x068 */ s32 autoPlay; /* setAutoPlay; play calls RequestStart at once when set */
}; /* 0x6C bytes: New_MoviePlayer */

extern MoviePlayerMethods gMoviePlayerMethods;
extern MoviePlayerMethods *GetMoviePlayerMethods(void); /* returns &gMoviePlayerMethods */

MoviePlayer *New_MoviePlayer(DrawRect *frame, s32 speed, s32 external);
s32 MoviePlayer__MoviePlayer(MoviePlayer *self, DrawRect *frame, s32 speed, s32 external);
void MoviePlayer__Finalize(MoviePlayer *self);
s32 MoviePlayer__InitFrame(MoviePlayer *self, DrawRect *frame, s32 external);
void MoviePlayer__FreeFrameBuffers(MoviePlayer *self);
s32 MoviePlayer__Play(MoviePlayer *self, char *name, s32 frameCount, s32 keepActive, s32 loops);
void MoviePlayer__RequestStart(MoviePlayer *self); /* pendingStart = 1; play's call */
void MoviePlayer__Rewind(MoviePlayer *self);
void MoviePlayer__RequestRestart(MoviePlayer *self); /* pendingStart = -1; rewind hands it to the stream's slot7C, whose occupant is empty */
s32 MoviePlayer__Advance(MoviePlayer *self);
void MoviePlayer__Abort(MoviePlayer *self);
void MoviePlayer__NoOpSlot50(void);
void MoviePlayer__NoOpSlot54(void);
s32 MoviePlayer__PullFrame(MoviePlayer *self);
void MoviePlayer__NoOpSlot5C(void);
void MoviePlayer__DrawStrip(MoviePlayer *self);
s32 MoviePlayer__PollActive(MoviePlayer *self);
s32 MoviePlayer__DecodeFrame(MoviePlayer *self);
void OnMdecStripDone(void); /* the DecDCTout callback: sActiveMoviePlayer's drawStrip */
void MoviePlayer__WaitFrameReady(MoviePlayer *self); /* spin until frameDone */
void MoviePlayer__SetAutoPlay(MoviePlayer *self, s32 autoPlay);

#endif
