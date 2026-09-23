#ifndef CODE_2C054_H
#define CODE_2C054_H

#include "common.h"

/*
 * The class allocated by New_StreamTaskObj (0xDC bytes, still INCLUDE_ASM in
 * this unit) and constructed through Get_vtable_StreamTaskObj's slot +0x008
 * (StreamTaskObj__StreamTaskObj). This is the SAME class `include/Class6D3C8.h` calls
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
typedef struct TaskCoreObjMethods TaskCoreObjMethods;
typedef struct TaskTextObj TaskTextObj;
typedef struct TaskTextMethods TaskTextMethods;
typedef struct TaskCoreObj TaskCoreObj;
typedef struct StreamTaskInitData StreamTaskInitData;

/* A 3-word struct: StreamTaskObj__StreamTaskObj's optional 5th (stack) argument, and also
 * GetDefaultStreamTaskInitData()'s return type -- both feed the exact same 3-word copy into
 * StreamTaskObj::unkA8/unkAC/unkB0, so they're the same shape. Real field
 * meanings unknown (never dereferenced beyond word offset by this unit's
 * queued functions). */
struct StreamTaskInitData {
    s32 unk0; /* +0x000 */
    s32 unk4; /* +0x004 */
    s32 unk8; /* +0x008 */
};

extern StreamTaskInitData *GetDefaultStreamTaskInitData(void);

struct StreamTaskObjMethods {
    s32 header; /* +0x000 */
    u8 pad04[0x008 - 0x004];
    /* +0x008, the constructor. FIVE parameters: New_StreamTaskObj (this unit's
     * allocator) dispatches it with $a0-$a3 plus a fifth argument stored to
     * 0x10($sp) -- the o32 stack-argument slot. See
     * docs/match-reports/New_StreamTaskObj.md. */
    void (*slot08)(StreamTaskObj *self, s32 a1, s32 a2, s32 a3, s32 a4);
    u8 pad0C[0x040 - 0x00C];
    void (*slot40)(StreamTaskObj *self); /* +0x040, StreamTaskObj__StreamTaskObj's forward target;
                                              occupied by this unit's own StreamTaskObj__Reset
                                              (per tools/classtable.py D_8006E5F8) --
                                              confirms the single-argument signature */
    u8 pad44[0x060 - 0x044];
    void (*slot60)(StreamTaskObj *self, s32 a1); /* +0x060, StreamTaskObj__func_8003BC14 occupies this slot
                                                      (per tools/classtable.py D_8006E5F8) */
    u8 pad64[0x06C - 0x064];
    void (*slot6C)(StreamTaskObj *self, s32 a1); /* +0x06C, StreamTaskObj__SetUnk40 occupies this slot
                                                      (per tools/classtable.py D_8006E5F8);
                                                      StreamTaskObj__func_8003BAB4 dispatches through it rather
                                                      than calling StreamTaskObj__SetUnk40 directly */
    u8 pad70[0x094 - 0x070];
    void (*slot94)(StreamTaskObj *self); /* +0x094, StreamTaskObj__func_8003BC14's forward target;
                                                D_8006E5F8+0x094 = StreamTaskObj__func_8003BDF4, this unit's
                                                own already-matched function (single-arg
                                                signature confirms the arity here) */
    u8 pad98[0x09C - 0x098];
    void (*slot9C)(StreamTaskObj *self, s32 a1); /* +0x09C, TaskCoreObj__Reset's forward target
                                                      (D_8006E5F8+0x09C = func_8003CAF8) */
    void (*slotA0)(StreamTaskObj *self, s32 a1); /* +0x0A0, TaskCoreObj__Reset's forward target
                                                      (D_8006E5F8+0x0A0 = func_8003CB30) */
    void (*slotA4)(StreamTaskObj *self, u8 *a1, u8 *a2, u8 *a3); /* +0x0A4, TaskCoreObj__Reset's
                                                      forward target (D_8006E5F8+0x0A4 = func_8003CB68) */
    u8 padA8[0x0D4 - 0x0A8];
    void (*slotD4)(StreamTaskObj *self, s32 a1, s32 a2); /* +0x0D4, TaskCoreObj__TaskCoreObj's forward
                                                      target (D_8006E5F8+0x0D4 = func_8003CDE0) */
    u8 padD8[0x0DC - 0x0D8];
    void (*slotDC)(StreamTaskObj *self); /* +0x0DC, TaskCoreObj__Destroy's forward target
                                                      (D_8006E5F8+0x0DC = func_8003D050) */
    void (*slotE0)(StreamTaskObj *self, s32 a1); /* +0x0E0, TaskCoreObj__func_8003C238's forward target */
    void (*slotE4)(StreamTaskObj *self, u8 *a1); /* +0x0E4, TaskCoreObj__func_8003C238's forward target;
                                                      a1 is `&self->unk90`, an address into self */
};

