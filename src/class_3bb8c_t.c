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
    u8 pad00[0x8];
    /* +0x008, called by this unit's own func_80057FC8 as (self, 0, str,
     * 0). */
    void (*slot8)(D_80087AACObj *self, s32 arg1, char *str, s32 arg2);
    u8 pad0C[0x44 - 0xC];
    /* +0x044, called by this unit's own func_80058390 as (self, arg1,
     * arg2). */
    void (*slot44)(D_80087AACObj *self, void *arg1, void *arg2);
    u8 pad48[0x5C - 0x48];
    /* +0x05C, called by this unit's own func_800580E0 as (self, arg1,
     * arg2). */
    void (*slot5C)(D_80087AACObj *self, void *arg1, void *arg2);
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
    u8 pad0C[0x40 - 0xC];
    /* +0x040, called by this unit's own func_80057FC8 as (self, arg1) --
     * a TAIL CALL, its return value forwarded as func_80057FC8's own. */
    void *(*slot40)(D_80087AACObj *self, void *arg1);
    u8 pad44[0x6C - 0x44];
    /* +0x06C, called by this unit's own func_80058078 as (self, flag). */
    void (*slot6C)(D_80087AACObj *self, s32 arg1);
    /* +0x070, called by this unit's own func_800581C4 as (self, size). */
    void (*slot70)(D_80087AACObj *self, s32 arg1);
    u8 pad74[0x94 - 0x74];
    /* +0x094, called by this unit's own func_800581C4 as (self). */
    void (*slot94)(D_80087AACObj *self);
    u8 pad98[0xD4 - 0x98];
    /* +0x0D4, called by this unit's own func_80058078 as (self, str,
     * 0). */
    void (*slotD4)(D_80087AACObj *self, char *str, s32 arg2);
    /* +0x0D8, called by this unit's own func_80057FC8 as (self, 0). */
    void (*slotD8)(D_80087AACObj *self, s32 arg1);
    u8 padDC[0x124 - 0xDC];
    /* +0x124, this unit's own func_80058694, called by func_800580E0 as
     * (self). */
    void (*slot124)(D_80087AACObj *self);
} D_80087AACMethods;
extern D_80087AACMethods *func_80058764(void);

/* This unit's own view of one entry of D_80087AACObj::unk_0xA8 -- only
 * the one slot this unit's own func_80058694 dispatches through is
 * typed. */
typedef struct D_80087AACEntry D_80087AACEntry;
typedef struct D_80087AACEntryMethods {
    u8 pad00[0x60];
    /* +0x060, called by this unit's own func_800580E0 as (self, flag). */
    void (*slot60)(D_80087AACEntry *self, s32 arg1);
    u8 pad64[0xB8 - 0x64];
    /* +0x0B8, called by this unit's own func_80058694 as (self, 1,
     * &global). */
    void (*slotB8)(D_80087AACEntry *self, s32 arg1, void *arg2);
} D_80087AACEntryMethods;
struct D_80087AACEntry {
    D_80087AACEntryMethods *methods;
};

/* Object pointed to by D_80087AACObj::unk_0x48 -- only the one slot
 * this unit's own func_80057FC8 dispatches through is typed. */
typedef struct D_80087AACUnk48Obj D_80087AACUnk48Obj;
typedef struct D_80087AACUnk48Methods {
    u8 pad00[0x9C];
    /* +0x09C, called by this unit's own func_80057FC8 as (self, -1). */
    void (*slot9C)(D_80087AACUnk48Obj *self, s32 arg1);
} D_80087AACUnk48Methods;
struct D_80087AACUnk48Obj {
    D_80087AACUnk48Methods *methods;
};

/* Object pointed to by D_80087AACObj::unk_0xA4 -- passed in as this
 * unit's own ctor's `arg1` (func_80057FC8) and dispatched through by
 * func_800580E0/func_80058404 (both still queued at slot +0x1B0). Only
 * that one slot is typed. */
typedef struct D_80087AACUnkA4Obj D_80087AACUnkA4Obj;
/* Return type of D_80087AACUnkA4Methods::slot1B0 -- only the two fields
 * this unit's own func_800580E0 reads are named. */
