#include "common.h"
#include "class_3bb8c.h"

void func_8004D704(Class86B60 *self)
{
    if (self->unkAC != NULL) {
        self->unkAC->methods->release(self->unkAC);
        self->unkA8->methods->release(self->unkA8);
    }
    Get_vtable_TaskCore()->slot0C(self);
}

void func_8004D788(Class86B60 *self, GenericHeaderObj_3bb8c_d *arg1, s32 arg2)
{
    Get_vtable_TaskCore()->slot38(self, arg1, arg2);
    if ((arg1->methods->header & 0xF) == 0xB) {
        self->methods->slot138(self, arg1, arg2);
    }
}

void func_8004D814(Class86B60 *self)
{
    self->unk34 = 0;
    self->unk2C = 0x190;
    self->methods->slotD4(self, &D_800114E8, 0);
    self->methods->slot6C(self, 0xA);
    self->unkA4->methods->slotF0(self->unkA4, 0, 0);
}

void func_8004D898(Class86B60 *self)
{
    u32 i;
    u8 *entry;

    i = 0;
    entry = (u8 *)&D_80086DAC;
    for (; i < 2; i++) {
        self->unkC->unk0->methods->slot78(self->unkC->unk0, &self->unk93, entry);
        entry += 0xC;
    }
}

void func_8004D90C(Class86B60 *self, s32 arg1)
{
    Get_vtable_TaskCore()->slot60(self, arg1);
    if (arg1 == 5) {
        self->methods->slot124(self, 0);
    }
    if (arg1 == 0xA) {
        self->methods->slot7C(self);
        self->methods->slotF0(self, self->unk4C->unk8, 1);
        self->methods->slot78(self);
    }
}

void func_8004D9D4(Class86B60 *self)
{
    void (*fn)(Class86B60 *);

    Get_vtable_TaskCore()->slot90(self);
    switch (self->unk58) {
    case 1:
        self->unk38 = 0;
        self->unkA4->methods->slotF0(self->unkA4, 0, 1);
        fn = self->methods->slot94;
        break;
    case 2:
        fn = self->methods->slot130;
        break;
    case 3:
        fn = self->methods->slot134;
        break;
    case 4:
        self->unk38 = 2;
        fn = self->methods->slot94;
        break;
    default:
        return;
    }
    fn(self);
}

void func_8004DABC(Class86B60 *self)
{
    s32 buf;

    Get_vtable_TaskCore()->slot94(self);
    buf = self->unk60->unk14;
    self->unkA4->methods->slot19C(self->unkA4, &buf);
}

/* func_8004DB18's own `arg1`: only its own +0x004 field is read, forwarded
 * opaquely as New_Obj6EAC0's `ctx` argument. */
typedef struct Arg1DB18_3bb8c_d Arg1DB18_3bb8c_d;
struct Arg1DB18_3bb8c_d {
    u8 pad0[0x004];
    void *unk4; /* +0x004 */
};

/* Sony's, linked from libc2 (config/psyq-objects.txt: libc2/strcpy,
 * libc2/strlen); declared locally per this project's established
 * per-unit convention for these two (see e.g. src/class_3bb8c_i.c,
 * src/class_3bb8c_j.c). */
extern char *strcpy(char *dest, char *src);
extern s32 strlen(char *s);

/* This unit's own view of DecodeFullWidthSjis (already matched,
 * src/code_2cc8c_f.c) -- return value unused at this call site, unlike
 * that unit's own `u8 *` view, so kept minimal per the project's
 * independent-arities convention. */
extern void DecodeFullWidthSjis(void *dst, void *src);

void func_8004DB18(Class86B60 *self, Arg1DB18_3bb8c_d *arg1)
{
    u32 size;
    char *buf;

    if (arg1 == NULL) {
        return;
    }
    if (self->unkA4->methods->slot1AC(self->unkA4)) {
        strcpy((char *)D_8008AA18 + 0x18, (char *)D_8008AA14);
        func_800507F8((s32)D_8008AA18, 0);
    }
    size = strlen((char *)D_8008AA18);
    size = (size >> 1) + 4;
    buf = func_80017B34(size);
    DecodeFullWidthSjis(buf, D_8008AA18);
    self->unkB0 = (Class86B60UnkB0Obj_3bb8c_d *)New_Obj6EAC0(arg1->unk4, size, buf);
    self->unkB0->unkAB = 8;
    self->unkB0->unkAC = 4;
    self->unkB0->unkAA = 9;
    func_80017CFC(buf);
}

