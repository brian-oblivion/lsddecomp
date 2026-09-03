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
    /* +0x030 */ void (*onFinalize)(BasicClass *self, s32 arg1);                  /* BasicClass__func_182cc; code_8220_b. Walks parentRefs, calling each parent's slot38(parent, self, arg1) */
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

/* Matched in code_8220_b, round 13. */
extern void func_800183A0(BasicClass **outValue, BasicClassListNode **cursor); /* pop *cursor into *outValue (or NULL), advance *cursor */
extern void func_80018288(BasicClassListNode **head);                          /* free every node in the list, does not clear *head itself */

/* BasicClass's own method table (BASICCLASS_METHODS), asm/data/57070.data.s.
 * 14 slots per BasicClassMethods, matching func_80018390 (code_8220_b, round
 * 12) which returns its address. */
extern BasicClassMethods D_8006B58C;
extern BasicClassMethods *func_80018390(void);                                 /* returns &D_8006B58C */

/* The "bMemPMgr = %p, poolSize = %ld in BMemPMgrInit\n" format string,
 * asm/data/A8C.rodata.s. */
extern const char D_8001028C[];

/* Global boolean flag read by func_8001934C, asm/data (bss/data, not yet
 * carved). Read-only from this unit; nothing here writes it. */
extern s32 D_8008E248;

/* GTE transform/clip/OT-bucket routine, this unit (code_8220_b, hand-rolled
 * asm -- see docs/match-reports/func_800195EC.md). arg1 is a per-primitive
 * scratch/context struct (OT base +0x0, OT shift +0x4, culled-flag +0x78,
 * SXY0-2 cache +0x60/0x64/0x68, computed OT bucket pointer +0x30, ...);
 * arg0's only touched field is a single output byte at +0x3, copied from
 * arg1->0x14. Returns 0 on success (OT bucket computed and stored), 1 if
 * the primitive was culled/degenerate. Declared here because its two
 * callers in this unit (func_800193C0, func_800194A4) are earlier in ROM
 * order and so precede its own definition in the .c file. */
extern s32 func_800195EC(void *arg0, void *arg1);

/* Called by func_800193C0/func_800194A4 after a successful OT insertion,
 * with a small literal "primitive kind" code (3 = triangle, 4 = quad).
 * code_8220_c, round 13. */
extern void func_8001A268(void *prim, s32 code);

/* Unaligned struct-field copy helpers, code_8220_c (round 13). */
extern void func_8001A3EC(void *dstArr, void *srcArr, void *src0, void *src1, void *src2);

/* gp_rel-blocked (docs/research/gp-relative-blocker.md), code_8220_c
 * round 13 -- see docs/match-reports/func_8001A380.md. Declared here only
 * so its ONE caller in this unit (func_800197C4) can compile; the extra
 * two args arrive on the stack in retail as plain 32-bit zero words (its
 * own body happens to read them with `lhu`, but that is internal to a
 * function that will never be matched under this toolchain). */
extern void func_8001A380(void *arg0, void *arg1, void *arg2, s32 arg3, s32 arg4, s32 arg5);

/* Psy-Q SDK (asm/psyq_rcpolyf3.s, not a carved C unit). Called by
 * func_800197C4 (code_8220_c) with (self, table). */
extern void func_8001A564(void *self, void *table);

/* Opaque table, referenced only by ADDRESS (never dereferenced in this
 * unit) and handed to func_8001A380/func_8001A564. asm/data, not yet
 * carved -- real element type unknown. */
extern u8 D_8008ACD0[];

/* Quad-flavored sibling of D_8008ACD0/func_8001A380/func_8001A564,
 * referenced the same way by func_80019B24 (code_8220_c, round 13). */
extern u8 D_8008AEE8[];

/* Unaligned struct-field copy helper (quad flavor: 4 fields, not 3),
 * code_8220_c round 13. STALLED -- see docs/match-reports/func_8001A4C0.md,
 * declared here only so its caller func_80019B24 can compile. */
extern void func_8001A4C0(void *arg0, void *arg1, void *arg2, void *arg3, void *arg4, void *arg5);

/* Psy-Q SDK (asm/psyq_rcpolyf4.s, not a carved C unit). Called by
 * func_80019B24 (code_8220_c) with (self, table) -- quad-flavored sibling
 * of func_8001A564. */
extern void func_8001A8D4(void *self, void *table);

/* Psy-Q SDK (asm/psyq_rcpolyg3.s, not a carved C unit). Called by
 * func_8001989C (code_8220_c) with (self, table) -- Gouraud-shaded
 * sibling of func_8001A564/func_8001A8D4. */
extern void func_8001AD54(void *self, void *table);

/* Psy-Q SDK (asm/psyq_rcpolyg3.s, same file as func_8001AD54, not a
 * carved C unit). Called by func_800199EC (code_8220_c) with
 * (self, table). */
extern void func_8001B6B4(void *self, void *table);

/* Psy-Q SDK (asm/psyq_rcpolyg3.s, same file as func_8001AD54/func_8001B6B4,
 * not a carved C unit). Called by func_80019C04 (code_8220_c) with
 * (self, table) -- Gouraud-shaded quad, quad-flavored sibling of
 * func_8001AD54. */
extern void func_8001B164(void *self, void *table);

/* Psy-Q SDK (asm/psyq_rcpolyft3.s, not a carved C unit). Called by
 * func_80019D84 (code_8220_c) with (self, table). */
extern void func_8001BAB4(void *self, void *table);

/* Psy-Q SDK (asm/psyq_rcpolygt3.s, not a carved C unit). Called by
 * func_80019EE4 (code_8220_c) with (self, table). */
extern void func_8001BFD4(void *self, void *table);

/* Psy-Q SDK (asm/psyq_rcpolygt3.s, same file as func_8001BFD4, not a
 * carved C unit). Called by func_8001A064 (code_8220_c) with (self,
 * table) -- Gouraud-shaded quad, quad-flavored sibling of func_8001BFD4. */
extern void func_8001C474(void *self, void *table);

#endif
