#ifndef CD_DRIVER_H
#define CD_DRIVER_H

#include "file_resource.h"

/**
 * @file cd_driver.h
 * @brief The CD-ROM data-source driver: the CdDriver class, its request queue,
 * its file table, its read state machines and its blocking file calls.
 *
 * Everything here is defined in src/cd/cd_driver.c, in five parts:
 *  1. the lifecycle: New_CdDriver, the ctor, Finalize and the empty slot
 *     +0x040;
 *  2. the request methods open, close, seek, read, loadFile and
 *     runRequestQueue;
 *  3. the module level: requestLoadFile, stopService and cancelRequests, the
 *     driver mode, the file table, the lock and the service tick;
 *  4. the read state machines, the queue's nodes and the file-table lookups;
 *  5. the blocking file calls the methods fall back on outside the queue
 *     modes.
 *
 * **The driver mode** (SetCdDriverMode) picks how every request method runs:
 *  - async and sync-queue both off: the method calls the blocking
 *    OpenCdFile / CloseCdFile / GetCdFileSize / ReadCdFile (or, for
 *    loadFile, FileResource__LoadFile) and returns;
 *  - otherwise, called from outside the queue (`inQueueDispatch` 0), it
 *    appends a CD_OP_* request for `self` with EnqueueCdRequest;
 *  - called back by runRequestQueue (`inQueueDispatch` 1) with the drive not
 *    busy, it starts the operation (StartCdOperation), then in async mode
 *    hands the work to a state machine through the seek target, sector count
 *    and buffer it sets, and in sync-queue mode does the work on the spot as a
 *    CdControl(CdlSetloc) / CdSync / CdRead / CdReadSync spin.
 *
 * **The service tick.** ServiceCdDriver runs once per VSync, either as the
 * VSyncCallback or as the DrawSystem singleton's callback (SetCdDriverMode's
 * `useVSyncCallback`). It steps the state machine the running operation
 * selected, then runs the queue through runRequestQueue while the queue is
 * enabled. runRequestQueue looks only at the queue's first node: a node not
 * yet started is dispatched to its owner's slot of the same name; a started
 * one, once the drive is idle, is reported to its owner as CD_FLAG_* bits in
 * `flags`, followed by onRequestDone, and freed. The service stops when the
 * queue empties. The tick does nothing while the driver is locked (LockCd /
 * UnlockCd, which every public entry point brackets its body with) or while
 * the BMemPMgr allocator is busy.
 *
 * **The state machines** walk CD_STATE_SETLOC (CdControlF(CdlSetloc) to the
 * seek target), CD_STATE_SETLOC_WAIT (poll CdSync), CD_STATE_READ (CdRead)
 * and CD_STATE_READ_WAIT (poll CdReadSync). A seek that errors or stays
 * unanswered for too many polls, and a read that errors, go back to
 * CD_STATE_SETLOC. TickCdStateMachine, which Open and Seek use (Read starts
 * it at CD_STATE_READ), ends the operation when the seek completes and
 * flushes the drive after a read error; TickCdLoadFileStateMachine,
 * LoadFile's, goes on to read after the seek and, when the read is done,
 * restores the seek target LoadFile saved.
 *
 * **The blocking calls** use only the calling object's `isOpen`, `pos` and
 * `size`. OpenCdFile looks the name up with CdSearchFile under the path
 * BuildCdFilePath makes ("\\<data directory><name>;1") and records where the
 * file starts and how long it is; ReadCdFile seeks to that start and reads
 * whole sectors, starting over from the seek on a disk error. In this mode a
 * seek does not move: CdDriver__Seek returns GetCdFileSize and every read
 * begins at the start of the file.
 */

typedef struct CdDriver CdDriver;
typedef struct CdDriverMethods CdDriverMethods;

/**
 * @brief CdDriver's method table: FileResource's 29 slots, eleven of them
 * (+0x040..+0x058 and +0x068..+0x074) the driver interface.
 *
 * The slot names and types are FileResource's, so open, close, loadFile,
 * onRequestDone and stopService return void and read's buffer is `void *`.
 * The table ends after +0x074; the word where FileResource's processBuffer
 * would be is the next table's header.
 */
