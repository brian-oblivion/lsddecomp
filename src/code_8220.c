#include "common.h"
#include "code_8220.h"

void *BMemPMgrInit(s32 poolSize)
{
    BMemPMgr *pool;

    if ((u32)poolSize < 0x400) {
        poolSize = 0x400;
    }
    pool = func_80011D34(poolSize + 0x20);
    if (pool != NULL) {
        pool->freeListHead = (u8 *)pool + 0x1C;
        pool->poolSize = poolSize;
        func_80017AC8(pool);
    } else {
        func_80012C20(D_8001028C, NULL, poolSize);
    }
    return pool;
}

INCLUDE_ASM("asm/nonmatchings/code_8220", func_80017A9C);

s32 func_80017AA8(void *ptr)
{
    return func_80011F68(ptr);
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

INCLUDE_ASM("asm/nonmatchings/code_8220", BasicClass__func_18040);

INCLUDE_ASM("asm/nonmatchings/code_8220", BasicClass__func_180bc);

INCLUDE_ASM("asm/nonmatchings/code_8220", BasicClass__func_180fc);

INCLUDE_ASM("asm/nonmatchings/code_8220", BasicClass__func_1811c);

INCLUDE_ASM("asm/nonmatchings/code_8220", BasicClass__func_1813c);

INCLUDE_ASM("asm/nonmatchings/code_8220", BasicClass__func_1816c);

INCLUDE_ASM("asm/nonmatchings/code_8220", func_800181AC);

INCLUDE_ASM("asm/nonmatchings/code_8220", func_80018208);
