/*
 * class_3bb8c_l -- sixth carved slice of the class_3bb8c block
 * (0x435E0..0x44518, vram 0x80052DE0..0x80053D18), 20 functions.
 * Carved round 15.
 *
 * Blocker profile (head's Gate 1 three-grep screen at carve time):
 *   func_80052F10  addiu-$at
 *   func_800534C8  gp_rel
 *   func_80053984  addiu-$at, and it OWNS jtbl_8001174C -- the rodata slot
 *                  at 0x1F4C is attached to this unit for that reason
 * All three have stub reports; do not attempt them. The other 17 are clean
 * (func_80052DE0 and func_800534C0 are bare `jr $ra; nop` stubs splat
 * generated itself, so 15 are real work).
 *
 * include/class_3bb8c.h is SHARED with every other class_3bb8c_* slice.
 * Header edits must be strictly ADDITIVE.
 */
#include "common.h"
#include "class_3bb8c.h"

void func_80052DE0(void) {
}

void func_80052DE8(Obj87034_3bb8c_l *self, Obj87034_3bb8c_l *arg1, s32 arg2) {
    arg1->unkC->methods->slotC8(arg1->unkC, func_80052E7C, self);
    self->unk3C = (DreamSysObj_3bb8c_l *)arg2;
    func_8004A4B8()->slot44(self, arg1, 1);
    self->methods->slot10(self, arg2);
}

void func_80052E7C(Obj87034_3bb8c_l *self, s32 code, s32 arg2, s32 arg3) {
    if (code >= 0) {
        func_80049060(self->unk38);
    } else {
        func_80049098(self->unk38, arg2, arg3);
    }
}

void func_80052EBC(Obj87034_3bb8c_l *self) {
    self->methods->slot14(self, self->unk3C);
    func_8004A4B8()->slot48(self);
}

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_l", func_80052F10);

void func_80053134(Obj87034_3bb8c_l *self) {
    self->methods->slot84(self);
    func_8005C76C();
    func_80054D30();
    self->unk54->methods->slot48(self->unk54);
}

void func_800531A0(Obj87034_3bb8c_l *self, void *arg1, s32 sel) {
    if (sel == 2) {
        func_800531CC(self, self->unk58);
    }
}

void func_800531CC(Obj87034_3bb8c_l *self, Obj87034_3bb8c_l *other) {
    s32 ret;
    s32 sel;
    void *a1;
    Obj87034Methods_3bb8c_l *m;

    if (self->unk60 != 0) {
        if (other->unk80 != 0) {
            other->methods->slot04(other);
            self->unk60 = 0;
            self->methods->slot80(self);
            ret = self->unk3C->methods->slot108(self->unk3C);
            self->unk3C->methods->slot104(self->unk3C, ret + 0x1E);
        } else if (other->unk3C != 0) {
            sel = self->unk50->unk14;
            m = other->methods;
            if (sel != 2) {
                a1 = self->unk50->unk18;
            } else {
                a1 = self->unk50->unkC;
            }
            m->slot7C(other, a1);
            other->methods->slot04(other);
            self->unk60 = 0;
            self->methods->slot80(self);
        }
    }
    if (self->unk60 == 0) {
        if (self->unk14->unk1B4 == 0 && self->unk68 == 0) {
            self->unk64 = 1;
            self->methods->slot88(self);
        }
    }
}

void func_80053358(Obj87034_3bb8c_l *self, void *arg1, s32 eventId) {
    Obj87034Methods_3bb8c_l *m = self->methods;
    void (*fn)(Obj87034_3bb8c_l *);

    if (self->unk68 == 0) {
        return;
    }
    if (eventId == 0x16) {
        goto case_c8;
    }
    if (eventId < 0x17) {
        if (eventId == 0xC) {
            goto case_c0;
        }
        return;
    }
    if (eventId == 0x21) {
        goto case_74;
    }
    if (eventId == 0x2C) {
        goto case_c4;
    }
    return;
case_74:
    fn = m->slot74;
    goto call;
case_c0:
    fn = m->slotC0;
    goto call;
case_c8:
    fn = m->slotC8;
    goto call;
case_c4:
    fn = m->slotC4;
call:
    fn(self);
}

void func_800533F0(Obj87034_3bb8c_l *self) {
    void (*fn)(Obj87034_3bb8c_l *);

    if (self->unk68 != 0) {
        self->unk1C++;
        if (self->unk80 != 0) {
            fn = self->methods->slotD0;
        } else {
            fn = self->methods->slot8C;
        }
        fn(self);
    }
}

void func_80053458(Obj87034_3bb8c_l *self) {
    Obj87034Methods_3bb8c_l *m = self->methods;

    if (self->unk80 != 0) {
        m->slotC4(self);
        m->slotD4(self);
    } else {
        m->slotD0(self);
    }
}

void func_800534C0(void) {
}

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_l", func_800534C8);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_l", func_800536B0);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_l", func_80053764);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_l", func_8005393C);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_l", func_80053984);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_l", func_80053ACC);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_l", func_80053BE8);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_l", func_80053C94);
