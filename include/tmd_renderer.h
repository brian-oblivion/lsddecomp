/**
 * @file tmd_renderer.h
 * @brief The game's TMD renderer, its replacement for libgs's GsSortObject4.
 *
 * src/graphics/tmd_renderer.c projects, lights and, where a face is too large
 * on screen, subdivides each face of a GsDOBJ2's TMD object into GPU
 * primitives linked into an ordering table.
 */
#ifndef TMD_RENDERER_H
#define TMD_RENDERER_H

#include "common.h"
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>

/**
 * @brief Draws `obj`'s TMD object into an ordering table, as GsSortObject4
 *        does (same arguments).
 *
 * Every face that survives back-face culling becomes a GPU primitive in the
 * GsOUT_PACKET_P buffer, lit from its normal or depth-cued, and linked into
 * the OT slot its depth selects; faces too large on screen, or whose depth
 * saturates, go through libgs's RCpoly* subdivision instead. The object's
 * attribute word supplies the subdivision level and light mode, and an
 * object with GsDOFF set is skipped.
 * @param obj     The object to draw; its TMD must be mapped
 *                (GsMapModelingData) and its matrices set (GsSetLsMatrix).
 * @param ot      The ordering table.
 * @param otShift A face's average Z shifted right by this is its OT slot.
 * @param scratch The work area for the per-object draw context, normally
 *                the scratchpad (getScratchAddr(0)).
 */
extern void SortTmdObject(GsDOBJ2 *obj, GsOT *ot, s32 otShift, void *scratch);

#endif
