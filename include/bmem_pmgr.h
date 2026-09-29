#ifndef BMEM_PMGR_H
#define BMEM_PMGR_H

#include "common.h"

/**
 * @file bmem_pmgr.h
 * @brief The BMemPMgr pool allocator, the game's general-purpose heap: the
 * pool and block layouts, allocation and free, and the pool's busy flag.
 *
 * BMemPMgrInit mallocs one area, a BMemPMgr header followed by the pool's
 * blocks, and makes them one free block; main() installs it with
 * SetDefaultBMemPMgr. BMemPMgrAlloc and BMemPMgrFree split and merge blocks
 * on the pool's doubly-linked free list. Every class allocator
 * (New_<Class>) takes its object from here. The functions are defined in
 * src/app/bmem_pmgr.c. BasicClass (basic_class.h), whose list nodes come
 * from the pool, is included for the files that use both.
 */
#include "basic_class.h"

typedef struct BMemBlockHdr BMemBlockHdr;

/**
 * The header word of one block in a BMemPMgr pool and, while the block is
 * free, the two free-list links that follow it. `sizeAndFlags` packs the
 * block's byte size (header word included) into the low 28 bits and flag
 * bits into the high 4. An allocated block's links are its caller's payload.
 */
struct BMemBlockHdr {
    /* +0x000 */ u32 sizeAndFlags; /**< BMEM_SIZE_MASK: bytes; BMEM_FLAG_MASK: BMEM_FREE, BMEM_PREV_FREE */
    /* +0x004 */ BMemBlockHdr *prev; /**< free list: toward BMemPMgr::freeListHead */
    /* +0x008 */ BMemBlockHdr *next; /**< free list: toward BMemPMgr::freeListTail */
};

/** @name sizeAndFlags
 * A block's size counts its header word and its footer; the flags are the
 * block's own state and its lower neighbour's. @{ */
#define BMEM_SIZE_MASK 0x0FFFFFFF /**< the block's byte size */
#define BMEM_FLAG_MASK 0xF0000000 /**< the BMEM_FREE and BMEM_PREV_FREE bits */
#define BMEM_FREE 0x40000000      /**< this block is on the free list */
#define BMEM_PREV_FREE 0x80000000 /**< the block below this one is free */
/** @} */

/** @name Block layout
 * The header word, then the payload BMemPMgrAlloc returns. A free block's
 * last word, its footer, points back at its header, which is how
 * BMemPMgrFree finds a free lower neighbour to merge with. A block is at
 * least BMEM_MIN_BLOCK bytes: a header word and a payload that can hold the
 * two free-list links and the footer. @{ */
#define BMEM_HEADER_SIZE 4  /**< the header word before the payload */
#define BMEM_MIN_PAYLOAD 12 /**< room for the two free-list links and the footer */
#define BMEM_MIN_BLOCK 16   /**< BMEM_HEADER_SIZE plus BMEM_MIN_PAYLOAD */
#define BMEM_BLOCK_SIZE(b) ((b)->sizeAndFlags & BMEM_SIZE_MASK) /**< block b's size in bytes */
/** the block above b */
#define BMEM_NEXT_BLOCK(b) ((BMemBlockHdr *)((u8 *)(b) + BMEM_BLOCK_SIZE(b)))
/** free block b's last word */
#define BMEM_FOOTER(b) (*(BMemBlockHdr **)((u8 *)(b) + BMEM_BLOCK_SIZE(b) - 4))
/** the footer of the block below b */
#define BMEM_PREV_FOOTER(b) (*(BMemBlockHdr **)((u8 *)(b) - 4))
/** what BMemPMgrAlloc returns for b */
#define BMEM_PAYLOAD(b) ((void *)((u8 *)(b) + BMEM_HEADER_SIZE))
/** the block a payload belongs to */
#define BMEM_HEADER_OF(payload) ((BMemBlockHdr *)((u8 *)(payload) - BMEM_HEADER_SIZE))
/** @} */

typedef struct BMemPMgr BMemPMgr;

