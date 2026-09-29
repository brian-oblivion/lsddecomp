#ifndef NULL_DRIVER_H
#define NULL_DRIVER_H

#include "file_resource.h"

/**
 * @file null_driver.h
 * @brief NullDriver, the data-source driver whose every method is empty, and
 * its mode accessors.
 */

typedef struct NullDriver NullDriver;
typedef struct NullDriverMethods NullDriverMethods;

/** NullDriver's class id (gNullDriverMethods word +0x000). */
#define NULLDRIVER_CLASS_ID 0x23

/**
 * @brief NullDriver's method table: FileResource's slots up to +0x074
 * (FILERESOURCE_BASE_SLOTS), the eleven
 * driver-interface slots (+0x040..+0x058, +0x068..+0x074) overridden with
 * empty bodies.
 *
 * The ctor's parameter list is FileResource's: every chained call reaches it
 * as GetActiveDataSourceMethods()->ctor(self). The table has no
 * processBuffer slot: the word after +0x074 is gVabStreamObjMethods's header.
 */
struct NullDriverMethods {
    FILERESOURCE_BASE_SLOTS(NullDriver, (NullDriver * self));
};

/**
 * @brief The data-source driver whose every method is empty (class id 0x23,
 * DATASOURCE_NULL), a FileResource subclass and CdDriver's sibling. No class
 * derives from it.
 *
 * SetActiveDataSource (src/app/game_shell.c) binds this table's
 * driver-interface slots into FileResource's table and every client table
 * whenever the active source is not DATASOURCE_CD, and
 * GetActiveDataSourceMethods returns it then, so with that source active the
 * file-I/O interface does nothing. VAB sound streaming is VabStreamObj's, a
 * separate FileResource subclass. Nothing allocates a NullDriver and no
 * method reads `self`, so the object is FileResource's fields and its size
 * is unknown. Methods in src/sound/vab_sound.c.
 */
struct NullDriver {
    FILERESOURCE_FIELDS(NullDriverMethods);
};

/** @brief NullDriver's method table (see NullDriverMethods). */
extern NullDriverMethods gNullDriverMethods;

/**
 * @brief Returns NullDriver's method table.
 * @return &gNullDriverMethods.
 */
extern NullDriverMethods *GetNullDriverMethods(void);

/* Every method is empty and reads none of its slot's parameters, so the
 * prototypes declare none. */

/** @brief Constructor (slot +0x008): empty. */
void NullDriver__NullDriver(void);
/** @brief Finalizer (slot +0x00C): empty. */
void NullDriver__Destroy(void);
/** @brief Slot +0x040: empty. */
void NullDriver__NoOpSlot40(void);
/** @brief Slot +0x044, open: does nothing. */
void NullDriver__Open(void);
/** @brief Slot +0x048, close: does nothing. */
void NullDriver__Close(void);
/** @brief Slot +0x04C, seek: does nothing. */
void NullDriver__Seek(void);
/** @brief Slot +0x050: empty. */
void NullDriver__NoOpSlot50(void);
/**
 * @brief Slot +0x054, read: reads nothing.
 * @return 0.
 */
s32 NullDriver__Read(void);
/** @brief Slot +0x058, loadFile: does nothing. */
void NullDriver__LoadFile(void);
/** @brief Slot +0x068, runRequestQueue: does nothing. */
void NullDriver__RunRequestQueue(void);
/** @brief Slot +0x06C, requestLoadFile: does nothing. */
void NullDriver__RequestLoadFile(void);
/** @brief Slot +0x070, stopService: does nothing. */
void NullDriver__StopService(void);
/** @brief Slot +0x074, cancelRequests: does nothing. */
void NullDriver__CancelRequests(void);

/* The driver's mode, as game_shell.c's data-source wrappers read and set it
 * (CdDriver's counterparts take a third argument). */

/**
 * @brief Reads back the two words SetNullDriverMode stored.
 * @param outMode2 Receives the second word; may be NULL.
 * @return The first word (`async`).
 */
extern s32 GetNullDriverMode(s32 *outMode2);

/**
 * @brief Stores the two mode words; nothing else reads them.
 * @param async The first word, GetNullDriverMode's result.
 * @param mode2 The second word.
 * @return 1.
 */
extern s32 SetNullDriverMode(s32 async, s32 mode2);

/**
 * @brief Whether the service tick is driven from VSyncCallback: never, for
 * this driver.
 * @return 0.
 */
extern s32 GetNullDriverUseVSyncCallback(void);

#endif
