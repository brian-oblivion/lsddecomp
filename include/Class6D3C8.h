#ifndef CLASS_6D3C8_H
#define CLASS_6D3C8_H

#include "common.h"
#include "DreamSys.h"

/*
 * The class allocated by New_Class6D3C8 / constructed by Class6D3C8__Class6D3C8.
 * Method table is D_8006D3C8 (25 slots). No FirecatFG name survives for
 * this class (only New_Class6D3C8 itself is named in the symbol file), so
 * fields are named by offset until real names are known.
 *
 * Inheritance, resolved with tools/classtable.py (never by counting):
 * BasicClass (D_8006B58C, 14 slots) -> an intermediate class at D_8006E4F0
 * (19 slots: overrides BasicClass's ctor/dtor at +0x008/+0x00C, adds four
 * slots at +0x040..+0x04C) -> this class (D_8006D3C8, 25 slots: keeps the
 * intermediate class's dtor override and its +0x048/+0x04C slots verbatim,
 * overrides +0x008 (own ctor, Class6D3C8__Class6D3C8), +0x040 and +0x044, and adds
 * six more of its own from +0x050). Confirmed by `classtable.py 0x8006D3C8
 * --vs 0x8006E4F0` sharing +0x00C/+0x048/+0x04C exactly where the earlier
 * `--vs 0x8006B58C` comparison did not, and by Class6D3C8__Class6D3C8 itself calling
 * the intermediate class's ctor slot (via func_8003B20C -> &D_8006E4F0)
 * before installing its own vtable — the base-constructor-through-slot+8
 * shape from docs/research/class-framework.md.
 *
 * Only the slots this unit's functions actually call through are given
 * concrete field types; the rest stay opaque `void *` so the struct keeps
 * the right size/offsets without requiring every method to be typed up
 * front.
 *
 * What the unit IS (round 77 naming pass): a startup/title-sequence
 * controller. The ctor (`Class6D3C8__Class6D3C8`) loads one 3D model
 * ("ETC\DREAME5.TMD") and builds this object's owned `DreamSys` from it;
 * every other vtable slot is a `Class6D3C8CtorArgs`-gated block that either
 * loads a named boot-time asset (the ASMK/OSD logo `LoaderTask`s and their
 * bracketing stream, `Class6D3C8__LoadIntroLogoSequence`), starts a
 * StreamTask keyed to a day/week or "GraphRoom"-poll-derived index
 * (`Class6D3C8__StartWeeklyStreamTask`, `Class6D3C8__StartGraphRoomStreamTask`,
 * `Class6D3C8__StartStreamTaskWithInit`), polls a "GraphRoom"-named PollTask
 * pair against the owned `DreamSys`'s status (`Class6D3C8__PollGraphRoomStatus`),
 * or resolves and streams the current cinematic
 * (`Class6D3C8__StartCinematicStream`, reached via
 * `Class6D3C8__PollStatusObj`'s status-code dispatch). All 13 of this
 * unit's own functions are named (tier A/B, see each one's match report's
 * `## Naming` section); no game-purpose name for the CLASS itself is
 * established yet (that is track 4's job, once a caller elsewhere names an
 * instance's role) -- `Class6D3C8` is kept as the table-address identity.
 */

/* The intermediate base class at D_8006E4F0. Same policy: only slots this
 * unit actually dispatches through (+0x044, from Class6D3C8__ForwardToBaseSlot44UnlessFlagged) are typed. */
typedef struct MiddleClassMethods {
    s32 header;                                             /* +0x000 */
    void *unk04;                                             /* +0x004 */
    void (*ctor)(void *self, s32 a1);                         /* +0x008 Class6E4F0__Class6E4F0 */
    void *unk0C;                                               /* +0x00C Class6E4F0__Finalize (dtor override, shared with Class6D3C8) */
    void *unk10, *unk14, *unk18, *unk1C, *unk20, *unk24, *unk28, *unk2C, *unk30, *unk34, *unk38; /* BasicClass, inherited */
    void *unk3C;                                                /* +0x03C null slot */
    void *unk40;                                                 /* +0x040 Class6E4F0__SetScreenDims */
    s32 (*slot44)(void *self, void *a1, void *a2, s32 a3);         /* +0x044 Class6E4F0__InitSystems */
    void *unk48;                                                    /* +0x048 Class6E4F0__NoOpSlot48, shared with Class6D3C8 */
    void *unk4C;                                                     /* +0x04C func_8003B110, shared with Class6D3C8 */
} MiddleClassMethods;

extern MiddleClassMethods *func_8003B20C(void);