struct CdDriverMethods {
    FILERESOURCE_SLOTS(CdDriver, (CdDriver * self));
};

/**
 * @brief The CD-ROM data-source driver (class id 0x13, DATASOURCE_CD), a
 * FileResource subclass and NullDriver's sibling. No class derives from it.
 *
 * The driver runs on other classes' objects. Nothing calls New_CdDriver:
 * SetActiveDataSource (src/app/game_shell.c) copies this table's eleven
 * interface slots into FileResource's table and every client table, so
 * `self` in Open, Read and the rest is whichever FileResource object called
 * its own `open` or `read` (a TimImage, a TodSet, ...), and a queued
 * request's `owner` is that object. Every field the methods use is
 * FileResource's, and the object has no fields of its own: New_CdDriver
 * allocates 0x2C bytes, FileResource's size. Methods and module functions
 * are in src/cd/cd_driver.c.
 */
struct CdDriver {
    FILERESOURCE_FIELDS(CdDriverMethods);
};

/** @brief CdDriver's method table (see CdDriverMethods). */
extern CdDriverMethods gCdDriverMethods;

/**
 * @brief Returns CdDriver's method table.
 * @return &gCdDriverMethods.
 */
extern CdDriverMethods *GetCdDriverMethods(void);

/**
 * @brief Allocates a CdDriver from the BMemPMgr pool and constructs it.
 * Nothing in the game calls it (see CdDriver).
 * @return The new driver, or NULL when the pool is exhausted.
 */
CdDriver *New_CdDriver(void);

/**
 * @brief Constructor (slot +0x008): FileResource's ctor, then this class's
 * table, `inQueueDispatch` cleared and InitCdDrive.
 * @param self The object being constructed.
 */
void CdDriver__CdDriver(CdDriver *self);

/**
 * @brief Finalizer (slot +0x00C): cancels the object's queued requests,
 * then frees its buffer.
 * @param self The object being finalized.
 */
void CdDriver__Finalize(CdDriver *self);

/** @brief Slot +0x040: empty. */
void CdDriver__NoOpSlot40(void);

/**
 * @brief Slot +0x044, open: looks `name` up and records its disc position
 * and size in `self`.
 *
 * Outside the queue modes it calls OpenCdFile. Otherwise, called from
 * outside the queue it enqueues a CD_OP_OPEN request; dispatched from the
 * queue with the drive free and the object closed, it starts
 * CD_OPERATION_OPEN. In async mode it takes the position and size from the
 * file table (FindCdFileEntry) and leaves the seek to the state machine; an
 * unknown name returns with the driver still locked. In sync-queue mode it
 * searches the disc until the file is found and seeks to it on the spot.
 * @param self   The FileResource object opening the file.
 * @param name   The file's name, matched as a substring of a file table entry.
 * @param param0 Stored in the queued request and passed back on dispatch.
 * @param param1 Stored in the queued request and passed back on dispatch.
 */
void CdDriver__Open(CdDriver *self, char *name, s32 param0, s32 param1);

/**
 * @brief Slot +0x048, close: marks `self` closed, through CloseCdFile or a
 * queued CD_OP_CLOSE request.
 * @param self The FileResource object closing its file.
 */
void CdDriver__Close(CdDriver *self);

/**
 * @brief Slot +0x04C, seek: moves the drive to `offset` bytes into the open
 * file, rounded up to a whole sector, or reports the file's size.
 *
 * Outside the queue modes it returns GetCdFileSize without moving. From
 * outside the queue it enqueues a CD_OP_SEEK request and returns 0.
 * Dispatched with the drive free and the file open: with `mode` 0 it seeks,
 * through the state machine (async) or on the spot (sync-queue); with any
 * other `mode` it ends the operation at once and returns the file's size
 * rounded up to a whole sector.
 * @param self   The FileResource object whose file is open.
 * @param offset Byte offset from the start of the file.
 * @param mode   0 to seek; anything else to ask for the size.
 * @return The rounded size for a size query, GetCdFileSize's result outside
 *         the queue modes, and 0 otherwise.
 */
s32 CdDriver__Seek(CdDriver *self, u32 offset, s32 mode);

/** @brief Slot +0x050: empty. */
void CdDriver__NoOpSlot50(void);

