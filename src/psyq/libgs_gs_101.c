/*
 * libgs_gs_101 -- Sony's libgs `gs_101` module, carried as C: one function,
 * GsSetNearClip, which stores the near clipping distance into libgs's
 * GsCLIP3near. gs_101 never placed as an object in this image, so the
 * function is matched as C; it counts as library, not game code.
 */
#include "common.h"
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>

/* libgs's near clipping distance, a bss global <libgs.h> does not declare. */
extern long GsCLIP3near;

void GsSetNearClip(long clip_near) {
    GsCLIP3near = clip_near;
}
