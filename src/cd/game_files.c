/*
 * game_files.c -- the LbdFile class (include/lbd_file.h), the loader for one
 * stage map chunk, STGnn\Mnnn.LBD: New_LbdFile to LbdFile__SetAutoLoadData
 * and GetLbdFileMethods; then the getters over the game's table of file
 * names, the record table (include/game_files.h, whose file documentation
 * lists the table's layout).
 */
#include "common.h"
#include "lbd_file.h"
#include "stage_grid.h"
#include "bmem_pmgr.h"
#include "game_files.h"
#include <rand.h>
#include "data_source.h"
#include <stdio.h>

/* sRecordTable's record indices. The first RECORD_TABLE_COUNT are the
 * sound banks (SND\*.VH/VB) and then each stage's files; the movie records
 * follow. */
enum RecordIndex {
    RECORD_TABLE_COUNT = 560,    /* GetRecordTable's count */
    RECORD_OPENING_MOVIES = 560, /* ETC\OPENINGA.STR .. ETC\OPENINGG.STR */
    RECORD_ENDING_MOVIE = 567,   /* ETC\ENDING.STR */
    RECORD_EVENT_MOVIES = 568,   /* FILM\EVENT1.STR .. FILM\EVENT6.STR */
    RECORD_SPECIAL_DAYS = 574    /* SPECIAL_DAY_RECORD_COUNT per special day */
};

/* A stage's records, from sStageFirstRecord[stage]: TEXA..TEXD.TIX, then
 * BGA..BGE.SEQ, then its Mnnn.LBD map chunks. */
#define STAGE_TEXTURE_COUNT 4
#define STAGE_BGM_COUNT 5
#define STAGE_RECORD_BGM STAGE_TEXTURE_COUNT
#define STAGE_RECORD_MAP_CHUNKS (STAGE_TEXTURE_COUNT + STAGE_BGM_COUNT)

/* PickStageTexture: one more of a stage's textures is in the pick every
 * DAYS_PER_TEXTURE days, repeating every STAGE_TEXTURE_COUNT of those. */
#define DAYS_PER_TEXTURE 10

/* The seven sSoundBankPaths entries, SND\AMBIENT .. SND\STANDERD. */
#define SOUND_BANK_COUNT 7

/* Movie ids: a movie record's index in sMovieFrameCounts, handed back
 * through the movie getters' movieIdOut. */
enum MovieId {
    MOVIE_OPENING_FIRST = 0,      /* OPENING_MOVIE_COUNT, A to G */
    MOVIE_ENDING = 7,             /* ETC\ENDING.STR */
    MOVIE_EVENT_FIRST = 8,        /* FILM\EVENTn.STR is MOVIE_EVENT_FIRST + n - 1 */
    MOVIE_SPECIAL_DAY_FIRST = 14, /* SPECIAL_DAY_MOVIE_COUNT per special day */
    MOVIE_ASMK = 49               /* ETC\ASMK.STR */
};

#define OPENING_MOVIE_COUNT 7

/* GetSpecialDayMovieSpan adds this to every movie's frame count but the
 * last. What the frames are for is not established. */
#define MOVIE_SPAN_GAP_FRAMES 10

/* allocator: a new map-chunk loader from the pool */
LbdFile *New_LbdFile(void) {
    LbdFile *obj = BMemPMgrAlloc(sizeof(LbdFile));
    if (obj != NULL) {
        GetLbdFileMethods()->ctor(obj);
        return obj;
    }
    return NULL;
}

/* slot +0x008 of gLbdFileMethods (ctor) */
void LbdFile__LbdFile(LbdFile *self) {
    GetActiveDataSourceMethods()->ctor((FileResource *)self);
    self->methods = GetLbdFileMethods();
    self->chunkIndex = -1;
    self->headerReady = 0;
    self->dataReady = 0;
    self->elemKey = 0;
    self->dataBuffer = NULL;
    self->autoLoadData = 1;
    self->buffer = BMemPMgrAlloc(LBDFILE_HEADER_BLOCK_SIZE);
    if (self->buffer != NULL) {
        self->bufferSize = LBDFILE_HEADER_BLOCK_SIZE;
    }
}

