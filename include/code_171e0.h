#ifndef CODE_171E0_H
#define CODE_171E0_H

#include "common.h"
#include "Class6D430.h"

/* D_8006D3C8 and its getter GetClass6D3C8Methods (defined in this unit) are
 * include/Class6D3C8.h's. */

/* Some class instance (a slot of D_8006D430's table, going by
 * classtable.py) with at least one flag word at offset 0x24, OR'd with 1 by
 * Class6D430__SetFlag. The full layout is derived further down this file, once
 * the method table type it needs (Class6D430Methods) is declared. */


/* A 3-word vector-like object, written wholesale by SetVec3. Only the
 * first three words are touched; nothing here says whether a further field
 * follows. */
typedef struct Vec3_171e0 {
    s32 x;
    s32 y;
    s32 z;
} Vec3_171e0;

extern void *BMemPMgrAlloc(s32 size); /* one arg confirmed by New_Class6D3C8.md (code_1677c) */
extern void BMemPMgrFree(void *arg);
extern s32 strlen(char *s);


char *strcat(char *dest, char *src);

#endif