/* The constructor argument block for Class6D3C8 (Class6D3C8__Class6D3C8). Observed
 * from its one call site (asm/main.s, gClass6D3C8CtorArgs: {0x13, 0, 1, 1, 1, 1}) --
 * only offsets +0x00 and +0x14 are actually read by Class6D3C8__Class6D3C8, so the
 * rest stays opaque padding until another caller needs it. */
typedef struct Class6D3C8CtorArgs {
    s32 unk00;                 /* +0x00, passed as the base ctor's own arg */
    s32 unk04;                 /* +0x04, forwarded as a plain word arg by Class6D3C8__PollStatusObj */
    void *unk08;                  /* +0x08, gates Class6D3C8__StartWeeklyStreamTask's whole body */
    void *unk0C;                    /* +0x0C, gates Class6D3C8__LoadIntroLogoSequence's whole body */
    void *unk10;                       /* +0x10, gates Class6D3C8__PollGraphRoomStatus's whole body */
    s32 unk14;                          /* +0x14, passed into the DreamSys call */
} Class6D3C8CtorArgs;

typedef struct Class6D3C8 Class6D3C8;

typedef struct Class6D3C8Methods {
    s32 header;                                            /* +0x000 */
    void *unk04;                                            /* +0x004 BasicClass__Release */
    void (*ctor)(Class6D3C8 *self, Class6D3C8CtorArgs *arg); /* +0x008 Class6D3C8__Class6D3C8 */
    void *unk0C;                                            /* +0x00C Class6E4F0__Finalize (dtor override) */
    void *unk10;                                            /* +0x010 BasicClass__AddChild */
    void *unk14;                                            /* +0x014 BasicClass__RemoveChild */
    void *unk18;                                            /* +0x018 BasicClass__RemoveAllChildren */
    void *unk1C;                                            /* +0x01C BasicClass__GetNextChild */
    void *unk20;                                            /* +0x020 BasicClass__AddParentRef */
    void *unk24;                                            /* +0x024 BasicClass__RemoveParentRef */
    void *unk28;                                            /* +0x028 BasicClass__ClearParentRefs */
    void *unk2C;                                            /* +0x02C BasicClass__GetNextParentRef */
    void *unk30;                                            /* +0x030 BasicClass__NotifyParents */
    void *unk34;                                            /* +0x034 BasicClass__func_18350 */
    void *unk38;                                            /* +0x038 BasicClass__OnNotify */
    void *unk3C;                                            /* +0x03C null slot */
    void (*setDayFromTickCount)(Class6D3C8 *self);          /* +0x040 Class6D3C8__SetDayFromTickCount (ignores self) */
    void (*forwardToBaseSlot44UnlessFlagged)(Class6D3C8 *self, void *a1, void *a2);  /* +0x044 Class6D3C8__ForwardToBaseSlot44UnlessFlagged.
                                                             * NOT renamed to match its occupant: src/main.c
                                                             * dispatches this slot by name directly
                                                             * (gClass6D3C8->methods->slot44(...)), outside this
                                                             * unit's ownership -- see the header's top comment
                                                             * and the round 77 proposed-field-names note. */
    void *unk48;                                            /* +0x048 Class6E4F0__NoOpSlot48 */
    void (*slot4C)(Class6D3C8 *self);                       /* +0x04C func_8003B110, first dispatched by main
                                                             * (src/main.c) -- same cross-unit-slot44 reason, not renamed. */
    void (*loadIntroLogoSequence)(Class6D3C8 *self);        /* +0x050 Class6D3C8__LoadIntroLogoSequence */
    void (*startWeeklyStreamTask)(Class6D3C8 *self);        /* +0x054 Class6D3C8__StartWeeklyStreamTask */
    s32 (*pollGraphRoomStatus)(Class6D3C8 *self);           /* +0x058 Class6D3C8__PollGraphRoomStatus */
    void (*noOpSlot5C)(void);                               /* +0x05C Class6D3C8__NoOpSlot5C */
    void (*pollStatusObj)(Class6D3C8 *self);                /* +0x060 Class6D3C8__PollStatusObj */
    void (*startStreamTaskWithInit)(Class6D3C8 *self);      /* +0x064 Class6D3C8__StartStreamTaskWithInit.
                                                             * Retyped from opaque `void *` to a real callable
                                                             * signature: no carved caller dispatches this slot
                                                             * yet, but its occupant's own signature (this unit,
                                                             * matched) is known, so the opaque placeholder was
                                                             * never load-bearing. */
} Class6D3C8Methods;

/* Object size is 0x2C (from the allocator call in New_Class6D3C8). Field
 * offsets below are only the ones observed so far in Class6D3C8__Class6D3C8. */
