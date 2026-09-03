#include "common.h"
#include "class_3bb8c.h"

void func_8004D704(Class86B60 *self)
{
    if (self->unkAC != NULL) {
        self->unkAC->methods->release(self->unkAC);
        self->unkA8->methods->release(self->unkA8);
    }
    func_8003DFBC()->slot0C(self);
}

void func_8004D788(Class86B60 *self, GenericHeaderObj_3bb8c_d *arg1, s32 arg2)
{
    func_8003DFBC()->slot38(self, arg1, arg2);
    if ((arg1->methods->header & 0xF) == 0xB) {
        self->methods->slot138(self, arg1, arg2);
    }
}

void func_8004D814(Class86B60 *self)
{
    self->unk34 = 0;
    self->unk2C = 0x190;
    self->methods->slotD4(self, &D_800114E8, 0);
    self->methods->slot6C(self, 0xA);
    self->unkA4->methods->slotF0(self->unkA4, 0, 0);
}

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_d", func_8004D898);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_d", func_8004D90C);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_d", func_8004D9D4);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_d", func_8004DABC);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_d", func_8004DB18);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_d", func_8004DC08);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_d", func_8004DC64);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_d", func_8004DCD0);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_d", func_8004DE08);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_d", func_8004DF64);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_d", func_8004E054);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_d", func_8004E0E4);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_d", func_8004E1C4);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_d", func_8004E230);

Class86B60Methods *func_8004E2D0(void)
{
    return &D_80086B60;
}

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_d", func_8004E2E0);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_d", func_8004E34C);
