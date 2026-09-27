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

/* The pool allocator and its free. Both bodies read a second argument, a
 * fallback pool used only while gDefaultBMemPMgr is unset, that no caller
 * passes: every other unit declares its own one-argument prototype for its
 * own call sites. code_8220.c defines them K&R so their bodies can name the
 * second parameter while its own later one-argument calls still compile; an
 * unprototyped declaration here keeps the earlier ones compiling too. A full
 * prototype here breaks one side or the other. */
extern void *BMemPMgrAlloc(); /* arity-ok: the body reads $a1 as the fallback pool (BMemPMgrAlloc.md, "Extern arity") */
extern void *BMemPMgrFree(); /* arity-ok: same as BMemPMgrAlloc (BMemPMgrFree.md, "Extern arity") */

/* Makes the whole pool one free block. One argument: the body also reads a
 * fallback pool from $a1 while gDefaultBMemPMgr is unset, but BMemPMgrInit,
 * its only caller, never loads $a1, and a second parameter here would make it
 * load one. */
extern void SetupBMemPMgrFreeList(BMemPMgr *pool);

/* The pool SetupBMemPMgrFreeList, BMemPMgrAlloc and BMemPMgrFree work on;
 * set by SetDefaultBMemPMgr (main.c, right after BMemPMgrInit). */
extern BMemPMgr *gDefaultBMemPMgr;

/* Set to 1 by BMemPMgrAlloc and BMemPMgrFree for the length of their free-list
 * work and back to 0 after (setter and getter in TmdRenderer.c). Nothing in
 * either waits on it. */
extern s32 gBMemPMgrBusy;
extern void SetBMemPMgrBusy(s32 val);
extern s32 GetBMemPMgrBusy(void);

/* Global boolean flag read by SetupPrimCode, asm/data (bss/data, not yet
 * carved). Read by SetupPrimCode, written by SortTmdObject (both TmdRenderer)
 * from bit 6 of the drawn object's flags word; SetupPrimCode ORs it into bit
 * 0x1 of the GPU command byte, which is the shade-texture bit Psy-Q's
 * SetShadeTex() sets. PROPOSED RENAME (round 51, tier B): gShadeTex.
 * tools/rename.py cannot do it -- "resolves to 0x8008e248, outside the
 * image", because it is bss past the image end -- so the head applies it. */
extern s32 D_8008E248;

/* GTE transform/clip/OT-bucket routine, this unit (TmdRenderer; ordinary C
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

/* TmdRenderer. Called by ProjectTriFace/ProjectQuadFace for a face that
 * survived the cull: takes the screen bounding box of its `count` (3 or 4)
 * cached XYs and sets ctx+0x78, the flag that makes the SubmitPoly*
 * wrappers subdivide the face through RCpoly*, when the box is wider or
 * taller than 256 pixels. */
extern void FlagLargePolyForDivide(void *ctx, s32 count);

/* The two subdivision work buffers TmdRenderer's SubmitPoly* wrappers hand
 * Sony's RCpoly* packers: a DIVPOLYGON3 and a DIVPOLYGON4 (libgte.h), laid
 * out back to back (0x218 bytes apart, sizeof(DIVPOLYGON3)). Declared as
 * bytes here because this header does not include <libgte.h>; TmdRenderer
 * only takes their addresses (InitDivPolygonPtrs) and TmdRenderer casts. */
extern u8 gDivPolygon3[];
extern u8 gDivPolygon4[];

#endif
