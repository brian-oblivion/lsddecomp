#ifndef CODE_2C054_H
#define CODE_2C054_H

#include "common.h"

/*
 * This unit implements (most of) two sibling "task" classes under the same
 * hand-rolled class framework as everywhere else in this game (see
 * CLAUDE.md's "Writing a class method"):
 *
 *  - StreamTask (D_8006E5F8's vtable, 0xDC-byte instances allocated by
 *    func_8003B854). This unit owns most of its overridden slots.
 *  - A base "task" class (D_8006E730's vtable, 0xA4-byte instances
 *    allocated by func_8003BE94, constructed by func_8003BF10). Several of
 *    StreamTask's own overrides finish by chaining into THIS class's
 *    unoverridden slot at the same offset -- fetched fresh each time via
 *    func_8003DFBC(), a pure getter (defined in the still-uncarved
 *    asm/code_2cc8c.s) that ignores its argument and always returns
 *    &D_8006E730. That chaining shape (own work, then dispatch through the
 *    base class's copy of the SAME slot index) is confirmed for slot 0x00C
 *    (func_8003B9DC), slot 0x044 (func_8003BA58/func_8003C1DC), slot 0x04C
 *    (func_8003BAB4), slot 0x05C (func_8003BB5C), slot 0x060 (func_8003BC14)
 *    and slot 0x078 (func_8003BD10), and follows the "ownDtorChain"
 *    convention already documented in code_171e0.h for slot 0x004.
 *
 *  - A THIRD, grandparent class (D_8006E878's vtable) sits above the base
 *    task class -- func_8003C1DC (the base task class's own slot +0x044,
 *    reached when StreamTask's own override at that slot chains down)
 *    itself chains one level further via func_8003E5C8(), a getter with the
 *    identical always-returns-a-fixed-table shape as func_8003DFBC. Nothing
 *    about this grandparent beyond its slot +0x044 signature is derived
 *    here.
 *
 * classtable.py confirms D_8006E5F8 and D_8006E730 share the great majority
 * of their slots verbatim (same function address at the same offset in
 * both tables) and differ only at 0x008/0x00C (ctor/dtor), 0x040/0x044/
 * 0x04C (StreamTask's own overrides vs. the base class's own
 * func_8003C11C/func_8003C1DC/func_8003C238) and 0x05C/0x060. This is
 * consistent with StreamTask deriving from the class whose own vtable IS
 * D_8006E730, though the base class's own name is not yet known -- it is
 * typed here only as TaskBaseMethods/BaseTask, with only the slots this
 * unit's queued functions actually dispatch through or construct.
 */

typedef struct StreamTask StreamTask;
typedef struct BaseTask BaseTask;

/* The grandparent class's own vtable (D_8006E878), fetched fresh via
 * func_8003E5C8() the same way func_8003DFBC() fetches D_8006E730. Only the
 * one slot this unit's func_8003C1DC dispatches through is typed. */
typedef struct TaskGrandBaseMethods TaskGrandBaseMethods;
struct TaskGrandBaseMethods {
    /* +0x000 */ s32 header;
    /* +0x004 */ void *unk04;
    /* +0x008 */ void *ctor;
    /* +0x00C */ void *dtor;
    /* +0x010 */ u8 pad10[0x044 - 0x010];
    /* +0x044 */ void (*slot44)(StreamTask *self, s32 a1, s32 a2); /* result discarded by func_8003C1DC */
};

extern TaskGrandBaseMethods *func_8003E5C8(void);

/* The base task class's own vtable (D_8006E730), fetched fresh via
 * func_8003DFBC() rather than through any object -- every call site in this
 * unit re-fetches it instead of caching a pointer, so it reads like a
 * "get the base class's own method table" helper, not an instance getter.
 * Only the slots dispatched through (or, for the ctor, constructed with) by
 * this unit's functions are typed. */
