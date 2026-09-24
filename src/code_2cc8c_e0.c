/*
 * code_2cc8c_e0 -- ONE function, SetClipNear (4 words), 0x3030C..0x3031C
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
 * Deliberately includes only common.h. SetClipNear is also declared in
 * include/code_2cc8c.h for its one caller (Unk18Obj__Update, in code_2cc8c_d),
 * and that declaration -- `extern void SetClipNear(void *a0);` -- agrees
 * with the definition below; this unit does not pull the header in, so it
 * shares none and is free to staff alongside anything. Check with
 * `python3 tools/headercontention.py`.
 *
 * The body is unchanged from the one matched in src/code_2cc8c_e.c, and its
 * match report (docs/match-reports/SetClipNear.md) still applies verbatim.
 *
 * ROUND 78 (naming pass): this game function's one write target,
 * `GsCLIP3near`, turned out to be Sony's own bss global (pinned in
 * config/psyq-objects.ld by fourteen libgs objects, sibling of
 * GsCLIP3far at 0x8008E9D8) -- so the extern below keeps SONY'S name,
 * never a game one, per CLAUDE.md. The function itself stays game code:
 * only the OTHER function that used to share this unit's old home
 * (Gssub_make_matrix, noted above) was Sony's.
 */
#include "common.h"

extern void *GsCLIP3near;

void SetClipNear(void *a0) {
    GsCLIP3near = a0;
}
