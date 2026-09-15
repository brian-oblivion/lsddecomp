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
 * BMemBlockHdr -- a single free-list node inside a BMemPMgr's pool area.
 * `sizeAndFlags` packs the block's byte size into the low 28 bits and
 * flag bits into the high 4 (0x40000000 = free); `prev`/`next` link the
 * pool's doubly-linked free list. Derived from func_80017AC8 (round 45)
 * and reused by func_80017B34/func_80017CFC's still-undecoded bodies,
 * which walk this same list via BMemPMgr's freeListStart/freeListEnd.
 */
typedef struct BMemBlockHdr BMemBlockHdr;
struct BMemBlockHdr {
    /* +0x000 */ u32 sizeAndFlags;
    /* +0x004 */ BMemBlockHdr *prev;
    /* +0x008 */ BMemBlockHdr *next;
};

/*
 * bMemPMgr -- BMemPMgrInit's own pool-header object. `freeListHead`/
 * `poolSize` are the two fields BMemPMgrInit itself writes; the three
 * below them (round 45, func_80017AC8) round out the pool's free-list
 * bookkeeping. What remains opaque is the pool AREA itself (poolSize +
 * 0x20 bytes total, starting at `freeListHead`), walked as a chain of
 * BMemBlockHdr nodes rather than through any field of this struct.
 */
typedef struct BMemPMgr BMemPMgr;
struct BMemPMgr {
    /* +0x000 */ void *freeListHead;   /* set to `self + 0x1C` by BMemPMgrInit; the pool's first free-list node */
    /* +0x004 */ s32 poolSize;
    /* +0x008 */ BMemBlockHdr *freeListStart; /* free list head, func_80017AC8/B34/CFC */
    /* +0x00C */ BMemBlockHdr *freeListEnd;   /* free list tail, same trio */
    /* +0x010 */ s32 unk10;            /* set to 1 by func_80017AC8; not yet read by any decoded function */
};

/* The generic pool allocator/free pair, established already by
 * include/class_3ac78.h, include/DreamSys.h, include/Entity.h etc. --
 * all single-argument, and every one of those ~15 headers declares its
 * own full ANSI prototype (`s32 size` / `void *ptr`), per this project's
 * multiple-independent-local-views convention -- none of them get their
 * declaration from this header.
 *
 * THIS header, uniquely, declares both with UNSPECIFIED parameters
 * (empty parens). Round 45 (func_80017B34/func_80017CFC, matched): each
 * function's own BODY genuinely reads a second argument ($a1, a fallback
 * pool pointer used only when the global default pool D_8008A818 is
 * unset -- dead in practice at every decoded call site, confirmed by
 * func_80017AC8/A9C setting that global before either is ever called).
 * Both are therefore DEFINED in code_8220.c with an old-style
 * (K&R identifier-list) parameter list, which is the only way to expose
 * that second parameter to their own bodies without contradicting the
 * ~15 external single-argument prototypes OR this same unit's own
 * single-argument call sites (func_800181AC's `func_80017B34(0x8)`,
 * func_80018208's `func_80017CFC(node)`) that appear LATER in
 * code_8220.c. A K&R-style definition does not install a prototype, so
 * those later 1-argument calls stay uncheck-and-compile clean; an
 * unspecified-parameter declaration here does the same for everything
 * before the definition. Do not "fix" this back to a full prototype --
 * that reintroduces the conflict this was written to route around. */
extern void *func_80017B34();
extern void *func_80017CFC();

/* BMemPMgr setup, gp_rel-blocked (docs/research/gp-relative-blocker.md).
 * Called only by BMemPMgrInit in this unit. Genuinely ONE argument: its
 * own body's $a1 is a fallback pool pointer (defaulting to $a0/self) used
 * only when the global default pool D_8008A818 is unset, and
 * BMemPMgrInit's call site never sets $a1 before the `jal` -- confirmed
 * by objdump: declaring a second parameter here forces the caller to
 * materialise a spurious `move a1,s1`, one word too many. */
extern void func_80017AC8(BMemPMgr *pool);

