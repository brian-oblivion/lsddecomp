#include "common.h"
#include "Entity.h"

void func_800634A8(Entity *this, EntityMoodHandlerArg *out) {
    if (this->unk44 == 0 && this->unkFC == 0) {
        if (rand() % 3 != 0) {
            this->unk44 = (rand() & 1) ? 0xB : 0xC;
        } else {
            this->methods->slot16C(this);
            this->methods->slotC4(this, -0x5000, 0);
            rand();
        }
    }
    if (this->unk44 == 0xC) {
        func_8001EACC(this, this->unk94, 1, 0, 0);
        if (this->unkFC == 0x14) {
            out->unk1C = 0x12;
            out->unk10 = 0;
            out->unk30 = 3;
            this->unk94->methods->slot130(this->unk94, 1);
        }
        if (this->unkFC >= 0x15) {
            this->methods->slotC4(this, -0x28, 0);
        }
        if (this->unkFC == 0x28) {
            this->methods->slot30(this, 0xA);
        }
    } else if (this->unk44 == 0xB) {
        this->methods->slot130(this);
        if (this->methods->slot144(this, this->unk94) < 0x200) {
            if (rand() % 3 != 0) {
                this->methods->slot160(this);
            } else {
                this->methods->slot16C(this);
            }
        }
    }
}

INCLUDE_ASM("asm/nonmatchings/Entity_f", func_800636E4);

INCLUDE_ASM("asm/nonmatchings/Entity_f", func_80063784);

INCLUDE_ASM("asm/nonmatchings/Entity_f", func_80063874);

INCLUDE_ASM("asm/nonmatchings/Entity_f", func_80063BC0);

INCLUDE_ASM("asm/nonmatchings/Entity_f", func_80063C84);

INCLUDE_ASM("asm/nonmatchings/Entity_f", func_80063CAC);

INCLUDE_ASM("asm/nonmatchings/Entity_f", func_80063CC8);

INCLUDE_ASM("asm/nonmatchings/Entity_f", func_80063D40);

INCLUDE_ASM("asm/nonmatchings/Entity_f", func_80063DC8);

INCLUDE_ASM("asm/nonmatchings/Entity_f", func_80063E68);

INCLUDE_ASM("asm/nonmatchings/Entity_f", func_80063ED4);

INCLUDE_ASM("asm/nonmatchings/Entity_f", func_80064078);

INCLUDE_ASM("asm/nonmatchings/Entity_f", func_800641C0);

INCLUDE_ASM("asm/nonmatchings/Entity_f", func_80064294);

INCLUDE_ASM("asm/nonmatchings/Entity_f", func_80064450);

INCLUDE_ASM("asm/nonmatchings/Entity_f", func_800644E8);
