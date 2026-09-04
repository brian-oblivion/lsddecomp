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
 *    header -- nothing else references it yet). Its base class IS
 *    BasicClass (include/code_8220.h): slots 0x0C/0x10/0x14/0x18 line up
 *    exactly with BasicClassMethods' finalize/addChild/removeChild/
 *    removeAllChildren. Reached via func_80018390(), which class_3bb8c.h
 *    ALREADY declares (class_3bb8c_f's own local view, `BasicMethods866E8F`)
 *    -- this round additively named those four slots on THAT existing
 *    type rather than adding a second, incompatible local declaration of
 *    the same function (which would conflict in this translation unit).
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

void func_80051814(Obj866E8 *self)
{
    if (self->unk48) {
        self->unk1C = 0;
        self->methods->slotA8(self, self->unk18, 0, 1);
    }
}

void func_80051858(Obj866E8 *self)
{
    s32 i;

    if (self->unk48) {
        self->unk1C = 0;
        i = self->unk10 - 1;
        if (i >= 0) {
            do {
                self->unk18 = i;
                self->methods->slotA8(self, i, self->unk1C, 0);
                i--;
            } while (i >= 0);
        }
        self->methods->slotA4(self, self->unk18, 1);
    }
}

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_j", func_800518F4);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_j", func_80051998);

/*
 * Class86ED0 -- a small BasicClass-derived sibling class, LOCAL to this
 * unit (see the file header comment for why this is not added to the
 * shared class_3bb8c.h). Alloc size 0x54 (func_80051A5C). Vtable
 * D_80086ED0 (func_80051A4C, a plain address-of getter).
 *
 * unk34/unk38 are single-slot caches for the most recently added child of
 * two distinguished "tag" kinds (established from func_80051D1C/
 * func_80051DA0: a child object's own `*(s32*)(*(void**)child) & 0xF`
 * selects unk34 for tag 2, unk38 for tag 5), layered on top of the
 * INHERITED BasicClass generic children list (added/removed via
 * func_80018390()'s addChild/removeChild in the same two functions).
 */
typedef struct Class86ED0Methods Class86ED0Methods; /* opaque -- no slot this round's functions dispatch through */
typedef struct Class86ED0 Class86ED0;

extern Class86ED0Methods D_80086ED0;

Class86ED0Methods *func_80051A4C(void)
{
    return &D_80086ED0;
}

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
