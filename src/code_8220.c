#include "common.h"
#include "code_8220.h"

/* This unit holds two unrelated things, decomp-adjacent by ROM address
 * only:
 *
 *  - The `BMemPMgr` pool allocator: `BMemPMgrInit` carves a block out of
 *    the Psy-Q heap (`malloc`) and hands it to `SetupBMemPMgrFreeList`,
 *    which threads it onto a single doubly-linked free list of
 *    `BMemBlockHdr` nodes. `BMemPMgrAlloc`/`BMemPMgrFree` split and
 *    coalesce blocks off that list; both resolve their target pool
 *    through the global `gDefaultBMemPMgr` (set by `SetDefaultBMemPMgr`)
 *    when the caller doesn't name one directly. It is the game's
 *    general-purpose small-object allocator -- called from a wide
 *    cross-section of units, not just this one.
 *  - `BasicClass`, the game's hand-rolled root class
 *    (`docs/research/class-framework.md`; full design in the
 *    `BasicClass`/`BasicClassMethods` comment in `code_8220.h`). Its
 *    fourteen virtual methods live here; `PushBasicClassListNode`/
 *    `RemoveBasicClassListNode` are the pool-backed list primitives both
 *    of its linked lists (`children`, `parentRefs`) share.
 *
 * See `code_8220_b`/`code_8220_c` for this unit's siblings (GTE/GPU
 * primitive code, unrelated to either of the above).
 */

/* Psy-Q heap, linked from Sony's own object (`_obj/malloc`) rather than
 * decompiled, so these carry Sony's exported names. The real prototypes are
 * in <malloc.h>; they are restated here rather than included because no unit
 * in this project pulls in the SDK headers yet. Local and not in
 * code_8220.h deliberately -- that header is shared with five other units,
 * and a second `free`/`malloc` declaration there would collide with
 * <malloc.h> in whichever of them includes it first. */
extern void *malloc(unsigned int size);
extern void free(void *ptr);

/* Psy-Q printf (libc2/printf). Variadic, and declared here with the argument
 * shape THIS call site passes -- the project's four other printf call sites
 * each declare their own, which is why no single declaration is shared. */
extern void printf(const char *fmt, void *arg1, s32 arg2);

void *BMemPMgrInit(s32 poolSize) {
    BMemPMgr *pool;

    if ((u32)poolSize < BMEMPMGR_MIN_POOL_SIZE) {
        poolSize = BMEMPMGR_MIN_POOL_SIZE;
    }
    pool = malloc(BMEMPMGR_HEADER_SIZE + poolSize + BMEMPMGR_SENTINEL_SIZE);
    if (pool != NULL) {
        pool->firstBlock = (u8 *)pool + BMEMPMGR_HEADER_SIZE;
        pool->poolSize = poolSize;
        SetupBMemPMgrFreeList(pool);
    } else {
        printf(sBMemPMgrInitFailFmt, NULL, poolSize);
    }
    return pool;
}

void SetDefaultBMemPMgr(BMemPMgr *pool) {
    gDefaultBMemPMgr = pool;
}

void FreeMem(void *ptr) {
    free(ptr);
}

void SetupBMemPMgrFreeList(BMemPMgr *pool) {
    BMemPMgr *mgr;
    BMemBlockHdr *header;
    BMemBlockHdr *sentinel;

    mgr = gDefaultBMemPMgr;
    if (mgr == NULL) {
        mgr = pool;
    }
    mgr->initialized = 1;
    header = mgr->firstBlock;
    mgr->freeListTail = header;
    mgr->freeListHead = header;
    header->sizeAndFlags = mgr->poolSize | BMEM_FREE;
    mgr->freeListTail->prev = NULL;
    mgr->freeListHead->next = NULL;
    sentinel = BMEM_NEXT_BLOCK(header);
    BMEM_PREV_FOOTER(sentinel) = header;
    sentinel->sizeAndFlags = BMEM_PREV_FREE;
}