struct Class6D3C8 {
    Class6D3C8Methods *methods;    /* +0x00 */
    u8 unk04[0x14];                 /* +0x04 .. +0x17, not yet decoded */
    s32 unk18;                       /* +0x18 guards Class6D3C8__ForwardToBaseSlot44UnlessFlagged's fallback to the base class */
    s32 unk1C;                        /* +0x1C, forwarded as a plain word arg by Class6D3C8__LoadIntroLogoSequence/Class6D3C8__StartLoaderTask */
    Class6D3C8CtorArgs *arg;        /* +0x20 the constructor's `arg` parameter */
    s32 unk24;                      /* +0x24 */
    DreamSys *dreamSys;              /* +0x28 result of New_DreamSys() */
};

extern Class6D3C8Methods D_8006D3C8;

/* Cross-unit accessor, matched C in src/code_171e0.c (not this unit). No
 * header currently declares it there, so it's declared here at the one
 * call site that needs it (Class6D3C8__Class6D3C8). Returns &D_8006D3C8. */
extern void *GetClass6D3C8Methods(void);

/* The game's allocator, in the uncarved code_8220 block. Returns void *
 * rather than a typed pointer because every New_X in the game calls it. */
extern void *BMemPMgrAlloc(s32 size);

/* Model-file-load request block used by Class6D3C8__Class6D3C8: {type; path}. Only
 * one call site is known so far (Class6D3C8__Class6D3C8, loading "ETC\DREAME5.TMD"
 * via sModelPathDreamE5), so field names are provisional. */
typedef struct LoadModelRequest {
    s32 type;
    const char *path;
    s32 unk08;
    s32 unk0C;
} LoadModelRequest;

extern const char sModelPathDreamE5[];       /* "ETC\DREAME5.TMD", asm/data/FA4.rodata.s */

extern s32 func_80048CF0(void);        /* reads a small-data global, unnamed so far */
extern void func_800270AC(s32 value);   /* stores its arg to a small-data global */
extern void *func_80043840(void *arg); /* code_1677c's own alloc+ctor shape, uncarved (psyq_memset.s); not this unit's to write */

/* A "New_X"-shaped task object allocated by New_StreamTaskObj -- 0xDC bytes,
 * constructed through Get_vtable_StreamTaskObj's slot +0x008. `New_StreamTaskObj`
 * itself is matched, byte-exact, as `StreamTaskObj` in src/code_2c054.c (same
 * function name, same allocator, same 0xDC size) -- this typedef stays a
 * separate LOCAL view rather than including that unit's header, per this
 * project's multiple-independent-local-views convention (only the slots this
 * unit actually dispatches through are typed here). Only the four slots
 * Class6D3C8__LoadIntroLogoSequence and its siblings dispatch through are
 * typed; everything else about this class is unknown. */
typedef struct StreamTaskMethods {
    s32 header;                                                    /* +0x000 */
    void (*start)(void *self);                                      /* +0x004: always the LAST call in every
                                                                         use site here, after every other slot
                                                                         (configure/setChannel/etc.) has run --
                                                                         a fire-and-forget "go" trigger. */
    u8 pad08[0x044 - 0x008];                                          /* +0x008 .. +0x043 */
    /* +0x044, named `configure`: called with (self, a fixed word from the
     * caller's own object, a second word whose meaning varies by call site --
     * a filename string in Class6D3C8__LoadIntroLogoSequence, a plain derived
     * count in Class6D3C8__StartWeeklyStreamTask, func_800493E4's return
     * value in Class6D3C8__StartGraphRoomStreamTask -- a type/format code,
     * and a literal 1 spilled onto the stack as a 5th argument -- confirmed a
     * real 5th argument, not a scheduling artifact, because MIPS o32 only
     * spills to the stack once a0-a3 are all otherwise assigned; a <=4-arg
     * call would never need the sp+0x10 store.
     *
     * Tier A, cross-unit evidence: `src/code_2c054.c`'s
     * `StreamTaskObj__Configure(self, a1, arg2, typeLookup, flag)` has this
     * EXACT parameter list, for the exact class this typedef is a local view
     * of (both go through `New_StreamTaskObj`). */
    void (*configure)(void *self, s32 a1, s32 arg2, s32 typeLookup, s32 flag);
    u8 pad48[0x06C - 0x048];                                          /* +0x048 .. +0x06B */
    void (*slot6C)(void *self, s32 a1);                                 /* +0x06C: NOT renamed. Weak correlation
                                                                            only -- `code_2c054.c`'s
                                                                            `StreamTaskObj__SetUnk40(self, a1)` sets
                                                                            self->unk40 = (a1 >= 0) ? a1*15 : a1,
                                                                            and this unit's only two call sites pass
                                                                            either 0 or `buf.count / 15` -- a
                                                                            round-trip that fits, but is not proof
                                                                            of slot alignment across the
                                                                            TaskCore/StreamTaskObj override chain. */
    u8 pad70[0x12C - 0x070];                                              /* +0x070 .. +0x12B */
    void (*slot12C)(void *self, s32 a1);                                    /* +0x12C: both call sites here pass a
                                                                                literal 0 -- not enough evidence for
                                                                                a name. */
} StreamTaskMethods;