/**
 * @brief Slot +0x054, read: reads `size` bytes, in whole sectors, from the
 * drive's current position into `buf`.
 *
 * Outside the queue modes it calls ReadCdFile. From outside the queue it
 * enqueues a CD_OP_READ request. Dispatched with the drive free and the file
 * open, it hands the read to the state machine (async) or reads on the spot,
 * retrying on an error (sync-queue).
 * @param self The FileResource object whose file is open.
 * @param buf  Destination buffer.
 * @param size Byte count; the sectors read are `size / CD_SECTOR_SIZE`.
 * @return 0.
 */
s32 CdDriver__Read(CdDriver *self, void *buf, u32 size);

/**
 * @brief Slot +0x058, loadFile: reads the whole file `name` into `self`'s
 * buffer, allocating one of the file's size rounded up to whole sectors when
 * the object has none.
 *
 * Outside the queue modes it runs FileResource__LoadFile, sets
 * CD_FLAG_LOAD_FILE_DONE and calls onRequestDone. From outside the queue it
 * enqueues a CD_OP_LOAD_FILE request. Dispatched with the drive free and the
 * buffer free to reuse, it saves the current seek target, looks the file up
 * in the file table, and reads through TickCdLoadFileStateMachine (async) or
 * on the spot (sync-queue). A failed allocation closes the object.
 * @param self The FileResource object loading the file.
 * @param name The file's name, matched as a substring of a file table entry.
 */
void CdDriver__LoadFile(CdDriver *self, char *name);

/**
 * @brief Slot +0x068, runRequestQueue: services the queue's first request.
 *
 * A request not yet started is dispatched to its owner's open, close, seek,
 * read or loadFile slot with `inQueueDispatch` set. A started one, once the
 * drive is idle, is finished: the owner's `pendingRequests` drops, its
 * `flags` gain CD_FLAG_DONE, CD_FLAG_NONE_PENDING when none are left, and
 * the CD_FLAG_*_DONE bit of the request's op; the owner's onRequestDone is
 * called, the node freed, and the service stopped when the queue is empty.
 */
void CdDriver__RunRequestQueue(void);

/**
 * @brief Slot +0x06C, requestLoadFile: queues a loadFile of `name` in async
 * mode; otherwise loads it at once and sets CD_FLAG_NONE_PENDING when the
 * object has no request pending. A NULL name does nothing.
 * @param self The FileResource object loading the file.
 * @param name The file's name, or NULL.
 */
void CdDriver__RequestLoadFile(CdDriver *self, char *name);

/** @brief Slot +0x070, stopService: StopCdServiceIfIdle under the lock. */
void CdDriver__StopService(void);

/**
 * @brief Slot +0x074, cancelRequests: frees every queued request `self`
 * owns and clears its `flags`, when it has any pending.
 *
 * When `self`'s request is the one running and the drive is not idle, it
 * first flushes the drive, ends the operation and restores the seek target
 * LoadFile saved.
 * @param self The FileResource object whose requests are cancelled.
 */
void CdDriver__CancelRequests(CdDriver *self);

/**
 * @brief One entry of the driver's file table: a name resolved once to a
 * disc position and size, then reused as a seek target.
 *
 * FindCdFileEntry, FindCdFileIndex and GetCdFileEntry walk the table; the
 * state machines seek to the `pos` of the entry the running operation
 * targets. The game's table is src/cd/game_files.c's record table
 * (game_files.h), whose getters hand out an entry's name as a file path.
 */
typedef struct CdFileEntry {
    /* +0x00 */ char name[0x14]; /**< The file's path, zero-padded. */
    /* +0x14 */ CdlLOC pos;      /**< Disc position (ResolveFileEntries). */
    /* +0x18 */ u32 size;        /**< Size in bytes (ResolveFileEntries). */
} CdFileEntry;                   /* size 0x1C */

/**
 * @brief One queued request, a node of the driver's doubly linked request
 * queue.
 *
 * AllocCdRequestNode allocates it and links it at the tail, EnqueueCdRequest
 * fills it, StartCdOperation marks the first node active, runRequestQueue
 * dispatches `op` back to `owner`'s slot of the same name, and
 * FreeCdRequestNode unlinks and frees it.
 */
