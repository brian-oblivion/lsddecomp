#ifndef CLASS_6D3C8_H
#define CLASS_6D3C8_H

#include "Application.h"

/*
 * GameApplication -- class id 0x1F60, method table gGameApplicationMethods (25 slots), a
 * subclass of Application (include/Application.h): its ctor calls
 * GetApplicationMethods()->ctor first, and `classtable.py gGameApplicationMethods --vs
 * gApplicationMethods` shares every slot but +0x008/+0x040/+0x044 and the six
 * +0x050..+0x064 slots Application leaves NULL. No class derives from it.
 * Methods in src/code_1677c.c; the table getter is in src/code_171e0.c.
 *
 * It is the game's root object: main() (src/main.c) builds exactly one,
 * New_GameApplication(&gGameApplicationCtorArgs) into gGameApplication, then runs its
 * initSystems and runMainLoop. What its own methods do: the ctor loads
 * "ETC\DREAME5.TMD" and builds the owned DreamSys from it; the six slots
 * Application's runMainLoop calls load the ASMK/OSD intro logos and their
 * stream (+0x050), start the day-of-week stream (+0x054), poll the
 * GraphRoom tasks against the DreamSys status (+0x058), and run a status
 * object whose code can start the current cinematic (+0x060, +0x064). The
 * name stays the table-address identity: nothing yet names what the class
 * IS beyond "the object main runs".
 *
 * Slots are Application's, flat (APPLICATION_SLOTS); that header already names
 * +0x050..+0x064 for this class's occupants. Two overrides take a different
 * parameter list from the slot they fill, so the slot keeps the inherited
 * type and the caller casts (FINISHING-PLAN track 4 step 6):
 *   +0x040 setScreenDims <- GameApplication__SeedRandom (self only);
 *          the ctor calls it through GameApplicationSetDayFn.
 *   +0x044 initSystems   <- GameApplication__InitSystems (no 4th argument);
 *          main calls it through GameApplicationInitSystemsFn.
 *
 * Object size 0x2C (New_GameApplication's BMemPMgrAlloc(0x2C)). +0x000..+0x01F
 * are Application's: this class's methods pass the parent's `aux` (+0x01C)
 * as the IntermediateBaseInitArgs of every task they start (it is the
 * 0x14-byte block initSystems allocates: {drawSystem, pad, 0, 0, 0}), and
 * InitSystems tests the parent's `initialized` (+0x018).
 */

typedef struct GameApplication GameApplication;
typedef struct GameApplicationMethods GameApplicationMethods;

/* The ctor's argument block. One instance, gGameApplicationCtorArgs =
 * {0x13, 0, 1, 1, 1, 1} (asm/data/57028.data.s), kept by the ctor at
 * self->ctorArgs; each gate below is read `!= 0` by the methods it names. */
typedef struct GameApplicationCtorArgs {
    /* +0x00 */ s32 dataSource; /* Application's ctor argument (0x13 = the CD driver's class id) */
    /* +0x04 */ s32 unk04;      /* New_Class865C8's 3rd argument, in PollStatusObj */
    /* +0x08 */ s32 playStreams; /* gates every StreamTask: StartWeeklyStreamTask, StartGraphRoomStreamTask,
                                     * StartCinematicStream's stream branch, StartStreamTaskWithInit */
    /* +0x0C */ s32 showIntroLogos; /* gates LoadIntroLogoSequence */
    /* +0x10 */ s32 pollGraphRoom;  /* gates PollGraphRoomStatus (0: it returns 2 at once) */
    /* +0x14 */ s32 unk14; /* the ctor passes it to the DreamSys's slot228 (DreamSys__func_5ba20) */
} GameApplicationCtorArgs;

struct GameApplicationMethods {
    APPLICATION_SLOTS(GameApplication, (GameApplication * self, GameApplicationCtorArgs *args));
};

struct GameApplication {
    APPLICATION_FIELDS(GameApplicationMethods);
    /* +0x020 */ GameApplicationCtorArgs *ctorArgs; /* the ctor's argument */
    /* +0x024 */ s32 skipGraphRoomPoll; /* ctor clears; PollStatusObj sets it on status 3; PollGraphRoomStatus
                                                * skips its New_GraphRoom poll while set, then clears it */
    /* +0x028 */ struct DreamSys *dreamSys; /* the ctor's New_DreamSys() */
};

/* The two overrides whose parameter lists differ from their slots'. */
typedef void (*GameApplicationSetDayFn)(GameApplication *self);
typedef void (*GameApplicationInitSystemsFn)(GameApplication *self, DrawSystem *drawSystem, struct Pad *pad);

extern GameApplicationMethods gGameApplicationMethods;
extern GameApplicationMethods *GetGameApplicationMethods(void); /* returns &gGameApplicationMethods */

GameApplication *New_GameApplication(GameApplicationCtorArgs *args);
void GameApplication__GameApplication(GameApplication *self, GameApplicationCtorArgs *args);
void GameApplication__SeedRandom(GameApplication *self);
void GameApplication__InitSystems(GameApplication *self, DrawSystem *drawSystem, struct Pad *pad);
void GameApplication__LoadIntroLogoSequence(GameApplication *self);
void GameApplication__StartLoaderTask(GameApplication *self, const char *path);
s32 GameApplication__LoaderTaskDoneCallback(void);
void GameApplication__StartWeeklyStreamTask(GameApplication *self);
s32 GameApplication__PollGraphRoomStatus(GameApplication *self);
void GameApplication__StartGraphRoomStreamTask(GameApplication *self);
void GameApplication__NoOpSlot5C(void);
s32 GameApplication__PollStatusObj(GameApplication *self);
void GameApplication__StartCinematicStream(GameApplication *self);
void GameApplication__StartStreamTaskWithInit(GameApplication *self);

#endif
