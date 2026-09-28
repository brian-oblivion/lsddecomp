/*
 * libgs_gs_101 -- Sony's libgs `gs_101` module, carried as C: one function,
 * GsSetNearClip, which stores the near clipping distance into libgs's
 * GsCLIP3near. The prototype is LIBGS.H's own. gs_101 never placed as an
 * object in this image, so the function is matched as C; it counts as
 * library, not game code.
 *
 * Deliberately includes only common.h: its one caller, Viewport__Update,
 * declares it locally from LIBGS.H, and no shared header carries a Sony
 * prototype.
 */
#include "common.h"

/* Sony's bss global, pinned in config/psyq-objects.ld. */
extern long GsCLIP3near;

/* LIBGS.H: void GsSetNearClip(long clip_near); */
void GsSetNearClip(long clip_near) {
    GsCLIP3near = clip_near;
}
