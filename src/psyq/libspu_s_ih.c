/*
 * libspu_s_ih -- Sony's libspu `s_ih` module, carried as C: one function,
 * SpuInitHot, which _SsInit calls where its mode is non-zero (it calls
 * SpuInit when the mode is zero). The body is one call, with argument 1,
 * to _SpuInit, the routine SpuInit itself calls, in the game's own
 * libspu build. That build is on no SDK disc, so the module cannot be
 * linked from lib/ and is matched as C.
 */
#include "common.h"

/* libspu's s_ini: the SPU reset that SpuInit (mode 0) and SpuInitHot
 * (mode 1) share. It returns nothing. */
extern void _SpuInit(s32 mode);

void SpuInitHot(void) {
    _SpuInit(1);
}
