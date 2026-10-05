/**
 * @file style_layer.h
 * @brief The style layer: the free functions that give a day's scene its
 *        colours, fog, decoration, StyleEffects and positional sound cues.
 *
 * Defined in src/world/style_layer.c, with IsStyleVariantEven (declared in
 * include/dream_aux.h, whose triggers test it). ObjM (include/objm.h) is
 * its one client: it registers the scene once, ticks the layer every frame
 * and tears it down when the scene ends. The layer keeps its state in its
 * own globals, so there is one scene's style at a time.
 */
#ifndef STYLE_LAYER_H
#define STYLE_LAYER_H

#include "common.h"
#include "stage_map.h"

/**
 * @brief Registers the scene the layer styles and picks its style config.
 *
 * Does nothing while a scene is already registered (until StyleTeardown).
 * Otherwise stores the arguments, clears the two sound-cue slots and returns
 * the stage's config, filled into the layer's one StyleConfig.
 * @param grid      The scene's StageMap.
 * @param stage     The stage index, which selects the fixed config.
 * @param sceneRefs ObjM's sound, dreamer TMD, TIM image and viewport block
 *                  (ObjM::ctorSound onward).
 * @param day       The dream day.
 * @param unreadArg Stored, never read.
 * @return The StyleConfig (include/objm.h), or 0 when a scene is already
 *         registered.
 */
extern void *RegisterStyleConfig(struct StageMap *grid, s32 stage, void *sceneRefs, s32 day,
                                 s32 unreadArg);

/**
 * @brief Releases everything TickStyle built and unregisters the scene.
 */
extern void StyleTeardown(void);

/**
 * @brief The per-frame step: builds the scene's style objects on the first
 *        call, then updates them and services the two sound-cue slots.
 * @param cell    The grid's target cell, whose world position is the target;
 *                NULL for none.
 * @param unused  Passed on to the cue functions, which do not read it.
 * @param lastCue Handed by address to the cue search of each free slot
 *                (ObjM passes 0).
 * @return lastCue as the cue searches left it.
 */
extern s32 TickStyle(Descriptor10 *cell, void *unused, s32 lastCue);

#endif
