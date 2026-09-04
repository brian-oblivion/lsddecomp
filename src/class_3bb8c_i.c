#include "common.h"
#include "class_3bb8c.h"

/* This project's own strcpy (matched elsewhere) -- func_80050F28's own
 * caller, same local-declaration convention as class_3bb8c_e.c/others. */
extern char *strcpy(char *dest, char *src);

/* This class's method table. Declared HERE and not in include/class_3bb8c.h
 * because src/class_3bb8c_j.c declares the same object as its own
 * `Class86ED0Methods` local view, and two incompatible declarations of one
 * symbol in a shared header reach both translation units. See the HEAD NOTE
 * next to Obj86ED0Methods in that header. */
extern Obj86ED0Methods D_80086ED0;

/* This class's own table getter -- func_80050BA8/func_80050C14's shared
 * dispatch. DEFINED in src/class_3bb8c_j.c (matched round 15 by runner
 * bravo, which returns it as its own `Class86ED0Methods *` local view of
 * the same table). Returns `&D_80086ED0`; confirmed in the disassembly as
 * `lui/addiu` materialising that exact address then `jr $ra`, the same
 * no-argument-getter shape as `func_80018390`. Declared here rather than
 * in the shared header because the two units' return types differ. */
extern Obj86ED0Methods *func_80051A4C(void);

void *func_80050BA8(s32 arg0, s32 arg1)
{
    Obj86ED0 *self;

    self = func_80017B34(0x4C);
    if (self != NULL) {
        func_80051A4C()->ctor(self, arg0, arg1);
        return self;
    }
    return NULL;
}

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_i", func_80050C14);

void func_80050CD8(Obj86ED0 *self)
{
    self->unk34 = NULL;
    self->unk38 = NULL;
    self->unk48 = NULL;
}

void func_80050CE8(Obj86ED0 *self)
{
    func_80017CFC(self->unk28);
    func_80018390()->finalize(self);
}

void func_80050D30(Obj86ED0 *self, void *arg1)
{
    s32 tag;
    s32 mask;

    if (arg1 != NULL) {
        func_80018390()->addChild(self, arg1);
        tag = **(s32 **)arg1;
        mask = tag & 0xF;
        if (mask == 2) {
            self->unk34 = arg1;
        } else if (mask == 5) {
            self->unk38 = arg1;
        }
    }
}

void func_80050DB4(Obj86ED0 *self, void *arg1)
{
    s32 tag;
    s32 mask;

    if (arg1 != NULL) {
        tag = **(s32 **)arg1;
        mask = tag & 0xF;
        if (mask == 2) {
            self->unk34 = NULL;
        } else if (mask == 5) {
            self->unk38 = NULL;
        }
        func_80018390()->removeChild(self, arg1);
    }
}

void func_80050E34(Obj86ED0 *self)
{
    self->unk34 = NULL;
    self->unk38 = NULL;
    self->unk48 = NULL;
    func_80018390()->removeAllChildren(self);
}

void func_80050E78(Obj86ED0 *self, void *arg1, s32 arg2)
{
    s32 tag;
    s32 mask;

    func_80018390()->slot38(self, arg1, arg2);

    tag = **(s32 **)arg1;
    mask = tag & 0xF;
    if (mask == 2) {
        self->methods->slot5C(self, arg1, arg2);
    } else if (mask == 5) {
        self->methods->slot58(self, arg1, arg2);
    }
}

void func_80050F28(Obj86ED0 *self, char *arg1, s32 mode)
{
    self->unkC = mode;
    self->unk24 = arg1;
    self->unk18 = 0;
    self->unk1C = 0;
    if (mode == 1) {
        func_80040FC0(self->unk28, arg1);
        self->unk10 /= 2;
    } else {
        strcpy(self->unk28, arg1);
    }
}

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_i", func_80050F98);

void func_80051174(Obj86ED0 *self)
{
    if (self->unk48 != NULL) {
        self->unk48 = self->unk48->methods->release(self->unk48);
        self->unk44->methods->release(self->unk44);
        self->unk40->methods->release(self->unk40);
    }
}

void func_80051200(Obj86ED0 *self, void *arg1, void *arg2, TargetObj86ED0 *arg3)
{
    self->methods->addChild(self, arg1);
    self->methods->addChild(self, arg2);
    self->unk3C = arg3;
    self->unk2C = 0;
    self->unk20 = 0;
}

void func_80051270(Obj86ED0 *self)
{
    self->methods->removeChild(self, self->unk34);
    self->methods->removeChild(self, self->unk38);
    self->unk3C = NULL;
}

void func_800512C8(Obj86ED0 *self, s32 arg1)
{
    self->unk30 = 0;
    if (arg1 < 2) {
        return;
    }
    switch (arg1) {
    case 2:
    case 3:
        self->methods->removeChild(self, self->unk34);
        self->methods->slot48(self);
        self->unk2C = arg1;
        break;
    case 4:
        self->methods->onFinalize(self, self->unk2C);
        break;
    }
}

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_i", func_80051370);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_i", func_800513D0);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_i", func_8005161C);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_i", func_8005165C);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_i", func_800516C0);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_i", func_80051720);
