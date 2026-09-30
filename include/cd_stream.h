#ifndef CD_STREAM_H
#define CD_STREAM_H

#include "basic_class.h"
#include <libcd.h>

/**
 * @file cd_stream.h
 * @brief CdStream, one streamed CD file (an FMV's sectors) read through
 * libcd's streaming library, and its helpers.
 */

typedef struct CdStream CdStream;
typedef struct CdStreamMethods CdStreamMethods;

/** CdStream's class id (gCdStreamMethods word +0x000). */
#define CDSTREAM_CLASS_ID 0x40

/**
 * @brief CdStream::state. Open seeks (SEEKING), startRead reads (READING),
 * stop pauses the drive (STOPPED), restart goes back to IDLE and seeks again.
 */
enum CdStreamState {
    CDSTREAM_IDLE = 0,    /**< Closed, or restarting. */
    CDSTREAM_SEEKING = 1, /**< Seeking to the file (open, seek, restart). */
    CDSTREAM_READING = 2, /**< Streaming (startRead). */
    CDSTREAM_STOPPED = 4  /**< Paused (stop). */
};

/**
 * @brief CdStream's method table: BasicClass's fifteen slots, with a ctor
 * that takes (cdSpeed, fps, reserved), then sixteen of its own.
 */
struct CdStreamMethods {
    BASICCLASS_SLOTS(CdStream, (CdStream * self, u32 cdSpeed, s32 fps, s32 reserved));
    /* +0x040 */ void (*setRing)(CdStream *self, u32 *ring, u32 size); /**< @see CdStream__SetRing */
    /* +0x044 */ s32 (*open)(CdStream *self, char *name, s32 tries);   /**< @see CdStream__Open */
    /* +0x048 */ void (*close)(CdStream *self);                        /**< @see CdStream__Close */
    /* +0x04C */ void (*seek)(CdStream *self, CdlLOC *pos);            /**< @see CdStream__Seek */
    /* +0x050 */ void (*startRead)(CdStream *self, u32 startFrame, s32 frameCount); /**< @see CdStream__StartRead */
    /* +0x054 */ void (*stop)(CdStream *self);    /**< @see CdStream__Stop */
    /* +0x058 */ void (*restart)(CdStream *self); /**< @see CdStream__Restart */
    /* +0x05C */ void (*slot5C)(CdStream *self);  /**< @see CdStream__NoOpSlot5C */
    /* +0x060 */ void (*slot60)(CdStream *self);  /**< @see CdStream__NoOpSlot60 */
    /* +0x064 */ void (*mute)(CdStream *self);    /**< @see CdStream__Mute */
    /* +0x068 */ void (*demute)(CdStream *self);  /**< @see CdStream__Demute */
    /* +0x06C */ s32 (*getNextFrame)(CdStream *self, u32 **addr, u32 *frame,
                                     s32 tries);             /**< @see CdStream__GetNextFrame */
    /* +0x070 */ u32 (*freeRing)(CdStream *self, u32 *base); /**< @see CdStream__FreeRing */
    /* +0x074 */ void (*unsetRing)(CdStream *self);          /**< @see CdStream__UnsetRing */
    /* +0x078 */ void (*clearRing)(CdStream *self);          /**< @see CdStream__ClearRing */
    /* +0x07C */ void (*setEndCallback)(CdStream *self, void (*fn)(),
                                        void *arg); /**< @see CdStream__SetEndCallback; `fn` and `arg` are what MoviePlayer__Rewind and MoviePlayer__Abort pass, and the empty occupant ignores them. */
}; /* 31 slots, 0x80 bytes */

/**
 * @brief One streamed CD file, an FMV's sectors, read through Sony's libcd
 * streaming library (StSetRing, StSetStream, StGetNext, StFreeRing) with the
 * CD audio mixed into the SPU output (class id 0x40). A direct BasicClass
 * subclass; no class derives from it. Methods in src/cd/cd_stream.c.
 *
 * One stream at a time: the stream that owns the drive is the one open last
 * set active, and close clears it; seek, startRead, stop, restart, mute and
 * demute do nothing for any other object. `state` runs IDLE -> open ->
 * SEEKING -> startRead -> READING -> stop -> STOPPED -> restart -> IDLE and
 * seeks again; close tears the stream down from any state.
 *
 * `cdSpeed` below 4 means double speed: the ctor then counts 300 sectors a
 * second (else 150) and startRead streams at double speed. `bytesPerFrame`
 * is (sectors a second / fps / 2) * 2054, and open divides the file's size by
 * it into `totalFrames`.
 *
 * Lifecycle: the one holder is MoviePlayer (src/graphics/movie_player.c),
 * whose ctor builds one with New_CdStream(cdSpeed, MOVIE_FPS, 0) and drives it
 * through the slots. The callback fields are cleared by the ctor and written
 * nowhere else, so nothing ever reaches them; MoviePlayer hands its callback
 * to slot +0x07C instead, whose occupant is empty.
 */
