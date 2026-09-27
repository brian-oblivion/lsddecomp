#ifndef CODE_8220_H
#define CODE_8220_H

#include "common.h"

/* The BasicClass type, its method table and its list primitives are
 * include/BasicClass.h (FINISHING-PLAN track 4: one header per class).
 * This header keeps the rest of the code_8220 units' declarations: the
 * pool allocator, the prim-setup and GTE helpers. */
#include "BasicClass.h"

/*
 * BMemBlockHdr -- the header word of one block in a BMemPMgr pool, and, while
 * the block is free, the two free-list links that follow it. `sizeAndFlags`
 * packs the block's byte size (header word included) into the low 28 bits
 * and flag bits into the high 4. `prev`/`next` link the pool's
 * doubly-linked free list, `prev` toward `freeListHead` and `next` toward
 * `freeListTail`. An allocated block's links are its caller's payload.
 */
typedef struct BMemBlockHdr BMemBlockHdr;

struct BMemBlockHdr {
    /* +0x000 */ u32 sizeAndFlags;
    /* +0x004 */ BMemBlockHdr *prev;
    /* +0x008 */ BMemBlockHdr *next;
};

/* sizeAndFlags. A block's size counts its header word and its footer; the
 * flags are the block's own state and its lower neighbour's. */
#define BMEM_SIZE_MASK 0x0FFFFFFF
#define BMEM_FLAG_MASK 0xF0000000
#define BMEM_FREE 0x40000000      /* this block is on the free list */
#define BMEM_PREV_FREE 0x80000000 /* the block below this one is free */

/* Block layout: the header word, then the payload BMemPMgrAlloc returns. A
 * free block's last word, its footer, points back at its header, which is how
 * BMemPMgrFree finds a free lower neighbour to merge with. A block is at least
 * BMEM_MIN_BLOCK bytes: a header word and a payload that can hold the two
 * free-list links and the footer. */
#define BMEM_HEADER_SIZE 4
#define BMEM_MIN_PAYLOAD 12
#define BMEM_MIN_BLOCK 16
#define BMEM_BLOCK_SIZE(b) ((b)->sizeAndFlags & BMEM_SIZE_MASK)
#define BMEM_NEXT_BLOCK(b) ((BMemBlockHdr *)((u8 *)(b) + BMEM_BLOCK_SIZE(b)))
#define BMEM_FOOTER(b) (*(BMemBlockHdr **)((u8 *)(b) + BMEM_BLOCK_SIZE(b) - 4))
#define BMEM_PREV_FOOTER(b) (*(BMemBlockHdr **)((u8 *)(b) - 4))
#define BMEM_PAYLOAD(b) ((void *)((u8 *)(b) + BMEM_HEADER_SIZE))
#define BMEM_HEADER_OF(payload) ((BMemBlockHdr *)((u8 *)(payload) - BMEM_HEADER_SIZE))

/*
 * BMemPMgr -- a pool's header, at the start of the one malloc'd area that
 * also holds its blocks. The blocks begin at `firstBlock` and are walked by
 * size, not through this struct; the free ones are also linked from
 * `freeListHead` to `freeListTail`. BMemPMgrFree appends a freed block at
 * the tail and BMemPMgrAlloc searches from the tail backward.
 */
typedef struct BMemPMgr BMemPMgr;

struct BMemPMgr {
    /* +0x000 */ void *firstBlock; /* the first block, just past this header */
    /* +0x004 */ s32 poolSize;     /* bytes of blocks, from firstBlock */
    /* +0x008 */ BMemBlockHdr *freeListTail;
    /* +0x00C */ BMemBlockHdr *freeListHead;
    /* +0x010 */ s32 initialized; /* set to 1 by SetupBMemPMgrFreeList; no reader */
};

/* The malloc'd area is this header, poolSize bytes of blocks, then a
 * zero-size sentinel block word that stops BMemPMgrFree's merge with the block
 * above. BMEMPMGR_HEADER_SIZE is where firstBlock starts; the struct above
 * names only the fields the code touches. */
#define BMEMPMGR_HEADER_SIZE 28
#define BMEMPMGR_SENTINEL_SIZE 4
#define BMEMPMGR_MIN_POOL_SIZE 1024

/* The generic pool allocator/free pair, established already by
 * include/DreamSys.h, include/Entity.h etc. --
 * all single-argument, and every one of those ~15 headers declares its
 * own full ANSI prototype (`s32 size` / `void *ptr`), per this project's
 * multiple-independent-local-views convention -- none of them get their
 * declaration from this header.
 *
 * THIS header, uniquely, declares both with UNSPECIFIED parameters
 * (empty parens). Round 45 (BMemPMgrAlloc/BMemPMgrFree, matched): each
 * function's own BODY genuinely reads a second argument ($a1, a fallback
 * pool pointer used only when the global default pool gDefaultBMemPMgr is
 * unset -- dead in practice at every decoded call site, confirmed by
 * SetupBMemPMgrFreeList/A9C setting that global before either is ever called).
 * Both are therefore DEFINED in code_8220.c with an old-style
 * (K&R identifier-list) parameter list, which is the only way to expose
 * that second parameter to their own bodies without contradicting the
 * ~15 external single-argument prototypes OR this same unit's own
 * single-argument call sites (PushBasicClassListNode's `BMemPMgrAlloc(0x8)`,
 * RemoveBasicClassListNode's `BMemPMgrFree(node)`) that appear LATER in
 * code_8220.c. A K&R-style definition does not install a prototype, so
 * those later 1-argument calls stay uncheck-and-compile clean; an
 * unspecified-parameter declaration here does the same for everything
 * before the definition. Do not "fix" this back to a full prototype --
 * that reintroduces the conflict this was written to route around. */
