#ifndef CODE_2C054_H
#define CODE_2C054_H

#include "common.h"

/*
 * The class allocated by func_8003B854 (0xDC bytes, still INCLUDE_ASM in
 * this unit) and constructed through func_8003BE84's slot +0x008
 * (func_8003B8E4). This is the SAME class `include/Class6D3C8.h` calls
 * `StreamTask`/`StreamTaskMethods` (established there from func_80026170's
 * and friends' call sites in a different unit, code_1677c). Per the
 * convention `include/Entity.h` documents for `BasicClassMethods` vs.
 * `code_55dd4.h`'s `Class65650Methods` -- two independent local views of the
 * identical table are normal and deliberately NOT unified into one shared
 * header, to keep each unit's own edits out of the other's file. This is
 * this unit's own local view: only the vtable slots and object fields this
 * unit's queued functions actually touch are given concrete types.
 *
 * Method table is D_8006E5F8 (77 slots, see `tools/classtable.py
 * D_8006E5F8`). Slots +0x004, +0x044, +0x06C, +0x12C already have an
 * established signature in `Class6D3C8.h`'s `StreamTaskMethods` (all void);
 * +0x00C, +0x080, +0x084 are new here and typed void by the same
 * established-sibling-slot convention (no counter-evidence found).
 */
typedef struct StreamTaskObj StreamTaskObj;
typedef struct StreamTaskObjMethods StreamTaskObjMethods;
typedef struct StreamTaskUnkB4Obj StreamTaskUnkB4Obj;
typedef struct StreamTaskUnkB4Methods StreamTaskUnkB4Methods;

struct StreamTaskObjMethods {
    s32 header; /* +0x000 */
};

/* Object size is 0xDC (from func_8003B854's allocator call). Only the
 * fields this unit's queued functions touch are named. */
struct StreamTaskObj {
    StreamTaskObjMethods *methods; /* +0x000 */
    u8 pad04[0x038 - 0x004];
    s32 unk38;                     /* +0x038, read (and returned) by func_8003C1DC */
    u8 pad3C[0x040 - 0x03C];
    s32 unk40;                     /* +0x040, set by func_8003BCF4 */
    u8 pad44[0x0B4 - 0x044];
    StreamTaskUnkB4Obj *unkB4;      /* +0x0B4, an object with its own 1-slot vtable
                                        (see StreamTaskUnkB4Obj below); dispatched
                                        through by func_8003B9DC */
    s32 unkB8;                       /* +0x0B8, set by func_8003BA58's arg2 */
    s32 unkBC;                        /* +0x0BC, set by func_8003BA58's typeLookup */
    s32 unkC0;                         /* +0x0C0, set by func_8003BA58's flag (5th, stack-spilled) */
    s32 unkC4;                          /* +0x0C4, get/set by func_8003BE5C */
    s32 unkC8;                           /* +0x0C8, get/set by func_8003BE64 */
    s32 unkCC;                            /* +0x0CC, get/set by func_8003BE6C */
    s32 unkD0;                             /* +0x0D0, get/set by func_8003BE74 */
    s32 unkD4;                              /* +0x0D4, get/set by func_8003BE7C */
};

/* This class's own "GetMethods" accessor (compare `Get_vtable_Entity` in
 * Entity.h) -- returns &D_8006E5F8 with no other side effect. Called by
 * func_8003B854/func_8003B8E4 (both still INCLUDE_ASM, not this batch) to
 * fetch the ctor at slot +0x008. */
extern StreamTaskObjMethods *func_8003BE84(void);
extern StreamTaskObjMethods D_8006E5F8;

/* self->unkB4's own tiny class: a 1-slot vtable, dispatched through by
 * func_8003B9DC as `self->unkB4->methods->slot04(self->unkB4)` (this
 * unit's only reader). Real shape beyond that one slot is unknown. */
struct StreamTaskUnkB4Methods {
    u8 pad00[0x04];
    void (*slot04)(StreamTaskUnkB4Obj *self); /* +0x004 */
};

struct StreamTaskUnkB4Obj {
    StreamTaskUnkB4Methods *methods; /* +0x000 */
};

/* A sibling class's own method table (D_8006E730, per `tools/classtable.py
 * D_8006E730`) that `StreamTaskObj`'s own slots +0x00C/+0x080/+0x084
 * (func_8003B9DC/func_8003BD74/func_8003BDAC) forward straight through to,
 * passing `self` on as if it were D_8006E730's own instance -- a delegation
 * pattern, not inheritance (D_8006E730 has its own distinct overrides at
 * +0x040/+0x044/etc., so it is a real sibling class, not StreamTaskObj's
 * base). `Class6D3C8.h` already names this exact table `LoaderTaskMethods`
 * (established from func_8003BE94's ctor call, a DIFFERENT allocator/unit)
 * -- this is this unit's own independent local view of the same table, kept
 * separate rather than editing that header (same policy as
 * StreamTaskObjMethods above). Slot +0x044 is `Class6D3C8.h`'s own
 * `LoaderTaskMethods::slot44`, typed void there from ITS call sites (a
 * different, discarding, caller); this unit's func_8003C1DC -- the function
 * that actually OCCUPIES that slot -- demonstrably reads and returns
 * `self->unk38` right after the call in its own disassembly, so it is typed
 * `s32` here instead. Both typings are compatible at the ABI level (a
 * discarding caller through a void-typed pointer simply never reads $v0);
 * flagged for the head to reconcile, see this unit's match reports. */
typedef struct TaskCoreMethods TaskCoreMethods;

struct TaskCoreMethods {
    u8 pad00[0x00C];
    void (*slot0C)(StreamTaskObj *self);                    /* +0x00C, func_8003B9DC's 2nd call */
    u8 pad10[0x044 - 0x010];
    s32 (*slot44)(StreamTaskObj *self, s32 a1, s32 a2);       /* +0x044, func_8003C1DC occupies this slot */
    u8 pad48[0x080 - 0x048];
    void (*slot80)(StreamTaskObj *self);                        /* +0x080, func_8003BD74's forward target */
    void (*slot84)(StreamTaskObj *self);                         /* +0x084, func_8003BDAC's forward target */
};

extern TaskCoreMethods *func_8003DFBC(void); /* returns &D_8006E730 */

/* A second sibling table (D_8006E878, `tools/classtable.py D_8006E878`),
 * used by func_8003C1DC to forward its own work one level further down the
 * same delegation chain. Only the one slot reached here is typed; its
 * return value is discarded at this call site either way. */
typedef struct TaskUtilMethods TaskUtilMethods;

struct TaskUtilMethods {
    u8 pad00[0x044];
    void (*slot44)(StreamTaskObj *self, s32 a1, s32 a2); /* +0x044 */
};

extern TaskUtilMethods *func_8003E5C8(void); /* returns &D_8006E878 */

#endif
