#ifndef STREAMTASK_H
#define STREAMTASK_H

#include "TaskCore.h"

/*
 * StreamTask -- class id 0x1130, method table gStreamTaskMethods, a TaskCore
 * subclass (`tools/classtable.py gStreamTaskMethods --vs gTaskCoreMethods`:
 * fourteen overrides and five slots of its own). No class derives from it.
 * src/code_2c054.c holds the whole class: allocator, ctor, every override,
 * the setters and the getter. The object is 0xDC bytes (New_StreamTask).
 *
 * The name is the table's stem with its `Obj` dropped (track 4 step 2);
 * include/Class6D3C8.h's view already called it StreamTask. What it does,
 * measured: it owns a MoviePlayer (`player`, New_MoviePlayer, gMoviePlayerMethods)
 * and runs one "ETC\*.STR" stream through it inside TaskCore's fade/state
 * machine. Every caller is code_1677c's Class6D3C8 (intro logo, weekly,
 * GraphRoom and cinematic streams): New_StreamTask(NULL, NULL, NULL, NULL),
 * optionally setFrameBound / setSkipOnConfirm(0), then init with the stream,
 * then release. The player's slots as this class calls them (code_2c054.h's
 * StreamTaskUnkB4Obj view): +0x040 MoviePlayer__Play, +0x048 __Advance,
 * +0x04C __Abort, +0x06C __SetResult.
 *
 * Overrides, each named for its slot:
 *   +0x008 ctor           StreamTask__StreamTask: TaskCore's ctor, this
 *                         table, initData copied from the fifth argument or
 *                         GetDefaultStreamTaskInitData(), the player,
 *                         streamName 0, resetCounters.
 *   +0x00C finalize       StreamTask__Finalize: releases the player, then
 *                         TaskCore's.
 *   +0x040 resetCounters  StreamTask__Reset: loopCount -1, unkC4 0,
 *                         skipOnConfirm 1, unkD0 0, abortBeforeFade 1. No
 *                         up-call to TaskCore__Reset.
 *   +0x044 init           StreamTask__Init(args, streamName, streamGroup,
 *                         autoPlay): stores the three, then TaskCore's
 *                         init(args, 0). See StreamTaskInitFn below.
 *   +0x04C onInit         StreamTask__OnInit: TaskCore's, playDone 0, the
 *                         player's SetResult(autoPlay) and Play(streamName,
 *                         streamGroup, unkC4, loopCount); a nonzero Play is
 *                         setFrameBound(0).
 *   +0x05C update         StreamTask__Update: TaskCore's; until playDone,
 *                         playDone = the player's Advance, and done while
 *                         not fadingOut is setState(7).
 *   +0x060 setState       StreamTask__SetState: TaskCore's; 5 clears
 *                         fadingOut, 7 sets it, 8 aborts the player unless
 *                         abortBeforeFade, 0x12 refreshViewValue.
 *   +0x06C setFrameBound  StreamTask__SetFrameBound: bound * 15 (TaskCore's
 *                         is * 20), negative kept.
 *   +0x078 onPadConfirm   StreamTask__OnPadConfirm: TaskCore's; with
 *                         skipOnConfirm, result 2 and setState(0x12).
 *   +0x080 onPadPrev      StreamTask__OnPadPrev: TaskCore's only.
 *   +0x084 onPadNext      StreamTask__OnPadNext: TaskCore's only.
 *   +0x088, +0x08C        StreamTask__NoOpSlot88/8C (NULL in TaskCore).
 *   +0x094 refreshViewValue StreamTask__RefreshViewValue: abortBeforeFade,
 *                         the player's Abort now; else setState(7) and the
 *                         abort at state 8.
 * The overrides of +0x04C/+0x080/+0x084 (and TaskCore's) take self alone;
 * the up-call to onInit casts the slot, as TaskCore.h's banner says.
 *
 * +0x044's contradiction, settled by track 4 step 6 (round 85's Class65650
 * +0x04C rule): StreamTask__Init takes (self, args, streamName, streamGroup,
 * autoPlay) where IntermediateBase's init takes (self, args, mode) and
 * returns s32. The table keeps the inherited slot type, and code_1677c's
 * five callers, which forward the extra arguments, cast the slot to
 * StreamTaskInitFn (a pointer cast, no code). The override returns nothing
 * (no caller reads it), as the round-84 `configure` view had it.
 *
 * `streamName` is MoviePlayer__Play's name argument (`char *` there); it is
 * s32 here because four of the five callers pass a helper's s32 result
 * (only the intro logo passes a string), and retyping those helpers is not
 * this class's job.
 */

typedef struct StreamTask StreamTask;
typedef struct StreamTaskMethods StreamTaskMethods;
typedef struct StreamTaskInitData StreamTaskInitData;

/* Three words: the ctor's optional fifth (stack) argument, else
 * GetDefaultStreamTaskInitData()'s default (&gDefaultStreamTaskInitData,
 * which TaskCore__OnInit also passes). Copied whole into `initData`
 * (retail loads all three words before storing any: a struct assignment);
 * no method of this class reads it back. */
