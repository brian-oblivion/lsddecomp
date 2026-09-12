/*
 * code_2cc8c_e0 -- ONE function, func_8003FB0C (4 words), 0x3030C..0x3031C
 * (vram 0x8003FB0C..0x8003FB1C).
 *
 * ROUND 34: this unit exists because the function BEHIND it turned out to be
 * Sony's. `code_2cc8c_e` used to start here; its second function,
 * func_8003FB1C, is `Gssub_make_matrix` (libgs/gs_123.o, Psy-Q 3.3) and is
 * now linked from the object, which splits the old segment into
 * [c code_2cc8c_e0][o libgs/gs_123][c ...]. A one-function unit is the
 * cheapest way to keep this function as C without dragging the object into a
 * C segment.
 *
 * Deliberately includes only common.h. func_8003FB0C is also declared in
 * include/code_2cc8c.h for its one caller (func_8003EEC0, in code_2cc8c_d),
 * and that declaration -- `extern void func_8003FB0C(void *a0);` -- agrees
 * with the definition below; this unit does not pull the header in, so it
 * shares none and is free to staff alongside anything. Check with
 * `python3 tools/headercontention.py`.
 *
 * The body is unchanged from the one matched in src/code_2cc8c_e.c, and its
 * match report (docs/match-reports/func_8003FB0C.md) still applies verbatim.
 */
#include "common.h"

extern void *D_800902E4;

void func_8003FB0C(void *a0) {
    D_800902E4 = a0;
}
