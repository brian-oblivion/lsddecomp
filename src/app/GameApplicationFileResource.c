/*
 * GameApplicationFileResource -- two classes, in ROM order: GameApplication,
 * the game's Application, and FileResource, the base of everything loaded
 * from a file, with the active-data-source layer and the data directory.
 *
 * GameApplication (include/GameApplication.h, which documents the class):
 * its allocator and ctor, the RNG seed and initSystems overrides, then the
 * hooks Application__RunMainLoop calls, each with the helpers it uses:
 *  - ShowIntroLogos: ETC\ASMKLOGO.TIM, the ETC\ASMK.STR movie, ETC\OSDLOGO.TIM
 *    (ShowImage for each image);
 *  - PlayOpeningMovie: one of the opening movies, at random;
 *  - RunTitleMenu: the day's GraphRoom (and PlaySpecialDayMovies when it
 *    scored), then the TitleMenu, and GraphRoom again whenever the menu's
 *    GRAPH is chosen (RunTask runs each);
 *  - RunDayTask: one day (a DayTask), then the cinematic DreamSys holds
 *    (PlayCinematic); nonzero once a year has gone by;
 *  - PlayEndingMovie: ETC\ENDING.STR.
 * Every hook but RunDayTask first calls SetActiveDataSourceDriverMode(0, 0,
 * 0). Every task is handed the Application's `aux` as its
 * IntermediateBaseInitArgs, runs to its end inside init, and is released. A
 * movie is a StreamTask given the path and movie id GameFiles.c's getters
 * return, the id turned into a frame count by GetMovieFrameCount; an image is
 * a TaskCore showing the TIM. Every movie but the intro's is gated by
 * config->playStreams. GetGameApplicationMethods, the table's getter, ends
 * the class.
 *
 * FileResource (include/FileResource.h): see the section banner below.
 */
#include "common.h"
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <libetc.h>
#include "GameApplication.h"
#include "DreamSys.h"
#include "LinkResource.h"
#include "TaskCore.h"
#include "StreamTask.h"
#include "GraphRoom.h"
#include "TitleMenu.h"
#include "DayTask.h"
#include "GameApplicationFileResource.h"
#include "VabDriver.h"
#include "CdDriver.h"

extern char sModelPathDreamE5[]; /* "ETC\DREAME5.TMD"; not const: ResourceSource's name is char * */

extern char *GetDefaultDataDirectory(void); /* GameFiles.c: "CDI\\" */
extern void SetDataDirectory(char *dir);    /* below */

/* Below: waits until the data source's driver takes the mode; every
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

/* MATCHING: the NULL path falls off the end; any explicit return there costs
 * an instruction (BMemPMgrAlloc's NULL is already in $v0). */
GameApplication *New_GameApplication(GameApplicationConfig *config) {
    GameApplication *self = BMemPMgrAlloc(sizeof(GameApplication));

    if (self != NULL) {
        GetGameApplicationMethods()->ctor(self, config);
        return self;
    }
}

/* ctor: Application's ctor with config->dataSource, this table, the config
 * kept, the data directory reset to its default, the DreamSys built from
 * ETC\DREAME5.TMD, config->unk14 handed to it, then the RNG seeded through
 * +0x040 (GameApplication__SeedRandom). */
void GameApplication__GameApplication(GameApplication *self, GameApplicationConfig *config) {
    /* MATCHING: mode is never set, but a bare ResourceSource shrinks the
     * frame by 8. */
    ResourceRequest req;

    GetApplicationMethods()->ctor((Application *)self, config->dataSource);
    self->methods = GetGameApplicationMethods();
    self->config = config;
    SetDataDirectory(GetDefaultDataDirectory());
    req.src.buffer = NULL;
    req.src.name = sModelPathDreamE5;
    self->dreamSys = New_DreamSys(New_LinkResource(&req.src), 0, 0);
    self->skipGraphRoomPoll = 0;
    self->dreamSys->methods->slot228(self->dreamSys, config->unk14);
    ((GameApplicationSeedRandomFn)self->methods->setScreenDims)(self);
}

extern s32 SeedAndRandom(s32 seed, s32 unused);

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
                                                (s32)moviePath, frameCount, 1);
        task->methods->release(task);
        GameApplication__ShowImage(self, sLogoPathOsd);
    }
}

/* Shows the TIM at `path` in a TaskCore (frame bound 0); its view callback,
 * run as the image ends, registers the game's files. */
void GameApplication__ShowImage(GameApplication *self, const char *path) {
    TaskCore *task = New_TaskCore(0, 0, 0);

    /* The callback takes no ctx and returns a value nobody reads. */
    task->methods->setCallback(task, (void (*)(void *))GameApplication__RegisterFilesCallback, self);
    task->methods->setFrameBound(task, 0);
    task->methods->setSubHandle(task, path, 0);
    task->methods->init(task, (IntermediateBaseInitArgs *)self->aux, 0);
    task->methods->release(task);
}

extern s32 RegisterRecordTableFiles(s32 all); /* DayTaskStageMap.c */

