/*
 * class_3bb8c_p -- functions 74..93 of the 113-function `class_3bb8c_n`
 * remainder, 0x47CC4..0x485BC (vram 0x800574C4..0x80057DBC).  Carved round 17
 * (2026-09-04), immediately behind `class_3bb8c_o`.
 *
 * Blocker census, three-grep screen run per function at carve time:
 * 20 of 20 clean -- no gp_rel, no addiu-$at, no nop_mflo_mfhi anywhere in the
 * slice.  This is the cleanest window found in the whole executable this
 * round.  Four of the 20 are two-word leaves; splat matched two of them
 * itself, so the queue below is 18.
 *
 * Owns NO switch jump table -- zero `jtbl_` references in the slice -- so no
 * rodata sub-slot is attached to this unit.
 *
 * EXPECT THIS SLICE TO SPAN MORE THAN ONE CLASS.  It is cut at ROM addresses,
 * not at class boundaries, and round 15 measured three of five such slices
 * spanning two or more vtables.  Identify each class with tools/classtable.py
 * rather than assuming the unit has one.  A class that spans a carve boundary
 * is also the normal reason two units name the same table -- see the
 * multiple-independent-local-views convention in CLAUDE.md before deciding
 * whether your view of one belongs in include/class_3bb8c.h or here.
 */
#include "common.h"
#include "DreamSys.h"

/* Two-element s16 array -- func_800574C4 and func_800574FC each write one
 * element (index 0 and 1 respectively) via a plain `sh` through a pointer
 * computed as %hi/%lo of `D_8008ABA4 + 2*index`, so splat's single-word
 * dlabel is really this 2-element array, not a lone s32 (round 2026-09-04).
 * Not referenced anywhere else in the repo (checked with grep), so this is
 * this unit's own reading -- kept local rather than added to a shared
 * header. */
extern s16 D_8008ABA4[2];

void func_80057534(DreamSys *self, s16 *slot, s32 val, void *extra, volatile s32 count);

void func_800574C4(DreamSys *self, s32 val, void *extra) {
    func_80057534(self, &D_8008ABA4[0], val, extra, 7);
}

void func_800574FC(DreamSys *self, s32 val, void *extra) {
    func_80057534(self, &D_8008ABA4[1], val, extra, 8);
}

/* `count` is `volatile` so it stays a stack reference reloaded at its one use
 * site, rather than being promoted to a callee-saved register across the
 * intervening func_80057444 call -- confirmed with a standalone reproducer
 * through the pinned toolchain: dropping `volatile` grows the frame by one
 * callee-saved register (s3) and changes 0x1c/0x20 byte offsets throughout,
 * which is not what retail does (round 2026-09-04).
 *
 * `val` arrives as `s32` (its callers forward an incoming register with no
 * conversion -- typing it `s16` here made the CALLERS re-sign-extend it on
 * every call, which retail does not do), but the two stores below are
 * genuinely 16-bit (`sh`). Truncating once into a local `s16` and storing
 * THAT (rather than truncating `val` twice inline) is what reproduces
 * retail's callee-saved register assignment for `slot`/`extra`
 * (confirmed with a standalone reproducer: inline truncation swaps which
 * of s0/s1 holds which, round 2026-09-04). */
void func_80057534(DreamSys *self, s16 *slot, s32 val, void *extra, volatile s32 count) {
    s16 val16 = (s16) val;
    *slot = val16;
    self->field_0x48 = val16;
    self->vt->func_80057444(self, &D_8008ABA4[0]);
    *slot = 0;
    if (extra != NULL) {
        self->vt->func_80058B08(self, count);
    }
}

void func_80057618(DreamSys *self, void (*callback)(DreamSys *, s32, void *), s32 val, void *extra);

void func_800575B0(DreamSys *self, s32 val, void *extra) {
    func_80057618(self, self->vt->func_8005748C, val, extra);
}

void func_800575E0(DreamSys *self, s32 val, void *extra) {
    func_80057618(self, self->vt->func_800574C4, val, extra);
}

void func_80057610(void) {
}

void func_80057668(DreamSys *self);

void func_80057618(DreamSys *self, void (*callback)(DreamSys *, s32, void *), s32 val, void *extra) {
    self->unk_0x28 = NULL;
    callback(self, val, extra);
    if (self->unk_0x28 == NULL) {
        func_80057668(self);
    }
}

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_p", func_80057668);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_p", func_80057784);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_p", func_80057954);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_p", func_80057A18);

extern s32 func_8001E7BC(void);

void *func_80057B54(void *arg0, void *arg1, void *arg2) {
    if (arg0 != NULL) {
        if (func_8001E7BC() != 0) {
            return arg0;
        }
    }
    return NULL;
}

/* This unit's own local view of func_8001E57C()'s return, matching only
 * the one slot this unit's own functions dispatch through directly (the
 * OTHER slot these functions use, +0xA0, is reached through the object's
 * OWN vtable instead -- see include/DreamSys.h's `slotA0`) -- per the
 * project's established "per-call-site signature" precedent
 * (include/code_d294.h's own file banner; that header's `Class6B5CCMethods`
 * types this SAME slot with a different argument count for ITS OWN call
 * sites, which is fine because the actual callee ignores unused trailing
 * register arguments). Kept local rather than added to code_d294.h. */
typedef struct DreamSysBasicSlots {
    u8 pad00[0x9C];
    void (*slot9C)(DreamSys *self, void *arg1, s32 count);
} DreamSysBasicSlots;
extern DreamSysBasicSlots *func_8001E57C(void);

void func_80057B90(DreamSys *self, void *arg1, s32 count) {
    func_8001E57C()->slot9C(self, arg1, count);
    if (count < 9) {
        if (count >= 5) {
            self->vt->slotA0(self, arg1, count);
        }
    }
}

void func_80057C14(DreamSys *self, void *arg1, s32 count) {
    func_8001E57C()->slot9C(self, arg1, count);
}

void func_80057C6C(DreamSys *self, s16 val) {
    self->field_0x48 = val;
}

void func_80057C74(void) {
}

void func_80057C7C(DreamSys *self, void *extra) {
    self->unk_0x54 = extra;
}

/* The shared intermediate base-class table -- see include/DreamSys.h's
 * `DreamSysBaseMethods` comment and include/code_55dd4.h's own independent
 * view (`D800878D4Methods`) of the SAME table. This unit's own extern,
 * typed to match the return type `func_80057C84` already carried in
 * include/DreamSys.h (round 2026-09-04). */
extern DreamSysBaseMethods D_800878D4;

DreamSysBaseMethods *func_80057C84(void) {
    return &D_800878D4;
}

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_p", func_80057C94);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_p", func_80057D10);
