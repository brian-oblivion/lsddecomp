/*
 * code_179d8_o -- the allocator, constructor, destructor and one empty slot
 * of Class6D4E8, the CD-drive data source (method table gCdDriverMethods, header
 * word 0x13 = code_171e0.c's DATASOURCE_CD).  vram 0x800271D8..0x800272D0.
 *
 * Class6D4E8 derives from Class6D430 (code_171e0.c, the buffer-owning data
 * source base), which derives from BasicClass.  `tools/classtable.py
 * 0x8006D4E8 --vs 0x8006D430` shows this unit's four functions as:
 *   New_CdDriver          allocates 0x2C bytes, dispatches table +0x008
 *   CdDriver__CdDriver  +0x008: Class6D430 ctor, own table, InitCdDrive
 *   CdDriver__Finalize     +0x00C: cancelRequests (+0x074), freeBuffer (+0x05C)
 *   CdDriver__NoOpSlot40  +0x040: empty; null in Class6D430's table
 * The class's other methods (RequestLoadFile, StopCdService,
 * CancelRequests, the table getter) live in code_179d8_q.c.  The struct
 * views below are local to this unit and cover only what it touches.
 */
#include "common.h"
#include "Class6D430.h"

#define CLASS6D4E8_SIZE 0x2C /* New_CdDriver's BMemPMgrAlloc request */

typedef struct Class6D4E8Methods Class6D4E8Methods;
struct Class6D4E8Methods {
    s32 header;                    /* +0x000, not a pointer -- 0x13 (DATASOURCE_CD in code_171e0.c) for gCdDriverMethods */
    void *ownDtorChain;            /* +0x004, Class6D430__Release -- unused by this unit's own functions */
    void (*ctor)(void *self);      /* +0x008, CdDriver__CdDriver (classtable.py; BasicClass__BasicClass
                                     * and Class6D430__Class6D430 sit at this slot in the base tables) */
    u8 pad00C[0x05C - 0x00C];
    void (*freeBuffer)(void *self);      /* +0x05C, inherited Class6D430__FreeBuffer */
    u8 pad060[0x074 - 0x060];
    void (*cancelRequests)(void *self);  /* +0x074, CdDriver__CancelRequests (code_179d8_q), which reads
                                           * its `self` in $a0.  Round 79 correction: this slot was typed
                                           * `void (*)(void)` on the strength of the nop delay slot, but $a0
                                           * still holds CdDriver__Finalize's own `self` there (the entry
                                           * `move s0,a0` leaves it intact), so passing `self` is
                                           * byte-identical -- the same shape as Class6D430__Finalize's
                                           * `close(this)` in code_171e0.c. */
};

typedef struct {
    Class6D4E8Methods *methods;      /* +0x000, set by the constructor to this class's own table
                                     * (GetCdDriverMethods()'s return value) -- the table-pointer-at-offset-0
                                     * convention CLAUDE.md documents for this project's class framework */
    u8 pad004[0x028 - 0x004];
    s16 inQueueDispatch;                     /* +0x028, cleared by the constructor (a halfword store, `sh`); Class6D430's
                                     * own ctor already clears it too (code_171e0.h: u16 unk28). No reader
                                     * in this unit, so no name. */
} Class6D4E8;


extern void *BMemPMgrAlloc(s32 size);              /* Psy-Q allocator, matched signature used project-wide */
extern Class6D4E8Methods *GetCdDriverMethods(void); /* code_179d8_q: returns &gCdDriverMethods, this class's table */
extern void InitCdDrive(void);                        /* code_179d8_q: one-shot CdSetDebug(0) + CdlSetmode double speed */

Class6D4E8 *New_CdDriver(void)
{
    Class6D4E8 *self;

    self = BMemPMgrAlloc(CLASS6D4E8_SIZE);
    if (self != NULL) {
        GetCdDriverMethods()->ctor(self);
        return self;
    }
    return NULL;
}

void CdDriver__CdDriver(Class6D4E8 *self)
{
    GetClass6D430Methods()->ctor((Class6D430 *)self);
    self->methods = GetCdDriverMethods();
    self->inQueueDispatch = 0;
    InitCdDrive();
}

void CdDriver__Finalize(Class6D4E8 *self)
{
    self->methods->cancelRequests(self);
    self->methods->freeBuffer(self);
}

void CdDriver__NoOpSlot40(void)
{
}