/* clang-format off */
void *BMemPMgrAlloc(size, pool)
    s32 size;
    BMemPMgr *pool;
{
    /* clang-format on */
    BMemPMgr *mgr;
    BMemBlockHdr *cursor;
    void *result;
    BMemBlockHdr *nextBlock;
    BMemBlockHdr *unused;
    u32 blockSize;
    u32 padded;

    SetBMemPMgrBusy(1);
    result = NULL;
    mgr = gDefaultBMemPMgr;
    if (mgr == NULL) {
        mgr = pool;
    }
    if (size != 0) {
        if (size & 0x3) {
            padded = size + 4;
            size = padded - (size & 0x3);
        }
        if ((u32)size < BMEM_MIN_PAYLOAD) {
            size = BMEM_MIN_PAYLOAD;
        }
        cursor = mgr->freeListTail;
        size += BMEM_HEADER_SIZE;
        while (cursor != NULL) {
            blockSize = BMEM_BLOCK_SIZE(cursor);
            if (blockSize >= (u32)size) {
                cursor->sizeAndFlags &= ~BMEM_FREE;
                result = BMEM_PAYLOAD(cursor);
                if (blockSize < (u32)size + BMEM_MIN_BLOCK) {
                    nextBlock = BMEM_NEXT_BLOCK(cursor);
                    nextBlock->sizeAndFlags &= ~BMEM_PREV_FREE;
                    {
                        BMemBlockHdr *n = cursor->next;
                        BMemBlockHdr *p = cursor->prev;

                        if (p != NULL) {
                            p->next = n;
                        } else {
                            mgr->freeListHead = n;
                        }
                    }
                    {
                        BMemBlockHdr *n = cursor->next;
                        BMemBlockHdr *p;

                        p = unused = cursor->prev;
                        if (n != NULL) {
                            n->prev = p;
                        } else {
                            mgr->freeListTail = p;
                        }
                    }
                } else {
                    cursor->sizeAndFlags = (cursor->sizeAndFlags & BMEM_FLAG_MASK) | (u32)size;
                    nextBlock = BMEM_NEXT_BLOCK(cursor);
                    nextBlock->sizeAndFlags = (blockSize - (u32)size) | BMEM_FREE;
                    nextBlock->prev = cursor->prev;
                    nextBlock->next = cursor->next;
                    {
                        BMemBlockHdr *p = cursor->prev;

                        if (p != NULL) {
                            p->next = nextBlock;
                        } else {
                            mgr->freeListHead = nextBlock;
                        }
                    }
                    {
                        BMemBlockHdr *n = cursor->next;

                        if (n != NULL) {
                            n->prev = nextBlock;
                        } else {
                            mgr->freeListTail = nextBlock;
                        }
                    }
                    BMEM_FOOTER(nextBlock) = nextBlock;
                }
                break;
            }
            cursor = cursor->prev;
        }
    }
    SetBMemPMgrBusy(0);
    return result;
}

/* clang-format off */
void *BMemPMgrFree(ptr, pool)
    void *ptr;
    BMemPMgr *pool;
{
    /* clang-format on */
    BMemPMgr *mgr;
    BMemBlockHdr *header;
    BMemBlockHdr *next;
    u32 nextFree;

    SetBMemPMgrBusy(1);
    mgr = gDefaultBMemPMgr;
    if (mgr == NULL) {
        mgr = pool;
    }
    if (ptr != NULL) {
        header = BMEM_HEADER_OF(ptr);
        next = BMEM_NEXT_BLOCK(header);
        nextFree = next->sizeAndFlags & BMEM_FREE;
        if (header->sizeAndFlags & BMEM_PREV_FREE) {
            u32 freedSize = BMEM_BLOCK_SIZE(header);

            header = BMEM_PREV_FOOTER(header);
            header->sizeAndFlags =
                (header->sizeAndFlags & BMEM_FLAG_MASK) | (freedSize + BMEM_BLOCK_SIZE(header));
            {
                BMemBlockHdr *n = header->next;
                BMemBlockHdr *p = header->prev;

                if (p != NULL) {
                    p->next = n;
                } else {
                    mgr->freeListHead = n;
                }
            }
            {
                BMemBlockHdr *p = header->prev;
                BMemBlockHdr *n = header->next;

                if (n != NULL) {
                    n->prev = p;
                } else {
                    mgr->freeListTail = p;
                }
            }
        }
        if (nextFree) {
            u32 nextSize = BMEM_BLOCK_SIZE(next);

            header->sizeAndFlags =
                (header->sizeAndFlags & BMEM_FLAG_MASK) | (nextSize + BMEM_BLOCK_SIZE(header));
            {
                BMemBlockHdr *n = next->next;
                BMemBlockHdr *p = next->prev;

                if (p != NULL) {
                    p->next = n;
                } else {
                    mgr->freeListHead = n;
                }
            }
            {
                BMemBlockHdr *p = next->prev;
                BMemBlockHdr *n = next->next;

                /* nextSize is dead by this point (its one real use, the
                 * sizeAndFlags fold above, already happened); reusing it
                 * to hold the branch condition here -- instead of a fresh
                 * anonymous temporary -- is what puts this comparison in
                 * the register retail uses. Found by permuter search. */
                nextSize = n != NULL;
                if (nextSize) {
                    n->prev = p;
                } else {
                    mgr->freeListTail = p;
                }
            }
            next = BMEM_NEXT_BLOCK(header);
        }
        header->prev = mgr->freeListTail;
        mgr->freeListTail = header;
        header->next = NULL;
        if (header->prev != NULL) {
            header->prev->next = header;
        } else {
            mgr->freeListHead = header;
        }
        BMEM_PREV_FOOTER(next) = header;
        header->sizeAndFlags |= BMEM_FREE;
        next->sizeAndFlags |= BMEM_PREV_FREE;
    }
    SetBMemPMgrBusy(0);
    return NULL;
}