/* Object size is 0xDC (from New_StreamTaskObj's allocator call). Only the
 * fields this unit's queued functions touch are named. */
struct StreamTaskObj {
    StreamTaskObjMethods *methods; /* +0x000 */
    u8 pad04[0x00C - 0x004];
    StreamTaskUnkCObj *unkC;       /* +0x00C, a pointer to a 1-word holder whose own
                                        word 0 is the actual dispatch target (see
                                        StreamTaskUnkCObj below); read by TaskCoreObj__func_8003C3D0 */
    u8 pad10[0x014 - 0x010];
    s32 unk14;                      /* +0x014, an argument forwarded to self->methods'
                                        slotE0 and (reloaded) unk78's slot4C by
                                        TaskCoreObj__func_8003C238 */
    TaskCoreObj *unk18;             /* +0x018, an object whose vtable is
                                        TaskCoreObjMethods -- NOT the same struct as
                                        TaskCoreMethods (see TaskCoreObj below for the
                                        correction) */
    u8 pad1C[0x028 - 0x01C];
    s32 unk28;                        /* +0x028, set to 3 by TaskCoreObj__Reset */
    s32 unk2C;                         /* +0x02C, set to 0x12C by TaskCoreObj__Reset */
    s32 unk30;                          /* +0x030, set to 0x40 by TaskCoreObj__Reset */
    s32 unk34;                       /* +0x034, tested by TaskCoreObj__func_8003C3D0, set to 1 by
                                          TaskCoreObj__Reset */
    s32 unk38;                     /* +0x038, read (and returned) by TaskCoreObj__func_8003C1DC */
    s32 unk3C;                     /* +0x03C, set to 0 by TaskCoreObj__Reset */
    s32 unk40;                     /* +0x040, set by StreamTaskObj__SetUnk40 */
    s32 unk44;                       /* +0x044, set from TaskCoreObj__TaskCoreObj's arg2 */
    StreamTaskUnkB4Obj *unk48;        /* +0x048, set by TaskCoreObj__TaskCoreObj: either a call
                                           result (arg2 nonzero) or arg3 verbatim;
                                           torn down by TaskCoreObj__Destroy's slot04 (retyped
                                           from a wrong s32 guess -- see that report) */
    u8 pad4C[0x070 - 0x04C];
    s32 unk70;                          /* +0x070, tested by TaskCoreObj__Destroy */
    StreamTaskUnkB4Obj *unk74;           /* +0x074, torn down by TaskCoreObj__Destroy's slot04 */
    StreamTaskUnk78Obj *unk78;       /* +0x078, its OWN class again (see StreamTaskUnk78Obj
                                          below) -- UN-unified from StreamTaskUnkB4Obj this
                                          same round, see TaskCoreObj__func_8003C238's report: its slot
                                          +0x04C needs 3 args here where StreamTaskUnkB4Obj's
                                          own slot +0x04C (StreamTaskObj__func_8003BDF4) needs 1; the two
                                          classes only ever agreed at slot04 */
    StreamTaskUnkB4Obj *unk7C;        /* +0x07C, set by TaskCoreObj__TaskCoreObj from a call result;
                                           torn down by TaskCoreObj__Destroy's slot04 (retyped
                                           from a wrong s32 guess -- see that report) */
    StreamTaskUnkB4Obj *unk80;         /* +0x080, set by TaskCoreObj__TaskCoreObj from a call result;
                                           torn down by TaskCoreObj__Destroy's slot04 (retyped
                                           from a wrong s32 guess -- see that report) */
    s32 unk84;                       /* +0x084, set to 9 by TaskCoreObj__Reset */
    s32 unk88;                        /* +0x088, tested by TaskCoreObj__func_8003C238 (guards a block) */
    u8 pad8C[0x090 - 0x08C];
    u8 unk90;                          /* +0x090, only ever address-taken (a buffer start,
                                            passed to TaskCoreObj__func_8003C238's slotE4/slotB8 calls);
                                            real extent beyond one byte unknown -- unk93,
                                            3 bytes further into the SAME buffer, is the
                                            other known address-taken offset in it */
    u8 pad91[0x093 - 0x091];
    s8 unk93;                          /* +0x093, only ever address-taken (a buffer
                                            passed to TaskCoreObj__func_8003C3D0's slot78 call);
                                            real extent beyond one byte unknown */
    u8 pad94[0x09C - 0x094];
    s32 unk9C;                          /* +0x09C, set to 0 by TaskCoreObj__Reset */
    s32 unkA0;                           /* +0x0A0, set to 0 by TaskCoreObj__Reset */
    s32 unkA4;                     /* +0x0A4, reset to 0 by StreamTaskObj__func_8003BAB4; read
                                        and set to a call result by StreamTaskObj__func_8003BB5C */
    StreamTaskInitData unkA8;         /* +0x0A8, whole-struct-copied by StreamTaskObj__StreamTaskObj from
                                          either its 5th argument or GetDefaultStreamTaskInitData()'s
                                          default (retail batches all 3 loads before all
                                          3 stores -- a struct assignment, not 3 separate
                                          field writes) */
    StreamTaskUnkB4Obj *unkB4;      /* +0x0B4, an object with its own 1-slot vtable
                                        (see StreamTaskUnkB4Obj below); dispatched
                                        through by StreamTaskObj__Destroy */
    s32 unkB8;                       /* +0x0B8, set by StreamTaskObj__Configure's arg2 */
    s32 unkBC;                        /* +0x0BC, set by StreamTaskObj__Configure's typeLookup */
    s32 unkC0;                         /* +0x0C0, set by StreamTaskObj__Configure's flag (5th, stack-spilled) */
    s32 unkC4;                          /* +0x0C4, get/set by StreamTaskObj__SetUnkC4 */
    s32 unkC8;                           /* +0x0C8, get/set by StreamTaskObj__SetUnkC8 */
    s32 unkCC;                            /* +0x0CC, get/set by StreamTaskObj__SetUnkCC */
    s32 unkD0;                             /* +0x0D0, get/set by StreamTaskObj__SetUnkD0 */
    s32 unkD4;                              /* +0x0D4, get/set by StreamTaskObj__SetUnkD4 */
    s32 unkD8;                               /* +0x0D8, read by StreamTaskObj__func_8003BB5C (object end, 0xDC) */
};

