/*
 * code_1677c -- GameApplication (include/GameApplication.h), the game's
 * Application subclass: its allocator and ctor, the RNG seed and initSystems
 * overrides, and the six hooks Application's main loop calls -- the intro
 * logos, the weekly stream, the GraphRoom poll, the DayTask run and the
 * streams that follow it -- with the loader- and poll-task helpers they share.
 * The table getter is in src/code_171e0.c.
 */
#include "common.h"
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include "GameApplication.h"
#include "DreamSys.h"
#include "LinkResource.h"
#include "TaskCore.h"
#include "StreamTask.h"
#include "GraphRoom.h"
#include "TitleMenu.h"
#include "DayTask.h"

/* The game's allocator, in the uncarved BMemPMgr block. Returns void *
 * rather than a typed pointer because every New_X in the game calls it. */
extern void *BMemPMgrAlloc(s32 size);

extern const char sModelPathDreamE5[]; /* "ETC\DREAME5.TMD", asm/data/FA4.rodata.s */

extern char *GetDefaultDataDirectory(void); /* code_39094.c: "CDI\\" */
extern void SetDataDirectory(char *dir);    /* code_171e0.c */

extern s32 SetActiveDataSourceDriverMode(s32 a0, s32 a1, s32 a2); /* code_171e0, still INCLUDE_ASM there; returns
                                                       the last value its internal dispatch loop got --
                                                       GameApplication__LoadIntroLogoSequence/GameApplication__StartWeeklyStreamTask discard it, but
                                                       GameApplication__StartCinematicStream keeps it */
extern const char *GetAsmkMovie(s32 *typeCodeOut); /* psyq_memset.s: writes 0x31 to *typeCodeOut if non-NULL, always returns &sAsmkMoviePath */
extern s32 GetMovieFrameCount(s32 index); /* psyq_memset.s: signed-halfword lookup into gMovieFrameCounts[index] */
extern s32 PickOpeningMovie(s32 *out, s32 param2); /* psyq_memset.s: day/week-style calculation (divides SeedAndRandom's result by 7); writes a related index to *out if non-NULL, returns a separate derived value */

extern const char sLogoPathAsmk[]; /* "ETC\ASMKLOGO.TIM" */
extern const char sLogoPathOsd[];  /* "ETC\OSDLOGO.TIM" */

/* The PollTasks GameApplication__RunPollTask runs (New_GraphRoom, New_TitleMenu)
 * are TaskCore-family classes; this is this unit's own minimal view of
 * them: +0x004 is BasicClass's release, +0x044 IntermediateBase's init.
 * Constructed directly by a caller-supplied function pointer
 * (GameApplication__RunPollTask's own a0) rather than a New_X-style allocator. */
typedef struct PollTaskMethods {
    s32 header;                                                              /* +0x000 */
    void (*slot4)(void *self);                                               /* +0x004 */
    u8 pad08[0x044 - 0x008];                                                 /* +0x008 .. +0x043 */
    s32 (*slot44)(void *self, IntermediateBaseInitArgs *initArgs, s32 mode); /* +0x044 */
} PollTaskMethods;

typedef struct PollTask {
    PollTaskMethods *methods;
} PollTask;

typedef PollTask *(*PollTaskCtor)(void *arg);

/* This unit's own function, defined later in ROM order (forward declared
 * for GameApplication__PollGraphRoomStatus, which comes first). Constructs a PollTask via the
 * caller-supplied `ctor`, dispatches slot44(task, initArgs, 0) and slot4(task)
 * on it, and returns slot44's result. */
s32 GameApplication__RunPollTask(PollTaskCtor ctor, void *dreamSys, IntermediateBaseInitArgs *initArgs);

/* PollTask constructors (not this unit's to write). Called directly (not
 * through any vtable) as GameApplication__RunPollTask's `ctor` argument.
 * New_GraphRoom is include/GraphRoom.h's and New_TitleMenu
 * include/TitleMenu.h's, each cast to PollTaskCtor. */

extern s32 GetSpecialDayMovieSpan(s32 *out, s32 a1, s32 a2); /* psyq_memset.s: writes a derived count to *out, returns a separate derived value */
/* code_39094.c: same "write to *out, return a separate value" shape as
 * GetAsmkMovie/PickOpeningMovie/GetSpecialDayMovieSpan. */