/**
 * A pool's header, at the start of the one malloc'd area that also holds its
 * blocks. The blocks begin at `firstBlock` and are walked by size, not
 * through this struct; the free ones are also linked from `freeListHead` to
 * `freeListTail`. BMemPMgrFree appends a freed block at the tail and
 * BMemPMgrAlloc searches from the tail backward.
 */
struct BMemPMgr {
    /* +0x000 */ void *firstBlock;           /**< the first block, just past this header */
    /* +0x004 */ s32 poolSize;               /**< bytes of blocks, from firstBlock */
    /* +0x008 */ BMemBlockHdr *freeListTail; /**< where frees append and allocation searches from */
    /* +0x00C */ BMemBlockHdr *freeListHead; /**< the other end of the free list */
    /* +0x010 */ s32 initialized;            /**< set to 1 by SetupBMemPMgrFreeList; no reader */
};

/** @name Pool area
 * The malloc'd area is this header, poolSize bytes of blocks, then a
 * zero-size sentinel block word that stops BMemPMgrFree's merge with the
 * block above. BMEMPMGR_HEADER_SIZE is where firstBlock starts; the struct
 * above names only the fields the code touches. @{ */
#define BMEMPMGR_HEADER_SIZE 28     /**< the pool header's bytes, before firstBlock */
#define BMEMPMGR_SENTINEL_SIZE 4    /**< the zero-size block word past the last block */
#define BMEMPMGR_MIN_POOL_SIZE 1024 /**< smaller requested pools are raised to this */
/** @} */

/** @brief Creates the game's one pool: mallocs its header, poolSize bytes of
 * blocks (at least BMEMPMGR_MIN_POOL_SIZE) and the sentinel, and makes the
 * blocks one free block. Declared without a prototype: the definition takes
 * `u32 poolSize`, and main() passes a second argument the body never reads.
 * @return the pool (a BMemPMgr), or NULL when malloc fails */
extern void *BMemPMgrInit(); /* arity-ok: main() passes a dead second argument */

/** @brief Makes `pool` the one BMemPMgrAlloc, BMemPMgrFree and
 * SetupBMemPMgrFreeList work on.
 * @param pool the pool BMemPMgrInit returned */
extern void SetDefaultBMemPMgr(BMemPMgr *pool);

/** @brief Frees ptr back to the C heap (free), not to the pool.
 * @param ptr a malloc'd block */
extern void FreeMem(void *ptr);

/* bmem_pmgr.c defines BMemPMgrAlloc and BMemPMgrFree K&R with a second
 * parameter, a fallback pool read only while no default pool is set, that no
 * caller passes, so that file does not see these one-argument prototypes. */
#ifndef BMEMPMGR_DEFINER
/** @brief Allocates from the default pool, first fit from the free list's
 * tail. `size` is rounded up to a word and to BMEM_MIN_PAYLOAD; a block that
 * would leave less than BMEM_MIN_BLOCK over is taken whole, otherwise its top
 * is split off as a new free block.
 * @param size bytes wanted; 0 allocates nothing
 * @return the payload, or NULL when no free block is large enough */
extern void *BMemPMgrAlloc(u32 size); /* arity-ok: the definition takes a fallback pool no caller passes */

/** @brief Returns a block to the default pool, merging it with free
 * neighbours below and above, and appends it to the free list's tail.
 * @param ptr a BMemPMgrAlloc payload, or NULL (nothing is freed)
 * @return NULL, for the caller to store over its pointer */
extern void *BMemPMgrFree(void *ptr); /* arity-ok: same as BMemPMgrAlloc */
#endif

/** @brief Makes the whole of the default pool (or, while none is set,
 * `pool`) one free block, with the sentinel word above it.
 * @param pool the pool to set up while no default pool is set */
extern void SetupBMemPMgrFreeList(BMemPMgr *pool);

/** @brief Sets the pool's busy flag; BMemPMgrAlloc and BMemPMgrFree set it
 * for the length of their free-list work.
 * @param busy 1 while working, 0 after */
extern void SetBMemPMgrBusy(s32 busy);

/** @brief Reads the pool's busy flag.
 * @return 1 while an allocation or free is in progress, else 0 */
extern s32 GetBMemPMgrBusy(void);

#endif
