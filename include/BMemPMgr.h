#ifndef BMEMPMGR_H
#define BMEMPMGR_H

#include "common.h"

/* The BMemPMgr pool allocator, src/app/BMemPMgr.c (its busy-flag accessors are
 * in src/graphics/TmdRenderer.c). BasicClass, whose child and parent lists are
 * allocated from the pool, is include/BasicClass.h; both files that include
 * this header use it. */
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
 * own call sites. BMemPMgr.c defines them K&R so their bodies can name the
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

#endif
