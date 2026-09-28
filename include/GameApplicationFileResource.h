#ifndef GAMEAPPLICATIONFILERESOURCE_H
#define GAMEAPPLICATIONFILERESOURCE_H

/* src/GameApplicationFileResource.c's own view of what it calls. The
 * classes it defines are declared by their own headers: GameApplication in
 * include/GameApplication.h, FileResource and ResourceRequest in
 * include/FileResource.h (included here for code_179d8_h.c, which reaches
 * it through this header). */

#include "common.h"
#include "FileResource.h"

/* The game's allocator, in the uncarved BMemPMgr block. Returns void *
 * rather than a typed pointer because every New_X in the game calls it
 * (one argument, confirmed by New_GameApplication.md). */
extern void *BMemPMgrAlloc(s32 size);
extern void BMemPMgrFree(void *arg);
extern s32 strlen(char *s);

char *strcat(char *dest, char *src);

#endif
