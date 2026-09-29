/**
 * @file movie_player.h
 * @brief MoviePlayer, CD-streamed FMV playback decoded by the MDEC, and its
 *        method table.
 */
#ifndef MOVIE_PLAYER_H
#define MOVIE_PLAYER_H

#include "basic_class.h"
#include "cd_stream.h"
#include "draw_system.h"

typedef struct MoviePlayer MoviePlayer;
typedef struct MoviePlayerMethods MoviePlayerMethods;

/**
 * @brief MoviePlayer's method table, gMoviePlayerMethods: BasicClass's slots,
 *        with an s32-returning ctor (MoviePlayer__MoviePlayer), then thirteen
 *        of its own.
 */
struct MoviePlayerMethods {
    BASICCLASS_SLOTS_R(MoviePlayer, s32, (MoviePlayer * self, DrawRect *frame, s32 speed, s32 external));
    /* +0x040 */ s32 (*play)(MoviePlayer *self, char *name, s32 frameCount, s32 keepActive,
                             s32 loops);                /**< @see MoviePlayer__Play */
    /* +0x044 */ void (*rewind)(MoviePlayer *self);     /**< @see MoviePlayer__Rewind */
    /* +0x048 */ s32 (*advance)(MoviePlayer *self);     /**< @see MoviePlayer__Advance */
    /* +0x04C */ void (*abort)(MoviePlayer *self);      /**< @see MoviePlayer__Abort */
    /* +0x050 */ void (*slot50)(void);                  /**< @see MoviePlayer__NoOpSlot50 */
    /* +0x054 */ void (*slot54)(void);                  /**< @see MoviePlayer__NoOpSlot54 */
    /* +0x058 */ s32 (*pullFrame)(MoviePlayer *self);   /**< @see MoviePlayer__PullFrame */
    /* +0x05C */ void (*slot5C)(void);                  /**< @see MoviePlayer__NoOpSlot5C */
    /* +0x060 */ void (*drawStrip)(MoviePlayer *self);  /**< @see MoviePlayer__DrawStrip */
    /* +0x064 */ s32 (*pollActive)(MoviePlayer *self);  /**< @see MoviePlayer__PollActive */
    /* +0x068 */ s32 (*decodeFrame)(MoviePlayer *self); /**< @see MoviePlayer__DecodeFrame */
    /* +0x06C */ void (*setAutoPlay)(MoviePlayer *self, s32 autoPlay); /**< @see MoviePlayer__SetAutoPlay */
};

/**
 * @brief An FMV player (class id 0x70): streams a movie's sectors from the CD
 *        and has the MDEC decode each frame into VRAM, one 16-pixel strip at a
 *        time.
 *
 * The pipeline: the ctor builds a CdStream and hands it the sector `ring`.
 * pullFrame takes the next frame's sectors, VLC-decodes them into the other
 * of `frames` (DecDCTvlc) and gives the sectors back. decodeFrame waits for
 * the previous frame's last strip, feeds the pulled frame to the MDEC
 * (DecDCTin) and asks for the first strip (DecDCTout into `strip`), then
 * pulls the next frame. The MDEC's callback, OnMdecStripDone, runs the active
 * player's drawStrip: it uploads `strip` at `stripRect`, steps right, and
 * either asks for the next strip or, past the frame's right edge, rewinds to
 * the left edge and sets `frameDone`.
 *
 * One movie plays at a time: rewind, advance, abort and decodeFrame act only
 * on the player play made active (sActiveMoviePlayer), and pollActive clears
 * it when the movie is over.
 *
 * Parent BasicClass; no subclasses. Methods in
 * src/graphics/graphics_resources.c. The object is 0x6C bytes
 * (New_MoviePlayer). Its one holder is StreamTask (`player`,
 * src/app/task.c), which builds it with New_MoviePlayer(GetDefaultMovieFrame(),
 * 0, 0) and calls setAutoPlay, play, advance (every tick), abort and release.
 */
