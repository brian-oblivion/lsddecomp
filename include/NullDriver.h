#ifndef NULLDRIVER_H
#define NULLDRIVER_H

#include "FileResource.h"

/*
 * NullDriver -- the data-source driver whose every method is empty (class
 * id 0x23, method table gNullDriverMethods), a FileResource subclass and
 * the CD-ROM driver's (gCdDriverMethods, 0x13) sibling. The id is DATASOURCE_NULL: SetActiveDataSource
 * (src/app/game_shell.c) binds this table's driver-interface slots into
 * FileResource's table and every client table whenever the active source is
 * not DATASOURCE_CD, and GetActiveDataSourceMethods returns it then.
 *
 * Every own method is empty. The ctor, the finalize and the eleven
 * interface slots it overrides (+0x040..+0x058, +0x068..+0x074) are
 * `jr $ra; nop`, `return 0` or a bare 0x40-byte frame (Open, NoOpSlot40),
 * so with the VAB source active the file-I/O interface does nothing; the
 * VAB streaming itself is VabStreamObj's (gVabStreamObjMethods, 0xA03,
 * src/sound/PlacementGridVabSound.c), a separate FileResource subclass. Methods in
 * src/sound/PlacementGridVabSound.c (ctor through NoOpSlot50) and src/sound/PlacementGridVabSound.c
 * (Read onward, and the getter). Each is named for its slot
 * (`classtable.py gNullDriverMethods --vs gFileResourceMethods`); the slot names are
 * FileResource's.
 *
 * NO FIELDS/SLOTS MACROS: no class lies below 0x23 (`typeviews.py --tree`).
 * The object has no known own fields: nothing allocates a NullDriver (no
 * New_NullDriver) and no method reads `self`, so the struct is FileResource's
 * fields and its size is unmeasured.
 */

typedef struct NullDriver NullDriver;
typedef struct NullDriverMethods NullDriverMethods;

struct NullDriverMethods {
    /* The ctor's parameter list is FileResource's: every chained call reaches
     * it as GetActiveDataSourceMethods()->ctor(self). */
    FILERESOURCE_SLOTS(NullDriver, (NullDriver * self));
    /* The table is 29 slots and ends after +0x074: the word at +0x078
     * (FileResource's processBuffer) is gVabStreamObjMethods's header, 0xA03. */
};

struct NullDriver {
    FILERESOURCE_FIELDS(NullDriverMethods);
};

extern NullDriverMethods gNullDriverMethods;
extern NullDriverMethods *GetNullDriverMethods(void);

/* The bodies take no arguments: none of them reads a register. */
void NullDriver__NullDriver(void);      /* +0x008 ctor */
void NullDriver__Destroy(void);         /* +0x00C finalize */
void NullDriver__NoOpSlot40(void);      /* +0x040 slot40 */
void NullDriver__Open(void);            /* +0x044 open */
void NullDriver__Close(void);           /* +0x048 close */
void NullDriver__Seek(void);            /* +0x04C seek */
void NullDriver__NoOpSlot50(void);      /* +0x050 slot50 */
s32 NullDriver__Read(void);             /* +0x054 read: returns 0 */
void NullDriver__LoadFile(void);        /* +0x058 loadFile */
void NullDriver__RunRequestQueue(void); /* +0x068 runRequestQueue */
void NullDriver__RequestLoadFile(void); /* +0x06C requestLoadFile */
void NullDriver__StopService(void);     /* +0x070 stopService */
void NullDriver__CancelRequests(void);  /* +0x074 cancelRequests */

/* The driver's mode, as game_shell.c's data-source wrappers
 * read and set it (the CD driver's counterparts take a third argument). */
extern s32 GetNullDriverMode(s32 *outMode2);
extern s32 SetNullDriverMode(s32 async, s32 mode2);
extern s32 GetNullDriverUseVSyncCallback(void);

#endif
