/*
 * game_shell.c -- two classes, in address order, and the layer on top of the
 * second:
 *  - GameApplication (include/game_application.h, which documents the class
 *    and the order its hooks run in): its allocator and ctor, the RNG seed
 *    and initSystems overrides, then each hook Application__RunMainLoop
 *    calls, with the helpers it uses, and the table getter;
 *  - FileResource (include/file_resource.h), the base of everything loaded
 *    from a file, then the active-data-source layer and the data directory
 *    (include/data_source.h).
 */
#include "common.h"
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <libetc.h>
#include "game_application.h"
#include "dream_sys.h"
#include "link_resource.h"
#include "task_core.h"
#include "stream_task.h"
#include "graph_room.h"
#include "title_menu.h"
#include "day_task.h"
#include "data_source.h"
#include "null_driver.h"
#include "cd_driver.h"
#include "bmem_pmgr.h"
#include "game_files.h"
#include <strings.h>
#include <stdio.h>
#include "dream_day.h"

/* The table getters of every class SetActiveDataSource rebinds, NULL-
 * terminated; it sits right after gFileResourceMethods's last slot. */
extern void *(*sDataSourceClientGetters[])(void);

extern char sModelPathDreamE5[]; /* "ETC\DREAME5.TMD"; not const: ResourceSource's name is char * */

extern const char sLogoPathAsmk[]; /* "ETC\ASMKLOGO.TIM" */
extern const char sLogoPathOsd[];  /* "ETC\OSDLOGO.TIM" */

/* What GameApplication__RunTask builds: an IntermediateBase allocator taking
 * the DreamSys, New_GraphRoom or New_TitleMenu cast to this type. */
typedef IntermediateBase *(*NewTaskFn)(struct DreamSys *dreamSys);

/* Defined below GameApplication__RunTitleMenu, its caller. */
s32 GameApplication__RunTask(NewTaskFn newTask, struct DreamSys *dreamSys,
                             IntermediateBaseInitArgs *initArgs);

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
    task->methods->init(task, (IntermediateBaseInitArgs *)self->aux, 0);
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
 * APPLICATION_LOOP_DAY for the menu's result 0, else
 * APPLICATION_LOOP_OPENING. */
s32 GameApplication__RunTitleMenu(GameApplication *self) {
    s32 status;
    s32 graphResult; /* MATCHING: GRAPH, set after the GraphRoom check, not the constant */

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

        graphResult = TITLEMENU_RESULT_GRAPH;
    retry:
        status = GameApplication__RunTask((NewTaskFn)New_TitleMenu, self->dreamSys,
                                          (IntermediateBaseInitArgs *)self->aux);
        if (status == graphResult) {
            GameApplication__RunTask((NewTaskFn)New_GraphRoom, self->dreamSys,
                                     (IntermediateBaseInitArgs *)self->aux);
            goto retry;
        }

        self->skipGraphRoomPoll = 0;
        /* MATCHING: status == 0 ? APPLICATION_LOOP_DAY : APPLICATION_LOOP_OPENING,
         * spelled as retail computes it; the ternary compiles differently. */
        return ((u32)status < 1) << 1;
    }
    return APPLICATION_LOOP_DAY;
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
        task->methods->init(task, (IntermediateBaseInitArgs *)self->aux, 0);
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

/*
 * FileResource's own methods (include/file_resource.h), then the
 * active-data-source layer on top of them and the data directory CD paths
 * are built in (include/data_source.h). strcat, which BuildFileName calls,
 * is Sony's libc2 object, linked after this file.
 */

/* sActiveDataSource's two observed values are the header words of the two
 * sibling classes it selects between: gCdDriverMethods (the CD-ROM read driver,
 * cd_driver.c) and gNullDriverMethods (NullDriver, the null driver, include/null_driver.h). */
#define DATASOURCE_CD 0x13
#define DATASOURCE_NULL 0x23

void *FileResource__Release(FileResource *self) {
    self->freeGuard = 0;
    self->methods->finalize(self);
    GetBasicClassMethods()->finalize((BasicClass *)self);
    BMemPMgrFree(self);
    return NULL;
}

void FileResource__FileResource(FileResource *self) {
    GetBasicClassMethods()->ctor((BasicClass *)self);
    self->methods = GetFileResourceMethods();
    self->isOpen = 0;
    self->buffer = NULL;
    self->bufferSize = 0;
    self->freeGuard = 0;
    self->pendingRequests = 0;
    self->flags = 0;
    self->inQueueDispatch = 0;
    self->loadState = 0;
}

void FileResource__Finalize(FileResource *self) {
    self->methods->close(self);
    self->methods->freeBuffer(self);
}

void FileResource__LoadFile(FileResource *self, char *name) {
    s32 savedIsOpen;
    s32 size;
    void *buffer;

    if (self->buffer != NULL) {
        return;
    }
    savedIsOpen = self->isOpen;
    self->isOpen = 0;
    self->methods->open(self, name, 1, 0);
    size = self->methods->seek(self, 0, SEEK_END);
    buffer = BMemPMgrAlloc(size);
    if (buffer != NULL) {
        self->methods->seek(self, 0, SEEK_SET);
        self->methods->read(self, buffer, size);
        self->methods->close(self);
        self->buffer = buffer;
        self->bufferSize = size;
        self->isOpen = savedIsOpen;
    } else {
        BMemPMgrFree(NULL);
        self->methods->close(self);
    }
}

void FileResource__FreeBuffer(FileResource *self) {
    if (self->buffer == NULL) {
        return;
    }
    if (self->bufferSize == 0) {
        return;
    }
    if (self->freeGuard != 0) {
        return;
    }
    BMemPMgrFree(self->buffer);
    self->buffer = NULL;
}

