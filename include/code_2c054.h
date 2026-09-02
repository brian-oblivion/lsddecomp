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
typedef struct StreamTaskUnk78Obj StreamTaskUnk78Obj;
typedef struct StreamTaskUnk78Methods StreamTaskUnk78Methods;
typedef struct StreamTaskUnkCObj StreamTaskUnkCObj;
typedef struct TaskTextObj TaskTextObj;
typedef struct TaskTextMethods TaskTextMethods;
typedef struct TaskCoreObj TaskCoreObj;

struct StreamTaskObjMethods {
    s32 header; /* +0x000 */
    u8 pad04[0x060 - 0x004];
    void (*slot60)(StreamTaskObj *self, s32 a1); /* +0x060, func_8003BC14 occupies this slot
                                                      (per tools/classtable.py D_8006E5F8) */
    u8 pad64[0x06C - 0x064];
    void (*slot6C)(StreamTaskObj *self, s32 a1); /* +0x06C, func_8003BCF4 occupies this slot
                                                      (per tools/classtable.py D_8006E5F8);
                                                      func_8003BAB4 dispatches through it rather
                                                      than calling func_8003BCF4 directly */
    u8 pad70[0x09C - 0x070];
    void (*slot9C)(StreamTaskObj *self, s32 a1); /* +0x09C, func_8003C11C's forward target
                                                      (D_8006E5F8+0x09C = func_8003CAF8) */
    void (*slotA0)(StreamTaskObj *self, s32 a1); /* +0x0A0, func_8003C11C's forward target
                                                      (D_8006E5F8+0x0A0 = func_8003CB30) */
    void (*slotA4)(StreamTaskObj *self, u8 *a1, u8 *a2, u8 *a3); /* +0x0A4, func_8003C11C's
                                                      forward target (D_8006E5F8+0x0A4 = func_8003CB68) */
};

/* Object size is 0xDC (from func_8003B854's allocator call). Only the
 * fields this unit's queued functions touch are named. */
struct StreamTaskObj {
    StreamTaskObjMethods *methods; /* +0x000 */
    u8 pad04[0x00C - 0x004];
    StreamTaskUnkCObj *unkC;       /* +0x00C, a pointer to a 1-word holder whose own
                                        word 0 is the actual dispatch target (see
                                        StreamTaskUnkCObj below); read by func_8003C3D0 */
    u8 pad10[0x018 - 0x010];
    TaskCoreObj *unk18;             /* +0x018, an object whose vtable has the same
                                        shape as TaskCoreMethods (see TaskCoreObj
                                        below); dispatched through by func_8003C3D0 */
    u8 pad1C[0x028 - 0x01C];
    s32 unk28;                        /* +0x028, set to 3 by func_8003C11C */
    s32 unk2C;                         /* +0x02C, set to 0x12C by func_8003C11C */
    s32 unk30;                          /* +0x030, set to 0x40 by func_8003C11C */
    s32 unk34;                       /* +0x034, tested by func_8003C3D0, set to 1 by
                                          func_8003C11C */
    s32 unk38;                     /* +0x038, read (and returned) by func_8003C1DC */
    s32 unk3C;                     /* +0x03C, set to 0 by func_8003C11C */
    s32 unk40;                     /* +0x040, set by func_8003BCF4 */
    u8 pad44[0x078 - 0x044];
    StreamTaskUnk78Obj *unk78;       /* +0x078, an object with its own tiny vtable
                                          (see StreamTaskUnk78Obj below); dispatched
                                          through by func_8003C3D0 */
    u8 pad7C[0x084 - 0x07C];
    s32 unk84;                       /* +0x084, set to 9 by func_8003C11C */
    u8 pad88[0x093 - 0x088];
    s8 unk93;                          /* +0x093, only ever address-taken (a buffer
                                            passed to func_8003C3D0's slot78 call);
                                            real extent beyond one byte unknown */
    u8 pad94[0x09C - 0x094];
    s32 unk9C;                          /* +0x09C, set to 0 by func_8003C11C */
    s32 unkA0;                           /* +0x0A0, set to 0 by func_8003C11C */
    s32 unkA4;                     /* +0x0A4, reset to 0 by func_8003BAB4; read
                                        and set to a call result by func_8003BB5C */
    u8 padA8[0x0B4 - 0x0A8];
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
    s32 unkD8;                               /* +0x0D8, read by func_8003BB5C (object end, 0xDC) */
};

/* This class's own "GetMethods" accessor (compare `Get_vtable_Entity` in
 * Entity.h) -- returns &D_8006E5F8 with no other side effect. Called by
 * func_8003B854/func_8003B8E4 (both still INCLUDE_ASM, not this batch) to
 * fetch the ctor at slot +0x008. */
extern StreamTaskObjMethods *func_8003BE84(void);
extern StreamTaskObjMethods D_8006E5F8;

/* A rodata table func_8003C11C reaches only by ADDRESS (`lui`/`addiu`, no
 * `lw`/`sw` here) -- passed to slotA4 as three pointers 3 bytes apart
 * (base, base+3, base+6). Never decoded further by this unit's queued
 * functions, so typed as a plain byte array. */
