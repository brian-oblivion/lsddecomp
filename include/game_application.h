#ifndef GAME_APPLICATION_H
#define GAME_APPLICATION_H

#include "application.h"

/**
 * @file game_application.h
 * @brief GameApplication, the game itself as an Application: the sequence of
 * logos, movies, title menu and days the outer loop runs.
 *
 * Declares the class, its configuration record and its methods, defined in
 * src/app/game_shell.c.
 */

typedef struct GameApplication GameApplication;
typedef struct GameApplicationMethods GameApplicationMethods;

/** The ctor's argument: which parts of the sequence run. One instance,
 * sGameApplicationConfig = {DATASOURCE_CD, 0, 1, 1, 1, 1}, kept at self->config; each
 * gate below is read `!= 0` by the methods it names. */
typedef struct GameApplicationConfig {
    /* +0x00 */ s32 dataSource; /**< Application's ctor argument: DATASOURCE_CD (data_source.h) */

    /** New_DayTask's syncDriver, in RunDayTask: that ctor passes
     * (syncDriver == 0) to SetActiveDataSourceDriverMode. */
    /* +0x04 */ s32 dayTaskSyncDriver;
    /** Gates every movie after the intro: PlayOpeningMovie,
     * PlaySpecialDayMovies, PlayCinematic's movie branch, PlayEndingMovie. */
    /* +0x08 */ s32 playStreams;
    /* +0x0C */ s32 showIntroLogos; /**< gates ShowIntroLogos */
    /* +0x10 */ s32 pollGraphRoom;  /**< gates RunTitleMenu (0: it returns 2 at once) */

    /** The ctor passes it to the DreamSys's getSetConfigOption
     * (DreamSys__GetSetConfigOption), which stores a value >= 0 in
     * DreamSys::configOption. DreamSys__ResetSessionState zeroes that field
     * and no code but the get/set reads it, so what it selects is not
     * established (sGameApplicationConfig passes 1). */
    /* +0x14 */ s32 dreamSysConfigOption;
} GameApplicationConfig;

/** GameApplication's method table: Application's slots, flat. Two overrides
 * take a different parameter list from the slot they fill, so the slot
 * keeps the inherited type and the caller casts:
 *  - +0x040 setScreenDims <- GameApplication__SeedRandom (self only); the
 *    ctor calls it through GameApplicationSeedRandomFn. Application's own
 *    ctor ran under Application's table, so the default screen is set.
 *  - +0x044 initSystems <- GameApplication__InitSystems (no 4th argument);
 *    main calls it through GameApplicationInitSystemsFn. */
struct GameApplicationMethods {
    APPLICATION_SLOTS(GameApplication, (GameApplication * self, GameApplicationConfig *args));
};

/**
 * GameApplication -- the game itself, as an Application (application.h):
 * the one object main() builds, New_GameApplication(&sGameApplicationConfig)
 * into sGameApplication, before running its initSystems and then
 * runMainLoop, which never returns. Application brings the console up and
 * owns the outer loop; this class fills the loop's six hooks with the game's
 * sequence and owns the game's DreamSys (dream_sys.h). Class id 0x1F60,
 * method table gGameApplicationMethods; methods in src/app/game_shell.c. No
 * class derives from it.
 *
 * Lifecycle:
 *  - ctor(config): Application's ctor with config->dataSource, this table,
 *    the config kept, the data directory reset to its default, the DreamSys
 *    built from "ETC\DREAME5.TMD" (New_LinkResource),
 *    config->dreamSysConfigOption handed to it, and the RNG seeded
 *    (setScreenDims's occupant).
 *  - initSystems: Application's, unless already initialized.
 *  - runMainLoop: Application's: once showIntroLogos, then forever
 *    playOpeningMovie and the runTitleMenu loop.
 *
 * The hooks, in the order runMainLoop calls them:
 *  - +0x050 ShowIntroLogos: the ASMK logo, the ASMK movie, the OSD logo;
 *  - +0x054 PlayOpeningMovie: an opening movie, picked at random;
 *  - +0x058 RunTitleMenu: GraphRoom and TitleMenu against the DreamSys;
 *    returns 0 or 2 to the loop (enum ApplicationLoopStatus);
 *  - +0x05C OnRepeatMenu: empty (status 1 is never returned);
 *  - +0x060 RunDayTask: one DayTask (day_task.h), then maybe the current
 *    cinematic; nonzero (a year gone by) runs +0x064;
 *  - +0x064 PlayEndingMovie: ETC\ENDING.STR.
 *
 * Every hook but RunDayTask first calls SetActiveDataSourceDriverMode(0, 0,
 * 0). Every task these start is given the Application's `aux` as its
 * IntermediateBaseInitArgs, runs to its end inside init, and is released. A
 * movie is a StreamTask given the path and movie id the game_files.h getters
 * return, the id turned into a frame count by GetMovieFrameCount; an image is
 * a TaskCore showing the TIM. Every movie but the intro's is gated by
 * config->playStreams.
 *
 * Object size 0x2C (New_GameApplication); its own fields start at +0x020.
 */
