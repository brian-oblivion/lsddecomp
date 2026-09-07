/* 25 queued, all fresh. Offered to runners in round 2026-08-30-a; no longer
 * banked. The first 25 of a 142-function block split at func_8005DE18; the
 * remainder is Entity_b, still an asm segment, and pairs well with a later
 * carve. No function in this unit touches a %gp_rel global, so none of it is
 * exposed to the gp-relative blocker (docs/research/gp-relative-blocker.md).
 */
#include "common.h"
#include "Entity.h"

Entity *New_Entity(void *arg0, void *arg1, void *arg2) {
    Entity *obj;

    obj = func_80017B34(0x108);
    if (obj == NULL) {
        return NULL;
    }
    if (Get_vtable_Entity()->ctor(obj, arg0, arg1, arg2) == NULL) {
        func_80017CFC(obj);
        return NULL;
    }
    return obj;
}

Entity *Entity__Entity(Entity *this, s32 arg1, s32 arg2, s32 arg3) {
    if (func_80066818()->ctor(this, arg2, arg3) != NULL) {
        this->methods = Get_vtable_Entity();
        this->moodIndex = arg1;
        this->unk9C = 0;
        this->unk100 = NULL;
        this->unk104 = NULL;
        this->methods->slot40(this);
        return this;
    }
    return NULL;
}

Unk100Obj *func_8005D108(Entity *this, void *name, void *arg2, void *arg3, s32 arg4) {
    Unk100Obj *cached;
    Unk100Obj *sub;
    Unk100Methods *m;
    void *dispatchArg2;

    cached = this->unk100;
    if (cached == NULL) {
        if (name == NULL) {
            name = D_8008AC14;
        }
        sub = func_8003FDB0(name, 0, arg4);
        if (sub == NULL) {
            return NULL;
        }
        this->unk100 = sub;
    } else {
        sub = cached;
    }
    sub->methods->slot50(sub);
    m = sub->methods;
    dispatchArg2 = arg2;
    if (dispatchArg2 == NULL) {
        dispatchArg2 = D_8008AC0C;
    }
    m->slot4C(sub, this, dispatchArg2);
    sub->methods->slotD0(sub, arg3);
    return sub;
}

void func_8005D1EC(Entity *this) {
    if (this->unk100 != NULL) {
        this->unk100->methods->slot04(this->unk100);
    }
    if (this->unk104 != NULL) {
        this->unk104->methods->slot04(this->unk104);
    }
    func_80066818()->dtor(this);
}

void func_8005D278(Entity *this) {
    s32 kind;

    kind = ((u8 *)D_80089EA6)[this->moodIndex * 0x10];
    if ((u32)(kind - 1) < 9) {
        this->methods->slot70(this, 1);
    }
    this->methods->slot10C(this, 0x42);
    this->methods->slot160(this);
}

INCLUDE_ASM("asm/nonmatchings/Entity", func_8005D314);

void func_8005D418(Entity *this) {
    if (this->unk0C != 0) {
        this->methods->slot160(this);
        func_80066818()->slot50(this);
        this->unk4C = 0;
    }
}

void func_8005D480(Entity *this, s32 a1, s32 a2) {
    if (this->methods->slot170(this) != 0) {
        this->methods->slot174(this);
    }
    if (this->methods->slot17C(this) != 0) {
        this->methods->slot180(this);
    }
    this->methods->slot178(this);
    func_80066818()->slot98(this, a1, a2);
}

void func_8005D560(Entity *this, s32 arg1, s32 arg2) {
    s32 linkStage;

    linkStage = D_80089EAB[this->moodIndex * 0x10];
    if ((u32)(arg2 - 2) < 7) {
        if (linkStage <= 0) {
            return;
        }
    }
    func_80066818()->slotDC(this, arg1, arg2);
    if (arg2 != 4) {
        return;
    }
    if (linkStage <= 0) {
        return;
    }
    if (linkStage != 0x7F) {
        arg2 = 0xA;
    } else if (D_80089EAC[this->moodIndex * 0x10] != 0) {
        arg2 = 0xB;
    } else {
        arg2 = 0xC;
    }
    this->methods->slot30(this, arg2);
}

void func_8005D658(Entity *this, s32 a1, s32 a2) {
    func_80066818()->slotE0(this, a1, a2);
    if (a2 == 4) {
        this->methods->slot160(this);
    }
}

