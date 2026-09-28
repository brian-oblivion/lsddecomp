#ifndef STREAM_TASK_H
#define STREAM_TASK_H

#include "TaskCore.h"
#include "draw_system.h"

/*
 * StreamTask -- class id 0x1130, method table gStreamTaskMethods, a TaskCore
 * subclass (`tools/classtable.py gStreamTaskMethods --vs gTaskCoreMethods`:
 * fourteen overrides and five slots of its own). No class derives from it.
 * src/app/task.c holds the whole class: allocator, ctor, every override,
 * the setters and the getter. The object is 0xDC bytes (New_StreamTask).
 *
 * What it does: it owns a MoviePlayer (`player`, New_MoviePlayer, gMoviePlayerMethods)
 * and runs one "ETC\*.STR" stream through it inside TaskCore's fade/state
 * machine. Every caller is game_shell's GameApplication (intro logo, weekly,
 * GraphRoom and cinematic streams): New_StreamTask(NULL, NULL, NULL, NULL),
 * optionally setFrameBound / setSkipOnConfirm(0), then init with the stream,
 * then release. The player's slots as this class calls them
 * (include/movie_player.h):
 * +0x040 play, +0x048 advance, +0x04C abort, +0x06C setAutoPlay, and
 * +0x004 release.
 *
 * Overrides, each named for its slot:
 *   +0x008 ctor           StreamTask__StreamTask: TaskCore's ctor, this
 *                         table, initData (a DrawRect) copied from the
 *                         fifth argument or GetDefaultMovieFrame(), the player,
 *                         streamName NULL, resetCounters.
 *   +0x00C finalize       StreamTask__Finalize: releases the player, then
 *                         TaskCore's.
 *   +0x040 resetCounters  StreamTask__Reset: loopCount -1, keepActive 0,
 *                         skipOnConfirm 1, unkD0 0, abortBeforeFade 1. No
 *                         up-call to TaskCore__Reset.
 *   +0x044 init           StreamTask__Init(args, streamName, streamGroup,
 *                         autoPlay): stores the three, then TaskCore's
 *                         init(args, 0). See StreamTaskInitFn below.
 *   +0x04C onInit         StreamTask__OnInit: TaskCore's, playDone 0, the
 *                         player's setAutoPlay(autoPlay) and Play(streamName,
 *                         streamGroup, keepActive, loopCount); a nonzero Play is
 *                         setFrameBound(0).
 *   +0x05C update         StreamTask__Update: TaskCore's; until playDone,
 *                         playDone = the player's Advance, and done while
 *                         not fadingOut is setState(7).
 *   +0x060 setState       StreamTask__SetState: TaskCore's; 5 clears
 *                         fadingOut, 7 sets it, 8 aborts the player unless
 *                         abortBeforeFade, 0x12 exit.
 *   +0x06C setFrameBound  StreamTask__SetFrameBound: bound * 15 (TaskCore's
 *                         is * 20), negative kept.
 *   +0x078 onPadConfirm   StreamTask__OnPadConfirm: TaskCore's; with
 *                         skipOnConfirm, result 2 and setState(0x12).
 *   +0x080 onPadPrev      StreamTask__OnPadPrev: TaskCore's only.
 *   +0x084 onPadNext      StreamTask__OnPadNext: TaskCore's only.
 *   +0x088, +0x08C        StreamTask__NoOpSlot88/8C (NULL in TaskCore).
 *   +0x094 exit StreamTask__Exit: abortBeforeFade,
 *                         the player's Abort now; else setState(7) and the
 *                         abort at state 8.
 * The overrides of +0x04C/+0x080/+0x084 (and TaskCore's) take self alone;
 * the up-call to onInit casts the slot, as TaskCore.h's banner says.
 *
 * +0x044: StreamTask__Init takes (self, args, streamName, streamGroup,
 * autoPlay) where IntermediateBase's init takes (self, args, mode) and
 * returns s32. The table keeps the inherited slot type, and game_shell's
 * five callers, which forward the extra arguments, cast the slot to
 * StreamTaskInitFn (a pointer cast, no code). The override returns nothing
 * (no caller reads it).
 *
 * `streamName` is MoviePlayer__Play's name argument (`char *` there): the
 * movie's path, from GetAsmkMovie or a CdFileEntry the game_files.h
 * getters return.
 */

typedef struct StreamTask StreamTask;
typedef struct StreamTaskMethods StreamTaskMethods;

/* StreamTask's own state, past TaskCore's (enum TaskCoreState): onPadConfirm
 * sets it when skipOnConfirm is on, and setState answers it with
 * exit, which aborts the player at once or fades out first. */
enum StreamTaskState { STREAMTASK_STATE_SKIPPED = 18 };

/* `result` after a confirm press skipped the stream (TaskCore's timeout
 * sets 1). */
#define STREAMTASK_RESULT_SKIPPED 2

/* setFrameBound's unit in this override: bound * 15 frames, where
 * TaskCore's is TASKCORE_FRAMES_PER_SECOND (20). Read as seconds of 15-frame
 * movie time, as TaskCore's bound is seconds; no caller's value shows it. */
#define STREAMTASK_FRAMES_PER_SECOND 15

