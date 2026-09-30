/*
 * libgs_gs_106 -- Sony's libgs `gs_106` module, carried as C: one function,
 * GsSetProjection, which sets the projection distance through the GTE's
 * SetGeomScreen. gs_106 never placed as an object in this image, so the
 * function is matched as C; it counts as library, not game code. It sits
 * after the game's Viewport code and before the libgs objects that follow.
 */
#include "common.h"
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>

void GsSetProjection(long h) {
    SetGeomScreen(h);
}
