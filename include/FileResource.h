#ifndef FILERESOURCE_H
#define FILERESOURCE_H

#include "BasicClass.h"

/*
 * FileResource -- the base of the game's data sources (class id 0x3, method
 * table D_8006D430): a BasicClass subclass that owns one file buffer and
 * declares the file-I/O interface its subclasses reach through their tables.
 * Methods in src/code_171e0.c; sixteen classes derive from it
 * (`typeviews.py --tree`), among them the CD-ROM driver (gCdDriverMethods, 0x13,
 * code_179d8_o/q/s) and the SPU/VAB driver (gVabDriverMethods, 0x23).
 *
 * The interface is bound AT RUN TIME. Slots +0x040..+0x058 and +0x068..+0x074
 * are NULL or placeholders in the static tables; SetActiveDataSource copies
 * the active driver's eleven interface slots (CopyDataSourceSlots) into this
 * class's table and into the table of every class gDataSourceClientGetters
 * lists, so `this->methods->read(...)` reaches whichever driver is active.
 * The slot names are the CD driver's occupants.
 */

/* A disc position in the shape of Psy-Q's CdlLOC, declared as two s16 so the
 * struct is 2-aligned and a whole-struct copy compiles to lwl/lwr + swl/swr
 * (the idiom CLAUDE.md documents). The halves are never read apart. It is
 * here, not in CdDriver.h, because FileResource's own +0x018 has this type:
 * the CD driver's methods run on every client object (SetActiveDataSource
 * binds them into the client tables), so the open file's position and size
 * are fields of the base, not of CdDriver (round 88). */
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
    /* +0x070 */ void (*stopService)(Self *self);               /* CD: CdDriver__StopService; neither occupant reads self, but CdDriver__RunRequestQueue loads $a0 = self before the jalr (round 88) */ \
    /* +0x074 */ void (*cancelRequests)(Self *self);            /* CD: CdDriver__CancelRequests */ \
    /* +0x078 */ void *slot78                                   /* NULL */
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
    /* +0x02A */ u16 unk2A            /* the object is 0x2C bytes: Class6D940's own fields start at +0x02C */
/* clang-format on */

struct FileResourceMethods {
    FILERESOURCE_SLOTS(FileResource, (FileResource * self));
};

struct FileResource {
    FILERESOURCE_FIELDS(FileResourceMethods);
};

extern FileResourceMethods D_8006D430;
extern FileResourceMethods *GetFileResourceMethods(void);

/* The table getters of every class SetActiveDataSource rebinds, NULL-
 * terminated; it sits right after D_8006D430's last slot. */
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
