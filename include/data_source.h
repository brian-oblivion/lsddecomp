#ifndef DATA_SOURCE_H
#define DATA_SOURCE_H

/* The data-source layer of src/app/game_shell.c: the free
 * functions that route FileResource's I/O to the active driver, the CD
 * (DATASOURCE_CD, include/cd_driver.h) or the SPU/VAB one (DATASOURCE_NULL,
 * include/NullDriver.h), and the data directory file names are built in.
 * The classes the file defines are declared by their own headers:
 * GameApplication in include/GameApplication.h, FileResource and
 * ResourceRequest in include/file_resource.h. */

#include "common.h"
#include "file_resource.h"

struct CdFileEntry; /* include/cd_driver.h */

/* The active driver's method table, and switching drivers: SetActiveDataSource
 * copies the new driver's interface slots into FileResource's table and every
 * registered client's (sDataSourceClientGetters). */
extern FileResourceMethods *GetActiveDataSourceMethods(void);
extern void SetActiveDataSource(s32 source);

/* The active driver's lock, state and mode; the CD driver's answer, or the
 * SPU driver's fixed one. SetActiveDataSourceDriverMode retries until the
 * driver accepts the mode. */
extern void LockActiveDataSource(void);
extern void UnlockActiveDataSource(void);
extern s32 IsActiveDataSourceBusy(void);
extern s32 IsActiveDataSourceIdle(void);
extern s32 GetActiveDataSourceOperation(void);
extern s32 GetActiveDataSourceState(void);
extern void SetActiveDataSourceDriverMode(s32 async, s32 mode2, s32 useVSyncCallback);
extern s32 GetActiveDataSourceDriverMode(s32 *outMode2);
extern s32 GetActiveDataSourceUseVSyncCallback(void);

/* Appends `count` entries of `table` to the CD driver's file table and
 * resolves them; returns 0 to be retried, and 1 when the CD driver is not
 * the active data source. */
extern s32 RegisterFileTableEntries(struct CdFileEntry *table, s32 count);

/* The data directory, and dest = dir + name + ext (dir may be NULL). */
extern void SetDataDirectory(char *dir);
extern char *GetDataDirectory(void);
extern char *BuildFileName(char *dest, char *name, char *dir, char *ext);

#endif
