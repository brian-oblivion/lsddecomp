/* 34 queued, all fresh. Offered to runners in round 2026-08-30-a; no longer
 * banked. No function in this unit touches a %gp_rel global, so none of it is
 * exposed to the gp-relative blocker (docs/research/gp-relative-blocker.md).
 */
#include "common.h"
#include "code_55dd4.h"

void *New_class_65650(void *arg1, void *arg2)
{
    Class65650 *self;
    Class65650Methods *vt;

    self = (Class65650 *)func_80017B34(0x98);
    if (self == NULL) {
        return NULL;
    }
    vt = func_80066818();
    if (vt->ctor(self, arg1, arg2) != NULL) {
        return self;
    }
    func_80017CFC(self);
    return NULL;
}

Class65650 *class_65650__Constructor(Class65650 *self, void *arg1, void *arg2)
{
    D800878D4Methods *base;

    base = func_80057C84();
    if (base->ctor(self) == NULL) {
        return NULL;
    }
    self->methods = func_80066818();
    self->arg2 = arg2;
    self->unk5C = NULL;
    self->unk68 = NULL;
    self->unk70 = NULL;
    self->unk94 = 0;
    if (self->methods->slot_setup5C(self, arg1) != 0) {
        base = func_80057C84();
        base->dtor(self);
        return NULL;
    }
    self->methods->slot10(self, self->unk5C);
    self->methods->slot40(self);
    return self;
}

void func_8006573C(Class65650 *self)
{
    self->methods->slot_teardown5C(self);
    func_80057C84()->dtor(self);
}

INCLUDE_ASM("asm/nonmatchings/code_55dd4", func_80065790);

INCLUDE_ASM("asm/nonmatchings/code_55dd4", func_80065830);

INCLUDE_ASM("asm/nonmatchings/code_55dd4", func_80065918);

INCLUDE_ASM("asm/nonmatchings/code_55dd4", func_800659D0);

INCLUDE_ASM("asm/nonmatchings/code_55dd4", func_80065A5C);

INCLUDE_ASM("asm/nonmatchings/code_55dd4", func_80065AE0);

INCLUDE_ASM("asm/nonmatchings/code_55dd4", func_80065B80);

INCLUDE_ASM("asm/nonmatchings/code_55dd4", func_80065BF4);

INCLUDE_ASM("asm/nonmatchings/code_55dd4", func_80065BFC);

INCLUDE_ASM("asm/nonmatchings/code_55dd4", func_80065C2C);

INCLUDE_ASM("asm/nonmatchings/code_55dd4", func_80065C5C);

INCLUDE_ASM("asm/nonmatchings/code_55dd4", func_80065CEC);

INCLUDE_ASM("asm/nonmatchings/code_55dd4", func_80065D64);

INCLUDE_ASM("asm/nonmatchings/code_55dd4", func_80065DBC);

INCLUDE_ASM("asm/nonmatchings/code_55dd4", func_80065DEC);

INCLUDE_ASM("asm/nonmatchings/code_55dd4", func_80065E1C);

INCLUDE_ASM("asm/nonmatchings/code_55dd4", func_80065F2C);

INCLUDE_ASM("asm/nonmatchings/code_55dd4", func_80065FD8);

INCLUDE_ASM("asm/nonmatchings/code_55dd4", func_800660BC);

INCLUDE_ASM("asm/nonmatchings/code_55dd4", func_8006613C);

INCLUDE_ASM("asm/nonmatchings/code_55dd4", func_80066148);

INCLUDE_ASM("asm/nonmatchings/code_55dd4", func_80066150);

void func_800661C4(void) {
}

void func_800661CC(void) {
}

INCLUDE_ASM("asm/nonmatchings/code_55dd4", func_800661D4);

INCLUDE_ASM("asm/nonmatchings/code_55dd4", func_80066214);

INCLUDE_ASM("asm/nonmatchings/code_55dd4", func_800662A8);

INCLUDE_ASM("asm/nonmatchings/code_55dd4", func_800662B4);

INCLUDE_ASM("asm/nonmatchings/code_55dd4", func_800662BC);

INCLUDE_ASM("asm/nonmatchings/code_55dd4", func_80066340);

INCLUDE_ASM("asm/nonmatchings/code_55dd4", func_80066748);

INCLUDE_ASM("asm/nonmatchings/code_55dd4", func_800667B0);

INCLUDE_ASM("asm/nonmatchings/code_55dd4", func_80066818);
