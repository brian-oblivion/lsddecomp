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

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_b", func_8003CDE0);

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_b", func_8003CE98);

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_b", func_8003D050);

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_b", func_8003D194);

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_b", func_8003D2CC);

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_b", func_8003D3B0);

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_b", func_8003D444);

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_b", func_8003D4DC);

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_b", func_8003D5C0);

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_b", func_8003D5CC);

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_b", func_8003D6D4);

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_b", func_8003D73C);

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_b", func_8003D980);

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_b", func_8003DA10);

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_b", func_8003DAD4);

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_b", func_8003DCAC);

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_b", func_8003DDC8);

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_b", func_8003DE30);

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_b", func_8003DE9C);
