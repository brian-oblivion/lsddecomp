#include "common.h"
#include "code_8220.h"

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

void *BMemPMgrInit(s32 poolSize)
{
    BMemPMgr *pool;

    if ((u32)poolSize < 0x400) {
        poolSize = 0x400;
    }
    pool = malloc(poolSize + 0x20);
    if (pool != NULL) {
        pool->freeListHead = (u8 *)pool + 0x1C;
        pool->poolSize = poolSize;
        func_80017AC8(pool);
    } else {
        printf(D_8001028C, NULL, poolSize);
    }
    return pool;
}

void func_80017A9C(BMemPMgr *pool)
{
    D_8008A818 = pool;
}

void func_80017AA8(void *ptr)
{
    free(ptr);
}

void func_80017AC8(BMemPMgr *pool)
{
    BMemPMgr *mgr;
    BMemBlockHdr *header;
    u8 *end;

    mgr = D_8008A818;
    if (mgr == NULL) {
        mgr = pool;
    }
    mgr->unk10 = 1;
    header = mgr->freeListHead;
    mgr->freeListStart = header;
    mgr->freeListEnd = header;
    header->sizeAndFlags = mgr->poolSize | 0x40000000;
    mgr->freeListStart->prev = NULL;
    mgr->freeListEnd->next = NULL;
    end = (u8 *)header + (header->sizeAndFlags & 0xFFFFFFF);
    *(BMemBlockHdr **)(end - 4) = header;
    *(u32 *)end = 0x80000000;
}

void *func_80017B34(size, pool)
    s32 size;
    void *pool;
{
    BMemPMgr *mgr;
    BMemBlockHdr *cursor;
    BMemBlockHdr *result;
    BMemBlockHdr *remainder;
    BMemBlockHdr *unused;
    u32 blockSize;
    u32 padded;

    SetBMemPMgrBusy(1);
    result = NULL;
    mgr = D_8008A818;
    if (mgr == NULL) {
        mgr = pool;
    }
    if (size != 0) {
        if (size & 0x3) {
            padded = size + 4;
            size = padded - (size & 0x3);
        }
        if ((u32)size < 0xC) {
            size = 0xC;
        }
        cursor = mgr->freeListStart;
        size += 4;
        while (cursor != NULL) {
            blockSize = cursor->sizeAndFlags & 0xFFFFFFF;
            if (blockSize >= (u32)size) {
                cursor->sizeAndFlags &= 0xBFFFFFFF;
                result = (BMemBlockHdr *)((u8 *)cursor + 4);
                if (blockSize < (u32)size + 0x10) {
                    remainder = (BMemBlockHdr *)((u8 *)cursor + (cursor->sizeAndFlags & 0xFFFFFFF));
                    remainder->sizeAndFlags &= 0x7FFFFFFF;
                    {
                        BMemBlockHdr *n = cursor->next;
                        BMemBlockHdr *p = cursor->prev;

                        if (p != NULL) {
                            p->next = n;
                        } else {
                            mgr->freeListEnd = n;
                        }
                    }
                    {
                        BMemBlockHdr *n = cursor->next;
                        BMemBlockHdr *p;

                        p = unused = cursor->prev;
                        if (n != NULL) {
                            n->prev = p;
                        } else {
                            mgr->freeListStart = p;
                        }
                    }
                } else {
                    cursor->sizeAndFlags = (cursor->sizeAndFlags & 0xF0000000) | (u32)size;
                    remainder = (BMemBlockHdr *)((u8 *)cursor + (cursor->sizeAndFlags & 0xFFFFFFF));
                    remainder->sizeAndFlags = (blockSize - (u32)size) | 0x40000000;
                    remainder->prev = cursor->prev;
                    remainder->next = cursor->next;
                    {
                        BMemBlockHdr *p = cursor->prev;

                        if (p != NULL) {
                            p->next = remainder;
                        } else {
                            mgr->freeListEnd = remainder;
                        }
                    }
                    {
                        BMemBlockHdr *n = cursor->next;

                        if (n != NULL) {
                            n->prev = remainder;
                        } else {
                            mgr->freeListStart = remainder;
                        }
                    }
                    *(BMemBlockHdr **)((u8 *)remainder + (remainder->sizeAndFlags & 0xFFFFFFF) - 4) = remainder;
                }
                break;
            }
            cursor = cursor->prev;
        }
    }
    SetBMemPMgrBusy(0);
    return result;
}

