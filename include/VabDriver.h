#ifndef VABDRIVER_H
#define VABDRIVER_H

#include "FileResource.h"

/*
 * VabDriver -- the SPU/VAB data-source driver (class id 0x23, method table
 * gVabDriverMethods), a FileResource subclass and the CD-ROM driver's
 * (gCdDriverMethods, 0x13) sibling. The id is DATASOURCE_SPU: SetActiveDataSource
 * (src/code_171e0.c) binds this table's driver-interface slots into
 * FileResource's table and every client table whenever the active source is
 * not DATASOURCE_CD, and GetActiveDataSourceMethods returns it then.
 *
 * Every own method is empty. The ctor, the finalize and the eleven
 * interface slots it overrides (+0x040..+0x058, +0x068..+0x074) are
 * `jr $ra; nop`, `return 0` or a bare 0x40-byte frame (Open, NoOpSlot40),
 * so with the VAB source active the file-I/O interface does nothing; the
 * VAB streaming itself is VabStreamObj's (gVabStreamObjMethods, 0xA03,
 * src/PlacementGridVabSound.c), a separate FileResource subclass. Methods in
 * src/PlacementGridVabSound.c (ctor through NoOpSlot50) and src/PlacementGridVabSound.c
 * (Read onward, and the getter). Each is named for its slot
 * (`classtable.py gVabDriverMethods --vs gFileResourceMethods`); the slot names are
 * FileResource's.
 *
 * NO FIELDS/SLOTS MACROS: no class lies below 0x23 (`typeviews.py --tree`).
 * The object has no known own fields: nothing allocates a VabDriver (no
 * New_VabDriver) and no method reads `self`, so the struct is FileResource's
 * fields and its size is unmeasured.
 */

typedef struct VabDriver VabDriver;
typedef struct VabDriverMethods VabDriverMethods;

struct VabDriverMethods {
    /* The ctor's parameter list is FileResource's: every chained call reaches
     * it as GetActiveDataSourceMethods()->ctor(self). */
    FILERESOURCE_SLOTS(VabDriver, (VabDriver * self));
    /* The table is 29 slots and ends after +0x074: the word at +0x078
     * (FileResource's slot78) is gVabStreamObjMethods's header, 0xA03. */
};

struct VabDriver {
    FILERESOURCE_FIELDS(VabDriverMethods);
};

extern VabDriverMethods gVabDriverMethods;
extern VabDriverMethods *GetVabDriverMethods(void);

/* The bodies take no arguments: none of them reads a register. */
void VabDriver__VabDriver(void);       /* +0x008 ctor */
void VabDriver__Destroy(void);         /* +0x00C finalize */
void VabDriver__NoOpSlot40(void);      /* +0x040 slot40 */
void VabDriver__Open(void);            /* +0x044 open */
void VabDriver__Close(void);           /* +0x048 close */
void VabDriver__Seek(void);            /* +0x04C seek */
void VabDriver__NoOpSlot50(void);      /* +0x050 slot50 */
s32 VabDriver__Read(void);             /* +0x054 read: returns 0 */
void VabDriver__LoadFile(void);        /* +0x058 loadFile */
void VabDriver__RunRequestQueue(void); /* +0x068 runRequestQueue */
void VabDriver__RequestLoadFile(void); /* +0x06C requestLoadFile */
void VabDriver__StopService(void);     /* +0x070 stopService */
void VabDriver__CancelRequests(void);  /* +0x074 cancelRequests */

#endif