struct MoviePlayer {
    BASICCLASS_FIELDS(MoviePlayerMethods);
    /* +0x00C */ s32 external; /**< the ctor's: nonzero when the caller owns the buffers, so InitFrame allocates and FreeFrameBuffers frees nothing */
    /* +0x010 */ u32 *ring;    /**< 0x12000 bytes, the stream's sector ring */
    /* +0x014 */ u32 *frames[2]; /**< w * h * 2 + 0x1000 bytes each: pullFrame's DecDCTvlc output, decodeFrame's DecDCTin input */
    /* +0x01C */ u32 *strip; /**< h * 32 bytes: one decoded 16-pixel column, DecDCTout's output and drawStrip's upload */
    /* +0x020 */ DrawRect frame; /**< the frame rectangle: play clears it to black; drawStrip's right edge and the strips' start */
    /* +0x02C */ DrawRect stripRect; /**< the next strip: `frame` with w = 16, stepped right by drawStrip. Its h, the frame height, decides whether drawStrip and decodeFrame DrawSync first (below 128) */
    /* +0x038 */ s32 stripSize;      /**< h * 16 / 2: one strip's size in words, DecDCTout's */
    /* +0x03C */ s32 frameIndex; /**< which of `frames` pullFrame decoded last; play and rewind reset it */
    /* +0x040 */ s32 haveFrame; /**< set by decodeFrame when pullFrame succeeded, so frames[frameIndex] waits for the MDEC */
    /* +0x044 */ s32 finished; /**< set by drawStrip at a frame's end once `streamEnded`, and by abort; decodeFrame then returns pollActive */
    /* +0x048 */ s32 streamEnded; /**< set when the stream reports its end (pullFrame) and by abort; pullFrame then pulls nothing */
    /* +0x04C */ s32 frameDone; /**< set by drawStrip after a frame's last strip; WaitFrameReady spins on it; play and rewind set it */
    /* +0x050 */ s32 pendingStart; /**< 1 from RequestStart, -1 from RequestRestart, 0 from the ctor: while nonzero, advance starts the stream reading, counting `loops` down on a restart, then clears it */
    /* +0x054 */ s32 keepActive; /**< play's: while set, pollActive does not finish but rewinds every 100 polls; abort clears it */
    /* +0x058 */ s32 loops;      /**< play's: advance mutes the stream when a restart runs it out */
    /* +0x05C */ s32 frameCount; /**< play's: the frame count advance hands the stream's startRead */
    /* +0x060 */ CdStream *stream; /**< the ctor's CdStream; finalize releases it */
    /* +0x064 */ s32 started; /**< set by advance after startRead, and by abort; rewind clears it. With no start pending, advance decodes only once it is set */
    /* +0x068 */ s32 autoPlay; /**< setAutoPlay's: when set, play requests the start at once */
};

/** MoviePlayer's method table. */
extern MoviePlayerMethods gMoviePlayerMethods;

/**
 * @brief Returns MoviePlayer's method table.
 * @return &gMoviePlayerMethods.
 */
extern MoviePlayerMethods *GetMoviePlayerMethods(void);

/**
 * @brief Allocates a MoviePlayer from the pool and constructs it.
 * @param frame    The frame rectangle movies are decoded into.
 * @param cdSpeed  The CD speed handed to New_CdStream.
 * @param external Nonzero when the caller provides the buffers.
 * @return The new player, or NULL when the pool is exhausted or the ctor
 *         fails (the object is then freed).
 */
MoviePlayer *New_MoviePlayer(DrawRect *frame, s32 cdSpeed, s32 external);

/**
 * @brief Constructor (slot +0x008): builds the CdStream and the decode
 *        buffers, resets the MDEC for the first player built, installs
 *        OnMdecStripDone as its output callback and turns auto-play on.
 * @param self     The object to construct.
 * @param frame    The frame rectangle.
 * @param cdSpeed  The CD speed handed to New_CdStream.
 * @param external Nonzero when the caller provides the buffers.
 * @return 0 on success, 1 when the stream or a buffer cannot be had.
 */
s32 MoviePlayer__MoviePlayer(MoviePlayer *self, DrawRect *frame, s32 cdSpeed, s32 external);

/**
 * @brief Finalizer (slot +0x00C): releases the stream, detaches and resets the
 *        MDEC, frees the buffers, then BasicClass's finalizer.
 * @param self The object being destroyed.
 */
void MoviePlayer__Finalize(MoviePlayer *self);

/**
 * @brief Allocates the decode buffers (unless `external`) and sets up the
 *        frame and the first strip at its left edge.
 * @param self     The player.
 * @param frame    The frame rectangle.
 * @param external Nonzero when the caller provides the buffers.
 * @return 0 on success; 1 when a buffer cannot be allocated, those already
 *         had freed.
 */