/* ShowImage's view callback: registers gRecordTable's files with the CD
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
        moviePath = PickOpeningMovie(&movieId, 0);
        frameCount = GetMovieFrameCount(movieId);
        ((StreamTaskInitFn)task->methods->init)(task, (IntermediateBaseInitArgs *)self->aux,
                                                (s32)moviePath, frameCount, 1);
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
    s32 graphResult; /* MATCHING: set after the GraphRoom check, it holds its 2 in a saved register */

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
        return ((u32)status < 1) << 1; /* MATCHING: status == 0 ? LOOP_DAY : LOOP_OPENING */
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

/* +0x05C: empty. */
void GameApplication__NoOpSlot5C(void) {}

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
 * TIM image (movie id -1) is shown for 10 seconds. */
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
        moviePath = GetEndingMovie(&movieId, 0);
        frameCount = GetMovieFrameCount(movieId);
        ((StreamTaskInitFn)task->methods->init)(task, (IntermediateBaseInitArgs *)self->aux,
                                                (s32)moviePath, frameCount, 1);
        task->methods->release(task);
    }
}

GameApplicationMethods *GetGameApplicationMethods(void) {
    return &gGameApplicationMethods;
}

/*
 * FileResource's own methods, the active-data-source dispatch
 * layer on top of them, and the data directory that CD paths are built in.
 *
 * FileResource (include/FileResource.h) is the base of every class the
 * game loads from a file: a BasicClass subclass owning one file buffer
 * (FileResource__LoadFile reads a whole named file into it, FreeBuffer
 * releases it) and declaring the file-I/O interface that the CD driver
 * (gCdDriverMethods, include/CdDriver.h) and the SPU/VAB driver
 * (gVabDriverMethods, include/VabDriver.h) implement.
 *
 * gActiveDataSource selects one of those two drivers. SetActiveDataSource
 * installs one and copies its interface slots into FileResource's table and
 * into every client table. The Lock/Unlock, IsBusy/Idle, Get.../Set...
 * functions after it forward to the CD driver when it is active, and
 * otherwise do nothing, return a fixed value or call the SPU/VAB driver.
 * RegisterFileTableEntries appends CdFileEntry records to the CD driver's
 * file table and resolves them.
 *
 * SetDataDirectory/GetDataDirectory hold the directory that BuildCdFilePath
 * and CdStream__Open put between the root `\` and a file name. It is "" until
 * GameApplication's ctor installs "CDI\". BuildFileName joins an optional
 * directory, a name and an extension. ResourceRequest__Set fills the
 * {buffer, name, mode} descriptor the resource classes' ctors take. strcat,
 * which BuildFileName calls, is Sony's libc2 object, linked after this file.
 */

/* gActiveDataSource's two observed values are the header words of the two
 * sibling classes it selects between: gCdDriverMethods (the CD-ROM read driver,
 * CdDriver.c) and gVabDriverMethods (VabDriver, the SPU/VAB driver, include/VabDriver.h). */
#define DATASOURCE_CD 0x13
#define DATASOURCE_SPU 0x23

void *FileResource__Release(FileResource *this) {
    this->freeGuard = 0;
    this->methods->finalize(this);
    GetBasicClassMethods()->finalize((BasicClass *)this);
    BMemPMgrFree(this);
    return NULL;
}

void FileResource__FileResource(FileResource *this) {
    GetBasicClassMethods()->ctor((BasicClass *)this);
    this->methods = GetFileResourceMethods();
    this->isOpen = 0;
    this->buffer = NULL;
    this->bufferSize = 0;
    this->freeGuard = 0;
    this->pendingRequests = 0;
    this->flags = 0;
    this->inQueueDispatch = 0;
    this->loadState = 0;
}

void FileResource__Finalize(FileResource *this) {
    this->methods->close(this);
    this->methods->freeBuffer(this);
}

void FileResource__LoadFile(FileResource *this, char *name) {
    s32 savedIsOpen;
    s32 size;
    void *buffer;

    if (this->buffer != NULL) {
        return;
    }
    savedIsOpen = this->isOpen;
    this->isOpen = 0;
    this->methods->open(this, name, 1, 0);
    size = this->methods->seek(this, 0, 2);
    buffer = BMemPMgrAlloc(size);
    if (buffer != NULL) {
        this->methods->seek(this, 0, 0);
        this->methods->read(this, buffer, size);
        this->methods->close(this);
        this->buffer = buffer;
        this->bufferSize = size;
        this->isOpen = savedIsOpen;
    } else {
        BMemPMgrFree(NULL);
        this->methods->close(this);
    }
}

void FileResource__FreeBuffer(FileResource *this) {
    if (this->buffer == NULL) {
        return;
    }
    if (this->bufferSize == 0) {
        return;
    }
    if (this->freeGuard != 0) {
        return;
    }
    BMemPMgrFree(this->buffer);
    this->buffer = NULL;
}

void NoOp(void) {}

void FileResource__OnRequestDone(FileResource *this) {
    this->flags |= 1;
}

FileResourceMethods *GetFileResourceMethods(void) {
    return &gFileResourceMethods;
}

