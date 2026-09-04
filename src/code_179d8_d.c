/*
 * code_179d8_d -- window [100..119] of the original 274-function code_179d8
 * monolith, 0x1C440..0x1CC08 (vram 0x8002BC40..0x8002C408).
 *
 * Carved MID-round 16, to re-staff a runner whose own unit was exhausted.
 * Screened 17/20 clean on the three-grep blocker census -- the highest of
 * any window in this monolith -- but that number overstates its value: 9 of
 * the 20 are 4-6 word leaves, and splat matched five of those itself as
 * empty `jr $ra; nop` bodies (func_8002C3C0/C3C8/C3F0/C3F8/C400 below).
 * Those five count as `matched` in tools/progress.py without having been
 * work, which is exactly the caveat CLAUDE.md attaches to that column.
 *
 * Two blocked functions have stub reports:
 *   func_8002BC40, func_8002BCEC  -- addiu_at
 * func_8002C278 was originally screened as a third (nop_mflo_mfhi) but
 * that screen was inverted (checked mult/div BEFORE mflo/mfhi instead of
 * after) -- the head corrected it mid-round and deleted the stub report.
 * It is fresh ground; the mult/mfhi pair in its body is retail's signed-
 * divide-by-constant idiom, not the blocked mflo/mfhi-then-mult direction.
 *
 * Unlike its siblings code_179d8_b and code_179d8_c, this slice owns NO
 * jump table -- all seven jtbl blocks in the 0xFD8 rodata slot fall outside
 * 0x8002BC40..0x8002C408 -- so no rodata sub-slot is attached to it.
 *
 * Sibling-slice finding worth having up front (runner charlie, this round,
 * code_179d8_b): this region is NOT class-framework code. tools/classtable.py
 * --scan has no hit anywhere near these globals, and the neighbouring
 * functions read as a low-level serial/link driver poking raw control words
 * into a block of globals that look like hardware/SIO register staging. Do
 * not expect vtables here; do not go looking for a `this` pointer.
 *
 * Declarations: keep anything that encodes THIS unit's reading of the region
 * next to the code, in this file. Do NOT create a shared code_179d8*.h --
 * the sibling slices are staffed independently and a shared header is what
 * makes their merges collide.
 */
#include "common.h"

/*
 * D_8006D940: a function-pointer table this unit's own `new_class_6d940`/
 * `func_8002C18C` dispatch through. Named/typed as a plain local struct,
 * NOT claimed to be a class-framework vtable -- per this unit's header
 * comment (sibling-slice finding: no classtable.py hit anywhere near this
 * region). Only the two slots this unit's own functions reach are typed;
 * the rest stays opaque padding. Kept LOCAL to this file, not a shared
 * header, per this round's rule for code_179d8 slices.
 */
typedef struct Table6D940 Table6D940;
struct Table6D940 {
    u8 pad000[0x008];
    /* +0x008, new_class_6d940's own dispatch -- this IS func_8002C18C
     * itself (same 2-arg (self, arg1) shape). */
    void (*slot08)(void *self, s32 arg1);
    u8 pad00C[0x06C - 0x00C];
    /* +0x06C, func_8002C18C's own conditional dispatch. */
    void (*slot6C)(void *self, s32 arg1);
};
extern Table6D940 D_8006D940;

/* The 0x34-byte object new_class_6d940 allocates. Only the fields
 * func_8002C18C itself touches are named. */
typedef struct Obj6D940 Obj6D940;
struct Obj6D940 {
    Table6D940 *methods; /* +0x000, func_8002C18C */
    u8 pad004[0x02C - 0x004];
    s32 unk2C;            /* +0x02C, func_8002C18C: zeroed */
    s32 unk30;             /* +0x030, func_8002C18C: zeroed */
};

/*
 * A second, DIFFERENT function-pointer table, reached only via the
 * uncarved accessor `func_80026CAC()` (not this unit's function to
 * define). Only the three slots this unit's functions dispatch through
 * are typed.
 */
typedef struct BaseTable6D940 BaseTable6D940;
struct BaseTable6D940 {
    u8 pad000[0x008];
    void (*slot08)(void *self); /* +0x008, func_8002C18C's own base-chain call */
    /* +0x00C, func_8002C200's own dispatch -- that function's whole body
     * is this one call with nothing after it, so its own return type is
     * genuinely ambiguous (a void wrapper around an s32 tail call is
     * byte-identical); typed s32 here per CLAUDE.md's rule to default to
     * `return callee(...)` absent positive void evidence. */
    s32 (*slot0C)(void *self);
    u8 pad010[0x064 - 0x010];
    /* +0x064, func_8002C238's own dispatch -- same tail-call ambiguity as
     * slot0C above. */
    s32 (*slot64)(void *self);
};
extern BaseTable6D940 *func_80026CAC(void);

/* Pool allocator, already established elsewhere (e.g.
 * include/class_16334.h, include/code_8220.h) -- declared LOCAL here since
 * this unit does not include either header. */
extern void *func_80017B34(s32 size);

INCLUDE_ASM("asm/nonmatchings/code_179d8_d", func_8002BC40);

