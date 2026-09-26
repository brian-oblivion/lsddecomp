/*
 * libspu_s_ih -- Sony's libspu `s_ih` module, carried as C: one function,
 * SpuInitHot, which _SsInit calls where its mode is non-zero (it calls
 * SpuInit when the mode is zero). The body is one call, with argument 1,
 * to func_80038E44, the routine SpuInit itself calls, in the game's own
 * libspu build. That build is on no SDK disc, so the module cannot be
 * linked from lib/ and is matched as C.
 *
 * What decided its edges (python3 tools/tuboundary.py): the placed object
 * libsnd/stop precedes it ("start edge possible"), and the placed object
 * libspu/s_sm (SpuSetMute) follows it. One function between two objects,
 * so nothing to merge with.
 *
 * The carve history is in docs/match-reports/SpuInitHot.md, "File history".
 */
#include "common.h"

/* func_80038E44 is defined in the Psy-Q SPU/SND block at 0x272C8..0x2C054
 * (the game's own libspu build, which no SDK disc has); its own
 * body is a single straight-line path (no branches) ending in a chain of
 * global stores with $v0 never touched afterward -- genuinely void, not
 * just an unobserved return. */
extern void func_80038E44(s32 a0);

void SpuInitHot(void)
{
    func_80038E44(1);
}
