/*
 * code_2cc8c_e0 -- ONE function, Sony's GsSetNearClip (libgs/gs_101), 4 words,
 * 0x3030C..0x3031C (vram 0x8003FB0C..0x8003FB1C).
 *
 * SONY CODE, written as C. Round 78's head identified it under FINISHING-PLAN
 * track 2: the body is exactly `GsCLIP3near = clip_near;`, `sdkname.py` scores
 * it EXACT (TINY, 4 words) against libgs/gs_101 GsSetNearClip on discs
 * 3.3/3.5/3.6, its one store target 0x800902E4 is `GsCLIP3near` (pinned in
 * config/psyq-objects.ld by fourteen libgs objects), and it sits inside the
 * run of placed libgs objects (after gs_133, before gs_123). The prototype is
 * LIBGS.H's own. `progress.py` counts it as library through the `identified`
 * comment on its symbols entry, so it is outside every game queue.
 *
 * ROUND 34: this unit exists because the object behind it, libgs/gs_123
 * (Gssub_make_matrix), is linked, which split the old code_2cc8c_e segment
 * into [c code_2cc8c_e0][o libgs/gs_123][c ...]. gs_101 itself never placed
 * as an object (no .o segment here), so the function stays as C.
 *
 * Deliberately includes only common.h. The one caller, Viewport__Update
 * (src/code_2cc8c_d.c), declares it locally from LIBGS.H; no shared header
 * carries a Sony prototype (FINISHING-PLAN track 2).
 */
#include "common.h"

/* Sony's bss global, pinned in config/psyq-objects.ld. */
extern long GsCLIP3near;

/* LIBGS.H: void GsSetNearClip(long clip_near); */
void GsSetNearClip(long clip_near) {
    GsCLIP3near = clip_near;
}
