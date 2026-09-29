#ifndef APPLICATION_H
#define APPLICATION_H

#include "basic_class.h"
#include "draw_system.h"
#include "intermediate_base.h"

/**
 * @file application.h
 * @brief Application, the program's application shell: console bring-up and
 * the game's outer loop, with the hooks its subclass fills.
 *
 * Declares the class (APPLICATION_SLOTS, APPLICATION_FIELDS for its one
 * subclass, GameApplication in game_application.h), the status its title
 * hook returns to the loop, and its methods, defined in
 * src/app/application.c. ScreenDims and DrawSystem are draw_system.h's.
 */

typedef struct Application Application;
typedef struct ApplicationMethods ApplicationMethods;

/** Application's class id (gApplicationMethods word +0x000). */
#define APPLICATION_CLASS_ID 0x60

struct Pad; /* pad.h: initSystems's pad, main()'s New_Pad(0, 0) */

/** Application's slots, BasicClass's first. +0x050..+0x064 are NULL in
 * Application's own table: runMainLoop calls them and the subclass fills
 * them, and they are named for GameApplication's occupants. */
/* clang-format off */
#define APPLICATION_SLOTS(Self, CtorParams)                                                         \
    BASICCLASS_SLOTS(Self, CtorParams);                                                            \
    /* +0x040 */ void (*setScreenDims)(Self *self, ScreenDims *dims, s32 vramMode); /**< @see Application__SetScreenDims */ \
    /* +0x044 */ void (*initSystems)(Self *self, DrawSystem *drawSystem, struct Pad *pad, s32 arg3); /**< @see Application__InitSystems; the occupant never reads arg3, GameApplication__InitSystems passes 0 */ \
    /* +0x048 */ void (*slot48)(Self *self);                    /**< @see Application__NoOpSlot48, empty; no caller */ \
    /* +0x04C */ void (*runMainLoop)(Self *self);               /**< @see Application__RunMainLoop */      \
    /* +0x050 */ void (*showIntroLogos)(Self *self);     /**< @see GameApplication__ShowIntroLogos; once, before the loop */ \
    /* +0x054 */ void (*playOpeningMovie)(Self *self);   /**< @see GameApplication__PlayOpeningMovie; each outer iteration */ \
    /* +0x058 */ s32 (*runTitleMenu)(Self *self);        /**< @see GameApplication__RunTitleMenu; returns an ApplicationLoopStatus */ \
    /* +0x05C */ void (*onRepeatMenu)(Self *self);       /**< @see GameApplication__OnRepeatMenu; on status 1, before runTitleMenu runs again */ \
    /* +0x060 */ s32 (*runDayTask)(Self *self);          /**< @see GameApplication__RunDayTask; on status 2; nonzero runs +0x064 */ \
    /* +0x064 */ void (*playEndingMovie)(Self *self)     /**< @see GameApplication__PlayEndingMovie */
/* clang-format on */

/** Application's fields, BasicClass's first. */
/* clang-format off */
#define APPLICATION_FIELDS(Methods)                                                                 \
    BASICCLASS_FIELDS(Methods);                                                                    \
    /* +0x00C */ ScreenDims dims;      /**< setScreenDims; initGraph's size */                       \
    /* +0x014 */ s32 vramMode;         /**< setScreenDims; initGraph's GsInitGraph vram mode */      \
    /* +0x018 */ s32 initialized;      /**< cleared by the ctor, set by initSystems; runMainLoop runs only once set */ \
    /* +0x01C */ IntermediateBaseInitArgs *aux /**< initSystems's allocation: every task's init argument */
/* clang-format on */

/** What runTitleMenu (+0x058) returns to Application__RunMainLoop, and the
 * hook each value runs; named, like the hooks, for GameApplication's
 * occupants. GameApplication__RunTitleMenu returns OPENING when TitleMenu's
 * result is nonzero (TaskCore's timeout is 1) and DAY when it is 0 or there
 * is no menu (config->pollGraphRoom 0). */
