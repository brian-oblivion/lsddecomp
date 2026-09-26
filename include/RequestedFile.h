#ifndef REQUESTEDFILE_H
#define REQUESTEDFILE_H

#include "FileResource.h"

/*
 * RequestedFile -- a FileResource data source (class id 0xB03, method table
 * gRequestedFileMethods) that requests one named file at construction and
 * records when the load has completed. Methods in src/code_322b4.c. No
 * classes derive from it. Its getter is an entry of
 * gDataSourceClientGetters, so SetActiveDataSource rebinds its interface
 * slots like every other client's.
 *
 * PARENT BY CTOR CHAIN: RequestedFile__RequestedFile's first call is
 * GetActiveDataSourceMethods()->ctor, and RequestedFile__Finalize forwards to
 * the active driver's finalize, as Class6D940 and TimBlockSrc do.
 *
 * Own mechanics, all three methods: the ctor clears `loaded` and, given a
 * name, passes a 32-byte stack copy of it to requestLoadFile (+0x06C); the
 * setFlag override (+0x064, RequestedFile__SetFlag) sets `loaded` to 1 -- the
 * CD driver calls setFlag when a queued operation completes
 * (CdDriver__LoadFile, the request-queue dispatch in code_179d8_s) --
 * and finalize clears it again. Nothing overrides loadFile (+0x058 is NULL
 * in the static table), so the file lands in FileResource's `buffer`.
 *
 * Its one user is WBgm (src/code_2a0e0.c): WBgm__SetSeq makes one per SEQ
 * name, and WBgm__HandleMonitorEvent waits for `loaded` before passing
 * `buffer` to SsSeqOpen. That is the caller's use, not the class's
 * mechanics, so the name stays address-derived.
 */

typedef struct RequestedFile RequestedFile;
typedef struct RequestedFileMethods RequestedFileMethods;

struct RequestedFileMethods {
    FILERESOURCE_SLOTS(RequestedFile, (RequestedFile * self, char *name));
};

struct RequestedFile {
    FILERESOURCE_FIELDS(RequestedFileMethods);
    /* +0x02C */ s32 loaded; /* 1 once setFlag reports the requested file loaded; cleared by the ctor and RequestedFile__Finalize */
}; /* 0x30 bytes: New_RequestedFile */

extern RequestedFileMethods gRequestedFileMethods;
extern RequestedFileMethods *GetRequestedFileMethods(void);

RequestedFile *New_RequestedFile(char *name);
void RequestedFile__RequestedFile(RequestedFile *self, char *name);
void RequestedFile__Finalize(RequestedFile *self);
void RequestedFile__SetFlag(RequestedFile *self);

#endif
