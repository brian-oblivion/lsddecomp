#ifndef GAMEAPPLICATION_H
#define GAMEAPPLICATION_H

#include "application.h"

/*
 * GameApplication -- the game itself, as an Application (include/application.h):
 * the one object main() (src/main.c) builds, New_GameApplication(
 * &sGameApplicationConfig) into sGameApplication, before running its
 * initSystems and then runMainLoop, which never returns. Application brings the
 * console up and owns the outer loop; this class fills the loop's six hooks with
 * the game's sequence and owns the game's DreamSys (include/dream_sys.h). Class id
 * 0x1F60, method table gGameApplicationMethods; the methods and the getter
 * GetGameApplicationMethods are in src/app/GameApplicationFileResource.c. No class
 * derives from it.
 *
 * Lifecycle.
 *   ctor(config)   Application's ctor with config->dataSource, this table, the
 *                  config kept, the DreamSys built from "ETC\DREAME5.TMD"
 *                  (New_LinkResource), config->dreamSysConfigOption handed to it, and the RNG
 *                  seeded (setScreenDims's occupant, below).
 *   initSystems    Application's, unless already initialized.
 *   runMainLoop    Application's: once showIntroLogos, then forever
 *                  playOpeningMovie and the runTitleMenu loop.
 * The hooks, in the order runMainLoop calls them:
 *   +0x050 showIntroLogos  ShowIntroLogos: the ASMK logo, the ASMK
 *                                 movie, the OSD logo
 *   +0x054 playOpeningMovie  PlayOpeningMovie: an opening movie
 *   +0x058 runTitleMenu    RunTitleMenu: GraphRoom and TitleMenu against
 *                                 the DreamSys; returns 0 or 2 to the loop
 *                                 (enum ApplicationLoopStatus)
 *   +0x05C onRepeatMenu           OnRepeatMenu: empty (status 1 is never
 *                                 returned)
 *   +0x060 runDayTask          RunDayTask: one DayTask (include/DayTask.h),
 *                                 then maybe the current cinematic;
 *                                 nonzero (a year gone by) runs +0x064
 *   +0x064 playEndingMovie  PlayEndingMovie: ETC\ENDING.STR
 * Every task these start is given the parent's `aux` as its
 * IntermediateBaseInitArgs, and every movie but the intro's is gated by
 * config->playStreams.
 *
 * Slots are Application's, flat (APPLICATION_SLOTS). Two overrides take a
 * different parameter list from the slot they fill, so the slot keeps the
 * inherited type and the caller casts:
 *   +0x040 setScreenDims <- GameApplication__SeedRandom (self only);
 *          the ctor calls it through GameApplicationSeedRandomFn. Application's
 *          own ctor ran under Application's table, so the default screen is set.
 *   +0x044 initSystems   <- GameApplication__InitSystems (no 4th argument);
 *          main calls it through GameApplicationInitSystemsFn.
 *
 * Object size 0x2C (New_GameApplication); its own fields start at +0x020.
 */

typedef struct GameApplication GameApplication;
typedef struct GameApplicationMethods GameApplicationMethods;

/* The ctor's argument: which parts of the sequence run. One instance,
 * sGameApplicationConfig = {0x13, 0, 1, 1, 1, 1}, kept at self->config; each
 * gate below is read `!= 0` by the methods it names. */
typedef struct GameApplicationConfig {
    /* +0x00 */ s32 dataSource; /* Application's ctor argument (0x13 = the CD driver's class id) */
    /* +0x04 */ s32 dayTaskSyncDriver; /* New_DayTask's syncDriver, in RunDayTask: that ctor
                            * passes (syncDriver == 0) to SetActiveDataSourceDriverMode */
    /* +0x08 */ s32 playStreams; /* gates every movie after the intro: PlayOpeningMovie, PlaySpecialDayMovies,
                                     * PlayCinematic's movie branch, PlayEndingMovie */
    /* +0x0C */ s32 showIntroLogos; /* gates ShowIntroLogos */
    /* +0x10 */ s32 pollGraphRoom;  /* gates RunTitleMenu (0: it returns 2 at once) */
    /* +0x14 */ s32 dreamSysConfigOption; /* the ctor passes it to the DreamSys's getSetConfigOption (DreamSys__GetSetConfigOption),
                            * which stores a value >= 0 in DreamSys::configOption. DreamSys__ResetSessionState
                            * zeroes that field and no code but the get/set reads it, so what it selects is
                            * not established (sGameApplicationConfig passes 1). */
} GameApplicationConfig;

struct GameApplicationMethods {
    APPLICATION_SLOTS(GameApplication, (GameApplication * self, GameApplicationConfig *args));
};

struct GameApplication {
    APPLICATION_FIELDS(GameApplicationMethods);
    /* +0x020 */ GameApplicationConfig *config; /* the ctor's argument */
    /* +0x024 */ s32 skipGraphRoomPoll; /* ctor clears; RunDayTask sets it on DAYTASK_RESULT_CLOSED;
                                         * RunTitleMenu skips its first GraphRoom while set, then clears it */
    /* +0x028 */ struct DreamSys *dreamSys; /* the ctor's New_DreamSys() */
};

/* The two overrides whose parameter lists differ from their slots'. */
typedef void (*GameApplicationSeedRandomFn)(GameApplication *self);
typedef void (*GameApplicationInitSystemsFn)(GameApplication *self, DrawSystem *drawSystem,
                                             struct Pad *pad);

extern GameApplicationMethods gGameApplicationMethods;
extern GameApplicationMethods *GetGameApplicationMethods(void); /* returns &gGameApplicationMethods */

GameApplication *New_GameApplication(GameApplicationConfig *args);
void GameApplication__GameApplication(GameApplication *self, GameApplicationConfig *args);
void GameApplication__SeedRandom(GameApplication *self);
void GameApplication__InitSystems(GameApplication *self, DrawSystem *drawSystem, struct Pad *pad);
void GameApplication__ShowIntroLogos(GameApplication *self);
void GameApplication__ShowImage(GameApplication *self, const char *path);
s32 GameApplication__RegisterFilesCallback(void);
void GameApplication__PlayOpeningMovie(GameApplication *self);
s32 GameApplication__RunTitleMenu(GameApplication *self);
void GameApplication__PlaySpecialDayMovies(GameApplication *self);
void GameApplication__OnRepeatMenu(void);
s32 GameApplication__RunDayTask(GameApplication *self);
void GameApplication__PlayCinematic(GameApplication *self);
void GameApplication__PlayEndingMovie(GameApplication *self);

#endif
