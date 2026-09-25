#ifndef CODE_2C054_H
#define CODE_2C054_H

#include "common.h"
#include "IntermediateBase.h"

/*
 * code_2c054: StreamTaskObj, the streaming-task class (`New_StreamTaskObj`,
 * 0xDC bytes) used to load and drive named "ETC\*.STR" stream files, plus
 * its own base class TaskCoreObj (`New_TaskCore`, 0xA4 bytes). Every
 * function in this unit is fully matched, all 28 named this round (track 3
 * naming pass); there are no `INCLUDE_ASM` bodies left to carve here.
 *
 * The class hierarchy, confirmed by `classtable.py` comparing StreamTaskObj's
 * own vtable (`gStreamTaskObjMethods`) against TaskCoreObj's
 * (`gTaskCoreMethods`) slot-for-slot: every slot StreamTaskObj does NOT
 * override still points at the exact same function in both tables, and every
 * slot it DOES override (008/00C/040/044/04C/05C/060/06C/078/080/084/088/
 * 08C/094) is a real StreamTaskObj-level override that explicitly up-calls
 * the base implementation at the same slot number where it needs to (the
 * `Get_vtable_TaskCore()->slotXX(self)` calls throughout this unit) -- an
 * ordinary override-and-call-super pattern, not delegation between unrelated
 * siblings as an earlier round's comments described it. TaskCoreObj is in
 * turn built the same way over `IntermediateBase` (its parent class,
 * include/IntermediateBase.h, reached via `Get_vtable_IntermediateBase()`).
 *
 * `include/Class6D3C8.h` independently names the SAME StreamTaskObj/TaskCoreObj
 * tables `StreamTask`/`LoaderTask` from a different unit's call sites
 * (`code_1677c`) -- deliberately kept as two separate local views rather
 * than unified, per the convention `include/Entity.h` documents for
 * `BasicClassMethods` vs. `code_55dd4.h`'s `Class65650Methods`. Only the
 * vtable slots and object fields this unit's own functions actually touch
 * are given concrete types here.
 */
typedef struct StreamTaskObj StreamTaskObj;
typedef struct StreamTaskObjMethods StreamTaskObjMethods;
typedef struct StreamTaskUnkB4Obj StreamTaskUnkB4Obj;
typedef struct StreamTaskUnkB4Methods StreamTaskUnkB4Methods;
typedef struct StreamTaskUnk78Obj StreamTaskUnk78Obj;
typedef struct StreamTaskUnk78Methods StreamTaskUnk78Methods;
typedef struct StreamTaskUnkCObj StreamTaskUnkCObj;
typedef struct StreamTaskUnk18Obj StreamTaskUnk18Obj;
typedef struct StreamTaskUnk18Methods StreamTaskUnk18Methods;
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
                                              (per tools/classtable.py gStreamTaskObjMethods) --
                                              confirms the single-argument signature */
    u8 pad44[0x060 - 0x044];
    void (*slot60)(StreamTaskObj *self, s32 a1); /* +0x060, StreamTaskObj__func_8003BC14 occupies this slot
                                                      (per tools/classtable.py gStreamTaskObjMethods) */
    u8 pad64[0x06C - 0x064];
    void (*slot6C)(StreamTaskObj *self, s32 a1); /* +0x06C, StreamTaskObj__SetUnk40 occupies this slot
                                                      (per tools/classtable.py gStreamTaskObjMethods);
                                                      StreamTaskObj__func_8003BAB4 dispatches through it rather
                                                      than calling StreamTaskObj__SetUnk40 directly */
    u8 pad70[0x094 - 0x070];
    void (*slot94)(StreamTaskObj *self); /* +0x094, StreamTaskObj__func_8003BC14's forward target;
                                                gStreamTaskObjMethods+0x094 = StreamTaskObj__func_8003BDF4, this unit's
                                                own already-matched function (single-arg
                                                signature confirms the arity here) */
    u8 pad98[0x09C - 0x098];
    void (*slot9C)(StreamTaskObj *self, s32 a1); /* +0x09C, TaskCore__Reset's forward target
                                                      (gStreamTaskObjMethods+0x09C = TaskCore__SetFadeCallbackEnabled) */
    void (*slotA0)(StreamTaskObj *self, s32 a1); /* +0x0A0, TaskCore__Reset's forward target
                                                      (gStreamTaskObjMethods+0x0A0 = TaskCore__SetFadeOutCallbackEnabled) */
    void (*slotA4)(StreamTaskObj *self, u8 *a1, u8 *a2, u8 *a3); /* +0x0A4, TaskCore__Reset's
                                                      forward target (gStreamTaskObjMethods+0x0A4 = TaskCore__SetColors) */
    u8 padA8[0x0D4 - 0x0A8];
    void (*slotD4)(StreamTaskObj *self, s32 a1, s32 a2); /* +0x0D4, TaskCore__TaskCore's forward
                                                      target (gStreamTaskObjMethods+0x0D4 = TaskCore__SetSubHandle) */
    u8 padD8[0x0DC - 0x0D8];
    void (*slotDC)(StreamTaskObj *self); /* +0x0DC, TaskCore__Finalize's forward target
                                                      (gStreamTaskObjMethods+0x0DC = TaskCore__ReleaseTarget) */
    void (*slotE0)(StreamTaskObj *self, s32 a1); /* +0x0E0, TaskCore__OnInit's forward target */
    void (*slotE4)(StreamTaskObj *self, u8 *a1); /* +0x0E4, TaskCore__OnInit's forward target;
                                                      a1 is `&self->unk90`, an address into self */
};

