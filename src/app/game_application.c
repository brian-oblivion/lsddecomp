/*
 * GameApplication's methods (include/game_application.h, which documents
 * the class and the order its hooks run in), in ROM order: its allocator
 * and ctor, the RNG seed and initSystems overrides, then each hook
 * Application__RunMainLoop calls, with the helpers it uses, ending with
 * its getter GetGameApplicationMethods; its method table closes the file.
 * FileResource and the data-source layer follow in file_resource.c and
 * data_source.c.
 */
#include "common.h"
#include <libgte.h>
#include <libetc.h>
#include "game_application.h"
#include "dream_sys.h"
#include "link_resource.h"
#include "stream_task.h"
#include "graph_room.h"
#include "title_menu.h"
#include "day_task.h"
#include "data_source.h"
#include "bmem_pmgr.h"

extern char sModelPathDreamE5[]; /* "ETC\DREAME5.TMD"; not const: ResourceSource's name is char * */

extern const char sLogoPathAsmk[]; /* "ETC\ASMKLOGO.TIM" */
extern const char sLogoPathOsd[];  /* "ETC\OSDLOGO.TIM" */

/* What GameApplication__RunTask builds: an IntermediateBase allocator taking
 * the DreamSys, New_GraphRoom or New_TitleMenu cast to this type. */
typedef IntermediateBase *(*NewTaskFn)(struct DreamSys *dreamSys);

/* Defined below GameApplication__RunTitleMenu, its caller. */
s32 GameApplication__RunTask(NewTaskFn newTask, struct DreamSys *dreamSys,
                             IntermediateBaseInitArgs *initArgs);

/* NULL on a failed allocation: the allocator's NULL is still the result. */
/* MATCHING: the NULL path falls off the end; an explicit return there costs an instruction. */
GameApplication *New_GameApplication(GameApplicationConfig *config) {
    GameApplication *self = BMemPMgrAlloc(sizeof(GameApplication));

    if (self != NULL) {
        GetGameApplicationMethods()->ctor(self, config);
        return self;
    }
}

/* ctor: Application's ctor with config->dataSource, this table, the config
 * kept, the data directory reset to its default, the DreamSys built from
 * ETC\DREAME5.TMD, config->dreamSysConfigOption handed to it, then the RNG seeded through
 * +0x040 (GameApplication__SeedRandom). */
void GameApplication__GameApplication(GameApplication *self, GameApplicationConfig *config) {
    /* MATCHING: a whole ResourceRequest though mode is never set; a bare
     * ResourceSource makes the stack frame smaller than retail's. */
    ResourceRequest req;

    GetApplicationMethods()->ctor((Application *)self, config->dataSource);
    self->methods = GetGameApplicationMethods();
    self->config = config;
    SetDataDirectory(GetDefaultDataDirectory());
    req.src.buffer = NULL;
    req.src.name = sModelPathDreamE5;
    self->dreamSys = New_DreamSys(New_LinkResource(&req.src), 0, 0);
    self->skipGraphRoomPoll = 0;
    self->dreamSys->methods->getSetConfigOption(self->dreamSys, config->dreamSysConfigOption);
    ((GameApplicationSeedRandomFn)self->methods->setScreenDims)(self);
}

/* +0x040: seeds rand() from the first scratchpad word, mod DAYS_PER_YEAR. */
void GameApplication__SeedRandom(GameApplication *self) {
    SeedAndRandom(*(s32 *)getScratchAddr(0) % DAYS_PER_YEAR, 0);
}

/* +0x044: Application's initSystems, unless already initialized. */
void GameApplication__InitSystems(GameApplication *self, DrawSystem *drawSystem, struct Pad *pad) {
    if (self->initialized == 0) {
        GetApplicationMethods()->initSystems((Application *)self, drawSystem, pad, 0);
    }
}

/* +0x050, gated by config->showIntroLogos: the ASMK logo, the ASMK movie,
 * the OSD logo. */
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
                                                moviePath, frameCount, 1);
        task->methods->release(task);
        GameApplication__ShowImage(self, sLogoPathOsd);
    }
}

/* Shows the TIM at `path` in a TaskCore (frame bound 0); its exit callback,
 * run as the image ends, registers the game's files. */
void GameApplication__ShowImage(GameApplication *self, const char *path) {
    TaskCore *task = New_TaskCore(0, 0, 0);

    /* The callback takes no ctx and returns a value nobody reads. */
    task->methods->setExitCallback(task, (void (*)(void *))GameApplication__RegisterFilesCallback, self);
    task->methods->setFrameBound(task, 0);
    task->methods->setSubHandle(task, path, 0);
    task->methods->init(task, (IntermediateBaseInitArgs *)self->aux, INTERMEDIATEBASE_INIT_RUN);
    task->methods->release(task);
}

