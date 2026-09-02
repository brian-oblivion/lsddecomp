/* Third slice of the Entity block -- 20 functions, 0x4F754..0x5077C. The
 * remainder is `Entity_d` and is still a monolithic asm segment.
 *
 * The whole 97-function remainder this came out of has zero `jlabel`s and
 * zero `jr $t2`, so no slice of it needs a rodata slot attached and none of
 * it is a BIOS trampoline. Entity/Entity_b's `include/Entity.h` is already
 * heavily typed and these functions are the same class family -- extend that
 * header rather than starting a new one.
 */
#include "common.h"
#include "Entity.h"

extern u8 D_80089E38[];

INCLUDE_ASM("asm/nonmatchings/Entity_c", func_8005EF54);

INCLUDE_ASM("asm/nonmatchings/Entity_c", func_8005EFF4);

INCLUDE_ASM("asm/nonmatchings/Entity_c", func_8005F0D8);

void func_8005F1A8(Entity *this) {
    func_8001EACC(this, this->unk94, 1, 0, 0);
}

INCLUDE_ASM("asm/nonmatchings/Entity_c", func_8005F1D4);

INCLUDE_ASM("asm/nonmatchings/Entity_c", func_8005F368);

INCLUDE_ASM("asm/nonmatchings/Entity_c", func_8005F454);

INCLUDE_ASM("asm/nonmatchings/Entity_c", func_8005F544);

INCLUDE_ASM("asm/nonmatchings/Entity_c", func_8005F608);

s32 func_8005F6D4(Entity *this) {
    return this->methods->slot48(this, 1, D_80089E38);
}

INCLUDE_ASM("asm/nonmatchings/Entity_c", func_8005F708);

INCLUDE_ASM("asm/nonmatchings/Entity_c", func_8005F800);

INCLUDE_ASM("asm/nonmatchings/Entity_c", func_8005F970);

void func_8005FA64(Entity *this) {
    this->methods->slotC4(this, -0x1E, 0);
}

INCLUDE_ASM("asm/nonmatchings/Entity_c", func_8005FA94);

INCLUDE_ASM("asm/nonmatchings/Entity_c", func_8005FB6C);

INCLUDE_ASM("asm/nonmatchings/Entity_c", func_8005FC58);

INCLUDE_ASM("asm/nonmatchings/Entity_c", func_8005FDFC);

s32 func_8005FEC8(Entity *this) {
    return this->methods->slotCC(this, -0x5A, 0);
}

INCLUDE_ASM("asm/nonmatchings/Entity_c", func_8005FEF8);
