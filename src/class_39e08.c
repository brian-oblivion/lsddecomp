#include "common.h"

/* New_ClassA: allocate a 0x50-byte instance and construct it. */
ClassA *func_80049608(void *arg1, void *arg2, s32 arg3) {
    ClassA *self;

    self = func_80017B34(0x50);
    if (self == NULL) {
        goto fail;
    }
    func_8004A060()->ctor(self, arg1, arg2, arg3);
    return self;
fail:
    return NULL;
}

INCLUDE_ASM("asm/nonmatchings/class_39e08", func_80049684);

INCLUDE_ASM("asm/nonmatchings/class_39e08", func_80049830);

INCLUDE_ASM("asm/nonmatchings/class_39e08", func_80049958);

/* ClassA::resetState -- +0x040 slot. */
void func_80049A14(ClassA *self) {
    self->state = 0;
}

INCLUDE_ASM("asm/nonmatchings/class_39e08", func_80049A1C);

INCLUDE_ASM("asm/nonmatchings/class_39e08", func_80049AC0);

INCLUDE_ASM("asm/nonmatchings/class_39e08", func_80049B54);

/* ClassA::slot50 -- dispatches the two calls on the unk18 sub-object. */
void func_80049C50(ClassA *self) {
    Dispatch18 *obj;

    obj = self->unk18;
    obj->methods->slot90(obj);
    obj->methods->slot74(obj);
}

INCLUDE_ASM("asm/nonmatchings/class_39e08", func_80049CA8);

INCLUDE_ASM("asm/nonmatchings/class_39e08", func_80049E20);

void func_80049EA4(void) {
}

void func_80049EAC(void) {
}

INCLUDE_ASM("asm/nonmatchings/class_39e08", func_80049EB4);

/* Get_vtable_ClassA -- a direct address load, not a call. */
ClassAMethods *func_8004A060(void) {
    return &D_800865C8;
}

INCLUDE_ASM("asm/nonmatchings/class_39e08", func_8004A070);

/* New_ClassC: allocate a 0x38-byte instance of a third, unrelated class and
 * construct it through BasicClass's own ctor slot. */
void *func_8004A130(void *arg1, void *arg2) {
    void *self;

    self = func_80017B34(0x38);
    if (self == NULL) {
        goto fail;
    }
    func_8004A4B8()->ctor(self, arg1, arg2);
    return self;
fail:
    return NULL;
}

INCLUDE_ASM("asm/nonmatchings/class_39e08", func_8004A19C);

INCLUDE_ASM("asm/nonmatchings/class_39e08", func_8004A228);

INCLUDE_ASM("asm/nonmatchings/class_39e08", func_8004A294);

INCLUDE_ASM("asm/nonmatchings/class_39e08", func_8004A2C4);

INCLUDE_ASM("asm/nonmatchings/class_39e08", func_8004A324);

void func_8004A35C(void) {
}

INCLUDE_ASM("asm/nonmatchings/class_39e08", func_8004A364);

INCLUDE_ASM("asm/nonmatchings/class_39e08", func_8004A3EC);

INCLUDE_ASM("asm/nonmatchings/class_39e08", func_8004A458);
