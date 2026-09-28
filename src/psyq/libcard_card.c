/*
 * libcard_card -- Sony's libcard module `card`: _card_clear, which clears a
 * memory card through libcard's _new_card and _card_write. The 3.3 disc's
 * libcard/card holds it at the same length and shape but not in the same
 * bytes, so the object cannot be linked in its place; the file is named
 * for that object. The function stays disassembly.
 */

#include "common.h"

/* MATCHING: unmatchable as C (Sony-assembled `li` form); keep INCLUDE_ASM. */
INCLUDE_ASM("asm/nonmatchings/psyq/libcard_card", _card_clear);
