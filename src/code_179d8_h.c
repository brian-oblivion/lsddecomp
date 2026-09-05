/*
 * code_179d8_h -- functions 43..59 of the original code_179d8 monolith's head,
 * 0x19098..0x194E0 (vram 0x80028898..0x800294E0).  Carved MID-round 17
 * (2026-09-04) to re-staff a runner whose own unit was exhausted.
 *
 * Blocker census, three-grep screen run per function at carve time:
 * 16 of 17 clean.
 *
 * BLOCKED, stub report already filed, do NOT spend attempts on it:
 *   gp_rel: func_80028B6C (only 3 instructions, so nothing is lost)
 *
 * Three of the entries carry real names inherited from FirecatFG's lsddecomp
 * (`strcpy`, `strstr`, `CdStatus`) -- treat those names as HYPOTHESES like any
 * other inherited symbol, but they are a strong hint about the shape.  Three
 * more are 2-instruction leaves that splat matched itself.
 *
 * The 43 functions in FRONT of this slice (still `code_179d8`) are
 * gp_rel-saturated -- 33 of 43 blocked -- and that remainder also owns this
 * segment's ONLY switch jump table (func_80027A24, which will need the Gate 2
 * rodata attach/split when it is carved).  The cut is placed here to leave
 * both debts behind: THIS slice owns no jump table and needs no rodata attach.
 *
 * Class-framework status: measured, not assumed.  Zero functions in this slice
 * reference any of the 60 method tables tools/classtable.py --scan finds.  The
 * sibling slice code_179d8_e DOES contain two class-table accessors, so the
 * "code_179d8 is not class-framework code" note is neighbourhood-scoped -- run
 * the check for your own functions rather than inheriting either verdict.
 */
#include "common.h"
/* code_171e0.h's UnkFlagsObj_171e0/UnkFlagsObjMethods_171e0 already
 * describe D_8006D430's class exactly -- func_80028898 dispatches
 * func_80026C9C()->ctor(self) (offset +0x008), matching that header's own
 * ctor slot. Reused UNCHANGED per CLAUDE.md's header discipline (a sibling
 * would use it unchanged), not redefined locally. */
#include "code_171e0.h"

/* func_80027E68 is still uncarved (asm/code_179d8.s) -- returns &D_8006D4E8,
 * a DIFFERENT class table (tools/classtable.py --scan: 29 slots, header
 * 0x13) than D_8006D430. func_80028898 chains UnkFlagsObj_171e0's base ctor
 * then overwrites self->methods with this class's own table -- the
 * standard "call base ctor, then install the derived vtable" idiom. Typed
 * against UnkFlagsObjMethods_171e0 for the assignment's sake; the two
 * tables are different classes but share the base's slot layout. */
extern UnkFlagsObjMethods_171e0 *func_80027E68(void);

/* func_80028A34/func_80028A50's own `self` -- offsets +0xC/+0x1C happen to
 * coincide with UnkFlagsObj_171e0::unk0C and its documented-unknown pad18
 * gap, but that header is code_171e0.c's shared reading and is off-limits
 * to edit here (out of unit) -- kept as this unit's own LOCAL, narrower
 * view per the project's multiple-independent-local-views convention. */
/* A 4-byte, alignment-2 pair -- the idiom CLAUDE.md documents for a struct
 * whose whole-struct assignment compiles to lwl/lwr + swl/swr instead of a
 * plain lw/sw (func_80028920's own unk18 copy needs this). Field meaning
 * unestablished beyond width/alignment. */
typedef struct Pair16_179D8H {
    s16 unk0;
    s16 unk2;
} Pair16_179D8H;

typedef struct ObjA34_179D8H ObjA34_179D8H;

/* ObjA34_179D8H's own methods table -- only the one slot func_80028A84
 * dispatches through is named. Total leading padding through +0xC is
 * unchanged from before this slot was identified (0x4 + 0x8 = 0xC), so
 * this is not a shifting edit -- confirmed by re-verifying func_80028A34
 * and func_80028A50 (both already matched, both readers of this struct)
 * after adding it. */
typedef struct MethodsA34_179D8H {
    u8 pad000[0x48];
    void (*slot48)(ObjA34_179D8H *self);
} MethodsA34_179D8H;