typedef struct CdRequestNode {
    /* +0x00 */ s32 active; /**< Set by StartCdOperation when the request starts. */
    /* +0x04 */ s32 unk4; /**< Zeroed at allocation; nonzero sets bit 0 of the owner's flags on completion. */
    /* +0x08 */ s32 op;          /**< CD_OP_*. */
    /* +0x0C */ CdDriver *owner; /**< The requesting object (any FileResource client). */
    /* +0x10 */ s32 fileIndex; /**< File table index (FindCdFileIndex) for open and loadFile, 0 otherwise. */
    /* +0x14 */ s32 param0; /**< First argument passed back on dispatch (seek's offset, read's buffer). */
    /* +0x18 */ s32 param1; /**< Second argument passed back on dispatch (seek's mode, read's size). */
    /* +0x1C */ struct CdRequestNode *prev; /**< Previous node, NULL for the first. */
    /* +0x20 */ struct CdRequestNode *next; /**< Next node, NULL for the last. */
} CdRequestNode;                            /* size 0x24 */

/* A queue node's `op`: EnqueueCdRequest's third argument, dispatched back by
 * CdDriver__RunRequestQueue to the owner's slot of the same name. */
#define CD_OP_OPEN 2
#define CD_OP_CLOSE 3
#define CD_OP_SEEK 4
#define CD_OP_READ 5
#define CD_OP_LOAD_FILE 7

/* Which of the two state machines ServiceCdDriver ticks. */
#define CD_TICK_NONE 0          /* neither: ResetCdStateMachine's value */
#define CD_TICK_STATE_MACHINE 1 /* TickCdStateMachine */
#define CD_TICK_LOAD_FILE 2     /* TickCdLoadFileStateMachine */

/* Which method's request the state machine is running: StartCdOperation's
 * first argument and GetCdOperation's (and so
 * GetActiveDataSourceOperation's) result. Each value is passed by exactly one
 * method. 0 is also what ResetCdStateMachine leaves when nothing runs: Close
 * resets straight after starting. */
#define CD_OPERATION_CLOSE 0
#define CD_OPERATION_OPEN 1
#define CD_OPERATION_SEEK 2
#define CD_OPERATION_READ 3
#define CD_OPERATION_LOAD_FILE 4

/* The state machines' phase: StartCdOperation's second argument and
 * GetCdState's result. */
#define CD_STATE_IDLE 0        /* ResetCdStateMachine's value */
#define CD_STATE_SETLOC 1      /* issue CdControl(CdlSetloc) */
#define CD_STATE_SETLOC_WAIT 2 /* poll CdSync for it */
#define CD_STATE_READ 7        /* issue CdRead */
#define CD_STATE_READ_WAIT 8   /* poll CdReadSync */

/* A CD-ROM data sector's user data (2048 bytes; <libcd.h>'s CdlModeSize0/1
 * clear). CdRead counts sectors, so byte sizes and offsets are shifted by
 * CD_SECTOR_SHIFT (ReadCdFile, CdDriver__Seek), and GetCdFileSize reports a
 * file's size in whole sectors. */
#define CD_SECTOR_SIZE 2048
#define CD_SECTOR_SHIFT 11

/* A path buffer for BuildCdFilePath's "\\<data directory><name>;1". */
#define CD_PATH_SIZE 64

/* The file lookups call CdSearchFile this many times before they print
 * "File not found" and give up (OpenCdFile, ResolveFileEntries). */
#define CD_SEARCH_ATTEMPTS 101

/* ---- The drive and its state, as game_shell.c's data-source wrappers read it. */

/**
 * @brief Puts the drive in double-speed mode, once per boot; later calls
 * return at once.
 */
extern void InitCdDrive(void);

/**
 * @brief Whether an operation is running (set by StartCdOperation, cleared by
 * ResetCdStateMachine).
 * @return 1 while an operation runs, else 0.
 */
extern s32 IsCdBusy(void);

/**
 * @brief Whether the drive is idle (cleared by StartCdOperation, set by
 * ResetCdStateMachine).
 * @return 1 when idle, else 0.
 */
extern s32 IsCdIdle(void);

/**
 * @brief The running operation.
 * @return A CD_OPERATION_* value.
 */
