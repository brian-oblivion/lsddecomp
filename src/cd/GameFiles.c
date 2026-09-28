/*
 * GameFiles -- the LbdFile class, and the getters over the game's table of
 * file names.
 *
 * LbdFile (include/LbdFile.h, which documents the class): New_LbdFile to
 * LbdFile__SetAutoLoadData and GetLbdFileMethods, the loader for one stage
 * map chunk, STGnn\Mnnn.LBD.
 *
 * sRecordTable is an array of 0x1C-byte records (FilePathRecord), each a file path
 * padded with zeros; DayTaskStageMap.c's RegisterRecordTableFiles hands them to
 * the CD driver. In order:
 *  - the seven sound banks' SND\name.VH/VB pairs and SND\SE.VH/VB;
 *  - each stage's files, from gStageFirstRecord[stage]: its four textures
 *    (TEXA..TEXD.TIX), five BGM sequences (BGA..BGE.SEQ) and map chunks
 *    (Mnnn.LBD, laid out as StageGrid.h's grid);
 *  - from RECORD_TABLE_COUNT, the movies (ETC\OPENINGA..G.STR,
 *    ETC\ENDING.STR, FILM\EVENTn.STR), then six records per special day
 *    (FILM\SPDAYnnA/B.STR, IMG1\SPDAYnnC..F.TIM).
 * The stage getters return a record, used as a path: PickStageBgm's goes to
 * the WBgm's setSeq, PickStageTexture's to New_TimBlockSrc, and a map
 * chunk's to an LbdFile. The movie getters also hand back a movie id, the
 * movie's index in sMovieFrameCounts, whose value GetMovieFrameCount gives
 * GameApplicationFileResource.c's StreamTasks as the MoviePlayer's frame count.
 *
 * The random pickers draw through SeedAndRandom; SetPickOverrides forces
 * PickSoundBank's and PickStageBgm's choice (1-based, 0 for random).
 */
#include "common.h"
#include "LbdFile.h"
#include "StageGrid.h"

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

/* A stage's records, from gStageFirstRecord[stage]: TEXA..TEXD.TIX, then
 * BGA..BGE.SEQ, then its Mnnn.LBD map chunks. */
#define STAGE_TEXTURE_COUNT 4
#define STAGE_BGM_COUNT 5
#define STAGE_RECORD_BGM STAGE_TEXTURE_COUNT
#define STAGE_RECORD_MAP_CHUNKS (STAGE_TEXTURE_COUNT + STAGE_BGM_COUNT)

/* PickStageTexture: one more of a stage's textures is in the pick every
 * DAYS_PER_TEXTURE days, repeating every STAGE_TEXTURE_COUNT of those. */
#define DAYS_PER_TEXTURE 10

/* The seven gSoundBankPaths entries, SND\AMBIENT .. SND\STANDERD. */
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

/* Each special day's six records: FILM\SPDAYnnA/B.STR (its two movies),
 * then IMG1\SPDAYnnC..F.TIM. Special day 0 is SPDAY01. */
#define SPECIAL_DAY_RECORD_COUNT 6
#define SPECIAL_DAY_MOVIE_COUNT 2

/* GetSpecialDayMovieSpan adds this to every movie's frame count but the
 * last. What the frames are for is not established. */
#define MOVIE_SPAN_GAP_FRAMES 10

/* One sRecordTable record: a file path, zero-padded to 0x1C bytes. */
typedef struct FilePathRecord {
    u8 data[0x1C];
} FilePathRecord;

extern int rand(void);
extern void srand(unsigned int seed);
extern void *BMemPMgrFree(void *ptr);
extern FileResourceMethods *GetActiveDataSourceMethods(void);
extern void *BMemPMgrAlloc(s32 size);

/* allocator: new LbdFile object */
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
                /* MATCHING: called with no argument; retail leaves $a0 unset */
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