/* This class's own "GetMethods" accessor (compare `Get_vtable_Entity` in
 * Entity.h) -- returns &D_8006E5F8 with no other side effect. Called by
 * New_StreamTaskObj/StreamTaskObj__StreamTaskObj (both still INCLUDE_ASM, not this batch) to
 * fetch the ctor at slot +0x008. */
extern StreamTaskObjMethods *Get_vtable_StreamTaskObj(void);
extern StreamTaskObjMethods D_8006E5F8;

/* A rodata table TaskCoreObj__Reset reaches only by ADDRESS (`lui`/`addiu`, no
 * `lw`/`sw` here) -- passed to slotA4 as three pointers 3 bytes apart
 * (base, base+3, base+6). Never decoded further by this unit's queued
 * functions, so typed as a plain byte array. */
extern u8 D_8006E860[];

/* Two more rodata symbols reached only by address (round 2, TaskCoreObj__func_8003C238):
 * `gDefaultStreamTaskInitData`, passed as `TaskTextMethods::slot78`'s 3rd argument, and
 * `D_8006E86C`, passed TWICE (same address) as `TaskCoreObjMethods::slot70`'s
 * 3rd AND 4th arguments. Neither is ever dereferenced by this unit's queued
 * functions. */
extern u8 gDefaultStreamTaskInitData[];
extern u8 D_8006E86C[];