/* ShowImage's exit callback: registers sRecordTable's files with the CD
 * driver. */
s32 GameApplication__RegisterFilesCallback(void) {
    return RegisterRecordTableFiles(0);
}

/* +0x054: one of the seven opening movies, picked at random. */
void GameApplication__PlayOpeningMovie(GameApplication *self) {
    const char *moviePath;
    s32 movieId;
    s32 frameCount;
    StreamTask *task;

    if (self->config->playStreams != 0) {
        SetActiveDataSourceDriverMode(0, 0, 0);
        task = New_StreamTask(0, 0, 0, 0);
        moviePath = (const char *)PickOpeningMovie(&movieId, 0);
        frameCount = GetMovieFrameCount(movieId);
        ((StreamTaskInitFn)task->methods->init)(task, (IntermediateBaseInitArgs *)self->aux,
                                                moviePath, frameCount, 1);
        task->methods->release(task);
    }
}

/* +0x058, gated by config->pollGraphRoom (0: straight to the day). Unless
 * it is day 1 or skipGraphRoomPoll is set (the last DayTask CLOSED), runs the
 * GraphRoom, then PlaySpecialDayMovies if it scored. Then the TitleMenu,
 * again after a GraphRoom each time it returns GRAPH. Returns
 * APPLICATION_LOOP_DAY when the menu ended normally (TASKCORE_RESULT_DONE),
 * else APPLICATION_LOOP_OPENING (it timed out). */
s32 GameApplication__RunTitleMenu(GameApplication *self) {
    s32 status;

    if (self->config->pollGraphRoom != 0) {
        SetActiveDataSourceDriverMode(0, 0, 0);

        status = self->dreamSys->methods->getCurrentDayAndYear(self->dreamSys, 0);
        if (status != 1) { /* the day number, 1-based */
            if (self->skipGraphRoomPoll == 0) {
                status = GameApplication__RunTask((NewTaskFn)New_GraphRoom, self->dreamSys,
                                                  (IntermediateBaseInitArgs *)self->aux);
                if (status == GRAPHROOM_RESULT_SCORED) {
                    GameApplication__PlaySpecialDayMovies(self);
                }
            }
        }

        for (;;) {
            status = GameApplication__RunTask((NewTaskFn)New_TitleMenu, self->dreamSys,
                                              (IntermediateBaseInitArgs *)self->aux);
            if (status != TITLEMENU_RESULT_GRAPH) {
                break;
            }
            GameApplication__RunTask((NewTaskFn)New_GraphRoom, self->dreamSys,
                                     (IntermediateBaseInitArgs *)self->aux);
        }

        self->skipGraphRoomPoll = 0;
        return status == TASKCORE_RESULT_DONE ? APPLICATION_LOOP_DAY : APPLICATION_LOOP_OPENING;
    }
    return APPLICATION_LOOP_DAY;
}

/* Builds a task with newTask(dreamSys), runs its init to the end
 * (INTERMEDIATEBASE_INIT_RUN),
 * releases it and returns init's result. */
s32 GameApplication__RunTask(NewTaskFn newTask, struct DreamSys *dreamSys,
                             IntermediateBaseInitArgs *initArgs) {
    IntermediateBase *task = newTask(dreamSys);
    s32 result = task->methods->init(task, initArgs, INTERMEDIATEBASE_INIT_RUN);

    task->methods->release(task);
    return result;
}

/* Streams FILM\SPDAY01A.STR with no frame count for the player (-1), a
 * frame bound of the total frames of the first ten special days' movies
 * (GetSpecialDayMovieSpan), and no skip on confirm. */
void GameApplication__PlaySpecialDayMovies(GameApplication *self) {
    StreamTask *task;

    /* MATCHING: frameTotal is the last word of a 12-byte local, where retail's
     * stack frame keeps it; a lone s32 sits elsewhere. */
    struct {
        u8 pad00[8];
        s32 frameTotal;
    } buf;

    const char *moviePath;

    if (self->config->playStreams != 0) {
        SetActiveDataSourceDriverMode(0, 0, 0);
        task = New_StreamTask(0, 0, 0, 0);
        moviePath = GetSpecialDayMovieSpan(&buf.frameTotal, 0, 10)->name;
        task->methods->setFrameBound(task, (u32)buf.frameTotal / STREAMTASK_FRAMES_PER_SECOND);
        task->methods->setSkipOnConfirm(task, 0);
        ((StreamTaskInitFn)task->methods->init)(task, (IntermediateBaseInitArgs *)self->aux,
                                                moviePath, -1, 1);
        task->methods->release(task);
    }
}

/* +0x05C: empty. */
void GameApplication__OnRepeatMenu(void) {}

/* +0x060: runs one DayTask. Its CINEMATIC result plays the cinematic
 * (PlayCinematic), CLOSED sets skipGraphRoomPoll. Returns nonzero when the
 * day is now day 1 of a year past the first, which runs +0x064. */