/* slot +0x078 of gLbdFileMethods: start streaming a file into the buffer */
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
    /* MATCHING: called with no argument; retail leaves $a0 unset */
    ((LbdFileReleaseDataBlockNoArgFn)self->methods->releaseDataBlock)();
    size = ((LbdFileHeader *)self->buffer)->dataSize;
    self->dataBuffer = BMemPMgrAlloc(size);
    if (self->dataBuffer == NULL) {
        return 0;
    }
    self->loadState = LBDFILE_LOAD_DATA;
    self->methods->seek(self, ((LbdFileHeader *)self->buffer)->dataOffset, 0);
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

extern char *sDefaultDataDirectory; /* "CDI\\" (sdata) */
extern s32 sForcedSoundBank;
extern s32 sForcedStageBgm;
extern u8 gSoundBankPaths[];
extern u8 sRecordTable[];
extern char *gSoundEffectDirPtr; /* -> "SND\\SE" */
extern const char sAsmkMoviePath[];
extern s16 sMovieFrameCounts[];
extern s16 gStageFirstRecord[];

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

void *GetRecordTable(s32 *countOut) {
    if (countOut != NULL) {
        *countOut = RECORD_TABLE_COUNT;
    }
    return sRecordTable;
}

void *GetSoundBankPaths(void) {
    return gSoundBankPaths;
}

/* One of the SND\name paths WBgm opens as its VAB, forced or random. */
s32 PickSoundBank(s32 unused) {
    u32 r = (u32)SeedAndRandom(0, unused) % SOUND_BANK_COUNT;
    s32 *table = GetSoundBankPaths();
    s32 *entry;
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
    return &gSoundEffectDirPtr;
}

char *GetSoundEffectDir(void) {
    return *GetSoundEffectDirRef();
}

FilePathRecord *GetStageRecords(s32 stage) {
    return &((FilePathRecord *)GetRecordTable(NULL))[gStageFirstRecord[stage]];
}

FilePathRecord *GetStageTextureRecords(s32 stage) {
    return GetStageRecords(stage);
}

/* A random texture record among the first one to four of the stage's,
 * one more every DAYS_PER_TEXTURE days. */
FilePathRecord *PickStageTexture(s32 stage, s32 unused, s32 day) {
    s32 textureCount = ((day - 1) % (STAGE_TEXTURE_COUNT * DAYS_PER_TEXTURE)) / DAYS_PER_TEXTURE + 1;
    s32 r = SeedAndRandom(0, unused) % textureCount;
    return &GetStageTextureRecords(stage)[r];
}

FilePathRecord *GetStageBgmRecords(s32 stage) {
    return &GetStageRecords(stage)[STAGE_RECORD_BGM];
}

/* A random or forced BGM record. Stage 9 never plays BGC.SEQ: a random
 * pick of it (2) becomes BGD, and a forced 3 becomes 4. */
