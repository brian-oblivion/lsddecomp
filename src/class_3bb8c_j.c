/*
 * class_3bb8c_j -- fourth carved slice of the class_3bb8c block
 * (0x41F84..0x429D4, vram 0x80051784..0x800521D4), 20 functions.
 * Carved round 15 out of the 193-function class_3bb8c_j remainder.
 *
 * Blocker profile (head's Gate 1 three-grep screen at carve time):
 *   func_800518F4  gp_rel     -- stub report filed, do not attempt
 *   func_80051998  gp_rel     -- stub report filed, do not attempt
 * The other 18 are clean. This unit owns NO switch jump table.
 *
 * include/class_3bb8c.h is SHARED with every other class_3bb8c_* slice.
 * Header edits must be strictly ADDITIVE.
 *
 * TWO classes share this unit's address range, discovered this round:
 *  - The first four functions (func_80051784/func_800517EC/func_80051814/
 *    func_80051858), plus the two gp_rel-blocked siblings, are Obj866E8
 *    methods (the SAME class already established across class_3bb8c_b/c/
 *    etc.) -- they touch offsets 0x10/0x14/0x18/0x1C/0x20/0x48, all
 *    previously-unnamed padding, and dispatch through the SAME
 *    Obj866E8Methods table (slotA4/slotA8, also newly named). See
 *    include/class_3bb8c.h's Obj866E8/Obj866E8Methods for the additive
 *    edits.
 *  - Everything from func_80051A4C on is a SEPARATE, much smaller sibling
 *    class (alloc size 0x54, vtable D_80086ED0 -- see func_80051A4C, a
 *    plain address-of getter, and func_80051A5C, its New_X allocator).
 *    Named `Class86ED0` here, LOCAL to this unit (not added to the shared
 *    header -- nothing else references it yet, and its own base-class
 *    getter `func_80018390()` already has an INCOMPATIBLE local view
 *    established by class_3bb8c_f inside class_3bb8c.h itself
 *    (`BasicMethods866E8F`), so redeclaring it with a different return
 *    type in a shared header would conflict. This unit's own view is
 *    `BaseMethods3bb8cJ`, following the same per-unit-local-view
 *    precedent as class_3bb8c_e.c's `BaseMethods3bb8cE`.). The real
 *    class is BasicClass (include/code_8220.h) -- slots 0x0C/0x10/0x14/
 *    0x18 line up exactly with BasicClassMethods' finalize/addChild/
 *    removeChild/removeAllChildren.
 */
#include "common.h"
#include "class_3bb8c.h"

void func_80051784(Obj866E8 *self)
{
    s32 count;

    if (self->unk48) {
        count = self->unk1C - 1;
        self->unk1C = count;
        if (count > 0) {
            self->methods->slotA8(self, self->unk18, count, 1);
        } else {
            self->unk1C = self->unk14;
        }
    }
}

void func_800517EC(Obj866E8 *self)
{
    if (self->unk48) {
        self->unk20 ^= 1;
    }
}

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_j", func_80051814);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_j", func_80051858);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_j", func_800518F4);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_j", func_80051998);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_j", func_80051A4C);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_j", func_80051A5C);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_j", func_80051AC8);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_j", func_80051C74);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_j", func_80051C84);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_j", func_80051D1C);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_j", func_80051DA0);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_j", func_80051E20);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_j", func_80051E64);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_j", func_80051F14);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_j", func_80051F24);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_j", func_800520A0);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_j", func_80052110);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_j", func_8005217C);