s32 GameApplication__RunDayTask(GameApplication *self) {
    s32 status;
    DayTask *dayTask;
    s32 year;
    s32 day;
    s32 result;

    dayTask = New_DayTask((IntermediateBaseInitArgs *)self->aux, self->dreamSys,
                          self->config->dayTaskSyncDriver);
    status = ((DayTaskInitFn)dayTask->methods->init)(dayTask);
    dayTask->methods->release(dayTask);

    switch (status) {
        case DAYTASK_RESULT_CINEMATIC:
            GameApplication__PlayCinematic(self);
            break;
        case DAYTASK_RESULT_CLOSED:
            self->skipGraphRoomPoll = 1;
            break;
    }

    day = self->dreamSys->methods->getCurrentDayAndYear(self->dreamSys, &year);
    result = 0;
    if (year != 0) {
        result = (day == 1);
    }
    return result;
}

/* The special day record or event movie DreamSys's getCinematic names: a
 * movie is streamed (gated by config->playStreams, no skip on confirm), a
 * TIM image (MOVIE_ID_NONE) is shown for 10 seconds. */
void GameApplication__PlayCinematic(GameApplication *self) {
    CinematicCall cc;

    s32 movieId;

    const char *path;
    s32 frameCount;
    TaskCore *task;

    cc = self->dreamSys->methods->getCinematic(self->dreamSys);
    path = GetSpecialDayOrEventRecord(&movieId, cc)->name;
    SetActiveDataSourceDriverMode(0, 0, 0);

    if (movieId != MOVIE_ID_NONE) {
        if (self->config->playStreams != 0) {
            StreamTask *streamTask = New_StreamTask(0, 0, 0, 0);

            streamTask->methods->setSkipOnConfirm(streamTask, 0);
            frameCount = GetMovieFrameCount(movieId);
            ((StreamTaskInitFn)streamTask->methods->init)(
                streamTask, (IntermediateBaseInitArgs *)self->aux, path, frameCount, 1);
            streamTask->methods->release(streamTask);
        }
    } else {
        task = New_TaskCore(0, 0, 0);
        task->methods->setFrameBound(task, 10);
        task->methods->setSubHandle(task, path, 0);
        task->methods->init(task, (IntermediateBaseInitArgs *)self->aux, INTERMEDIATEBASE_INIT_RUN);
        task->methods->release(task);
    }
}

/* +0x064: ETC\ENDING.STR, with no skip on confirm. */
void GameApplication__PlayEndingMovie(GameApplication *self) {
    StreamTask *task;
    s32 movieId;
    const char *moviePath;
    s32 frameCount;

    if (self->config->playStreams != 0) {
        SetActiveDataSourceDriverMode(0, 0, 0);
        task = New_StreamTask(0, 0, 0, 0);
        task->methods->setSkipOnConfirm(task, 0);
        moviePath = GetEndingMovie(&movieId, 0)->name;
        frameCount = GetMovieFrameCount(movieId);
        ((StreamTaskInitFn)task->methods->init)(task, (IntermediateBaseInitArgs *)self->aux,
                                                moviePath, frameCount, 1);
        task->methods->release(task);
    }
}

GameApplicationMethods *GetGameApplicationMethods(void) {
    return &gGameApplicationMethods;
}

/* The method table. A (void *) entry is a function whose declared type
 * differs from its slot's: a method inherited from a parent class and
 * declared on the parent's type. */

/* GameApplication (include/game_application.h): Application's table with
 * the ctor, the RNG seed and initSystems, then the hooks
 * Application__RunMainLoop calls, in the order it calls them. */
GameApplicationMethods gGameApplicationMethods = {
    /* +0x000 header */ GAMEAPPLICATION_CLASS_ID,
    /* +0x004 release */ (void *)BasicClass__Release,
    /* +0x008 ctor */ GameApplication__GameApplication,
    /* +0x00C finalize */ (void *)Application__Finalize,
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
    /* +0x040 setScreenDims */ (void *)GameApplication__SeedRandom,
    /* +0x044 initSystems */ (void *)GameApplication__InitSystems,
    /* +0x048 slot48 */ (void *)Application__NoOpSlot48,
    /* +0x04C runMainLoop */ (void *)Application__RunMainLoop,
    /* +0x050 showIntroLogos */ GameApplication__ShowIntroLogos,
    /* +0x054 playOpeningMovie */ GameApplication__PlayOpeningMovie,
    /* +0x058 runTitleMenu */ GameApplication__RunTitleMenu,
    /* +0x05C onRepeatMenu */ (void *)GameApplication__OnRepeatMenu,
    /* +0x060 runDayTask */ GameApplication__RunDayTask,
    /* +0x064 playEndingMovie */ GameApplication__PlayEndingMovie,
};