struct CdStream {
    BASICCLASS_FIELDS(CdStreamMethods);
    /* +0x00C */ CdlFILE file; /**< CdSearchFile's result (open); file.pos is where seek and startRead go. */
    /* +0x024 */ u8 cdResult[8];    /**< CdSync's result buffer (CdStream__Sync). */
    /* +0x02C */ s32 state;         /**< enum CdStreamState. */
    /* +0x030 */ s32 muted;         /**< Set by mute, cleared by demute. */
    /* +0x034 */ s32 cdSpeed;       /**< The ctor's; below 4 is double speed. */
    /* +0x038 */ s32 bytesPerFrame; /**< The ctor's, from speed and fps. */
    /* +0x03C */ s32 reserved;      /**< The ctor's third argument; nothing reads it. */
    /* +0x040 */ s32 totalFrames; /**< file.size / bytesPerFrame (open), or startRead's frameCount. */
    /* +0x044 */ void *cbArg; /**< The argument onFrameReady and onSeekDone are called with; never written. */
    /* +0x048 */ void (*onFrameReady)(void *arg); /**< Called before a frame's sectors go back to the ring. */
    /* +0x04C */ void (*onStreamEnd)(void *arg); /**< Only tested: when set, OnStreamEnd calls onFrameReady and closes. */
    /* +0x050 */ u32 *ring; /**< setRing's ring buffer; open fails without one. */
    /* +0x054 */ void (*onSeekDone)(void *arg); /**< When set, seek is asynchronous and OnCdSeekComplete calls it. */
    /* +0x058 */ s32 lastFrame; /**< The last frame number getNextFrame saw. */
}; /* 0x5C bytes: New_CdStream */

/** @brief CdStream's method table (see CdStreamMethods). */
extern CdStreamMethods gCdStreamMethods;

/**
 * @brief Returns CdStream's method table.
 * @return &gCdStreamMethods.
 */
extern CdStreamMethods *GetCdStreamMethods(void);

/**
 * @brief Allocates a CdStream from the BMemPMgr pool and constructs it.
 * @param cdSpeed  Drive speed code; below 4 is double speed.
 * @param fps      The movie's frames per second.
 * @param reserved Stored, never read.
 * @return The new stream, or NULL when the pool is exhausted.
 */
CdStream *New_CdStream(s32 cdSpeed, s32 fps, s32 reserved);

/**
 * @brief Constructor (slot +0x008): BasicClass's ctor, the speed and
 * `bytesPerFrame` from `cdSpeed` and `fps`, the ring and callbacks cleared,
 * idle and not muted.
 * @param self     The object being constructed.
 * @param cdSpeed  Drive speed code; below 4 is double speed.
 * @param fps      The movie's frames per second.
 * @param reserved Stored, never read.
 */
void CdStream__CdStream(CdStream *self, u32 cdSpeed, s32 fps, s32 reserved);

/**
 * @brief Finalizer (slot +0x00C): closes the stream, then BasicClass's
 * finalize.
 * @param self The object being finalized.
 */
void CdStream__Finalize(CdStream *self);

/**
 * @brief Slot +0x040: while idle, hands `ring` to StSetRing and keeps it.
 * @param self The stream.
 * @param ring The ring buffer.
 * @param size Its size in bytes (StSetRing takes sectors).
 */
void CdStream__SetRing(CdStream *self, u32 *ring, u32 size);

/**
 * @brief Slot +0x044: opens "\<data directory><name>;1" and seeks to it.
 *
 * Only while idle and with a ring set: looks the file up, retrying up to
 * `tries` more times (forever if negative), counts its frames, sets up the
 * CD audio mix (SetupCdStreamAudio), makes this the active stream and seeks
 * to the file.
 * @param self  The stream.
 * @param name  The file's name under the data directory.
 * @param tries Extra lookups before giving up; negative retries forever.
 * @return 0 once seeking, or when another stream owns the drive; 1 when the
 *         stream is not idle, has no ring, or the file is not found.
 */
s32 CdStream__Open(CdStream *self, char *name, s32 tries);

/**
 * @brief Slot +0x048: stops the active stream, sets it idle and gives up the
 * drive. Does nothing for an idle or inactive stream.
 * @param self The stream.
 */
