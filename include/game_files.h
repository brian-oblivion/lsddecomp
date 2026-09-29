#ifndef GAME_FILES_H
#define GAME_FILES_H

/**
 * @file game_files.h
 * @brief The getters over the game's table of file names (the record table),
 * in src/cd/game_files.c.
 *
 * A record is one CdFileEntry (cd_driver.h) of the record table, whose name
 * is a zero-padded path; RegisterRecordTableFiles (src/world/dream_day.c)
 * hands the table to the CD driver as its file table, which fills in each
 * entry's disc position and size. In order, the table holds:
 *  - the seven sound banks' SND\\name.VH/VB pairs and SND\\SE.VH/VB;
 *  - each stage's files, from the stage's first record: its four textures
 *    (TEXA..TEXD.TIX), five BGM sequences (BGA..BGE.SEQ) and map chunks
 *    (Mnnn.LBD, laid out as stage_grid.h's grid);
 *  - after the GetRecordTable count, the movies (ETC\\OPENINGA..G.STR,
 *    ETC\\ENDING.STR, FILM\\EVENTn.STR), then six records per special day
 *    (FILM\\SPDAYnnA/B.STR, IMG1\\SPDAYnnC..F.TIM).
 *
 * The stage getters return a record, used as a path: PickStageBgm's goes to
 * the WBgm's setSeq, PickStageTexture's to New_TimBlockSrc, and a map chunk's
 * to an LbdFile (lbd_file.h, whose class the same file holds). The movie
 * getters also hand back a movie id through `movieIdOut`, which
 * GetMovieFrameCount turns into the movie's frame count for the MoviePlayer.
 * The random pickers draw through SeedAndRandom; SetPickOverrides forces
 * PickSoundBank's and PickStageBgm's choice.
 */

#include "common.h"
#include "cd_driver.h"

/** @name Special day records
 * Each special day's six records, of which a CinematicCall's entry picks
 * one: FILM\\SPDAYnnA/B.STR (its two movies), then IMG1\\SPDAYnnC..F.TIM.
 * Special day 0 is SPDAY01. @{ */
#define SPECIAL_DAY_RECORD_COUNT 6 /**< records per special day */
#define SPECIAL_DAY_MOVIE_COUNT 2  /**< of them, the leading .STR movies */

/** @} */

/**
 * @brief The cinematic DreamSys's getCinematic names, and what
 * GetSpecialDayOrEventRecord takes.
 */
typedef struct CinematicCall {
    s16 bank;  /**< A special day, or negative for an event movie. */
    s16 entry; /**< One of the special day's records, or the event movie; -1 is no cinematic. */
} CinematicCall;

/**
 * @brief The data directory a GameApplication starts with.
 * @return "CDI\\".
 */
extern char *GetDefaultDataDirectory(void);

/**
 * @brief rand(), after srand(seed) when `seed` is nonzero.
 * @param seed   The seed, or 0 to continue the sequence.
 * @param unused Ignored.
 * @return rand()'s result.
 */
extern s32 SeedAndRandom(s32 seed, s32 unused);

/**
 * @brief Forces the choice PickSoundBank and PickStageBgm make.
 * @param soundBank The sound bank, 1-based, 0 for random; negative leaves it.
 * @param stageBgm  The stage BGM, 1-based, 0 for random; negative leaves it.
 */
extern void SetPickOverrides(s32 soundBank, s32 stageBgm);

/**
 * @brief The record table.
 * @param countOut Receives the count of sound bank and stage records, the
 *                 ones before the movies; may be NULL.
 * @return The table's first record.
 */
extern CdFileEntry *GetRecordTable(s32 *countOut);

/**
 * @brief The seven SND\\name sound bank paths.
 * @return An array of seven `char *`.
 */
extern void *GetSoundBankPaths(void);

/**
 * @brief One of the seven sound bank paths, the forced one or a random one;
 * WBgm opens it as its VAB.
 * @param unused Passed to SeedAndRandom, which ignores it.
 * @return The path.
 */
extern char *PickSoundBank(s32 unused);

/**
 * @brief The word that holds the sound effect bank's path.
 * @return Its address.
 */
extern char **GetSoundEffectDirRef(void);

/**
 * @brief The sound effect bank's path.
 * @return "SND\\SE".
 */
extern char *GetSoundEffectDir(); /* arity-ok: DayTask's ctor passes an argument it ignores */

/**
 * @brief A stage's records: its textures, then its BGM sequences, then its
 * map chunks.
 * @param stage The stage.
 * @return The stage's first record.
 */
extern CdFileEntry *GetStageRecords(s32 stage);

/**
 * @brief A stage's four texture records.
 * @param stage The stage.
 * @return The first texture record.
 */
extern CdFileEntry *GetStageTextureRecords(s32 stage);