/* The default-pool global itself (see the comment above). Setter is
 * func_80017A9C(BMemPMgr *pool), a one-line `D_8008A818 = pool;`. Not yet
 * called from any carved C -- BMemPMgrInit never calls it, so whoever
 * establishes the game's one default pool is still asm. */
extern BMemPMgr *D_8008A818;

/* Pool allocator/free critical-section flag, code_8220_b (setter
 * func_8001844C, getter func_80018458). func_80017B34/func_80017CFC in
 * THIS unit bracket their free-list walk with func_8001844C(1) on entry
 * and func_8001844C(0) on exit -- an enter/exit pair, not a real lock
 * (no busy-wait or check on entry visible in either caller). */
extern s32 D_8008A820;
extern void func_8001844C(s32 val);
extern s32 func_80018458(void);

/* The Psy-Q declarations that used to sit here (func_80011D34 is malloc,
 * func_80011F68 is free, func_80012C20 is printf) moved into src/code_8220.c
 * when the SDK objects were linked. They are deliberately NOT shared:
 * printf is variadic and five units each declare the argument shape their
 * own call site passes, and malloc/free now have Sony's real names, so a
 * copy in this header would collide with <malloc.h> in whichever sibling
 * unit includes the SDK header first. See CLAUDE.md, "To include/ has one
 * exception". */

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

/* GTE transform/clip/OT-bucket routine, this unit (code_8220_b; ordinary C
 * over the include/gte.h macros -- see docs/match-reports/func_800195EC.md;
 * the .c holds its local struct view of arg1). arg1 is a per-primitive
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

/* Round 13: this unit's own minimal, local view of the Psy-Q GPU primitive
 * tag word -- the same shape as `P_TAG` in include/psyq/LIBGPU.H, declared
 * locally because that SDK header does not compile standalone under this
 * toolchain (it needs the LIBGTE/RECT chain) and no unit includes it yet.
 *
 * The 24-bit BITFIELD is the load-bearing part. On little-endian MIPS
 * `addr` occupies bits 0..23, so writing it is a read-modify-write that
 * GCC 2.6.3 emits as `& 0xFF000000` on the old word, `& 0x00FFFFFF` on the
 * new value, and an `or` -- which is exactly retail's OT linked-list
 * splice. Hand-writing those masks produces the same VALUE with different
 * register allocation and does NOT match. */
typedef struct OtTag {
    u32 addr : 24;
    u32 len  : 8;
} OtTag;

/* func_8001A3EC's payload types (round 13). Both are ALL-s16 and that is
 * load-bearing: all-s16 members give alignment 2, which is what makes a
 * whole-struct assignment compile to unaligned lwl/lwr + swl/swr instead of
 * aligned lw/sw. See DECOMPILATION_LEARNINGS, "A struct whose members are
 * all s8/s16 has alignment 2". One stray s32 member and the copy stops
 * matching. */
typedef struct PolyXY8 {
    s16 a;
    s16 b;
    s16 c;
    s16 d;
} PolyXY8;

typedef struct PolyUV4 {
    s16 u;
    s16 v;
} PolyUV4;

/* func_8001A3EC's element type. Only the 8-byte payload at +0x000 and the
 * 4-byte payload at +0x010 are touched by that function; the span between
 * is opaque from it alone. */
typedef struct PolyVtx {
    PolyXY8 xy;                  /* +0x000 */
    u8 pad008[0x010 - 0x008];
    PolyUV4 uv;                  /* +0x010 */
} PolyVtx;

/* Unaligned struct-field copy helper, code_8220_c (round 13). Takes two
 * 3-element arrays of PolyVtx pointers plus three UV sources. */
extern void func_8001A3EC(PolyVtx **dst, PolyVtx **src, PolyUV4 *uv0,
                          PolyUV4 *uv1, PolyUV4 *uv2);

