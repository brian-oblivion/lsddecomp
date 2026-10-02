/*
 * game_files.c -- the getters over the game's table of file names, the
 * record table (include/game_files.h, whose file documentation lists the
 * table's layout), in ROM order from GetDefaultDataDirectory to
 * GetSpecialDayMovieSpan. The file tables themselves (the sound banks,
 * each stage's first record, the record table and the movies' frame
 * counts) end the file. LbdFile, the map-chunk loader before them in ROM,
 * is in lbd_file.c.
 */
#include "common.h"
#include "stage_grid.h"
#include "game_files.h"
#include <rand.h>

/* sRecordTable's record indices. The first RECORD_TABLE_COUNT are the
 * sound banks (SND\*.VH/VB) and then each stage's files; the movie records
 * follow. */
enum RecordIndex {
    RECORD_TABLE_COUNT = 560,    /* GetRecordTable's count */
    RECORD_OPENING_MOVIES = 560, /* ETC\OPENINGA.STR .. ETC\OPENINGG.STR */
    RECORD_ENDING_MOVIE = 567,   /* ETC\ENDING.STR */
    RECORD_EVENT_MOVIES = 568,   /* FILM\EVENT1.STR .. FILM\EVENT6.STR */
    RECORD_SPECIAL_DAYS = 574,   /* SPECIAL_DAY_RECORD_COUNT per special day */
    RECORD_COUNT = 653           /* the whole table, through ETC\ETCSE.VB */
};

/* The stages whose files the table lists, STG00..STG13. */
#define RECORD_STAGE_COUNT 14

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
    MOVIE_ASMK = 49,              /* ETC\ASMK.STR */
    MOVIE_COUNT = 50
};

#define OPENING_MOVIE_COUNT 7

/* GetSpecialDayMovieSpan adds this to every movie's frame count but the
 * last. What the frames are for is not established. */
#define MOVIE_SPAN_GAP_FRAMES 10

/* The game's data directory and the sound-effect directory, and the
 * pickers' forced choices (SetPickOverrides: 1-based, 0 for random). */
static char sSoundEffectDirName[];
static char sDefaultDataDirectoryName[] SDATA = "CDI\\";
static char *sDefaultDataDirectory SDATA = sDefaultDataDirectoryName;
static s32 sForcedSoundBank SDATA = 0;
static s32 sForcedStageBgm SDATA = 0;
static char *sSoundEffectDirPtr SDATA = sSoundEffectDirName;
static char sSoundEffectDirName[] SDATA = "SND\\SE";

extern char *sSoundBankPaths[SOUND_BANK_COUNT];
extern CdFileEntry sRecordTable[RECORD_COUNT];
/* The seven sound banks PickSoundBank picks among, as the SND\name paths
 * WBgm opens (".VH"/".VB" appended; sSoundBankPaths below lists them, its
 * char * elements hence the casts), then the ASMK logo movie. */
const char sSoundBankStanderdPath[] = "SND\\STANDERD";
const char sSoundBankLovelyPath[] = "SND\\LOVELY";
const char sSoundBankHumanPath[] = "SND\\HUMAN";
const char sSoundBankEthnovaPath[] = "SND\\ETHNOVA";
const char sSoundBankElectroPath[] = "SND\\ELECTRO";
const char sSoundBankCartoonPath[] = "SND\\CARTOON";
const char sSoundBankAmbientPath[] = "SND\\AMBIENT";
const char sAsmkMoviePath[] = "ETC\\ASMK.STR";
extern s16 sMovieFrameCounts[MOVIE_COUNT];
extern s16 sStageFirstRecord[RECORD_STAGE_COUNT];

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

/* A random or forced BGM record. Clockwork Machines never plays BGC.SEQ: a random
 * pick of it (2) becomes BGD, and a forced 3 becomes 4. */
