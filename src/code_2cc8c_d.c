#include "common.h"
#include "code_2cc8c.h"

/* Forwards to the inherited BasicClass slot38, then dispatches self's OWN
 * slot94 or slot98 depending on arg1's dynamic class tag (5 or 1
 * respectively, per its header nibble -- same tag idiom as func_8003E770,
 * round 13). Neither dispatch happens for any other tag. */
void func_8003E8B8(Unk18Obj *self, GenericObj *arg1, s32 arg2) {
    s32 tag;

    func_80018390()->slot38(self, arg1, arg2);

    tag = arg1->methods->header & 0xF;
    if (tag == 5) {
        self->methods->slot94(self, arg1, arg2);
    } else if (tag == 1) {
        self->methods->slot98(self, arg1, arg2);
    }
}

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_d", func_8003E968);

void func_8003EA0C(Unk18Obj *self, Pair32_d294 *pair) {
    self->unk34 = *pair;
}

void func_8003EA24(Unk18Obj *self, s32 a1) {
    self->unk3C = a1;
}

/* Only writes unk44 the first time (guarded by the unk70 latch). */
void func_8003EA2C(Unk18Obj *self, s32 a1) {
    if (self->unk70 == 0) {
        self->unk44 = a1;
    }
}

/* Same guard as func_8003EA2C, writes unk48 instead. */
void func_8003EA48(Unk18Obj *self, s32 a1) {
    if (self->unk70 == 0) {
        self->unk48 = a1;
    }
}

void func_8003EA64(Unk18Obj *self, s32 a1) {
    self->unk40 = a1;
}

void func_8003EA6C(void) {
}

void func_8003EA74(void) {
}

void func_8003EA7C(Unk18Obj *self, s32 a1) {
    self->unk54 = a1;
}

void func_8003EA84(Unk18Obj *self, SByte3_d294 *src) {
    self->unk58 = *src;
}

void func_8003EAA4(Unk18Obj *self, SByte3_d294 *src) {
    self->unk5B = *src;
}

void func_8003EAC4(Unk18Obj *self, s32 a1) {
    self->unk60 = a1;
}

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_d", func_8003EACC);

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_d", func_8003EB84);

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_d", func_8003EBC4);

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_d", func_8003EBF8);

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_d", func_8003EC2C);

void func_8003ECC0(void) {
}

void func_8003ECC8(void) {
}

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_d", func_8003ECD0);

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_d", func_8003EDF4);

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_d", func_8003EE40);

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_d", func_8003EE88);

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_d", func_8003EEC0);

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_d", func_8003F04C);

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_d", func_8003F1A8);

SubHandleObj *func_8003F230(Unk18Obj *self) {
    return self->unkB0;
}

void func_8003F23C(Unk18Obj *self, s32 a1) {
    self->unkB4 = a1;
}

void func_8003F244(Unk18Obj *self, s32 a1) {
    self->unkB8 = a1;
}

/* Plain no-arg getter for Unk18Obj's own vtable. */
Unk18ObjMethods *func_8003F24C(void) {
    return &D_8006E8E4;
}

/* Walks self->unkC repeatedly while non-NULL, finding the tail of a
 * singly-linked list rooted at self, threaded through unkC. Returns self
 * itself unchanged if self->unkC is already NULL. */
Unk18Obj *func_8003F25C(Unk18Obj *self) {
    while (self->unkC != NULL) {
        self = (Unk18Obj *)self->unkC;
    }
    return self;
}

void func_8003F28C(Unk18Obj *self) {
    func_80024B90(self);
}
