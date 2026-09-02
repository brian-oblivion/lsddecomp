#include "common.h"
#include "code_2c054.h"

INCLUDE_ASM("asm/nonmatchings/code_2c054", func_8003B854);

INCLUDE_ASM("asm/nonmatchings/code_2c054", func_8003B8E4);

void func_8003B9DC(StreamTaskObj *self) {
    self->unkB4->methods->slot04(self->unkB4);
    func_8003DFBC()->slot0C(self);
}

void func_8003BA38(StreamTaskObj *self) {
    self->unkC8 = -1;
    self->unkC4 = 0;
    self->unkCC = 1;
    self->unkD0 = 0;
    self->unkD4 = 1;
}

void func_8003BA58(StreamTaskObj *self, s32 a1, s32 arg2, s32 typeLookup, s32 flag) {
    self->unkB8 = arg2;
    self->unkBC = typeLookup;
    self->unkC0 = flag;
    func_8003DFBC()->slot44(self, a1, 0);
}

void func_8003BAB4(StreamTaskObj *self) {
    func_8003DFBC()->slot4C(self);
    self->unkA4 = 0;
    self->unkB4->methods->slot6C(self->unkB4, self->unkC0);
    if (self->unkB4->methods->slot40(self->unkB4, self->unkB8, self->unkBC, self->unkC4, self->unkC8) != 0) {
        self->methods->slot6C(self, 0);
    }
}

void func_8003BB5C(StreamTaskObj *self, s32 a1, s32 a2) {
    func_8003DFBC()->slot5C(self, a1, a2);
    if (self->unkA4 != 0) {
        return;
    }
    self->unkA4 = self->unkB4->methods->slot48(self->unkB4);
    if (self->unkA4 == 0) {
        return;
    }
    if (self->unkD8 != 0) {
        return;
    }
    self->methods->slot60(self, 7);
}

INCLUDE_ASM("asm/nonmatchings/code_2c054", func_8003BC14);

void func_8003BCF4(StreamTaskObj *self, s32 a1) {
    self->unk40 = a1;
    if (a1 >= 0) {
        self->unk40 = a1 * 15;
    }
}

void func_8003BD10(StreamTaskObj *self) {
    func_8003DFBC()->slot78(self);
    if (self->unkCC != 0) {
        self->unk38 = 2;
        self->methods->slot60(self, 0x12);
    }
}

void func_8003BD74(StreamTaskObj *self) {
    func_8003DFBC()->slot80(self);
}

void func_8003BDAC(StreamTaskObj *self) {
    func_8003DFBC()->slot84(self);
}

void func_8003BDE4(void) {
}

void func_8003BDEC(void) {
}

void func_8003BDF4(StreamTaskObj *self) {
    if (self->unkD4 != 0) {
        self->unkB4->methods->slot4C(self->unkB4);
    } else {
        self->methods->slot60(self, 7);
    }
}

void func_8003BE5C(StreamTaskObj *self, s32 a1) {
    self->unkC4 = a1;
}

void func_8003BE64(StreamTaskObj *self, s32 a1) {
    self->unkC8 = a1;
}

void func_8003BE6C(StreamTaskObj *self, s32 a1) {
    self->unkCC = a1;
}

void func_8003BE74(StreamTaskObj *self, s32 a1) {
    self->unkD0 = a1;
}

void func_8003BE7C(StreamTaskObj *self, s32 a1) {
    self->unkD4 = a1;
}

StreamTaskObjMethods *func_8003BE84(void) {
    return &D_8006E5F8;
}

INCLUDE_ASM("asm/nonmatchings/code_2c054", func_8003BE94);

INCLUDE_ASM("asm/nonmatchings/code_2c054", func_8003BF10);

INCLUDE_ASM("asm/nonmatchings/code_2c054", func_8003C008);

void func_8003C11C(StreamTaskObj *self) {
    StreamTaskObjMethods *methods = self->methods;
    methods->slot6C(self, -1);
    methods->slotA4(self, &D_8006E860[0], &D_8006E860[3], &D_8006E860[6]);
    methods->slot9C(self, 1);
    methods->slotA0(self, 1);
    self->unk84 = 9;
    self->unk28 = 3;
    self->unk2C = 0x12C;
    self->unk30 = 0x40;
    self->unk9C = 0;
    self->unkA0 = 0;
    self->unk34 = 1;
    self->unk3C = 0;
}

s32 func_8003C1DC(StreamTaskObj *self, s32 a1, s32 a2) {
    func_8003E5C8()->slot44(self, a1, a2);
    return self->unk38;
}

INCLUDE_ASM("asm/nonmatchings/code_2c054", func_8003C238);

void func_8003C3D0(StreamTaskObj *self) {
    TaskCoreObj *obj = self->unk18;
    obj->methods->slot90(obj);
    obj->methods->slot74(obj);
    self->unk78->methods->slot50(self->unk78);
    if (self->unk34 != 0) {
        self->unkC->unk0->methods->slot78(self->unkC->unk0, &self->unk93, 0);
    }
}
