/*
 * code_179d8_s -- the last 7 functions of the original `code_179d8` monolith's
 * head, 0x17AD0..0x18480 (620 words).  Carved round 47 (2026-09-16); with
 * class_3bb8c_q this was the last uncarved game code in the executable.
 *
 * Census 2026-09-16, `tools/uncarved.py --functions`: 7 of 7 blocker-clean.
 * The `gp_rel` tags that tool prints for six of them are
 * RESOLVED-not-a-blocker (maspsx --gp-symbols, round 42) -- an ordinary
 * `lw $v0, %gp_rel(sym)($gp)` is a plain global access here, not a wall.
 * Gate 2 boundary checks all zero: no `jr $t2` trampoline, no `alabel`, no
 * non-`.L` alt-entry label, no function with two prologues.
 *
 * func_80027A24 owns this unit's only jump tables (jtbl_80010810 and
 * jtbl_80010828).  The 0xFD8 rodata slot was SPLIT at 0x1010 to attach them:
 * the two strings in the same slot belong to code_179d8_q and code_179d8_h
 * and stay standalone.  You do not need to do anything about this -- it is
 * recorded so that a link error mentioning either symbol is attributable.
 *
 * Expect this slice to span more than one class; identify each with
 * `tools/classtable.py` rather than assuming the unit has one.
 */

#include "common.h"

INCLUDE_ASM("asm/nonmatchings/code_179d8_s", func_800272D0);

/* Local view of the object func_80027480/func_800282AC/func_80028A34 read
 * through -- the real struct is ObjA34_179D8H (src/code_179d8_h.c), but that
 * type is that unit's own local reading, not a shared header, so this unit
 * carries its own minimal view of the two offsets it actually touches. */
typedef struct Obj80027480 {
    u8 pad0[0xC];
    s32 unk0C;
    u8 pad10[0x28 - 0x10];
    u16 unk28;
} Obj80027480;

extern s32 D_8008A85C;
extern s32 D_8008A860;
extern s32 D_8008A864;

extern void func_80028A34(Obj80027480 *self);
extern void func_800280D0(void);
extern void func_80028844(s32 arg0, s32 arg1);
extern void func_80028864(void);
extern void func_800282AC(Obj80027480 *arg0, s32 arg1, s32 arg2, s32 arg3,
                           s32 arg4);
extern void func_800280E0(void);

void func_80027480(Obj80027480 *self) {
    if (D_8008A85C == 0 && D_8008A860 == 0) {
        func_80028A34(self);
        return;
    }
    func_800280D0();
    if (self->unk28 != 0) {
        if (D_8008A864 == 0) {
            func_80028844(0, 0);
            self->unk0C = 0;
            func_80028864();
        }
    } else {
        func_800282AC(self, 0, 3, 0, 0);
    }
    func_800280E0();
}

INCLUDE_ASM("asm/nonmatchings/code_179d8_s", func_80027528);

void func_800276C8(void) {
}

extern s32 D_8008A880; /* CdRead sector count */
extern void *D_8008A884; /* CdRead target buffer */
extern s32 D_8008A898;

extern void func_80028A84(Obj80027480 *self, void *arg1, s32 arg2);
extern s32 CdRead(s32 sectors, void *buf, s32 mode);
extern s32 CdReadSync(s32 mode, s32 result);
extern void func_80028864(void);

s32 func_800276D0(Obj80027480 *self, void *buf, u32 size) {
    s32 v1;

    if (D_8008A85C == 0 && D_8008A860 == 0) {
        func_80028A84(self, buf, size);
        return 0;
    }
    func_800280D0();
    if (self->unk28 != 0) {
        if (D_8008A864 == 0 && self->unk0C != 0) {
            func_80028844(3, 7);
            if (D_8008A85C != 0) {
                D_8008A880 = size >> 11;
                D_8008A884 = buf;
                D_8008A898 = 1;
            } else {
            retry:
                CdRead(size >> 11, buf, 0x80);
                do {
                    v1 = CdReadSync(0, 0);
                } while (v1 > 0);
                if (v1 == -1) {
                    goto retry;
                }
                func_80028864();
            }
        }
    } else {
        func_800282AC(self, 0, 5, (s32)buf, size);
    }
    func_800280E0();
    return 0;
}

INCLUDE_ASM("asm/nonmatchings/code_179d8_s", func_80027800);

INCLUDE_ASM("asm/nonmatchings/code_179d8_s", func_80027A24);
