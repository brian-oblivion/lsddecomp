/*
 * BMemPMgr -- the game's pool allocator and the first half of BasicClass,
 * two things that share this file:
 *
 *  - The BMemPMgr pool allocator, the game's general-purpose allocator.
 *    BMemPMgrInit mallocs one area, a BMemPMgr header followed by the pool's
 *    blocks (the layout is in BMemPMgr.h), and SetupBMemPMgrFreeList makes
 *    the whole pool one free block. BMemPMgrAlloc and BMemPMgrFree split and
 *    merge blocks on the pool's free list. SetupBMemPMgrFreeList,
 *    BMemPMgrAlloc and BMemPMgrFree work on sDefaultBMemPMgr
 *    (SetDefaultBMemPMgr) and fall back to their pool argument only while
 *    no default is set.
 *  - BasicClass's methods (include/basic_class.h), with
 *    PushBasicClassListNode/RemoveBasicClassListNode, the pool-backed list
 *    primitives its `children` and `parentRefs` lists share, and the list
 *    helpers, table getter and notification pair that close the file, with
 *    the pool's busy-flag accessors.
 */

#include "common.h"
/* MATCHING: BMemPMgrAlloc/Free are defined K&R with a second parameter (BMemPMgr.h) */
#define BMEMPMGR_DEFINER
#include "BMemPMgr.h"
#include <malloc.h>
#include <stdio.h>

/* The pool SetupBMemPMgrFreeList, BMemPMgrAlloc and BMemPMgrFree work on;
 * set by SetDefaultBMemPMgr (main.c, right after BMemPMgrInit). */
extern BMemPMgr *sDefaultBMemPMgr;

/* "bMemPMgr = %p, poolSize = %ld in BMemPMgrInit\n", BMemPMgrInit's
 * malloc-failure message. */
extern char sBMemPMgrInitFailFmt[];

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
    sDefaultBMemPMgr = pool;
}

void FreeMem(void *ptr) {
    free(ptr);
}

void SetupBMemPMgrFreeList(BMemPMgr *pool) {
    BMemPMgr *mgr;
    BMemBlockHdr *header;
    BMemBlockHdr *sentinel;

    mgr = sDefaultBMemPMgr;
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

/* First fit, searching the free list from its tail. `size` is rounded up to
 * a word and to BMEM_MIN_PAYLOAD; a block that would leave less than
 * BMEM_MIN_BLOCK over is taken whole, otherwise its top is split off as a new
 * free block that replaces it on the list. Returns the payload, or NULL.
 *
 * Defined K&R so the body can read the second argument that callers never
 * pass (BMemPMgr.h, at the declaration). */
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
    mgr = sDefaultBMemPMgr;
    if (mgr == NULL) {
        mgr = pool;
    }
    if (size != 0) {
        if (size & 0x3) {
            /* MATCHING: one expression reassociates the + 4 into the subtract. */
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
                    /* MATCHING: nextBlock is one variable in both arms, or it takes the wrong register. */
                    nextBlock = BMEM_NEXT_BLOCK(cursor);
                    nextBlock->sizeAndFlags &= ~BMEM_PREV_FREE;
                    /* MATCHING: every unlink scopes its own n/p; shared locals take other registers. */
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

                        /* MATCHING: the dead store to `unused` picks the store's register. */
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

/* Merges the block with a free lower neighbour (found through that block's
 * footer) and a free upper neighbour, then appends the result to the free
 * list's tail. K&R for the same reason as BMemPMgrAlloc. */
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
    mgr = sDefaultBMemPMgr;
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
            /* MATCHING: every unlink scopes its own n/p; shared locals take other registers. */
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

                /* MATCHING: reusing the dead nextSize for the test picks its register. */
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
    self->methods = GetBasicClassMethods();
    self->parentRefs = NULL;
    self->children = NULL;
}

void BasicClass__Finalize(BasicClass *self) {
    self->methods->notifyParents(self, BASICCLASS_EVENT_FINALIZED);
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

    /* MATCHING: the named childPtr and the if/do-while with comma tests keep
     * &child in one saved register and the loop body first. */
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

/* Set to 1 by BMemPMgrAlloc and BMemPMgrFree for the length of their free-list
 * work and back to 0 after (SetBMemPMgrBusy, GetBMemPMgrBusy below). Nothing
 * in either waits on it. */
extern s32 sBMemPMgrBusy;

void FreeBasicClassList(BasicClassListNode **head) {
    BasicClassListNode *node = *head;

    while (node != NULL) {
        BasicClassListNode *cur = node;
        node = node->next;
        BMemPMgrFree(cur);
    }
}

/* BasicClassMethods slot +0x030. Tell every object holding a reference to
 * `self` that `event` happened, by calling each one's onNotify slot with
 * `self` as the sender. BasicClass__Finalize passes 1, "going away". */
void BasicClass__NotifyParents(BasicClass *self, s32 event) {
    BasicClassListNode *cursor = self->parentRefs;
    BasicClass *parent;

    for (GetNextBasicClass(&parent, &cursor); parent != NULL; GetNextBasicClass(&parent, &cursor)) {
        parent->methods->onNotify(parent, self, event);
    }
}

/* BasicClassMethods slot +0x034. Empty, and no BasicClass-derived table
 * overrides it. */
void BasicClass__NoOpSlot34(void) {}

/* BasicClassMethods slot +0x038, the receiving half of NotifyParents:
 * `sender` is telling `self` that `event` happened. The base class treats
 * BASICCLASS_EVENT_FINALIZED as "sender is going away" and drops it from its own children.
 * Subclasses override it, call this first, then look at the sender's class
 * tag as well, so `event` is a notification code, not a boolean. */
void BasicClass__OnNotify(BasicClass *self, void *sender, s32 event) {
    if (event == BASICCLASS_EVENT_FINALIZED) {
        self->methods->removeChild(self, (BasicClass *)sender);
    }
}

BasicClassMethods *GetBasicClassMethods(void) {
    return &gBasicClassMethods;
}

void GetNextBasicClass(BasicClass **outValue, BasicClassListNode **cursor) {
    if (*cursor != NULL) {
        *outValue = (*cursor)->value;
        *cursor = (*cursor)->next;
    } else {
        *outValue = NULL;
    }
}

void ReleaseBasicClassArray(BasicClass **array, s32 count) {
    if (count-- > 0) {
        do {
            *array = (BasicClass *)(*array)->methods->release(*array);
            array++;
        } while (count-- > 0);
    }
}

void SetBMemPMgrBusy(s32 busy) {
    sBMemPMgrBusy = busy;
}

s32 GetBMemPMgrBusy(void) {
    return sBMemPMgrBusy;
}
