#ifndef GAMEAPPLICATIONFILERESOURCE_H
#define GAMEAPPLICATIONFILERESOURCE_H

/* src/app/GameApplicationFileResource.c's own view of what it calls. The
 * classes it defines are declared by their own headers: GameApplication in
 * include/GameApplication.h, FileResource and ResourceRequest in
 * include/FileResource.h (included here for CdDriver.c, which reaches
 * it through this header). */

#include "common.h"
#include "FileResource.h"

/* The game's pool allocator (src/app/BMemPMgr.c), as every New_X calls it:
 * one argument, returning void *. */
extern void *BMemPMgrAlloc(s32 size);
extern void BMemPMgrFree(void *arg);
extern s32 strlen(char *s);

char *strcat(char *dest, char *src);

#endif