extern s32 GetCdOperation(void);

/**
 * @brief The state machine's phase.
 * @return A CD_STATE_* value.
 */
extern s32 GetCdState(void);

/**
 * @brief Reads the driver mode back.
 * @param outSyncQueueMode Receives the sync-queue flag; may be NULL.
 * @return The async flag.
 */
extern s32 GetCdDriverMode(s32 *outSyncQueueMode);

/**
 * @brief Sets the driver mode, unless an operation is running.
 *
 * With `useVSyncCallback` 0, turning async on installs ServiceCdDriver as
 * the DrawSystem singleton's callback and turning it off removes it; with
 * `useVSyncCallback` set, StartCdService hooks it to VSyncCallback instead.
 * @param async            Nonzero: requests queue and run in the background.
 * @param syncQueueMode    Nonzero with `async` 0: requests queue, then each
 *                         runs as a blocking spin when dispatched.
 * @param useVSyncCallback Nonzero: drive the service tick from VSyncCallback.
 * @return 1 when the mode was set, 0 when an operation was running.
 */
extern s32 SetCdDriverMode(s32 async, s32 syncQueueMode, s32 useVSyncCallback);

/* ---- The file table. */

/**
 * @brief Installs the file table the lookups search.
 * @param table The table's first entry.
 */
extern void SetFileTable(CdFileEntry *table);

/**
 * @brief Sets the file table's entry count.
 * @param count Number of entries.
 */
extern void SetFileTableCount(s32 count);

/**
 * @brief The file table's entry count.
 * @return The count SetFileTableCount set.
 */
extern s32 GetFileTableCount(void);

/**
 * @brief Fills each entry's disc position and size from the disc
 * (CdSearchFile, CD_SEARCH_ATTEMPTS tries per name, printing "File not
 * found" after the last). Initializes the drive first.
 * @param entries The first entry to resolve.
 * @param count   Number of entries.
 * @return 1.
 */
extern s32 ResolveFileEntries(CdFileEntry *entries, s32 count);

/* ---- The lock, the service and the request queue. */

/**
 * @brief Sets the driver lock, which makes ServiceCdDriver skip its tick.
 * Not a mutex: nothing waits on it.
 */
extern void LockCd(void);

/** @brief Clears the driver lock. */
extern void UnlockCd(void);

/**
 * @brief The service tick, run once per VSync: steps the running state
 * machine and then the request queue. Does nothing while the driver is
 * locked or the BMemPMgr allocator is busy.
 * @return 0.
 */
extern s32 ServiceCdDriver(void);

/**
 * @brief Enables the request queue and marks the service installed, hooking
 * ServiceCdDriver to VSyncCallback the first time when that mode is set.
 */
extern void StartCdService(void);

/**
 * @brief Once no state machine is running, unhooks the service tick and
 * disables the queue.
 */
extern void StopCdServiceIfIdle(void);

/** @brief Disables the request queue, leaving the service tick installed. */
extern void DisableCdQueue(void);

/**
 * @brief Queues a request: fills a node AllocCdRequestNode has linked at the
 * tail, counts it in the owner's `pendingRequests`, clears the owner's
 * `flags` and starts the service.
 * @param owner     The requesting object.
 * @param fileIndex File table index for open and loadFile, else 0.
 * @param op        A CD_OP_* value.
 * @param param0    First argument passed back on dispatch.
 * @param param1    Second argument passed back on dispatch.
 */
extern void EnqueueCdRequest(CdDriver *owner, s32 fileIndex, s32 op, s32 param0, s32 param1);

/**
 * @brief Allocates a zeroed request node and links it at the queue's tail.
 * @return The node, or NULL when the pool is exhausted.
 */
extern CdRequestNode *AllocCdRequestNode(void);

/**
 * @brief Unlinks a request node from the queue and frees it. NULL is ignored.
 * @param node The node to free.
 */
extern void FreeCdRequestNode(CdRequestNode *node);

/* ---- The file-table lookups. */

/**
 * @brief Finds the first file table entry whose name contains `name`.
 *
 * The not-found return leaves the driver locked, so the service tick stays
 * off until the next UnlockCd anywhere.
 * @param name The substring to look for.
 * @return The CdFileEntry, or NULL.
 */