void CdStream__Close(CdStream *self);

/**
 * @brief Slot +0x04C: seeks the active, not-streaming stream to `pos` and
 * sets it SEEKING. With `onSeekDone` set the seek is asynchronous and
 * OnCdSeekComplete reports it; otherwise it blocks until the drive takes
 * the command.
 * @param self The stream.
 * @param pos  The disc position.
 */
void CdStream__Seek(CdStream *self, CdlLOC *pos);

/**
 * @brief Slot +0x050: starts streaming the active, seeking stream from
 * `startFrame`, muted while the read is issued, and sets it READING.
 * @param self       The stream.
 * @param startFrame First frame, StSetStream's start.
 * @param frameCount The frame count to stop at, or 0 to keep totalFrames.
 */
void CdStream__StartRead(CdStream *self, u32 startFrame, s32 frameCount);

/**
 * @brief Slot +0x054: stops the active, streaming stream: mutes, clears and
 * unsets the ring, pauses the drive and sets it STOPPED.
 * @param self The stream.
 */
void CdStream__Stop(CdStream *self);

/**
 * @brief Slot +0x058: sets the active, stopped stream idle and seeks back to
 * the start of the file.
 * @param self The stream.
 */
void CdStream__Restart(CdStream *self);

/**
 * @brief Slot +0x05C: empty; nothing calls it.
 * @param self The stream.
 */
void CdStream__NoOpSlot5C(CdStream *self);

/**
 * @brief Slot +0x060: empty; nothing calls it.
 * @param self The stream.
 */
void CdStream__NoOpSlot60(CdStream *self);

/**
 * @brief Slot +0x064: mutes the CD audio of the active stream, unless it is
 * muted.
 * @param self The stream.
 */
void CdStream__Mute(CdStream *self);

/**
 * @brief Slot +0x068: unmutes the CD audio of the active, muted stream.
 * @param self The stream.
 */
void CdStream__Demute(CdStream *self);

/**
 * @brief Slot +0x06C: waits for the next frame's sectors.
 *
 * At the end of the stream (a frame number past totalFrames, or lower than
 * the last one, which reports frame 0) it releases the frame and, when
 * onStreamEnd is set, closes the stream.
 * @param self  The stream.
 * @param addr  Receives the frame's data.
 * @param frame Receives the frame number.
 * @param tries Polls before giving up; negative for a very large count.
 * @return 1 with a frame, 0 when none came (the ring is freed), -1 at the
 *         end of the stream.
 */
s32 CdStream__GetNextFrame(CdStream *self, u32 **addr, u32 *frame, s32 tries);

/**
 * @brief Slot +0x070: gives a frame's sectors back to the ring.
 * @param self The stream.
 * @param base The frame's data, as StGetNext returned it.
 * @return StFreeRing's result.
 */
u32 CdStream__FreeRing(CdStream *self, u32 *base);

/**
 * @brief Slot +0x074: StUnSetRing.
 * @param self The stream.
 */
void CdStream__UnsetRing(CdStream *self);

/**
 * @brief Slot +0x078: StClearRing.
 * @param self The stream.
 */
void CdStream__ClearRing(CdStream *self);

/**
 * @brief Slot +0x07C: empty.
 * @param self The stream.
 */
void CdStream__SetEndCallback(CdStream *self);

/* ---- Helpers, not slots. */

/**
 * @brief Sets the SPU's master and CD input volumes to full and mixes the CD
 * audio in.
 * @param self The stream (unused).
 * @return 1.
 */
s32 SetupCdStreamAudio(CdStream *self);

/**
 * @brief The asynchronous seek's CdSyncCallback: when the command completes,
 * removes itself and calls the active stream's onSeekDone.
 * @param status The command's status (CdlComplete on success).
 * @param result The command's result bytes (unused).
 */
void OnCdSeekComplete(u8 status, u8 *result);

/**
 * @brief When onFrameReady is set, calls it and gives the frame's sectors
 * back to the ring.
 * @param self  The stream.
 * @param base  The frame's data.
 * @param frame The frame number (unused).
 */
void CdStream__ReleaseFrame(CdStream *self, u32 *base, u32 frame);

/**
 * @brief When onStreamEnd is set, calls onFrameReady (not onStreamEnd) and
 * closes the stream.
 * @param self The stream.
 */
void CdStream__OnStreamEnd(CdStream *self);

/**
 * @brief CdSync into the stream's result buffer. Nothing calls it.
 * @param self The stream.
 * @param mode CdSync's mode (0 waits, 1 polls).
 * @return CdSync's status.
 */
int CdStream__Sync(CdStream *self, int mode);

#endif
