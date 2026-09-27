#ifndef GAMEAPPLICATION_H
#define GAMEAPPLICATION_H

#include "Application.h"

/*
 * GameApplication -- the game itself, as an Application (include/Application.h):
 * the one object main() (src/main.c) builds, New_GameApplication(
 * &gGameApplicationConfig) into gGameApplication, before running its
 * initSystems and then runMainLoop, which never returns. Application brings the
 * console up and owns the outer loop; this class fills the loop's six hooks with
 * the game's sequence and owns the game's DreamSys (include/DreamSys.h). Class id
 * 0x1F60, method table gGameApplicationMethods, getter GetGameApplicationMethods
 * (src/code_171e0.c); methods in src/code_1677c.c. No class derives from it.
 *
 * Lifecycle.
 *   ctor(config)   Application's ctor with config->dataSource, this table, the
 *                  config kept, the DreamSys built from "ETC\DREAME5.TMD"
 *                  (New_LinkResource), config->unk14 handed to it, and the RNG
 *                  seeded (setScreenDims's occupant, below).
 *   initSystems    Application's, unless already initialized.
 *   runMainLoop    Application's: once loadIntroLogoSequence, then forever
 *                  startWeeklyStreamTask and the pollGraphRoomStatus loop.
 * The hooks, in the order runMainLoop calls them:
 *   +0x050 loadIntroLogoSequence  the ASMK logo, the ASMK stream, the OSD logo
 *   +0x054 startWeeklyStreamTask  the stream PickOpeningMovie picks
 *   +0x058 pollGraphRoomStatus    runs GraphRoom and TitleMenu against the
 *                                 DreamSys; returns 0, 1 or 2 to the loop
 *   +0x05C slot5C                 empty (GameApplication__NoOpSlot5C)
 *   +0x060 pollStatusObj          runs one DayTask (include/DayTask.h)
 *                                 and may start the current cinematic's stream;
 *                                 nonzero runs +0x064
 *   +0x064 startStreamTaskWithInit  the stream GetEndingMovie names
 * Every task these start is given the parent's `aux` as its
 * IntermediateBaseInitArgs, and every stream is gated by config->playStreams.
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
 * gGameApplicationConfig = {0x13, 0, 1, 1, 1, 1}, kept at self->config; each
 * gate below is read `!= 0` by the methods it names. */
typedef struct GameApplicationConfig {
    /* +0x00 */ s32 dataSource; /* Application's ctor argument (0x13 = the CD driver's class id) */
    /* +0x04 */ s32 unk04;      /* New_DayTask's 3rd argument, in PollStatusObj: that ctor
                            * passes (unk04 == 0) to SetActiveDataSourceDriverMode */
    /* +0x08 */ s32 playStreams; /* gates every StreamTask: StartWeeklyStreamTask, StartGraphRoomStreamTask,
                                     * StartCinematicStream's stream branch, StartStreamTaskWithInit */
    /* +0x0C */ s32 showIntroLogos; /* gates LoadIntroLogoSequence */
    /* +0x10 */ s32 pollGraphRoom;  /* gates PollGraphRoomStatus (0: it returns 2 at once) */
    /* +0x14 */ s32 unk14; /* the ctor passes it to the DreamSys's slot228 (DreamSys__func_5ba20),
                            * which stores a value >= 0 at DreamSys +0x924 */
} GameApplicationConfig;

struct GameApplicationMethods {
    APPLICATION_SLOTS(GameApplication, (GameApplication * self, GameApplicationConfig *args));
};

struct GameApplication {
    APPLICATION_FIELDS(GameApplicationMethods);
    /* +0x020 */ GameApplicationConfig *config; /* the ctor's argument */
    /* +0x024 */ s32 skipGraphRoomPoll; /* ctor clears; PollStatusObj sets it on status 3; PollGraphRoomStatus
                                                * skips its New_GraphRoom poll while set, then clears it */
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
s32 GameApplication__PollGraphRoomStatus(GameApplication *self);
void GameApplication__PlaySpecialDayMovies(GameApplication *self);
void GameApplication__NoOpSlot5C(void);
s32 GameApplication__PollStatusObj(GameApplication *self);
void GameApplication__PlayCinematic(GameApplication *self);
void GameApplication__PlayEndingMovie(GameApplication *self);

#endif
