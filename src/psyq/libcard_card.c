/*
 * libcard_card -- Sony's libcard module `card`: _card_clear, carried as
 * assembly.
 *
 * _card_clear is memory-card library code: it calls only libcard's _new_card
 * and _card_write. The 3.3 disc's libcard/card holds it at the same length
 * and shape, but retail's copy was assembled differently, so the object
 * never places and the function cannot be linked like its neighbours; the
 * file is named for that object. It stays INCLUDE_ASM for good:
 * its word 5 is an `addiu $a1,$zero,0x3F`, the form Sony's libcard
 * assembler emitted, which the pinned pipeline cannot produce (it emits
 * `ori`). The evidence is in docs/match-reports/_card_clear.md.
 *
 * Edges: both are placed Sony objects (libcard/c171 _card_info before,
 * libcard/a78 _card_write after), so this file cannot merge with a
 * neighbour.
 */

#include "common.h"

/* MATCHING: unmatchable as C (Sony-assembled `li` form); keep INCLUDE_ASM. */
INCLUDE_ASM("asm/nonmatchings/psyq/libcard_card", _card_clear);
