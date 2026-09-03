#include "common.h"
#include "code_d294.h"

void func_8001E58C(Class6B5CCObj *self, Class6B5CCSub44 *dst, s16 *src) {
    u8 buf[0x20];

    self->methods->slot84(self, buf, 0);
    dst->unk0 = src[0];
    dst->unk4 = src[1];
    dst->unk8 = src[2];
    func_8001EE98(dst, dst, 1, buf);
}

INCLUDE_ASM("asm/nonmatchings/code_d294_c", func_8001E600);

INCLUDE_ASM("asm/nonmatchings/code_d294_c", func_8001E6F8);

void func_8001E770(Class6B5CCObj *self, GenericObj_d294 *other) {
    self->unk20 = other;
    self->unk18 = other->unk10;
    GsLinkObject4((u8 *)((GenericObj_d294 *)self->unk20)->unkC + 0xC, &self->unk10, 0);
}

void func_8001E7B0(Class6B5CCObj *self) {
    self->unk18 = 0;
    self->unk20 = 0;
}

INCLUDE_ASM("asm/nonmatchings/code_d294_c", func_8001E7BC);

void func_8001EA8C(s32 *dest, s16 *b, s16 *a) {
    dest[0] = a[0] - b[0];
    dest[1] = a[1] - b[1];
    dest[2] = a[2] - b[2];
}

INCLUDE_ASM("asm/nonmatchings/code_d294_c", func_8001EACC);

INCLUDE_ASM("asm/nonmatchings/code_d294_c", func_8001EC84);

s32 func_8001ECFC(s16 *box, s16 *point) {
    s32 flags;

    flags = 0;
    if (box[3] < point[0]) {
        flags = 8;
    } else if (point[0] < box[0]) {
        flags = 4;
    }
    if (box[4] < point[1]) {
        flags |= 2;
    } else if (point[1] < box[1]) {
        flags |= 1;
    }
    if (box[5] < point[2]) {
        flags |= 0x20;
    } else if (point[2] < box[2]) {
        flags |= 0x10;
    }
    return flags;
}

INCLUDE_ASM("asm/nonmatchings/code_d294_c", func_8001EDAC);

INCLUDE_ASM("asm/nonmatchings/code_d294_c", func_8001EE04);

INCLUDE_ASM("asm/nonmatchings/code_d294_c", func_8001EE98);

s32 func_8001EF14(s32 *a, s32 range, s32 *b) {
    s32 i;

    for (i = 0; i < 3; i++, a++, b++) {
        if (*b < *a - range) {
            return 0;
        }
        if (*a + range < *b) {
            return 0;
        }
    }
    return 1;
}

INCLUDE_ASM("asm/nonmatchings/code_d294_c", func_8001EF60);