/* `initData` is a DrawRect (include/draw_system.h): the ctor's optional fifth
 * (stack) argument, else GetDefaultMovieFrame()'s &sDefaultMovieFrame,
 * {x 640, y 0, w 320, h 240}, the rect the ctor also hands New_MoviePlayer
 * as the player's frame and TaskCore__OnInit clears. Copied whole (retail
 * loads all three words before storing any: a struct assignment); no method
 * of this class reads it back. */

/* The ctor takes FIVE parameters: New_StreamTask dispatches it with
 * $a0-$a3 plus a fifth stored to 0x10($sp), the o32 stack-argument slot
 * (docs/match-reports/New_StreamTask.md). */
struct StreamTaskMethods {
    TASKCORE_SLOTS(StreamTask, (StreamTask * self, TaskCoreTarget *target, char *soundBankPath,
                                BasicClass *sound, DrawRect *initData));
    /* +0x124 */ void (*setKeepActive)(StreamTask *self, s32 keepActive); /* StreamTask__SetKeepActive */
    /* +0x128 */ void (*setLoopCount)(StreamTask *self, s32 count); /* StreamTask__SetLoopCount */
    /* +0x12C */ void (*setSkipOnConfirm)(StreamTask *self, s32 enable); /* StreamTask__SetSkipOnConfirm; game_shell passes 0 */
    /* +0x130 */ void (*setUnkD0)(StreamTask *self, s32 value); /* StreamTask__SetUnkD0 */
    /* +0x134 */ void (*setAbortBeforeFade)(StreamTask *self, s32 enable); /* StreamTask__SetAbortBeforeFade */
};

/* TaskCore's 0xA4 bytes, then this class's own to 0xDC (New_StreamTask). */
struct StreamTask {
    TASKCORE_FIELDS(StreamTaskMethods);
    /* +0x0A4 */ s32 playDone;      /* OnInit: 0; Update: the player's Advance until nonzero */
    /* +0x0A8 */ DrawRect initData; /* the ctor's fifth argument or the default */
    /* +0x0B4 */ struct MoviePlayer *player; /* New_MoviePlayer(GetDefaultMovieFrame(), 0, 0); finalize releases it */
    /* +0x0B8 */ const char *streamName; /* Init's; the player's Play name. ctor: NULL */
    /* +0x0BC */ s32 streamGroup; /* Init's (GetMovieFrameCount, or -1); Play's second argument */
    /* +0x0C0 */ s32 autoPlay; /* Init's (every caller 1); the player's setAutoPlay (MoviePlayer::autoPlay, +0x068), which Play tests to RequestStart at once */
    /* +0x0C4 */ s32 keepActive; /* setKeepActive; reset 0; Play's keepActive (MoviePlayer::keepActive, +0x054) */
    /* +0x0C8 */ s32 loopCount; /* setLoopCount; reset -1; Play's fourth argument, the player's `loops` (MoviePlayer__Advance) */
    /* +0x0CC */ s32 skipOnConfirm; /* setSkipOnConfirm; reset 1; OnPadConfirm: nonzero ends the task with result 2 */
    /* +0x0D0 */ s32 unkD0; /* setUnkD0; reset 0; no method of this class reads it */
    /* +0x0D4 */ s32 abortBeforeFade; /* setAbortBeforeFade; reset 1; Exit aborts at once, else SetState(8) after the fade */
    /* +0x0D8 */ s32 fadingOut; /* SetState: 5 clears, 7 sets; Update skips its setState(7) when set */
};

/* +0x044's occupant as its callers need it (see the banner). */
typedef void (*StreamTaskInitFn)(StreamTask *self, IntermediateBaseInitArgs *args,
                                 const char *streamName, s32 streamGroup, s32 autoPlay);

extern StreamTaskMethods gStreamTaskMethods;
extern StreamTaskMethods *GetStreamTaskMethods(void); /* returns &gStreamTaskMethods */

StreamTask *New_StreamTask(TaskCoreTarget *target, char *soundBankPath, BasicClass *sound,
                           DrawRect *initData);
void StreamTask__StreamTask(StreamTask *self, TaskCoreTarget *target, char *soundBankPath,
                            BasicClass *sound, DrawRect *initData);
void StreamTask__Finalize(StreamTask *self);
void StreamTask__Reset(StreamTask *self);
void StreamTask__Init(StreamTask *self, IntermediateBaseInitArgs *args, const char *streamName,
                      s32 streamGroup, s32 autoPlay);
void StreamTask__OnInit(StreamTask *self);
void StreamTask__Update(StreamTask *self, BasicClass *sender, s32 event);
void StreamTask__SetState(StreamTask *self, s32 state);
void StreamTask__SetFrameBound(StreamTask *self, s32 bound);
void StreamTask__OnPadConfirm(StreamTask *self);
void StreamTask__OnPadPrev(StreamTask *self);
void StreamTask__OnPadNext(StreamTask *self);
void StreamTask__NoOpSlot88(void);
void StreamTask__NoOpSlot8C(void);
void StreamTask__Exit(StreamTask *self);
void StreamTask__SetKeepActive(StreamTask *self, s32 keepActive);
void StreamTask__SetLoopCount(StreamTask *self, s32 count);
void StreamTask__SetSkipOnConfirm(StreamTask *self, s32 enable);
void StreamTask__SetUnkD0(StreamTask *self, s32 value);
void StreamTask__SetAbortBeforeFade(StreamTask *self, s32 enable);

#endif
