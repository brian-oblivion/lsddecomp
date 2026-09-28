/*
 * CdDriver's lifecycle methods (the class is include/CdDriver.h's): the
 * allocator, the constructor, the finalizer, and the empty slot +0x040.
 *
 *   New_CdDriver          allocate a CdDriver, construct it through its own
 *                         table's ctor slot
 *   CdDriver__CdDriver    FileResource's ctor, then CdDriver's own table,
 *                         inQueueDispatch cleared, and InitCdDrive (once per
 *                         boot: CdSetDebug(0), double-speed mode)
 *   CdDriver__Finalize    cancel the object's queued requests, then free its
 *                         buffer (FileResource__Finalize's shape)
 *   CdDriver__NoOpSlot40  an empty body for slot +0x040, which FileResource's
 *                         table leaves null
 *
 * Nothing calls New_CdDriver: the driver's slots are copied into its
 * clients' tables by SetActiveDataSource (CdDriver.h), so the class's
 * request methods run on other objects. The rest of the class is in
 * CdDriver.c, which this file belongs in by content: the merge waits only
 * on a yaml edit (CdDriver.c's .rodata attach moves here), the head's.
 */
#include "common.h"
#include "CdDriver.h"

/* The game's pool allocator, src/BMemPMgr.c. */
extern void *BMemPMgrAlloc(s32 size);
/* Defined in CdDriver.c. */
extern void InitCdDrive(void);

CdDriver *New_CdDriver(void) {
    CdDriver *self;

    self = BMemPMgrAlloc(sizeof(CdDriver));
    if (self != NULL) {
        GetCdDriverMethods()->ctor(self);
        return self;
    }
    return NULL;
}

void CdDriver__CdDriver(CdDriver *self) {
    GetFileResourceMethods()->ctor((FileResource *)self);
    self->methods = GetCdDriverMethods();
    self->inQueueDispatch = 0;
    InitCdDrive();
}

void CdDriver__Finalize(CdDriver *self) {
    self->methods->cancelRequests(self);
    self->methods->freeBuffer(self);
}

void CdDriver__NoOpSlot40(void) {}
