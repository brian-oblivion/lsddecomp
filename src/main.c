/*
 * main.c -- the game's entry point, `main`, and Sony's empty `__main`.
 *
 * Sony's crt0 calls `main` once. It sets the memory size, makes the game's
 * one BMemPMgr pool and installs it as the default every BMemPMgrAlloc
 * uses, builds the root GameApplication from sGameApplicationConfig,
 * builds the DrawSystem and a Pad on the first controller port, and hands
 * both to the application's initSystems. Then it calls runMainLoop, which
 * never returns: the game loop is Application's, not this file's.
 *
 * `main` does not call `__main` in the C: a call to `__main` is generated at
 * the top of any function named `main`. `__main` is Sony's empty stub (the
 * SDK's `_obj/none`); no linked SDK object provides it, so it is written
 * here.
 */
#include "common.h"
#include "game_application.h"
#include "pad.h"
#include "bmem_pmgr.h"
#include "data_source.h"
#include <kernel.h>
#ifdef PLATFORM_PC
#include <libapi.h> /* SetMem: psyz declares it here, not in kernel.h */
#endif

/* The game's one BMemPMgr pool and the root GameApplication, both made
 * once, below. */
static BMemPMgr *sStartupBMemPMgr SDATA = NULL;
static GameApplication *sGameApplication SBSS = NULL;

/* The shipped game's configuration (the fields are documented in
 * game_application.h): data from the CD-ROM, and the movies, intro logos
 * and title menu all on. */
GameApplicationConfig sGameApplicationConfig = {
    DATASOURCE_CD, /* dataSource */
    0,             /* dayTaskSyncDriver */
    1,             /* playStreams */
    1,             /* showIntroLogos */
    1,             /* pollGraphRoom */
    1,             /* dreamSysConfigOption */
};

/* SetMem's argument: the RAM size in megabytes (Psy-Q libapi takes 2, a
 * production console, or 8, a development board). */
#define CONSOLE_RAM_MB 2

/* Bytes of blocks in the game's one BMemPMgr pool, BMemPMgrInit's poolSize
 * (0x166C00): every BMemPMgrAlloc in the game is carved from it. */
#define DEFAULT_POOL_SIZE (1435 * 1024)

void main(void) {
    DrawSystem *drawSystem;
    Pad *pad;

    SetMem(CONSOLE_RAM_MB);
    /* MATCHING: the unread 0 is retail's `move $a1, $zero`. */
    sStartupBMemPMgr = BMemPMgrInit(DEFAULT_POOL_SIZE, 0);
    SetDefaultBMemPMgr(sStartupBMemPMgr);
    sGameApplication = New_GameApplication(&sGameApplicationConfig);
    drawSystem = New_DrawSystem();
    pad = New_Pad(0, 0); /* PadInit mode 0, port 0 */
    /* MATCHING: the override, GameApplication__InitSystems, takes three
     * arguments, not the slot's four; calling the slot's type would load $a3. */
    ((GameApplicationInitSystemsFn)sGameApplication->methods->initSystems)(sGameApplication,
                                                                           drawSystem, pad);
    sGameApplication->methods->runMainLoop(sGameApplication);
}

/* Sony's _obj/none: empty. Its one call is the one generated at the top of
 * main. */
void __main(void) {}