typedef struct TaskBaseMethods TaskBaseMethods;
struct TaskBaseMethods {
    /* +0x000 */ s32 header;
    /* +0x004 */ void *unk04;
    /* +0x008 */ void (*ctor)(BaseTask *self, s32 a0, s32 a1, s32 a2); /* func_8003BF10 */
    /* +0x00C */ void *(*dtor)(StreamTask *self); /* func_8003C008 */
    /* +0x010 */ u8 pad10[0x044 - 0x010];
    /* +0x044 */ s32 (*slot44)(StreamTask *self, s32 a1, s32 a2); /* func_8003C1DC; returns self->unk38 */
    /* +0x048 */ u8 pad48[0x04C - 0x048];
    /* +0x04C */ void (*slot4C)(StreamTask *self); /* func_8003C238 */
    /* +0x050 */ u8 pad50[0x05C - 0x050];
    /* +0x05C */ void (*slot5C)(StreamTask *self, s32 a1, s32 a2); /* func_8003C51C, uncarved here */
    /* +0x060 */ void (*slot60)(StreamTask *self, s32 a1); /* func_8003C63C, uncarved here */
    /* +0x064 */ u8 pad64[0x06C - 0x064];
    /* +0x06C */ void (*slot6C)(BaseTask *self, s32 a1); /* func_8003C11C's own dispatch through self->methods */
    /* +0x070 */ u8 pad70[0x078 - 0x070];
    /* +0x078 */ void (*slot78)(StreamTask *self); /* func_8003C858, shared with StreamTask's own 0x078 */
    /* +0x07C */ u8 pad7C[0x080 - 0x07C];
    /* +0x080 */ s32 (*slot80)(StreamTask *self); /* func_8003C858 (shared with StreamTask's own 0x080) */
    /* +0x084 */ s32 (*slot84)(StreamTask *self); /* func_8003C9B0 (shared with StreamTask's own 0x084) */
    /* +0x088 */ u8 pad88[0x09C - 0x088];
    /* +0x09C */ void (*slot9C)(BaseTask *self, s32 a1); /* func_8003C11C's own dispatch through self->methods */
    /* +0x0A0 */ void (*slotA0)(BaseTask *self, s32 a1);
    /* +0x0A4 */ void (*slotA4)(BaseTask *self, const char *a1, const char *a2, const char *a3);
};

extern TaskBaseMethods *func_8003DFBC(void);

/* A bare instance of the base task class (never a StreamTask -- StreamTask
 * overrides slot +0x040 with its own func_8003BA38, which never chains
 * down, so func_8003C11C's own self is always exactly this class). Object
 * size 0xA4 (func_8003BE94's allocator call). Only the fields this unit's
 * queued functions touch are named. */
struct BaseTask {
    /* +0x000 */ TaskBaseMethods *methods;
    /* +0x004 */ u8 pad04[0x028 - 0x004];
    /* +0x028 */ s32 unk28;
    /* +0x02C */ s32 unk2C;
    /* +0x030 */ s32 unk30;
    /* +0x034 */ s32 unk34;
    /* +0x038 */ u8 pad38[0x03C - 0x038];
    /* +0x03C */ s32 unk3C;
    /* +0x040 */ u8 pad40[0x084 - 0x040];
    /* +0x084 */ s32 unk84;
    /* +0x088 */ u8 pad88[0x09C - 0x088];
    /* +0x09C */ s32 unk9C;
    /* +0x0A0 */ s32 unkA0;
};

/* New_X-shaped allocator for a bare base-task instance: 0xA4 bytes, ctor
 * fetched from func_8003DFBC()'s own slot +0x008. */
extern BaseTask *func_8003BE94(s32 a0, s32 a1, s32 a2);

/* "ETC..." was NOT confirmed -- this is a plain rodata byte string read as
 * three overlapping/adjacent C-string slices (base, base+3, base+6) by
 * func_8003C11C. Not yet transcribed; only its address is needed here. */
extern const char D_8006E860[];

/* StreamTask's own vtable (D_8006E5F8). Only the slots implemented by this
 * unit's queued functions are typed; the rest stay as padding. */
