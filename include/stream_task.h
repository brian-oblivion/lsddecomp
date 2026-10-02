#ifndef STREAM_TASK_H
#define STREAM_TASK_H

#include "task_core.h"
#include "draw_system.h"

/**
 * @file stream_task.h
 * @brief StreamTask, a TaskCore that plays one movie stream through a
 * MoviePlayer inside TaskCore's fade and state machine.
 *
 * Declares the class, its own state and result, and its methods, all
 * defined in src/app/stream_task.c.
 */

typedef struct StreamTask StreamTask;
typedef struct StreamTaskMethods StreamTaskMethods;

/** StreamTask's class id (gStreamTaskMethods word +0x000). */
#define STREAMTASK_CLASS_ID 0x1130

/** StreamTask's own state, past TaskCore's (enum TaskCoreState): onPadConfirm
 * sets it when skipOnConfirm is on, and setState answers it with exit, which
 * aborts the player at once or fades out first. */
enum StreamTaskState { STREAMTASK_STATE_SKIPPED = 18 };

/** `result` after a confirm press skipped the stream (TaskCore's timeout
 * sets TASKCORE_RESULT_TIMED_OUT). */
#define STREAMTASK_RESULT_SKIPPED 2

/** setFrameBound's unit in this override: bound * 15 frames, where
 * TaskCore's is TASKCORE_FRAMES_PER_SECOND (20). Read as seconds of 15-frame
 * movie time, as TaskCore's bound is seconds; no caller's value shows it. */
#define STREAMTASK_FRAMES_PER_SECOND 15

/** StreamTask's method table: TaskCore's slots, fourteen of them overridden
 * (see the class documentation), then five setters of its own. The ctor
 * takes a fifth argument, the optional initData rectangle. */
struct StreamTaskMethods {
    TASKCORE_SLOTS(StreamTask, (StreamTask * self, TaskCoreTarget *target, char *soundBankPath,
                                BasicClass *sound, DrawRect *initData));
    /* +0x124 */ void (*setKeepActive)(StreamTask *self, s32 keepActive); /**< @see StreamTask__SetKeepActive */
    /* +0x128 */ void (*setLoopCount)(StreamTask *self, s32 count); /**< @see StreamTask__SetLoopCount */
    /* +0x12C */ void (*setSkipOnConfirm)(StreamTask *self, s32 enable); /**< @see StreamTask__SetSkipOnConfirm; game_application.c passes 0 */
    /* +0x130 */ void (*setUnusedD0)(StreamTask *self, s32 value); /**< @see StreamTask__SetUnusedD0 */
    /* +0x134 */ void (*setAbortBeforeFade)(StreamTask *self, s32 enable); /**< @see StreamTask__SetAbortBeforeFade */
};

