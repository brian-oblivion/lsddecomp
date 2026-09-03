#include "common.h"
#include "Entity.h"

void func_80064618(Entity *this, EntityMoodHandlerArg *out) {
    if (this->unkF4 != 0) {
        if (func_8005D108(this, NULL, 0, 0xA, 0) != 0) {
            this->unk100->methods->slotD4(this->unk100, this->unk50, 7, 0);
            this->methods->slot160(this);
            this->unk94->methods->slot21C(this->unk94);
        }
    }
    this->methods->slotC4(this, -0x1E, 1);
}

INCLUDE_ASM("asm/nonmatchings/Entity_g", func_800646D8);

INCLUDE_ASM("asm/nonmatchings/Entity_g", func_80064928);

INCLUDE_ASM("asm/nonmatchings/Entity_g", func_80064AA4);

INCLUDE_ASM("asm/nonmatchings/Entity_g", func_80064B14);

INCLUDE_ASM("asm/nonmatchings/Entity_g", func_80064B80);

INCLUDE_ASM("asm/nonmatchings/Entity_g", func_80064CA4);

INCLUDE_ASM("asm/nonmatchings/Entity_g", func_80064CEC);

INCLUDE_ASM("asm/nonmatchings/Entity_g", func_80064D48);

INCLUDE_ASM("asm/nonmatchings/Entity_g", func_80064E34);

INCLUDE_ASM("asm/nonmatchings/Entity_g", func_80064FBC);

INCLUDE_ASM("asm/nonmatchings/Entity_g", func_800650D4);

INCLUDE_ASM("asm/nonmatchings/Entity_g", func_800650F4);

INCLUDE_ASM("asm/nonmatchings/Entity_g", func_8006519C);

INCLUDE_ASM("asm/nonmatchings/Entity_g", func_800651D0);

INCLUDE_ASM("asm/nonmatchings/Entity_g", func_80065204);

INCLUDE_ASM("asm/nonmatchings/Entity_g", func_80065238);

INCLUDE_ASM("asm/nonmatchings/Entity_g", func_8006536C);

INCLUDE_ASM("asm/nonmatchings/Entity_g", func_800654A0);

INCLUDE_ASM("asm/nonmatchings/Entity_g", func_80065514);