/* Object size is 0xDC (from New_StreamTaskObj's allocator call). Only the
 * fields this unit's queued functions touch are named. */
struct StreamTaskObj {
    StreamTaskObjMethods *methods; /* +0x000 */
    u8 pad04[0x00C - 0x004];
    StreamTaskUnkCObj *unkC;       /* +0x00C, a pointer to a 1-word holder whose own
                                        word 0 is the actual dispatch target (see
                                        StreamTaskUnkCObj below); read by TaskCore__OnDeinit */
    u8 pad10[0x014 - 0x010];
    s32 unk14;                      /* +0x014, an argument forwarded to self->methods'
                                        slotE0 and (reloaded) unk78's slot4C by
                                        TaskCore__OnInit */
    StreamTaskUnk18Obj *unk18;      /* +0x018, an object whose vtable is
                                        StreamTaskUnk18Methods -- NOT the same struct as
                                        TaskCoreMethods (see StreamTaskUnk18Obj below for
                                        the correction). Renamed off the collision with
                                        `TaskCoreObj` this round: that name is now reserved
                                        for New_TaskCore's own 0xA4-byte allocation
                                        (whose real vtable is TaskCoreMethods/gTaskCoreMethods,
                                        confirmed by TaskCore__TaskCore's own
                                        `self->methods = (StreamTaskObjMethods *)core;`)
                                        -- a genuinely different class that happened to
                                        share the same struct name. */
    u8 pad1C[0x028 - 0x01C];
    s32 unk28;                        /* +0x028, set to 3 by TaskCore__Reset */
    s32 unk2C;                         /* +0x02C, set to 0x12C by TaskCore__Reset */
    s32 unk30;                          /* +0x030, set to 0x40 by TaskCore__Reset */
    s32 unk34;                       /* +0x034, tested by TaskCore__OnDeinit, set to 1 by
                                          TaskCore__Reset */
    s32 unk38;                     /* +0x038, read (and returned) by TaskCore__Init */
    s32 unk3C;                     /* +0x03C, set to 0 by TaskCore__Reset */
    s32 unk40;                     /* +0x040, set by StreamTaskObj__SetUnk40 */
    s32 unk44;                       /* +0x044, set from TaskCore__TaskCore's arg2 */
    StreamTaskUnkB4Obj *unk48;        /* +0x048, set by TaskCore__TaskCore: either a call
                                           result (arg2 nonzero) or arg3 verbatim;
                                           torn down by TaskCore__Finalize's slot04 (retyped
                                           from a wrong s32 guess -- see that report) */
    u8 pad4C[0x070 - 0x04C];
    s32 unk70;                          /* +0x070, tested by TaskCore__Finalize */
    StreamTaskUnkB4Obj *unk74;           /* +0x074, torn down by TaskCore__Finalize's slot04 */
    StreamTaskUnk78Obj *unk78;       /* +0x078, its OWN class again (see StreamTaskUnk78Obj
                                          below) -- UN-unified from StreamTaskUnkB4Obj this
                                          same round, see TaskCore__OnInit's report: its slot
                                          +0x04C needs 3 args here where StreamTaskUnkB4Obj's
                                          own slot +0x04C (StreamTaskObj__func_8003BDF4) needs 1; the two
                                          classes only ever agreed at slot04 */
    StreamTaskUnkB4Obj *unk7C;        /* +0x07C, set by TaskCore__TaskCore from a call result;
                                           torn down by TaskCore__Finalize's slot04 (retyped
                                           from a wrong s32 guess -- see that report) */
    StreamTaskUnkB4Obj *unk80;         /* +0x080, set by TaskCore__TaskCore from a call result;
                                           torn down by TaskCore__Finalize's slot04 (retyped
                                           from a wrong s32 guess -- see that report) */
    s32 unk84;                       /* +0x084, set to 9 by TaskCore__Reset */
    s32 unk88;                        /* +0x088, tested by TaskCore__OnInit (guards a block) */
    u8 pad8C[0x090 - 0x08C];
    u8 unk90;                          /* +0x090, only ever address-taken (a buffer start,
                                            passed to TaskCore__OnInit's slotE4/slotB8 calls);
                                            real extent beyond one byte unknown -- unk93,
                                            3 bytes further into the SAME buffer, is the
                                            other known address-taken offset in it */
    u8 pad91[0x093 - 0x091];
    s8 unk93;                          /* +0x093, only ever address-taken (a buffer
                                            passed to TaskCore__OnDeinit's slot78 call);
                                            real extent beyond one byte unknown */
    u8 pad94[0x09C - 0x094];
    s32 unk9C;                          /* +0x09C, set to 0 by TaskCore__Reset */
    s32 unkA0;                           /* +0x0A0, set to 0 by TaskCore__Reset */
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
 * Entity.h) -- returns &gStreamTaskObjMethods with no other side effect. Called by
 * New_StreamTaskObj/StreamTaskObj__StreamTaskObj (both still INCLUDE_ASM, not this batch) to
 * fetch the ctor at slot +0x008. */