typedef struct D_80087AACUnkA4Result {
    u8 pad00[0x4];
    s32 unk_0x4;
    s32 unk_0x8;
} D_80087AACUnkA4Result;
typedef struct D_80087AACUnkA4Methods {
    u8 pad00[0x1B0];
    /* +0x1B0, called by this unit's own func_800580E0/func_80058404 as
     * (self, 0). */
    D_80087AACUnkA4Result *(*slot1B0)(D_80087AACUnkA4Obj *self, s32 arg1);
} D_80087AACUnkA4Methods;
struct D_80087AACUnkA4Obj {
    D_80087AACUnkA4Methods *methods;
};

struct D_80087AACObj {
    D_80087AACMethods *methods;
    u8 pad04[0x1C - 0x4];
    /* +0x01C, read by this unit's own func_80058694 (unsigned
     * comparisons -- `sltiu`). */
    u32 unk_0x1C;
    u8 pad20[0x2C - 0x20];
    /* +0x02C, written by this unit's own func_80058078. */
    s32 unk_0x2C;
    u8 pad30[0x38 - 0x30];
    /* +0x038, read by this unit's own func_80058390. */
    s32 unk_0x38;
    /* +0x03C, read by this unit's own func_800580E0. */
    s32 unk_0x3C;
    u8 pad40[0x48 - 0x40];
    /* +0x048, read by this unit's own func_80057FC8. */
    D_80087AACUnk48Obj *unk_0x48;
    u8 pad4C[0x84 - 0x4C];
    /* +0x084, written by this unit's own func_80058078. */
    s32 unk_0x84;
    u8 pad88[0xA4 - 0x88];
    /* +0x0A4, set by this unit's own ctor (func_80057FC8) to its own
     * `arg1`; dispatched through by func_800580E0/func_80058404. */
    D_80087AACUnkA4Obj *unk_0xA4;
    /* +0x0A8, a 100-entry array of `D_80087AACEntry *` -- built by this
     * unit's own func_80058228 (still queued), destroyed by
     * func_80058308 (still queued), indexed by func_80058694. */
    D_80087AACEntry *unk_0xA8[100];
    /* +0x238, read by this unit's own func_800581C4/func_80058390. */
    s32 unk_0x238;
    /* +0x23C, read/written by this unit's own func_80058694 (unsigned
     * comparison -- `sltiu`). */
    u32 unk_0x23C;
    /* +0x240, read by this unit's own func_80058694. */
    s8 *unk_0x240;
};

void *func_80057F68(void *arg1) {
    void *obj = func_80017B34(0x244);
    if (obj != NULL) {
        func_80058764()->ctor(obj, arg1);
        return obj;
    }
    return NULL;
}

extern char D_8001176C[];

void *func_80057FC8(D_80087AACObj *self, void *arg1) {
    func_8003DFBC()->slot8(self, 0, D_8001176C, 0);
    self->methods = func_80058764();
    self->unk_0x48->methods->slot9C(self->unk_0x48, -1);
    self->unk_0xA4 = arg1;
    self->methods->slotD8(self, 0);
    return self->methods->slot40(self, arg1);
}

extern char D_80011778[];

void func_80058078(D_80087AACObj *self) {
    self->unk_0x84 = 5;
    self->unk_0x2C = 0x190;
    self->methods->slotD4(self, D_80011778, 0);
    self->methods->slot6C(self, 0xA);
}

void func_800580E0(D_80087AACObj *self, void *arg1, void *arg2) {
    func_8003DFBC()->slot5C(self, arg1, arg2);
    if (self->unk_0x3C == 1) {
        D_80087AACUnkA4Result *result = self->unk_0xA4->methods->slot1B0(self->unk_0xA4, 0);
        if (result->unk_0x4 != 0 || result->unk_0x8 != 0) {
            self->unk_0xA8[0]->methods->slot60(self->unk_0xA8[0], self->unk_0x1C & 1);
        }
    }
    self->methods->slot124(self);
}

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

extern s32 D_8008ABBC;

void func_80058694(D_80087AACObj *self) {
    if (self->unk_0x238 != 0) {
        if (self->unk_0x1C >= 0x1F) {
            if (self->unk_0x23C < 4) {
                if ((self->unk_0x1C % 24) == 0) {
                    s8 idx = self->unk_0x240[self->unk_0x23C];
                    self->unk_0xA8[idx]->methods->slotB8(self->unk_0xA8[idx], 1, &D_8008ABBC);
                    self->unk_0x23C += 1;
                }
            }
        }
    }
}

extern D_80087AACMethods D_80087AAC;

D_80087AACMethods *func_80058764(void) {
    return &D_80087AAC;
}
