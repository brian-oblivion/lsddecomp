#ifndef FILE_RESOURCE_H
#define FILE_RESOURCE_H

#include "common.h"
#include <libcd.h>
#include "basic_class.h"

/**
 * @file file_resource.h
 * @brief FileResource, the base of every class loaded from a file and of the
 * data-source drivers, and the descriptors the file-backed ctors take.
 *
 * Declares the class (FILERESOURCE_SLOTS and the shorter
 * FILERESOURCE_BASE_SLOTS, FILERESOURCE_FIELDS for its
 * subclasses), the ResourceSource and ResourceRequest descriptors, the flag
 * bits the CD driver reports request completion with, and FileResource's own
 * methods, defined in src/app/file_resource.c. The functions that bind the
 * active driver into these tables are data_source.h's (src/app/data_source.c).
 */

typedef struct FileResource FileResource;
typedef struct FileResourceMethods FileResourceMethods;

/** FileResource's class id (gFileResourceMethods word +0x000). */
#define FILERESOURCE_CLASS_ID 0x3

/** FileResource's slots up to +0x074, BasicClass's first: the whole table
 * of a class that has no processBuffer step (RequestedFile and the two
 * drivers, CdDriver and NullDriver), whose table ends there.
 * +0x040..+0x058 and +0x068..+0x074 are the data-source interface: NULL or
 * placeholders in the static tables, filled at run time from the active
 * driver's table (SetActiveDataSource), and named for the CD driver's
 * occupants. */
/* clang-format off */
#define FILERESOURCE_BASE_SLOTS(Self, CtorParams)                                                    \
    BASICCLASS_SLOTS(Self, CtorParams);                                                            \
    /* +0x040 */ void (*slot40)(void);                          /**< CD: CdDriver__NoOpSlot40 */   \
    /* +0x044 */ void (*open)(Self *self, char *name, s32 arg2, s32 arg3); /**< CD: CdDriver__Open */ \
    /* +0x048 */ void (*close)(Self *self);                     /**< CD: CdDriver__Close */        \
    /* +0x04C */ s32 (*seek)(Self *self, u32 offset, s32 mode); /**< CD: CdDriver__Seek; mode SEEK_SET moves to offset; LoadFile's seek(0, SEEK_END) returns the size (<stdio.h>) */ \
    /* +0x050 */ void (*slot50)(void);                          /**< CD: CdDriver__NoOpSlot50 */   \
    /* +0x054 */ s32 (*read)(Self *self, void *buf, u32 size);  /**< CD: CdDriver__Read */         \
    /* +0x058 */ void (*loadFile)(Self *self, char *name);      /**< @see FileResource__LoadFile; CD: CdDriver__LoadFile */ \
    /* +0x05C */ void (*freeBuffer)(Self *self);                /**< @see FileResource__FreeBuffer */       \
    /* +0x060 */ void (*slot60)(void);                          /**< @see NoOp, in every FileResource table */ \
    /* +0x064 */ void (*onRequestDone)(Self *self);             /**< @see FileResource__OnRequestDone */          \
    /* +0x068 */ void (*runRequestQueue)(void);                 /**< CD: CdDriver__RunRequestQueue */ \
    /* +0x06C */ void (*requestLoadFile)(Self *self, char *name); /**< CD: CdDriver__RequestLoadFile */ \
    /* +0x070 */ void (*stopService)(Self *self);               /**< CD: CdDriver__StopService; neither occupant reads self, but CdDriver__RunRequestQueue passes it */ \
    /* +0x074 */ void (*cancelRequests)(Self *self)             /**< CD: CdDriver__CancelRequests */
/* clang-format on */

/** FileResource's slots: FILERESOURCE_BASE_SLOTS, then processBuffer. */
/* clang-format off */
#define FILERESOURCE_SLOTS(Self, CtorParams)                                                         \
    FILERESOURCE_BASE_SLOTS(Self, CtorParams);                                                     \
    /* +0x078 */ void *processBuffer                            /**< NULL here; each subclass's step that consumes the loaded buffer (TimImage__Upload, ModelData__BuildResources, ...), signature per class */
