/*
 * code_179d8_f_b -- the TAIL half of the old code_179d8_f slice, split off in
 * round 34 (2026-09-12) when Sony's `libsnd/stop.o` was linked into the middle
 * of it. File 0x272A8..0x272C8, vram 0x80036AA8..0x80036AC8: ONE function.
 *
 * WHY THE SPLIT EXISTS. `func_800368E8`, `func_80036A54` and `func_80036A7C`
 * are Sony's `Snd_stop`, `SsSeqStop` and `SsSepStop` (`libsnd/stop`, Psy-Q
 * 3.3, 0x1C0 text covering exactly those three). All three had been MATCHED as
 * C; reclassifying them out of the game count is the correction CLAUDE.md asks
 * for, not a regression. A placed object cannot live inside a `c` segment, so
 * the slice had to become [c][o][c] and the second `c` needed its own name.
 * The first half kept `code_179d8_f`.
 *
 * A ONE-FUNCTION UNIT IS FINE and has precedent (`class_3bb8c_v`). Nothing
 * here is a candidate for folding into a neighbour: `code_179d8_f` ends at the
 * object, and 0x272C8 onward is the psyq_SpuSetMute block.
 *
 * NO RODATA ATTACH CAME WITH THIS HALF, and that is measured, not assumed:
 * the old code_179d8_f owned zero `jtbl_` references and the splat yaml's
 * rodata slot list names no `.rodata, code_179d8_f` line at all. So unlike
 * round 33's code_179d8_c_b there was nothing to move, and a link failure of
 * the form `undefined reference to '.L8003....'` would mean something else.
 *
 * Declarations: keep anything that encodes THIS unit's reading next to the
 * code, in this file. Do not create a shared code_179d8*.h -- the sibling
 * slices are staffed independently and a shared header is what makes their
 * merges collide.
 *
 * ROUND 78 (bravo), track 3 naming pass: `func_80036AA8` stays a
 * placeholder. It is a free function (no class table entry, no vtable slot),
 * its only caller (`_SsInit`, `src/code_179d8_c.c`) has just this one call
 * site to go on, and its callee `func_80038E44` is itself unnamed and
 * uncarved. See `docs/match-reports/func_80036AA8.md`'s `## Naming` section
 * for the full evidence trail. No fields, slots or globals in this unit to
 * name.
 */
#include "common.h"

/* func_80038E44 is defined in the Psy-Q SPU/SND block at 0x272C8..0x2C054
 * (the game's own libspu build, which no SDK disc has); its own
 * body is a single straight-line path (no branches) ending in a chain of
 * global stores with $v0 never touched afterward -- genuinely void, not
 * just an unobserved return. */
extern void func_80038E44(s32 a0);

void func_80036AA8(void)
{
    func_80038E44(1);
}
