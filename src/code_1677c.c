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

extern char sModelPathDreamE5[]; /* "ETC\DREAME5.TMD"; not const: ResourceSource's name is char * */

extern char *GetDefaultDataDirectory(void); /* GameFiles.c: "CDI\\" */
extern void SetDataDirectory(char *dir);    /* code_171e0.c */

/* code_171e0.c: waits until the data source's driver takes the mode; every
 * task here starts with (0, 0, 0). */
extern void SetActiveDataSourceDriverMode(s32 async, s32 mode2, s32 useVSyncCallback);

/* GameFiles.c's movie getters. Each returns a movie's path (a gRecordTable
 * record, or "ETC\ASMK.STR") and writes its movie id, which
 * GetMovieFrameCount turns into the frame count a StreamTask plays. */
extern const char *GetAsmkMovie(s32 *movieIdOut);
extern const char *PickOpeningMovie(s32 *movieIdOut, s32 unused);
extern s32 GetMovieFrameCount(s32 movieId);

extern const char sLogoPathAsmk[]; /* "ETC\ASMKLOGO.TIM" */
extern const char sLogoPathOsd[];  /* "ETC\OSDLOGO.TIM" */

/* What GameApplication__RunTask builds: an IntermediateBase allocator taking
 * the DreamSys, New_GraphRoom or New_TitleMenu cast to this type. */
typedef IntermediateBase *(*NewTaskFn)(struct DreamSys *dreamSys);

/* Defined below GameApplication__RunTitleMenu, its caller. */
s32 GameApplication__RunTask(NewTaskFn newTask, struct DreamSys *dreamSys,
                             IntermediateBaseInitArgs *initArgs);

/* The first movie of special day `day`, and the frames of dayCount days'
 * movies from it. */
extern const char *GetSpecialDayMovieSpan(s32 *frameTotal, s32 day, s32 dayCount);
extern const char *GetEndingMovie(s32 *movieIdOut, s32 unused); /* arity-ok: the definition takes movieIdOut alone; retail's call still sets $a1 = 0 */
/* A special day's record or an event movie, for DreamSys's getCinematic
 * pair packed into one word (bank low, entry high, each zero-extended);
 * *movieIdOut is -1 for a record that is a TIM image. */
extern const char *GetSpecialDayOrEventRecord(s32 *movieIdOut, s32 packedPick);

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
    req.src.name = sModelPathDreamE5;
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
 * "loader" task for "ETC\ASMKLOGO.TIM" (GameApplication__ShowImage), then a separate
 * "stream" task for whatever type code GetAsmkMovie hands back
 * ("ETC\ASMK.STR"), then a second loader task for "ETC\OSDLOGO.TIM". */
void GameApplication__ShowIntroLogos(GameApplication *self) {
    const char *moviePath;
    s32 movieId;
    s32 frameCount;
    StreamTask *task;

    if (self->config->showIntroLogos != 0) {
        SetActiveDataSourceDriverMode(0, 0, 0);
        GameApplication__ShowImage(self, sLogoPathAsmk);
        task = New_StreamTask(0, 0, 0, 0);
        moviePath = GetAsmkMovie(&movieId);
        frameCount = GetMovieFrameCount(movieId);
        ((StreamTaskInitFn)task->methods->init)(task, (IntermediateBaseInitArgs *)self->aux,
                                                (s32)moviePath, frameCount, 1);
        task->methods->release(task);
        GameApplication__ShowImage(self, sLogoPathOsd);
    }
}

/* Registers a "loader" task for the given resource path: allocates the
 * task, gives it a completion callback (GameApplication__RegisterFilesCallback) and context
 * (self), then sets its remaining parameters (path, the parent's aux as its init args) and
 * starts it. */
void GameApplication__ShowImage(GameApplication *self, const char *path) {
    TaskCore *task = New_TaskCore(0, 0, 0);

    /* setCallback's occupant stores a void (*)(void *ctx); this callback
     * takes nothing and returns RegisterRecordTableFiles's value, which
     * TaskCore__RefreshViewValue ignores. */
    task->methods->setCallback(task, (void (*)(void *))GameApplication__RegisterFilesCallback, self);
    task->methods->setFrameBound(task, 0);
    task->methods->setSubHandle(task, path, 0);
    task->methods->init(task, (IntermediateBaseInitArgs *)self->aux, 0);
    task->methods->release(task);
}

extern s32 RegisterRecordTableFiles(s32 a0);