extern void *FindCdFileEntry(char *name);

/**
 * @brief Finds the index of the first file table entry whose name contains
 * `name`. Like FindCdFileEntry, the not-found return leaves the driver locked.
 * @param name The substring to look for.
 * @return The entry's index, or -1.
 */
extern s32 FindCdFileIndex(char *name);

/**
 * @brief The file table entry at `index`, which, its name being its first
 * member, is also that name.
 * @param index Entry index.
 * @return The CdFileEntry.
 */
extern void *GetCdFileEntry(s32 index);

/* ---- The state machines. */

/**
 * @brief One step of the open, seek and read state machine
 * (CD_TICK_STATE_MACHINE): ends the operation when a seek completes or a
 * read is done; restarts from the seek on a seek timeout or error, and on a
 * read error after flushing the drive.
 */
extern void TickCdStateMachine(void);

/**
 * @brief One step of loadFile's state machine (CD_TICK_LOAD_FILE): seeks,
 * then reads; restarts from the seek on a timeout or error; when the read is
 * done ends the operation and restores the seek target LoadFile saved.
 */
extern void TickCdLoadFileStateMachine(void);

/**
 * @brief Starts an operation on the queue's first request: marks the
 * driver busy and not idle, and the request active.
 * @param op    A CD_OPERATION_* value.
 * @param state The first CD_STATE_*.
 */
extern void StartCdOperation(s32 op, s32 state);

/**
 * @brief Ends the operation: no operation, CD_STATE_IDLE, no tick step, the
 * timeout counter cleared, the driver idle and not busy.
 */
extern void ResetCdStateMachine(void);

/**
 * @brief Moves the state machine to `state` and clears the timeout counter.
 * @param state A CD_STATE_* value.
 */
extern void SetCdState(s32 state);

/* ---- The blocking file access the request methods fall back on. */

/**
 * @brief Turns a FileResource into a CD driver object: FileResource's ctor,
 * then gCdDriverMethods, closed. Nothing in the game calls it.
 * @param self The object to set up.
 */
extern void FileResource__InstallCdReadDriver(FileResource *self);

/**
 * @brief FileResource's finalize, the counterpart of
 * FileResource__InstallCdReadDriver. Nothing in the game calls it.
 * @param self The object to finalize.
 */
extern void FileResource__DestroyCdReadDriver(FileResource *self);

/**
 * @brief Resolves `name` on the disc and marks the object open; an open
 * object is left alone. After CD_SEARCH_ATTEMPTS failed lookups it prints
 * the path and leaves the object closed.
 * @param self The FileResource object opening the file.
 * @param name The file's name, under the data directory.
 */
extern void OpenCdFile(CdDriver *self, char *name);

/**
 * @brief Builds the ISO 9660 path "\\<data directory><name>;1".
 * @param dest Destination buffer, CD_PATH_SIZE bytes.
 * @param name The file's name.
 * @return `dest`.
 */
extern char *BuildCdFilePath(char *dest, char *name);

/**
 * @brief Marks the object closed.
 * @param self The FileResource object.
 */
extern void CloseCdFile(CdDriver *self);

/**
 * @brief The open file's size in whole sectors, one sector over when it is
 * already a multiple (CdDriver__Seek's queued path rounds up only when it is
 * not).
 * @param self The FileResource object.
 * @return The size in bytes, or 0 when the object is closed.
 */
extern s32 GetCdFileSize(CdDriver *self);

/**
 * @brief Reads `size` bytes, rounded down to whole sectors, from the start
 * of the open file, retrying from the seek on a disk error. A closed object
 * is sent to its own close slot instead.
 * @param self The FileResource object whose file is open.
 * @param buf  Destination buffer.
 * @param size Byte count.
 * @return 0.
 */
extern s32 ReadCdFile(CdDriver *self, void *buf, s32 size);

/**
 * @brief Whether the service tick is driven from VSyncCallback, the CD half
 * of game_shell.c's GetActiveDataSourceUseVSyncCallback.
 * @return SetCdDriverMode's `useVSyncCallback`.
 */
extern s32 GetCdUseVSyncCallback(void);

#endif /* CD_DRIVER_H */
