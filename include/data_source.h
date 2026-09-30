#ifndef DATA_SOURCE_H
#define DATA_SOURCE_H

/**
 * @file data_source.h
 * @brief The data-source layer: which driver FileResource's I/O goes to, the
 * driver's lock, state and mode, file-table registration, and the data
 * directory file names are built in.
 *
 * sActiveDataSource selects one of FileResource's two driver subclasses:
 * the CD-ROM driver (DATASOURCE_CD, 0x13, cd_driver.h) or the null driver
 * (DATASOURCE_NULL, 0x23, null_driver.h), whose every method is empty.
 * SetActiveDataSource installs one and copies its interface slots into
 * FileResource's table and into every client class's table. The
 * lock/unlock, busy/idle and get/set functions after it forward to the CD
 * driver when it is active, and otherwise do nothing, return a fixed value
 * or call the null driver. RegisterFileTableEntries appends CdFileEntry
 * records to the CD driver's file table and resolves them.
 *
 * SetDataDirectory/GetDataDirectory hold the directory BuildCdFilePath and
 * CdStream__Open put between the root `\` and a file name: "" until
 * GameApplication's ctor installs GetDefaultDataDirectory()'s. BuildFileName joins an
 * optional directory, a name and an extension.
 *
 * All of it is defined in src/app/data_source.c, which follows
 * FileResource's methods; ResourceRequest__Set and CopyDataSourceSlots,
 * among them, are declared in file_resource.h.
 */

#include "common.h"
#include "file_resource.h"

struct CdFileEntry; /* cd_driver.h */

/** @name Data sources
 * SetActiveDataSource's `source` (GameApplicationConfig::dataSource): the
 * class id, the header word of its method table, of the driver it selects. @{ */
#define DATASOURCE_CD 0x13   /**< CdDriver, gCdDriverMethods: the CD-ROM */
#define DATASOURCE_NULL 0x23 /**< NullDriver, gNullDriverMethods: every method empty */
/** @} */

/** @brief The active driver's method table.
 * @return the null driver's table when it is active, else the CD driver's */
extern FileResourceMethods *GetActiveDataSourceMethods(void);

/** @brief Makes `source` the active driver and copies its interface slots
 * (CopyDataSourceSlots) into FileResource's table and every registered
 * client class's.
 * @param source DATASOURCE_CD selects the CD driver; any other value the
 *        null driver */
extern void SetActiveDataSource(s32 source);

/** @brief Locks the CD driver (LockCd) when it is the active source. */
extern void LockActiveDataSource(void);

/** @brief Unlocks the CD driver (UnlockCd) when it is the active source. */
extern void UnlockActiveDataSource(void);

/** @brief Whether the active driver is busy.
 * @return IsCdBusy() for the CD driver, else 0 */
extern s32 IsActiveDataSourceBusy(void);

/** @brief Whether the active driver is idle.
 * @return IsCdIdle() for the CD driver, else 1 */
extern s32 IsActiveDataSourceIdle(void);

/** @brief The active driver's current operation.
 * @return GetCdOperation() for the CD driver, else 0 */
extern s32 GetActiveDataSourceOperation(void);

/** @brief The active driver's state.
 * @return GetCdState() for the CD driver, else 0 */
extern s32 GetActiveDataSourceState(void);

/** @brief Sets the active driver's mode, retrying until the driver accepts it
 * (SetCdDriverMode refuses while the CD is busy; the null driver ignores
 * the third argument).
 * @param async nonzero: requests are queued and run in the background
 * @param mode2 the CD driver's sync-queue mode: nonzero queues requests but
 *        runs each as a blocking spin
 * @param useVSyncCallback nonzero: the CD driver is serviced from
 *        VSyncCallback; 0: from the DrawSystem's callback */
extern void SetActiveDataSourceDriverMode(s32 async, s32 mode2, s32 useVSyncCallback);

/** @brief Reads the active driver's mode.
 * @param outMode2 receives the sync-queue mode (see
 *        SetActiveDataSourceDriverMode)
 * @return the async flag */
extern s32 GetActiveDataSourceDriverMode(s32 *outMode2);

/** @brief Whether the active driver steps from the VSync callback.
 * @return the useVSyncCallback setting */
extern s32 GetActiveDataSourceUseVSyncCallback(void);

/** @brief Appends `count` entries of `table` to the CD driver's file table
 * and resolves them.
 * @param table the entries; the driver keeps the pointer
 * @param count how many to append
 * @return ResolveFileEntries's result (0: retry), or 1 when the CD driver is
 *         not the active data source */
extern s32 RegisterFileTableEntries(struct CdFileEntry *table, s32 count);

/** @brief Sets the directory CD paths are built in.
 * @param dir the directory, kept by pointer */
extern void SetDataDirectory(char *dir);

/** @brief The directory CD paths are built in.
 * @return the SetDataDirectory pointer */
extern char *GetDataDirectory(void);

/** @brief Builds dest = dir + name + ext.
 * @param dest the output buffer
 * @param name the file name
 * @param dir the directory to put first, or NULL
 * @param ext the extension to append
 * @return dest */
extern char *BuildFileName(char *dest, char *name, char *dir, char *ext);

/** The path buffer TextEntry's and ItemList's resource loaders give
 * BuildFileName as `dest` for a "CARD\\" + name + ".TIM" path. */
#define CARD_TIM_PATH_SIZE 32

#endif
