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
 * `main` does not call `__main` in the C. cc1 inserts `jal __main` at the
 * top of any function named `main`, and that call is retail's first
 * instruction. `__main` is Sony's empty stub (the SDK's `_obj/none`); no
 * linked object places it, so it is written here.
 */
#include "common.h"
#include "GameApplication.h"
#include "Pad.h"
#include "BMemPMgr.h"
#include <kernel.h>

extern BMemPMgr *sStartupBMemPMgr;
extern GameApplication *sGameApplication;
extern GameApplicationConfig sGameApplicationConfig;

/* SetMem's argument: the RAM size in megabytes (Psy-Q libapi takes 2, a
 * retail console, or 8, a development board). */
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

/* Sony's _obj/none: empty. Its one call is the one cc1 puts in main. */
void __main(void) {}
