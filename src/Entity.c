/* 25 queued, all fresh. Offered to runners in round 2026-08-30-a; no longer
 * banked. The first 25 of a 142-function block split at func_8005DE18; the
 * remainder is Entity_b, still an asm segment, and pairs well with a later
 * carve. No function in this unit touches a %gp_rel global, so none of it is
 * exposed to the gp-relative blocker (docs/research/gp-relative-blocker.md).
 */
#include "common.h"
#include "Entity.h"

INCLUDE_ASM("asm/nonmatchings/Entity", New_Entity);

INCLUDE_ASM("asm/nonmatchings/Entity", Entity__Entity);

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

INCLUDE_ASM("asm/nonmatchings/Entity", func_8005DAAC);

INCLUDE_ASM("asm/nonmatchings/Entity", func_8005DAFC);

INCLUDE_ASM("asm/nonmatchings/Entity", func_8005DB8C);

INCLUDE_ASM("asm/nonmatchings/Entity", func_8005DBF0);

INCLUDE_ASM("asm/nonmatchings/Entity", func_8005DD18);
