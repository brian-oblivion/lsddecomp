/*
 * The data-source layer (include/data_source.h): which driver FileResource's
 * file I/O goes to, in ROM order: GetActiveDataSourceMethods,
 * ResourceRequest__Set (include/file_resource.h), SetActiveDataSource and
 * CopyDataSourceSlots, the slot copy it rebinds each client table with,
 * then the lock, state and mode forwarders, RegisterFileTableEntries, and
 * the data directory file names are built in. strcat, which BuildFileName
 * calls, is Sony's libc2 object, linked after this file. The getters of
 * every class SetActiveDataSource rebinds end the file.
 */
#include "common.h"
#include <libgte.h>
#include <libgpu.h>
#include "link_resource.h"
#include "data_source.h"
#include "null_driver.h"
#include "game_files.h"
#include <strings.h>
#include "placement_grid.h"
#include "tim_image.h"
#include "tile_atlas.h"
#include "tile_map.h"
#include "tim_array_src.h"
#include "tim_block_src.h"
#include "vab_stream_obj.h"
#include "requested_file.h"
#include "lbd_file.h"
#include "trigger_world.h"

/* The table getters of every class SetActiveDataSource rebinds, NULL-
 * terminated; defined at the end of the file. */
/* A class's table getter, as sDataSourceClientGetters lists them. */
typedef void *(*MethodsGetterFn)(void);

extern MethodsGetterFn sDataSourceClientGetters[];

/* The driver every FileResource method goes through: DATASOURCE_CD until
 * main's GameApplication config says otherwise (SetActiveDataSource). */
static s32 sActiveDataSource SDATA = DATASOURCE_CD;

/* Set once RegisterFileTableEntries has handed the CD driver a table. */
static s32 sFileTableRegistered SDATA = 0;

/* The directory every file name is looked up under (SetDataDirectory).
 * It starts as "": the zero word it points at is the NULL processBuffer
 * slot that ends FileResource's table. */
/* MATCHING: the slot's address, not a "" literal, which would land in .rodata. */
static char *sDataDirectory SDATA = (char *)&gFileResourceMethods.processBuffer;

FileResourceMethods *GetActiveDataSourceMethods(void) {
    if (sActiveDataSource == DATASOURCE_NULL) {
        return (FileResourceMethods *)GetNullDriverMethods();
    } else {
        return (FileResourceMethods *)GetCdDriverMethods();
    }
}

ResourceRequest *ResourceRequest__Set(ResourceRequest *self, void *buffer, char *name, s32 mode) {
    self->src.buffer = buffer;
    self->src.name = name;
    self->mode = mode;
    return self;
}

/* Install a new active data source, then copy its method block
 * (CopyDataSourceSlots) into FileResource's own table and into the table of
 * every registered client. */
void SetActiveDataSource(s32 source) {
    FileResourceMethods *src;
    FileResourceMethods *methods;
    MethodsGetterFn getMethods;
    MethodsGetterFn *entry;

    entry = sDataSourceClientGetters;
    sActiveDataSource = source;
    if (source == DATASOURCE_CD) {
        src = (FileResourceMethods *)GetCdDriverMethods();
    } else {
        src = (FileResourceMethods *)GetNullDriverMethods();
    }
    methods = GetFileResourceMethods();
    /* MATCHING: gotos, because retail's loop is entered at its bottom test;
     * a while or for loop tests at the top. */
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

void LockActiveDataSource(void) {
    if (sActiveDataSource == DATASOURCE_CD) {
        LockCd();
    }
}

void UnlockActiveDataSource(void) {
    if (sActiveDataSource == DATASOURCE_CD) {
        UnlockCd();
    }
}

s32 IsActiveDataSourceBusy(void) {
    if (sActiveDataSource == DATASOURCE_CD) {
        return IsCdBusy();
    }
    return 0;
}

s32 IsActiveDataSourceIdle(void) {
    if (sActiveDataSource == DATASOURCE_CD) {
        return IsCdIdle();
    }
    return 1;
}

s32 GetActiveDataSourceOperation(void) {
    if (sActiveDataSource == DATASOURCE_CD) {
        return GetCdOperation();
    }
    return 0;
}

s32 GetActiveDataSourceState(void) {
    if (sActiveDataSource == DATASOURCE_CD) {
        return GetCdState();
    }
    return 0;
}

typedef s32 (*DataSourceSetDriverModeFn)(s32, s32, s32);

/* SetNullDriverMode takes two arguments and SetCdDriverMode three. Both are
 * called through the three-argument type, and the VAB driver ignores the
 * third. Assigning SetNullDriverMode to `fn` warns about incompatible pointer
 * types, and that is harmless. */

void SetActiveDataSourceDriverMode(s32 async, s32 mode2, s32 useVSyncCallback) {
    DataSourceSetDriverModeFn fn;

    fn = SetNullDriverMode;
    if (sActiveDataSource == DATASOURCE_CD) {
        fn = SetCdDriverMode;
    }
    /* Retry until the driver accepts: SetCdDriverMode refuses while sCdBusy. */
    do {
    } while (fn(async, mode2, useVSyncCallback) == 0);
}

s32 GetActiveDataSourceDriverMode(s32 *outMode2) {
    if (sActiveDataSource == DATASOURCE_CD) {
        return GetCdDriverMode(outMode2);
    } else {
        return GetNullDriverMode(outMode2);
    }
}

s32 GetActiveDataSourceUseVSyncCallback(void) {
    if (sActiveDataSource == DATASOURCE_CD) {
        return GetCdUseVSyncCallback();
    } else {
        return GetNullDriverUseVSyncCallback();
    }
}

s32 RegisterFileTableEntries(CdFileEntry *table, s32 count) {
    s32 first;

    if (sActiveDataSource == DATASOURCE_CD) {
        sFileTableRegistered = 1;
        SetFileTable(table);
        first = GetFileTableCount();
        SetFileTableCount(first + count);
        return ResolveFileEntries(&table[first], count);
    }
    return 1;
}

void SetDataDirectory(char *dir) {
    sDataDirectory = dir;
}

char *GetDataDirectory(void) {
    return sDataDirectory;
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

/* The table getter of every data-source client, NULL-terminated.
 * SetActiveDataSource rebinds each table's file-I/O slots. A (void *)
 * entry is a getter returning its own class's table type. */
MethodsGetterFn sDataSourceClientGetters[] = {
    (void *)GetPlacementGridMethods,
    (void *)GetTimImageMethods,
    (void *)GetTileAtlasMethods,
    (void *)GetTileMapMethods,
    (void *)GetTimArraySrcMethods,
    (void *)GetTimBlockSrcMethods,
    (void *)GetLinkResourceMethods,
    (void *)GetVabStreamObjMethods,
    (void *)GetRequestedFileMethods,
    (void *)GetLbdFileMethods,
    (void *)GetTodMethods,
    (void *)GetTodSetMethods,
    (void *)GetModelDataMethods,
    (void *)GetTriggerWorldMethods,
    NULL,
};