struct StreamTaskInitData {
    /* +0x000 */ s32 unk0;
    /* +0x004 */ s32 unk4;
    /* +0x008 */ s32 unk8;
};

/* The ctor takes FIVE parameters: New_StreamTask dispatches it with
 * $a0-$a3 plus a fifth stored to 0x10($sp), the o32 stack-argument slot
 * (docs/match-reports/New_StreamTask.md). */
struct StreamTaskMethods {
    TASKCORE_SLOTS(StreamTask, (StreamTask *self, TaskCoreTarget *target, char *soundBankPath, BasicClass *sound, StreamTaskInitData *initData));
    /* +0x124 */ void (*setUnkC4)(StreamTask *self, s32 value);           /* StreamTask__SetUnkC4 */
    /* +0x128 */ void (*setLoopCount)(StreamTask *self, s32 count);       /* StreamTask__SetLoopCount */
    /* +0x12C */ void (*setSkipOnConfirm)(StreamTask *self, s32 enable);  /* StreamTask__SetSkipOnConfirm; code_1677c passes 0 */
    /* +0x130 */ void (*setUnkD0)(StreamTask *self, s32 value);           /* StreamTask__SetUnkD0 */
    /* +0x134 */ void (*setAbortBeforeFade)(StreamTask *self, s32 enable); /* StreamTask__SetAbortBeforeFade */
};

/* TaskCore's 0xA4 bytes, then this class's own to 0xDC (New_StreamTask). */
struct StreamTask {
    TASKCORE_FIELDS(StreamTaskMethods);
    /* +0x0A4 */ s32 playDone;          /* OnInit: 0; Update: the player's Advance until nonzero */
    /* +0x0A8 */ StreamTaskInitData initData; /* the ctor's fifth argument or the default */
    /* +0x0B4 */ BasicClass *player;    /* New_MoviePlayer(GetDefaultStreamTaskInitData(), 0, 0); finalize releases it */
    /* +0x0B8 */ s32 streamName;        /* Init's; the player's Play name. ctor: 0 */
    /* +0x0BC */ s32 streamGroup;       /* Init's (GetStreamGroupForType, or -1); Play's second argument */
    /* +0x0C0 */ s32 autoPlay;          /* Init's (every caller 1); the player's SetResult, its +0x068, which Play tests to MarkPlaying at once */
    /* +0x0C4 */ s32 unkC4;             /* setUnkC4; reset 0; Play's third argument (the player's +0x054) */
    /* +0x0C8 */ s32 loopCount;         /* setLoopCount; reset -1; Play's fourth argument, the player's `loops` (MoviePlayer__Advance) */
    /* +0x0CC */ s32 skipOnConfirm;     /* setSkipOnConfirm; reset 1; OnPadConfirm: nonzero ends the task with result 2 */
    /* +0x0D0 */ s32 unkD0;             /* setUnkD0; reset 0; no method of this class reads it */
    /* +0x0D4 */ s32 abortBeforeFade;   /* setAbortBeforeFade; reset 1; RefreshViewValue aborts at once, else SetState(8) after the fade */
    /* +0x0D8 */ s32 fadingOut;         /* SetState: 5 clears, 7 sets; Update skips its setState(7) when set */
};

/* +0x044's occupant as its callers need it (see the banner). */
typedef void (*StreamTaskInitFn)(StreamTask *self, IntermediateBaseInitArgs *args, s32 streamName, s32 streamGroup, s32 autoPlay);

extern StreamTaskMethods gStreamTaskMethods;
extern StreamTaskMethods *Get_vtable_StreamTask(void); /* returns &gStreamTaskMethods */

StreamTask *New_StreamTask(TaskCoreTarget *target, char *soundBankPath, BasicClass *sound, StreamTaskInitData *initData);
void StreamTask__StreamTask(StreamTask *self, TaskCoreTarget *target, char *soundBankPath, BasicClass *sound, StreamTaskInitData *initData);
void StreamTask__Finalize(StreamTask *self);
void StreamTask__Reset(StreamTask *self);
void StreamTask__Init(StreamTask *self, IntermediateBaseInitArgs *args, s32 streamName, s32 streamGroup, s32 autoPlay);
void StreamTask__OnInit(StreamTask *self);
void StreamTask__Update(StreamTask *self, BasicClass *sender, s32 event);
void StreamTask__SetState(StreamTask *self, s32 state);
void StreamTask__SetFrameBound(StreamTask *self, s32 bound);
void StreamTask__OnPadConfirm(StreamTask *self);
void StreamTask__OnPadPrev(StreamTask *self);
void StreamTask__OnPadNext(StreamTask *self);
void StreamTask__NoOpSlot88(void);
void StreamTask__NoOpSlot8C(void);
void StreamTask__RefreshViewValue(StreamTask *self);
void StreamTask__SetUnkC4(StreamTask *self, s32 value);
void StreamTask__SetLoopCount(StreamTask *self, s32 count);
void StreamTask__SetSkipOnConfirm(StreamTask *self, s32 enable);
void StreamTask__SetUnkD0(StreamTask *self, s32 value);
void StreamTask__SetAbortBeforeFade(StreamTask *self, s32 enable);

#endif
