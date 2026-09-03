#include "common.h"
#include "class_3bb8c.h"

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_g", func_8004FBE4);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_g", func_8004FE24);

void func_8004FF40(Class86E00_3bb8c_g *self)
{
    if (self->unk70 != NULL) {
        self->unk70 = self->unk70->methods->slot4(self->unk70);
    }
}

void func_8004FF90(Class86E00_3bb8c_g *self, s32 arg1, s32 arg2)
{
    if (self->unk28 != 0) {
        if (arg2 == 0x19) {
            self->methods->slot90(self);
        } else if (arg2 == 0x17) {
            self->methods->slot94(self);
        }
    }
}

void func_8004FFF4(Class86E00_3bb8c_g *self, s32 arg1)
{
    if (self->unk6C != NULL) {
        self->unk6C->methods->slot80(self->unk6C, arg1, 0x7F, 0x7F);
    }
}

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_g", func_80050034);

void func_800501F0(Class86E00_3bb8c_g *self)
{
    switch (self->unk28) {
    case 4:
    case 6:
    case 0xA:
    case 0xE:
        self->methods->slot8C(self, 0x10);
        self->methods->slot7C(self, 0x17);
        break;
    default:
        break;
    }
}

void func_80050280(Class86E00_3bb8c_g *self)
{
    s32 old;
    s32 newVal;

    if (self->unk28 == 7) {
        old = self->unk5C;
        newVal = old + 1;
        self->unk5C = newVal;
        if (old < 6) {
            return;
        }
        self->methods->slot7C(self, 0x13);
    } else if (self->unk28 == 0xB) {
        old = self->unk5C;
        newVal = old + 1;
        self->unk5C = newVal;
        if (old < 6) {
            return;
        }
        self->methods->slot7C(self, 0x14);
    } else if (self->unk28 == 0xF) {
        old = self->unk5C;
        newVal = old + 1;
        self->unk5C = newVal;
        if (old < 6) {
            return;
        }
        self->methods->slot7C(self, 0x15);
    }
}

void func_80050340(Class86E00_3bb8c_g *self)
{
    if (self->unk68 != 0 && self->unk60 != 0) {
        if (self->unk78 == NULL) {
            self->unk78 = func_80050BA8((self->unk48 << 1) + self->unk44, 1);
            self->unk74 = 1;
        }
        self->methods->slot10(self, self->unk78);
        self->unk78->methods->slot44(self->unk78, self->unk68);
        self->unk78->methods->slot4C(self->unk78, self->unk60, self->unk64, self->unk6C);
    }
}

void func_80050410(Class86E00_3bb8c_g *self)
{
    if (self->unk68 != 0 && self->unk60 != 0 && self->unk78 != NULL) {
        self->unk78->methods->slot50(self->unk78);
        self->unk78->methods->slot48(self->unk78);
        if (self->unk74 != 0) {
            self->unk78->methods->release(self->unk78);
            self->unk78 = NULL;
        }
    }
}

void func_800504D0(Class86E00_3bb8c_g *self, void *arg1, s32 arg2)
{
    switch (arg2) {
    case 2:
        self->methods->slotA0(self);
        self->methods->slot78(self, self->unk40, self->unk44, self->unk48,
                               self->unk4C, self->unk50, self->unk54, self->unk58);
        break;
    case 3:
        self->methods->slotA0(self);
        self->methods->slot7C(self, 0x17);
        break;
    }
}

void func_800505A8(Class86E00_3bb8c_g *self)
{
    if (self->unk68 != 0 && self->unk60 != 0) {
        if (self->unk7C == NULL) {
            self->unk7C = func_80051A5C(self->unk38, 1);
            self->unk74 = 1;
        }
        self->methods->slot10(self, self->unk7C);
        self->unk7C->methods->slot44(self->unk7C, self->unk68);
        self->unk7C->methods->slot4C(self->unk7C, self->unk60, self->unk64, self->unk6C);
    }
}

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_g", func_80050670);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_g", func_80050730);

GenericCtorTable_3bb8c_d *func_800507E8(void)
{
    return &D_80086DC4;
}

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_g", func_800507F8);
