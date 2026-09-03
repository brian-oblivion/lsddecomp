#ifndef CODE_8220_H
#define CODE_8220_H

#include "common.h"

/*
 * BasicClass -- the game's hand-rolled base class, root of the class
 * framework (docs/research/class-framework.md). Vtable is D_8006B58C,
 * 14 slots, resolved with `tools/classtable.py D_8006B58C`. Constructed
 * with no arguments beyond `self`; every other class's own constructor
 * dispatches its BASE construction through BasicClass's own ctor slot,
 * per the class-framework doc ("constructors are called through the
 * method table, base-class constructors included").
 *
 * BasicClass maintains two singly-linked lists of OTHER BasicClass
 * objects, built from pool-allocated 8-byte nodes (BasicClassListNode):
 *  - `children` (+0x004): objects added via `addChild`/`removeChild`.
 *    Adding one also registers `self` in the CHILD's own `parentRefs`
 *    list (child->methods->addParentRef(child, self)), so the
 *    relationship is bidirectional -- a child can look back at every
 *    parent holding a reference to it. Confirmed from BasicClass__func_
 *    17f98/17ff0 (addChild/removeChild), which dispatch through the
 *    CHILD's own vtable slots +0x020/+0x024 -- the same slots this
 *    class's own `addParentRef`/`removeParentRef` occupy -- so any
 *    BasicClass-family object can play "child" here.
 *  - `parentRefs` (+0x008): the back-reference list described above,
 *    plain push/remove/clear with no notification callback (unlike
 *    `children`, which fires the child's own vtable slot).
 *
 * List nodes come from the pool allocator (BMemPMgrInit/func_80017B34/
 * func_80017CFC family; func_80017B34 and func_80017CFC's second
 * "pool" parameter is a fallback used only when a global default pool
 * pointer, D_8008A818, is unset -- established already by
 * include/class_3ac78.h, include/DreamSys.h etc., which all declare
 * both as single-argument).
 */
typedef struct BasicClass BasicClass;
typedef struct BasicClassMethods BasicClassMethods;
typedef struct BasicClassListNode BasicClassListNode;

/* One node of either of BasicClass's two lists. 8 bytes -- the literal
 * allocation size func_800181AC passes to the pool allocator. */
struct BasicClassListNode {
    BasicClassListNode *next;  /* +0x000 */
    BasicClass *value;          /* +0x004 */
};

struct BasicClassMethods {
    /* +0x000 */ s32 header;
    /* +0x004 */ void *(*release)(BasicClass *self);                              /* BasicClass__func_17eb0: virtual finalize, then free self */
    /* +0x008 */ void (*ctor)(BasicClass *self);                                  /* BasicClass__BasicClass */
    /* +0x00C */ void (*finalize)(BasicClass *self);                              /* BasicClass__func_17f2c: onFinalize(1), removeAllChildren(), clearParentRefs() */
    /* +0x010 */ void (*addChild)(BasicClass *self, BasicClass *child);           /* BasicClass__func_17f98 */
    /* +0x014 */ void (*removeChild)(BasicClass *self, BasicClass *child);        /* BasicClass__func_17ff0 */
    /* +0x018 */ void (*removeAllChildren)(BasicClass *self);                     /* BasicClass__func_18040 */
    /* +0x01C */ void (*getNextChild)(BasicClass *self, BasicClass **outChild, BasicClassListNode **cursor); /* BasicClass__func_180bc */
    /* +0x020 */ s32 (*addParentRef)(BasicClass *self, BasicClass *parent);       /* BasicClass__func_180fc; tail-calls func_800181AC, so typed non-void per the one-line-wrapper rule */
    /* +0x024 */ void (*removeParentRef)(BasicClass *self, BasicClass *parent);   /* BasicClass__func_1811c; tail-calls func_80018208, which is void (see below) */
    /* +0x028 */ void (*clearParentRefs)(BasicClass *self);                       /* BasicClass__func_1813c */
    /* +0x02C */ void (*getNextParentRef)(BasicClass *self, BasicClass **outParent, BasicClassListNode **cursor); /* BasicClass__func_1816c */
    /* +0x030 */ void (*onFinalize)(BasicClass *self, s32 arg1);                  /* BasicClass__func_182cc; out of this round's carved slice (code_8220_b) */
    /* +0x034 */ void (*slot34)(void);                                            /* BasicClass__func_18350; empty (`jr $ra; nop`) for the base class, code_8220_b */
    /* +0x038 */ void (*slot38)(BasicClass *self, void *arg1, s32 arg2);          /* BasicClass__func_18358; code_8220_b */
};