typedef struct StreamTask {
    StreamTaskMethods *methods;
} StreamTask;

extern StreamTask *New_StreamTaskObj(s32 a0, s32 a1, s32 a2, s32 a3);

extern s32 SetActiveDataSourceDriverMode(s32 a0, s32 a1, s32 a2); /* code_171e0, still INCLUDE_ASM there; returns
                                                       the last value its internal dispatch loop got --
                                                       Class6D3C8__LoadIntroLogoSequence/Class6D3C8__StartWeeklyStreamTask discard it, but
                                                       Class6D3C8__StartCinematicStream keeps it */
extern const char *func_800490F4(s32 *typeCodeOut);  /* psyq_memset.s: writes 0x31 to *typeCodeOut if non-NULL, always returns &D_800113DC */
extern s32 func_800493C8(s32 index);                   /* psyq_memset.s: signed-halfword lookup into D_80086170[index] */
extern s32 func_8004913C(s32 *out, s32 param2);          /* psyq_memset.s: day/week-style calculation (divides func_80048CFC's result by 7); writes a related index to *out if non-NULL, returns a separate derived value */

extern const char sLogoPathAsmk[]; /* "ETC\ASMKLOGO.TIM" */
extern const char sLogoPathOsd[]; /* "ETC\OSDLOGO.TIM" */

/* Forward declaration: Class6D3C8__StartLoaderTask (this unit, defined later in ROM
 * order) is called by Class6D3C8__LoadIntroLogoSequence, which comes first in the file. */
void Class6D3C8__StartLoaderTask(Class6D3C8 *self, const char *path);

/* A second "New_X"-shaped task object, allocated by New_TaskCoreObj -- 0xA4
 * bytes, constructed through Get_vtable_TaskCore's slot +0x008. `TaskCoreObj`
 * is matched in src/code_2c054.c (a separate real class, its own base under
 * StreamTaskObj in that unit's inheritance chain -- see that file's own
 * `TaskCoreObj__TaskCoreObj`). Different class from StreamTaskMethods above
 * (different allocator, different slot signatures at the same offsets), used
 * by Class6D3C8__StartLoaderTask to register a named resource with a
 * completion callback. */
typedef struct LoaderTaskMethods {
    s32 header;                                              /* +0x000 */
    void (*start)(void *self);                                 /* +0x004, same fire-and-forget "go" trigger
                                                                    shape as StreamTaskMethods::start above --
                                                                    always the last call at this unit's one
                                                                    LoaderTask use site (Class6D3C8__StartLoaderTask). */
    u8 pad08[0x044 - 0x008];                                     /* +0x008 .. +0x043 */
    /* +0x044: RETURNS s32, not void. This slot's occupant is TaskCoreObj__func_8003C1DC
     * (matched in code_2c054), and its own body loads self->unk38 into $v0
     * immediately before the epilogue with nothing else consuming it -- a
     * load whose only purpose is to be the return value. The earlier `void`
     * came from THIS header's caller, which discards the result; that
     * describes what the caller does with the value, not what the callee
     * computes. Harmless at the ABI level either way (a discarding caller
     * simply never reads $v0), which is why correcting it changes zero bytes.
     * Matches PollTaskMethods::slot44 below, as the comment there predicts. */
    s32 (*slot44)(void *self, s32 a1, s32 a2);                     /* +0x044 */
    u8 pad48[0x06C - 0x048];                                         /* +0x048 .. +0x06B */
    void (*slot6C)(void *self, s32 a1);                                /* +0x06C */
    u8 pad70[0x098 - 0x070];                                             /* +0x070 .. +0x097 */
    void (*slot98)(void *self, s32 (*callback)(void), void *ctx);         /* +0x098 */
    u8 pad9C[0x0D4 - 0x09C];                                                /* +0x09C .. +0x0D3 */
    void (*slotD4)(void *self, const char *path, s32 a2);                    /* +0x0D4 */
} LoaderTaskMethods;

typedef struct LoaderTask {
    LoaderTaskMethods *methods;
} LoaderTask;

