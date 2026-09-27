#ifndef FILERESOURCE_H
#define FILERESOURCE_H

#include "BasicClass.h"

/*
 * FileResource -- the base of every class the game loads from a file (class
 * id 0x3, method table gFileResourceMethods, parent BasicClass). Its own
 * methods are in src/code_171e0.c.
 *
 * The object owns one file buffer. FileResource__LoadFile opens a named
 * file, takes its size from seek(0, 2), allocates that much from the
 * BMemPMgr heap, reads the whole file into it and closes it; FreeBuffer
 * releases the buffer unless freeGuard is set. The subclasses are the
 * game's file-backed assets (TimImage, TileMap, TileAtlas, ModelData, Tod,
 * VabStreamObj, ...; `typeviews.py --tree` lists all sixteen). A subclass
 * constructor names its file (loadFile, or requestLoadFile to queue it on
 * the CD driver), and slot +0x078 is the subclass's own step that consumes
 * the loaded buffer: TimImage__Upload sends it to VRAM, ModelData and
 * TriggerWorld build their resources from it, Tod scans its packets. The
 * usual use is New_<Sub>(name), slot +0x078, freeBuffer, and release when
 * done. FileResource itself is never created on its own.
 *
 * Two subclasses are not assets but the device drivers that implement the
 * file-I/O slots: CdDriver (0x13, the CD-ROM) and VabDriver (0x23). The
 * interface is bound at run time: slots +0x040..+0x058 and +0x068..+0x074
 * are NULL or placeholders in the static tables, and SetActiveDataSource
 * copies the active driver's eleven interface slots (CopyDataSourceSlots)
 * into this class's table and into the table of every class
 * gDataSourceClientGetters lists, so `this->methods->read(...)` reaches
 * whichever driver is active. The slot names are the CD driver's occupants.
 */

/* A disc position: Psy-Q's CdlLOC (minute, second, sector, track) by
 * layout and by use (CdControl's setloc argument, CdSearchFile's output,
 * CdPosToInt's input). The halves are never read apart. Declared here
 * rather than taken from <libcd.h> only because four CD units
 * (code_179d8_h/q/r/s) still declare the Cd* functions their own way and
 * would collide with Sony's prototypes; the whole-struct copies compile to
 * the same lwl/lwr + swl/swr with Sony's 1-aligned CdlLOC. It lives in this
 * header because the CD driver's methods run on every client object, so the
 * open file's position and size are fields of the base. */
typedef struct CdLoc16 {
    s16 unk0;
    s16 unk2;
} CdLoc16;

typedef struct FileResource FileResource;
typedef struct FileResourceMethods FileResourceMethods;

/* clang-format off */
#define FILERESOURCE_SLOTS(Self, CtorParams)                                                         \
    BASICCLASS_SLOTS(Self, CtorParams);                                                            \
    /* +0x040 */ void (*slot40)(void);                          /* CD: CdDriver__NoOpSlot40 */   \
    /* +0x044 */ void (*open)(Self *self, char *name, s32 arg2, s32 arg3); /* CD: CdDriver__Open */ \
    /* +0x048 */ void (*close)(Self *self);                     /* CD: CdDriver__Close */        \
    /* +0x04C */ s32 (*seek)(Self *self, u32 offset, s32 mode); /* CD: CdDriver__Seek; LoadFile's seek(0, 2) returns the size */ \
    /* +0x050 */ void (*slot50)(void);                          /* CD: CdDriver__NoOpSlot50 */   \
    /* +0x054 */ s32 (*read)(Self *self, void *buf, u32 size);  /* CD: CdDriver__Read */         \
    /* +0x058 */ void (*loadFile)(Self *self, char *name);      /* FileResource__LoadFile; CD: CdDriver__LoadFile */ \
    /* +0x05C */ void (*freeBuffer)(Self *self);                /* FileResource__FreeBuffer */       \
    /* +0x060 */ void (*slot60)(void);                          /* NoOp, in every FileResource table */ \
    /* +0x064 */ void (*setFlag)(Self *self);                   /* FileResource__SetFlag */          \
    /* +0x068 */ void (*runRequestQueue)(void);                 /* CD: CdDriver__RunRequestQueue */ \
    /* +0x06C */ void (*requestLoadFile)(Self *self, char *name); /* CD: CdDriver__RequestLoadFile */ \
    /* +0x070 */ void (*stopService)(Self *self);               /* CD: CdDriver__StopService; neither occupant reads self, but CdDriver__RunRequestQueue passes it */ \
    /* +0x074 */ void (*cancelRequests)(Self *self);            /* CD: CdDriver__CancelRequests */ \
    /* +0x078 */ void *processBuffer                                   /* NULL here; each subclass's step that consumes the loaded buffer (TimImage__Upload, ModelData__BuildResources, ...), signature per class */
/* clang-format on */

/* clang-format off */
#define FILERESOURCE_FIELDS(Methods)                                                                 \
    BASICCLASS_FIELDS(Methods);                                                                    \
    /* +0x00C */ s32 isOpen;          /* cleared while LoadFile runs, then restored */             \
    /* +0x010 */ void *buffer;        /* LoadFile's allocation, NULL when none */                  \
    /* +0x014 */ s32 bufferSize;                                                                   \
    /* +0x018 */ CdLoc16 pos;      /* the open file's disc position: the CD driver's Open sets it, */ \
    /* +0x01C */ u32 size;         /* its byte size; Seek and ReadCdFile seek from pos (CdDriver.h) */ \
    /* +0x020 */ u16 freeGuard;       /* nonzero: FreeBuffer keeps the buffer */                   \
    /* +0x022 */ u16 pendingRequests;                                                              \
    /* +0x024 */ s32 flags;           /* bit 0 set by SetFlag */                                   \
    /* +0x028 */ u16 inQueueDispatch;                                                              \
    /* +0x02A */ u16 loadState            /* a subclass's load step, 0 when idle (LbdFile, VabStreamObj); the object is 0x2C bytes */
/* clang-format on */

struct FileResourceMethods {
    FILERESOURCE_SLOTS(FileResource, (FileResource * self));
};

struct FileResource {
    FILERESOURCE_FIELDS(FileResourceMethods);
};

/* The descriptor the file-backed subclasses' ctors take (LinkResource, Tod,
 * TodSet, ModelData, TriggerWorld): a buffer to adopt, or else (buffer
 * NULL) a file name to request through requestLoadFile. */
typedef struct ResourceSource {
    /* +0x00 */ void *buffer;
    /* +0x04 */ char *name;
} ResourceSource;

extern FileResourceMethods gFileResourceMethods;
extern FileResourceMethods *GetFileResourceMethods(void);

/* The table getters of every class SetActiveDataSource rebinds, NULL-
 * terminated; it sits right after gFileResourceMethods's last slot. */
extern void *(*gDataSourceClientGetters[])(void);

void *FileResource__Release(FileResource *self);
void FileResource__FileResource(FileResource *self);
void FileResource__Finalize(FileResource *self);
void FileResource__LoadFile(FileResource *self, char *name);
void FileResource__FreeBuffer(FileResource *self);
void NoOp(void);
void FileResource__SetFlag(FileResource *self);
void CopyDataSourceSlots(FileResourceMethods *dst, FileResourceMethods *src);

#endif
