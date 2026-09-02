#include "common.h"
#include "Entity.h"

/* Data rows this unit's mood-dispatch handlers pass through to a vtable
 * call as an opaque argument -- never dereferenced here, so an opaque byte
 * array is enough to form &D_8008xxxx correctly. Real element type/count
 * unknown. */
extern u8 D_80089DF0[];
extern u8 D_80089D78[];

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

s32 func_8005DEE0(Entity *this) {
    EntityMoodRow *row;
    s32 *xptr;
    s32 dist;

    if (this->unkF0 != 0 && this->unkF8 == 0 && this->unk44 != 1) {
        row = &D_80089EA4[this->moodIndex];
        if (row->unkB != 0) {
            xptr = &this->unk14->x;
            dist = row->unkB;
            if (dist < 0) {
                dist = ~dist + 1;
            }
            if (func_8005D714(this, xptr, dist, row->unk9) != 0) {
                this->methods->slot168(this);
            }
        }
    }
    return this->unkF8;
}

INCLUDE_ASM("asm/nonmatchings/Entity_b", func_8005DF9C);

INCLUDE_ASM("asm/nonmatchings/Entity_b", func_8005E02C);

INCLUDE_ASM("asm/nonmatchings/Entity_b", func_8005E0B0);

EntityMethods *Get_vtable_Entity(void) {
    return &D_80089AD4;
}

INCLUDE_ASM("asm/nonmatchings/Entity_b", func_8005E160);

INCLUDE_ASM("asm/nonmatchings/Entity_b", func_8005E3C4);

void func_8005E480(Entity *this, EntityMoodHandlerArg *out) {
    out->unk10 = this->methods->slot148(this);
    if (out->unk4 == 0) {
        out->unk1C = 0x17;
    }
}

INCLUDE_ASM("asm/nonmatchings/Entity_b", func_8005E4D0);

void func_8005E694(Entity *this) {
    this->methods->slot48(this, 1, D_80089DF0);
    this->methods->slotBC(this, D_80089D78);
}

INCLUDE_ASM("asm/nonmatchings/Entity_b", func_8005E6F0);

void func_8005E7A8(Entity *this, EntityMoodHandlerArg *out) {
    out->unk10 = 0;
    if (out->unk4 == 0) {
        out->unk1C = 0xB;
        out->unk30 = 0xB;
        out->unk44 = 0xB;
    }
    this->methods->slotC4(this, -0x1E, 0);
}

INCLUDE_ASM("asm/nonmatchings/Entity_b", func_8005E7F8);

INCLUDE_ASM("asm/nonmatchings/Entity_b", func_8005EA94);

INCLUDE_ASM("asm/nonmatchings/Entity_b", func_8005EBB4);

INCLUDE_ASM("asm/nonmatchings/Entity_b", func_8005EC98);

void func_8005ED10(Entity *this, EntityMoodHandlerArg *out) {
    if (this->unkFC == 0) {
        out->unk10 = 0;
        out->unk1C = 0xF;
    }
}

INCLUDE_ASM("asm/nonmatchings/Entity_b", func_8005ED30);

INCLUDE_ASM("asm/nonmatchings/Entity_b", func_8005EF20);
