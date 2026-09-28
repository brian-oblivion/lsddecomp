#ifndef GAMEFILES_H
#define GAMEFILES_H

/*
 * The getters over the game's table of file names, src/cd/GameFiles.c (the
 * LbdFile class it also holds is include/LbdFile.h). A record is one
 * sRecordTable entry, a CdFileEntry (include/CdDriver.h) whose name is a
 * zero-padded path: RegisterRecordTableFiles hands the table to the CD
 * driver as its file table, which fills in each entry's position and size.
 * The file's banner lists the table's layout. The movie getters also write a movie id through movieIdOut, which
 * GetMovieFrameCount turns into the movie's frame count.
 */

#include "common.h"
#include "CdDriver.h"

/* Each special day's six records, of which a CinematicCall's entry picks
 * one: FILM\SPDAYnnA/B.STR (its two movies), then IMG1\SPDAYnnC..F.TIM.
 * Special day 0 is SPDAY01. */
#define SPECIAL_DAY_RECORD_COUNT 6
#define SPECIAL_DAY_MOVIE_COUNT 2

/* The cinematic DreamSys's getCinematic names, and what
 * GetSpecialDayOrEventRecord takes: bank is a special
 * day and entry one of its records, or, with bank negative, entry is an
 * event movie. entry -1 is no cinematic. */
typedef struct CinematicCall {
    s16 bank;
    s16 entry;
} CinematicCall;

/* "CDI\\", the data directory a GameApplication starts with. */
extern char *GetDefaultDataDirectory(void);

/* rand(), after srand(seed) when seed is nonzero. */
extern s32 SeedAndRandom(s32 seed, s32 unused);

/* Forces PickSoundBank's and PickStageBgm's choice (1-based, 0 for random);
 * a negative argument leaves that one as it was. */
extern void SetPickOverrides(s32 soundBank, s32 stageBgm);

/* The record table; *countOut (when not NULL) is its count of sound bank
 * and stage records, the ones before the movies. */
extern CdFileEntry *GetRecordTable(s32 *countOut);

/* The seven SND\name sound bank paths, one word each. */
extern void *GetSoundBankPaths(void);

/* One of the SND\name paths WBgm opens as its VAB, forced or random. */
extern char *PickSoundBank(s32 unused);

/* "SND\\SE", the sound effect bank, and the word that holds it. */
extern char **GetSoundEffectDirRef(void);
extern char *GetSoundEffectDir(); /* MATCHING: unprototyped, DayTask's ctor passes a dead argument retail loads (arity-ok: dead argument) */

/* A stage's records, its four textures, its five BGM sequences and its map
 * chunks, and the pickers over them. */
extern CdFileEntry *GetStageRecords(s32 stage);
extern CdFileEntry *GetStageTextureRecords(s32 stage);
extern CdFileEntry *PickStageTexture(s32 stage, s32 unused, s32 day);
extern CdFileEntry *GetStageBgmRecords(s32 stage);
extern CdFileEntry *PickStageBgm(s32 stage, s32 unused);
extern CdFileEntry *GetStageMapChunkRecords(s32 stage);
extern CdFileEntry *GetStageMapChunkRecord(s32 stage, s32 chunk);
extern CdFileEntry *GetStageMapChunkRecordXY(s32 stage, s32 x, s32 y);

/* The movies: "ETC\\ASMK.STR", the seven openings (one picked at random),
 * the ending, the six events and each special day's six records. */
extern const char *GetAsmkMovie(s32 *movieIdOut);
extern CdFileEntry *GetOpeningMovieRecords(s32 *movieIdOut);
extern CdFileEntry *PickOpeningMovie(s32 *movieIdOut, s32 unused);
extern CdFileEntry *GetEndingMovieRecord(s32 *movieIdOut);
extern CdFileEntry *GetEndingMovie(); /* MATCHING: unprototyped, PlayEndingMovie passes a dead second argument retail loads (arity-ok: dead argument) */
extern CdFileEntry *GetEventMovieRecords(s32 *movieIdOut);
extern CdFileEntry *GetEventMovie(s32 *movieIdOut, s32 event);
extern CdFileEntry *GetSpecialDayRecords(s32 *movieIdOut, s32 day);

/* A special day's record or an event movie, for the pair DreamSys's
 * getCinematic returns: *movieIdOut is -1 for a special day's TIM images. */
extern CdFileEntry *GetSpecialDayOrEventRecord(s32 *movieIdOut, CinematicCall pick);

extern s32 GetMovieFrameCount(s32 movieId);

/* The first movie of special day `day`; *frameTotal is the frame count of
 * the movies of dayCount special days from it. */
extern CdFileEntry *GetSpecialDayMovieSpan(s32 *frameTotal, s32 day, s32 dayCount);

#endif
