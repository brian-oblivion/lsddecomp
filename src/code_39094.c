/*
 * code_39094 -- two independent groups of game code:
 *
 * 1. LbdFile (include/LbdFile.h): the loader for one stage map chunk,
 *    STGnn\Mnnn.LBD. LbdFile__LoadHeader streams the file's first 0xB358
 *    bytes into the object's fixed buffer; when that read completes
 *    (LbdFile__AdvanceLoadState) LbdFile__LoadDataBlock reads the optional
 *    data block the header locates into a second allocation, unless
 *    LbdFile__SetAutoLoadData turned that off.
 * 2. Free functions over gRecordTable, a table of 0x230+ fixed 0x1C-byte
 *    records (Rec1C) whose first bytes are a file path (the sound banks
 *    SND\*.VH/VB; then per stage its TEXx.TIX, BGx.SEQ and Mnnn.LBD files;
 *    then the FILM .STR and IMG .TIM files): random-or-forced
 *    pickers (SeedAndRandom, SetPickOverrides/gForcedSoundBank/
 *    gForcedStageBgm), per-stage record-group accessors indexed by
 *    gStageFirstRecord and, for GetStageMapChunkRecordXY, by StageGrid.h's cell
 *    columns, and a family of "stream channel" lookups (GetAsmkMovie,
 *    PickOpeningMovie, GetEndingMovie, GetSpecialDayOrEventRecord,
 *    GetSpecialDayMovieSpan) whose shapes match their call sites in
 *    code_1677c.c one for one. The records' other fields and the channels'
 *    in-game meaning are not established.
 */
#include "common.h"
#include "LbdFile.h"
#include "StageGrid.h"

/* FileResource::flags: the driver's read into the buffer has completed
 * (setFlag). Same value and spelling as GraphicsResources.c's. */
#define CD_FLAG_READ_DONE 0x080

/* gRecordTable's record indices. The first RECORD_TABLE_COUNT are the
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

/* Movie ids: a movie record's index in gMovieFrameCounts, handed back
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

/* One 0x1C-byte record of the table GetRecordTable returns (gRecordTable,
 * 0x230 records); only its size is known here. */
typedef struct Rec1C {
    u8 data[0x1C];
} Rec1C;

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

