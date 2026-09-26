#ifndef CLASS6EED8_H
#define CLASS6EED8_H

#include "Class6D430.h"

/*
 * Class6EED8 -- a Class6D430 data source (class id 0xB03, method table
 * gClass6EED8Methods) that requests one named file at construction and
 * records when the load has completed. Methods in src/code_322b4.c. No
 * classes derive from it. Its getter is an entry of
 * gDataSourceClientGetters, so SetActiveDataSource rebinds its interface
 * slots like every other client's.
 *
 * PARENT BY CTOR CHAIN: Class6EED8__Class6EED8's first call is
 * GetActiveDataSourceMethods()->ctor, and Class6EED8__Finalize forwards to
 * the active driver's finalize, as Class6D940 and TimBlockSrc do.
 *
 * Own mechanics, all three methods: the ctor clears `loaded` and, given a
 * name, passes a 32-byte stack copy of it to requestLoadFile (+0x06C); the
 * setFlag override (+0x064, Class6EED8__SetFlag) sets `loaded` to 1 -- the
 * CD driver calls setFlag when a queued operation completes
 * (Class6D4E8__LoadFile, the request-queue dispatch in code_179d8_s) --
 * and finalize clears it again. Nothing overrides loadFile (+0x058 is NULL
 * in the static table), so the file lands in Class6D430's `buffer`.
 *
 * Its one user is WBgm (src/code_2a0e0.c): WBgm__SetSeq makes one per SEQ
 * name, and WBgm__HandleMonitorEvent waits for `loaded` before passing
 * `buffer` to SsSeqOpen. That is the caller's use, not the class's
 * mechanics, so the name stays address-derived.
 */

typedef struct Class6EED8 Class6EED8;
typedef struct Class6EED8Methods Class6EED8Methods;

struct Class6EED8Methods {
    CLASS6D430_SLOTS(Class6EED8, (Class6EED8 *self, char *name));
};

struct Class6EED8 {
    CLASS6D430_FIELDS(Class6EED8Methods);
    /* +0x02C */ s32 loaded; /* 1 once setFlag reports the requested file loaded; cleared by the ctor and Class6EED8__Finalize */
};                           /* 0x30 bytes: New_Class6EED8 */

extern Class6EED8Methods gClass6EED8Methods;
extern Class6EED8Methods *GetClass6EED8Methods(void);

Class6EED8 *New_Class6EED8(char *name);
void Class6EED8__Class6EED8(Class6EED8 *self, char *name);
void Class6EED8__Finalize(Class6EED8 *self);
void Class6EED8__SetFlag(Class6EED8 *self);

#endif