extern StreamTaskObjMethods *Get_vtable_StreamTaskObj(void);
extern StreamTaskObjMethods gStreamTaskObjMethods;

/* A rodata table TaskCore__Reset reaches only by ADDRESS (`lui`/`addiu`, no
 * `lw`/`sw` here) -- passed to slotA4 as three pointers 3 bytes apart
 * (base, base+3, base+6). Never decoded further by this unit's queued
 * functions, so typed as a plain byte array. */
extern u8 D_8006E860[];

/* Two more rodata symbols reached only by address (round 2, TaskCore__OnInit):
 * `gDefaultStreamTaskInitData`, passed as `TaskTextMethods::slot78`'s 3rd argument, and
 * `D_8006E86C`, passed TWICE (same address) as `StreamTaskUnk18Methods::slot70`'s
 * 3rd AND 4th arguments. Neither is ever dereferenced by this unit's queued
 * functions. */
extern u8 gDefaultStreamTaskInitData[];
extern u8 D_8006E86C[];

/* self->unkB4's own tiny class: a 1-slot vtable, dispatched through by
 * StreamTaskObj__Destroy as `self->unkB4->methods->slot04(self->unkB4)` (this
 * unit's only reader). Real shape beyond that one slot is unknown.
 *
 * REUSED for StreamTaskObj::unk48/unk74/unk7C/unk80 too (round 2,
 * TaskCore__Finalize): all four fields are torn down identically --
 * `field->methods->slot04(field)` -- from the SAME function, in sequence.
 *
 * unk78 was ALSO folded in here for one round (TaskCore__Finalize), then
 * UN-folded back into its own `StreamTaskUnk78Obj/Methods` type by
 * TaskCore__OnInit: that function calls unk78's own slot +0x04C with 3
 * arguments, where THIS struct's slot +0x04C (StreamTaskObj__func_8003BDF4's already-
 * matched, byte-exact call on unkB4) needs exactly 1. Since a vtable slot's
 * signature must be consistent for every instance of the SAME class, a real
 * arity conflict at a shared offset is proof the two are sibling classes
 * (agreeing only on slot04, likely via a common base), not one class --
 * unlike unk48/unk74/unk7C/unk80, which no function has yet called through
 * any slot besides 04 and so have not (yet) produced any such conflict. See
 * TaskCore__OnInit's report for the discovery and TaskCore__Finalize's report for
 * an addendum recording the reversal. */
