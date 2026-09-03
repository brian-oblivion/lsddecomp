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

void func_8004D898(Class86B60 *self)
{
    u32 i;
    u8 *entry;

    i = 0;
    entry = (u8 *)&D_80086DAC;
    for (; i < 2; i++) {
        self->unkC->unk0->methods->slot78(self->unkC->unk0, &self->unk93, entry);
        entry += 0xC;
    }
}

void func_8004D90C(Class86B60 *self, s32 arg1)
{
    func_8003DFBC()->slot60(self, arg1);
    if (arg1 == 5) {
        self->methods->slot124(self, 0);
    }
    if (arg1 == 0xA) {
        self->methods->slot7C(self);
        self->methods->slotF0(self, self->unk4C->unk8, 1);
        self->methods->slot78(self);
    }
}

void func_8004D9D4(Class86B60 *self)
{
    void (*fn)(Class86B60 *);

    func_8003DFBC()->slot90(self);
    switch (self->unk58) {
    case 1:
        self->unk38 = 0;
        self->unkA4->methods->slotF0(self->unkA4, 0, 1);
        fn = self->methods->slot94;
        break;
    case 2:
        fn = self->methods->slot130;
        break;
    case 3:
        fn = self->methods->slot134;
        break;
    case 4:
        self->unk38 = 2;
        fn = self->methods->slot94;
        break;
    default:
        return;
    }
    fn(self);
}

void func_8004DABC(Class86B60 *self)
{
    s32 buf;

    func_8003DFBC()->slot94(self);
    buf = self->unk60->unk14;
    self->unkA4->methods->slot19C(self->unkA4, &buf);
}

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