/* slot +0x00C of gLbdFileMethods (finalize) */
void LbdFile__Finalize(LbdFile *self) {
    self->methods->releaseDataBlock(self);
    GetActiveDataSourceMethods()->finalize((FileResource *)self);
}

/* slot +0x064 of gLbdFileMethods (onRequestDone) */
void LbdFile__AdvanceLoadState(LbdFile *self) {
    if (self->loadState == LBDFILE_LOAD_HEADER) {
        if (self->flags & CD_FLAG_READ_DONE) {
            self->loadState = LBDFILE_LOAD_IDLE;
            self->headerReady = 1;
            if (self->autoLoadData != 0) {
                /* MATCHING: called with no argument, as retail does; the slot still
                 * receives this loader, the caller's own `self`. */
                ((LbdFileLoadDataBlockNoArgFn)self->methods->loadDataBlock)();
            }
        }
    } else if (self->loadState == LBDFILE_LOAD_DATA) {
        if (self->flags & CD_FLAG_READ_DONE) {
            self->dataReady = 1;
            self->loadState = LBDFILE_LOAD_IDLE;
        }
    }
    GetActiveDataSourceMethods()->onRequestDone((FileResource *)self);
}

/* slot +0x074 of gLbdFileMethods (cancelRequests) */
void LbdFile__CancelRequests(LbdFile *self) {
    GetActiveDataSourceMethods()->cancelRequests((FileResource *)self);
    self->headerReady = 0;
    self->dataReady = 0;
    self->loadState = LBDFILE_LOAD_IDLE;
}

/* slot +0x078 of gLbdFileMethods (processBuffer): read the chunk file's header
 * block into the fixed buffer, cancelling any load in progress */
void LbdFile__LoadHeader(LbdFile *self, char *name) {
    if (self->buffer != NULL && name != NULL) {
        if (self->loadState == LBDFILE_LOAD_IDLE) {
            self->headerReady = 0;
        } else {
            self->methods->cancelRequests(self);
        }
        self->loadState = LBDFILE_LOAD_HEADER;
        self->methods->close(self);
        self->methods->open(self, name, 1, 0);
        self->methods->read(self, self->buffer, LBDFILE_HEADER_BLOCK_SIZE);
    }
}

void LbdFile__ReleaseHeader(LbdFile *self) {
    self->methods->freeBuffer(self);
    self->headerReady = 0;
    self->chunkIndex = -1;
}

/* slot +0x080 of gLbdFileMethods: load the data block the header describes */
s32 LbdFile__LoadDataBlock(LbdFile *self) {
    s32 size;
    if (((LbdFileHeader *)self->buffer)->hasData == 0) {
        return 0;
    }
    if (self->loadState != LBDFILE_LOAD_IDLE) {
        return 0;
    }
    /* MATCHING: called with no argument, as retail does; the slot still
     * receives this loader, the caller's own `self`. */
    ((LbdFileReleaseDataBlockNoArgFn)self->methods->releaseDataBlock)();
    size = ((LbdFileHeader *)self->buffer)->dataSize;
    self->dataBuffer = BMemPMgrAlloc(size);
    if (self->dataBuffer == NULL) {
        return 0;
    }
    self->loadState = LBDFILE_LOAD_DATA;
    self->methods->seek(self, ((LbdFileHeader *)self->buffer)->dataOffset, SEEK_SET);
    self->methods->read(self, self->dataBuffer, size);
    return 1;
}

/* slot +0x084 of gLbdFileMethods */
void LbdFile__ReleaseDataBlock(LbdFile *self) {
    self->dataReady = 0;
    if (self->dataBuffer != NULL) {
        self->dataBuffer = BMemPMgrFree(self->dataBuffer);
    }
}

extern char *sDefaultDataDirectory; /* "CDI\\" */
extern s32 sForcedSoundBank;
extern s32 sForcedStageBgm;
extern u8 sSoundBankPaths[];
extern CdFileEntry sRecordTable[];
extern char *sSoundEffectDirPtr; /* -> "SND\\SE" */
extern const char sAsmkMoviePath[];
extern s16 sMovieFrameCounts[];
extern s16 sStageFirstRecord[];

/* slot +0x088 of gLbdFileMethods */
void LbdFile__SetAutoLoadData(LbdFile *self, s32 value) {
    self->autoLoadData = value;
}

