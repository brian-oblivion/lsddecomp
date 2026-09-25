/*
 * code_179d8_o -- the allocator, constructor, destructor and one empty slot
 * of Class6D4E8, the CD-drive data source (method table D_8006D4E8, header
 * word 0x13 = code_171e0.c's DATASOURCE_CD).  vram 0x800271D8..0x800272D0.
 *
 * Class6D4E8 derives from Class6D430 (code_171e0.c, the buffer-owning data
 * source base), which derives from BasicClass.  `tools/classtable.py
 * 0x8006D4E8 --vs 0x8006D430` shows this unit's four functions as:
 *   New_Class6D4E8          allocates 0x2C bytes, dispatches table +0x008
 *   Class6D4E8__Class6D4E8  +0x008: Class6D430 ctor, own table, InitCdDrive
 *   Class6D4E8__Destroy     +0x00C: cancelRequests (+0x074), freeBuffer (+0x05C)
 *   Class6D4E8__NoOpSlot40  +0x040: empty; null in Class6D430's table
 * The class's other methods (RequestLoadFile, StopCdService,
 * CancelRequests, the table getter) live in code_179d8_q.c.  The struct
 * views below are local to this unit and cover only what it touches.
 */
#include "common.h"

#define CLASS6D4E8_SIZE 0x2C /* New_Class6D4E8's BMemPMgrAlloc request */

typedef struct Class6D4E8Methods Class6D4E8Methods;
struct Class6D4E8Methods {
    s32 header;                    /* +0x000, not a pointer -- 0x13 (DATASOURCE_CD in code_171e0.c) for D_8006D4E8 */
    void *ownDtorChain;            /* +0x004, DestroyChained -- unused by this unit's own functions */
    void (*ctor)(void *self);      /* +0x008, Class6D4E8__Class6D4E8 (classtable.py; BasicClass__BasicClass
                                     * and Class6D430__Class6D430 sit at this slot in the base tables) */
    u8 pad00C[0x05C - 0x00C];
    void (*freeBuffer)(void *self);      /* +0x05C, inherited Class6D430__FreeBuffer */
    u8 pad060[0x074 - 0x060];
    void (*cancelRequests)(void *self);  /* +0x074, Class6D4E8__CancelRequests (code_179d8_q), which reads
                                           * its `self` in $a0.  Round 79 correction: this slot was typed
                                           * `void (*)(void)` on the strength of the nop delay slot, but $a0
                                           * still holds Class6D4E8__Destroy's own `self` there (the entry
                                           * `move s0,a0` leaves it intact), so passing `self` is
                                           * byte-identical -- the same shape as Class6D430__Destroy's
                                           * `onBufferChanged(this)` in code_171e0.c. */
};

typedef struct {
    Class6D4E8Methods *methods;      /* +0x000, set by the constructor to this class's own table
                                     * (GetClass6D4E8Methods()'s return value) -- the table-pointer-at-offset-0
                                     * convention CLAUDE.md documents for this project's class framework */
    u8 pad004[0x028 - 0x004];
    s16 unk28;                     /* +0x028, cleared by the constructor (a halfword store, `sh`); Class6D430's
                                     * own ctor already clears it too (code_171e0.h: u16 unk28). No reader
                                     * in this unit, so no name. */
} Class6D4E8;

/* The parent class's table (D_8006D430, Class6D430Methods in
 * include/code_171e0.h), viewed only down to the one slot this unit's
 * constructor uses: +0x008 is Class6D430__Class6D430. */
typedef struct {
    u8 pad0[0x008];
    void (*ctor)(void *self);      /* +0x008 */
} Class6D430CtorView;

extern void *BMemPMgrAlloc(s32 size);              /* Psy-Q allocator, matched signature used project-wide */
/* ROUND 59 (extern review): parameter list only, return type untouched. This
 * is NOT still INCLUDE_ASM -- it is defined in src/code_171e0.c as
 * `void *GetClass6D430Methods(void)`, and the callee at 0x80026C9C is
 * lui/addiu/jr reading no argument register. Measured before changing it:
 * Class6D4E8__Class6D4E8's `jal 80026c9c` (0x80027234) carries `move s0,a0` in the
 * delay slot -- a callee-save spill, not argument setup -- so the `self`
 * passed below costs zero bytes and the one-parameter prototype was simply
 * false. Unspecified parameters keep Class6D4E8__Class6D4E8's call site untouched. */
extern Class6D430CtorView *GetClass6D430Methods();
extern Class6D4E8Methods *GetClass6D4E8Methods(void); /* code_179d8_q: returns &D_8006D4E8, this class's table */
extern void InitCdDrive(void);                        /* code_179d8_q: one-shot CdSetDebug(0) + CdlSetmode double speed */

Class6D4E8 *New_Class6D4E8(void)
{
    Class6D4E8 *self;

    self = BMemPMgrAlloc(CLASS6D4E8_SIZE);
    if (self != NULL) {
        GetClass6D4E8Methods()->ctor(self);
        return self;
    }
    return NULL;
}

void Class6D4E8__Class6D4E8(Class6D4E8 *self)
{
    GetClass6D430Methods(self)->ctor(self);
    self->methods = GetClass6D4E8Methods();
    self->unk28 = 0;
    InitCdDrive();
}

void Class6D4E8__Destroy(Class6D4E8 *self)
{
    self->methods->cancelRequests(self);
    self->methods->freeBuffer(self);
}

void Class6D4E8__NoOpSlot40(void)
{
}