void NoOp5(void) {}

void *BasicClass__Release(BasicClass *self) {
    self->methods->finalize(self);
    BMemPMgrFree(self);
    return NULL;
}

void BasicClass__BasicClass(BasicClass *self) {
    self->methods = Get_vtable_BasicClass();
    self->parentRefs = NULL;
    self->children = NULL;
}

void BasicClass__Finalize(BasicClass *self) {
    self->methods->notifyParents(self, 1);
    self->methods->removeAllChildren(self);
    self->methods->clearParentRefs(self);
}

void BasicClass__AddChild(BasicClass *self, BasicClass *child) {
    if (PushBasicClassListNode(&self->children, child)) {
        child->methods->addParentRef(child, self);
    }
}

void BasicClass__RemoveChild(BasicClass *self, BasicClass *child) {
    RemoveBasicClassListNode(&self->children, child);
    child->methods->removeParentRef(child, self);
}

void BasicClass__RemoveAllChildren(BasicClass *self) {
    BasicClass *child;
    BasicClass **childPtr;
    BasicClassListNode *cursor;

    childPtr = &child;
    cursor = self->children;
    if (GetNextBasicClass(childPtr, &cursor), child != NULL) {
        do {
            self->methods->removeChild(self, child);
        } while (GetNextBasicClass(childPtr, &cursor), child != NULL);
    }
}

void BasicClass__GetNextChild(BasicClass *self, BasicClass **outChild, BasicClassListNode **cursor) {
    if (*outChild == NULL) {
        *cursor = self->children;
    }
    GetNextBasicClass(outChild, cursor);
}

s32 BasicClass__AddParentRef(BasicClass *self, BasicClass *parent) {
    return PushBasicClassListNode(&self->parentRefs, parent);
}

void BasicClass__RemoveParentRef(BasicClass *self, BasicClass *parent) {
    RemoveBasicClassListNode(&self->parentRefs, parent);
}

void BasicClass__ClearParentRefs(BasicClass *self) {
    FreeBasicClassList(&self->parentRefs);
    self->parentRefs = NULL;
}

void BasicClass__GetNextParentRef(BasicClass *self, BasicClass **outParent, BasicClassListNode **cursor) {
    if (*outParent == NULL) {
        *cursor = self->parentRefs;
    }
    GetNextBasicClass(outParent, cursor);
}

s32 PushBasicClassListNode(BasicClassListNode **head, BasicClass *value) {
    BasicClassListNode *node;
    BasicClassListNode *oldHead;

    node = BMemPMgrAlloc(sizeof(BasicClassListNode));
    if (node != NULL) {
        oldHead = *head;
        node->value = value;
        node->next = oldHead;
        *head = node;
        return 1;
    }
    return 0;
}

void RemoveBasicClassListNode(BasicClassListNode **head, BasicClass *value) {
    BasicClassListNode *prev;
    BasicClassListNode *node;

    prev = NULL;
    node = *head;
    while (node != NULL) {
        if (node->value == value) {
            if (prev != NULL) {
                prev->next = node->next;
            } else {
                *head = node->next;
            }
            BMemPMgrFree(node);
            return;
        }
        prev = node;
        node = node->next;
    }
}