CdFileEntry *PickStageBgm(s32 stage, s32 unused) {
    u32 r = (u32)SeedAndRandom(0, unused) % STAGE_BGM_COUNT;
    CdFileEntry *rec;
    if (stage == STAGE_CLOCKWORK_MACHINES) {
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

/* clang-format off */
char *sSoundBankPaths[SOUND_BANK_COUNT] = {
    (char *)sSoundBankAmbientPath,
    (char *)sSoundBankCartoonPath,
    (char *)sSoundBankElectroPath,
    (char *)sSoundBankEthnovaPath,
    (char *)sSoundBankHumanPath,
    (char *)sSoundBankLovelyPath,
    (char *)sSoundBankStanderdPath,
};
/* clang-format on */

/* Each stage's first record in sRecordTable (GetStageRecords): its four
 * TEXn.TIX textures, five BGn.SEQ tunes, then its Mnnn.LBD map chunks. */
s16 sStageFirstRecord[RECORD_STAGE_COUNT] = {
    16,  /* STG00 */
    30,  /* STG01 */
    45,  /* STG02 */
    90,  /* STG03 */
    355, /* STG04 */
    394, /* STG05 */
    433, /* STG06 */
    448, /* STG07 */
    462, /* STG08 */
    474, /* STG09 */
    485, /* STG10 */
    497, /* STG11 */
    518, /* STG12 */
    547, /* STG13 */
};

/* The game's files, by record index (enum RecordIndex): path and the disc
 * position and size ResolveFileEntries fills in when the table is
 * registered with the CD driver. */
/* clang-format off */
CdFileEntry sRecordTable[RECORD_COUNT] = {
    /*   0 */ {"SND\\AMBIENT.VH"},
    /*   1 */ {"SND\\AMBIENT.VB"},
    /*   2 */ {"SND\\CARTOON.VH"},
    /*   3 */ {"SND\\CARTOON.VB"},
    /*   4 */ {"SND\\ELECTRO.VH"},
    /*   5 */ {"SND\\ELECTRO.VB"},
    /*   6 */ {"SND\\ETHNOVA.VH"},
    /*   7 */ {"SND\\ETHNOVA.VB"},
    /*   8 */ {"SND\\HUMAN.VH"},
    /*   9 */ {"SND\\HUMAN.VB"},
    /*  10 */ {"SND\\LOVELY.VH"},
    /*  11 */ {"SND\\LOVELY.VB"},
    /*  12 */ {"SND\\STANDERD.VH"},
    /*  13 */ {"SND\\STANDERD.VB"},
    /*  14 */ {"SND\\SE.VH"},
    /*  15 */ {"SND\\SE.VB"},
    /*  16 */ {"STG00\\TEXA.TIX"},
    /*  17 */ {"STG00\\TEXB.TIX"},
    /*  18 */ {"STG00\\TEXC.TIX"},
    /*  19 */ {"STG00\\TEXD.TIX"},
    /*  20 */ {"STG00\\BGA.SEQ"},
    /*  21 */ {"STG00\\BGB.SEQ"},
    /*  22 */ {"STG00\\BGC.SEQ"},
    /*  23 */ {"STG00\\BGD.SEQ"},
    /*  24 */ {"STG00\\BGE.SEQ"},
    /*  25 */ {"STG00\\M000.LBD"},
    /*  26 */ {"STG00\\M001.LBD"},
    /*  27 */ {"STG00\\M002.LBD"},
    /*  28 */ {"STG00\\M003.LBD"},
    /*  29 */ {"STG00\\M004.LBD"},
    /*  30 */ {"STG01\\TEXA.TIX"},
    /*  31 */ {"STG01\\TEXB.TIX"},
    /*  32 */ {"STG01\\TEXC.TIX"},
    /*  33 */ {"STG01\\TEXD.TIX"},
    /*  34 */ {"STG01\\BGA.SEQ"},
    /*  35 */ {"STG01\\BGB.SEQ"},
    /*  36 */ {"STG01\\BGC.SEQ"},
    /*  37 */ {"STG01\\BGD.SEQ"},
    /*  38 */ {"STG01\\BGE.SEQ"},
    /*  39 */ {"STG01\\M000.LBD"},
    /*  40 */ {"STG01\\M001.LBD"},
    /*  41 */ {"STG01\\M002.LBD"},
    /*  42 */ {"STG01\\M003.LBD"},
    /*  43 */ {"STG01\\M004.LBD"},
    /*  44 */ {"STG01\\M005.LBD"},
    /*  45 */ {"STG02\\TEXA.TIX"},
    /*  46 */ {"STG02\\TEXB.TIX"},
    /*  47 */ {"STG02\\TEXC.TIX"},
    /*  48 */ {"STG02\\TEXD.TIX"},
    /*  49 */ {"STG02\\BGA.SEQ"},
    /*  50 */ {"STG02\\BGB.SEQ"},
    /*  51 */ {"STG02\\BGC.SEQ"},
    /*  52 */ {"STG02\\BGD.SEQ"},
    /*  53 */ {"STG02\\BGE.SEQ"},
    /*  54 */ {"STG02\\M000.LBD"},
    /*  55 */ {"STG02\\M001.LBD"},
    /*  56 */ {"STG02\\M002.LBD"},
    /*  57 */ {"STG02\\M003.LBD"},
    /*  58 */ {"STG02\\M004.LBD"},
    /*  59 */ {"STG02\\M005.LBD"},
    /*  60 */ {"STG02\\M006.LBD"},
    /*  61 */ {"STG02\\M007.LBD"},
    /*  62 */ {"STG02\\M008.LBD"},
    /*  63 */ {"STG02\\M009.LBD"},
    /*  64 */ {"STG02\\M010.LBD"},
    /*  65 */ {"STG02\\M011.LBD"},
    /*  66 */ {"STG02\\M012.LBD"},
    /*  67 */ {"STG02\\M013.LBD"},
    /*  68 */ {"STG02\\M014.LBD"},
    /*  69 */ {"STG02\\M015.LBD"},
    /*  70 */ {"STG02\\M016.LBD"},
    /*  71 */ {"STG02\\M017.LBD"},
    /*  72 */ {"STG02\\M018.LBD"},
    /*  73 */ {"STG02\\M019.LBD"},
    /*  74 */ {"STG02\\N1\\M020.LBD"},
    /*  75 */ {"STG02\\N1\\M021.LBD"},
    /*  76 */ {"STG02\\N1\\M022.LBD"},
    /*  77 */ {"STG02\\N1\\M023.LBD"},
    /*  78 */ {"STG02\\N1\\M024.LBD"},
    /*  79 */ {"STG02\\N1\\M025.LBD"},
    /*  80 */ {"STG02\\N1\\M026.LBD"},
    /*  81 */ {"STG02\\N1\\M027.LBD"},
    /*  82 */ {"STG02\\N1\\M028.LBD"},
    /*  83 */ {"STG02\\N1\\M029.LBD"},
    /*  84 */ {"STG02\\N1\\M030.LBD"},
    /*  85 */ {"STG02\\N1\\M031.LBD"},
    /*  86 */ {"STG02\\N1\\M032.LBD"},
    /*  87 */ {"STG02\\N1\\M033.LBD"},
    /*  88 */ {"STG02\\N1\\M034.LBD"},
    /*  89 */ {"STG02\\N1\\M035.LBD"},
    /*  90 */ {"STG03\\TEXA.TIX"},
    /*  91 */ {"STG03\\TEXB.TIX"},
    /*  92 */ {"STG03\\TEXC.TIX"},
    /*  93 */ {"STG03\\TEXD.TIX"},
    /*  94 */ {"STG03\\BGA.SEQ"},
    /*  95 */ {"STG03\\BGB.SEQ"},
    /*  96 */ {"STG03\\BGC.SEQ"},
    /*  97 */ {"STG03\\BGD.SEQ"},
    /*  98 */ {"STG03\\BGE.SEQ"},
    /*  99 */ {"STG03\\M000.LBD"},
    /* 100 */ {"STG03\\M001.LBD"},
    /* 101 */ {"STG03\\M002.LBD"},
    /* 102 */ {"STG03\\M003.LBD"},
    /* 103 */ {"STG03\\M004.LBD"},
    /* 104 */ {"STG03\\M005.LBD"},
    /* 105 */ {"STG03\\M006.LBD"},
    /* 106 */ {"STG03\\M007.LBD"},
    /* 107 */ {"STG03\\M008.LBD"},
    /* 108 */ {"STG03\\M009.LBD"},
    /* 109 */ {"STG03\\M010.LBD"},
    /* 110 */ {"STG03\\M011.LBD"},
    /* 111 */ {"STG03\\M012.LBD"},
    /* 112 */ {"STG03\\N1\\M013.LBD"},
    /* 113 */ {"STG03\\N1\\M014.LBD"},
    /* 114 */ {"STG03\\N1\\M015.LBD"},
    /* 115 */ {"STG03\\N1\\M016.LBD"},
    /* 116 */ {"STG03\\N1\\M017.LBD"},
    /* 117 */ {"STG03\\N1\\M018.LBD"},
    /* 118 */ {"STG03\\N1\\M019.LBD"},
    /* 119 */ {"STG03\\N1\\M020.LBD"},
    /* 120 */ {"STG03\\N1\\M021.LBD"},
    /* 121 */ {"STG03\\N1\\M022.LBD"},
    /* 122 */ {"STG03\\N1\\M023.LBD"},
    /* 123 */ {"STG03\\N1\\M024.LBD"},
    /* 124 */ {"STG03\\N1\\M025.LBD"},
    /* 125 */ {"STG03\\N1\\M026.LBD"},
    /* 126 */ {"STG03\\N1\\M027.LBD"},
    /* 127 */ {"STG03\\N1\\M028.LBD"},
    /* 128 */ {"STG03\\N1\\M029.LBD"},
    /* 129 */ {"STG03\\N1\\M030.LBD"},
    /* 130 */ {"STG03\\N1\\M031.LBD"},
    /* 131 */ {"STG03\\N1\\M032.LBD"},
    /* 132 */ {"STG03\\N1\\M033.LBD"},
    /* 133 */ {"STG03\\N1\\M034.LBD"},
    /* 134 */ {"STG03\\N1\\M035.LBD"},
    /* 135 */ {"STG03\\N1\\M036.LBD"},
    /* 136 */ {"STG03\\N1\\M037.LBD"},
    /* 137 */ {"STG03\\N1\\M038.LBD"},
    /* 138 */ {"STG03\\N1\\M039.LBD"},
    /* 139 */ {"STG03\\N1\\M040.LBD"},
    /* 140 */ {"STG03\\N1\\M041.LBD"},
    /* 141 */ {"STG03\\N1\\M042.LBD"},
    /* 142 */ {"STG03\\N2\\M043.LBD"},
    /* 143 */ {"STG03\\N2\\M044.LBD"},
    /* 144 */ {"STG03\\N2\\M045.LBD"},
    /* 145 */ {"STG03\\N2\\M046.LBD"},
    /* 146 */ {"STG03\\N2\\M047.LBD"},
    /* 147 */ {"STG03\\N2\\M048.LBD"},
    /* 148 */ {"STG03\\N2\\M049.LBD"},
    /* 149 */ {"STG03\\N2\\M050.LBD"},
    /* 150 */ {"STG03\\N2\\M051.LBD"},
    /* 151 */ {"STG03\\N2\\M052.LBD"},
    /* 152 */ {"STG03\\N2\\M053.LBD"},
    /* 153 */ {"STG03\\N2\\M054.LBD"},
    /* 154 */ {"STG03\\N2\\M055.LBD"},
    /* 155 */ {"STG03\\N2\\M056.LBD"},
    /* 156 */ {"STG03\\N2\\M057.LBD"},
    /* 157 */ {"STG03\\N2\\M058.LBD"},
    /* 158 */ {"STG03\\N2\\M059.LBD"},
    /* 159 */ {"STG03\\N2\\M060.LBD"},
    /* 160 */ {"STG03\\N2\\M061.LBD"},
    /* 161 */ {"STG03\\N2\\M062.LBD"},
    /* 162 */ {"STG03\\N2\\M063.LBD"},
    /* 163 */ {"STG03\\N2\\M064.LBD"},
    /* 164 */ {"STG03\\N2\\M065.LBD"},
    /* 165 */ {"STG03\\N2\\M066.LBD"},
    /* 166 */ {"STG03\\N2\\M067.LBD"},
    /* 167 */ {"STG03\\N2\\M068.LBD"},
    /* 168 */ {"STG03\\N2\\M069.LBD"},
    /* 169 */ {"STG03\\N2\\M070.LBD"},
    /* 170 */ {"STG03\\N2\\M071.LBD"},
    /* 171 */ {"STG03\\N2\\M072.LBD"},
    /* 172 */ {"STG03\\N3\\M073.LBD"},
    /* 173 */ {"STG03\\N3\\M074.LBD"},
    /* 174 */ {"STG03\\N3\\M075.LBD"},
    /* 175 */ {"STG03\\N3\\M076.LBD"},
    /* 176 */ {"STG03\\N3\\M077.LBD"},
    /* 177 */ {"STG03\\N3\\M078.LBD"},
    /* 178 */ {"STG03\\N3\\M079.LBD"},
    /* 179 */ {"STG03\\N3\\M080.LBD"},
    /* 180 */ {"STG03\\N3\\M081.LBD"},
    /* 181 */ {"STG03\\N3\\M082.LBD"},
    /* 182 */ {"STG03\\N3\\M083.LBD"},
    /* 183 */ {"STG03\\N3\\M084.LBD"},
    /* 184 */ {"STG03\\N3\\M085.LBD"},
    /* 185 */ {"STG03\\N3\\M086.LBD"},
    /* 186 */ {"STG03\\N3\\M087.LBD"},
    /* 187 */ {"STG03\\N3\\M088.LBD"},
    /* 188 */ {"STG03\\N3\\M089.LBD"},
    /* 189 */ {"STG03\\N3\\M090.LBD"},
    /* 190 */ {"STG03\\N3\\M091.LBD"},
    /* 191 */ {"STG03\\N3\\M092.LBD"},
    /* 192 */ {"STG03\\N3\\M093.LBD"},
    /* 193 */ {"STG03\\N3\\M094.LBD"},
    /* 194 */ {"STG03\\N3\\M095.LBD"},
    /* 195 */ {"STG03\\N3\\M096.LBD"},
    /* 196 */ {"STG03\\N3\\M097.LBD"},
    /* 197 */ {"STG03\\N3\\M098.LBD"},
    /* 198 */ {"STG03\\N3\\M099.LBD"},
    /* 199 */ {"STG03\\N3\\M100.LBD"},
    /* 200 */ {"STG03\\N3\\M101.LBD"},
    /* 201 */ {"STG03\\N3\\M102.LBD"},
    /* 202 */ {"STG03\\N4\\M103.LBD"},
    /* 203 */ {"STG03\\N4\\M104.LBD"},
    /* 204 */ {"STG03\\N4\\M105.LBD"},
    /* 205 */ {"STG03\\N4\\M106.LBD"},
    /* 206 */ {"STG03\\N4\\M107.LBD"},
    /* 207 */ {"STG03\\N4\\M108.LBD"},
    /* 208 */ {"STG03\\N4\\M109.LBD"},
    /* 209 */ {"STG03\\N4\\M110.LBD"},
    /* 210 */ {"STG03\\N4\\M111.LBD"},
    /* 211 */ {"STG03\\N4\\M112.LBD"},
    /* 212 */ {"STG03\\N4\\M113.LBD"},
    /* 213 */ {"STG03\\N4\\M114.LBD"},
    /* 214 */ {"STG03\\N4\\M115.LBD"},
    /* 215 */ {"STG03\\N4\\M116.LBD"},
    /* 216 */ {"STG03\\N4\\M117.LBD"},
    /* 217 */ {"STG03\\N4\\M118.LBD"},
    /* 218 */ {"STG03\\N4\\M119.LBD"},
    /* 219 */ {"STG03\\N4\\M120.LBD"},
    /* 220 */ {"STG03\\N4\\M121.LBD"},
    /* 221 */ {"STG03\\N4\\M122.LBD"},
    /* 222 */ {"STG03\\N4\\M123.LBD"},
    /* 223 */ {"STG03\\N4\\M124.LBD"},
    /* 224 */ {"STG03\\N4\\M125.LBD"},
    /* 225 */ {"STG03\\N4\\M126.LBD"},
    /* 226 */ {"STG03\\N4\\M127.LBD"},
    /* 227 */ {"STG03\\N4\\M128.LBD"},
    /* 228 */ {"STG03\\N4\\M129.LBD"},
    /* 229 */ {"STG03\\N4\\M130.LBD"},
    /* 230 */ {"STG03\\N4\\M131.LBD"},
    /* 231 */ {"STG03\\N4\\M132.LBD"},
    /* 232 */ {"STG03\\N5\\M133.LBD"},
    /* 233 */ {"STG03\\N5\\M134.LBD"},
    /* 234 */ {"STG03\\N5\\M135.LBD"},
    /* 235 */ {"STG03\\N5\\M136.LBD"},
    /* 236 */ {"STG03\\N5\\M137.LBD"},
    /* 237 */ {"STG03\\N5\\M138.LBD"},
    /* 238 */ {"STG03\\N5\\M139.LBD"},
    /* 239 */ {"STG03\\N5\\M140.LBD"},
    /* 240 */ {"STG03\\N5\\M141.LBD"},
    /* 241 */ {"STG03\\N5\\M142.LBD"},
    /* 242 */ {"STG03\\N5\\M143.LBD"},
    /* 243 */ {"STG03\\N5\\M144.LBD"},
    /* 244 */ {"STG03\\N5\\M145.LBD"},
    /* 245 */ {"STG03\\N5\\M146.LBD"},
    /* 246 */ {"STG03\\N5\\M147.LBD"},
    /* 247 */ {"STG03\\N5\\M148.LBD"},
    /* 248 */ {"STG03\\N5\\M149.LBD"},
    /* 249 */ {"STG03\\N5\\M150.LBD"},
    /* 250 */ {"STG03\\N5\\M151.LBD"},
    /* 251 */ {"STG03\\N5\\M152.LBD"},
    /* 252 */ {"STG03\\N5\\M153.LBD"},
    /* 253 */ {"STG03\\N5\\M154.LBD"},
    /* 254 */ {"STG03\\N5\\M155.LBD"},
    /* 255 */ {"STG03\\N5\\M156.LBD"},
    /* 256 */ {"STG03\\N5\\M157.LBD"},
    /* 257 */ {"STG03\\N5\\M158.LBD"},
    /* 258 */ {"STG03\\N5\\M159.LBD"},
    /* 259 */ {"STG03\\N5\\M160.LBD"},
    /* 260 */ {"STG03\\N5\\M161.LBD"},
    /* 261 */ {"STG03\\N5\\M162.LBD"},
    /* 262 */ {"STG03\\N6\\M163.LBD"},
    /* 263 */ {"STG03\\N6\\M164.LBD"},
    /* 264 */ {"STG03\\N6\\M165.LBD"},
    /* 265 */ {"STG03\\N6\\M166.LBD"},
    /* 266 */ {"STG03\\N6\\M167.LBD"},
    /* 267 */ {"STG03\\N6\\M168.LBD"},
    /* 268 */ {"STG03\\N6\\M169.LBD"},
    /* 269 */ {"STG03\\N6\\M170.LBD"},
    /* 270 */ {"STG03\\N6\\M171.LBD"},
    /* 271 */ {"STG03\\N6\\M172.LBD"},
    /* 272 */ {"STG03\\N6\\M173.LBD"},
    /* 273 */ {"STG03\\N6\\M174.LBD"},
    /* 274 */ {"STG03\\N6\\M175.LBD"},
    /* 275 */ {"STG03\\N6\\M176.LBD"},
    /* 276 */ {"STG03\\N6\\M177.LBD"},
    /* 277 */ {"STG03\\N6\\M178.LBD"},
    /* 278 */ {"STG03\\N6\\M179.LBD"},
    /* 279 */ {"STG03\\N6\\M180.LBD"},
    /* 280 */ {"STG03\\N6\\M181.LBD"},
    /* 281 */ {"STG03\\N6\\M182.LBD"},
    /* 282 */ {"STG03\\N6\\M183.LBD"},
    /* 283 */ {"STG03\\N6\\M184.LBD"},
    /* 284 */ {"STG03\\N6\\M185.LBD"},
    /* 285 */ {"STG03\\N6\\M186.LBD"},
    /* 286 */ {"STG03\\N6\\M187.LBD"},
    /* 287 */ {"STG03\\N6\\M188.LBD"},
    /* 288 */ {"STG03\\N6\\M189.LBD"},
    /* 289 */ {"STG03\\N6\\M190.LBD"},
    /* 290 */ {"STG03\\N6\\M191.LBD"},
    /* 291 */ {"STG03\\N6\\M192.LBD"},
    /* 292 */ {"STG03\\N7\\M193.LBD"},
    /* 293 */ {"STG03\\N7\\M194.LBD"},
    /* 294 */ {"STG03\\N7\\M195.LBD"},
    /* 295 */ {"STG03\\N7\\M196.LBD"},
    /* 296 */ {"STG03\\N7\\M197.LBD"},
    /* 297 */ {"STG03\\N7\\M198.LBD"},
    /* 298 */ {"STG03\\N7\\M199.LBD"},
    /* 299 */ {"STG03\\N7\\M200.LBD"},
    /* 300 */ {"STG03\\N7\\M201.LBD"},
    /* 301 */ {"STG03\\N7\\M202.LBD"},
    /* 302 */ {"STG03\\N7\\M203.LBD"},
    /* 303 */ {"STG03\\N7\\M204.LBD"},
    /* 304 */ {"STG03\\N7\\M205.LBD"},
    /* 305 */ {"STG03\\N7\\M206.LBD"},
    /* 306 */ {"STG03\\N7\\M207.LBD"},
    /* 307 */ {"STG03\\N7\\M208.LBD"},
    /* 308 */ {"STG03\\N7\\M209.LBD"},
    /* 309 */ {"STG03\\N7\\M210.LBD"},
    /* 310 */ {"STG03\\N7\\M211.LBD"},
    /* 311 */ {"STG03\\N7\\M212.LBD"},
    /* 312 */ {"STG03\\N7\\M213.LBD"},
    /* 313 */ {"STG03\\N7\\M214.LBD"},
    /* 314 */ {"STG03\\N7\\M215.LBD"},
    /* 315 */ {"STG03\\N7\\M216.LBD"},
    /* 316 */ {"STG03\\N7\\M217.LBD"},
    /* 317 */ {"STG03\\N7\\M218.LBD"},
    /* 318 */ {"STG03\\N7\\M219.LBD"},
    /* 319 */ {"STG03\\N7\\M220.LBD"},
    /* 320 */ {"STG03\\N7\\M221.LBD"},
    /* 321 */ {"STG03\\N7\\M222.LBD"},
    /* 322 */ {"STG03\\N8\\M223.LBD"},
    /* 323 */ {"STG03\\N8\\M224.LBD"},
    /* 324 */ {"STG03\\N8\\M225.LBD"},
    /* 325 */ {"STG03\\N8\\M226.LBD"},
    /* 326 */ {"STG03\\N8\\M227.LBD"},
    /* 327 */ {"STG03\\N8\\M228.LBD"},
    /* 328 */ {"STG03\\N8\\M229.LBD"},
    /* 329 */ {"STG03\\N8\\M230.LBD"},
    /* 330 */ {"STG03\\N8\\M231.LBD"},
    /* 331 */ {"STG03\\N8\\M232.LBD"},
    /* 332 */ {"STG03\\N8\\M233.LBD"},
    /* 333 */ {"STG03\\N8\\M234.LBD"},
    /* 334 */ {"STG03\\N8\\M235.LBD"},
    /* 335 */ {"STG03\\N8\\M236.LBD"},
    /* 336 */ {"STG03\\N8\\M237.LBD"},
    /* 337 */ {"STG03\\N8\\M238.LBD"},
    /* 338 */ {"STG03\\N8\\M239.LBD"},
    /* 339 */ {"STG03\\N8\\M240.LBD"},
    /* 340 */ {"STG03\\N8\\M241.LBD"},
    /* 341 */ {"STG03\\N8\\M242.LBD"},
    /* 342 */ {"STG03\\N8\\M243.LBD"},
    /* 343 */ {"STG03\\N8\\M244.LBD"},
    /* 344 */ {"STG03\\N8\\M245.LBD"},
    /* 345 */ {"STG03\\N8\\M246.LBD"},
    /* 346 */ {"STG03\\N8\\M247.LBD"},
    /* 347 */ {"STG03\\N8\\M248.LBD"},
    /* 348 */ {"STG03\\N8\\M249.LBD"},
    /* 349 */ {"STG03\\N8\\M250.LBD"},
    /* 350 */ {"STG03\\N8\\M251.LBD"},
    /* 351 */ {"STG03\\N8\\M252.LBD"},
    /* 352 */ {"STG03\\N9\\M253.LBD"},
    /* 353 */ {"STG03\\N9\\M254.LBD"},
    /* 354 */ {"STG03\\N9\\M255.LBD"},
    /* 355 */ {"STG04\\TEXA.TIX"},
    /* 356 */ {"STG04\\TEXB.TIX"},
    /* 357 */ {"STG04\\TEXC.TIX"},
    /* 358 */ {"STG04\\TEXD.TIX"},
    /* 359 */ {"STG04\\BGA.SEQ"},
    /* 360 */ {"STG04\\BGB.SEQ"},
    /* 361 */ {"STG04\\BGC.SEQ"},
    /* 362 */ {"STG04\\BGD.SEQ"},
    /* 363 */ {"STG04\\BGE.SEQ"},
    /* 364 */ {"STG04\\M000.LBD"},
    /* 365 */ {"STG04\\M001.LBD"},
    /* 366 */ {"STG04\\M002.LBD"},
    /* 367 */ {"STG04\\M003.LBD"},
    /* 368 */ {"STG04\\M004.LBD"},
    /* 369 */ {"STG04\\M005.LBD"},
    /* 370 */ {"STG04\\M006.LBD"},
    /* 371 */ {"STG04\\M007.LBD"},
    /* 372 */ {"STG04\\M008.LBD"},
    /* 373 */ {"STG04\\M009.LBD"},
    /* 374 */ {"STG04\\M010.LBD"},
    /* 375 */ {"STG04\\M011.LBD"},
    /* 376 */ {"STG04\\M012.LBD"},
    /* 377 */ {"STG04\\M013.LBD"},
    /* 378 */ {"STG04\\M014.LBD"},
    /* 379 */ {"STG04\\M015.LBD"},
    /* 380 */ {"STG04\\M016.LBD"},
    /* 381 */ {"STG04\\M017.LBD"},
    /* 382 */ {"STG04\\M018.LBD"},
    /* 383 */ {"STG04\\M019.LBD"},
    /* 384 */ {"STG04\\N1\\M020.LBD"},
    /* 385 */ {"STG04\\N1\\M021.LBD"},
    /* 386 */ {"STG04\\N1\\M022.LBD"},
    /* 387 */ {"STG04\\N1\\M023.LBD"},
    /* 388 */ {"STG04\\N1\\M024.LBD"},
    /* 389 */ {"STG04\\N1\\M025.LBD"},
    /* 390 */ {"STG04\\N1\\M026.LBD"},
    /* 391 */ {"STG04\\N1\\M027.LBD"},
    /* 392 */ {"STG04\\N1\\M028.LBD"},
    /* 393 */ {"STG04\\N1\\M029.LBD"},
    /* 394 */ {"STG05\\TEXA.TIX"},
    /* 395 */ {"STG05\\TEXB.TIX"},
    /* 396 */ {"STG05\\TEXC.TIX"},
    /* 397 */ {"STG05\\TEXD.TIX"},
    /* 398 */ {"STG05\\BGA.SEQ"},
    /* 399 */ {"STG05\\BGB.SEQ"},
    /* 400 */ {"STG05\\BGC.SEQ"},
    /* 401 */ {"STG05\\BGD.SEQ"},
    /* 402 */ {"STG05\\BGE.SEQ"},
    /* 403 */ {"STG05\\M000.LBD"},
    /* 404 */ {"STG05\\M001.LBD"},
    /* 405 */ {"STG05\\M002.LBD"},
    /* 406 */ {"STG05\\M003.LBD"},
    /* 407 */ {"STG05\\M004.LBD"},
    /* 408 */ {"STG05\\M005.LBD"},
    /* 409 */ {"STG05\\M006.LBD"},
    /* 410 */ {"STG05\\M007.LBD"},
    /* 411 */ {"STG05\\M008.LBD"},
    /* 412 */ {"STG05\\M009.LBD"},
    /* 413 */ {"STG05\\M010.LBD"},
    /* 414 */ {"STG05\\M011.LBD"},
    /* 415 */ {"STG05\\M012.LBD"},
    /* 416 */ {"STG05\\M013.LBD"},
    /* 417 */ {"STG05\\M014.LBD"},
    /* 418 */ {"STG05\\M015.LBD"},
    /* 419 */ {"STG05\\M016.LBD"},
    /* 420 */ {"STG05\\M017.LBD"},
    /* 421 */ {"STG05\\M018.LBD"},
    /* 422 */ {"STG05\\M019.LBD"},
    /* 423 */ {"STG05\\N1\\M020.LBD"},
    /* 424 */ {"STG05\\N1\\M021.LBD"},
    /* 425 */ {"STG05\\N1\\M022.LBD"},
    /* 426 */ {"STG05\\N1\\M023.LBD"},
    /* 427 */ {"STG05\\N1\\M024.LBD"},
    /* 428 */ {"STG05\\N1\\M025.LBD"},
    /* 429 */ {"STG05\\N1\\M026.LBD"},
    /* 430 */ {"STG05\\N1\\M027.LBD"},
    /* 431 */ {"STG05\\N1\\M028.LBD"},
    /* 432 */ {"STG05\\N1\\M029.LBD"},
    /* 433 */ {"STG06\\TEXA.TIX"},
    /* 434 */ {"STG06\\TEXB.TIX"},
    /* 435 */ {"STG06\\TEXC.TIX"},
    /* 436 */ {"STG06\\TEXD.TIX"},
    /* 437 */ {"STG06\\BGA.SEQ"},
    /* 438 */ {"STG06\\BGB.SEQ"},
    /* 439 */ {"STG06\\BGC.SEQ"},
    /* 440 */ {"STG06\\BGD.SEQ"},
    /* 441 */ {"STG06\\BGE.SEQ"},
    /* 442 */ {"STG06\\M000.LBD"},
    /* 443 */ {"STG06\\M001.LBD"},
    /* 444 */ {"STG06\\M002.LBD"},
    /* 445 */ {"STG06\\M003.LBD"},
    /* 446 */ {"STG06\\M004.LBD"},
    /* 447 */ {"STG06\\M005.LBD"},
    /* 448 */ {"STG07\\TEXA.TIX"},
    /* 449 */ {"STG07\\TEXB.TIX"},
    /* 450 */ {"STG07\\TEXC.TIX"},
    /* 451 */ {"STG07\\TEXD.TIX"},
    /* 452 */ {"STG07\\BGA.SEQ"},
    /* 453 */ {"STG07\\BGB.SEQ"},
    /* 454 */ {"STG07\\BGC.SEQ"},
    /* 455 */ {"STG07\\BGD.SEQ"},
    /* 456 */ {"STG07\\BGE.SEQ"},
    /* 457 */ {"STG07\\M000.LBD"},
    /* 458 */ {"STG07\\M001.LBD"},
    /* 459 */ {"STG07\\M002.LBD"},
    /* 460 */ {"STG07\\M003.LBD"},
    /* 461 */ {"STG07\\M004.LBD"},
    /* 462 */ {"STG08\\TEXA.TIX"},
    /* 463 */ {"STG08\\TEXB.TIX"},
    /* 464 */ {"STG08\\TEXC.TIX"},
    /* 465 */ {"STG08\\TEXD.TIX"},
    /* 466 */ {"STG08\\BGA.SEQ"},
    /* 467 */ {"STG08\\BGB.SEQ"},
    /* 468 */ {"STG08\\BGC.SEQ"},
    /* 469 */ {"STG08\\BGD.SEQ"},
    /* 470 */ {"STG08\\BGE.SEQ"},
    /* 471 */ {"STG08\\M000.LBD"},
    /* 472 */ {"STG08\\M001.LBD"},
    /* 473 */ {"STG08\\M002.LBD"},
    /* 474 */ {"STG09\\TEXA.TIX"},
    /* 475 */ {"STG09\\TEXB.TIX"},
    /* 476 */ {"STG09\\TEXC.TIX"},
    /* 477 */ {"STG09\\TEXD.TIX"},
    /* 478 */ {"STG09\\BGA.SEQ"},
    /* 479 */ {"STG09\\BGB.SEQ"},
    /* 480 */ {"STG09\\BGC.SEQ"},
    /* 481 */ {"STG09\\BGD.SEQ"},
    /* 482 */ {"STG09\\BGE.SEQ"},
    /* 483 */ {"STG09\\M000.LBD"},
    /* 484 */ {"STG09\\M001.LBD"},
    /* 485 */ {"STG10\\TEXA.TIX"},
    /* 486 */ {"STG10\\TEXB.TIX"},
    /* 487 */ {"STG10\\TEXC.TIX"},
    /* 488 */ {"STG10\\TEXD.TIX"},
    /* 489 */ {"STG10\\BGA.SEQ"},
    /* 490 */ {"STG10\\BGB.SEQ"},
    /* 491 */ {"STG10\\BGC.SEQ"},
    /* 492 */ {"STG10\\BGD.SEQ"},
    /* 493 */ {"STG10\\BGE.SEQ"},
    /* 494 */ {"STG10\\M000.LBD"},
    /* 495 */ {"STG10\\M001.LBD"},
    /* 496 */ {"STG10\\M002.LBD"},
    /* 497 */ {"STG11\\TEXA.TIX"},
    /* 498 */ {"STG11\\TEXB.TIX"},
    /* 499 */ {"STG11\\TEXC.TIX"},
    /* 500 */ {"STG11\\TEXD.TIX"},
    /* 501 */ {"STG11\\BGA.SEQ"},
    /* 502 */ {"STG11\\BGB.SEQ"},
    /* 503 */ {"STG11\\BGC.SEQ"},
    /* 504 */ {"STG11\\BGD.SEQ"},
    /* 505 */ {"STG11\\BGE.SEQ"},
    /* 506 */ {"STG11\\M000.LBD"},
    /* 507 */ {"STG11\\M001.LBD"},
    /* 508 */ {"STG11\\M002.LBD"},
    /* 509 */ {"STG11\\M003.LBD"},
    /* 510 */ {"STG11\\M004.LBD"},
    /* 511 */ {"STG11\\M005.LBD"},
    /* 512 */ {"STG11\\M006.LBD"},
    /* 513 */ {"STG11\\M007.LBD"},
    /* 514 */ {"STG11\\M008.LBD"},
    /* 515 */ {"STG11\\M009.LBD"},
    /* 516 */ {"STG11\\M010.LBD"},
    /* 517 */ {"STG11\\M011.LBD"},
    /* 518 */ {"STG12\\TEXA.TIX"},
    /* 519 */ {"STG12\\TEXB.TIX"},
    /* 520 */ {"STG12\\TEXC.TIX"},
    /* 521 */ {"STG12\\TEXD.TIX"},
    /* 522 */ {"STG12\\BGA.SEQ"},
    /* 523 */ {"STG12\\BGB.SEQ"},
    /* 524 */ {"STG12\\BGC.SEQ"},
    /* 525 */ {"STG12\\BGD.SEQ"},
    /* 526 */ {"STG12\\BGE.SEQ"},
    /* 527 */ {"STG12\\M000.LBD"},
    /* 528 */ {"STG12\\M001.LBD"},
    /* 529 */ {"STG12\\M002.LBD"},
    /* 530 */ {"STG12\\M003.LBD"},
    /* 531 */ {"STG12\\M004.LBD"},
    /* 532 */ {"STG12\\M005.LBD"},
    /* 533 */ {"STG12\\M006.LBD"},
    /* 534 */ {"STG12\\M007.LBD"},
    /* 535 */ {"STG12\\M008.LBD"},
    /* 536 */ {"STG12\\M009.LBD"},
    /* 537 */ {"STG12\\M010.LBD"},
    /* 538 */ {"STG12\\M011.LBD"},
    /* 539 */ {"STG12\\M012.LBD"},
    /* 540 */ {"STG12\\M013.LBD"},
    /* 541 */ {"STG12\\M014.LBD"},
    /* 542 */ {"STG12\\M015.LBD"},
    /* 543 */ {"STG12\\M016.LBD"},
    /* 544 */ {"STG12\\M017.LBD"},
    /* 545 */ {"STG12\\M018.LBD"},
    /* 546 */ {"STG12\\M019.LBD"},
    /* 547 */ {"STG13\\TEXA.TIX"},
    /* 548 */ {"STG13\\TEXB.TIX"},
    /* 549 */ {"STG13\\TEXC.TIX"},
    /* 550 */ {"STG13\\TEXD.TIX"},
    /* 551 */ {"STG13\\BGA.SEQ"},
    /* 552 */ {"STG13\\BGB.SEQ"},
    /* 553 */ {"STG13\\BGC.SEQ"},
    /* 554 */ {"STG13\\BGD.SEQ"},
    /* 555 */ {"STG13\\BGE.SEQ"},
    /* 556 */ {"STG13\\M000.LBD"},
    /* 557 */ {"STG13\\M001.LBD"},
    /* 558 */ {"STG13\\M002.LBD"},
    /* 559 */ {"STG13\\M003.LBD"},
    /* 560 */ {"ETC\\OPENINGA.STR"},
    /* 561 */ {"ETC\\OPENINGB.STR"},
    /* 562 */ {"ETC\\OPENINGC.STR"},
    /* 563 */ {"ETC\\OPENINGD.STR"},
    /* 564 */ {"ETC\\OPENINGE.STR"},
    /* 565 */ {"ETC\\OPENINGF.STR"},
    /* 566 */ {"ETC\\OPENINGG.STR"},
    /* 567 */ {"ETC\\ENDING.STR"},
    /* 568 */ {"FILM\\EVENT1.STR"},
    /* 569 */ {"FILM\\EVENT2.STR"},
    /* 570 */ {"FILM\\EVENT3.STR"},
    /* 571 */ {"FILM\\EVENT4.STR"},
    /* 572 */ {"FILM\\EVENT5.STR"},
    /* 573 */ {"FILM\\EVENT6.STR"},
    /* 574 */ {"FILM\\SPDAY01A.STR"},
    /* 575 */ {"FILM\\SPDAY01B.STR"},
    /* 576 */ {"IMG1\\SPDAY01C.TIM"},
    /* 577 */ {"IMG1\\SPDAY01D.TIM"},
    /* 578 */ {"IMG1\\SPDAY01E.TIM"},
    /* 579 */ {"IMG1\\SPDAY01F.TIM"},
    /* 580 */ {"FILM\\SPDAY02A.STR"},
    /* 581 */ {"FILM\\SPDAY02B.STR"},
    /* 582 */ {"IMG1\\SPDAY02C.TIM"},
    /* 583 */ {"IMG1\\SPDAY02D.TIM"},
    /* 584 */ {"IMG1\\SPDAY02E.TIM"},
    /* 585 */ {"IMG1\\SPDAY02F.TIM"},
    /* 586 */ {"FILM\\SPDAY03A.STR"},
    /* 587 */ {"FILM\\SPDAY03B.STR"},
    /* 588 */ {"IMG1\\SPDAY03C.TIM"},
    /* 589 */ {"IMG1\\SPDAY03D.TIM"},
    /* 590 */ {"IMG1\\SPDAY03E.TIM"},
    /* 591 */ {"IMG1\\SPDAY03F.TIM"},
    /* 592 */ {"FILM\\SPDAY04A.STR"},
    /* 593 */ {"FILM\\SPDAY04B.STR"},
    /* 594 */ {"IMG1\\SPDAY04C.TIM"},
    /* 595 */ {"IMG1\\SPDAY04D.TIM"},
    /* 596 */ {"IMG1\\SPDAY04E.TIM"},
    /* 597 */ {"IMG1\\SPDAY04F.TIM"},
    /* 598 */ {"FILM\\SPDAY05A.STR"},
    /* 599 */ {"FILM\\SPDAY05B.STR"},
    /* 600 */ {"IMG1\\SPDAY05C.TIM"},
    /* 601 */ {"IMG1\\SPDAY05D.TIM"},
    /* 602 */ {"IMG1\\SPDAY05E.TIM"},
    /* 603 */ {"IMG1\\SPDAY05F.TIM"},
    /* 604 */ {"FILM\\SPDAY06A.STR"},
    /* 605 */ {"FILM\\SPDAY06B.STR"},
    /* 606 */ {"IMG1\\SPDAY06C.TIM"},
    /* 607 */ {"IMG1\\SPDAY06D.TIM"},
    /* 608 */ {"IMG1\\SPDAY06E.TIM"},
    /* 609 */ {"IMG1\\SPDAY06F.TIM"},
    /* 610 */ {"FILM\\SPDAY07A.STR"},
    /* 611 */ {"FILM\\SPDAY07B.STR"},
    /* 612 */ {"IMG2\\SPDAY07C.TIM"},
    /* 613 */ {"IMG2\\SPDAY07D.TIM"},
    /* 614 */ {"IMG2\\SPDAY07E.TIM"},
    /* 615 */ {"IMG2\\SPDAY07F.TIM"},
    /* 616 */ {"FILM\\SPDAY08A.STR"},
    /* 617 */ {"FILM\\SPDAY08B.STR"},
    /* 618 */ {"IMG2\\SPDAY08C.TIM"},
    /* 619 */ {"IMG2\\SPDAY08D.TIM"},
    /* 620 */ {"IMG2\\SPDAY08E.TIM"},
    /* 621 */ {"IMG2\\SPDAY08F.TIM"},
    /* 622 */ {"FILM\\SPDAY09A.STR"},
    /* 623 */ {"FILM\\SPDAY09B.STR"},
    /* 624 */ {"IMG2\\SPDAY09C.TIM"},
    /* 625 */ {"IMG2\\SPDAY09D.TIM"},
    /* 626 */ {"IMG2\\SPDAY09E.TIM"},
    /* 627 */ {"IMG2\\SPDAY09F.TIM"},
    /* 628 */ {"FILM\\SPDAY10A.STR"},
    /* 629 */ {"FILM\\SPDAY10B.STR"},
    /* 630 */ {"IMG2\\SPDAY10C.TIM"},
    /* 631 */ {"IMG2\\SPDAY10D.TIM"},
    /* 632 */ {"IMG2\\SPDAY10E.TIM"},
    /* 633 */ {"IMG2\\SPDAY10F.TIM"},
    /* 634 */ {"FILM\\SPDAY11A.STR"},
    /* 635 */ {"FILM\\SPDAY11B.STR"},
    /* 636 */ {"IMG2\\SPDAY11C.TIM"},
    /* 637 */ {"IMG2\\SPDAY11D.TIM"},
    /* 638 */ {"IMG2\\SPDAY11E.TIM"},
    /* 639 */ {"IMG2\\SPDAY11F.TIM"},
    /* 640 */ {"FILM\\SPDAY12A.STR"},
    /* 641 */ {"FILM\\SPDAY12B.STR"},
    /* 642 */ {"IMG2\\SPDAY12C.TIM"},
    /* 643 */ {"IMG2\\SPDAY12D.TIM"},
    /* 644 */ {"IMG2\\SPDAY12E.TIM"},
    /* 645 */ {"IMG2\\SPDAY12F.TIM"},
    /* 646 */ {"ETC\\FONTICON.TIM"},
    /* 647 */ {"ETC\\ASMKLOGO.TIM"},
    /* 648 */ {"ETC\\OSDLOGO.TIM"},
    /* 649 */ {"ETC\\TITLE.TIM"},
    /* 650 */ {"ETC\\HGRAPH.TIM"},
    /* 651 */ {"ETC\\ETCSE.VH"},
    /* 652 */ {"ETC\\ETCSE.VB"},
};
/* clang-format on */

/* Each movie's frame count, by movie id (enum MovieId). Ids 38 to 48 are
 * no movie record's. */
/* clang-format off */
s16 sMovieFrameCounts[MOVIE_COUNT] = {
    /*  0 */ 1060, /* ETC\OPENINGA.STR */
    /*  1 */ 1088, /* ETC\OPENINGB.STR */
    /*  2 */ 953,  /* ETC\OPENINGC.STR */
    /*  3 */ 1179, /* ETC\OPENINGD.STR */
    /*  4 */ 979,  /* ETC\OPENINGE.STR */
    /*  5 */ 1231, /* ETC\OPENINGF.STR */
    /*  6 */ 1046, /* ETC\OPENINGG.STR */
    /*  7 */ 474,  /* ETC\ENDING.STR */
    /*  8 */ 524,  /* FILM\EVENT1.STR */
    /*  9 */ 439,  /* FILM\EVENT2.STR */
    /* 10 */ 349,  /* FILM\EVENT3.STR */
    /* 11 */ 379,  /* FILM\EVENT4.STR */
    /* 12 */ 352,  /* FILM\EVENT5.STR */
    /* 13 */ 352,  /* FILM\EVENT6.STR */
    /* 14 */ 475,  /* FILM\SPDAY01A.STR */
    /* 15 */ 867,  /* FILM\SPDAY01B.STR */
    /* 16 */ 527,  /* FILM\SPDAY02A.STR */
    /* 17 */ 445,  /* FILM\SPDAY02B.STR */
    /* 18 */ 399,  /* FILM\SPDAY03A.STR */
    /* 19 */ 579,  /* FILM\SPDAY03B.STR */
    /* 20 */ 264,  /* FILM\SPDAY04A.STR */
    /* 21 */ 541,  /* FILM\SPDAY04B.STR */
    /* 22 */ 566,  /* FILM\SPDAY05A.STR */
    /* 23 */ 325,  /* FILM\SPDAY05B.STR */
    /* 24 */ 436,  /* FILM\SPDAY06A.STR */
    /* 25 */ 524,  /* FILM\SPDAY06B.STR */
    /* 26 */ 522,  /* FILM\SPDAY07A.STR */
    /* 27 */ 523,  /* FILM\SPDAY07B.STR */
    /* 28 */ 649,  /* FILM\SPDAY08A.STR */
    /* 29 */ 451,  /* FILM\SPDAY08B.STR */
    /* 30 */ 465,  /* FILM\SPDAY09A.STR */
    /* 31 */ 473,  /* FILM\SPDAY09B.STR */
    /* 32 */ 411,  /* FILM\SPDAY10A.STR */
    /* 33 */ 490,  /* FILM\SPDAY10B.STR */
    /* 34 */ 467,  /* FILM\SPDAY11A.STR */
    /* 35 */ 523,  /* FILM\SPDAY11B.STR */
    /* 36 */ 187,  /* FILM\SPDAY12A.STR */
    /* 37 */ 384,  /* FILM\SPDAY12B.STR */
    /* 38 */ 90,
    /* 39 */ 90,
    /* 40 */ 90,
    /* 41 */ 90,
    /* 42 */ 90,
    /* 43 */ 90,
    /* 44 */ 90,
    /* 45 */ 90,
    /* 46 */ 90,
    /* 47 */ 90,
    /* 48 */ -10,
    /* 49 */ 90,   /* ETC\ASMK.STR */
};
/* clang-format on */