/* self->unkB4's own tiny class: a 1-slot vtable, dispatched through by
 * StreamTaskObj__Destroy as `self->unkB4->methods->slot04(self->unkB4)` (this
 * unit's only reader). Real shape beyond that one slot is unknown.
 *
 * REUSED for StreamTaskObj::unk48/unk74/unk7C/unk80 too (round 2,
 * TaskCoreObj__Destroy): all four fields are torn down identically --
 * `field->methods->slot04(field)` -- from the SAME function, in sequence.
 *
 * unk78 was ALSO folded in here for one round (TaskCoreObj__Destroy), then
 * UN-folded back into its own `StreamTaskUnk78Obj/Methods` type by
 * TaskCoreObj__func_8003C238: that function calls unk78's own slot +0x04C with 3
 * arguments, where THIS struct's slot +0x04C (StreamTaskObj__func_8003BDF4's already-
 * matched, byte-exact call on unkB4) needs exactly 1. Since a vtable slot's
 * signature must be consistent for every instance of the SAME class, a real
 * arity conflict at a shared offset is proof the two are sibling classes
 * (agreeing only on slot04, likely via a common base), not one class --
 * unlike unk48/unk74/unk7C/unk80, which no function has yet called through
 * any slot besides 04 and so have not (yet) produced any such conflict. See
 * TaskCoreObj__func_8003C238's report for the discovery and TaskCoreObj__Destroy's report for
 * an addendum recording the reversal. */
struct StreamTaskUnkB4Methods {
    u8 pad00[0x04];
    void (*slot04)(StreamTaskUnkB4Obj *self); /* +0x004, also TaskCoreObj__Destroy's forward target
                                        via unk48/unk74/unk7C/unk80 */
    u8 pad08[0x040 - 0x008];
    s32 (*slot40)(StreamTaskUnkB4Obj *self, s32 a1, s32 a2, s32 a3, s32 a4); /* +0x040,
                                        StreamTaskObj__func_8003BAB4's forward target; return tested
                                        directly (not stored) there */
    u8 pad44[0x048 - 0x044];
    s32 (*slot48)(StreamTaskUnkB4Obj *self); /* +0x048, StreamTaskObj__func_8003BB5C's forward target;
                                        return stored into StreamTaskObj::unkA4 there */
    void (*slot4C)(StreamTaskUnkB4Obj *self); /* +0x04C, StreamTaskObj__func_8003BDF4's forward target;
                                        1 argument -- see the struct comment for why this
                                        is NOT the same slot as StreamTaskUnk78Methods::slot4C */
    u8 pad50[0x06C - 0x050];
    void (*slot6C)(StreamTaskUnkB4Obj *self, s32 a1); /* +0x06C, StreamTaskObj__func_8003BAB4's forward target */
};

struct StreamTaskUnkB4Obj {
    StreamTaskUnkB4Methods *methods; /* +0x000 */
};

/* self->unk78's own tiny class, RESTORED as its own separate type this same
 * round (see the comment above StreamTaskUnkB4Methods for why). Three known
 * slots now, all from TaskCoreObj__func_8003C238 (slot50 originally from TaskCoreObj__func_8003C3D0). */