/* slot +0x064 of gLbdFileMethods (setFlag) */
void LbdFile__AdvanceLoadState(LbdFile *self) {
    if (self->loadState == LBDFILE_LOAD_HEADER) {
        if (self->flags & CD_FLAG_READ_DONE) {
            self->loadState = LBDFILE_LOAD_IDLE;
            self->headerReady = 1;
            if (self->autoLoadData != 0) {
                ((LbdFileLoadDataBlockNoArgFn)self->methods->loadDataBlock)(); /* retail passes no argument */
            }
        }
    } else if (self->loadState == LBDFILE_LOAD_DATA) {
        if (self->flags & CD_FLAG_READ_DONE) {
            self->dataReady = 1;
            self->loadState = LBDFILE_LOAD_IDLE;
        }
    }
    GetActiveDataSourceMethods()->setFlag((FileResource *)self);
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
    ((LbdFileReleaseDataBlockNoArgFn)self->methods->releaseDataBlock)(); /* retail passes no argument */
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

extern char *gDefaultDataDirectory; /* "CDI\\" (sdata) */
extern s32 gForcedSoundBank;
extern s32 gForcedStageBgm;
extern u8 gSoundBankPaths[];
extern u8 gRecordTable[];
extern char *gSoundEffectDirPtr; /* -> "SND\\SE" */
extern const char sAsmkMoviePath[];
extern s16 gMovieFrameCounts[];
extern s16 gStageFirstRecord[];

/* slot +0x088 of gLbdFileMethods */
void LbdFile__SetAutoLoadData(LbdFile *self, s32 value) {
    self->autoLoadData = value;
}

LbdFileMethods *GetLbdFileMethods(void) {
    return &gLbdFileMethods;
}

char *GetDefaultDataDirectory(void) {
    return gDefaultDataDirectory;
}

s32 SeedAndRandom(s32 seed, s32 unused) {
    if (seed != 0) {
        srand(seed);
    }
    return rand();
}

void SetPickOverrides(s32 soundBank, s32 stageBgm) {
    if (soundBank >= 0) {
        gForcedSoundBank = soundBank;
    }
    if (stageBgm >= 0) {
        gForcedStageBgm = stageBgm;
    }
}

void *GetRecordTable(s32 *countOut) {
    if (countOut != NULL) {
        *countOut = RECORD_TABLE_COUNT;
    }
    return gRecordTable;
}

void *GetSoundBankPaths(void) {
    return gSoundBankPaths;
}

s32 PickSoundBank(s32 unused) {
    u32 r = (u32)SeedAndRandom(0, unused) % SOUND_BANK_COUNT;
    s32 *table = GetSoundBankPaths();
    s32 *entry;
    s32 index;
    if (gForcedSoundBank != 0) {
        index = gForcedSoundBank - 1;
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

Rec1C *GetStageRecords(s32 stage) {
    return &((Rec1C *)GetRecordTable(NULL))[gStageFirstRecord[stage]];
}

Rec1C *GetStageTextureRecords(s32 stage) {
    return GetStageRecords(stage);
}

Rec1C *PickStageTexture(s32 stage, s32 unused, s32 day) {
    s32 textureCount = ((day - 1) % (STAGE_TEXTURE_COUNT * DAYS_PER_TEXTURE)) / DAYS_PER_TEXTURE + 1;
    s32 r = SeedAndRandom(0, unused) % textureCount;
    return &GetStageTextureRecords(stage)[r];
}

Rec1C *GetStageBgmRecords(s32 stage) {
    return &GetStageRecords(stage)[STAGE_RECORD_BGM];
}

Rec1C *PickStageBgm(s32 stage, s32 unused) {
    u32 r = (u32)SeedAndRandom(0, unused) % STAGE_BGM_COUNT;
    Rec1C *rec;
    if (stage == 9) {
        if (r == 2) {
            r = 3;
        }
        if (gForcedStageBgm == 3) {
            gForcedStageBgm = 4;
        }
    }
    rec = GetStageBgmRecords(stage);
    return &rec[gForcedStageBgm != 0 ? gForcedStageBgm - 1 : r];
}

Rec1C *GetStageMapChunkRecords(s32 stage) {
    return &GetStageRecords(stage)[STAGE_RECORD_MAP_CHUNKS];
}

Rec1C *GetStageMapChunkRecord(s32 stage, s32 chunk) {
    return &GetStageMapChunkRecords(stage)[chunk];
}

Rec1C *GetStageMapChunkRecordXY(s32 stage, s32 x, s32 y) {
    return GetStageMapChunkRecord(stage, x + GetStageGridDimensions(stage)->columns * y);
}

const char *GetAsmkMovie(s32 *movieIdOut) {
    if (movieIdOut != NULL) {
        *movieIdOut = MOVIE_ASMK;
    }
    return sAsmkMoviePath;
}

Rec1C *GetOpeningMovieRecords(s32 *movieIdOut) {
    if (movieIdOut != NULL) {
        *movieIdOut = MOVIE_OPENING_FIRST;
    }
    return &((Rec1C *)GetRecordTable(NULL))[RECORD_OPENING_MOVIES];
}

Rec1C *PickOpeningMovie(s32 *movieIdOut, s32 unused) {
    u32 r = (u32)SeedAndRandom(0, unused) % OPENING_MOVIE_COUNT;
    s32 firstMovieId;
    Rec1C *rec = GetOpeningMovieRecords(&firstMovieId);
    if (movieIdOut != NULL) {
        *movieIdOut = r + firstMovieId;
    }
    return &rec[r];
}

Rec1C *GetEndingMovieRecord(s32 *movieIdOut) {
    if (movieIdOut != NULL) {
        *movieIdOut = MOVIE_ENDING;
    }
    return &((Rec1C *)GetRecordTable(NULL))[RECORD_ENDING_MOVIE];
}

Rec1C *GetEndingMovie(s32 *movieIdOut) {
    s32 movieId;
    Rec1C *rec = GetEndingMovieRecord(&movieId);
    if (movieIdOut != NULL) {
        *movieIdOut = movieId;
    }
    return rec;
}

Rec1C *GetEventMovieRecords(s32 *movieIdOut) {
    if (movieIdOut != NULL) {
        *movieIdOut = MOVIE_EVENT_FIRST;
    }
    return &((Rec1C *)GetRecordTable(NULL))[RECORD_EVENT_MOVIES];
}

Rec1C *GetEventMovie(s32 *movieIdOut, s32 event) {
    s32 firstMovieId;
    Rec1C *rec = GetEventMovieRecords(&firstMovieId);
    if (movieIdOut != NULL) {
        *movieIdOut = event + firstMovieId;
    }
    return &rec[event];
}

Rec1C *GetSpecialDayRecords(s32 *movieIdOut, s32 day) {
    Rec1C *rec = &((Rec1C *)GetRecordTable(NULL))[RECORD_SPECIAL_DAYS];
    if (movieIdOut != NULL) {
        *movieIdOut = day * SPECIAL_DAY_MOVIE_COUNT + MOVIE_SPECIAL_DAY_FIRST;
    }
    return &rec[day * SPECIAL_DAY_RECORD_COUNT];
}

/* two s16 halves passed by value in one register */
typedef struct RecPick {
    s16 group;
    s16 sub;
} RecPick;

Rec1C *GetSpecialDayOrEventRecord(s32 *movieIdOut, RecPick pick) {
    s32 firstMovieId;
    Rec1C *rec;
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
    return gMovieFrameCounts[movieId];
}

Rec1C *GetSpecialDayMovieSpan(s32 *frameTotal, s32 day, s32 dayCount) {
    s32 firstMovieId;
    s32 movieId;
    s32 start;
    Rec1C *rec = GetSpecialDayRecords(&firstMovieId, day);
    dayCount *= SPECIAL_DAY_MOVIE_COUNT; /* MATCHING: one variable; becomes the end movie id */
    *frameTotal = 0;
    start = firstMovieId;
    dayCount += start;
    for (movieId = start; movieId < dayCount; movieId++) {
        *frameTotal += gMovieFrameCounts[movieId] + MOVIE_SPAN_GAP_FRAMES;
    }
    *frameTotal -= MOVIE_SPAN_GAP_FRAMES;
    return rec;
}
