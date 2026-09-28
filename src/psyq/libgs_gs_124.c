/*
 * libgs_gs_124 -- Sony's libgs module gs_124: GsSetWorkBase, which sets
 * GsOUT_PACKET_P, the cursor libgs writes GPU packets through.
 *
 * Sony code carried as C: gs_124 never placed as a linked object, so its one
 * function is written here instead, against <libgs.h>'s prototype and
 * global.
 */
#include "common.h"
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>

void GsSetWorkBase(PACKET *outpacketp) {
    GsOUT_PACKET_P = outpacketp;
}
