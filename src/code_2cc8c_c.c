#include "common.h"
#include "code_2cc8c.h"

s32 func_8003DFA0(Obj86B60 *self)
{
    return self->unk60[self->unk58];
}

/* TaskCoreMethods table (see code_2c054.h's own richer local view); opaque
 * here since this unit never dereferences it, only returns its address. */
extern u8 D_8006E730[];

void *func_8003DFBC(void)
{
    return D_8006E730;
}

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_c", func_8003DFCC);

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_c", func_8003DFDC);

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_c", func_8003E030);

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_c", func_8003E100);

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_c", func_8003E10C);

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_c", func_8003E280);

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_c", func_8003E418);

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_c", func_8003E4A4);

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_c", func_8003E4B8);

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_c", func_8003E538);

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_c", func_8003E578);

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_c", func_8003E5C8);

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_c", func_8003E5D8);

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_c", func_8003E628);

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_c", func_8003E6CC);

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_c", func_8003E770);

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_c", func_8003E7F4);

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_c", func_8003E874);