extern LoaderTask *New_TaskCoreObj(s32 a0, s32 a1, s32 a2);

/* Forward declaration: Class6D3C8__LoaderTaskDoneCallback (this unit, defined right after
 * Class6D3C8__StartLoaderTask in ROM order) is used by Class6D3C8__StartLoaderTask as a completion
 * callback. Already matched: s32 Class6D3C8__LoaderTaskDoneCallback(void) { return
 * func_8004A070(0); } */
s32 Class6D3C8__LoaderTaskDoneCallback(void);

/* A third small class, constructed directly by a caller-supplied function
 * pointer (Class6D3C8__RunPollTask's own a0) rather than through a New_X-style
 * allocator -- confirmed by Class6D3C8__RunPollTask's shape: `jalr` straight on the
 * incoming a0 with the object argument in a0, no allocation call at all.
 * Shares slot +0x004/+0x044 offsets and signatures with LoaderTaskMethods,
 * consistent with the class-framework's shared low base-class slots, but
 * kept as its own type since nothing ties the two classes together. */
typedef struct PollTaskMethods {
    s32 header;                                   /* +0x000 */
    void (*slot4)(void *self);                      /* +0x004 */
    u8 pad08[0x044 - 0x008];                          /* +0x008 .. +0x043 */
    s32 (*slot44)(void *self, s32 a1, s32 a2);          /* +0x044 */
} PollTaskMethods;

typedef struct PollTask {
    PollTaskMethods *methods;
} PollTask;

typedef PollTask *(*PollTaskCtor)(void *arg);

/* This unit's own function, defined later in ROM order (forward declared
 * for Class6D3C8__PollGraphRoomStatus, which comes first). Constructs a PollTask via the
 * caller-supplied `ctor`, dispatches slot44(task, extra, 0) and slot4(task)
 * on it, and returns slot44's result. */
s32 Class6D3C8__RunPollTask(PollTaskCtor ctor, void *dreamSys, s32 extra);

/* This unit's own function, defined later in ROM order (forward declared
 * for Class6D3C8__PollGraphRoomStatus, which comes first). */
void Class6D3C8__StartGraphRoomStreamTask(Class6D3C8 *self);

/* PollTask constructors, uncarved (not this unit's to write). Called
 * directly (not through any vtable) as Class6D3C8__RunPollTask's `ctor` argument. */
extern PollTask *New_GraphRoomObj(void *dreamSys);
extern PollTask *New_Class86B60(void *dreamSys);

extern s32 func_800493E4(s32 *out, s32 a1, s32 a2); /* psyq_memset.s: writes a derived count to *out, returns a separate derived value */
extern s32 func_800491FC(s32 *out, s32 unused); /* psyq_memset.s: same "write to *out, return a
    separate value" shape as func_800490F4/func_8004913C/func_800493E4 */
extern s32 func_80049334(s32 *out, s32 packedBankEntry); /* psyq_memset.s: resolves a packed
    {bank; entry} CinematicCall (low 16 bits = bank, high 16 = entry) to a channel index written
    to *out (-1 if unresolved); the packing must zero-extend both halves before combining
    (retail loads them with lhu, not lh) since the result is bitwise-composed, not a value read
    back as a signed 32-bit number. Also returns its own (separate) s32 value, kept by
    Class6D3C8__StartCinematicStream. */

/* A fourth small class, allocated by New_Obj865C8 (uncarved,
 * asm/class_39e08.s, New_X shape, 0x50 bytes). slot4 here is called with
 * ONLY self (no extra args) and its return is used as a small status
 * code -- a different signature from every other class's slot4 in this
 * unit, confirming these per-class slot tables are independent even
 * where offsets coincide. */
typedef struct StatusObjMethods {
    s32 header;                              /* +0x000 */
    void (*slot4)(void *self);                  /* +0x004 */
    u8 pad08[0x044 - 0x008];                     /* +0x008 .. +0x043 */
    s32 (*slot44)(void *self);                    /* +0x044: return value matters -- confirmed by the
                                                       call site, which keeps THIS return (not slot4's,
                                                       captured via slot4's own jalr delay slot the same
                                                       way Class6D3C8__RunPollTask keeps its own slot44 result). */
} StatusObjMethods;

typedef struct StatusObj {
    StatusObjMethods *methods;
} StatusObj;

extern StatusObj *New_Obj865C8(s32 a0, void *dreamSys, s32 a2);

/* This unit's own function, defined later in ROM order (forward declared
 * for Class6D3C8__PollStatusObj, which comes first). */
void Class6D3C8__StartCinematicStream(Class6D3C8 *self);

#endif
