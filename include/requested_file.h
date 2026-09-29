#ifndef REQUESTED_FILE_H
#define REQUESTED_FILE_H

#include "file_resource.h"

/**
 * @file requested_file.h
 * @brief RequestedFile, one whole file requested from the active data-source
 *        driver at construction, with a flag that says when it has arrived.
 *
 * New_RequestedFile(name) allocates it and the ctor passes a 32-byte stack
 * copy of the name to requestLoadFile (+0x06C). On the CD driver with async
 * loading on, that queues a load-file request, which CdDriver__RunRequestQueue
 * dispatches to loadFile and, once the read completes, reports through
 * onRequestDone (+0x064); otherwise loadFile runs at once and calls
 * onRequestDone itself. Either way the file lands in FileResource's
 * `buffer`, and onRequestDone's occupant here, RequestedFile__MarkLoaded,
 * sets `loaded`. The class adds no step that consumes the buffer (+0x078 is
 * not in its table): the owner reads `buffer` itself once `loaded` is set,
 * and releases the object (FileResource__Release) when done.
 *
 * Its one user is WBgm (include/wbgm.h): WBgm__SetSeq makes one per SEQ
 * path (`seqData`), and WBgm__HandleMonitorEvent passes its `buffer` to
 * SsSeqOpen once `loaded` is set.
 */

typedef struct RequestedFile RequestedFile;
typedef struct RequestedFileMethods RequestedFileMethods;

/** RequestedFile's class id (gRequestedFileMethods word +0x000): 0xB under
 * FileResource's 0x3. */
#define REQUESTEDFILE_CLASS_ID 0xB03

/** The ctor's stack copy of the name it hands requestLoadFile: 32 bytes,
 * NUL included (strcpy, unchecked). */
#define REQUESTEDFILE_NAME_SIZE 32

/** RequestedFile's method table: FileResource's slots up to +0x074
 * (FILERESOURCE_BASE_SLOTS; there is no processBuffer) with its ctor
 * parameters. */
struct RequestedFileMethods {
    FILERESOURCE_BASE_SLOTS(RequestedFile, (RequestedFile * self, char *name));
};

/**
 * RequestedFile: a FileResource that asks for one named file when it is
 * built. Class id 0xB03, table gRequestedFileMethods, parent FileResource;
 * methods in src/graphics/sprite.c; no subclasses. The object is 0x30 bytes
 * (New_RequestedFile).
 *
 * Like every FileResource client it runs on the active driver: the ctor and
 * finalize chain to GetActiveDataSourceMethods()'s, and
 * GetRequestedFileMethods is in sDataSourceClientGetters, so
 * SetActiveDataSource rebinds this table's file-I/O slots.
 */
struct RequestedFile {
    FILERESOURCE_FIELDS(RequestedFileMethods);
    /* +0x02C */ s32 loaded; /**< 1 once onRequestDone reports the requested file loaded; cleared by the ctor and RequestedFile__Finalize */
};

/** RequestedFile's method table (class id 0xB03). */
extern RequestedFileMethods gRequestedFileMethods;

/**
 * @brief The RequestedFile method table.
 * @return &gRequestedFileMethods.
 */
extern RequestedFileMethods *GetRequestedFileMethods(void);

/**
 * @brief Allocates a RequestedFile and runs its ctor, which requests `name`.
 * @param name The file's path, at most 31 characters.
 * @return The new object, or NULL when the allocation fails.
 */
RequestedFile *New_RequestedFile(char *name);

/**
 * @brief Constructor (slot +0x008): the active driver's ctor, installs
 *        gRequestedFileMethods, clears `loaded`, and, when `name` is not
 *        NULL, passes a stack copy of it to requestLoadFile.
 * @param self The object.
 * @param name The file's path, at most 31 characters, or NULL to request
 *        nothing.
 */
void RequestedFile__RequestedFile(RequestedFile *self, char *name);

/**
 * @brief Finalizer (slot +0x00C): clears `loaded`, then the active driver's
 *        finalize.
 * @param self The object.
 */
void RequestedFile__Finalize(RequestedFile *self);

/**
 * @brief onRequestDone (slot +0x064): the driver reports the requested file
 *        loaded; sets `loaded`.
 * @param self The object.
 */
void RequestedFile__MarkLoaded(RequestedFile *self);

#endif