typedef struct StreamTaskMethods StreamTaskMethods;
struct StreamTaskMethods {
    /* +0x000 */ s32 header;
    /* +0x004 */ void *unk04;
    /* +0x008 */ void (*ctor)(StreamTask *self, s32 a1, s32 a2, s32 a3, s32 a4); /* func_8003B8E4 */
    /* +0x00C */ void *dtor; /* func_8003B9DC */
    /* +0x010 */ u8 pad10[0x040 - 0x010];
    /* +0x040 */ void (*slot40)(StreamTask *self); /* func_8003BA38 */
    /* +0x044 */ void (*slot44)(StreamTask *self, s32 a1, s32 a2, s32 typeLookup, s32 flag); /* func_8003BA58 */
    /* +0x048 */ u8 pad48[0x060 - 0x048];
    /* +0x060 */ void (*slot60)(StreamTask *self, s32 a1); /* func_8003BC14 */
    /* +0x064 */ u8 pad64[0x06C - 0x064];
    /* +0x06C */ void (*slot6C)(StreamTask *self, s32 a1); /* func_8003BCF4 */
    /* +0x070 */ u8 pad70[0x078 - 0x070];
    /* +0x078 */ void (*slot78)(StreamTask *self); /* func_8003BD10 */
    /* +0x07C */ u8 pad7C[0x080 - 0x07C];
    /* +0x080 */ s32 (*slot80)(StreamTask *self); /* func_8003BD74 */
    /* +0x084 */ s32 (*slot84)(StreamTask *self); /* func_8003BDAC */
    /* +0x088 */ u8 pad88[0x094 - 0x088];
    /* +0x094 */ void (*slot94)(StreamTask *self); /* func_8003BDF4 */
};

extern StreamTaskMethods D_8006E5F8;

/* Returns StreamTask's own class table (&D_8006E5F8), the same
 * getter-shape already seen for other classes (e.g. code_171e0.h's
 * func_80026C9C -> &D_8006D430). Used by func_8003B854 to fetch the ctor
 * from slot +0x008. */
extern StreamTaskMethods *func_8003BE84(void);

/* New_X-shaped allocator for a StreamTask: 0xDC bytes, ctor fetched from
 * func_8003BE84()'s own slot +0x008. */
extern void *func_80017B34(s32 size);

/* A sub-object referenced from StreamTask+0xB4 -- some other class
 * instance whose method table sits at its own offset 0, per the class
 * framework convention. Slots +0x004 (func_8003B9DC), +0x040
 * (func_8003BAB4), +0x048 (func_8003BB5C), +0x04C (func_8003BAB4 and
 * func_8003BDF4) and +0x06C (func_8003BAB4) are dispatched by this unit. */
typedef struct StreamTaskSubMethods StreamTaskSubMethods;
struct StreamTaskSubMethods {
    /* +0x000 */ s32 header;
    /* +0x004 */ void (*slot4)(void *self);
    /* +0x008 */ u8 pad08[0x040 - 0x008];
    /* +0x040 */ s32 (*slot40)(void *self, s32 a1, s32 a2, s32 a3, s32 a4);
    /* +0x044 */ u8 pad44[0x048 - 0x044];
    /* +0x048 */ s32 (*slot48)(void *self);
    /* +0x04C */ void (*slot4C)(void *self);
    /* +0x050 */ u8 pad50[0x06C - 0x050];
    /* +0x06C */ void (*slot6C)(void *self, s32 a1);
};

typedef struct StreamTaskSub {
    StreamTaskSubMethods *methods;
} StreamTaskSub;

/* Three more small vtable-tagged objects reachable from StreamTask, all
 * dispatched only by func_8003C3D0 (StreamTask's slot +0x050, shared
 * verbatim with the base task class). Kept as separate minimal types since
 * nothing ties them to StreamTaskSub or to each other. */
typedef struct Obj18Methods Obj18Methods;
struct Obj18Methods {
    /* +0x000 */ s32 header;
    /* +0x004 */ u8 pad04[0x074 - 0x004];
    /* +0x074 */ void (*slot74)(void *self);
    /* +0x078 */ u8 pad78[0x090 - 0x078];
    /* +0x090 */ void (*slot90)(void *self);
};
typedef struct Obj18 {
    Obj18Methods *methods;
} Obj18;

