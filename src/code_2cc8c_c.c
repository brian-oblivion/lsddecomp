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

/* A 3-word struct (see code_2c054.h's own StreamTaskInitData local view);
 * opaque here since this unit never dereferences it, only returns its
 * address. */
extern u8 D_8006E854[];

void *func_8003DFCC(void)
{
    return D_8006E854;
}

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_c", func_8003DFDC);

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_c", func_8003E030);

void func_8003E100(Obj86B60 *self)
{
    self->unk1C = 0;
    self->unk20 = 0;
}

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_c", func_8003E10C);

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_c", func_8003E280);

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_c", func_8003E418);

void func_8003E4A4(Obj86B60 *self)
{
    self->unk1C++;
}

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_c", func_8003E4B8);

void func_8003E538(Obj86B60 *self)
{
    Obj86B60UnkCTarget *target;

    self->unk1C = 0;
    target = self->unkC->target;
    target->methods->slot48(target);
}

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_c", func_8003E578);

IntermediateBaseMethods *func_8003E5C8(void)
{
    return &D_8006E878;
}

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_c", func_8003E5D8);

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_c", func_8003E628);

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_c", func_8003E6CC);

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_c", func_8003E770);

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_c", func_8003E7F4);

void func_8003E874(Obj86B60 *self)
{
    self->unk30 = 0;
    self->unk10 = 0;
    self->unkC = NULL;
    func_80018390()->slot18(self);
}