struct StreamTaskUnkB4Methods {
    u8 pad00[0x04];
    void (*slot04)(StreamTaskUnkB4Obj *self); /* +0x004, also TaskCore__Finalize's forward target
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
 * slots now, all from TaskCore__OnInit (slot50 originally from TaskCore__OnDeinit). */
struct StreamTaskUnk78Methods {
    u8 pad00[0x04];
    void (*slot04)(StreamTaskUnk78Obj *self); /* +0x004, TaskCore__Finalize's forward target */
    u8 pad08[0x04C - 0x008];
    void (*slot4C)(StreamTaskUnk78Obj *self, s32 a1, s32 a2); /* +0x04C, TaskCore__OnInit's
                                        forward target -- 3 arguments, see above */
    void (*slot50)(StreamTaskUnk78Obj *self); /* +0x050, TaskCore__OnDeinit's forward target */
    u8 pad54[0x0B8 - 0x054];
    void (*slotB8)(StreamTaskUnk78Obj *self, s32 a1, u8 *a2); /* +0x0B8, TaskCore__OnInit's
                                        forward target */
};

struct StreamTaskUnk78Obj {
    StreamTaskUnk78Methods *methods; /* +0x000 */
};

/* self->unkC points at a 1-word holder (not a class instance itself -- its
 * one word is read but its own vtable, if it has one, is never reached).
 * That word is the real dispatch target, `TaskTextObj`: another generic
 * "vtable at offset 0" object, reached by TaskCore__OnDeinit through one slot
 * (+0x078) that takes a buffer pointer and a flag -- resembles a
 * print-into-buffer call, but nothing here confirms that beyond the shape
 * of the call (self->unk93's address is the second argument). */
struct StreamTaskUnkCObj {
    TaskTextObj *unk0; /* +0x000 */
};

struct TaskTextMethods {
    u8 pad00[0x078];
    void (*slot78)(TaskTextObj *self, s8 *buf, u8 *flag); /* +0x078; 3rd param widened from
                                        `s32` to a pointer here (round 2, TaskCore__OnInit):
                                        TaskCore__OnDeinit's call passes literal 0 (valid for
                                        either type, no byte change there), but
                                        TaskCore__OnInit's own call to the same slot passes
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
 * (established from New_TaskCore's ctor call, a DIFFERENT allocator/unit)
 * -- this is this unit's own independent local view of the same table, kept
 * separate rather than editing that header (same policy as
 * StreamTaskObjMethods above). Slot +0x044 is `Class6D3C8.h`'s own
 * `LoaderTaskMethods::slot44`, typed void there from ITS call sites (a
 * different, discarding, caller); this unit's TaskCore__Init -- the function
 * that actually OCCUPIES that slot -- demonstrably reads and returns
 * `self->unk38` right after the call in its own disassembly, so it is typed
 * `s32` here instead. Both typings are compatible at the ABI level (a
 * discarding caller through a void-typed pointer simply never reads $v0);
 * flagged for the head to reconcile, see this unit's match reports. */
typedef struct TaskCoreMethods TaskCoreMethods;

struct TaskCoreMethods {
    u8 pad00[0x008];
    void (*slot08)(StreamTaskObj *self, s32 a1, s32 a2, s32 a3); /* +0x008, StreamTaskObj__StreamTaskObj's
                                              forward target (gTaskCoreMethods+0x008 = TaskCore__TaskCore,
                                              this unit's own queued function -- confirms arity) */
    void (*slot0C)(StreamTaskObj *self);                    /* +0x00C, StreamTaskObj__Destroy's 2nd call */
    u8 pad10[0x044 - 0x010];
    s32 (*slot44)(StreamTaskObj *self, s32 a1, s32 a2);       /* +0x044, TaskCore__Init occupies this slot */
    u8 pad48[0x04C - 0x048];
    void (*slot4C)(StreamTaskObj *self);                       /* +0x04C, StreamTaskObj__func_8003BAB4's forward target
                                                                     (gTaskCoreMethods+0x04C = TaskCore__OnInit) */
    u8 pad50[0x05C - 0x050];
    void (*slot5C)(StreamTaskObj *self, s32 a1, s32 a2);       /* +0x05C, StreamTaskObj__func_8003BB5C's forward target
                                                                     (gTaskCoreMethods+0x05C = TaskCore__Update) */
    void (*slot60)(StreamTaskObj *self, s32 a1);                /* +0x060, StreamTaskObj__func_8003BC14's forward target
                                                                     (gTaskCoreMethods+0x060 = TaskCore__SetState) */
    u8 pad64[0x078 - 0x064];
    void (*slot78)(StreamTaskObj *self);                        /* +0x078, StreamTaskObj__func_8003BD10's forward target
                                                                      (gTaskCoreMethods+0x078 = TaskCore__OnPadConfirm) */
    u8 pad7C[0x080 - 0x07C];
    void (*slot80)(StreamTaskObj *self);                        /* +0x080, StreamTaskObj__func_8003BD74's forward target */
    void (*slot84)(StreamTaskObj *self);                         /* +0x084, StreamTaskObj__func_8003BDAC's forward target */
    u8 pad88[0x0D8 - 0x088];
    void (*slotD8)(StreamTaskObj *self, s32 a1);                    /* +0x0D8, TaskCore__TaskCore's forward
                                                                        target -- called on `self` itself, not
                                                                        an unk18-style sub-instance, right after
                                                                        self->methods is (temporarily) set to
                                                                        this table's own pointer: a base-class
                                                                        constructor chaining pattern, see that
                                                                        function's report
                                                                        (gTaskCoreMethods+0x0D8 = TaskCore__SetTarget) */
};

extern TaskCoreMethods *Get_vtable_TaskCore(void); /* returns &gTaskCoreMethods */

/* `StreamTaskObj::unk18` and its vtable, `StreamTaskUnk18Methods`.
 *
 * ORIGINALLY modeled (TaskCore__OnDeinit, round 1) as sharing `TaskCoreMethods`
 * itself -- self->unk18's own slots +0x074/+0x090, called there, happened to
 * line up with non-null entries in gTaskCoreMethods's OWN table at those same
 * offsets (`classtable.py gTaskCoreMethods`). That agreement turned out to be
 * coincidence, not evidence of a shared class: TaskCore__OnInit (round 2) calls
 * THREE MORE slots on self->unk18 (+0x048/+0x04C/+0x050, all 2-argument
 * setters) that cannot exist on `TaskCoreMethods` -- `gTaskCoreMethods`'s own
 * +0x04C is TaskCore__OnInit's own occupant slot (this very function!),
 * confirmed single-argument by StreamTaskObj__func_8003BAB4's byte-exact call. A vtable
 * slot's arity must be consistent for every instance of the SAME class, so
 * a genuine arity conflict at a shared offset means self->unk18 is a
 * DIFFERENT class that merely happens to also be a `BasicClass` descendant
 * (hence the shared low slots every table in this game has) -- not an
 * instance of gTaskCoreMethods's own class. Split into its own type,
 * originally spelled `TaskCoreObjMethods`; see TaskCore__OnInit's
 * report for the original discovery.
 *
 * RENAMED to `StreamTaskUnk18Obj`/`StreamTaskUnk18Methods` this round (track
 * 3, naming pass): the name `TaskCoreObj` was doing double duty for two
 * unrelated classes -- this one (self->unk18, vtable split from
 * `TaskCoreMethods` right here) and New_TaskCore's own 0xA4-byte
 * allocation (whose real vtable is `TaskCoreMethods`/`gTaskCoreMethods`
 * itself, confirmed by TaskCore__TaskCore's own
 * `self->methods = (StreamTaskObjMethods *)core;`, `core` being
 * `Get_vtable_TaskCore()`). Nothing dereferenced `struct TaskCoreObj`'s body
 * except through self->unk18, so freeing the name costs nothing: `TaskCoreObj`
 * stays reserved for New_TaskCore's own class (now a plain opaque pointer
 * type below, since nothing needs its body), and this sub-object gets the
 * same `StreamTaskUnkNNObj` spelling already used for the other private
 * sub-objects on this class (`StreamTaskUnkB4Obj`, `StreamTaskUnk78Obj`,
 * `StreamTaskUnkCObj`). No compiled byte changes -- pure type renaming. */

struct StreamTaskUnk18Methods {
    u8 pad00[0x048];
    void (*slot48)(StreamTaskUnk18Obj *self, s32 a1); /* +0x048, TaskCore__OnInit's forward target */
    void (*slot4C)(StreamTaskUnk18Obj *self, s32 a1); /* +0x04C, TaskCore__OnInit's forward target */
    void (*slot50)(StreamTaskUnk18Obj *self, s32 a1); /* +0x050, TaskCore__OnInit's forward target */
    u8 pad54[0x070 - 0x054];
    void (*slot70)(StreamTaskUnk18Obj *self, s32 a1, u8 *a2, u8 *a3, s32 a4); /* +0x070,
                                        TaskCore__OnInit's forward target; a2 and a3 are
                                        the SAME address (&D_8006E86C) at that call site */
    void (*slot74)(StreamTaskUnk18Obj *self); /* +0x074, TaskCore__OnDeinit's forward target */
    u8 pad78[0x08C - 0x078];
    void (*slot8C)(StreamTaskUnk18Obj *self); /* +0x08C, TaskCore__OnInit's forward target */
    void (*slot90)(StreamTaskUnk18Obj *self); /* +0x090, TaskCore__OnDeinit's forward target */
};

struct StreamTaskUnk18Obj {
    StreamTaskUnk18Methods *methods; /* +0x000 */
};

/* New_TaskCore's own class: opaque here on purpose. Its ctor
 * (TaskCore__TaskCore) points its `methods` field at
 * `Get_vtable_TaskCore()` (`TaskCoreMethods *`, cast from
 * `StreamTaskObj *`, see that function's report), and nothing in this
 * unit ever dereferences a bare `TaskCoreObj *` -- New_TaskCore
 * returns it untouched. A real `struct TaskCoreObj { TaskCoreMethods
 * *methods; }` body would just restate that cast, so it is left as a
 * pointer-only type instead of asserting a body nothing here checks. */

/* TaskCore's parent, IntermediateBase (gIntermediateBaseMethods):
 * include/IntermediateBase.h (track 4). */

/* Allocates/initializes self->unkB4 (a StreamTaskUnkB4Obj); called by
 * StreamTaskObj__StreamTaskObj as `New_MoviePlayer(GetDefaultStreamTaskInitData(), 0, 0)`. Not this unit's
 * own function (no INCLUDE_ASM here), so only the call site's own argument
 * and return types are modeled. */
extern StreamTaskUnkB4Obj *New_MoviePlayer(StreamTaskInitData *a0, s32 a1, s32 a2);

/* Four more externs reached only by TaskCore__TaskCore's own tail, none of them
 * in this unit. Types are the call sites' own register usage only.
 *
 * New_VabStreamObj/New_TileAtlas/New_TileMap's return types were originally
 * guessed `s32` (no counter-evidence at the time). TaskCore__Finalize (round 2)
 * dereferences StreamTaskObj::unk48/unk7C/unk80 -- all three fed directly by
 * these calls -- as `methods->slot04(...)` objects, which is impossible for
 * a plain integer. Retyped to StreamTaskUnkB4Obj* here; this changes no
 * bytes in TaskCore__TaskCore (same register width, pure pointer/int relabeling)
 * but corrects the semantics -- see TaskCore__Finalize's report and
 * TaskCore__TaskCore's report addendum. */
extern StreamTaskUnkB4Obj *New_VabStreamObj(s32 a0);
extern StreamTaskUnkB4Obj *New_TileAtlas(s32 a0);
extern StreamTaskUnkB4Obj *New_TileMap(s32 a0, StreamTaskUnkB4Obj *a1);
extern StreamTaskUnk78Obj *New_BgLayer(StreamTaskUnkB4Obj *a0, s32 a1); /* return type
                                        StreamTaskUnk78Obj* (feeds self->unk78, un-unified
                                        from StreamTaskUnkB4Obj this round -- see that
                                        struct's comment) */

#endif