extern s32 GetEndingMovie(s32 *out, s32 unused); /* arity-ok: the definition is 1-parameter and reads only $a0 (it neither reads nor forwards $a1), but the 2nd argument IS byte-load-bearing -- retail emits `move a1,zero` in the jal's delay slot at 0x80026974 */
extern s32 GetSpecialDayOrEventRecord(s32 *out, s32 packedBankEntry); /* psyq_memset.s: resolves a packed
    {bank; entry} CinematicCall (low 16 bits = bank, high 16 = entry) to a channel index written
    to *out (-1 if unresolved); the packing must zero-extend both halves before combining
    (retail loads them with lhu, not lh) since the result is bitwise-composed, not a value read
    back as a signed 32-bit number. Also returns its own (separate) s32 value, kept by
    GameApplication__StartCinematicStream. */

/* The `New_X` allocator for the class whose method table is gGameApplicationMethods:
 * allocates a 0x2C-byte instance and, on success, runs the class's own
 * constructor through slot +0x008 of the table GetGameApplicationMethods() returns.
 *
 * The null path deliberately falls off the end rather than returning a
 * value. That is not an oversight in the transcription -- it is what
 * retail does, and it is the ONLY form that matches. `BMemPMgrAlloc`
 * already left the null in $v0, so the original source never had to
 * restate it; every spelling that returns explicitly on that path
 * (`return 0`, `return self`, an early return, a goto to a shared exit)
 * costs an extra instruction that retail does not have. GCC 2.6.3 warns
 * "control reaches end of non-void function" here, and the warning is
 * correct about the C -- the bytes are what say the original had it too.
 * See docs/match-reports/New_GameApplication.md for the full derivation. */
GameApplication *New_GameApplication(GameApplicationConfig *arg) {
    GameApplication *self = BMemPMgrAlloc(0x2C);

    if (self != 0) {
        GetGameApplicationMethods()->ctor(self, arg);
        return self;
    }
}

/* Constructs a GameApplication instance: runs the intermediate base class's own
 * constructor (through its ctor slot), installs this class's own vtable,
 * stores the ctor argument, loads the "ETC\DREAME5.TMD" model, builds this
 * object's owned DreamSys from it, dispatches one DreamSys init call, then
 * runs this class's own slot40 (GameApplication__SeedRandom) once. */
void GameApplication__GameApplication(GameApplication *self, GameApplicationConfig *arg) {
    /* MATCHING: mode is never set, but a bare ResourceSource shrinks the
     * frame by 8. */
    ResourceRequest req;

    GetApplicationMethods()->ctor((Application *)self, arg->dataSource);
    self->methods = GetGameApplicationMethods();
    self->config = arg;
    SetDataDirectory(GetDefaultDataDirectory());
    req.src.buffer = NULL;
    req.src.name = (char *)sModelPathDreamE5;
    self->dreamSys = New_DreamSys(New_LinkResource(&req.src), 0, 0);
    self->skipGraphRoomPoll = 0;
    self->dreamSys->methods->slot228(self->dreamSys, arg->unk14);
    ((GameApplicationSeedRandomFn)self->methods->setScreenDims)(self);
}

extern void SeedAndRandom(s32 day, s32 unused);

/* Seeds the C library's random generator from the scratchpad word at
 * 0x1F800000 (the PS-X data-cache-as-RAM region) reduced mod 365; the
 * random value SeedAndRandom returns is discarded. */
void GameApplication__SeedRandom(GameApplication *self) {
    SeedAndRandom(*(s32 *)0x1F800000 % 365, 0);
}

/* initSystems (+0x044): runs Application's own initSystems unless the parent's
 * `initialized` latch is already set. */
void GameApplication__InitSystems(GameApplication *self, DrawSystem *drawSystem, struct Pad *pad) {
    if (self->initialized == 0) {
        GetApplicationMethods()->initSystems((Application *)self, drawSystem, pad, 0);
    }
}

