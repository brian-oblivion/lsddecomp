#ifndef GAMEAPPLICATIONFILERESOURCE_H
#define GAMEAPPLICATIONFILERESOURCE_H

#include "common.h"
#include "FileResource.h"

/* gGameApplicationMethods and its getter GetGameApplicationMethods (defined in this unit) are
 * include/GameApplication.h's. */

/* Some class instance (a slot of gFileResourceMethods's table, going by
 * classtable.py) with at least one flag word at offset 0x24, OR'd with 1 by
 * FileResource__SetFlag. The full layout is derived further down this file, once
 * the method table type it needs (FileResourceMethods) is declared. */


/* ResourceRequest, which ResourceRequest__Set fills, is include/FileResource.h's. */

extern void *BMemPMgrAlloc(s32 size); /* one arg confirmed by New_GameApplication.md (GameApplicationFileResource) */
extern void BMemPMgrFree(void *arg);
extern s32 strlen(char *s);


char *strcat(char *dest, char *src);

#endif