typedef struct Obj78Methods Obj78Methods;
struct Obj78Methods {
    /* +0x000 */ s32 header;
    /* +0x004 */ u8 pad04[0x050 - 0x004];
    /* +0x050 */ void (*slot50)(void *self);
};
typedef struct Obj78 {
    Obj78Methods *methods;
} Obj78;

typedef struct ObjCDerefMethods ObjCDerefMethods;
struct ObjCDerefMethods {
    /* +0x000 */ s32 header;
    /* +0x004 */ u8 pad04[0x078 - 0x004];
    /* +0x078 */ void (*slot78)(void *self, void *a1, s32 a2);
};
typedef struct ObjCDeref {
    ObjCDerefMethods *methods;
} ObjCDeref;

/* An instance of StreamTask. Object size is 0xDC (from func_8003B854's
 * allocator call). Only the fields this unit's queued functions actually
 * touch are named; the large unexplored spans stay as padding. */
struct StreamTask {
    /* +0x000 */ StreamTaskMethods *methods;
    /* +0x004 */ u8 pad04[0x00C - 0x004];
    /* +0x00C */ ObjCDeref **unkC;      /* dereferenced, then dispatched via its own vtable, by func_8003C3D0 */
    /* +0x010 */ u8 pad10[0x018 - 0x010];
    /* +0x018 */ Obj18 *unk18;          /* dispatched (slot74, slot90) by func_8003C3D0 */
    /* +0x01C */ u8 pad1C[0x034 - 0x01C];
    /* +0x034 */ s32 unk34;             /* gates the last part of func_8003C3D0 */
    /* +0x038 */ s32 unk38;             /* "state"; set to 2 by func_8003BD10, read back by func_8003C1DC */
    /* +0x03C */ u8 pad3C[0x040 - 0x03C];
    /* +0x040 */ s32 unk40;             /* set by func_8003BCF4, StreamTaskMethods slot +0x06C */
    /* +0x044 */ u8 pad44[0x078 - 0x044];
    /* +0x078 */ Obj78 *unk78;          /* dispatched (slot50) by func_8003C3D0 */
    /* +0x07C */ u8 pad7C[0x093 - 0x07C];
    /* +0x093 */ u8 unk93;              /* only its ADDRESS is used (func_8003C3D0, as a callee's arg1);
                                          * true extent of this field/buffer is not derived here */
    /* +0x094 */ u8 pad94[0x0A4 - 0x094];
    /* +0x0A4 */ s32 unkA4;             /* a cached dispatch result / status, per func_8003BAB4/func_8003BB5C */
    /* +0x0A8 */ u8 padA8[0x0B4 - 0x0A8];
    /* +0x0B4 */ StreamTaskSub *unkB4;  /* sub-object, ticked by func_8003B9DC's dtor override */
    /* +0x0B8 */ s32 unkB8;             /* set by func_8003BA58 */
    /* +0x0BC */ s32 unkBC;             /* set by func_8003BA58 ("typeLookup") */
    /* +0x0C0 */ s32 unkC0;             /* set by func_8003BA58 ("flag", the 5th stack arg) */
    /* +0x0C4 */ s32 unkC4;             /* set individually by func_8003BE5C, or reset by func_8003BA38 */
    /* +0x0C8 */ s32 unkC8;             /* set individually by func_8003BE64, or reset by func_8003BA38 */
    /* +0x0CC */ s32 unkCC;             /* set individually by func_8003BE6C, or reset by func_8003BA38 */
    /* +0x0D0 */ s32 unkD0;             /* set individually by func_8003BE74, or reset by func_8003BA38 */
    /* +0x0D4 */ s32 unkD4;             /* set individually by func_8003BE7C, or reset by func_8003BA38;
                                          * gates func_8003BDF4/func_8003BC14's case-8 branch */
    /* +0x0D8 */ s32 unkD8;             /* set by func_8003BC14's cases 5/7, gates func_8003BB5C's tail call */
};

#endif