/* Optional stream-load block, gated by self->config->showIntroLogos: registers a
 * "loader" task for "ETC\ASMKLOGO.TIM" (GameApplication__StartLoaderTask), then a separate
 * "stream" task for whatever type code GetAsmkMovie hands back
 * ("ETC\ASMK.STR"), then a second loader task for "ETC\OSDLOGO.TIM". */
void GameApplication__LoadIntroLogoSequence(GameApplication *self) {
    const char *streamName;
    s32 typeCode;
    s32 typeLookup;
    StreamTask *task;

    if (self->config->showIntroLogos != 0) {
        SetActiveDataSourceDriverMode(0, 0, 0);
        GameApplication__StartLoaderTask(self, sLogoPathAsmk);
        task = New_StreamTask(0, 0, 0, 0);
        streamName = GetAsmkMovie(&typeCode);
        typeLookup = GetMovieFrameCount(typeCode);
        ((StreamTaskInitFn)task->methods->init)(task, (IntermediateBaseInitArgs *)self->aux,
                                                streamName, typeLookup, 1);
        task->methods->release(task);
        GameApplication__StartLoaderTask(self, sLogoPathOsd);
    }
}

/* Registers a "loader" task for the given resource path: allocates the
 * task, gives it a completion callback (GameApplication__LoaderTaskDoneCallback) and context
 * (self), then sets its remaining parameters (path, the parent's aux as its init args) and
 * starts it. */
void GameApplication__StartLoaderTask(GameApplication *self, const char *path) {
    TaskCore *task = New_TaskCore(0, 0, 0);

    /* setCallback's occupant stores a void (*)(void *ctx); this callback
     * takes nothing and returns RegisterRecordTableFiles's value, which
     * TaskCore__RefreshViewValue ignores. */
    task->methods->setCallback(task, (void (*)(void *))GameApplication__LoaderTaskDoneCallback, self);
    task->methods->setFrameBound(task, 0);
    task->methods->setSubHandle(task, path, 0);
    task->methods->init(task, (IntermediateBaseInitArgs *)self->aux, 0);
    task->methods->release(task);
}

extern s32 RegisterRecordTableFiles(s32 a0);

s32 GameApplication__LoaderTaskDoneCallback(void) {
    return RegisterRecordTableFiles(0);
}

/* Optional stream-task init block, gated by self->config->playStreams (the same
 * shape as GameApplication__LoadIntroLogoSequence's self->config->showIntroLogos gate, minus the two
 * GameApplication__StartLoaderTask loader-task calls, and using PickOpeningMovie instead of
 * GetAsmkMovie to derive the type code). */
void GameApplication__StartWeeklyStreamTask(GameApplication *self) {
    s32 derivedValue;
    s32 typeCode;
    s32 typeLookup;
    StreamTask *task;

    if (self->config->playStreams != 0) {
        SetActiveDataSourceDriverMode(0, 0, 0);
        task = New_StreamTask(0, 0, 0, 0);
        derivedValue = PickOpeningMovie(&typeCode, 0);
        typeLookup = GetMovieFrameCount(typeCode);
        ((StreamTaskInitFn)task->methods->init)(task, (IntermediateBaseInitArgs *)self->aux,
                                                derivedValue, typeLookup, 1);
        task->methods->release(task);
    }
}

/* Gated by self->config->pollGraphRoom. Checks the DreamSys's own status slot
 * (+0x1A0); if it isn't already "1" and self->skipGraphRoomPoll hasn't latched, kicks
 * off one PollTask (New_GraphRoom) and, if THAT reports "2", runs
 * GameApplication__StartGraphRoomStreamTask. Then polls a second PollTask (New_TitleMenu) in a loop,
 * restarting the first PollTask each time it reports "2", until it
 * reports anything else; clears self->skipGraphRoomPoll and returns 0 or 2 depending
 * on whether that final status was below 1. */