/* clang-format on */

/** FileResource's fields, BasicClass's first. `pos` is a disc position,
 * Sony's CdlLOC: the CD driver's methods run on every client object, so the
 * open file's position and size are fields of the base. */
/* clang-format off */
#define FILERESOURCE_FIELDS(Methods)                                                                 \
    BASICCLASS_FIELDS(Methods);                                                                    \
    /* +0x00C */ s32 isOpen;          /**< cleared while LoadFile runs, then restored */             \
    /* +0x010 */ void *buffer;        /**< LoadFile's allocation, NULL when none */                  \
    /* +0x014 */ s32 bufferSize;      /**< the buffer's bytes; 0: FreeBuffer keeps it */            \
    /* +0x018 */ CdlLOC pos;          /**< the open file's disc position: the CD driver's Open sets it */ \
    /* +0x01C */ u32 size;            /**< its byte size; Seek and ReadCdFile seek from pos (cd_driver.h) */ \
    /* +0x020 */ u16 freeGuard;       /**< nonzero: FreeBuffer keeps the buffer */                   \
    /* +0x022 */ u16 pendingRequests; /**< CD requests queued for this object and not yet completed */ \
    /* +0x024 */ s32 flags;           /**< bit 0 set by OnRequestDone; the CD_FLAG_* bits */         \
    /* +0x028 */ u16 inQueueDispatch; /**< 1 while the CD driver's runRequestQueue dispatches a queued request to this object */ \
    /* +0x02A */ u16 loadState        /**< a subclass's load step, 0 when idle (LbdFile, VabStreamObj); the object is 0x2C bytes */
/* clang-format on */

/** FileResource's method table: BasicClass's slots and FILERESOURCE_SLOTS'
 * own. */
struct FileResourceMethods {
    FILERESOURCE_SLOTS(FileResource, (FileResource * self));
};

/**
 * FileResource -- the base of every class the game loads from a file: class
 * id 0x3, method table gFileResourceMethods, parent BasicClass. Its own
 * methods are in src/app/file_resource.c.
 *
 * The object owns one file buffer. FileResource__LoadFile opens a named
 * file, takes its size from seek(0, SEEK_END), allocates that much from the
 * BMemPMgr pool, reads the whole file into it and closes it; FreeBuffer
 * releases the buffer unless freeGuard is set. The subclasses are the game's
 * file-backed assets (TimImage, TileMap, TileAtlas, ModelData, Tod,
 * VabStreamObj, ...). A subclass constructor names its file (loadFile, or
 * requestLoadFile to queue it on the CD driver), and slot +0x078 is the
 * subclass's own step that consumes the loaded buffer: TimImage__Upload
 * sends it to VRAM, ModelData and TriggerWorld build their resources from
 * it, Tod scans its packets. The usual lifecycle is New_<Sub>(name), slot
 * +0x078, freeBuffer, and release when done. FileResource itself is never
 * created on its own.
 *
 * Two subclasses are not assets but the device drivers that implement the
 * file-I/O slots: CdDriver (0x13, the CD-ROM, cd_driver.h) and NullDriver
 * (0x23, null_driver.h). The interface is bound at run time:
 * SetActiveDataSource copies the active driver's eleven interface slots
 * (CopyDataSourceSlots) into this class's table and into the table of every
 * client class, so `self->methods->read(...)` reaches whichever driver is
 * active.
 */
struct FileResource {
    FILERESOURCE_FIELDS(FileResourceMethods);
};

/** The descriptor the file-backed subclasses' ctors take (LinkResource, Tod,
 * TodSet, ModelData, TriggerWorld): a buffer to adopt, or else (buffer
 * NULL) a file name to request through requestLoadFile. */
typedef struct ResourceSource {
    /* +0x00 */ void *buffer; /**< an already-loaded file to adopt, or NULL */
    /* +0x04 */ char *name;   /**< the file to load while buffer is NULL */
} ResourceSource;