s32 GameApplication__RegisterFilesCallback(void) {
    return RegisterRecordTableFiles(0);
}

/* Optional stream-task init block, gated by self->config->playStreams (the same
 * shape as GameApplication__ShowIntroLogos's self->config->showIntroLogos gate, minus the two
 * GameApplication__ShowImage loader-task calls, and using PickOpeningMovie instead of
 * GetAsmkMovie to derive the type code). */
void GameApplication__PlayOpeningMovie(GameApplication *self) {
    const char *moviePath;
    s32 movieId;
    s32 frameCount;
    StreamTask *task;

    if (self->config->playStreams != 0) {
        SetActiveDataSourceDriverMode(0, 0, 0);
        task = New_StreamTask(0, 0, 0, 0);
        moviePath = PickOpeningMovie(&movieId, 0);
        frameCount = GetMovieFrameCount(movieId);
        ((StreamTaskInitFn)task->methods->init)(task, (IntermediateBaseInitArgs *)self->aux,
                                                (s32)moviePath, frameCount, 1);
        task->methods->release(task);
    }
}

/* Gated by self->config->pollGraphRoom. Checks the DreamSys's own status slot
 * (+0x1A0); if it isn't already "1" and self->skipGraphRoomPoll hasn't latched, kicks
 * off one PollTask (New_GraphRoom) and, if THAT reports "2", runs
 * GameApplication__PlaySpecialDayMovies. Then polls a second PollTask (New_TitleMenu) in a loop,
 * restarting the first PollTask each time it reports "2", until it
 * reports anything else; clears self->skipGraphRoomPoll and returns 0 or 2 depending
 * on whether that final status was below 1. */
s32 GameApplication__RunTitleMenu(GameApplication *self) {
    s32 status;
    s32 pollDone;

    if (self->config->pollGraphRoom != 0) {
        SetActiveDataSourceDriverMode(0, 0, 0);

        status = self->dreamSys->methods->getCurrentDayAndYear(self->dreamSys, 0);
        if (status != 1) {
            if (self->skipGraphRoomPoll == 0) {
                status = GameApplication__RunTask((NewTaskFn)New_GraphRoom, self->dreamSys,
                                                  (IntermediateBaseInitArgs *)self->aux);
                if (status == 2) {
                    GameApplication__PlaySpecialDayMovies(self);
                }
            }
        }

        pollDone = 2;
    retry:
        status = GameApplication__RunTask((NewTaskFn)New_TitleMenu, self->dreamSys,
                                          (IntermediateBaseInitArgs *)self->aux);
        if (status == pollDone) {
            GameApplication__RunTask((NewTaskFn)New_GraphRoom, self->dreamSys,
                                     (IntermediateBaseInitArgs *)self->aux);
            goto retry;
        }

        self->skipGraphRoomPoll = 0;
        return ((u32)status < 1) << 1;
    }
    return 2;
}

/* Builds a task with newTask(dreamSys), runs its init to the end (mode 0),
 * releases it and returns init's result. */
s32 GameApplication__RunTask(NewTaskFn newTask, struct DreamSys *dreamSys,
                             IntermediateBaseInitArgs *initArgs) {
    IntermediateBase *task = newTask(dreamSys);
    s32 result = task->methods->init(task, initArgs, 0);

    task->methods->release(task);
    return result;
}

/* Called by GameApplication__RunTitleMenu when its first PollTask reports "2". Gated by
 * self->config->playStreams (same gate as GameApplication__PlayOpeningMovie). Builds a StreamTask,
 * derives a count via GetSpecialDayMovieSpan, sets the task's frame bound
 * to that count / 15 and clears skipOnConfirm, then runs its init (stream
 * group -1, unlike the other call sites) and releases it. */
void GameApplication__PlaySpecialDayMovies(GameApplication *self) {
    StreamTask *task;

    /* MATCHING: the frame keeps 12 bytes here, frameTotal in the last 4. */
    struct {
        u8 pad00[8];
        s32 frameTotal;
    } buf;

    const char *moviePath;

    if (self->config->playStreams != 0) {
        SetActiveDataSourceDriverMode(0, 0, 0);
        task = New_StreamTask(0, 0, 0, 0);
        moviePath = GetSpecialDayMovieSpan(&buf.frameTotal, 0, 10);
        task->methods->setFrameBound(task, (u32)buf.frameTotal / STREAMTASK_FRAMES_PER_SECOND);
        task->methods->setSkipOnConfirm(task, 0);
        ((StreamTaskInitFn)task->methods->init)(task, (IntermediateBaseInitArgs *)self->aux,
                                                (s32)moviePath, -1, 1);
        task->methods->release(task);
    }
}