struct StreamTaskUnk78Methods {
    u8 pad00[0x04];
    void (*slot04)(StreamTaskUnk78Obj *self); /* +0x004, TaskCoreObj__Destroy's forward target */
    u8 pad08[0x04C - 0x008];
    void (*slot4C)(StreamTaskUnk78Obj *self, s32 a1, s32 a2); /* +0x04C, TaskCoreObj__func_8003C238's
                                        forward target -- 3 arguments, see above */
    void (*slot50)(StreamTaskUnk78Obj *self); /* +0x050, TaskCoreObj__func_8003C3D0's forward target */
    u8 pad54[0x0B8 - 0x054];
    void (*slotB8)(StreamTaskUnk78Obj *self, s32 a1, u8 *a2); /* +0x0B8, TaskCoreObj__func_8003C238's
                                        forward target */
};

struct StreamTaskUnk78Obj {
    StreamTaskUnk78Methods *methods; /* +0x000 */
};

/* self->unkC points at a 1-word holder (not a class instance itself -- its
 * one word is read but its own vtable, if it has one, is never reached).
 * That word is the real dispatch target, `TaskTextObj`: another generic
 * "vtable at offset 0" object, reached by TaskCoreObj__func_8003C3D0 through one slot
 * (+0x078) that takes a buffer pointer and a flag -- resembles a
 * print-into-buffer call, but nothing here confirms that beyond the shape
 * of the call (self->unk93's address is the second argument). */
struct StreamTaskUnkCObj {
    TaskTextObj *unk0; /* +0x000 */
};

struct TaskTextMethods {
    u8 pad00[0x078];
    void (*slot78)(TaskTextObj *self, s8 *buf, u8 *flag); /* +0x078; 3rd param widened from
                                        `s32` to a pointer here (round 2, TaskCoreObj__func_8003C238):
                                        TaskCoreObj__func_8003C3D0's call passes literal 0 (valid for
                                        either type, no byte change there), but
                                        TaskCoreObj__func_8003C238's own call to the same slot passes
                                        `&gDefaultStreamTaskInitData` -- a real address, not an integer */
};

struct TaskTextObj {
    TaskTextMethods *methods; /* +0x000 */
};

/* A sibling class's own method table (gTaskCoreMethods, per `tools/classtable.py
 * gTaskCoreMethods`) that `StreamTaskObj`'s own slots +0x00C/+0x080/+0x084
 * (StreamTaskObj__Destroy/StreamTaskObj__func_8003BD74/StreamTaskObj__func_8003BDAC) forward straight through to,
 * passing `self` on as if it were gTaskCoreMethods's own instance -- a delegation
 * pattern, not inheritance (gTaskCoreMethods has its own distinct overrides at
 * +0x040/+0x044/etc., so it is a real sibling class, not StreamTaskObj's
 * base). `Class6D3C8.h` already names this exact table `LoaderTaskMethods`
 * (established from New_TaskCoreObj's ctor call, a DIFFERENT allocator/unit)
 * -- this is this unit's own independent local view of the same table, kept
 * separate rather than editing that header (same policy as
 * StreamTaskObjMethods above). Slot +0x044 is `Class6D3C8.h`'s own
 * `LoaderTaskMethods::slot44`, typed void there from ITS call sites (a
 * different, discarding, caller); this unit's TaskCoreObj__func_8003C1DC -- the function
 * that actually OCCUPIES that slot -- demonstrably reads and returns
 * `self->unk38` right after the call in its own disassembly, so it is typed
 * `s32` here instead. Both typings are compatible at the ABI level (a
 * discarding caller through a void-typed pointer simply never reads $v0);
 * flagged for the head to reconcile, see this unit's match reports. */
typedef struct TaskCoreMethods TaskCoreMethods;

