#include "common.h"
#include "code_d294.h"

Class6B5CCObj *func_8001CA94(void) {
    Class6B5CCObj *obj;

    obj = func_80017B34(0x44);
    if (obj == NULL) {
        return NULL;
    }
    if (func_8001E57C()->ctor(obj) != NULL) {
        return obj;
    }
    func_80017CFC(obj);
    return NULL;
}

void *func_8001CAF4(Class6B5CCObj *self) {
    void *blockB;

    self->unk14 = func_80017B34(0x50);
    if (self->unk14 == NULL) {
        return NULL;
    }
    blockB = func_80017B34(0x28);
    self->unk14->unk44 = blockB;
    if (blockB == NULL) {
        func_80017CFC(self->unk14);
        return NULL;
    }
    func_80018390()->ctor(self);
    self->methods = func_8001E57C();
    self->unk20 = 0;
    self->unk18 = 0;
    self->unkC = NULL;
    self->unk14->unk48 = 0;
    self->methods->slot40(self);
    return self;
}

void func_8001CBA4(Class6B5CCObj *self) {
    Class6B5CCSub14 *sub;

    self->methods->slot50(self);
    self->methods->slot54(self);
    self->methods->slot5C(self, 0);
    sub = self->unk14;
    func_80017CFC(sub->unk44);
    func_80017CFC(self->unk14);
    func_80018390()->dtor(self);
}

INCLUDE_ASM("asm/nonmatchings/code_d294", func_8001CC48);

INCLUDE_ASM("asm/nonmatchings/code_d294", func_8001CCB4);

INCLUDE_ASM("asm/nonmatchings/code_d294", func_8001CD20);

INCLUDE_ASM("asm/nonmatchings/code_d294", func_8001CD60);

INCLUDE_ASM("asm/nonmatchings/code_d294", func_8001CE30);

INCLUDE_ASM("asm/nonmatchings/code_d294", func_8001CEB4);

INCLUDE_ASM("asm/nonmatchings/code_d294", func_8001D008);

INCLUDE_ASM("asm/nonmatchings/code_d294", func_8001D0EC);

INCLUDE_ASM("asm/nonmatchings/code_d294", func_8001D1A4);

INCLUDE_ASM("asm/nonmatchings/code_d294", func_8001D204);

INCLUDE_ASM("asm/nonmatchings/code_d294", func_8001D280);

void func_8001D33C(void) {
}

s32 func_8001D344(Class6B5CCObj *self, s32 a1) {
    return func_8001EDAC(&self->unk10, 0x1F, 1, a1 == 0) == 0;
}

u32 func_8001D374(Class6B5CCObj *self, s32 a1) {
    return func_8001EDAC(&self->unk10, 0x1E, 1, a1 != 0);
}

u32 func_8001D3A0(Class6B5CCObj *self, u32 a1) {
    return func_8001EDAC(&self->unk10, 0x1C, 2, a1);
}

u32 func_8001D3CC(Class6B5CCObj *self, s32 a1) {
    return func_8001EDAC(&self->unk10, 6, 1, a1 == 0);
}

u32 func_8001D3F8(Class6B5CCObj *self, u32 a1) {
    return func_8001EDAC(&self->unk10, 3, 3, a1);
}
