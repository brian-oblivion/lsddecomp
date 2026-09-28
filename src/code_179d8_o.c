/*
 * code_179d8_o -- the allocator, constructor, finalize and one empty slot
 * of CdDriver, the CD-ROM data source (include/CdDriver.h; method table
 * gCdDriverMethods, header word 0x13 = GameApplicationFileResource.c's DATASOURCE_CD).
 * vram 0x800271D8..0x800272D0. `tools/classtable.py gCdDriverMethods --vs
 * gFileResourceMethods` shows this unit's four functions as:
 *   New_CdDriver          allocates 0x2C bytes, dispatches table +0x008
 *   CdDriver__CdDriver    +0x008: FileResource ctor, own table, InitCdDrive
 *   CdDriver__Finalize    +0x00C: cancelRequests (+0x074), freeBuffer (+0x05C)
 *   CdDriver__NoOpSlot40  +0x040: empty; null in FileResource's table
 * The class's other methods are in code_179d8_s.c and code_179d8_q.c.
 */
#include "common.h"
#include "CdDriver.h"

extern void *BMemPMgrAlloc(s32 size); /* Psy-Q allocator, matched signature used project-wide */
extern void InitCdDrive(void); /* code_179d8_q: one-shot CdSetDebug(0) + CdlSetmode double speed */

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
