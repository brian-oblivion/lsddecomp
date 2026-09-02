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

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_c", func_8004D300);

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

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_c", func_8004D47C);

void *func_8004D500(void *self)
{
    return self;
}

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_c", func_8004D508);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_c", func_8004D518);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_c", func_8004D578);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_c", func_8004D678);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_c", func_8004D6AC);