INCLUDE_ASM("asm/nonmatchings/code_179d8_d", func_8002BCEC);

/* Cross-unit calls into the still-INCLUDE_ASM code_179d8_b (charlie's
 * unit, this round) -- declared LOCAL to this unit, per-call-site typed,
 * since none of them have an established prototype anywhere yet. */
extern void func_800292F4(void *arg0, s32 *outBuf);
extern void func_80028DF0(s32 arg0, s32 *buf, s32 arg2);
extern void func_80029274(void *arg0, void *arg1, s32 arg2);
extern s32 func_80029254(s32 arg0, s32 arg1);

s32 func_8002BFA8(void *p0, void *p1, void *p2)
{
    s32 buf[2];

    func_800292F4(p1, buf);
    func_80028DF0(2, buf, 0);
    func_80029274(p0, p2, 0x80);
    return (u32)func_80029254(0, 0) < 1;
}

void func_8002C014(char *dest, char *src, s32 count)
{
    s32 i;

    for (i = count - 1; i != -1; i--) {
        *dest++ = *src++;
    }
}

INCLUDE_ASM("asm/nonmatchings/code_179d8_d", func_8002C048);

INCLUDE_ASM("asm/nonmatchings/code_179d8_d", func_8002C0AC);

void *new_class_6d940(s32 arg1)
{
    void *self;
    Table6D940 *table;

    self = func_80017B34(0x34);
    if (self != NULL) {
        table = func_8002C3A8();
        table->slot08(self, arg1);
        return self;
    }
    return NULL;
}

void func_8002C18C(Obj6D940 *self, s32 arg1)
{
    func_80026CAC()->slot08(self);
    self->methods = func_8002C3A8();
    self->unk2C = 0;
    self->unk30 = 0;
    if (arg1 != 0) {
        self->methods->slot6C(self, arg1);
    }
}

s32 func_8002C200(void *self)
{
    return func_80026CAC()->slot0C(self);
}

s32 func_8002C238(s32 *self)
{
    self[0xC] = 1;
    return func_80026CAC()->slot64(self);
}

/*
 * func_8002C278's own "descriptor" pointer, resolved either from a cached
 * byte offset (Obj278::unk34) or freshly from `index*12+8` into
 * Ctx278::unk10's byte array. Field meaning unestablished beyond
 * offset/width -- this region reads as raw hardware/SIO register staging
 * (no classtable.py hit anywhere nearby), not class-framework data.
 */
typedef struct Entry278 {
    u8 unk0;   /* +0x0, tag/kind: zero means "not present", tested first */
    u8 unk1;   /* +0x1 */
    u16 unk2;  /* +0x2 */
    u8 unk4;   /* +0x4 */
    u8 unk5;   /* +0x5 */
    s16 unk6;  /* +0x6 */
    s32 unk8;  /* +0x8 */
} Entry278;

/* func_8002C278's own object (its own `arg1`). Only the fields this
 * function itself touches are named. */
typedef struct Obj278 {
    u8 pad0[0xC];
    s32 unkC;   /* +0xC */
    s32 unk10;  /* +0x10 */
    s32 unk14;  /* +0x14 */
    u8 pad18[0x1A - 0x18];
    s16 unk1A;  /* +0x1A */
    u8 pad1C[0x2C - 0x1C];
    s16 unk2C;  /* +0x2C */
    s16 unk2E;  /* +0x2E */
    s32 unk30;  /* +0x30, cache-hit flag: 1 if unk34 was reused, 0 if freshly computed */
    s32 unk34;  /* +0x34, BEFORE resolution: a cached byte offset into Ctx278::unk10; AFTER: Entry278::unk8 */
    s32 unk38;  /* +0x38 */
} Obj278;

/* Opaque target of Ctx278::unk2C -- only the one dispatched slot named. */
typedef struct Ctx278SubMethods Ctx278SubMethods;
typedef struct Ctx278Sub Ctx278Sub;
struct Ctx278SubMethods {
    u8 pad000[0x080];
    s32 (*slot80)(Ctx278Sub *self);
};
struct Ctx278Sub {
    Ctx278SubMethods *methods;
};

/* func_8002C278's own `arg0`. Only the fields this function itself
 * touches are named. */
typedef struct Ctx278 {
    u8 pad0[0x10];
    u8 *unk10;       /* +0x10, byte-addressed base for the Entry278 table */
    u8 pad14[0x2C - 0x14];
    Ctx278Sub *unk2C; /* +0x2C */
} Ctx278;

INCLUDE_ASM("asm/nonmatchings/code_179d8_d", func_8002C278);

Table6D940 *func_8002C3A8(void)
{
    return &D_8006D940;
}

s32 func_8002C3B8(void)
{
    return 0;
}

void func_8002C3C0(void) {
}

void func_8002C3C8(void) {
}

void func_8002C3D0(void)
{
    char buf[0x40];
}

void func_8002C3E0(void)
{
    char buf[0x40];
}

void func_8002C3F0(void) {
}

void func_8002C3F8(void) {
}

void func_8002C400(void) {
}