void *func_80017CFC(ptr, pool)
    void *ptr;
    void *pool;
{
    BMemPMgr *mgr;
    BMemBlockHdr *header;
    BMemBlockHdr *next;
    u32 nextFree;

    SetBMemPMgrBusy(1);
    mgr = D_8008A818;
    if (mgr == NULL) {
        mgr = pool;
    }
    if (ptr != NULL) {
        header = (BMemBlockHdr *)((u8 *)ptr - 4);
        next = (BMemBlockHdr *)((u8 *)header + (header->sizeAndFlags & 0xFFFFFFF));
        nextFree = next->sizeAndFlags & 0x40000000;
        if ((s32)header->sizeAndFlags < 0) {
            u32 freedSize = header->sizeAndFlags & 0xFFFFFFF;

            header = *(BMemBlockHdr **)((u8 *)ptr - 8);
            header->sizeAndFlags = (header->sizeAndFlags & 0xF0000000)
                | (freedSize + (header->sizeAndFlags & 0xFFFFFFF));
            {
                BMemBlockHdr *n = header->next;
                BMemBlockHdr *p = header->prev;

                if (p != NULL) {
                    p->next = n;
                } else {
                    mgr->freeListEnd = n;
                }
            }
            {
                BMemBlockHdr *p = header->prev;
                BMemBlockHdr *n = header->next;

                if (n != NULL) {
                    n->prev = p;
                } else {
                    mgr->freeListStart = p;
                }
            }
        }
        if (nextFree) {
            u32 nextSize = next->sizeAndFlags & 0xFFFFFFF;

            header->sizeAndFlags = (header->sizeAndFlags & 0xF0000000) | (nextSize + (header->sizeAndFlags & 0xFFFFFFF));
            {
                BMemBlockHdr *n = next->next;
                BMemBlockHdr *p = next->prev;

                if (p != NULL) {
                    p->next = n;
                } else {
                    mgr->freeListEnd = n;
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
                    mgr->freeListStart = p;
                }
            }
            next = (BMemBlockHdr *)((u8 *)header + (header->sizeAndFlags & 0xFFFFFFF));
        }
        header->prev = mgr->freeListStart;
        mgr->freeListStart = header;
        header->next = NULL;
        if (header->prev != NULL) {
            header->prev->next = header;
        } else {
            mgr->freeListEnd = header;
        }
        *(BMemBlockHdr **)((u8 *)next - 4) = header;
        header->sizeAndFlags |= 0x40000000;
        next->sizeAndFlags |= 0x80000000;
    }
    SetBMemPMgrBusy(0);
    return NULL;
}

void func_80017EA8(void) {
}

void *BasicClass__func_17eb0(BasicClass *self)
{
    self->methods->finalize(self);
    func_80017CFC(self);
    return NULL;
}

void BasicClass__BasicClass(BasicClass *self)
{
    self->methods = Get_vtable_BasicClass();
    self->parentRefs = NULL;
    self->children = NULL;
}

void BasicClass__func_17f2c(BasicClass *self)
{
    self->methods->notifyParents(self, 1);
    self->methods->removeAllChildren(self);
    self->methods->clearParentRefs(self);
}

void BasicClass__func_17f98(BasicClass *self, BasicClass *child)
{
    if (func_800181AC(&self->children, child)) {
        child->methods->addParentRef(child, self);
    }
}

void BasicClass__func_17ff0(BasicClass *self, BasicClass *child)
{
    func_80018208(&self->children, child);
    child->methods->removeParentRef(child, self);
}

void BasicClass__func_18040(BasicClass *self)
{
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

void BasicClass__func_180bc(BasicClass *self, BasicClass **outChild, BasicClassListNode **cursor)
{
    if (*outChild == NULL) {
        *cursor = self->children;
    }
    GetNextBasicClass(outChild, cursor);
}

s32 BasicClass__func_180fc(BasicClass *self, BasicClass *parent)
{
    return func_800181AC(&self->parentRefs, parent);
}

void BasicClass__func_1811c(BasicClass *self, BasicClass *parent)
{
    func_80018208(&self->parentRefs, parent);
}

void BasicClass__func_1813c(BasicClass *self)
{
    FreeBasicClassList(&self->parentRefs);
    self->parentRefs = NULL;
}

void BasicClass__func_1816c(BasicClass *self, BasicClass **outParent, BasicClassListNode **cursor)
{
    if (*outParent == NULL) {
        *cursor = self->parentRefs;
    }
    GetNextBasicClass(outParent, cursor);
}

s32 func_800181AC(BasicClassListNode **head, BasicClass *value)
{
    BasicClassListNode *node;
    BasicClassListNode *oldHead;

    node = func_80017B34(0x8);
    if (node != NULL) {
        oldHead = *head;
        node->value = value;
        node->next = oldHead;
        *head = node;
        return 1;
    }
    return 0;
}

void func_80018208(BasicClassListNode **head, BasicClass *value)
{
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
            func_80017CFC(node);
            return;
        }
        prev = node;
        node = node->next;
    }
}