extern u8 D_8006E860[];

/* self->unkB4's own tiny class: a 1-slot vtable, dispatched through by
 * func_8003B9DC as `self->unkB4->methods->slot04(self->unkB4)` (this
 * unit's only reader). Real shape beyond that one slot is unknown. */
struct StreamTaskUnkB4Methods {
    u8 pad00[0x04];
    void (*slot04)(StreamTaskUnkB4Obj *self); /* +0x004 */
    u8 pad08[0x040 - 0x008];
    s32 (*slot40)(StreamTaskUnkB4Obj *self, s32 a1, s32 a2, s32 a3, s32 a4); /* +0x040,
                                        func_8003BAB4's forward target; return tested
                                        directly (not stored) there */
    u8 pad44[0x048 - 0x044];
    s32 (*slot48)(StreamTaskUnkB4Obj *self); /* +0x048, func_8003BB5C's forward target;
                                        return stored into StreamTaskObj::unkA4 there */
    void (*slot4C)(StreamTaskUnkB4Obj *self); /* +0x04C, func_8003BDF4's forward target */
    u8 pad50[0x06C - 0x050];
    void (*slot6C)(StreamTaskUnkB4Obj *self, s32 a1); /* +0x06C, func_8003BAB4's forward target */
};

struct StreamTaskUnkB4Obj {
    StreamTaskUnkB4Methods *methods; /* +0x000 */
};

/* self->unk78's own tiny class -- structurally identical shape to
 * StreamTaskUnkB4Obj/Methods (a 1-word vtable pointer object), but kept as a
 * SEPARATE type rather than reused: nothing ties the two fields to the same
 * concrete class, only the same generic "vtable at offset 0" idiom every
 * class in this game uses. Only the one slot func_8003C3D0 reaches is
 * known. */
struct StreamTaskUnk78Methods {
    u8 pad00[0x050];
    void (*slot50)(StreamTaskUnk78Obj *self); /* +0x050, func_8003C3D0's forward target */
};

struct StreamTaskUnk78Obj {
    StreamTaskUnk78Methods *methods; /* +0x000 */
};

/* self->unkC points at a 1-word holder (not a class instance itself -- its
 * one word is read but its own vtable, if it has one, is never reached).
 * That word is the real dispatch target, `TaskTextObj`: another generic
 * "vtable at offset 0" object, reached by func_8003C3D0 through one slot
 * (+0x078) that takes a buffer pointer and a flag -- resembles a
 * print-into-buffer call, but nothing here confirms that beyond the shape
 * of the call (self->unk93's address is the second argument). */
struct StreamTaskUnkCObj {
    TaskTextObj *unk0; /* +0x000 */
};

struct TaskTextMethods {
    u8 pad00[0x078];
    void (*slot78)(TaskTextObj *self, s8 *buf, s32 flag); /* +0x078 */
};

struct TaskTextObj {
    TaskTextMethods *methods; /* +0x000 */
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
    u8 pad48[0x04C - 0x048];
    void (*slot4C)(StreamTaskObj *self);                       /* +0x04C, func_8003BAB4's forward target
                                                                     (D_8006E730+0x04C = func_8003C238) */
    u8 pad50[0x05C - 0x050];
    void (*slot5C)(StreamTaskObj *self, s32 a1, s32 a2);       /* +0x05C, func_8003BB5C's forward target
                                                                     (D_8006E730+0x05C = func_8003C51C) */
    u8 pad60[0x074 - 0x060];
    void (*slot74)(TaskCoreObj *self);                          /* +0x074, func_8003C3D0's forward target
                                                                      via self->unk18, an OWN INSTANCE of this
                                                                      class rather than the D_8006E730 singleton
                                                                      (D_8006E730+0x074 = func_8003C7F4) */
    void (*slot78)(StreamTaskObj *self);                        /* +0x078, func_8003BD10's forward target
                                                                      (D_8006E730+0x078 = func_8003C858) */
    u8 pad7C[0x080 - 0x07C];
    void (*slot80)(StreamTaskObj *self);                        /* +0x080, func_8003BD74's forward target */
    void (*slot84)(StreamTaskObj *self);                         /* +0x084, func_8003BDAC's forward target */
    u8 pad88[0x090 - 0x088];
    void (*slot90)(TaskCoreObj *self);                            /* +0x090, func_8003C3D0's forward target
                                                                        via self->unk18 (same instance as slot74;
                                                                        D_8006E730+0x090 = func_8003CA1C) */
};

extern TaskCoreMethods *func_8003DFBC(void); /* returns &D_8006E730 */

/* An actual instance of TaskCoreMethods's class (as opposed to the
 * D_8006E730 singleton `func_8003DFBC` always returns) -- reached through
 * `StreamTaskObj::unk18`. Only the vtable-pointer field is needed: every
 * call through it passes the TaskCoreObj itself as `self`, exactly like
 * StreamTaskUnkB4Obj/StreamTaskUnk78Obj above. */
struct TaskCoreObj {
    TaskCoreMethods *methods; /* +0x000 */
};

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