LbdFileMethods *GetLbdFileMethods(void) {
    return &gLbdFileMethods;
}

char *GetDefaultDataDirectory(void) {
    return sDefaultDataDirectory;
}

/* rand(), after srand(seed) when seed is nonzero. */
s32 SeedAndRandom(s32 seed, s32 unused) {
    if (seed != 0) {
        srand(seed);
    }
    return rand();
}

/* Forces the sound bank and stage BGM the pickers return (1-based, 0 for
 * random); a negative argument leaves that one as it was. */
void SetPickOverrides(s32 soundBank, s32 stageBgm) {
    if (soundBank >= 0) {
        sForcedSoundBank = soundBank;
    }
    if (stageBgm >= 0) {
        sForcedStageBgm = stageBgm;
    }
}

CdFileEntry *GetRecordTable(s32 *countOut) {
    if (countOut != NULL) {
        *countOut = RECORD_TABLE_COUNT;
    }
    return sRecordTable;
}

void *GetSoundBankPaths(void) {
    return sSoundBankPaths;
}

/* One of the SND\name paths WBgm opens as its VAB, forced or random. */
char *PickSoundBank(s32 unused) {
    u32 r = (u32)SeedAndRandom(0, unused) % SOUND_BANK_COUNT;
    char **table = GetSoundBankPaths();
    char **entry;
    s32 index;
    if (sForcedSoundBank != 0) {
        index = sForcedSoundBank - 1;
        entry = &table[index];
    } else {
        entry = &table[r];
    }
    return *entry;
}

char **GetSoundEffectDirRef(void) {
    return &sSoundEffectDirPtr;
}

/* MATCHING: game_files.h declares it unprototyped: DayTask's ctor passes a dead
 * argument, which a prototype would drop. */
char *GetSoundEffectDir(void) {
    return *GetSoundEffectDirRef();
}

CdFileEntry *GetStageRecords(s32 stage) {
    return &GetRecordTable(NULL)[sStageFirstRecord[stage]];
}

CdFileEntry *GetStageTextureRecords(s32 stage) {
    return GetStageRecords(stage);
}

/* A random texture record among the first one to four of the stage's,
 * one more every DAYS_PER_TEXTURE days. */
CdFileEntry *PickStageTexture(s32 stage, s32 unused, s32 day) {
    s32 textureCount = ((day - 1) % (STAGE_TEXTURE_COUNT * DAYS_PER_TEXTURE)) / DAYS_PER_TEXTURE + 1;
    s32 r = SeedAndRandom(0, unused) % textureCount;
    return &GetStageTextureRecords(stage)[r];
}

CdFileEntry *GetStageBgmRecords(s32 stage) {
    return &GetStageRecords(stage)[STAGE_RECORD_BGM];
}

/* A random or forced BGM record. Stage 9 never plays BGC.SEQ: a random
 * pick of it (2) becomes BGD, and a forced 3 becomes 4. */
CdFileEntry *PickStageBgm(s32 stage, s32 unused) {
    u32 r = (u32)SeedAndRandom(0, unused) % STAGE_BGM_COUNT;
    CdFileEntry *rec;
    if (stage == 9) {
        if (r == 2) {
            r = 3;
        }
        if (sForcedStageBgm == 3) {
            sForcedStageBgm = 4;
        }
    }
    rec = GetStageBgmRecords(stage);
    return &rec[sForcedStageBgm != 0 ? sForcedStageBgm - 1 : r];
}

CdFileEntry *GetStageMapChunkRecords(s32 stage) {
    return &GetStageRecords(stage)[STAGE_RECORD_MAP_CHUNKS];
}

CdFileEntry *GetStageMapChunkRecord(s32 stage, s32 chunk) {
    return &GetStageMapChunkRecords(stage)[chunk];
}

/* The chunk at column x, row y of the stage's grid. */
CdFileEntry *GetStageMapChunkRecordXY(s32 stage, s32 x, s32 y) {
    return GetStageMapChunkRecord(stage, x + GetStageGridDimensions(stage)->columns * y);
}

const char *GetAsmkMovie(s32 *movieIdOut) {
    if (movieIdOut != NULL) {
        *movieIdOut = MOVIE_ASMK;
    }
    return sAsmkMoviePath;
}

