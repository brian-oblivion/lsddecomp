/*
 * code_179d8_i_b -- the TAIL half of the old code_179d8_i slice, split off in
 * round 34 (2026-09-12) when Sony's `libsnd/replay.o` and `libsnd/vs_vab.o`
 * were linked into the middle of it. File 0x2490C..0x24938, vram
 * 0x8003410C..0x80034138: ONE function.
 *
 * WHY THE SPLIT EXISTS. `func_80033FB8`, `func_80034020` and `func_800340B0`
 * are Sony's `Snd_replay` (`libsnd/replay`, Psy-Q 3.3) and `SsVabClose` /
 * `SsVabOpen` (`libsnd/vs_vab`, Psy-Q 3.3). All three had been MATCHED as C;
 * reclassifying them out of the game count is the correction CLAUDE.md asks
 * for, not a regression. A placed object cannot live inside a `c` segment, so
 * the slice had to become [c][o][o][c] and the second `c` needed its own name.
 * The first half kept `code_179d8_i` and is now func_80033C90 alone.
 *
 * A ONE-FUNCTION UNIT IS FINE and has precedent (`class_3bb8c_v`, and
 * `code_179d8_f_b` earlier in this same round).
 *
 * NO RODATA ATTACH CAME WITH THIS HALF, and that is measured, not assumed:
 * code_179d8_i owned zero `jtbl_` references and the splat yaml's rodata slot
 * list names no `.rodata, code_179d8_i` line at all -- both of the old
 * code_179d8_tail's jump tables went to code_179d8_k. So unlike round 33's
 * code_179d8_c_b there was nothing to move, and a link failure of the form
 * `undefined reference to '.L8003....'` would mean something else.
 *
 * BLOCKER PROFILE: screen with `python3 tools/nearmiss.py`, never by
 * re-implementing the greps and never for `addiu_at` (resolved round 21).
 * This unit's one function is matched, so there is nothing queued here.
 *
 * Declarations: keep anything that encodes THIS unit's reading next to the
 * code, in this file. Do not create a shared code_179d8*.h -- the sibling
 * slices are staffed independently and a shared header is what makes their
 * merges collide.
 */
#include "common.h"

extern s32 func_80034138(s16 a0, s16 a1);

s32 func_8003410C(s16 a0, s16 a1)
{
    return func_80034138(a0, a1);
}
