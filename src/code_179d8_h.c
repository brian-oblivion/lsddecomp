/*
 * The CD driver's blocking file access, and a pair that turns a FileResource
 * into a CD driver.
 *
 * OpenCdFile, CloseCdFile, GetCdFileSize and ReadCdFile are what CdDriver's
 * open, close, seek and read slots (code_179d8_s.c) call when the driver is
 * not in async mode. Like those slots they run on whichever FileResource
 * object called them (CdDriver.h's banner) and use only its isOpen, pos and
 * size. OpenCdFile looks the name up with CdSearchFile under the path
 * BuildCdFilePath makes ("\\<data directory><name>;1") and records where
 * the file starts and how long it is; ReadCdFile seeks to that start and
 * reads whole sectors, starting over from the seek on a disk error. In this
 * mode a seek does not move: CdDriver__Seek returns GetCdFileSize and every
 * read begins at the start of the file.
 *
 * FileResource__InstallCdReadDriver runs FileResource's ctor, then gives the
 * object gCdDriverMethods and marks it closed; FileResource__DestroyCdReadDriver
 * runs FileResource's finalize. GetCdUseVSyncCallback is the CD half of
 * code_171e0.c's GetActiveDataSourceUseVSyncCallback.
 *
 * Nothing in the executable calls the install/destroy pair or NoOp2, NoOp3
 * and NoOp4 (no jal, stored pointer or built address reaches them).
 */
#include "common.h"
#include <libcd.h>
#include "CdDriver.h"
/* FileResource and its table come from include/FileResource.h, through code_171e0.h. */
#include "code_171e0.h"

/* Defined in other units: GetDataDirectory (code_171e0.c) returns the data
 * directory's name; strcpy and strcat are Sony's libc2. */
extern void printf(const char *fmt, void *arg);
extern char *GetDataDirectory(void);
extern char *strcpy(char *dest, char *src);
extern char *strcat(char *dest, char *src);

extern char gCdFileNotFoundFmt[];   /* "File not found. path = %s\n" */
extern char gCdFileVersionSuffix[]; /* ";1", the ISO9660 CD file-version suffix */

/* Defined below, after its caller OpenCdFile: functions stay in ROM order. */
char *BuildCdFilePath(char *dest, char *name);

void FileResource__InstallCdReadDriver(FileResource *self) {
    GetFileResourceMethods()->ctor(self);
    self->methods = (FileResourceMethods *)GetCdDriverMethods();
    self->isOpen = 0;
}

void FileResource__DestroyCdReadDriver(FileResource *self) {
    GetFileResourceMethods()->finalize(self);
}

void NoOp2(void) {}

/* Resolves `name` once and marks the object open; an open object is left
 * alone. After CD_SEARCH_ATTEMPTS failed lookups it prints the path and
 * returns with the object still closed.
 * MATCHING: the retry is a label and goto; a while or for loop hoists &path
 * out of it and rotates the saved registers. */
void OpenCdFile(CdDriver *self, char *name) {
    s32 retries;
    CdlFILE file;
    char path[CD_PATH_SIZE];

    retries = 0;
    if (self->isOpen == 0) {
        BuildCdFilePath(path, name);
    retry:
        if (CdSearchFile(&file, path) == 0) {
            if (retries++ < CD_SEARCH_ATTEMPTS - 1) {
                goto retry;
            }
            printf(gCdFileNotFoundFmt, path);
            return;
        }
        /* CdLoc16 is the project's spelling of CdlLOC's four bytes (FileResource.h). */
        self->pos = *(CdLoc16 *)&file.pos;
        self->size = file.size;
        self->isOpen = 1;
    }
}

char *BuildCdFilePath(char *dest, char *name) {
    dest[0] = '\\';
    strcpy(dest + 1, GetDataDirectory());
    strcat(dest, name);
    strcat(dest, gCdFileVersionSuffix);
    return dest;
}

void CloseCdFile(CdDriver *self) {
    if (self->isOpen != 0) {
        self->isOpen = 0;
    }
}

/* The open file's size in whole sectors, one sector over when it is
 * already a multiple (CdDriver__Seek's async path rounds up only when it is
 * not); 0 when the object is closed. */
s32 GetCdFileSize(CdDriver *self) {
    u32 result;

    if (self->isOpen == 0) {
        result = 0;
    } else {
        result = ((self->size >> CD_SECTOR_SHIFT) + 1) << CD_SECTOR_SHIFT;
    }
    return result;
}

void NoOp3(void) {}

/* Reads `size` bytes, rounded down to whole sectors, from the start of the
 * open file into `buf`, and returns 0. A closed object is sent to its own
 * close slot instead.
 * MATCHING: the seek retry and the CdSync wait are label and goto loops and
 * only the CdReadSync wait is a do-while; any other loop kind moves the
 * branch targets. */
s32 ReadCdFile(CdDriver *self, void *buf, s32 size) {
    s32 sectors;
    s32 status;
    char scratch[2048]; /* MATCHING: never used; it sizes the frame so syncResult sits where retail's does */
    u_char syncResult[16]; /* CdSync writes 8 bytes; 16 is the size retail reserved */

    if (self->isOpen != 0) {
    retry:
        sectors = (u32)size >> CD_SECTOR_SHIFT;
        CdControl(CdlSetloc, (u_char *)&self->pos, 0); /* pos is CdLoc16, Sony's CdlLOC by layout */
    sync:
        status = CdSync(0, syncResult);
        if (status == CdlNoIntr) {
            goto sync;
        }
        if (status == CdlDiskError) {
            goto retry;
        }
        if (sectors != 0) {
            CdRead(sectors, (u_long *)buf, CdlModeSpeed);
            do {
                status = CdReadSync(0, 0);
            } while (status > 0);
            if (status == -1) {
                goto retry;
            }
            return 0;
        }
    } else {
        self->methods->close(self);
    }
    return 0;
}

void NoOp4(void) {}

s32 GetCdUseVSyncCallback(void) {
    return gCdUseVSyncCallback;
}