/**
 * @brief A random texture record among the first one to four of the stage's,
 * one more every ten days, repeating every forty.
 * @param stage  The stage.
 * @param unused Passed to SeedAndRandom, which ignores it.
 * @param day    The day number, from 1.
 * @return The texture record.
 */
extern CdFileEntry *PickStageTexture(s32 stage, s32 unused, s32 day);

/**
 * @brief A stage's five BGM sequence records.
 * @param stage The stage.
 * @return The first BGM record.
 */
extern CdFileEntry *GetStageBgmRecords(s32 stage);

/**
 * @brief The forced or a random BGM record of the stage. Stage 9 never plays
 * BGC.SEQ: a random pick of it becomes BGD, and a forced 3 becomes 4.
 * @param stage  The stage.
 * @param unused Passed to SeedAndRandom, which ignores it.
 * @return The BGM record.
 */
extern CdFileEntry *PickStageBgm(s32 stage, s32 unused);

/**
 * @brief A stage's map-chunk records.
 * @param stage The stage.
 * @return The first map-chunk record.
 */
extern CdFileEntry *GetStageMapChunkRecords(s32 stage);

/**
 * @brief One map-chunk record of a stage.
 * @param stage The stage.
 * @param chunk The chunk index.
 * @return The record.
 */
extern CdFileEntry *GetStageMapChunkRecord(s32 stage, s32 chunk);

/**
 * @brief The map-chunk record at column `x`, row `y` of the stage's grid.
 * @param stage The stage.
 * @param x     Column.
 * @param y     Row.
 * @return The record.
 */
extern CdFileEntry *GetStageMapChunkRecordXY(s32 stage, s32 x, s32 y);

/**
 * @brief The movie ETC\\ASMK.STR.
 * @param movieIdOut Receives its movie id; may be NULL.
 * @return "ETC\\ASMK.STR".
 */
extern const char *GetAsmkMovie(s32 *movieIdOut);

/**
 * @brief The seven opening movies' records.
 * @param movieIdOut Receives the first one's movie id; may be NULL.
 * @return The first opening movie's record.
 */
extern CdFileEntry *GetOpeningMovieRecords(s32 *movieIdOut);

/**
 * @brief A random opening movie.
 * @param movieIdOut Receives its movie id; may be NULL.
 * @param unused     Passed to SeedAndRandom, which ignores it.
 * @return Its record.
 */
extern CdFileEntry *PickOpeningMovie(s32 *movieIdOut, s32 unused);

/**
 * @brief The ending movie's record.
 * @param movieIdOut Receives its movie id; may be NULL.
 * @return The record.
 */
extern CdFileEntry *GetEndingMovieRecord(s32 *movieIdOut);

/**
 * @brief The ending movie, through GetEndingMovieRecord. Declared without
 * a prototype; the definition takes `s32 *movieIdOut`, which receives the
 * movie id and may be NULL.
 * @return The record.
 */
extern CdFileEntry *GetEndingMovie(); /* arity-ok: PlayEndingMovie passes a second argument it ignores */

/**
 * @brief The six event movies' records.
 * @param movieIdOut Receives the first one's movie id; may be NULL.
 * @return The first event movie's record.
 */
extern CdFileEntry *GetEventMovieRecords(s32 *movieIdOut);

/**
 * @brief One event movie.
 * @param movieIdOut Receives its movie id; may be NULL.
 * @param event      The event, 0-based (FILM\\EVENT1.STR is 0).
 * @return Its record.
 */
extern CdFileEntry *GetEventMovie(s32 *movieIdOut, s32 event);

/**
 * @brief A special day's six records.
 * @param movieIdOut Receives its first movie's id; may be NULL.
 * @param day        The special day, 0-based.
 * @return Its first record.
 */
extern CdFileEntry *GetSpecialDayRecords(s32 *movieIdOut, s32 day);

/**
 * @brief A special day's record or an event movie, for the pair DreamSys's
 * getCinematic returns.
 * @param movieIdOut Receives the movie id, or -1 for a special day's TIM
 *                   images; may be NULL.
 * @param pick       The special day and record, or the event movie.
 * @return The record.
 */
extern CdFileEntry *GetSpecialDayOrEventRecord(s32 *movieIdOut, CinematicCall pick);

/**
 * @brief A movie's frame count.
 * @param movieId A movie id from one of the movie getters.
 * @return Its frame count.
 */
extern s32 GetMovieFrameCount(s32 movieId);

/**
 * @brief The first movie of special day `day`, and the frame count of the
 * movies of `dayCount` special days from it, with a ten-frame gap between
 * each.
 * @param frameTotal Receives the frame count.
 * @param day        The first special day, 0-based.
 * @param dayCount   How many special days.
 * @return The first movie's record.
 */
extern CdFileEntry *GetSpecialDayMovieSpan(s32 *frameTotal, s32 day, s32 dayCount);

#endif