struct GameApplication {
    APPLICATION_FIELDS(GameApplicationMethods);
    /* +0x020 */ GameApplicationConfig *config; /**< the ctor's argument */
    /** Cleared by the ctor; RunDayTask sets it on DAYTASK_RESULT_CLOSED;
     * RunTitleMenu skips its first GraphRoom while set, then clears it. */
    /* +0x024 */ s32 skipGraphRoomPoll;
    /* +0x028 */ struct DreamSys *dreamSys; /**< the ctor's New_DreamSys() */
};

/** The type the ctor calls slot +0x040 (GameApplication__SeedRandom) through. */
typedef void (*GameApplicationSeedRandomFn)(GameApplication *self);

/** The type main() calls slot +0x044 (GameApplication__InitSystems) through. */
typedef void (*GameApplicationInitSystemsFn)(GameApplication *self, DrawSystem *drawSystem,
                                             struct Pad *pad);

/** GameApplication's own method table. */
extern GameApplicationMethods gGameApplicationMethods;

/** @brief GameApplication's method-table getter.
 * @return &gGameApplicationMethods */
extern GameApplicationMethods *GetGameApplicationMethods(void);

/** @brief Allocates a GameApplication from the pool and runs its ctor.
 * @param config which parts of the sequence run; kept by the object
 * @return the new object, or NULL when the pool allocation fails */
GameApplication *New_GameApplication(GameApplicationConfig *config);

/** @brief Constructor: Application's with config->dataSource, then this
 * table, the default data directory, the DreamSys and the RNG seed.
 * @param self the object
 * @param config which parts of the sequence run; kept at self->config */
void GameApplication__GameApplication(GameApplication *self, GameApplicationConfig *config);

/** @brief Slot +0x040: seeds rand() from the first scratchpad word, mod
 * DAYS_PER_YEAR.
 * @param self the object (unread) */
void GameApplication__SeedRandom(GameApplication *self);

/** @brief Slot +0x044: Application's initSystems, unless already initialized.
 * @param self the object
 * @param drawSystem the DrawSystem main() built
 * @param pad the Pad main() built */
void GameApplication__InitSystems(GameApplication *self, DrawSystem *drawSystem, struct Pad *pad);

/** @brief Slot +0x050, gated by config->showIntroLogos: shows the ASMK logo,
 * streams the ASMK movie, shows the OSD logo.
 * @param self the object */
void GameApplication__ShowIntroLogos(GameApplication *self);

/** @brief Shows a TIM image in a TaskCore with a frame bound of 0; its exit
 * callback, run as the image ends, registers the game's files.
 * @param self the object whose `aux` the task is started with
 * @param path the image's file name */
void GameApplication__ShowImage(GameApplication *self, const char *path);

/** @brief ShowImage's exit callback: registers sRecordTable's files with the
 * CD driver.
 * @return RegisterRecordTableFiles's result, which nothing reads */
s32 GameApplication__RegisterFilesCallback(void);

/** @brief Slot +0x054, gated by config->playStreams: streams one of the
 * opening movies, picked at random.
 * @param self the object */
void GameApplication__PlayOpeningMovie(GameApplication *self);

/** @brief Slot +0x058, gated by config->pollGraphRoom. Unless it is day 1 or
 * skipGraphRoomPoll is set, runs the GraphRoom, then PlaySpecialDayMovies if
 * it scored. Then runs the TitleMenu, again after a GraphRoom each time the
 * menu returns GRAPH, and clears skipGraphRoomPoll.
 * @param self the object
 * @return APPLICATION_LOOP_DAY for the menu's result 0 or with no menu, else
 *         APPLICATION_LOOP_OPENING */
s32 GameApplication__RunTitleMenu(GameApplication *self);

/** @brief Streams FILM\SPDAY01A.STR, bounded by the total frames of the first
 * ten special days' movies, with no skip on confirm. Gated by
 * config->playStreams.
 * @param self the object */
void GameApplication__PlaySpecialDayMovies(GameApplication *self);

/** @brief Slot +0x05C: empty. */
void GameApplication__OnRepeatMenu(void);

/** @brief Slot +0x060: runs one DayTask. Its CINEMATIC result plays the
 * cinematic (PlayCinematic); CLOSED sets skipGraphRoomPoll.
 * @param self the object
 * @return nonzero when the day is now day 1 of a year past the first */
s32 GameApplication__RunDayTask(GameApplication *self);

/** @brief Plays the special day record or event movie DreamSys's getCinematic
 * names: a movie is streamed (gated by config->playStreams, no skip on
 * confirm); a TIM image (movie id -1) is shown for 10 seconds.
 * @param self the object */
void GameApplication__PlayCinematic(GameApplication *self);

/** @brief Slot +0x064, gated by config->playStreams: streams ETC\ENDING.STR
 * with no skip on confirm.
 * @param self the object */
void GameApplication__PlayEndingMovie(GameApplication *self);

#endif
