#include "common.h"
#include "Entity.h"

s32 func_8005DE18(Entity *this) {
    EntityMoodRow *row;
    s32 *xptr;
    s32 dist;

    row = &D_80089EA4[this->moodIndex];
    if (this->unkF0 != 0) {
        if (this->unkF4 == 0) {
            xptr = &this->unk14->x;
            dist = row->unk6;
            if (dist < 0) {
                dist = ~dist + 1;
            }
            if (func_8005D714(this, xptr, dist, row->unk9) != 0) {
                this->methods->slot164(this, 1);
            }
        }
        if (row->unk6 < 0) {
            func_8001EACC(this, this->unk94, 1, 0, 0);
        }
    }
    return this->unkF4;
}

INCLUDE_ASM("asm/nonmatchings/Entity_b", func_8005DEE0);

INCLUDE_ASM("asm/nonmatchings/Entity_b", func_8005DF9C);

INCLUDE_ASM("asm/nonmatchings/Entity_b", func_8005E02C);

INCLUDE_ASM("asm/nonmatchings/Entity_b", func_8005E0B0);

INCLUDE_ASM("asm/nonmatchings/Entity_b", Get_vtable_Entity);

INCLUDE_ASM("asm/nonmatchings/Entity_b", func_8005E160);

INCLUDE_ASM("asm/nonmatchings/Entity_b", func_8005E3C4);

INCLUDE_ASM("asm/nonmatchings/Entity_b", func_8005E480);

INCLUDE_ASM("asm/nonmatchings/Entity_b", func_8005E4D0);

INCLUDE_ASM("asm/nonmatchings/Entity_b", func_8005E694);

INCLUDE_ASM("asm/nonmatchings/Entity_b", func_8005E6F0);

INCLUDE_ASM("asm/nonmatchings/Entity_b", func_8005E7A8);

INCLUDE_ASM("asm/nonmatchings/Entity_b", func_8005E7F8);

INCLUDE_ASM("asm/nonmatchings/Entity_b", func_8005EA94);

INCLUDE_ASM("asm/nonmatchings/Entity_b", func_8005EBB4);

INCLUDE_ASM("asm/nonmatchings/Entity_b", func_8005EC98);

INCLUDE_ASM("asm/nonmatchings/Entity_b", func_8005ED10);

INCLUDE_ASM("asm/nonmatchings/Entity_b", func_8005ED30);

INCLUDE_ASM("asm/nonmatchings/Entity_b", func_8005EF20);
