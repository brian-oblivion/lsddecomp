#ifndef REQUESTEDFILE_H
#define REQUESTEDFILE_H

#include "file_resource.h"

/*
 * RequestedFile -- one whole file, requested from the active data-source
 * driver at construction, with a flag that says when it has arrived (class
 * id 0xB03, method table gRequestedFileMethods, parent FileResource; methods
 * in src/graphics/sprite.c; no subclasses).
 *
 * New_RequestedFile(name) allocates it and the ctor passes a 32-byte stack
 * copy of the name to requestLoadFile (+0x06C). On the CD driver with async
 * loading on, that queues a load-file request, which CdDriver__RunRequestQueue
 * dispatches to loadFile and, once the read completes, reports through
 * onRequestDone (+0x064); otherwise loadFile runs at once and calls onRequestDone itself.
 * Either way the file lands in FileResource's `buffer`, and onRequestDone's
 * occupant here, RequestedFile__MarkLoaded, sets `loaded`. The ctor and
 * finalize clear it. The class adds no step that consumes the buffer
 * (+0x078 is not in its table): the owner reads `buffer` itself once
 * `loaded` is set, and releases the object (FileResource__Release) when done.
 *
 * Like every FileResource client it runs on the active driver: the ctor and
 * finalize chain to GetActiveDataSourceMethods()'s first, and
 * GetRequestedFileMethods is in sDataSourceClientGetters, so
 * SetActiveDataSource rebinds this table's file-I/O slots.
 *
 * Its one user is WBgm (include/wbgm.h): WBgm__SetSeq makes one per SEQ
 * path (`seqData`), and WBgm__HandleMonitorEvent passes its `buffer` to
 * SsSeqOpen once `loaded` is set.
 */

typedef struct RequestedFile RequestedFile;
typedef struct RequestedFileMethods RequestedFileMethods;

/* The ctor's stack copy of the name it hands requestLoadFile: 32 bytes,
 * NUL included (strcpy, unchecked). */
#define REQUESTEDFILE_NAME_SIZE 32

struct RequestedFileMethods {
    FILERESOURCE_SLOTS(RequestedFile, (RequestedFile * self, char *name));
};

struct RequestedFile {
    FILERESOURCE_FIELDS(RequestedFileMethods);
    /* +0x02C */ s32 loaded; /* 1 once onRequestDone reports the requested file loaded; cleared by the ctor and RequestedFile__Finalize */
}; /* 0x30 bytes: New_RequestedFile */

extern RequestedFileMethods gRequestedFileMethods;
extern RequestedFileMethods *GetRequestedFileMethods(void);

RequestedFile *New_RequestedFile(char *name);
void RequestedFile__RequestedFile(RequestedFile *self, char *name);
void RequestedFile__Finalize(RequestedFile *self);
void RequestedFile__MarkLoaded(RequestedFile *self);

#endif
