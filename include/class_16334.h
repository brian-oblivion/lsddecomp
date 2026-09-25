#ifndef CLASS_16334_H
#define CLASS_16334_H

#include "common.h"
#include "Pad.h"

/* class_16334 is the Pad class's unit: every function in it is a Pad method
 * or New_Pad. The class itself (object, method table, methods) is
 * include/Pad.h; this header keeps what only the unit's own bodies use. */

extern void *BMemPMgrAlloc(s32 size);

/* Psy-Q pad library (libetc, include/psyq/LIBETC.H). */
extern void PadInit(void *arg1);
extern u32 PadRead(s32 port);
extern void PadStop(void);

extern s32 sPadRefCount;          /* live instances: the first ctor calls PadInit, the last finalize PadStop */
extern u32 sButtonMasks[16];      /* runtime copy of the button-mask table, filled by Pad__LoadButtonTable */

/* A 0x40-byte block, copied as a whole (GCC's inlined block-move codegen for
 * a struct assignment, not a word loop) rather than word-indexed. */
typedef struct { u32 w[16]; } Block64;
extern Block64 D_80010764;      /* Psy-Q's own default button-mask table (psyq_15d04 rodata) */

#endif
