/*
 * code_171e0 -- FileResource's own methods, the active-data-source dispatch
 * layer on top of them, and the data directory that CD paths are built in.
 *
 * FileResource (include/FileResource.h) is the base of every class the
 * game loads from a file: a BasicClass subclass owning one file buffer
 * (FileResource__LoadFile reads a whole named file into it, FreeBuffer
 * releases it) and declaring the file-I/O interface that the CD driver
 * (gCdDriverMethods, include/CdDriver.h) and the SPU/VAB driver
 * (gVabDriverMethods, include/VabDriver.h) implement.
 *
 * gActiveDataSource selects one of those two drivers. SetActiveDataSource
 * installs one and copies its interface slots into FileResource's table and
 * into every client table. The Lock/Unlock, IsBusy/Idle, Get.../Set...
 * functions after it forward to the CD driver when it is active, and
 * otherwise do nothing, return a fixed value or call the SPU/VAB driver.
 * RegisterFileTableEntries appends CdFileEntry records to the CD driver's
 * file table and resolves them.
 *
 * SetDataDirectory/GetDataDirectory hold the directory that BuildCdFilePath
 * and CdStream__Open put between the root `\` and a file name. It is "" until
 * GameApplication's ctor installs "CDI\". BuildFileName joins an optional
 * directory, a name and an extension. ResourceRequest__Set fills the
 * {buffer, name, mode} descriptor the resource classes' ctors take, and
 * GetGameApplicationMethods is GameApplication's table getter. strcat, which
 * BuildFileName calls, is Sony's libc2 object, linked after this unit.
 */
#include "common.h"
#include "code_171e0.h"
#include "VabDriver.h"
#include "CdDriver.h"
#include "GameApplication.h"

/* gActiveDataSource's two observed values are the header words of the two
 * sibling classes it selects between: gCdDriverMethods (the CD-ROM read driver,
 * code_179d8_q.c) and gVabDriverMethods (VabDriver, the SPU/VAB driver, include/VabDriver.h). */
#define DATASOURCE_CD 0x13
#define DATASOURCE_SPU 0x23

GameApplicationMethods *GetGameApplicationMethods(void) {
    return &gGameApplicationMethods;
}

void *FileResource__Release(FileResource *this) {
    this->freeGuard = 0;
    this->methods->finalize(this);
    Get_vtable_BasicClass()->finalize((BasicClass *)this);
    BMemPMgrFree(this);
    return NULL;
}

void FileResource__FileResource(FileResource *this) {
    Get_vtable_BasicClass()->ctor((BasicClass *)this);
    this->methods = GetFileResourceMethods();
    this->isOpen = 0;
    this->buffer = NULL;
    this->bufferSize = 0;
    this->freeGuard = 0;
    this->pendingRequests = 0;
    this->flags = 0;
    this->inQueueDispatch = 0;
    this->loadState = 0;
}

void FileResource__Finalize(FileResource *this) {
    this->methods->close(this);
    this->methods->freeBuffer(this);
}

void FileResource__LoadFile(FileResource *this, char *name) {
    s32 savedIsOpen;
    s32 size;
    void *buffer;

    if (this->buffer != NULL) {
        return;
    }
    savedIsOpen = this->isOpen;
    this->isOpen = 0;
    this->methods->open(this, name, 1, 0);
    size = this->methods->seek(this, 0, 2);
    buffer = BMemPMgrAlloc(size);
    if (buffer != NULL) {
        this->methods->seek(this, 0, 0);
        this->methods->read(this, buffer, size);
        this->methods->close(this);
        this->buffer = buffer;
        this->bufferSize = size;
        this->isOpen = savedIsOpen;
    } else {
        BMemPMgrFree(NULL);
        this->methods->close(this);
    }
}

void FileResource__FreeBuffer(FileResource *this) {
    if (this->buffer == NULL) {
        return;
    }
    if (this->bufferSize == 0) {
        return;
    }
    if (this->freeGuard != 0) {
        return;
    }
    BMemPMgrFree(this->buffer);
    this->buffer = NULL;
}

void NoOp(void) {}

void FileResource__SetFlag(FileResource *this) {
    this->flags |= 1;
}

FileResourceMethods *GetFileResourceMethods(void) {
    return &gFileResourceMethods;
}

extern s32 gActiveDataSource;

void *GetActiveDataSourceMethods(void) {
    if (gActiveDataSource == DATASOURCE_SPU) {
        return GetVabDriverMethods();
    } else {
        return GetCdDriverMethods();
    }
}

ResourceRequest *ResourceRequest__Set(ResourceRequest *this, void *buffer, char *name, s32 mode) {
    this->src.buffer = buffer;
    this->src.name = name;
    this->mode = mode;
    return this;
}

/* Install a new active data source, then copy its method block
 * (CopyDataSourceSlots) into FileResource's own table and into the table of
 * every registered client. */