struct TaskCoreMethods {
    u8 pad00[0x008];
    void (*slot08)(StreamTaskObj *self, s32 a1, s32 a2, s32 a3); /* +0x008, StreamTaskObj__StreamTaskObj's
                                              forward target (gTaskCoreMethods+0x008 = TaskCoreObj__TaskCoreObj,
                                              this unit's own queued function -- confirms arity) */
    void (*slot0C)(StreamTaskObj *self);                    /* +0x00C, StreamTaskObj__Destroy's 2nd call */
    u8 pad10[0x044 - 0x010];
    s32 (*slot44)(StreamTaskObj *self, s32 a1, s32 a2);       /* +0x044, TaskCoreObj__func_8003C1DC occupies this slot */
    u8 pad48[0x04C - 0x048];
    void (*slot4C)(StreamTaskObj *self);                       /* +0x04C, StreamTaskObj__func_8003BAB4's forward target
                                                                     (gTaskCoreMethods+0x04C = TaskCoreObj__func_8003C238) */
    u8 pad50[0x05C - 0x050];
    void (*slot5C)(StreamTaskObj *self, s32 a1, s32 a2);       /* +0x05C, StreamTaskObj__func_8003BB5C's forward target
                                                                     (gTaskCoreMethods+0x05C = func_8003C51C) */
    void (*slot60)(StreamTaskObj *self, s32 a1);                /* +0x060, StreamTaskObj__func_8003BC14's forward target
                                                                     (gTaskCoreMethods+0x060 = func_8003C63C) */
    u8 pad64[0x078 - 0x064];
    void (*slot78)(StreamTaskObj *self);                        /* +0x078, StreamTaskObj__func_8003BD10's forward target
                                                                      (gTaskCoreMethods+0x078 = func_8003C858) */
    u8 pad7C[0x080 - 0x07C];
    void (*slot80)(StreamTaskObj *self);                        /* +0x080, StreamTaskObj__func_8003BD74's forward target */
    void (*slot84)(StreamTaskObj *self);                         /* +0x084, StreamTaskObj__func_8003BDAC's forward target */
    u8 pad88[0x0D8 - 0x088];
    void (*slotD8)(StreamTaskObj *self, s32 a1);                    /* +0x0D8, TaskCoreObj__TaskCoreObj's forward
                                                                        target -- called on `self` itself, not
                                                                        an unk18-style sub-instance, right after
                                                                        self->methods is (temporarily) set to
                                                                        this table's own pointer: a base-class
                                                                        constructor chaining pattern, see that
                                                                        function's report
                                                                        (gTaskCoreMethods+0x0D8 = func_8003CE98) */
};

extern TaskCoreMethods *Get_vtable_TaskCore(void); /* returns &gTaskCoreMethods */

/* `StreamTaskObj::unk18` and its vtable, `TaskCoreObjMethods`.
 *
 * ORIGINALLY modeled (TaskCoreObj__func_8003C3D0, round 1) as sharing `TaskCoreMethods`
 * itself -- self->unk18's own slots +0x074/+0x090, called there, happened to
 * line up with non-null entries in gTaskCoreMethods's OWN table at those same
 * offsets (`classtable.py gTaskCoreMethods`). That agreement turned out to be
 * coincidence, not evidence of a shared class: TaskCoreObj__func_8003C238 (round 2) calls
 * THREE MORE slots on self->unk18 (+0x048/+0x04C/+0x050, all 2-argument
 * setters) that cannot exist on `TaskCoreMethods` -- `gTaskCoreMethods`'s own
 * +0x04C is TaskCoreObj__func_8003C238's own occupant slot (this very function!),
 * confirmed single-argument by StreamTaskObj__func_8003BAB4's byte-exact call. A vtable
 * slot's arity must be consistent for every instance of the SAME class, so
 * a genuine arity conflict at a shared offset means self->unk18 is a
 * DIFFERENT class that merely happens to also be a `BasicClass` descendant
 * (hence the shared low slots every table in this game has) -- not an
 * instance of gTaskCoreMethods's own class. Split into its own
 * `TaskCoreObjMethods` type; see TaskCoreObj__func_8003C238's report. */

