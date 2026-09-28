#ifndef CLASS_16334_H
#define CLASS_16334_H

#include "common.h"
#include <libetc.h>
#include "Pad.h"

/* class_16334 is the Pad class's unit: every function in it is a Pad method
 * or New_Pad. The class itself (object, method table, methods) is
 * include/Pad.h; this header keeps what only the unit's own bodies use.
 * The pad library it wraps (PadInit, PadRead, PadStop) is declared by Sony's
 * <libetc.h>; main() is the one creator, New_Pad(0, 0). */

extern void *BMemPMgrAlloc(s32 size);

extern s32 sPadRefCount; /* live instances: the first ctor calls PadInit, the last finalize PadStop */
extern u32 sButtonMasks[PAD_BUTTON_COUNT]; /* runtime copy of the button-mask table, filled by Pad__LoadButtonTable */

/* A 0x40-byte block, copied as a whole (GCC's inlined block-move codegen for
 * a struct assignment, not a word loop) rather than word-indexed. */
typedef struct {
    u32 w[16];
} Block64;

/* The unit's own read-only table of libetc's 16 button masks, in enum
 * PadButton order (PADLup, PADLdown, ... PADstart). */
extern Block64 sDefaultButtonMasks;

#endif
