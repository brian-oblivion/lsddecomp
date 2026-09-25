#ifndef CODE_171E0_H
#define CODE_171E0_H

#include "common.h"
#include "Class6D430.h"

/* Method table (25 slots per tools/classtable.py) for the class whose
 * constructor caller is New_Class6D3C8 (src unit code_1677c). Not yet named
 * or typed field-by-field -- only its address is needed here, by
 * GetClass6D3C8Methods, which hands it to New_Class6D3C8 so the constructor slot
 * (+0x008, Class6D3C8__Class6D3C8) can be fetched and called indirectly. See
 * CLAUDE.md's "Writing a class method" for the +0x008 constructor-slot
 * convention. */
extern s32 D_8006D3C8[];

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