void NoOp(void) {}

void FileResource__OnRequestDone(FileResource *self) {
    self->flags |= 1;
}

FileResourceMethods *GetFileResourceMethods(void) {
    return &gFileResourceMethods;
}

extern s32 sActiveDataSource;

FileResourceMethods *GetActiveDataSourceMethods(void) {
    if (sActiveDataSource == DATASOURCE_NULL) {
        return (FileResourceMethods *)GetNullDriverMethods();
    } else {
        return (FileResourceMethods *)GetCdDriverMethods();
    }
}

ResourceRequest *ResourceRequest__Set(ResourceRequest *self, void *buffer, char *name, s32 mode) {
    self->src.buffer = buffer;
    self->src.name = name;
    self->mode = mode;
    return self;
}

/* Install a new active data source, then copy its method block
 * (CopyDataSourceSlots) into FileResource's own table and into the table of
 * every registered client. */
void SetActiveDataSource(s32 source) {
    FileResourceMethods *src;
    FileResourceMethods *methods;
    void *(*getMethods)(void);
    void *(**entry)(void);

    entry = sDataSourceClientGetters;
    sActiveDataSource = source;
    if (source == DATASOURCE_CD) {
        src = (FileResourceMethods *)GetCdDriverMethods();
    } else {
        src = (FileResourceMethods *)GetNullDriverMethods();
    }
    methods = GetFileResourceMethods();
    /* MATCHING: gotos, because retail's loop is entered at its bottom test;
     * a while or for loop tests at the top. */
    goto copy;
next:
    entry++;
    methods = getMethods();
copy:
    CopyDataSourceSlots(methods, src);
    getMethods = *entry;
    if (getMethods != NULL) {
        goto next;
    }
}

/* Copy the eleven data-source interface slots of one method table into
 * another: SetActiveDataSource's rebinding step. +0x05C..+0x064 are the
 * base's own and are not copied. */
void CopyDataSourceSlots(FileResourceMethods *dst, FileResourceMethods *src) {
    dst->slot40 = src->slot40;
    dst->open = src->open;
    dst->close = src->close;
    dst->seek = src->seek;
    dst->slot50 = src->slot50;
    dst->read = src->read;
    dst->loadFile = src->loadFile;
    dst->runRequestQueue = src->runRequestQueue;
    dst->requestLoadFile = src->requestLoadFile;
    dst->stopService = src->stopService;
    dst->cancelRequests = src->cancelRequests;
}

void LockActiveDataSource(void) {
    if (sActiveDataSource == DATASOURCE_CD) {
        LockCd();
    }
}

void UnlockActiveDataSource(void) {
    if (sActiveDataSource == DATASOURCE_CD) {
        UnlockCd();
    }
}

s32 IsActiveDataSourceBusy(void) {
    if (sActiveDataSource == DATASOURCE_CD) {
        return IsCdBusy();
    }
    return 0;
}

s32 IsActiveDataSourceIdle(void) {
    if (sActiveDataSource == DATASOURCE_CD) {
        return IsCdIdle();
    }
    return 1;
}

s32 GetActiveDataSourceOperation(void) {
    if (sActiveDataSource == DATASOURCE_CD) {
        return GetCdOperation();
    }
    return 0;
}

s32 GetActiveDataSourceState(void) {
    if (sActiveDataSource == DATASOURCE_CD) {
        return GetCdState();
    }
    return 0;
}

typedef s32 (*DataSourceSetDriverModeFn)(s32, s32, s32);

/* SetNullDriverMode takes two arguments and SetCdDriverMode three. Both are
 * called through the three-argument type, and the VAB driver ignores the
 * third. Assigning SetNullDriverMode to `fn` warns about incompatible pointer
 * types, and that is harmless. */

void SetActiveDataSourceDriverMode(s32 async, s32 mode2, s32 useVSyncCallback) {
    DataSourceSetDriverModeFn fn;

    fn = SetNullDriverMode;
    if (sActiveDataSource == DATASOURCE_CD) {
        fn = SetCdDriverMode;
    }
    /* Retry until the driver accepts: SetCdDriverMode refuses while sCdBusy. */
    do {
    } while (fn(async, mode2, useVSyncCallback) == 0);
}

s32 GetActiveDataSourceDriverMode(s32 *outMode2) {
    if (sActiveDataSource == DATASOURCE_CD) {
        return GetCdDriverMode(outMode2);
    } else {
        return GetNullDriverMode(outMode2);
    }
}

s32 GetActiveDataSourceUseVSyncCallback(void) {
    if (sActiveDataSource == DATASOURCE_CD) {
        return GetCdUseVSyncCallback();
    } else {
        return GetNullDriverUseVSyncCallback();
    }
}

extern s32 sFileTableRegistered;

s32 RegisterFileTableEntries(CdFileEntry *table, s32 count) {
    s32 first;

    if (sActiveDataSource == DATASOURCE_CD) {
        sFileTableRegistered = 1;
        SetFileTable(table);
        first = GetFileTableCount();
        SetFileTableCount(first + count);
        return ResolveFileEntries(&table[first], count);
    }
    return 1;
}

extern char *sDataDirectory;

void SetDataDirectory(char *dir) {
    sDataDirectory = dir;
}

char *GetDataDirectory(void) {
    return sDataDirectory;
}

char *BuildFileName(char *dest, char *name, char *dir, char *ext) {
    dest[0] = '\0';
    if (dir != NULL) {
        strcat(dest, dir);
    }
    strcat(dest, name);
    strcat(dest, ext);
    return dest;
}