/**
 * StreamTask -- class id 0x1130, method table gStreamTaskMethods, a TaskCore
 * subclass (task_core.h). No class derives from it. src/app/stream_task.c
 * holds the whole class: allocator, ctor, every override, the setters and the getter.
 * The object is 0xDC bytes (New_StreamTask): TaskCore's 0xA4, then its own.
 *
 * What it does: it owns a MoviePlayer (`player`, movie_player.h) and runs one
 * "ETC\*.STR" stream through it inside TaskCore's fade/state machine. Every
 * caller is game_application.c's GameApplication (intro logo, opening, special day
 * and cinematic streams): New_StreamTask(NULL, NULL, NULL, NULL), optionally
 * setFrameBound / setSkipOnConfirm(0), then init with the stream, then
 * release. It calls the player's play, advance, abort, setAutoPlay and
 * release slots.
 *
 * Overrides, each named for its slot:
 *  - +0x008 ctor, StreamTask__StreamTask: TaskCore's ctor, this table,
 *    initData copied from the fifth argument or GetDefaultMovieFrame(), the
 *    player, streamName NULL, resetCounters.
 *  - +0x00C finalize, StreamTask__Finalize: releases the player, then
 *    TaskCore's.
 *  - +0x040 resetCounters, StreamTask__Reset: loopCount -1, keepActive 0,
 *    skipOnConfirm 1, unusedD0 0, abortBeforeFade 1. No up-call to
 *    TaskCore__Reset.
 *  - +0x044 init, StreamTask__Init(args, streamName, streamGroup, autoPlay):
 *    stores the three, then TaskCore's init(args, 0). The slot keeps
 *    IntermediateBase's (self, args, mode) type, and the callers, which pass
 *    the extra arguments, cast it to StreamTaskInitFn; the override returns
 *    nothing and no caller reads a result.
 *  - +0x04C onInit, StreamTask__OnInit: TaskCore's, playDone 0, the player's
 *    setAutoPlay(autoPlay) and play(streamName, streamGroup, keepActive,
 *    loopCount); a nonzero play result is setFrameBound(0).
 *  - +0x05C update, StreamTask__Update: TaskCore's; until playDone,
 *    playDone = the player's advance, and done while not fadingOut is
 *    setState(FADE_OUT).
 *  - +0x060 setState, StreamTask__SetState: TaskCore's; ACTIVE clears
 *    fadingOut, FADE_OUT sets it, FADED_OUT aborts the player unless
 *    abortBeforeFade, SKIPPED runs exit.
 *  - +0x06C setFrameBound, StreamTask__SetFrameBound: bound * 15 (TaskCore's
 *    is * 20), negative kept.
 *  - +0x078 onPadConfirm, StreamTask__OnPadConfirm: TaskCore's; with
 *    skipOnConfirm, result STREAMTASK_RESULT_SKIPPED and setState(SKIPPED).
 *  - +0x080 onPadPrev and +0x084 onPadNext: TaskCore's only.
 *  - +0x088, +0x08C: StreamTask__NoOpSlot88/8C (NULL in TaskCore).
 *  - +0x094 exit, StreamTask__Exit: with abortBeforeFade, the player's abort
 *    now; else setState(FADE_OUT) and the abort at FADED_OUT.
 *
 * `initData` is a DrawRect (draw_system.h): the ctor's optional fifth
 * argument, else GetDefaultMovieFrame()'s {x 640, y 0, w 320, h 240}, the
 * rect the ctor also hands New_MoviePlayer as the player's frame and
 * TaskCore__OnInit clears. It is copied whole; no method of this class reads
 * it back. `streamName` is the player's play name argument: the movie's
 * path, from GetAsmkMovie or a CdFileEntry the game_files.h getters return.
 */
struct StreamTask {
    TASKCORE_FIELDS(StreamTaskMethods);
    /* +0x0A4 */ s32 playDone;      /**< OnInit: 0; Update: the player's advance until nonzero */
    /* +0x0A8 */ DrawRect initData; /**< the ctor's fifth argument or the default */
    /* +0x0B4 */ struct MoviePlayer *player; /**< New_MoviePlayer(GetDefaultMovieFrame(), 0, 0); finalize releases it */
    /* +0x0B8 */ const char *streamName; /**< Init's; the player's play name. ctor: NULL */
    /* +0x0BC */ s32 streamGroup; /**< Init's (GetMovieFrameCount, or -1); play's second argument */

    /** Init's (every caller 1); the player's setAutoPlay
     * (MoviePlayer::autoPlay), which play tests to start at once. */
    /* +0x0C0 */ s32 autoPlay;
    /* +0x0C4 */ s32 keepActive; /**< setKeepActive; reset 0; play's keepActive (MoviePlayer::keepActive) */
    /* +0x0C8 */ s32 loopCount; /**< setLoopCount; reset -1; play's fourth argument, the player's `loops` */
    /* +0x0CC */ s32 skipOnConfirm; /**< setSkipOnConfirm; reset 1; OnPadConfirm: nonzero ends the task with result 2 */
    /* +0x0D0 */ s32 unusedD0; /**< setUnusedD0 (never called); reset 0; nothing reads it */
    /* +0x0D4 */ s32 abortBeforeFade; /**< setAbortBeforeFade; reset 1; Exit aborts at once, else after the fade */
    /* +0x0D8 */ s32 fadingOut; /**< SetState: ACTIVE clears, FADE_OUT sets; Update skips its setState(FADE_OUT) when set */
};

/** Slot +0x044's occupant, StreamTask__Init, as its callers call it. */
typedef void (*StreamTaskInitFn)(StreamTask *self, IntermediateBaseInitArgs *args,
                                 const char *streamName, s32 streamGroup, s32 autoPlay);

/** StreamTask's own method table. */
extern StreamTaskMethods gStreamTaskMethods;

/** @brief StreamTask's method-table getter.
 * @return &gStreamTaskMethods */
extern StreamTaskMethods *GetStreamTaskMethods(void);

/** @brief Allocates a StreamTask from the pool and runs its ctor.
 * @param target TaskCore's menu description (every caller NULL)
 * @param soundBankPath TaskCore's sound bank path (every caller NULL)
 * @param sound TaskCore's sound object (every caller NULL)
 * @param initData the frame rectangle to keep, or NULL for the default
 * @return the new task, or NULL when the pool allocation fails */