struct TaskCoreObjMethods {
    u8 pad00[0x048];
    void (*slot48)(TaskCoreObj *self, s32 a1); /* +0x048, TaskCoreObj__func_8003C238's forward target */
    void (*slot4C)(TaskCoreObj *self, s32 a1); /* +0x04C, TaskCoreObj__func_8003C238's forward target */
    void (*slot50)(TaskCoreObj *self, s32 a1); /* +0x050, TaskCoreObj__func_8003C238's forward target */
    u8 pad54[0x070 - 0x054];
    void (*slot70)(TaskCoreObj *self, s32 a1, u8 *a2, u8 *a3, s32 a4); /* +0x070,
                                        TaskCoreObj__func_8003C238's forward target; a2 and a3 are
                                        the SAME address (&D_8006E86C) at that call site */
    void (*slot74)(TaskCoreObj *self); /* +0x074, TaskCoreObj__func_8003C3D0's forward target */
    u8 pad78[0x08C - 0x078];
    void (*slot8C)(TaskCoreObj *self); /* +0x08C, TaskCoreObj__func_8003C238's forward target */
    void (*slot90)(TaskCoreObj *self); /* +0x090, TaskCoreObj__func_8003C3D0's forward target */
};

struct TaskCoreObj {
    TaskCoreObjMethods *methods; /* +0x000 */
};

/* A second sibling table (gIntermediateBaseMethods, `tools/classtable.py gIntermediateBaseMethods`),
 * used by TaskCoreObj__func_8003C1DC to forward its own work one level further down the
 * same delegation chain. Only the one slot reached here is typed; its
 * return value is discarded at this call site either way. */
typedef struct TaskUtilMethods TaskUtilMethods;

struct TaskUtilMethods {
    u8 pad00[0x008];
    void (*slot08)(StreamTaskObj *self); /* +0x008, TaskCoreObj__TaskCoreObj's forward target
                                              (gIntermediateBaseMethods+0x008 = IntermediateBase__IntermediateBase) */
    void (*slot0C)(StreamTaskObj *self); /* +0x00C, TaskCoreObj__Destroy's forward target
                                              (gIntermediateBaseMethods+0x00C = BasicClass__func_17f2c) */
    u8 pad10[0x044 - 0x010];
    void (*slot44)(StreamTaskObj *self, s32 a1, s32 a2); /* +0x044 */
};

extern TaskUtilMethods *Get_vtable_IntermediateBase(void); /* returns &gIntermediateBaseMethods */

/* Allocates/initializes self->unkB4 (a StreamTaskUnkB4Obj); called by
 * StreamTaskObj__StreamTaskObj as `func_80045438(GetDefaultStreamTaskInitData(), 0, 0)`. Not this unit's
 * own function (no INCLUDE_ASM here), so only the call site's own argument
 * and return types are modeled. */
extern StreamTaskUnkB4Obj *func_80045438(StreamTaskInitData *a0, s32 a1, s32 a2);

/* Four more externs reached only by TaskCoreObj__TaskCoreObj's own tail, none of them
 * in this unit. Types are the call sites' own register usage only.
 *
 * New_VabStreamObj/func_80044F30/func_80044CD4's return types were originally
 * guessed `s32` (no counter-evidence at the time). TaskCoreObj__Destroy (round 2)
 * dereferences StreamTaskObj::unk48/unk7C/unk80 -- all three fed directly by
 * these calls -- as `methods->slot04(...)` objects, which is impossible for
 * a plain integer. Retyped to StreamTaskUnkB4Obj* here; this changes no
 * bytes in TaskCoreObj__TaskCoreObj (same register width, pure pointer/int relabeling)
 * but corrects the semantics -- see TaskCoreObj__Destroy's report and
 * TaskCoreObj__TaskCoreObj's report addendum. */
extern StreamTaskUnkB4Obj *New_VabStreamObj(s32 a0);
extern StreamTaskUnkB4Obj *func_80044F30(s32 a0);
extern StreamTaskUnkB4Obj *func_80044CD4(s32 a0, StreamTaskUnkB4Obj *a1);
extern StreamTaskUnk78Obj *func_800441B4(StreamTaskUnkB4Obj *a0, s32 a1); /* return type
                                        StreamTaskUnk78Obj* (feeds self->unk78, un-unified
                                        from StreamTaskUnkB4Obj this round -- see that
                                        struct's comment) */

#endif
