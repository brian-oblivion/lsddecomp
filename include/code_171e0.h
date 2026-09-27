#ifndef CODE_171E0_H
#define CODE_171E0_H

#include "common.h"
#include "FileResource.h"

/* gGameApplicationMethods and its getter GetGameApplicationMethods (defined in this unit) are
 * include/GameApplication.h's. */

/* Some class instance (a slot of gFileResourceMethods's table, going by
 * classtable.py) with at least one flag word at offset 0x24, OR'd with 1 by
 * FileResource__SetFlag. The full layout is derived further down this file, once
 * the method table type it needs (FileResourceMethods) is declared. */


/* The descriptor the LinkResource, Tod, TodSet, ModelData and TriggerWorld
 * ctors take (GraphicsResources.c's ResourceSource, which declares only the
 * first two words): a buffer to adopt, or else (buffer NULL) a file name to
 * request. ResourceRequest__Set fills all three words; every caller passes
 * mode 1, and no ctor reads it. */
typedef struct ResourceRequest {
    /* +0x00 */ void *buffer;
    /* +0x04 */ char *name;
    /* +0x08 */ s32 mode;
} ResourceRequest;

extern void *BMemPMgrAlloc(s32 size); /* one arg confirmed by New_GameApplication.md (code_1677c) */
extern void BMemPMgrFree(void *arg);
extern s32 strlen(char *s);


char *strcat(char *dest, char *src);

#endif
