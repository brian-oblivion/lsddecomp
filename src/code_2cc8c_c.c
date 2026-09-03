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

void func_8003DFDC(Obj86B60 *self)
{
    func_80018390()->ctor(self);
    self->methods = (Obj86B60Methods *)func_8003E5C8();
    self->methods->slot40(self);
}

void func_8003E030(Obj86B60 *self, EventArg *arg1, s32 arg2)
{
    s32 header;

    func_80018390()->slot38(self, arg1, arg2);
    header = arg1->target->header & 0xF;
    if (header == 1) {
        self->methods->slot54(self, arg1, arg2);
    } else if (header == 2) {
        self->methods->slot58(self, arg1, arg2);
    } else if (header == 5) {
        self->methods->slot5C(self, arg1, arg2);
    }
}

void func_8003E100(Obj86B60 *self)
{
    self->unk1C = 0;
    self->unk20 = 0;
}

void func_8003E10C(Obj86B60 *self, Obj86B60InitArgs *arg1, s32 arg2)
{
    Obj86B60Methods *methods;
    Unk18Obj *obj18;

    methods = self->methods;
    if (arg1->unk8 != NULL) {
        self->unk10 = (s32)arg1->unk8;
    } else {
        self->unk10 = (s32)func_80042400();
    }
    if (arg1->unkC != NULL) {
        self->unk14 = (s32)arg1->unkC;
    } else {
        self->unk14 = (s32)func_80042694();
    }
    if (arg1->unk10 != NULL) {
        self->unk18 = arg1->unk10;
    } else {
        self->unk18 = func_8003E5D8();
    }
    self->unkC = (Obj86B60UnkC *)arg1;
    obj18 = self->unk18;
    methods->slot10(self, arg1->unk0);
    methods->slot10(self, arg1->unk4);
    methods->slot10(self, (void *)self->unk10);
    methods->slot4C(self, 0, 0, 0);
    self->unk24 = arg2;
    if (arg2 == 0) {
        obj18->methods->slot10(obj18, arg1->unk0);
        obj18->methods->slot10(obj18, (void *)self->unk10);
        ((Unk14Obj *)self->unk14)->methods->slot10((Unk14Obj *)self->unk14, (void *)self->unk10);
        methods->slot60(self, 2);
        methods->slot48(self);
    }
}

void func_8003E280(Obj86B60 *self)
{
    Obj86B60Methods *methods;
    Unk18Obj *obj18;

    methods = self->methods;
    methods->slot50(self);
    obj18 = self->unk18;
    if (self->unk24 == 0) {
        ((Unk14Obj *)self->unk14)->methods->slot14((Unk14Obj *)self->unk14, (void *)self->unk10);
        obj18->methods->slot14(obj18, (void *)self->unk10);
        obj18->methods->slot14(obj18, ((Obj86B60InitArgs *)self->unkC)->unk0);
    }
    methods->slot14(self, (void *)self->unk10);
    methods->slot14(self, ((Obj86B60InitArgs *)self->unkC)->unk4);
    methods->slot14(self, ((Obj86B60InitArgs *)self->unkC)->unk0);
    if (((Obj86B60InitArgs *)self->unkC)->unk10 != obj18) {
        self->unk18 = obj18->methods->slot4(obj18);
    }
    if ((void *)((Obj86B60InitArgs *)self->unkC)->unkC != (void *)self->unk14) {
        self->unk14 = (s32)((Unk14Obj *)self->unk14)->methods->slot4((Unk14Obj *)self->unk14);
    }
    if (((Obj86B60InitArgs *)self->unkC)->unk8 != (void *)self->unk10) {
        self->unk10 = (s32)((Unk10Obj *)self->unk10)->methods->slot4((Unk10Obj *)self->unk10);
    }
}

void func_8003E418(Obj86B60 *self, EventArg *arg1, s32 arg2)
{
    Unk4ArgObj *obj4;

    if (arg2 == 2) {
        ((Unk10Obj *)self->unk10)->methods->slot44((Unk10Obj *)self->unk10);
        obj4 = ((Obj86B60InitArgs *)self->unkC)->unk4;
        obj4->methods->slot44(obj4);
        obj4->methods->slot48(obj4);
    }
}

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
