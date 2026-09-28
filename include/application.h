#ifndef APPLICATION_H
#define APPLICATION_H

#include "basic_class.h"
#include "draw_system.h"
#include "IntermediateBase.h"

/*
 * Application -- the program's application shell: it brings up the console's
 * subsystems and runs the game's outer loop, and leaves what the loop does to
 * its subclass. Class id 0x60, method table gApplicationMethods, a direct
 * BasicClass subclass. Methods in src/app/application.c.
 *
 * Lifecycle. It is abstract and never built on its own: its one subclass,
 * GameApplication (include/game_application.h), is the object main() (src/main.c)
 * builds, and that subclass's ctor runs this one first.
 *   ctor(dataSource)  runs CdInit once per boot, selects the data source
 *                     (SetActiveDataSource) and sets the default screen,
 *                     {320, 240} in vram mode 0, through setScreenDims.
 *   initSystems       main() passes the DrawSystem and Pad it built. Once:
 *                     registers the DrawSystem (SetDrawSystem), opens the
 *                     display at the stored size and mode (its initGraph),
 *                     then SsInit and GsInit3D, and allocates `aux`.
 *   runMainLoop       once initialized, never returns: the six hooks at
 *                     +0x050..+0x064 below, in the order their comments give.
 *
 * `aux` is the IntermediateBaseInitArgs (include/IntermediateBase.h) every
 * task the subclass starts is given: {drawSystem, pad, NULL, NULL, NULL}; the
 * NULLs make each task's IntermediateBase__Init create its own FrameClock,
 * LightRig and Viewport.
 *
 * The table is 0x68 bytes: the six hooks are NULL words in this class's own
 * table, filled by the subclass, and named for its occupants. Object size
 * 0x20 (no allocator; the subclass's own fields start at +0x020).
 */

typedef struct Application Application;
typedef struct ApplicationMethods ApplicationMethods;
struct Pad; /* initSystems's pad: main()'s New_Pad(0, 0) */

/* ScreenDims and DrawSystem are include/draw_system.h's. */

/* clang-format off */
#define APPLICATION_SLOTS(Self, CtorParams)                                                         \
    BASICCLASS_SLOTS(Self, CtorParams);                                                            \
    /* +0x040 */ void (*setScreenDims)(Self *self, ScreenDims *dims, s32 vramMode); /* Application__SetScreenDims */ \
    /* +0x044: the occupant never reads arg3; GameApplication__InitSystems passes 0 */             \
    /* +0x044 */ void (*initSystems)(Self *self, DrawSystem *drawSystem, struct Pad *pad, s32 arg3); /* Application__InitSystems */ \
    /* +0x048 */ void (*slot48)(Self *self);                    /* Application__NoOpSlot48, empty; no caller */ \
    /* +0x04C */ void (*runMainLoop)(Self *self);               /* Application__RunMainLoop */      \
    /* +0x050..+0x064: NULL here, called by runMainLoop; named for GameApplication's occupants */      \
    /* +0x050 */ void (*showIntroLogos)(Self *self);     /* once, before the loop */        \
    /* +0x054 */ void (*playOpeningMovie)(Self *self);     /* each outer iteration */         \
    /* +0x058 */ s32 (*runTitleMenu)(Self *self);        /* 0 ends the inner loop, 1 and 2 dispatch */ \
    /* +0x05C */ void (*onRepeatMenu)(Self *self);              /* on status 1, before runTitleMenu runs again; GameApplication__OnRepeatMenu, empty */ \
    /* +0x060 */ s32 (*runDayTask)(Self *self);              /* on status 2; nonzero runs +0x064 */ \
    /* +0x064 */ void (*playEndingMovie)(Self *self)    /* GameApplication__PlayEndingMovie */
/* clang-format on */

/* clang-format off */
#define APPLICATION_FIELDS(Methods)                                                                 \
    BASICCLASS_FIELDS(Methods);                                                                    \
    /* +0x00C */ ScreenDims dims;      /* setScreenDims; initGraph's size */                       \
    /* +0x014 */ s32 vramMode;         /* setScreenDims; initGraph's GsInitGraph vram mode */      \
    /* +0x018 */ s32 initialized;      /* cleared by the ctor, set by initSystems; runMainLoop runs only once set */ \
    /* +0x01C */ IntermediateBaseInitArgs *aux /* initSystems's allocation: every task's init argument */
/* clang-format on */

/* What runTitleMenu (+0x058) returns to Application__RunMainLoop, and the
 * hook each value runs; named, like the hooks, for GameApplication's
 * occupants. GameApplication__RunTitleMenu returns OPENING when TitleMenu's
 * result is nonzero (TaskCore's timeout is 1) and DAY when it is 0 or there
 * is no menu (config->pollGraphRoom 0). */
enum ApplicationLoopStatus {
    APPLICATION_LOOP_OPENING = 0, /* ends the inner loop: +0x054 (playOpeningMovie) again */
    /* +0x05C (onRepeatMenu), then runTitleMenu again; GameApplication never returns it */
    APPLICATION_LOOP_REPEAT_MENU = 1,
    APPLICATION_LOOP_DAY = 2 /* +0x060 (runDayTask), +0x064 if it returns nonzero, then runTitleMenu again */
};

struct ApplicationMethods {
    APPLICATION_SLOTS(Application, (Application * self, s32 dataSource));
};

struct Application {
    APPLICATION_FIELDS(ApplicationMethods);
};

extern ApplicationMethods gApplicationMethods;
extern ApplicationMethods *GetApplicationMethods(void); /* returns &gApplicationMethods */

void Application__Application(Application *self, s32 dataSource);
void Application__Finalize(Application *self);
void Application__SetScreenDims(Application *self, ScreenDims *dims, s32 vramMode);
void Application__InitSystems(Application *self, DrawSystem *drawSystem, struct Pad *pad);
void Application__NoOpSlot48(Application *self);
void Application__RunMainLoop(Application *self);

#endif