/* Populates a GPU primitive header at `arg0` (D_8008ACD0/D_8008AEE8):
 * +0x00 an OT/code word (D_8008A834 when D_8008A830 is set, else
 * D_80090C18), +0x04 D_8008A824, +0x08 D_8008A828 -- these three are
 * UNCONDITIONAL (the third rides in the branch's own delay slot in
 * retail); only the two u16 stack args at +0x0C/+0x0E are actually
 * gated on `arg3 != 0`. +0x10 is an unaligned PolyUV4 copied from
 * `*arg2` (same lwl/lwr idiom as func_8001A3EC, forced by PolyUV4's
 * alignment-2 all-s16 layout); +0x14 is the plain word at
 * `arg1 + 0x30`. MATCHED round 44 after the gp_rel blocker that
 * stalled it at carve time (round 13) was resolved -- see
 * docs/match-reports/func_8001A380.md. Declared here so its caller in
 * this unit, func_800197C4, can compile (still INCLUDE_ASM). */
extern void func_8001A380(void *arg0, void *arg1, PolyUV4 *arg2, s32 arg3, u16 arg4, u16 arg5);

/* Psy-Q SDK (asm/psyq_rcpolyf3.s, not a carved C unit). Called by
 * func_800197C4 (code_8220_c) with (self, table). */
extern void RCpolyF3(void *self, void *table);

/* Opaque table, referenced only by ADDRESS (never dereferenced in this
 * unit) and handed to func_8001A380/RCpolyF3. asm/data, not yet
 * carved -- real element type unknown. */
extern u8 D_8008ACD0[];

/* Quad-flavored sibling of D_8008ACD0/func_8001A380/RCpolyF3,
 * referenced the same way by func_80019B24 (code_8220_c, round 13). */
extern u8 D_8008AEE8[];

/* Unaligned struct-field copy helper (quad flavor: 4 fields, not 3).
 * Extends func_8001A3EC to a 4th vertex: forwards elements 0-2 to it
 * unchanged, then does its own dst[3]->xy = src[3]->xy / dst[3]->uv = *uv3
 * (round 20). */
extern void func_8001A4C0(PolyVtx **dst, PolyVtx **src, PolyUV4 *uv0, PolyUV4 *uv1,
                          PolyUV4 *uv2, PolyUV4 *uv3);

/* Psy-Q SDK (asm/psyq_rcpolyf4.s, not a carved C unit). Called by
 * func_80019B24 (code_8220_c) with (self, table) -- quad-flavored sibling
 * of RCpolyF3. */
extern void RCpolyF4(void *self, void *table);

/* Psy-Q SDK (asm/psyq_rcpolyg3.s, not a carved C unit). Called by
 * func_8001989C (code_8220_c) with (self, table) -- Gouraud-shaded
 * sibling of RCpolyF3/RCpolyF4. */
extern void RCpolyG3(void *self, void *table);

/* Psy-Q SDK (asm/psyq_rcpolyg3.s, same file as RCpolyG3, not a
 * carved C unit). Called by func_800199EC (code_8220_c) with
 * (self, table). */
extern void RCpolyFT3(void *self, void *table);

/* Psy-Q SDK (asm/psyq_rcpolyg3.s, same file as RCpolyG3/RCpolyFT3,
 * not a carved C unit). Called by func_80019C04 (code_8220_c) with
 * (self, table) -- Gouraud-shaded quad, quad-flavored sibling of
 * RCpolyG3. */
extern void RCpolyG4(void *self, void *table);

/* Psy-Q SDK (asm/psyq_rcpolyft3.s, not a carved C unit). Called by
 * func_80019D84 (code_8220_c) with (self, table). */
extern void RCpolyFT4(void *self, void *table);

/* Psy-Q SDK (asm/psyq_rcpolygt3.s, not a carved C unit). Called by
 * func_80019EE4 (code_8220_c) with (self, table). */
extern void RCpolyGT3(void *self, void *table);

/* Psy-Q SDK (asm/psyq_rcpolygt3.s, same file as RCpolyGT3, not a
 * carved C unit). Called by func_8001A064 (code_8220_c) with (self,
 * table) -- Gouraud-shaded quad, quad-flavored sibling of RCpolyGT3. */
extern void RCpolyGT4(void *self, void *table);

#endif
