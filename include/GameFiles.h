#ifndef GAMEFILES_H
#define GAMEFILES_H

/*
 * The getters over the game's table of file names, src/cd/GameFiles.c (the
 * LbdFile class it also holds is include/LbdFile.h). A record is one
 * zero-padded path in gRecordTable; the file's banner lists the table's
 * layout. The movie getters also write a movie id through movieIdOut, which
 * GetMovieFrameCount turns into the movie's frame count.
 */

#include "common.h"

/* One gRecordTable record: a file path, zero-padded to 0x1C bytes. */
typedef struct FilePathRecord {
    u8 data[0x1C];
} FilePathRecord;

/* "CDI\\", the data directory a GameApplication starts with. */
extern char *GetDefaultDataDirectory(void);

/* rand(), after srand(seed) when seed is nonzero. */
extern s32 SeedAndRandom(s32 seed, s32 unused);

/* Forces PickSoundBank's and PickStageBgm's choice (1-based, 0 for random);
 * a negative argument leaves that one as it was. */
extern void SetPickOverrides(s32 soundBank, s32 stageBgm);

/* The record table; *countOut (when not NULL) is its count of sound bank
 * and stage records, the ones before the movies. */
extern void *GetRecordTable(s32 *countOut);

/* The seven SND\name sound bank paths, one word each. */
extern void *GetSoundBankPaths(void);

/* One of the SND\name paths WBgm opens as its VAB, forced or random. */
extern char *PickSoundBank(s32 unused);

/* "SND\\SE", the sound effect bank, and the word that holds it. */
extern char **GetSoundEffectDirRef(void);
extern char *GetSoundEffectDir(); /* MATCHING: unprototyped, DayTask's ctor passes a dead argument retail loads (arity-ok: dead argument) */

/* A stage's records, its four textures, its five BGM sequences and its map
 * chunks, and the pickers over them. */
extern FilePathRecord *GetStageRecords(s32 stage);
extern FilePathRecord *GetStageTextureRecords(s32 stage);
extern FilePathRecord *PickStageTexture(s32 stage, s32 unused, s32 day);
extern FilePathRecord *GetStageBgmRecords(s32 stage);
extern FilePathRecord *PickStageBgm(s32 stage, s32 unused);
extern FilePathRecord *GetStageMapChunkRecords(s32 stage);
extern FilePathRecord *GetStageMapChunkRecord(s32 stage, s32 chunk);
extern FilePathRecord *GetStageMapChunkRecordXY(s32 stage, s32 x, s32 y);

/* The movies: "ETC\\ASMK.STR", the seven openings (one picked at random),
 * the ending, the six events and each special day's six records. */
extern const char *GetAsmkMovie(s32 *movieIdOut);
extern FilePathRecord *GetOpeningMovieRecords(s32 *movieIdOut);
extern FilePathRecord *PickOpeningMovie(s32 *movieIdOut, s32 unused);
extern FilePathRecord *GetEndingMovieRecord(s32 *movieIdOut);
extern FilePathRecord *GetEndingMovie(); /* MATCHING: unprototyped, PlayEndingMovie passes a dead second argument retail loads (arity-ok: dead argument) */
extern FilePathRecord *GetEventMovieRecords(s32 *movieIdOut);
extern FilePathRecord *GetEventMovie(s32 *movieIdOut, s32 event);
extern FilePathRecord *GetSpecialDayRecords(s32 *movieIdOut, s32 day);

/* A special day's record or an event movie, for DreamSys's getCinematic pair
 * (s32 *movieIdOut, RecPick pick): *movieIdOut is -1 for a special day's TIM
 * images. */
extern FilePathRecord *GetSpecialDayOrEventRecord(); /* MATCHING: unprototyped, PlayCinematic passes the pair packed into one word (arity-ok: one register) */

extern s32 GetMovieFrameCount(s32 movieId);

/* The first movie of special day `day`; *frameTotal is the frame count of
 * the movies of dayCount special days from it. */
extern FilePathRecord *GetSpecialDayMovieSpan(s32 *frameTotal, s32 day, s32 dayCount);

#endif