FilePathRecord *PickStageBgm(s32 stage, s32 unused) {
    u32 r = (u32)SeedAndRandom(0, unused) % STAGE_BGM_COUNT;
    FilePathRecord *rec;
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

FilePathRecord *GetStageMapChunkRecords(s32 stage) {
    return &GetStageRecords(stage)[STAGE_RECORD_MAP_CHUNKS];
}

FilePathRecord *GetStageMapChunkRecord(s32 stage, s32 chunk) {
    return &GetStageMapChunkRecords(stage)[chunk];
}

/* The chunk at column x, row y of the stage's grid. */
FilePathRecord *GetStageMapChunkRecordXY(s32 stage, s32 x, s32 y) {
    return GetStageMapChunkRecord(stage, x + GetStageGridDimensions(stage)->columns * y);
}

const char *GetAsmkMovie(s32 *movieIdOut) {
    if (movieIdOut != NULL) {
        *movieIdOut = MOVIE_ASMK;
    }
    return sAsmkMoviePath;
}

FilePathRecord *GetOpeningMovieRecords(s32 *movieIdOut) {
    if (movieIdOut != NULL) {
        *movieIdOut = MOVIE_OPENING_FIRST;
    }
    return &((FilePathRecord *)GetRecordTable(NULL))[RECORD_OPENING_MOVIES];
}

FilePathRecord *PickOpeningMovie(s32 *movieIdOut, s32 unused) {
    u32 r = (u32)SeedAndRandom(0, unused) % OPENING_MOVIE_COUNT;
    s32 firstMovieId;
    FilePathRecord *rec = GetOpeningMovieRecords(&firstMovieId);
    if (movieIdOut != NULL) {
        *movieIdOut = r + firstMovieId;
    }
    return &rec[r];
}

FilePathRecord *GetEndingMovieRecord(s32 *movieIdOut) {
    if (movieIdOut != NULL) {
        *movieIdOut = MOVIE_ENDING;
    }
    return &((FilePathRecord *)GetRecordTable(NULL))[RECORD_ENDING_MOVIE];
}

FilePathRecord *GetEndingMovie(s32 *movieIdOut) {
    s32 movieId;
    FilePathRecord *rec = GetEndingMovieRecord(&movieId);
    if (movieIdOut != NULL) {
        *movieIdOut = movieId;
    }
    return rec;
}

FilePathRecord *GetEventMovieRecords(s32 *movieIdOut) {
    if (movieIdOut != NULL) {
        *movieIdOut = MOVIE_EVENT_FIRST;
    }
    return &((FilePathRecord *)GetRecordTable(NULL))[RECORD_EVENT_MOVIES];
}

FilePathRecord *GetEventMovie(s32 *movieIdOut, s32 event) {
    s32 firstMovieId;
    FilePathRecord *rec = GetEventMovieRecords(&firstMovieId);
    if (movieIdOut != NULL) {
        *movieIdOut = event + firstMovieId;
    }
    return &rec[event];
}

FilePathRecord *GetSpecialDayRecords(s32 *movieIdOut, s32 day) {
    FilePathRecord *rec = &((FilePathRecord *)GetRecordTable(NULL))[RECORD_SPECIAL_DAYS];
    if (movieIdOut != NULL) {
        *movieIdOut = day * SPECIAL_DAY_MOVIE_COUNT + MOVIE_SPECIAL_DAY_FIRST;
    }
    return &rec[day * SPECIAL_DAY_RECORD_COUNT];
}

/* DreamSys's getCinematic pair, passed by value in one register: a special
 * day and one of its records, or (group negative) an event movie. */
typedef struct RecPick {
    s16 group;
    s16 sub;
} RecPick;

/* A special day's record or an event movie; *movieIdOut is -1 for a
 * special day's TIM images. */
FilePathRecord *GetSpecialDayOrEventRecord(s32 *movieIdOut, RecPick pick) {
    s32 firstMovieId;
    FilePathRecord *rec;
    if (pick.group >= 0) {
        rec = GetSpecialDayRecords(&firstMovieId, pick.group);
        if (movieIdOut != NULL) {
            *movieIdOut = ((u16)pick.sub < SPECIAL_DAY_MOVIE_COUNT) ? pick.sub + firstMovieId : -1;
        }
        return &rec[pick.sub];
    }
    return GetEventMovie(movieIdOut, pick.sub);
}

s32 GetMovieFrameCount(s32 movieId) {
    return sMovieFrameCounts[movieId];
}

/* The first movie of special day `day`; *frameTotal is the frame count of
 * the movies of dayCount special days from it, MOVIE_SPAN_GAP_FRAMES
 * between each. */
FilePathRecord *GetSpecialDayMovieSpan(s32 *frameTotal, s32 day, s32 dayCount) {
    s32 firstMovieId;
    s32 movieId;
    s32 start;
    FilePathRecord *rec = GetSpecialDayRecords(&firstMovieId, day);
    dayCount *= SPECIAL_DAY_MOVIE_COUNT; /* MATCHING: one variable; becomes the end movie id */
    *frameTotal = 0;
    start = firstMovieId;
    dayCount += start;
    for (movieId = start; movieId < dayCount; movieId++) {
        *frameTotal += sMovieFrameCounts[movieId] + MOVIE_SPAN_GAP_FRAMES;
    }
    *frameTotal -= MOVIE_SPAN_GAP_FRAMES;
    return rec;
}
