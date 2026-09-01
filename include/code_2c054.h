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
 *    (func_8003B9DC) and slot 0x044 (func_8003BA58) by their own
 *    disassembly, and follows the "ownDtorChain" convention already
 *    documented in code_171e0.h for slot 0x004.
 *
 * classtable.py confirms D_8006E5F8 and D_8006E730 share the great majority
 * of their slots verbatim (same function address at the same offset in
 * both tables) and differ only at 0x008/0x00C (ctor/dtor), 0x040/0x044/
 * 0x04C (StreamTask's own overrides vs. the base class's own
 * func_8003C11C/func_8003C1DC/func_8003C238) and 0x05C/0x060. This is
 * consistent with StreamTask deriving from the class whose own vtable IS
 * D_8006E730, though the base class's own name is not yet known -- it is
 * typed here only as TaskBaseMethods, with only the slots this unit's
 * queued functions actually dispatch through.
 */

typedef struct StreamTask StreamTask;

/* The base task class's own vtable (D_8006E730), fetched fresh via
 * func_8003DFBC() rather than through any object -- every call site in this
 * unit re-fetches it instead of caching a pointer, so it reads like a
 * "get the base class's own method table" helper, not an instance getter.
 * Only the slots dispatched through by this unit's functions are typed. */
typedef struct TaskBaseMethods TaskBaseMethods;
struct TaskBaseMethods {
    /* +0x000 */ s32 header;
    /* +0x004 */ void *unk04;
    /* +0x008 */ void *ctor; /* func_8003BF10; signature not needed by this unit */
    /* +0x00C */ void *(*dtor)(StreamTask *self); /* func_8003C008 */
    /* +0x010 */ u8 pad10[0x044 - 0x010];
    /* +0x044 */ void (*slot44)(StreamTask *self, s32 a1, s32 a2); /* func_8003C1DC */
    /* +0x048 */ u8 pad48[0x080 - 0x048];
    /* +0x080 */ s32 (*slot80)(StreamTask *self); /* func_8003C858 (shared with StreamTask's own 0x080) */
    /* +0x084 */ s32 (*slot84)(StreamTask *self); /* func_8003C9B0 (shared with StreamTask's own 0x084) */
};

extern TaskBaseMethods *func_8003DFBC(void);

/* StreamTask's own vtable (D_8006E5F8). Only the slots implemented by this
 * unit's queued functions are typed; the rest stay as padding. */
typedef struct StreamTaskMethods StreamTaskMethods;
struct StreamTaskMethods {
    /* +0x000 */ s32 header;
    /* +0x004 */ void *unk04;
    /* +0x008 */ void *ctor; /* func_8003B8E4 */
    /* +0x00C */ void *dtor; /* func_8003B9DC */
    /* +0x010 */ u8 pad10[0x040 - 0x010];
    /* +0x040 */ void (*slot40)(StreamTask *self); /* func_8003BA38 */
    /* +0x044 */ void (*slot44)(StreamTask *self, s32 a1, s32 a2, s32 typeLookup, s32 flag); /* func_8003BA58 */
    /* +0x048 */ u8 pad48[0x06C - 0x048];
    /* +0x06C */ void (*slot6C)(StreamTask *self, s32 a1); /* func_8003BCF4 */
    /* +0x070 */ u8 pad70[0x080 - 0x070];
    /* +0x080 */ s32 (*slot80)(StreamTask *self); /* func_8003BD74 */
    /* +0x084 */ s32 (*slot84)(StreamTask *self); /* func_8003BDAC */
};

extern StreamTaskMethods D_8006E5F8;

/* Returns StreamTask's own class table (&D_8006E5F8), the same
 * getter-shape already seen for other classes (e.g. code_171e0.h's
 * func_80026C9C -> &D_8006D430). Used by func_8003B854 (the New_X
 * allocator, uncarved by this runner) to fetch the ctor from slot +0x008. */
extern StreamTaskMethods *func_8003BE84(void);

/* A sub-object referenced from StreamTask+0xB4 -- some other class
 * instance whose method table sits at its own offset 0, per the class
 * framework convention. Only slot +0x004 (a self-only call) is dispatched
 * by this unit's func_8003B9DC. */
typedef struct StreamTaskSubMethods StreamTaskSubMethods;
struct StreamTaskSubMethods {
    /* +0x000 */ s32 header;
    /* +0x004 */ void (*slot4)(void *self);
};

typedef struct StreamTaskSub {
    StreamTaskSubMethods *methods;
} StreamTaskSub;

/* An instance of StreamTask. Object size is 0xDC (from func_8003B854's
 * allocator call). Only the fields this unit's queued functions actually
 * touch are named; the large unexplored spans stay as padding. */
struct StreamTask {
    /* +0x000 */ StreamTaskMethods *methods;
    /* +0x004 */ u8 pad04[0x040 - 0x004];
    /* +0x040 */ s32 unk40;             /* set by func_8003BCF4, StreamTaskMethods slot +0x06C */
    /* +0x044 */ u8 pad44[0x0B4 - 0x044];
    /* +0x0B4 */ StreamTaskSub *unkB4;   /* sub-object, ticked by func_8003B9DC's dtor override */
    /* +0x0B8 */ s32 unkB8;               /* set by func_8003BA58 */
    /* +0x0BC */ s32 unkBC;                /* set by func_8003BA58 ("typeLookup") */
    /* +0x0C0 */ s32 unkC0;                 /* set by func_8003BA58 ("flag", the 5th stack arg) */
    /* +0x0C4 */ s32 unkC4;                  /* set individually by func_8003BE5C, or reset by func_8003BA38 */
    /* +0x0C8 */ s32 unkC8;                   /* set individually by func_8003BE64, or reset by func_8003BA38 */
    /* +0x0CC */ s32 unkCC;                    /* set individually by func_8003BE6C, or reset by func_8003BA38 */
    /* +0x0D0 */ s32 unkD0;                     /* set individually by func_8003BE74, or reset by func_8003BA38 */
    /* +0x0D4 */ s32 unkD4;                      /* set individually by func_8003BE7C, or reset by func_8003BA38 */
};

#endif
