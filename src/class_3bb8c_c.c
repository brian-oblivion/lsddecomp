#include "common.h"
#include "class_3bb8c.h"

Class869D8 *func_8004D254(void)
{
    Class869D8 *self;

    self = func_80017B34(0xDC);
    if (self != NULL) {
        func_8004D37C()->ctor(self);
        return self;
    }
    return NULL;
}

void func_8004D2A4(Class869D8 *self)
{
    func_8003F24C()->ctor(self);
    self->methods = func_8004D37C();
    self->methods->slot40(self);
}

void func_8004D2F8(void) {
}

void func_8004D300(Class869D8 *self)
{
    if (self->unk10 != 0 && self->unk70 != 0) {
        func_8003F24C()->slot9C(self);
    }
}

void func_8004D35C(void) {
}

void func_8004D364(void) {
}

void func_8004D36C(void) {
}

void func_8004D374(void) {
}

Class869D8Methods *func_8004D37C(void)
{
    return &D_800869D8;
}

Class86AA0 *func_8004D38C(void)
{
    Class86AA0 *self;

    self = func_80017B34(0x3C);
    if (self != NULL) {
        func_8004D508()->ctor(self);
        return self;
    }
    return NULL;
}

void func_8004D3DC(Class86AA0 *self)
{
    func_8001E57C(self)->ctor(self);
    self->methods = func_8004D508();
    self->unk34 = 0;
    self->unk36 = 0;
    self->unk38 = 0;
}

void func_8004D42C(void) {
}

void func_8004D434(Class86AA0 *self, GenericTagInst_3bb8c_c *arg1)
{
    if (arg1->methods->tag == 0x34) {
        self->methods->slotB8(self);
    }
}

void func_8004D47C(Class86AA0 *self, GenericTagInst_3bb8c_c *arg1, s32 arg2)
{
    func_8001E57C(self)->slot9C(self, arg1, arg2);
    if (arg2 >= 9) {
        return;
    }
    do {
        if (arg2 < 5) {
            return;
        }
    } while (0);
    self->methods->slotA0(self, arg1, arg2);
}

void *func_8004D500(void *self)
{
    return self;
}

Class86AA0Methods *func_8004D508(void)
{
    return &D_80086AA0;
}

Class86B60 *func_8004D518(void *dreamSys)
{
    Class86B60 *self;

    self = func_80017B34(0xC4);
    if (self != NULL) {
        func_8004E2D0()->ctor(self, dreamSys);
        return self;
    }
    return NULL;
}

void func_8004D578(Class86B60 *self, void *dreamSys)
{
    DreamSysView_3bb8c_c *dream;
    Class86B60Unk48Obj *obj;

    func_8003DFBC()->slot08(self, &D_80086D44, &D_800114DC, 0);
    self->methods = func_8004E2D0();
    obj = self->unk48;
    obj->methods->slot9C(obj, -1);
    self->unkA4 = dreamSys;
    self->unkAC = 0;
    dream = dreamSys;
    self->unkBC = dream->methods->slot1B0(dream, &self->unkC0);
    func_8004D6AC(dream->methods->slot1A0(dream, 0));
    self->methods->slotD8(self, &D_80086D44);
    self->methods->slot40(self, dreamSys);
}

void func_8004D678(Ctx678_3bb8c_c *ctx, Result678_3bb8c_c *out)
{
    Obj866E8 *target = ctx->target;
    s32 flag = 1;

    if (target->unkC > 9999999) {
        flag = (target->unk2F4 == 0);
    }
    out->block[1] = flag;
}

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_c", func_8004D6AC);
