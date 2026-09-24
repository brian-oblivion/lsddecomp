/*
 * code_2cc8c_e1 -- ONE function, func_8003FBE4 (4 words), 0x303E4..0x303F4
 * (vram 0x8003FBE4..0x8003FBF4).
 *
 * ROUND 34: this unit exists because the functions on BOTH SIDES of it are
 * Sony's. `code_2cc8c_e` used to hold a contiguous run from SetClipNear to
 * the end of the segment; seven of those functions turned out to be Psy-Q
 * library code, leaving this one wedged between `libgs/gs_123`
 * (Gssub_make_matrix, in front) and `libgs/gs_111` (GsDrawOt, behind). A
 * one-function unit is the only way to keep it as C without putting an `o`
 * segment inside a `c` one -- the same disposition class_3bb8c_v got in
 * round 27 for a function wedged between two trampoline clusters.
 *
 * Deliberately includes only common.h. func_8003FBE4 is also declared in
 * include/code_2cc8c.h for its one caller (Unk18Obj__Update, in code_2cc8c_d),
 * and that declaration -- `extern void func_8003FBE4(void *a0);` -- agrees
 * with the definition below; this unit does not pull the header in, so it
 * shares none and is free to staff alongside anything. Check with
 * `python3 tools/headercontention.py`.
 *
 * The body is unchanged from the one matched in src/code_2cc8c_e.c, and its
 * match report (docs/match-reports/func_8003FBE4.md) still applies verbatim.
 */
#include "common.h"

extern void *D_8008E794;

void func_8003FBE4(void *a0) {
    D_8008E794 = a0;
}