enum ApplicationLoopStatus {
    APPLICATION_LOOP_OPENING = 0, /**< ends the inner loop: +0x054 (playOpeningMovie) again */
    /** +0x05C (onRepeatMenu), then runTitleMenu again; GameApplication never returns it */
    APPLICATION_LOOP_REPEAT_MENU = 1,
    APPLICATION_LOOP_DAY = 2 /**< +0x060 (runDayTask), +0x064 if it returns nonzero, then runTitleMenu again */
};

/** Application's method table: BasicClass's slots and APPLICATION_SLOTS'
 * own; the ctor takes the data source to select. */
struct ApplicationMethods {
    APPLICATION_SLOTS(Application, (Application * self, s32 dataSource));
};

/**
 * Application -- the program's application shell: it brings up the console's
 * subsystems and runs the game's outer loop, and leaves what the loop does to
 * its subclass. Class id 0x60, method table gApplicationMethods, a direct
 * BasicClass subclass. Methods in src/app/application.c.
 *
 * Lifecycle. It is abstract and never built on its own: its one subclass,
 * GameApplication (game_application.h), is the object main() builds, and that
 * subclass's ctor runs this one first.
 *  - ctor(dataSource): runs CdInit once per boot, selects the data source
 *    (SetActiveDataSource) and sets the default screen, {320, 240} in vram
 *    mode 0, through setScreenDims.
 *  - initSystems: main() passes the DrawSystem and Pad it built. Once:
 *    registers the DrawSystem (SetDrawSystem), opens the display at the
 *    stored size and mode (its initGraph), then SsInit and GsInit3D, and
 *    allocates `aux`.
 *  - runMainLoop: once initialized, never returns: showIntroLogos once, then
 *    forever playOpeningMovie and a runTitleMenu loop that dispatches on the
 *    ApplicationLoopStatus it returns.
 *
 * `aux` is the IntermediateBaseInitArgs (intermediate_base.h) every task the
 * subclass starts is given: {drawSystem, pad, NULL, NULL, NULL}; the NULLs
 * make each task's IntermediateBase__Init create its own FrameClock,
 * LightRig and Viewport.
 *
 * The table is 0x68 bytes. The object is 0x20 bytes; there is no allocator,
 * and the subclass's own fields start at +0x020.
 */
struct Application {
    APPLICATION_FIELDS(ApplicationMethods);
};

/** Application's own method table. */
extern ApplicationMethods gApplicationMethods;

/** @brief Application's method-table getter.
 * @return &gApplicationMethods */
extern ApplicationMethods *GetApplicationMethods(void);

/** @brief Constructor: BasicClass's, then CdInit (first time only), clears
 * `initialized`, selects the data source and sets the default screen size.
 * @param self the object
 * @param dataSource the data source's class id (DATASOURCE_CD, 0x13, for the
 *        game) */
void Application__Application(Application *self, s32 dataSource);

/** @brief Finalize override: empty.
 * @param self the object */
void Application__Finalize(Application *self);

/** @brief Stores the screen size and vram mode initSystems opens the display
 * with.
 * @param self the object
 * @param dims width and height
 * @param vramMode GsInitGraph's vram mode */
void Application__SetScreenDims(Application *self, ScreenDims *dims, s32 vramMode);

/** @brief Brings up display, sound and 3D once: registers the DrawSystem,
 * opens the display, runs SsInit and GsInit3D, and allocates `aux`, the
 * argument every task is started with.
 * @param self the object
 * @param drawSystem the DrawSystem main() built
 * @param pad the Pad main() built */
void Application__InitSystems(Application *self, DrawSystem *drawSystem, struct Pad *pad);

/** @brief Slot +0x048: empty, and nothing calls it.
 * @param self the object */
void Application__NoOpSlot48(Application *self);

/** @brief The game's outer loop; never returns once initialized (returns at
 * once otherwise). Runs showIntroLogos, then forever playOpeningMovie and the
 * title loop: runTitleMenu's status 1 runs onRepeatMenu, 2 runs runDayTask
 * (and playEndingMovie when that returns nonzero), 0 goes back to the
 * opening movie.
 * @param self the object */
void Application__RunMainLoop(Application *self);

#endif