StreamTask *New_StreamTask(TaskCoreTarget *target, char *soundBankPath, BasicClass *sound,
                           DrawRect *initData);

/** @brief Constructor: TaskCore's, this table, initData, a new MoviePlayer on
 * the default movie frame, no stream, resetCounters.
 * @param self the task
 * @param target TaskCore's menu description
 * @param soundBankPath TaskCore's sound bank path
 * @param sound TaskCore's sound object
 * @param initData the frame rectangle to keep, or NULL for the default */
void StreamTask__StreamTask(StreamTask *self, TaskCoreTarget *target, char *soundBankPath,
                            BasicClass *sound, DrawRect *initData);

/** @brief Finalize: releases the player, then TaskCore's finalize.
 * @param self the task */
void StreamTask__Finalize(StreamTask *self);

/** @brief resetCounters override: loopCount -1, no keep-active, skip on
 * confirm, abort before the fade; TaskCore's reset is not run.
 * @param self the task */
void StreamTask__Reset(StreamTask *self);

/** @brief init override: stores the stream to play, then TaskCore's init in
 * INTERMEDIATEBASE_INIT_RUN, which plays it to the end.
 * @param self the task
 * @param args the objects to work with
 * @param streamName the movie's path
 * @param streamGroup its frame count (GetMovieFrameCount), or -1
 * @param autoPlay nonzero starts playback at once */
void StreamTask__Init(StreamTask *self, IntermediateBaseInitArgs *args, const char *streamName,
                      s32 streamGroup, s32 autoPlay);

/** @brief onInit override: TaskCore's, then starts the player on the stream;
 * a failed start sets the frame bound to 0.
 * @param self the task */
void StreamTask__OnInit(StreamTask *self);

/** @brief update override: TaskCore's, then advances the player until it
 * reports done, and fades out when it does.
 * @param self the task
 * @param sender the FrameClock
 * @param event its event code */
void StreamTask__Update(StreamTask *self, BasicClass *sender, s32 event);

/** @brief setState override: TaskCore's, then tracks the fade-out, aborts
 * the player once faded out (unless it was aborted before the fade), and
 * answers SKIPPED with exit.
 * @param self the task
 * @param state the new state */
void StreamTask__SetState(StreamTask *self, s32 state);

/** @brief Sets the frame bound in STREAMTASK_FRAMES_PER_SECOND units; a
 * negative bound is kept as is.
 * @param self the task
 * @param bound the bound, or negative for none */
void StreamTask__SetFrameBound(StreamTask *self, s32 bound);

/** @brief onPadConfirm override: TaskCore's; with skipOnConfirm, ends the
 * task with STREAMTASK_RESULT_SKIPPED.
 * @param self the task */
void StreamTask__OnPadConfirm(StreamTask *self);

/** @brief onPadPrev override: TaskCore's only.
 * @param self the task */
void StreamTask__OnPadPrev(StreamTask *self);

/** @brief onPadNext override: TaskCore's only.
 * @param self the task */
void StreamTask__OnPadNext(StreamTask *self);

/** @brief Slot +0x088: empty. */
void StreamTask__NoOpSlot88(void);

/** @brief Slot +0x08C: empty. */
void StreamTask__NoOpSlot8C(void);

/** @brief exit override: aborts the player now when abortBeforeFade is set,
 * else fades out first (the abort follows at FADED_OUT).
 * @param self the task */
void StreamTask__Exit(StreamTask *self);

/** @brief Sets the player's keep-active flag for the next play.
 * @param self the task
 * @param keepActive the flag */
void StreamTask__SetKeepActive(StreamTask *self, s32 keepActive);

/** @brief Sets the player's loop count for the next play.
 * @param self the task
 * @param count loops, or -1 */
void StreamTask__SetLoopCount(StreamTask *self, s32 count);

/** @brief Sets whether a confirm press ends the stream.
 * @param self the task
 * @param enable nonzero: confirm skips */
void StreamTask__SetSkipOnConfirm(StreamTask *self, s32 enable);

/** @brief Sets unusedD0, which nothing reads; nothing calls it.
 * @param self the task
 * @param value the value */
void StreamTask__SetUnusedD0(StreamTask *self, s32 value);

/** @brief Sets whether exit aborts the player before the fade-out.
 * @param self the task
 * @param enable nonzero: abort at once */
void StreamTask__SetAbortBeforeFade(StreamTask *self, s32 enable);

#endif
