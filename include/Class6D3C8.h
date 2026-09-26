#ifndef CLASS_6D3C8_H
#define CLASS_6D3C8_H

#include "common.h"
#include "DreamSys.h"
#include "TaskCore.h"
#include "StreamTask.h"
#include "Class6E4F0.h"

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
 * the intermediate class's ctor slot (via GetClass6E4F0Methods -> &D_8006E4F0)
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

/* The parent class, D_8006E4F0, is declared once in include/Class6E4F0.h
 * (track 4, round 84). This header's own views of it (MiddleClassMethods and
 * the table getter's extern) are gone; the parent's ctor and +0x044 slot are
 * reached through GetClass6E4F0Methods() with an upcast. */

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
    void (*slot4C)(Class6D3C8 *self);                       /* +0x04C Class6E4F0__RunMainLoop, first dispatched by main
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
extern void *New_LinkResource(void *arg); /* defined in code_33808.c (LinkResource allocator) */

/* The stream tasks this class starts are StreamTasks (include/StreamTask.h,
 * round 87; this header's local StreamTask view was merged there). */

extern s32 SetActiveDataSourceDriverMode(s32 a0, s32 a1, s32 a2); /* code_171e0, still INCLUDE_ASM there; returns
                                                       the last value its internal dispatch loop got --
                                                       Class6D3C8__LoadIntroLogoSequence/Class6D3C8__StartWeeklyStreamTask discard it, but
                                                       Class6D3C8__StartCinematicStream keeps it */
extern const char *GetIntroStreamName(s32 *typeCodeOut);  /* psyq_memset.s: writes 0x31 to *typeCodeOut if non-NULL, always returns &sAsmkStreamPath */
extern s32 GetStreamGroupForType(s32 index);                   /* psyq_memset.s: signed-halfword lookup into gStreamTypeToGroupTable[index] */
extern s32 PickWeeklyStreamChannel(s32 *out, s32 param2);          /* psyq_memset.s: day/week-style calculation (divides SeedAndRandom's result by 7); writes a related index to *out if non-NULL, returns a separate derived value */

extern const char sLogoPathAsmk[]; /* "ETC\ASMKLOGO.TIM" */
extern const char sLogoPathOsd[]; /* "ETC\OSDLOGO.TIM" */

/* Forward declaration: Class6D3C8__StartLoaderTask (this unit, defined later in ROM
 * order) is called by Class6D3C8__LoadIntroLogoSequence, which comes first in the file. */
void Class6D3C8__StartLoaderTask(Class6D3C8 *self, const char *path);

/* The "loader" task Class6D3C8__StartLoaderTask and __StartCinematicStream
 * build is a plain TaskCore (New_TaskCore, include/TaskCore.h); this header
 * viewed it as `LoaderTask` until track 4 (round 84). Its old `start` slot
 * (+0x004) is BasicClass's release: init with mode 0 runs the task to its
 * end, and release finalizes and frees it. */

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

/* PollTask constructors (not this unit's to write). Called directly (not
 * through any vtable) as Class6D3C8__RunPollTask's `ctor` argument.
 * New_GraphRoom is include/GraphRoom.h's (code_1677c includes it and casts
 * it to PollTaskCtor). */
extern PollTask *New_Class86B60(void *dreamSys);

extern s32 GetGraphRoomStreamChannel(s32 *out, s32 a1, s32 a2); /* psyq_memset.s: writes a derived count to *out, returns a separate derived value */
/* code_39094.c: same "write to *out, return a separate value" shape as
 * GetIntroStreamName/PickWeeklyStreamChannel/GetGraphRoomStreamChannel. */
extern s32 GetStreamChannelInit(s32 *out, s32 unused); /* arity-ok: the definition is 1-parameter and reads only $a0 (it neither reads nor forwards $a1), but the 2nd argument IS byte-load-bearing -- retail emits `move a1,zero` in the jal's delay slot at 0x80026974 */
extern s32 ResolveCinematicChannel(s32 *out, s32 packedBankEntry); /* psyq_memset.s: resolves a packed
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