s32 GameApplication__PollGraphRoomStatus(GameApplication *self) {
    s32 status;
    s32 pollDone;

    if (self->config->pollGraphRoom != 0) {
        SetActiveDataSourceDriverMode(0, 0, 0);

        status = self->dreamSys->methods->getCurrentDayAndYear(self->dreamSys, 0);
        if (status != 1) {
            if (self->skipGraphRoomPoll == 0) {
                status = GameApplication__RunPollTask((PollTaskCtor)New_GraphRoom, self->dreamSys,
                                                      (IntermediateBaseInitArgs *)self->aux);
                if (status == 2) {
                    GameApplication__StartGraphRoomStreamTask(self);
                }
            }
        }

        pollDone = 2;
    retry:
        status = GameApplication__RunPollTask((PollTaskCtor)New_TitleMenu, self->dreamSys,
                                              (IntermediateBaseInitArgs *)self->aux);
        if (status == pollDone) {
            GameApplication__RunPollTask((PollTaskCtor)New_GraphRoom, self->dreamSys,
                                         (IntermediateBaseInitArgs *)self->aux);
            goto retry;
        }

        self->skipGraphRoomPoll = 0;
        return ((u32)status < 1) << 1;
    }
    return 2;
}

/* Constructs a PollTask via the caller-supplied `ctor`, dispatches
 * slot44(task, initArgs, 0) and slot4(task) on it (fire-and-forget), and
 * returns slot44's result. */
s32 GameApplication__RunPollTask(PollTaskCtor ctor, void *dreamSys, IntermediateBaseInitArgs *initArgs) {
    PollTask *task = ctor(dreamSys);
    s32 result = task->methods->slot44(task, initArgs, 0);

    task->methods->slot4(task);
    return result;
}

/* Called by GameApplication__PollGraphRoomStatus when its first PollTask reports "2". Gated by
 * self->config->playStreams (same gate as GameApplication__StartWeeklyStreamTask). Builds a StreamTask,
 * derives a count via GetSpecialDayMovieSpan, sets the task's frame bound
 * to that count / 15 and clears skipOnConfirm, then runs its init (stream
 * group -1, unlike the other call sites) and releases it. */
void GameApplication__StartGraphRoomStreamTask(GameApplication *self) {
    StreamTask *task;

    struct {
        u32 unk00;
        u32 unk04;
        u32 count;
    } buf;

    s32 extra;

    if (self->config->playStreams != 0) {
        SetActiveDataSourceDriverMode(0, 0, 0);
        task = New_StreamTask(0, 0, 0, 0);
        extra = GetSpecialDayMovieSpan(&buf.count, 0, 10);
        task->methods->setFrameBound(task, buf.count / 15);
        task->methods->setSkipOnConfirm(task, 0);
        ((StreamTaskInitFn)task->methods->init)(task, (IntermediateBaseInitArgs *)self->aux, extra,
                                                -1, 1);
        task->methods->release(task);
    }
}

void GameApplication__NoOpSlot5C(void) {}

/* Builds a DayTask (include/DayTask.h), runs its init with self
 * alone (DayTask__Init takes nothing else, hence DayTaskInitFn) and
 * releases it; init's return, TimedTask::result, is a status code: 2 runs GameApplication__StartCinematicStream, 3 latches self->skipGraphRoomPoll.
 * Then queries the DreamSys status slot again (as GameApplication__PollGraphRoomStatus does),
 * this time passing an out-param, and derives a 0/1 result from both the
 * call's return and the out-param. */
/* Builds a DayTask for this instance's current state, reads one status
 * code off it, tears it down, and reacts to two of the codes. Then asks the
 * owned DreamSys a question and reports whether its answer was 1.
 *
 * Two things here were long-standing misreadings, both worth keeping written
 * down (docs/match-reports/GameApplication__PollStatusObj.md):
 *
 *  - `case 3` stores 1, NOT 3. Retail's `li $v0, 0x1` sits in the delay slot
 *    of the case-3 branch, so it executes before the jump is taken and $v0
 *    holds 1 -- not the 3 it held for the comparison -- by the time the
 *    store runs. Reading the store as `unk24 = 3` (the discriminant) was
 *    what produced the old 53/57 and the "the compiler materialises an
 *    unused default-arm constant" theory attached to it. There is no unused
 *    constant: `li $v0, 0x1` is the value being stored, hoisted into a delay
 *    slot on the only path that needs it.
 *  - `result = (check == 1)` is the whole comparison. GCC 2.6.3 lowers an
 *    equality test against a small constant to `xori` + `sltiu`, which reads
 *    back out of the disassembly as `(u32)(check ^ 1) < 1`. That transcription
 *    is arithmetically right and cost two instructions; the plain `== 1` is
 *    what the source said. */
