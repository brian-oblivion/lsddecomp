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

/* Local view of the object func_80027480/func_800282AC/func_80028A34 read
 * through -- the real struct is ObjA34_179D8H (src/code_179d8_h.c), but that
 * type is that unit's own local reading, not a shared header, so this unit
 * carries its own minimal view of the two offsets it actually touches. */
typedef struct Pos18 {
    s16 unk0;
    s16 unk2;
} Pos18; /* alignment 2, matches the project's lwl/lwr+swl/swr idiom */

typedef struct Obj80027480 {
    u8 pad0[0xC];
    s32 unk0C;
    u8 pad10[0x18 - 0x10];
    Pos18 unk18; /* CdlLOC-shaped position */
    u32 unk1C;
    u8 pad20[0x28 - 0x20];
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

/* Linear-scan table lookups over the 0x1C-byte string records at
 * D_8008A868 (src/code_179d8_r.c). Declared LOCAL here (own reading of the
 * trailing fields this function reads), not via a shared header. */
typedef struct Rec80028448 {
    u8 pad0[0x14];
    Pos18 unk14;
    u32 unk18;
} Rec80028448;

extern void *func_80028448(char *arg0);
extern s32 func_800284C4(char *arg0);
extern void *D_8008A87C;
extern s32 D_8008A898;

extern void func_80028920(Obj80027480 *self, char *suffix);
extern char *func_800289CC(char *dest, char *suffix);
extern s32 CdSearchFile(void *statBuf, char *path);
extern void CdControl(s32 arg0, void *buf, s32 arg2);
extern s32 CdSync(s32 mode, void *result);

typedef struct StatBuf80027 {
    Pos18 unk0;
    u32 unk4;
    u8 pad8[0x18 - 8];
} StatBuf80027;

void func_800272D0(Obj80027480 *self, char *suffix, s32 arg2, s32 arg3) {
    char path[0x40];
    StatBuf80027 statBuf;
    Rec80028448 *rec;
    s32 temp;
    s32 v0;

    if (D_8008A85C == 0 && D_8008A860 == 0) {
        func_80028920(self, suffix);
        return;
    }
    func_800280D0();
    if (self->unk28 != 0) {
        if (D_8008A864 == 0 && self->unk0C == 0) {
            func_80028844(1, 1);
            if (D_8008A85C != 0) {
                rec = func_80028448(suffix);
                D_8008A87C = rec;
                if (rec == NULL) {
                    return;
                }
                self->unk18 = rec->unk14;
                temp = ((Rec80028448 *)D_8008A87C)->unk18;
                D_8008A898 = 1;
                self->unk0C = 1;
                self->unk1C = temp;
            } else {
                func_800289CC(path, suffix);
                do {
                } while (CdSearchFile(&statBuf, path) == 0);
                self->unk18 = statBuf.unk0;
                self->unk1C = statBuf.unk4;
                do {
                    CdControl(2, &self->unk18, 0);
                    do {
                        v0 = CdSync(0, 0);
                    } while (v0 == 0);
                } while (v0 == 5);
                self->unk0C = 1;
                func_80028864();
            }
        }
    } else {
        func_800282AC(self, func_800284C4(suffix), 2, arg2, arg3);
    }
    func_800280E0();
}

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

extern u8 D_8006D574[8];
extern void *D_8008A87C;
extern s32 D_8008A898;

extern s32 func_80028A50(Obj80027480 *self);
extern s32 CdPosToInt(void *pos);
extern void CdIntToPos(s32 i, void *pos);
extern void CdControl(s32 arg0, void *buf, s32 arg2);
extern s32 CdSync(s32 mode, void *result);

s32 func_80027528(Obj80027480 *self, u32 arg1, s32 arg2) {
    s32 v0;
    u32 s0tmp;

    if (D_8008A85C == 0 && D_8008A860 == 0) {
        return func_80028A50(self);
    }
    func_800280D0();
    if (self->unk28 != 0) {
        if (D_8008A864 == 0 && self->unk0C != 0) {
            func_80028844(2, 1);
            s0tmp = arg1 >> 11;
            if ((arg1 & 0x7FF) != 0) {
                s0tmp = s0tmp + 1;
            }
            v0 = CdPosToInt(&self->unk18);
            CdIntToPos(v0 + s0tmp, D_8006D574);
            if (arg2 == 0) {
                if (D_8008A85C != 0) {
                    D_8008A87C = D_8006D574 - 0x14;
                    D_8008A898 = 1;
                } else {
                    do {
                        CdControl(2, D_8006D574, 0);
                        do {
                            v0 = CdSync(0, 0);
                        } while (v0 == 0);
                    } while (v0 == 5);
                    func_80028864();
                }
            } else {
                func_80028864();
                func_800280E0();
                if ((self->unk1C & 0x7FF) != 0) {
                    return ((self->unk1C >> 11) + 1) << 11;
                }
                return self->unk1C;
            }
        }
    } else {
        func_800282AC(self, 0, 4, (s32)arg1, arg2);
    }
    func_800280E0();
    return 0;
}

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
