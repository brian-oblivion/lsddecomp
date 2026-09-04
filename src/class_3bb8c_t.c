/*
 * class_3bb8c_t -- functions 96..112 of the 113-function `class_3bb8c_n`
 * remainder, 0x48738..0x48F74 (vram 0x80057F38..0x80058774).  Carved
 * MID-round 17 (2026-09-04) to re-staff a runner whose own unit was
 * exhausted.  This is the LAST slice of the class_3bb8c block.
 *
 * Blocker census, three-grep screen run per function at carve time:
 * 16 of 17 clean.
 *
 * BLOCKED, stub report already filed, do NOT spend attempts on it:
 *   addiu_at: func_800585B4
 *
 * Four of the 17 are 2-instruction leaves that splat matched itself.
 *
 * This slice holds D_8001176C and D_80011778, the two strings left
 * STANDALONE when the 0x1EF4 rodata slot was split -- referenced from
 * func_80057FEC and func_80058084, both of which are in this unit.  A
 * string is referenced by SYMBOL and a standalone rodata object resolves
 * that fine (the `code_8220` / 0xA8C precedent in Gate 2), so no attach was
 * needed and the link came up green, which is the check that settles it.
 * The slice owns no `jtbl_` reference either.
 *
 * EXPECT THIS SLICE TO SPAN MORE THAN ONE CLASS.  It is cut at ROM
 * addresses, not class boundaries.  Identify each with tools/classtable.py.
 */
#include "common.h"

/* The class allocated by this unit's own func_80057F68, table D_800879C4
 * (49 slots, resolved via tools/classtable.py). Its ctor (func_80057D10)
 * and its own funcs 80057F38/40/48/50 live in the neighbouring
 * `class_3bb8c_p` unit (round 2026-09-04 earlier this round), which
 * already carries its own local view of this table
 * (`D_800879C4Methods`/`D_800879C4Obj` in that file). This unit's own
 * view is kept separate per the multiple-independent-local-views
 * convention -- func_80057F58 itself needs no fields, only the address. */
typedef struct D_800879C4Table D_800879C4Table;
extern D_800879C4Table D_800879C4;

extern void *func_80017B34(s32 size);

typedef struct D_80087AACObj D_80087AACObj;

/* This unit's own local view of the shared base-class table returned by
 * func_8003DFBC() (a plain no-argument getter, established elsewhere --
 * e.g. include/code_2c054.h, include/class_3bb8c.h -- as returning
 * &D_8006E730). Only the slots this unit's own functions dispatch
 * through are typed, per the project's "per-call-site signature"
 * convention (multiple units already carry independent local views of
 * this same table with different slot arities). */
typedef struct D_8006E730Methods {
    u8 pad00[0x44];
    /* +0x044, called by this unit's own func_80058390 as (self, arg1,
     * arg2). */
    void (*slot44)(D_80087AACObj *self, void *arg1, void *arg2);
} D_8006E730Methods;
extern D_8006E730Methods *func_8003DFBC(void);

void func_80057F38(void) {
}

void func_80057F40(void) {
}

void func_80057F48(void) {
}

void func_80057F50(void) {
}

D_800879C4Table *func_80057F58(void) {
    return &D_800879C4;
}

/* The class allocated below, table D_80087AAC (73 slots, resolved via
 * tools/classtable.py). This unit owns the whole class -- ctor, dtor and
 * every slot referenced from within it are all in this file. Only the
 * fields/slots each function actually touches are typed; the rest stay
 * opaque so the struct keeps the right size without requiring every
 * method to be named up front. */
typedef struct D_80087AACMethods {
    u8 pad00[0x8];
    /* +0x008, this unit's own ctor (func_80057FC8). */
    D_80087AACObj *(*ctor)(D_80087AACObj *self, void *arg1);
    u8 pad0C[0x70 - 0xC];
    /* +0x070, called by this unit's own func_800581C4 as (self, size). */
    void (*slot70)(D_80087AACObj *self, s32 arg1);
    u8 pad74[0x94 - 0x74];
    /* +0x094, called by this unit's own func_800581C4 as (self). */
    void (*slot94)(D_80087AACObj *self);
} D_80087AACMethods;
extern D_80087AACMethods *func_80058764(void);

struct D_80087AACObj {
    D_80087AACMethods *methods;
    u8 pad04[0x38 - 0x4];
    /* +0x038, read by this unit's own func_80058390. */
    s32 unk_0x38;
    u8 pad3C[0x238 - 0x3C];
    /* +0x238, read by this unit's own func_800581C4/func_80058390. */
    s32 unk_0x238;
};

void *func_80057F68(void *arg1) {
    void *obj = func_80017B34(0x244);
    if (obj != NULL) {
        func_80058764()->ctor(obj, arg1);
        return obj;
    }
    return NULL;
}

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_t", func_80057FC8);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_t", func_80058078);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_t", func_800580E0);

void func_800581C4(D_80087AACObj *self) {
    if (self->unk_0x238 == 0) {
        self->methods->slot70(self, 0x10);
        self->methods->slot94(self);
    }
}

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_t", func_80058228);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_t", func_80058308);

s32 func_80058390(D_80087AACObj *self, void *arg1, void *arg2) {
    s32 result;
    func_8003DFBC()->slot44(self, arg1, arg2);
    result = 2;
    if (self->unk_0x238 == 0) {
        result = self->unk_0x38;
    }
    return result;
}

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_t", func_80058404);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_t", func_800585B4);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_t", func_80058694);

extern D_80087AACMethods D_80087AAC;

D_80087AACMethods *func_80058764(void) {
    return &D_80087AAC;
}