extern void *BMemPMgrAlloc(); /* arity-ok: re-measured round 59 -- the body really does read $a1 -- `move s1,a1` at 0x80017B40, consumed as `move t0,s1` at 0x80017B68 only when the gp default pool is unset. The ~22 one-parameter declarations elsewhere are right about THEIR call sites (retail emits $a0 only, e.g. `move a0,s2` at 0x80026B74); this unprototyped pair is required by the K&R definitions in code_8220.c. */
extern void *BMemPMgrFree(); /* arity-ok: re-measured round 59, same -- `move s1,a1` at 0x80017D0C, consumed as `move t0,s1` at 0x80017D2C on the unset-default-pool path. */

/* BMemPMgr setup, gp_rel-blocked (docs/research/gp-relative-blocker.md).
 * Called only by BMemPMgrInit in this unit. Genuinely ONE argument: its
 * own body's $a1 is a fallback pool pointer (defaulting to $a0/self) used
 * only when the global default pool gDefaultBMemPMgr is unset, and
 * BMemPMgrInit's call site never sets $a1 before the `jal` -- confirmed
 * by objdump: declaring a second parameter here forces the caller to
 * materialise a spurious `move a1,s1`, one word too many. */
extern void SetupBMemPMgrFreeList(BMemPMgr *pool);

/* The default-pool global itself (see the comment above). Setter is
 * SetDefaultBMemPMgr(BMemPMgr *pool), a one-line `gDefaultBMemPMgr = pool;`. Not yet
 * called from any carved C -- BMemPMgrInit never calls it, so whoever
 * establishes the game's one default pool is still asm. */
extern BMemPMgr *gDefaultBMemPMgr;

/* Pool allocator/free critical-section flag, code_8220_b (setter
 * SetBMemPMgrBusy, getter GetBMemPMgrBusy). BMemPMgrAlloc/BMemPMgrFree in
 * THIS unit bracket their free-list walk with SetBMemPMgrBusy(1) on entry
 * and SetBMemPMgrBusy(0) on exit -- an enter/exit pair, not a real lock
 * (no busy-wait or check on entry visible in either caller). */
extern s32 gBMemPMgrBusy;
extern void SetBMemPMgrBusy(s32 val);
extern s32 GetBMemPMgrBusy(void);

/* The Psy-Q declarations that used to sit here (func_80011D34 is malloc,
 * func_80011F68 is free, func_80012C20 is printf) moved into src/code_8220.c
 * when the SDK objects were linked. They are deliberately NOT shared:
 * printf is variadic and five units each declare the argument shape their
 * own call site passes, and malloc/free now have Sony's real names, so a
 * copy in this header would collide with <malloc.h> in whichever sibling
 * unit includes the SDK header first. See CLAUDE.md, "To include/ has one
 * exception". */

/* The "bMemPMgr = %p, poolSize = %ld in BMemPMgrInit\n" format string,
 * asm/data/A8C.rodata.s. */
extern const char sBMemPMgrInitFailFmt[];

/* Global boolean flag read by SetupPrimCode, asm/data (bss/data, not yet
 * carved). Read by SetupPrimCode, written by SortTmdObject (both code_8220_b)
 * from bit 6 of the drawn object's flags word; SetupPrimCode ORs it into bit
 * 0x1 of the GPU command byte, which is the shade-texture bit Psy-Q's
 * SetShadeTex() sets. PROPOSED RENAME (round 51, tier B): gShadeTex.
 * tools/rename.py cannot do it -- "resolves to 0x8008e248, outside the
 * image", because it is bss past the image end -- so the head applies it. */
extern s32 D_8008E248;

/* GTE transform/clip/OT-bucket routine, this unit (code_8220_b; ordinary C
 * over the include/gte.h macros -- see docs/match-reports/TransformAndCullPoly.md;
 * the .c holds its local struct views of both arguments). arg1 is the
 * per-object draw context (OT base +0x0, OT shift +0x4, culled-flag +0x78,
 * SXY0-2 cache +0x60/0x64/0x68, computed OT bucket pointer +0x30, ...);
 * arg0 is the Psy-Q GPU primitive being filled in, and its only touched
 * field is the P_TAG length byte at +0x3, re-stamped from the copy
 * SetupPrimCode cached at arg1->0x14. Returns 0 on success (OT bucket
 * computed and stored), 1 if the primitive was culled/degenerate. Declared
 * here because its two callers in this unit (ProjectTriFace,
 * ProjectQuadFace) are earlier in ROM order and so precede its own
 * definition in the .c file. */
extern s32 TransformAndCullPoly(void *arg0, void *arg1);

/* code_8220_c. Called by ProjectTriFace/ProjectQuadFace for a face that
 * survived the cull: takes the screen bounding box of its `count` (3 or 4)
 * cached XYs and sets ctx+0x78, the flag that makes the SubmitPoly*
 * wrappers subdivide the face through RCpoly*, when the box is wider or
 * taller than 256 pixels. */
extern void FlagLargePolyForDivide(void *ctx, s32 count);

/* The two subdivision work buffers code_8220_c's SubmitPoly* wrappers hand
 * Sony's RCpoly* packers: a DIVPOLYGON3 and a DIVPOLYGON4 (libgte.h), laid
 * out back to back (0x218 bytes apart, sizeof(DIVPOLYGON3)). Declared as
 * bytes here because this header does not include <libgte.h>; code_8220_b
 * only takes their addresses (InitDivPolygonPtrs) and code_8220_c casts. */
extern u8 gDivPolygon3[];
extern u8 gDivPolygon4[];

#endif
