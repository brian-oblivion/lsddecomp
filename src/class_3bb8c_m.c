/*
 * class_3bb8c_m -- seventh carved slice of the class_3bb8c block
 * (0x44518..0x44F14, vram 0x80053D18..0x80054714), 20 functions.
 * Carved round 15.
 *
 * Blocker profile (head's Gate 1 three-grep screen at carve time):
 *   func_800544E4  gp_rel
 *   func_80054558  gp_rel AND addiu-$at -- blocked until both move
 *   func_800545FC  addiu-$at
 *   func_80054660  gp_rel
 * All four have stub reports; do not attempt them. They are the LAST four in
 * ROM order, so the workable run is contiguous from the top of the unit.
 * This unit owns NO switch jump table.
 *
 * include/class_3bb8c.h is SHARED with every other class_3bb8c_* slice.
 * Header edits must be strictly ADDITIVE.
 */
#include "common.h"
#include "class_3bb8c.h"

/* Forward declaration: defined later in this same unit, but called by
 * func_80053D18/func_80053D9C/func_80053E00 above its own definition. */
extern void func_80053EB4(ObjM *self, s32 arg1, s32 arg2, s32 arg3, s32 arg4);

void func_80053D18(ObjM *self) {
    s32 val;
    self->unk20 = 7;
    self->unk3C->methods->slotF0(self->unk3C, &val, -1);
    func_80053EB4(self, val, 0, 5, 1);
    self->unk3C->methods->slotFC(self->unk3C);
}

void func_80053D9C(ObjM *self) {
    self->unk20 = 8;
    func_80053EB4(self, 0, 0, 6, 1);
    self->unk3C->methods->slotF4(self->unk3C, 1);
}

void func_80053E00(ObjM *self) {
    self->unk20 = 0xA;
    func_80053EB4(self, 0, 0, 6, 1);
    self->unk3C->methods->slot13C(self->unk3C, 2);
    self->unk3C->methods->slotF4(self->unk3C, 2);
}

void func_80053E84(ObjM *self) {
    self->methods->slot30(self, 0xB);
}

void func_80053EB4(ObjM *self, s32 arg1, s32 arg2, s32 arg3, s32 arg4) {
    ChildM_AC *obj = self->unk18->methods->slotAC(self->unk18);
    if (arg3 != 0) {
        obj->methods->slotD0(obj, arg3);
    }
    if (arg4 != 0) {
        self->methods->slot10(self, obj);
    }
    obj->methods->slotD8(obj, self->unk10, arg1, arg2);
}

void func_80053F84(ObjM *self, ParamM *p1, s32 sel) {
    s32 v;
    switch (sel) {
    case 5:
        self->methods->slot14(self, p1);
        self->unk3C->methods->slotF4(self->unk3C, 0);
        self->unk20 = 0;
        break;
    case 6:
        self->methods->slot14(self, p1);
        v = p1->methods->slotE4(p1);
        self->unk18->methods->slot64(self->unk18, v);
        if (self->unk20 != 5 && self->unk20 != 8 && self->unk20 == 0xA) {
            self->unk3C->methods->slot17C(self->unk3C, 1);
            self->unk3C->methods->slotF4(self->unk3C, 0);
            self->unk20 = 4;
        }
        self->methods->slot30(self, self->unk20);
        break;
    }
}

void func_800540E8(ObjM *self, s32 arg1, s32 arg2) {
    if (arg2 == 7) {
        self->methods->slotB8(self);
    }
}

s32 func_80054120(ObjM *self) {
    s32 out;
    s32 result;
    ChildM114 *child = self->unk14->methods->slot114(self->unk14, &out);
    void *thing = self->unk3C->methods->slot1A0(self->unk3C, 0);
    result = func_8005C7D4(child->unk4->unk34, &out, thing);
    child->unk14 = result;
    if (result != 0) {
        return 0;
    }
    child->unk4->methods->slot84(child->unk4);
    return 1;
}

void func_800541CC(void) {
}

void func_800541D4(ObjM *self) {
    if (self->unk80 != 0 && self->unk20 == 0) {
        self->unk84 = 1;
    }
}

void func_80054200(ObjM *self) {
    self->unk84 = 0;
}

void func_80054208(ObjM *self) {
    if (self->unk84) {
        self->methods->slotD4(self);
        self->methods->slot30(self, 0xD);
    }
}

void func_8005426C(ObjM *self) {
    if (self->unk84) {
        self->methods->slotD4(self);
        self->methods->slot30(self, 0xC);
    }
}

void func_800542D0(ObjM *self) {
    s32 state = self->unk80;
    if (state == 0) {
        self->unk7C = func_800408CC(self->unk74, 5, &D_8008AB44[0]);
        self->unk7C->methods->slot4C(self->unk7C, self->unk14, &D_8008AB38);
        self->unk7C->methods->slotB8(self->unk7C, &D_8008AB40);
        self->unk80 = state + 1;
        return;
    }
    self->unk80 = state + 1;
    if (state != 4) {
        return;
    }
    self->unk18->methods->slotB4(self->unk18, 0);
    self->unk10->methods->slot4C(self->unk10);
    self->unk54->methods->slot4C(self->unk54);
    self->unk34->methods->slot88(self->unk34);
}

void func_800543FC(ObjM *self) {
    if (self->unk80 != 0) {
        self->unk7C->methods->slot4(self->unk7C);
    }
    self->unk34->methods->slot8C(self->unk34);
    self->unk54->methods->slot50(self->unk54);
    self->unk10->methods->slot50(self->unk10);
    self->unk18->methods->slotB4(self->unk18, 1);
    self->unk80 = 0;
}

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_m", func_800544D4);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_m", func_800544E4);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_m", func_80054558);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_m", func_800545FC);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_m", func_80054660);
