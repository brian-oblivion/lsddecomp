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

INCLUDE_ASM("asm/nonmatchings/Entity", func_8005D108);

INCLUDE_ASM("asm/nonmatchings/Entity", func_8005D1EC);

INCLUDE_ASM("asm/nonmatchings/Entity", func_8005D278);

INCLUDE_ASM("asm/nonmatchings/Entity", func_8005D314);

INCLUDE_ASM("asm/nonmatchings/Entity", func_8005D418);

INCLUDE_ASM("asm/nonmatchings/Entity", func_8005D480);

INCLUDE_ASM("asm/nonmatchings/Entity", func_8005D560);

INCLUDE_ASM("asm/nonmatchings/Entity", func_8005D658);

void func_8005D6D4(Entity *this) {
    func_8002CD08(this->unk58, this->unk9C);
    this->unkFC++;
}

INCLUDE_ASM("asm/nonmatchings/Entity", func_8005D714);

INCLUDE_ASM("asm/nonmatchings/Entity", func_8005D7FC);

INCLUDE_ASM("asm/nonmatchings/Entity", func_8005D864);

void *Entity__GetMoodEffect(Entity *this) {
    return &D_80089EA4[this->moodIndex * 0x10];
}

INCLUDE_ASM("asm/nonmatchings/Entity", Entity__GetUnlockEffect);

INCLUDE_ASM("asm/nonmatchings/Entity", Entity__GetLinkStage);

INCLUDE_ASM("asm/nonmatchings/Entity", Entity__GetEventVideo);

void func_8005D9F4(Entity *this) {
    this->methods->slot60(this, 1);
    this->unkF0 = 1;
    this->unk24 = 0;
}

INCLUDE_ASM("asm/nonmatchings/Entity", func_8005DA3C);

void func_8005DAAC(Entity *this, s32 arg1) {
    if (arg1 != 0) {
        this->methods->slot30(this, 9);
    }
    this->unkF4 = arg1;
}

INCLUDE_ASM("asm/nonmatchings/Entity", func_8005DAFC);

void func_8005DB8C(Entity *this) {
    func_8002CC84(this->unk58, this->unk9C);
    this->methods->slot130(this);
    this->methods->slot114(this);
    this->unkF8 = 0;
}

INCLUDE_ASM("asm/nonmatchings/Entity", func_8005DBF0);

INCLUDE_ASM("asm/nonmatchings/Entity", func_8005DD18);
