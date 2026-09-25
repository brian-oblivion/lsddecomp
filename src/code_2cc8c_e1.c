/*
 * code_2cc8c_e1 -- ONE function, Sony's GsSetWorkBase (libgs/gs_124), 4 words,
 * 0x303E4..0x303F4 (vram 0x8003FBE4..0x8003FBF4).
 *
 * SONY CODE, written as C. Round 78's head identified it under FINISHING-PLAN
 * track 2: the body is exactly `GsOUT_PACKET_P = outpacketp;`, `sdkname.py`
 * scores it EXACT (TINY, 4 words) against libgs/gs_124 GsSetWorkBase on discs
 * 3.3/3.5/3.6, its one store target 0x8008E794 is Sony's `GsOUT_PACKET_P`
 * (pinned in config/psyq-objects.ld; LIBGS.H: "Work Base pointer"), and it
 * sits between placed libgs/gs_123 and gs_111 -- the gs_124 slot of the
 * archive's link order. The prototype is LIBGS.H's own. `progress.py` counts
 * it as library through the `identified` comment on its symbols entry.
 *
 * ROUND 34: this unit exists because the functions on BOTH SIDES of it are
 * Sony's linked objects (gs_123 Gssub_make_matrix in front, gs_111 GsDrawOt
 * behind). gs_124 itself never placed as an object, so the function stays as
 * C in its own unit rather than putting an `o` segment inside a `c` one.
 *
 * Deliberately includes only common.h. The one caller, Viewport__Update
 * (src/code_2cc8c_d.c), declares it locally from LIBGS.H; no shared header
 * carries a Sony prototype (FINISHING-PLAN track 2).
 */
#include "common.h"

/* LIBGS.H: typedef unsigned char PACKET; extern PACKET *GsOUT_PACKET_P; */
extern unsigned char *GsOUT_PACKET_P;

/* LIBGS.H: void GsSetWorkBase(PACKET *outpacketp); */
void GsSetWorkBase(unsigned char *outpacketp) {
    GsOUT_PACKET_P = outpacketp;
}