struct ObjA34_179D8H {
    MethodsA34_179D8H *methods;
    u8 pad4[0x0C - 0x04];
    s32 unk0C;
    u8 pad10[0x18 - 0x10];
    Pair16_179D8H unk18;
    u32 unk1C;
};

/* func_8002B640's own stat-like output buffer (func_80028920's local
 * `sp+0x10`). Only the two fields func_80028920 itself copies out are
 * named; the buffer runs up to sp+0x28, where func_80028920's own path
 * string buffer starts, so it's at least 0x18 bytes -- the rest is
 * unestablished. */
typedef struct StatBuf179D8H {
    Pair16_179D8H unk0;
    u32 unk4;
    u8 pad8[0x18 - 0x8];
} StatBuf179D8H;

/* func_8002B640: still uncarved in its own unit (code_179d8_g, BLOCKED
 * addiu_at there) -- declared LOCAL here, per-call-site typed. */
extern s32 func_8002B640(StatBuf179D8H *statBuf, char *path);
extern void func_80012C20(const char *fmt, void *arg1);
extern char D_800107F4[];

/* Forward declaration: func_800289CC is defined later in this file (ROM
 * order), but func_80028920 (earlier in ROM order) calls it. */
char *func_800289CC(char *dest, char *suffix);

/* func_800270B8, strcpy, strcat: still uncarved (code_171e0.c / this unit's
 * own later entries respectively) -- declared LOCAL, per-call-site typed. */
extern char *func_800270B8(void);
extern char *strcpy(char *dest, char *src);
extern char *strcat(char *dest, char *src);
extern char D_8008A8A8[]; /* ";1", the ISO9660 CD file-version suffix */

void func_80028898(UnkFlagsObj_171e0 *self) {
    ((UnkFlagsObjMethods_171e0 *)func_80026C9C())->ctor(self);
    self->methods = func_80027E68();
    self->unk0C = 0;
}

void *func_800288E0(UnkFlagsObj_171e0 *self) {
    return ((UnkFlagsObjMethods_171e0 *)func_80026C9C())->dtor(self);
}

void func_80028918(void) {
}

INCLUDE_ASM("asm/nonmatchings/code_179d8_h", func_80028920);

char *func_800289CC(char *dest, char *suffix) {
    dest[0] = '\\';
    strcpy(dest + 1, func_800270B8());
    strcat(dest, suffix);
    strcat(dest, D_8008A8A8);
    return dest;
}

void func_80028A34(ObjA34_179D8H *self) {
    if (self->unk0C != 0) {
        self->unk0C = 0;
    }
}

s32 func_80028A50(ObjA34_179D8H *self) {
    u32 result;

    if (self->unk0C == 0) {
        result = 0;
    } else {
        result = ((self->unk1C >> 11) + 1) << 11;
    }
    return result;
}

void func_80028A7C(void) {
}

extern void func_80028DF0(s32 arg0, Pair16_179D8H *buf, s32 arg2);
extern s32 func_80028D68(s32 arg0, void *buf);
extern s32 func_80029274(s32 arg0, void *arg1, s32 arg2);
extern s32 func_80029254(s32 arg0, s32 arg1);

INCLUDE_ASM("asm/nonmatchings/code_179d8_h", func_80028A84);

void func_80028B64(void) {
}

INCLUDE_ASM("asm/nonmatchings/code_179d8_h", func_80028B6C);

INCLUDE_ASM("asm/nonmatchings/code_179d8_h", strcpy);

INCLUDE_ASM("asm/nonmatchings/code_179d8_h", strstr);

extern u8 D_8006D60C;

u8 CdStatus(void) {
    return D_8006D60C;
}

extern u8 D_8006D61D;

u8 func_80028C44(void) {
    return D_8006D61D;
}

extern s32 func_8002A6EC(void);
extern s32 func_8002A75C(void);
extern s32 func_8002A5F8(void);

s32 func_80028C54(s32 mode) {
    if (mode == 2) {
        func_8002A6EC();
        return 1;
    }
    if (func_8002A75C() != 0) {
        return 0;
    }
    if (mode == 1) {
        if (func_8002A5F8() != 0) {
            return 0;
        }
    }
    return 1;
}

extern void func_8002A510(void);

void func_80028CC0(void) {
    func_8002A510();
}