struct BasicClass {
    /* +0x000 */ BasicClassMethods *methods;
    /* +0x004 */ BasicClassListNode *children;
    /* +0x008 */ BasicClassListNode *parentRefs;
};

/*
 * bMemPMgr -- BMemPMgrInit's own pool-header object. Only the two fields
 * BMemPMgrInit itself writes are typed; the rest of the allocation
 * (poolSize + 0x20 bytes total) is an opaque free-list area owned by
 * func_80017AC8/func_80017B34/func_80017CFC (all gp_rel-blocked, see
 * docs/research/gp-relative-blocker.md), manipulated as a packed
 * size+flags word per node -- never as this struct's own fields.
 */
typedef struct BMemPMgr BMemPMgr;
struct BMemPMgr {
    /* +0x000 */ void *freeListHead;  /* set to `self + 0x1C` by BMemPMgrInit; a free-block header immediately after this struct */
    /* +0x004 */ s32 poolSize;
};

/* The generic pool allocator/free pair, established already by
 * include/class_3ac78.h, include/DreamSys.h, include/Entity.h etc. --
 * both single-argument; see the BasicClass doc comment above for why
 * the pool-pointer second parameter these two functions' own bodies
 * read is not a real argument in practice. */
extern void *func_80017B34(s32 size);
extern void *func_80017CFC(void *ptr);

/* BMemPMgr setup, gp_rel-blocked (docs/research/gp-relative-blocker.md).
 * Called only by BMemPMgrInit in this unit. Genuinely ONE argument: its
 * own body's $a1 is a fallback pool pointer (defaulting to $a0/self) used
 * only when the global default pool D_8008A818 is unset, and
 * BMemPMgrInit's call site never sets $a1 before the `jal` -- confirmed
 * by objdump: declaring a second parameter here forces the caller to
 * materialise a spurious `move a1,s1`, one word too many. */
extern void func_80017AC8(BMemPMgr *pool);

/* Psy-Q SDK (asm/psyq_2258.s, not a carved C unit). */
extern void *func_80011D34(s32 size);                              /* heap allocator behind BMemPMgrInit's fallback error path */
extern void func_80012C20(const char *fmt, void *arg1, s32 arg2);  /* Psy-Q printf wrapper; BMemPMgrInit's only caller passes exactly 2 variadic args */
extern s32 func_80011F68(void *ptr);                                /* marks the block header at ptr-4's low bit; func_80017AA8's sole callee */

/* BasicClass list primitives, this unit. func_800181AC/func_80018208
 * stay INCLUDE_ASM this round; calling into a still-INCLUDE_ASM function
 * in the same or another unit is fine (docs/DECOMPILATION_LEARNINGS.md). */
extern s32 func_800181AC(BasicClassListNode **head, BasicClass *value);  /* push: allocate a node, prepend to *head */
extern void func_80018208(BasicClassListNode **head, BasicClass *value); /* find node by ->value == value, unlink, free; void -- see .md */

/* Not yet carved (asm/code_8220_b.s, the tail of this 56-function block). */
extern void func_800183A0(BasicClass **outValue, BasicClassListNode **cursor); /* pop *cursor into *outValue (or NULL), advance *cursor */
extern void func_80018288(BasicClassListNode **head);                          /* free every node in the list, does not clear *head itself */
extern BasicClassMethods *func_80018390(void);                                 /* returns &D_8006B58C, i.e. BASICCLASS_METHODS */

/* The "bMemPMgr = %p, poolSize = %ld in BMemPMgrInit\n" format string,
 * asm/data/A8C.rodata.s. */
extern const char D_8001028C[];

#endif
