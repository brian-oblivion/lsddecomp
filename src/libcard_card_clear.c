/*
 * libcard_card_clear -- Sony's libcard _card_clear, carried as assembly.
 *
 * _card_clear is memory-card library code: it calls only libcard's _new_card
 * and _card_write. Its Sony object is not on any SDK disc in sdk/, so it
 * cannot be linked like its neighbours and the module id is unknown; the
 * file is named for the function instead. It stays INCLUDE_ASM for good:
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
INCLUDE_ASM("asm/nonmatchings/libcard_card_clear", _card_clear);
