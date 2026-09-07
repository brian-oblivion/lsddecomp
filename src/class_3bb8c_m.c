/*
 * class_3bb8c_m -- seventh carved slice of the class_3bb8c block
 * (0x44518..0x44F14, vram 0x80053D18..0x80054714), 20 functions.
 * Carved round 15.
 *
 * Blocker profile -- RE-SCREENED round 23 (2026-09-07). The carve-time screen
 * was a THREE-grep screen; `addiu_at` was resolved in round 21 (maspsx
 * `--addiu-at`; docs/research/addiu-at-blocker.md) and screening for it now
 * INVENTS blockers, so the live screen is TWO greps -- `gp_rel` and
 * `nop_mflo_mfhi`. Current state:
 *   func_800544E4  gp_rel                -- still blocked
 *   func_80054558  gp_rel (+ addiu-$at)  -- still blocked, on gp_rel alone
 *   func_800545FC  was addiu-$at ONLY    -- NOT BLOCKED. MATCHED round 23, 25/25.
 *   func_80054660  gp_rel                -- still blocked
 * The old profile said "all four have stub reports; do not attempt them",
 * which was true when written and became a false blocker on one of the four
 * the moment `addiu_at` was fixed. Screen with `python3 tools/nearmiss.py`
 * rather than trusting any transcribed profile, this one included.
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

void *func_800544D4(void) {
    return &D_80087034;
}

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_m", func_800544E4);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_m", func_80054558);

/* func_800545FC's destination is NOT an `ObjM`. That struct's +0x014 and +0x018
 * are already established as unrelated object pointers by five other functions
 * in this unit (`FieldM14 *`/`FieldM18 *`), whereas this function writes a
 * colour-table POINTER to +0x018 and a plain sign-extended byte to +0x014. So
 * this is a separate descriptor, and its view stays LOCAL rather than going
 * into include/class_3bb8c.h -- which eleven units share, and where adding
 * `unkC`/`unk1C` to `ObjM` on this evidence would be a claim the bytes do not
 * support.
 *
 * D_800872C4 is a table of 24 three-byte entries (0x48 bytes; the first four
 * are 00/00/00, 40/40/40, 80/80/80, FF/FF/FF -- a greyscale ramp, so RGB
 * triples). Indexing it as `u8[][3]` is what produces retail's `i*2 + i + base`
 * stride-3 address arithmetic. D_8008730C is six words, 0x6800 down to 0x0800. */
struct StyleM {
    u8 pad000[0x00C];
    const u8 *unkC;                 /* +0x00C, a D_800872C4 entry */
    u8 pad010[0x014 - 0x010];
    s32 unk14;                      /* +0x014, cfg[0] sign-extended */
    const u8 *unk18;                /* +0x018, a D_800872C4 entry */
    s32 unk1C;                      /* +0x01C, a D_8008730C value */
};

extern u8 D_800872C4[][3];
extern s32 D_8008730C[];

void func_800545FC(struct StyleM *style, s8 *cfg) {
    style->unkC = D_800872C4[cfg[3]];
    style->unk18 = D_800872C4[cfg[2]];
    style->unk1C = D_8008730C[cfg[1]];
    style->unk14 = cfg[0];
}

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_m", func_80054660);