CdFileEntry *GetOpeningMovieRecords(s32 *movieIdOut) {
    if (movieIdOut != NULL) {
        *movieIdOut = MOVIE_OPENING_FIRST;
    }
    return &GetRecordTable(NULL)[RECORD_OPENING_MOVIES];
}

CdFileEntry *PickOpeningMovie(s32 *movieIdOut, s32 unused) {
    u32 r = (u32)SeedAndRandom(0, unused) % OPENING_MOVIE_COUNT;
    s32 firstMovieId;
    CdFileEntry *rec = GetOpeningMovieRecords(&firstMovieId);
    if (movieIdOut != NULL) {
        *movieIdOut = r + firstMovieId;
    }
    return &rec[r];
}

CdFileEntry *GetEndingMovieRecord(s32 *movieIdOut) {
    if (movieIdOut != NULL) {
        *movieIdOut = MOVIE_ENDING;
    }
    return &GetRecordTable(NULL)[RECORD_ENDING_MOVIE];
}

/* MATCHING: game_files.h declares it unprototyped: PlayEndingMovie passes a
 * dead second argument, which a prototype would drop. */
CdFileEntry *GetEndingMovie(s32 *movieIdOut) {
    s32 movieId;
    CdFileEntry *rec = GetEndingMovieRecord(&movieId);
    if (movieIdOut != NULL) {
        *movieIdOut = movieId;
    }
    return rec;
}

CdFileEntry *GetEventMovieRecords(s32 *movieIdOut) {
    if (movieIdOut != NULL) {
        *movieIdOut = MOVIE_EVENT_FIRST;
    }
    return &GetRecordTable(NULL)[RECORD_EVENT_MOVIES];
}

CdFileEntry *GetEventMovie(s32 *movieIdOut, s32 event) {
    s32 firstMovieId;
    CdFileEntry *rec = GetEventMovieRecords(&firstMovieId);
    if (movieIdOut != NULL) {
        *movieIdOut = event + firstMovieId;
    }
    return &rec[event];
}

CdFileEntry *GetSpecialDayRecords(s32 *movieIdOut, s32 day) {
    CdFileEntry *rec = &GetRecordTable(NULL)[RECORD_SPECIAL_DAYS];
    if (movieIdOut != NULL) {
        *movieIdOut = day * SPECIAL_DAY_MOVIE_COUNT + MOVIE_SPECIAL_DAY_FIRST;
    }
    return &rec[day * SPECIAL_DAY_RECORD_COUNT];
}

/* A special day's record or an event movie; *movieIdOut is MOVIE_ID_NONE for a
 * special day's TIM images. */
CdFileEntry *GetSpecialDayOrEventRecord(s32 *movieIdOut, CinematicCall pick) {
    s32 firstMovieId;
    CdFileEntry *rec;
    if (pick.bank >= 0) {
        rec = GetSpecialDayRecords(&firstMovieId, pick.bank);
        if (movieIdOut != NULL) {
            *movieIdOut = ((u16)pick.entry < SPECIAL_DAY_MOVIE_COUNT) ? pick.entry + firstMovieId
                                                                      : MOVIE_ID_NONE;
        }
        return &rec[pick.entry];
    }
    return GetEventMovie(movieIdOut, pick.entry);
}

s32 GetMovieFrameCount(s32 movieId) {
    return sMovieFrameCounts[movieId];
}

/* The first movie of special day `day`; *frameTotal is the frame count of
 * the movies of dayCount special days from it, MOVIE_SPAN_GAP_FRAMES
 * between each. */
CdFileEntry *GetSpecialDayMovieSpan(s32 *frameTotal, s32 day, s32 dayCount) {
    s32 firstMovieId;
    s32 movieId;
    s32 start;
    CdFileEntry *rec = GetSpecialDayRecords(&firstMovieId, day);
    /* MATCHING: dayCount becomes the end movie id; a separate local differs. */
    dayCount *= SPECIAL_DAY_MOVIE_COUNT;
    *frameTotal = 0;
    start = firstMovieId;
    dayCount += start;
    for (movieId = start; movieId < dayCount; movieId++) {
        *frameTotal += sMovieFrameCounts[movieId] + MOVIE_SPAN_GAP_FRAMES;
    }
    *frameTotal -= MOVIE_SPAN_GAP_FRAMES;
    return rec;
}
