#include "common.h"
#include "Entity.h"

INCLUDE_ASM("asm/nonmatchings/Entity_e", func_80061A90);

void func_80061C04(Entity *this, EntityMoodHandlerArg *out) {
    if (this->unk84 == 0x1E) {
        out->unk1C = 0x12;
        out->unk10 = 0;
        out->unk20 = -1;
    }
}

INCLUDE_ASM("asm/nonmatchings/Entity_e", func_80061C2C);

void func_80061E60(Entity *this, EntityMoodHandlerArg *out) {
    s32 r = out->unk4 % 300;

    out->unk10 = this->methods->slot148(this);
    if (r < 0x14) {
        out->unk1C = 5;
        out->unk20 = -2;
    } else if (r == 0x16) {
        out->unk1C = -2;
    }
    this->methods->slotC4(this, -0xA, 0);
}

INCLUDE_ASM("asm/nonmatchings/Entity_e", func_80061F30);

INCLUDE_ASM("asm/nonmatchings/Entity_e", func_8006204C);

INCLUDE_ASM("asm/nonmatchings/Entity_e", func_800620C4);

INCLUDE_ASM("asm/nonmatchings/Entity_e", func_800621A8);

INCLUDE_ASM("asm/nonmatchings/Entity_e", func_800623E8);

INCLUDE_ASM("asm/nonmatchings/Entity_e", func_800624BC);

INCLUDE_ASM("asm/nonmatchings/Entity_e", func_80062570);

INCLUDE_ASM("asm/nonmatchings/Entity_e", func_80062660);

INCLUDE_ASM("asm/nonmatchings/Entity_e", func_80062730);

INCLUDE_ASM("asm/nonmatchings/Entity_e", func_800628D4);

INCLUDE_ASM("asm/nonmatchings/Entity_e", func_80062970);

INCLUDE_ASM("asm/nonmatchings/Entity_e", func_80062A40);

INCLUDE_ASM("asm/nonmatchings/Entity_e", func_80062C58);

INCLUDE_ASM("asm/nonmatchings/Entity_e", func_80062FAC);

INCLUDE_ASM("asm/nonmatchings/Entity_e", func_80063094);

INCLUDE_ASM("asm/nonmatchings/Entity_e", func_80063144);