void SetActiveDataSource(s32 source) {
    FileResourceMethods *src;
    FileResourceMethods *methods;
    void *(*getMethods)(void);
    void *(**entry)(void);

    entry = gDataSourceClientGetters;
    gActiveDataSource = source;
    if (source == DATASOURCE_CD) {
        src = (FileResourceMethods *)GetCdDriverMethods();
    } else {
        src = (FileResourceMethods *)GetVabDriverMethods();
    }
    methods = GetFileResourceMethods();
    /* MATCHING: a while/for loop compiles top-tested; retail jumps into a bottom test. */
    goto copy;
next:
    entry++;
    methods = getMethods();
copy:
    CopyDataSourceSlots(methods, src);
    getMethods = *entry;
    if (getMethods != NULL) {
        goto next;
    }
}

/* Copy the eleven data-source interface slots of one method table into
 * another: SetActiveDataSource's rebinding step. +0x05C..+0x064 are the
 * base's own and are not copied. */
void CopyDataSourceSlots(FileResourceMethods *dst, FileResourceMethods *src) {
    dst->slot40 = src->slot40;
    dst->open = src->open;
    dst->close = src->close;
    dst->seek = src->seek;
    dst->slot50 = src->slot50;
    dst->read = src->read;
    dst->loadFile = src->loadFile;
    dst->runRequestQueue = src->runRequestQueue;
    dst->requestLoadFile = src->requestLoadFile;
    dst->stopService = src->stopService;
    dst->cancelRequests = src->cancelRequests;
}

extern s32 LockCd(void);

void LockActiveDataSource(void) {
    if (gActiveDataSource == DATASOURCE_CD) {
        LockCd();
    }
}

extern s32 UnlockCd(void);

void UnlockActiveDataSource(void) {
    if (gActiveDataSource == DATASOURCE_CD) {
        UnlockCd();
    }
}

extern s32 IsCdBusy(void);

s32 IsActiveDataSourceBusy(void) {
    if (gActiveDataSource == DATASOURCE_CD) {
        return IsCdBusy();
    }
    return 0;
}

extern s32 IsCdIdle(void);

s32 IsActiveDataSourceIdle(void) {
    if (gActiveDataSource == DATASOURCE_CD) {
        return IsCdIdle();
    }
    return 1;
}

extern s32 GetCdOperation(void);

s32 GetActiveDataSourceOperation(void) {
    if (gActiveDataSource == DATASOURCE_CD) {
        return GetCdOperation();
    }
    return 0;
}

extern s32 GetCdState(void);

s32 GetActiveDataSourceState(void) {
    if (gActiveDataSource == DATASOURCE_CD) {
        return GetCdState();
    }
    return 0;
}

typedef s32 (*DataSourceSetDriverModeFn)(s32, s32, s32);
/* SetVabDriverMode takes two arguments and SetCdDriverMode three. Both are
 * called through the three-argument type, and the VAB driver ignores the
 * third. Assigning SetVabDriverMode to `fn` warns about incompatible pointer
 * types, and that is harmless. */
extern s32 SetVabDriverMode(s32 async, s32 mode2);
extern s32 SetCdDriverMode(s32 async, s32 mode2, s32 useVSyncCallback);

void SetActiveDataSourceDriverMode(s32 async, s32 mode2, s32 useVSyncCallback) {
    DataSourceSetDriverModeFn fn;

    fn = SetVabDriverMode;
    if (gActiveDataSource == DATASOURCE_CD) {
        fn = SetCdDriverMode;
    }
    /* Retry until the driver accepts: SetCdDriverMode refuses while gCdBusy. */
    do {
    } while (fn(async, mode2, useVSyncCallback) == 0);
}

extern s32 GetCdDriverMode(void); /* arity-ok: the definition takes (s32 *outMode2); retail's tail call passes nothing */
extern s32 GetVabDriverMode(void); /* arity-ok: the definition takes (s32 *outMode2); retail's tail call passes nothing */

s32 GetActiveDataSourceDriverMode(void) {
    if (gActiveDataSource == DATASOURCE_CD) {
        return GetCdDriverMode();
    } else {
        return GetVabDriverMode();
    }
}

extern s32 GetCdUseVSyncCallback(void);
extern s32 GetVabUseVSyncCallback(void);

s32 GetActiveDataSourceUseVSyncCallback(void) {
    if (gActiveDataSource == DATASOURCE_CD) {
        return GetCdUseVSyncCallback();
    } else {
        return GetVabUseVSyncCallback();
    }
}

extern s32 gFileTableRegistered;
extern void SetFileTable(CdFileEntry *table);
extern s32 GetFileTableCount(void);
extern void SetFileTableCount(s32 count);
extern s32 ResolveFileEntries(CdFileEntry *entries, s32 count);

s32 RegisterFileTableEntries(CdFileEntry *table, s32 count) {
    s32 first;

    if (gActiveDataSource == DATASOURCE_CD) {
        gFileTableRegistered = 1;
        SetFileTable(table);
        first = GetFileTableCount();
        SetFileTableCount(first + count);
        return ResolveFileEntries(&table[first], count);
    }
    return 1;
}

extern char *gDataDirectory;

void SetDataDirectory(char *dir) {
    gDataDirectory = dir;
}

char *GetDataDirectory(void) {
    return gDataDirectory;
}

char *BuildFileName(char *dest, char *name, char *dir, char *ext) {
    dest[0] = '\0';
    if (dir != NULL) {
        strcat(dest, dir);
    }
    strcat(dest, name);
    strcat(dest, ext);
    return dest;
}