void func_8004DC08(Class86B60 *self)
{
    self->unkB0->methods->release(self->unkB0);
    Get_vtable_TaskCore()->slotDC(self);
}

void func_8004DC64(Class86B60 *self, s32 arg1)
{
    Get_vtable_TaskCore()->slotE0(self, arg1);
    self->unkB0->methods->slot4C(self->unkB0, arg1, &D_8008A9B4);
}

/* func_8004DCD0's own `arg1`: only the first three signed bytes are read.
 * Kept a minimal, distinct local type rather than reusing this header's
 * broader `Descriptor10` (same 3-byte shape, but an unrelated context --
 * nothing here shows a 4th byte or the two trailing halfwords). */
typedef struct Arg1DCD0_3bb8c_d Arg1DCD0_3bb8c_d;
struct Arg1DCD0_3bb8c_d {
    s8 b0;
    s8 b1;
    s8 b2;
};

#if 0
/* STALL, round 43 -- see docs/match-reports/func_8004DCD0.md.
 * 2 words short (76/78), register-class residue on the shared local
 * pointer ("base"): retail keeps it in a THIRD callee-saved register
 * ($s1, alongside self=$s2/arg1=$s0) across the whole function; every
 * structural variant tried here (with or without an explicit `goto` to
 * the shared store, with or without a named "base"/"p" pointer, folding
 * the offset into "base" itself) gets GCC 2.6.3 -O2 to allocate the same
 * value into a caller-saved temp ($a0/$a1) instead, 8 bytes short.
 * Preserved verbatim below -- it is the closest candidate found (only
 * this one register-class difference plus the two words it costs), not
 * a working match. */
void func_8004DCD0(Class86B60 *self, Arg1DCD0_3bb8c_d *arg1)
{
    u8 buf[3];
    u8 *base;
    u8 v;

    Get_vtable_TaskCore()->slotE4(self, arg1);
    base = buf;
    if (self->unk3C != 0) {
        base[0] = 0;
        base[1] = 0;
        base[2] = 0;
        base += D_8008AA28;
        v = 0x80;
        goto store;
    }
    base[0] = arg1->b0;
    base[1] = arg1->b1;
    base[2] = arg1->b2;
    if (D_8008AA2C < 0x80) {
        base[0] = base[0] + 0x80;
        goto skip;
    }
    base += D_8008AA28;
    v = *base + 0x80;
store:
    *base = v;
skip:
    D_8008AA28++;
    if (D_8008AA28 >= 3) {
        D_8008AA28 = 0;
    }
    D_8008AA2C++;
    if (D_8008AA2C >= 0x101) {
        D_8008AA2C = 0;
    }
    self->unkB0->methods->slotB8(self->unkB0, buf);
}
#endif

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_d", func_8004DCD0);

/* func_8004D678 is ALREADY MATCHED (src/class_3bb8c_c.c), as a genuinely
 * 2-argument function -- but THIS call site sets up a 3rd argument
 * (self->unkA4, in $a2) that the other unit's own 2-parameter view never
 * receives. Same independent-arities situation already documented for
 * Get_vtable_TaskCore/BaseTaskCtorTable_3bb8c_c: this unit's own local view
 * matches what THIS call site needs. */
extern void func_8004D678(void *arg0, void *arg1, void *arg2); /* arity-ok: the definition is 2-parameter and the callee WRITES $a2 (`li a2,0x1` at 0x8004D690) before reading it, but the 3rd argument is byte-load-bearing here -- retail emits `lw a2,164(s0)` at 0x8004DE74 */

void func_8004DE08(Class86B60 *self)
{
    s32 size;
    s32 origUnk58;
    void *buf1;
    s32 buf2;

    size = self->unkB0->unkA9;
    origUnk58 = self->unk58;
    buf1 = func_80017B34(size);
    DecodeFullWidthSjis(buf1, D_8008AA18);
    self->unkB0->methods->slotCC(self->unkB0, buf1);
    func_80017CFC(buf1);
    func_8004D678(self, self->unk4C, self->unkA4);
    self->methods->slotE0(self, self->unk14);
    self->unkA4->methods->slot19C(self->unkA4, &buf2);
    self->unk58 = 5;
    self->methods->slot60(self, 0xB);
    self->methods->slot11C(self, buf2, 1);
    self->methods->slot60(self, 0xF);
    self->methods->slotF0(self, (void *)origUnk58, 0);
    self->unkA4->methods->slot19C(self->unkA4, &buf2);
}

