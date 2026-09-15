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

INCLUDE_ASM("asm/nonmatchings/code_8220", func_80017AC8);

INCLUDE_ASM("asm/nonmatchings/code_8220", func_80017B34);

INCLUDE_ASM("asm/nonmatchings/code_8220", func_80017CFC);

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
    self->methods = func_80018390();
    self->parentRefs = NULL;
    self->children = NULL;
}

void BasicClass__func_17f2c(BasicClass *self)
{
    self->methods->onFinalize(self, 1);
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
    if (func_800183A0(childPtr, &cursor), child != NULL) {
        do {
            self->methods->removeChild(self, child);
        } while (func_800183A0(childPtr, &cursor), child != NULL);
    }
}

void BasicClass__func_180bc(BasicClass *self, BasicClass **outChild, BasicClassListNode **cursor)
{
    if (*outChild == NULL) {
        *cursor = self->children;
    }
    func_800183A0(outChild, cursor);
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
    func_80018288(&self->parentRefs);
    self->parentRefs = NULL;
}

void BasicClass__func_1816c(BasicClass *self, BasicClass **outParent, BasicClassListNode **cursor)
{
    if (*outParent == NULL) {
        *cursor = self->parentRefs;
    }
    func_800183A0(outParent, cursor);
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