void func_8005D6D4(Entity *this) {
    func_8002CD08(this->unk58, &this->unk9C);
    this->unkFC++;
}

typedef struct EntityVec3 EntityVec3;
struct EntityVec3 {
    s32 x;
    s32 y;
    s32 z;
};

s32 func_8005D714(Entity *this, void *pos, s32 arg2, s32 arg3) {
    EntityVec3 local;
    s32 kind;

    local = *(EntityVec3 *)pos;
    kind = ((u8 *)D_80089EA6)[this->moodIndex * 0x10];
    if ((u32)((kind + 9) & 0xFF) < 9) {
        local.y += (s8)kind * 1024;
    }
    if (arg3 < 0) {
        arg3 = 0x800 / (~arg3 + 1);
    } else {
        arg3 <<= 11;
    }
    return this->unk94->methods->slot120(this->unk94, 0, arg2 << 11, &local, arg3);
}

s32 func_8005D7FC(Entity *this, EntityRegionRef *region) {
    EntityRegionSlot *range;
    EntityPos *pos;
    s32 dx;
    s32 dz;

    range = NULL;
    if (region->flag != 0) {
        range = &region->slots[1];
    }
    pos = this->unk14;
    dx = pos->x - range->x0;
    if (dx < 0) {
        dx = ~dx + 1;
    }
    dz = pos->z - range->z0;
    return (dz >= 0) ? (dx + dz) : (dx - dz);
}

INCLUDE_ASM("asm/nonmatchings/Entity", func_8005D864);

void *Entity__GetMoodEffect(Entity *this) {
    return &D_80089EA4[this->moodIndex];
}

INCLUDE_ASM("asm/nonmatchings/Entity", Entity__GetUnlockEffect);

INCLUDE_ASM("asm/nonmatchings/Entity", Entity__GetLinkStage);

INCLUDE_ASM("asm/nonmatchings/Entity", Entity__GetEventVideo);

void func_8005D9F4(Entity *this) {
    this->methods->slot60(this, 1);
    this->unkF0 = 1;
    this->unk24 = 0;
}

void func_8005DA3C(Entity *this) {
    this->methods->slot60(this, 0);
    this->methods->slot16C(this);
    this->methods->slot164(this, 0);
    this->unkF0 = 0;
}

void func_8005DAAC(Entity *this, s32 arg1) {
    if (arg1 != 0) {
        this->methods->slot30(this, 9);
    }
    this->unkF4 = arg1;
}

typedef struct EntityMoodHandlerRow EntityMoodHandlerRow;
struct EntityMoodHandlerRow {
    void *handler; /* +0x00 */
    u8 pad04[0x10 - 0x04];
};
extern EntityMoodHandlerRow D_80089EB0[];
extern void func_8002CC34(s32 arg0, void *arg1, s32 arg2, Entity *arg3, void *arg4);

void func_8005DAFC(Entity *this) {
    func_8002CC34(this->unk58, &this->unk9C, this->moodIndex + 1, this,
                  D_80089EB0[this->moodIndex].handler);
    this->methods->slot12C(this);
    this->methods->slot110(this);
    this->unkFC = 0;
    this->unkF8 = 1;
}

void func_8005DB8C(Entity *this) {
    func_8002CC84(this->unk58, &this->unk9C);
    this->methods->slot130(this);
    this->methods->slot114(this);
    this->unkF8 = 0;
}

INCLUDE_ASM("asm/nonmatchings/Entity", func_8005DBF0);

s32 func_8005DD18(Entity *this) {
    EntityMoodRow *row;
    s32 doDetach;
    s32 dist;
    s32 scaled;

    if (this->unkF0 != 0) {
        row = &D_80089EA4[this->moodIndex];
        doDetach = 0;
        func_8005DF9C(this, 0);
        if (row->linkKind != 0 && row->linkKind != 3) {
            if (row->linkKind >= 10) {
                scaled = this->unk24;
                if ((scaled ^ (row->linkKind * 15)) == 0) {
                    doDetach = 1;
                }
            } else if (row->unk5 != 0) {
                dist = func_8005D714(this, &this->unk14->x, row->unk5, row->unk9);
                if (dist != 0) {
                    if (row->linkKind == 1) {
                        doDetach = 1;
                    }
                } else if (row->linkKind == 2) {
                    doDetach = 1;
                }
            }
        }
        if (doDetach) {
            this->methods->slot160(this);
        }
    }
    return this->unkF0;
}