s32 GameApplication__PollStatusObj(GameApplication *self) {
    s32 status;
    DayTask *obj;
    s32 outVal;
    s32 check;
    s32 result;

    obj = New_DayTask((IntermediateBaseInitArgs *)self->aux, self->dreamSys, self->config->unk04);
    status = ((DayTaskInitFn)obj->methods->init)(obj);
    obj->methods->release(obj);

    switch (status) {
        case DAYTASK_RESULT_CINEMATIC:
            GameApplication__StartCinematicStream(self);
            break;
        case DAYTASK_RESULT_CLOSED:
            self->skipGraphRoomPoll = 1;
            break;
    }

    check = self->dreamSys->methods->getCurrentDayAndYear(self->dreamSys, &outVal);
    result = 0;
    if (outVal != 0) {
        result = (check == 1);
    }
    return result;
}

/* Reads DreamSys's current cinematic slot, resolves it to a channel index
 * (GetSpecialDayOrEventRecord); if that fails (-1), starts a LoaderTask on the fixed
 * "no cinematic" path; otherwise, if self->config->playStreams gates it, starts a
 * StreamTask on the resolved channel. Either branch finishes by starting
 * whichever task it built; if neither branch runs, nothing happens. */
void GameApplication__StartCinematicStream(GameApplication *self) {
    CinematicCall cc;

    struct {
        s32 chan;
        u32 unk04;
        u32 unk08;
    } chanBuf;

    s32 groupId;
    s32 lookup;
    TaskCore *task;

    cc = self->dreamSys->methods->getCinematic(self->dreamSys);
    groupId = GetSpecialDayOrEventRecord(&chanBuf.chan, (u16)cc.bank | ((u32)(u16)cc.entry << 16));
    SetActiveDataSourceDriverMode(0, 0, 0);

    if (chanBuf.chan != -1) {
        if (self->config->playStreams != 0) {
            StreamTask *streamTask = New_StreamTask(0, 0, 0, 0);

            streamTask->methods->setSkipOnConfirm(streamTask, 0);
            lookup = GetMovieFrameCount(chanBuf.chan);
            ((StreamTaskInitFn)streamTask->methods->init)(
                streamTask, (IntermediateBaseInitArgs *)self->aux, groupId, lookup, 1);
            streamTask->methods->release(streamTask);
        }
    } else {
        task = New_TaskCore(0, 0, 0);
        task->methods->setFrameBound(task, 10);
        task->methods->setSubHandle(task, groupId, 0);
        task->methods->init(task, (IntermediateBaseInitArgs *)self->aux, 0);
        task->methods->release(task);
    }
}

/* GameApplicationMethods slot +0x064. Gated by self->config->playStreams (same gate as
 * GameApplication__StartWeeklyStreamTask/GameApplication__StartGraphRoomStreamTask). Builds a StreamTask, clears its skipOnConfirm,
 * derives a type code via GetEndingMovie, looks it up via GetMovieFrameCount,
 * initializes the task with it, then starts it -- the same shape as
 * GameApplication__LoadIntroLogoSequence/GameApplication__StartWeeklyStreamTask, but with setSkipOnConfirm(0) added and GetEndingMovie
 * in place of GetAsmkMovie/PickOpeningMovie. */
void GameApplication__StartStreamTaskWithInit(GameApplication *self) {
    StreamTask *task;
    s32 typeCode;
    s32 outerValue;
    s32 typeLookup;

    if (self->config->playStreams != 0) {
        SetActiveDataSourceDriverMode(0, 0, 0);
        task = New_StreamTask(0, 0, 0, 0);
        task->methods->setSkipOnConfirm(task, 0);
        outerValue = GetEndingMovie(&typeCode, 0);
        typeLookup = GetMovieFrameCount(typeCode);
        ((StreamTaskInitFn)task->methods->init)(task, (IntermediateBaseInitArgs *)self->aux,
                                                outerValue, typeLookup, 1);
        task->methods->release(task);
    }
}