/** The descriptor ResourceRequest__Set fills and callers pass on as its
 * `src`. Every ResourceRequest__Set caller sets mode 1; the callers that
 * fill `src` by hand (StageMap__PopulateSlotCells, DayTask__DayTask,
 * GameApplication__GameApplication) leave it unset, and no ctor reads it. */
typedef struct ResourceRequest {
    /* +0x00 */ ResourceSource src; /**< what the ctor is handed */
    /* +0x08 */ s32 mode;           /**< 1 from every ResourceRequest__Set caller; no reader */
} ResourceRequest;

/** @brief Fills a ResourceRequest.
 * @param self the request to fill
 * @param buffer src.buffer: an already-loaded file, or NULL
 * @param name src.name: the file to load
 * @param mode stored in `mode`
 * @return self */
ResourceRequest *ResourceRequest__Set(ResourceRequest *self, void *buffer, char *name, s32 mode);

/** FileResource's own method table. */
extern FileResourceMethods gFileResourceMethods;

/** @brief FileResource's method-table getter.
 * @return &gFileResourceMethods */
extern FileResourceMethods *GetFileResourceMethods(void);

/** @brief Clears freeGuard, finalizes the object (its own finalize, then
 * BasicClass's) and returns its memory to the pool.
 * @param self the object to destroy
 * @return NULL, for the caller to store over its pointer */
void *FileResource__Release(FileResource *self);

/** @brief Constructor: BasicClass's, then this table, no buffer, and every
 * state field zeroed.
 * @param self the object */
void FileResource__FileResource(FileResource *self);

/** @brief Finalize: closes the file and frees the buffer.
 * @param self the object */
void FileResource__Finalize(FileResource *self);

/** @brief Reads a whole file into a new pool buffer, unless the object
 * already holds one: open, seek to the end for the size, allocate, seek
 * back, read, close. isOpen is cleared meanwhile and restored on success.
 * @param self the object that gets the buffer
 * @param name the file to load */
void FileResource__LoadFile(FileResource *self, char *name);

/** @brief Frees the buffer back to the pool, unless there is none, its size
 * is 0 or freeGuard is set.
 * @param self the object */
void FileResource__FreeBuffer(FileResource *self);

/** @name CD request flags
 * Bits CdDriver__RunRequestQueue (cd_driver.c) ORs into a client's `flags`
 * when one of its requests completes; the clients poll them. Bit 0 (1) is
 * left a literal: it is also FileResource__OnRequestDone's bit, and the
 * queue node field that sets it (`unk4`) has no established meaning. @{ */
#define CD_FLAG_DONE 0x002           /**< some request completed */
#define CD_FLAG_NONE_PENDING 0x004   /**< ... and pendingRequests reached 0 */
#define CD_FLAG_OPEN_DONE 0x010      /**< an open request completed */
#define CD_FLAG_CLOSE_DONE 0x020     /**< a close request completed */
#define CD_FLAG_SEEK_DONE 0x040      /**< a seek request completed */
#define CD_FLAG_READ_DONE 0x080      /**< a read request completed */
#define CD_FLAG_LOAD_FILE_DONE 0x200 /**< a load-file request completed */

/** @} */

/** @brief A method table's ctor slot, unprototyped: the allocators that
 * check the ctor's result call it through this. */
typedef struct UnprototypedCtorTable {
    /* +0x000 */ u8 pad0[8];
    /* +0x008 */ s32 (*ctor)(); /**< the class's ctor; nonzero when it succeeded */
} UnprototypedCtorTable;

/** @brief Slot +0x060 of every FileResource table: empty. */
void NoOp(void);

/** @brief Slot +0x064: sets bit 0 of `flags`.
 * @param self the object */
void FileResource__OnRequestDone(FileResource *self);

/** @brief Copies the eleven data-source interface slots (+0x040..+0x058,
 * +0x068..+0x074) of one method table into another; +0x05C..+0x064 are the
 * base's own and are not copied. SetActiveDataSource's rebinding step.
 * @param dst the table to rebind
 * @param src the active driver's table */
void CopyDataSourceSlots(FileResourceMethods *dst, FileResourceMethods *src);

#endif
