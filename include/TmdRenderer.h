#ifndef TMDRENDERER_H
#define TMDRENDERER_H

/*
 * The TMD renderer, src/graphics/TmdRenderer.c: the per-face projection,
 * lighting and subdivision a GsDOBJ2's TMD goes through into an ordering
 * table. The BasicClass methods and the pool allocator's busy flag at the
 * head of that file are declared by include/basic_class.h and
 * include/BMemPMgr.h.
 */

#include "common.h"
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>

/* Sorts obj's TMD into `ot` at otShift, using `scratch` as its work area. */
extern void SortTmdObject(GsDOBJ2 *obj, GsOT *ot, s32 otShift, void *scratch);

#endif