/* This unit's own local view of func_8003B39C (already matched elsewhere,
 * many independent-arity views project-wide -- see e.g.
 * src/class_3bb8c_g.c, src/class_3bb8c_i.c). Return type matches what
 * this call site actually stores it into (`self->unkA8`). */
extern GenericReleaseObj_3bb8c_d *func_8003B39C(const char *path);

void func_8004DF64(Class86B60 *self)
{
    if (self->unkAC == NULL) {
        self->unkA8 = func_8003B39C(D_800114F8);
        self->unkAC = func_8004E2E0((void *)1, NULL);
    }
    self->unkAC->methods->slot6C(self->unkAC, D_8008A9D0, &D_80086D6C,
                                  self->unkC->unk4, self->unk10, self->unk14,
                                  self->unk48);
    self->methods->slot10(self, self->unkAC);
    self->methods->slot14(self, self->unkC->unk4);
    self->methods->slot14(self, self->unk10);
}

void func_8004E054(Class86B60 *self)
{
    self->methods->slot10(self, self->unkC->unk4);
    self->methods->slot10(self, self->unk10);
    self->methods->slot14(self, self->unkAC);
    self->unkAC->methods->slot70(self->unkAC);
}

void func_8004E0E4(Class86B60 *self)
{
    s32 buf;

    buf = self->unk60->unk14;
    self->unkA4->methods->slot19C(self->unkA4, &buf);
    self->methods->slot128(self);
    if (self->unkA4->methods->slot1AC(self->unkA4)) {
        *(u8 *)D_8008AA10 = 0;
    }
    self->unkAC->methods->slot78(self->unkAC, D_8008AA10, D_8008AA18, 0xD, 3,
                                  self->unkA8, self->unkBC, self->unkC0);
}

void func_8004E1C4(Class86B60 *self)
{
    self->methods->slot128(self);
    self->unkAC->methods->slot74(self->unkAC, D_8008AA10, D_8008AA18,
                                  self->unkBC, self->unkC0);
}

void func_8004E230(Class86B60 *self, s32 arg1, s32 value)
{
    if (value < 0x18) {
        if (value >= 0x16) {
            self->methods->slot12C(self);
            if (value == 0x16) {
                self->unkA4->methods->slot1A8(self->unkA4);
                self->methods->slot124(self, 0x16);
            }
        }
    }
}

Class86B60Methods *func_8004E2D0(void)
{
    return &D_80086B60;
}

void *func_8004E2E0(void *arg0, void *arg1)
{
    void *self;

    self = func_80017B34(0x84);
    if (self == NULL) {
        goto fail;
    }
    func_800507E8()->ctor(self, arg0, arg1);
    return self;
fail:
    return NULL;
}

/* func_8004E3F4 is ALREADY MATCHED, but in class_3bb8c_e.c under its own
 * local type (`Node3bb8cE *`) -- this unit's own independent local view
 * of the same real object per the project's convention; only `self` is
 * ever forwarded here, never dereferenced. */
extern void func_8004E3F4(void *self);

/* libcard, linked SDK objects (config/psyq-objects.txt: libcard/a74,
 * libcard/a75, libcard/c112 -- see docs/match-reports/func_8004E34C.md).
 * Declared locally rather than in the shared header, same policy as
 * malloc/free/printf (CLAUDE.md, "To include/ has one exception"). */
extern void InitCARD(s32 padEnable);
extern void StartCARD(void);
extern void _bu_init(void);

void func_8004E34C(GenericCtorObj_3bb8c_d *self, s32 arg1, s32 arg2)
{
    s32 count;

    Get_vtable_BasicClass()->ctor(self);
    self->methods = func_800507E8();
    count = D_8008AA30;
    D_8008AA30 = count + 1;
    if (count == 0) {
        InitCARD(arg1);
        StartCARD();
        _bu_init();
    }
    func_8004E3F4(self);
    self->methods->slot40(self, arg2);
}