s32 MoviePlayer__InitFrame(MoviePlayer *self, DrawRect *frame, s32 external);

/**
 * @brief Frees InitFrame's buffers, unless the caller provided them.
 * @param self The player.
 */
void MoviePlayer__FreeFrameBuffers(MoviePlayer *self);

/**
 * @brief Slot +0x040: unless a movie is already playing, opens `name` on the
 *        stream and makes this the active player, its frame cleared to black.
 * @param self       The player.
 * @param name       The movie's file name.
 * @param frameCount The frame count handed to the stream when reading starts.
 * @param keepActive Nonzero to keep the player active (and rewinding) after
 *                   the movie ends, until abort.
 * @param loops      How many restarts play before the stream is muted.
 * @return 0 when started or when another movie is already active; 1 when the
 *         file will not open.
 */
s32 MoviePlayer__Play(MoviePlayer *self, char *name, s32 frameCount, s32 keepActive, s32 loops);

/**
 * @brief Asks advance to start the stream reading (pendingStart = 1).
 * @param self The player.
 */
void MoviePlayer__RequestStart(MoviePlayer *self);

/**
 * @brief Slot +0x044: when active, resets the frame state and restarts the
 *        stream from the beginning.
 * @param self The player.
 */
void MoviePlayer__Rewind(MoviePlayer *self);

/**
 * @brief Asks advance for a restart (pendingStart = -1). Rewind hands it to
 *        the stream's setEndCallback, whose occupant does nothing with it.
 * @param self The player.
 */
void MoviePlayer__RequestRestart(MoviePlayer *self);

/**
 * @brief Slot +0x048, StreamTask's per-tick call: when active, starts the
 *        stream reading on a pending start (on a restart, muting it once
 *        `loops` runs out), else decodes once started.
 * @param self The player.
 * @return 0 after starting the stream, else decodeFrame's result. Undefined
 *         when this player is not active, or active but not yet started.
 */
s32 MoviePlayer__Advance(MoviePlayer *self);

/**
 * @brief Slot +0x04C: when active, closes the stream and marks the movie
 *        finished.
 * @param self The player.
 */
void MoviePlayer__Abort(MoviePlayer *self);

/** @brief Slot +0x050: does nothing. */
void MoviePlayer__NoOpSlot50(void);

/** @brief Slot +0x054: does nothing. */
void MoviePlayer__NoOpSlot54(void);

/**
 * @brief Slot +0x058: VLC-decodes the stream's next frame into the other
 *        frame buffer; the stream's end stops it.
 * @param self The player.
 * @return 0 when a frame was taken (or the end reached); 1 when no data is
 *         ready yet or the stream has already ended.
 */
s32 MoviePlayer__PullFrame(MoviePlayer *self);

/** @brief Slot +0x05C: does nothing. */
void MoviePlayer__NoOpSlot5C(void);

/**
 * @brief Slot +0x060, run from the MDEC callback: uploads the decoded strip
 *        and asks for the next, or, past the frame's right edge, returns to
 *        the left edge and marks the frame done.
 * @param self The player.
 */
void MoviePlayer__DrawStrip(MoviePlayer *self);

/**
 * @brief Slot +0x064, once the movie has finished: with keepActive, stays
 *        active, rewinding every 100 calls; otherwise stops being the active
 *        player.
 * @param self The player.
 * @return 0 while kept active; 1 once no movie is active.
 */
s32 MoviePlayer__PollActive(MoviePlayer *self);

/**
 * @brief Slot +0x068: when active and not finished, hands the pulled frame to
 *        the MDEC once the previous one is drawn, and pulls the next.
 * @param self The player.
 * @return 0 while playing; pollActive's result once finished. Undefined when
 *         this player is not active.
 */
s32 MoviePlayer__DecodeFrame(MoviePlayer *self);

/** @brief The MDEC's DecDCTout callback: the active player's drawStrip, when
 *         there is one. */
void OnMdecStripDone(void);

/**
 * @brief Spins until drawStrip has finished the current frame.
 * @param self The player.
 */
void MoviePlayer__WaitFrameReady(MoviePlayer *self);

/**
 * @brief Slot +0x06C: sets whether play requests the stream's start at once.
 * @param self     The player.
 * @param autoPlay Nonzero for auto-play.
 */
void MoviePlayer__SetAutoPlay(MoviePlayer *self, s32 autoPlay);

#endif