void GameApplication__NoOpSlot5C(void) {}

/* Builds a DayTask (include/DayTask.h), runs its init with self
 * alone (DayTask__Init takes nothing else, hence DayTaskInitFn) and
 * releases it; init's return, TimedTask::result, is a status code: 2 runs GameApplication__PlayCinematic, 3 latches self->skipGraphRoomPoll.
 * Then queries the DreamSys status slot again (as GameApplication__RunTitleMenu does),
 * this time passing an out-param, and derives a 0/1 result from both the
 * call's return and the out-param. */
/* Builds a DayTask for this instance's current state, reads one status
 * code off it, tears it down, and reacts to two of the codes. Then asks the
 * owned DreamSys a question and reports whether its answer was 1.
 *
 * Two things here were long-standing misreadings, both worth keeping written
 * down (docs/match-reports/GameApplication__RunDayTask.md):
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
s32 GameApplication__RunDayTask(GameApplication *self) {
    s32 status;
    DayTask *obj;
    s32 outVal;
    s32 check;
    s32 result;

    obj = New_DayTask((IntermediateBaseInitArgs *)self->aux, self->dreamSys,
                      self->config->dayTaskSyncDriver);
    status = ((DayTaskInitFn)obj->methods->init)(obj);
    obj->methods->release(obj);

    switch (status) {
        case DAYTASK_RESULT_CINEMATIC:
            GameApplication__PlayCinematic(self);
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
void GameApplication__PlayCinematic(GameApplication *self) {
    CinematicCall cc;

    /* MATCHING: the frame keeps 12 bytes here, movieId in the first 4. */
    struct {
        s32 movieId;
        u8 pad04[8];
    } idBuf;

    const char *path;
    s32 frameCount;
    TaskCore *task;

    cc = self->dreamSys->methods->getCinematic(self->dreamSys);
    path = GetSpecialDayOrEventRecord(&idBuf.movieId, (u16)cc.bank | ((u32)(u16)cc.entry << 16));
    SetActiveDataSourceDriverMode(0, 0, 0);

    if (idBuf.movieId != -1) {
        if (self->config->playStreams != 0) {
            StreamTask *streamTask = New_StreamTask(0, 0, 0, 0);

            streamTask->methods->setSkipOnConfirm(streamTask, 0);
            frameCount = GetMovieFrameCount(idBuf.movieId);
            ((StreamTaskInitFn)streamTask->methods->init)(
                streamTask, (IntermediateBaseInitArgs *)self->aux, (s32)path, frameCount, 1);
            streamTask->methods->release(streamTask);
        }
    } else {
        task = New_TaskCore(0, 0, 0);
        task->methods->setFrameBound(task, 10);
        task->methods->setSubHandle(task, path, 0);
        task->methods->init(task, (IntermediateBaseInitArgs *)self->aux, 0);
        task->methods->release(task);
    }
}

/* GameApplicationMethods slot +0x064. Gated by self->config->playStreams (same gate as
 * GameApplication__PlayOpeningMovie/GameApplication__PlaySpecialDayMovies). Builds a StreamTask, clears its skipOnConfirm,
 * derives a type code via GetEndingMovie, looks it up via GetMovieFrameCount,
 * initializes the task with it, then starts it -- the same shape as
 * GameApplication__ShowIntroLogos/GameApplication__PlayOpeningMovie, but with setSkipOnConfirm(0) added and GetEndingMovie
 * in place of GetAsmkMovie/PickOpeningMovie. */
void GameApplication__PlayEndingMovie(GameApplication *self) {
    StreamTask *task;
    s32 movieId;
    const char *moviePath;
    s32 frameCount;

    if (self->config->playStreams != 0) {
        SetActiveDataSourceDriverMode(0, 0, 0);
        task = New_StreamTask(0, 0, 0, 0);
        task->methods->setSkipOnConfirm(task, 0);
        moviePath = GetEndingMovie(&movieId, 0);
        frameCount = GetMovieFrameCount(movieId);
        ((StreamTaskInitFn)task->methods->init)(task, (IntermediateBaseInitArgs *)self->aux,
                                                (s32)moviePath, frameCount, 1);
        task->methods->release(task);
    }
}
