#include "common.h"
#include "code_2cc8c.h"

s32 func_8003CD48(Obj86B60 *self)
{
    s32 c = 0x80 - (self->unk1C * self->unk84);
    u8 buf[3];

    buf[0] = c;
    buf[1] = c;
    buf[2] = c;
    self->methods->slotE4(self, buf);
    self->unk78->methods->slotB8(self->unk78, 1, buf);
    return (u8)c >= 0x81;
}

void func_8003CDE0(Obj86B60 *self, const char *a1, Unk74Obj *a2)
{
    if (a1 != NULL) {
        if (self->unk70 != NULL) {
            self->unk74->methods->slot4(self->unk74);
        }
        self->unk74 = func_8003B39C(a1);
        self->unk74->methods->slot78(self->unk74);
        self->unk74->methods->slot5C(self->unk74);
    } else {
        self->unk74 = a2;
    }
    self->unk70 = a1;
}

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_b", func_8003CE98);

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_b", func_8003D050);

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_b", func_8003D194);

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_b", func_8003D2CC);

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_b", func_8003D3B0);

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_b", func_8003D444);

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_b", func_8003D4DC);

s32 func_8003D5C0(Obj86B60 *self)
{
    return self->unk58;
}

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_b", func_8003D5CC);

void func_8003D6D4(Obj86B60 *self)
{
    func_800183DC(self->unk64[self->unk58], self->unk5C[self->unk58]);
    func_80017CFC(self->unk64[self->unk58]);
}

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_b", func_8003D73C);

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_b", func_8003D980);

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_b", func_8003DA10);

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_b", func_8003DAD4);

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_b", func_8003DCAC);

void func_8003DDC8(Obj86B60 *self)
{
    s32 idx = self->unk58;
    s32 v = self->unk60[idx];

    v++;
    if (v >= self->unk5C[idx]) {
        v = 0;
    }
    self->methods->slot11C(self, v, 1);
}

void func_8003DE30(Obj86B60 *self)
{
    s32 idx = self->unk58;
    s32 v = self->unk60[idx];

    v--;
    if (v < 0) {
        v = self->unk5C[idx] - 1;
    }
    self->methods->slot11C(self, v, 1);
}

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_b", func_8003DE9C);