extern s32 gActiveDataSource;

void *GetActiveDataSourceMethods(void) {
    if (gActiveDataSource == DATASOURCE_SPU) {
        return GetVabDriverMethods();
    } else {
        return GetCdDriverMethods();
    }
}

ResourceRequest *ResourceRequest__Set(ResourceRequest *this, void *buffer, char *name, s32 mode) {
    this->src.buffer = buffer;
    this->src.name = name;
    this->mode = mode;
    return this;
}

/* Install a new active data source, then copy its method block
 * (CopyDataSourceSlots) into FileResource's own table and into the table of
 * every registered client. */
void SetActiveDataSource(s32 source) {
    FileResourceMethods *src;
    FileResourceMethods *methods;
    void *(*getMethods)(void);
    void *(**entry)(void);

    entry = gDataSourceClientGetters;
    gActiveDataSource = source;
    if (source == DATASOURCE_CD) {
        src = (FileResourceMethods *)GetCdDriverMethods();
    } else {
        src = (FileResourceMethods *)GetVabDriverMethods();
    }
    methods = GetFileResourceMethods();
    /* MATCHING: a while/for loop compiles top-tested; retail jumps into a bottom test. */
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

extern s32 LockCd(void);

void LockActiveDataSource(void) {
    if (gActiveDataSource == DATASOURCE_CD) {
        LockCd();
    }
}

extern s32 UnlockCd(void);

void UnlockActiveDataSource(void) {
    if (gActiveDataSource == DATASOURCE_CD) {
        UnlockCd();
    }
}

extern s32 IsCdBusy(void);

s32 IsActiveDataSourceBusy(void) {
    if (gActiveDataSource == DATASOURCE_CD) {
        return IsCdBusy();
    }
    return 0;
}

extern s32 IsCdIdle(void);

s32 IsActiveDataSourceIdle(void) {
    if (gActiveDataSource == DATASOURCE_CD) {
        return IsCdIdle();
    }
    return 1;
}

extern s32 GetCdOperation(void);

s32 GetActiveDataSourceOperation(void) {
    if (gActiveDataSource == DATASOURCE_CD) {
        return GetCdOperation();
    }
    return 0;
}

extern s32 GetCdState(void);

s32 GetActiveDataSourceState(void) {
    if (gActiveDataSource == DATASOURCE_CD) {
        return GetCdState();
    }
    return 0;
}

typedef s32 (*DataSourceSetDriverModeFn)(s32, s32, s32);
/* SetVabDriverMode takes two arguments and SetCdDriverMode three. Both are
 * called through the three-argument type, and the VAB driver ignores the
 * third. Assigning SetVabDriverMode to `fn` warns about incompatible pointer
 * types, and that is harmless. */
extern s32 SetVabDriverMode(s32 async, s32 mode2);
extern s32 SetCdDriverMode(s32 async, s32 mode2, s32 useVSyncCallback);

void SetActiveDataSourceDriverMode(s32 async, s32 mode2, s32 useVSyncCallback) {
    DataSourceSetDriverModeFn fn;

    fn = SetVabDriverMode;
    if (gActiveDataSource == DATASOURCE_CD) {
        fn = SetCdDriverMode;
    }
    /* Retry until the driver accepts: SetCdDriverMode refuses while gCdBusy. */
    do {
    } while (fn(async, mode2, useVSyncCallback) == 0);
}

extern s32 GetCdDriverMode(void); /* arity-ok: the definition takes (s32 *outMode2); retail's tail call passes nothing */
extern s32 GetVabDriverMode(void); /* arity-ok: the definition takes (s32 *outMode2); retail's tail call passes nothing */

s32 GetActiveDataSourceDriverMode(void) {
    if (gActiveDataSource == DATASOURCE_CD) {
        return GetCdDriverMode();
    } else {
        return GetVabDriverMode();
    }
}

extern s32 GetCdUseVSyncCallback(void);
extern s32 GetVabUseVSyncCallback(void);

s32 GetActiveDataSourceUseVSyncCallback(void) {
    if (gActiveDataSource == DATASOURCE_CD) {
        return GetCdUseVSyncCallback();
    } else {
        return GetVabUseVSyncCallback();
    }
}

extern s32 gFileTableRegistered;
extern void SetFileTable(CdFileEntry *table);
extern s32 GetFileTableCount(void);
extern void SetFileTableCount(s32 count);
extern s32 ResolveFileEntries(CdFileEntry *entries, s32 count);

s32 RegisterFileTableEntries(CdFileEntry *table, s32 count) {
    s32 first;

    if (gActiveDataSource == DATASOURCE_CD) {
        gFileTableRegistered = 1;
        SetFileTable(table);
        first = GetFileTableCount();
        SetFileTableCount(first + count);
        return ResolveFileEntries(&table[first], count);
    }
    return 1;
}

extern char *gDataDirectory;

void SetDataDirectory(char *dir) {
    gDataDirectory = dir;
}

char *GetDataDirectory(void) {
    return gDataDirectory;
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
