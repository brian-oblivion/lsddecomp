/*
 * code_179d8_i -- functions 220..237 of the original code_179d8 monolith,
 * 0x23500..0x24938 (18 functions).  Carved round 21 (2026-09-06) as the
 * FRONT slice of the old code_179d8_tail; the remainder keeps both of that
 * segment's switch jump tables, so this unit owns no rodata.
 *
 * Blocker census at carve time (four screens, canonical shell forms):
 * 9 of 18 clean, 9 addiu-$at.  Every blocked function has a stub report in
 * docs/match-reports/ -- do not re-screen them, and do not spend attempts
 * on them; they are the operator's call, not a matching problem.
 *
 * WORKABLE (all four screens clean):
 *   func_80032D00 13w   func_800334A0 20w   func_800336CC 16w
 *   func_8003370C 11w   func_80033738 157w  func_800339AC 40w
 *   func_80033A4C 25w   func_80033FB8 26w   func_8003410C 11w
 *
 * Expect this slice to span more than one class -- a ~20-function window cut
 * at ROM-address boundaries has no reason to align with class boundaries.
 * Identify each with tools/classtable.py rather than assuming the unit has
 * one.  Keep every function in strict ROM-address order.
 */

#include "common.h"

extern s16 func_80032D34(void *a0, s16 a1, s32 a2, s32 a3);

s16 func_80032D00(void *a0, s16 a1, s32 a2)
{
    return func_80032D34(a0, a1, 1, a2);
}

INCLUDE_ASM("asm/nonmatchings/code_179d8_i", func_80032D34);

/* Field layout matches VabHdr in include/psyq/LIBSND.H, which declares
 * `extern short SsUtGetVabHdr(short, VabHdr*);` -- strong evidence this
 * game-unit function IS the SDK utility, not a coincidentally-named local
 * one. Not #include-d directly: LIBSND.H's own `#include <sys/types.h>`
 * does not resolve under this pinned include path on a case-sensitive
 * filesystem (only uppercase SYS/TYPES.H exists), confirmed with cpp
 * (exit 33, "sys/types.h: No such file or directory"). Local copy of the
 * same 0x20-byte layout instead, per this project's independent-local-view
 * convention. */
typedef struct {
    s32 form;
    s32 ver;
    s32 id;
    u32 fsize;
    u16 reserved0;
    u16 ps;
    u16 ts;
    u16 vs;
    u8 mvol;
    u8 pan;
    u8 attr1;
    u8 attr2;
    u32 reserved1;
} VabHdr;

extern u8 D_8008EA2C[];
extern VabHdr *D_8008E80C[];
extern VabHdr *D_8008E970;

short SsUtGetVabHdr(short vabId, VabHdr *hdr)
{
    VabHdr *vab;

    if (D_8008EA2C[vabId] != 1) {
        return -1;
    }
    vab = D_8008E80C[vabId];
    hdr->form = vab->form;
    hdr->id = vab->id;
    hdr->ver = vab->ver;
    hdr->ps = vab->ps;
    hdr->ts = vab->ts;
    D_8008E970 = vab;
    hdr->vs = vab->vs;
    hdr->mvol = vab->mvol;
    hdr->pan = D_8008E970->pan;
    hdr->attr1 = D_8008E970->attr1;
    hdr->attr2 = D_8008E970->attr2;
    return 0;
}

INCLUDE_ASM("asm/nonmatchings/code_179d8_i", func_80033260);

typedef struct {
    s32 unk0;
    s16 unk4;
    s16 unk6;
    u8 pad8[0x28 - 0x8];
} UnkStruct_800334A0;

extern void func_8003760C(UnkStruct_800334A0 *arg);

void func_800334A0(s16 a0, s16 a1)
{
    UnkStruct_800334A0 s;

    s.unk0 = 3;
    s.unk4 = a0 * 129;
    s.unk6 = a1 * 129;
    func_8003760C(&s);
}

INCLUDE_ASM("asm/nonmatchings/code_179d8_i", func_800334F0);

extern void func_80039158(s32 a0);
extern void func_8003918C(s32 a0);
extern void func_800391C8(s32 a0, s32 a1);
extern void func_80039228(s32 a0);
extern u8 D_8008EA2C[];
extern s32 D_80090BD4[];
extern s32 D_80090B90[];

s16 func_800335FC(s32 a0, s16 a1)
{
    s16 chan = a1;

    if ((u16)a1 < 0x11 && D_8008EA2C[chan] == 2) {
        s32 t = D_80090BD4[chan];
        func_80039158(0);
        func_8003918C(t);
        func_800391C8(a0, D_80090B90[chan]);
        D_8008EA2C[chan] = 1;
        return chan;
    }
    func_80039228(0);
    return -1;
}

extern void func_80036AC8(s32 a0);

void func_800336CC(u8 a0)
{
    s32 mode;

    if (a0 != 0) {
        if (a0 != 1) {
            return;
        }
        mode = 1;
    } else {
        mode = 0;
    }
    func_80036AC8(mode);
}

extern s32 func_8003904C(s16 a0);

s16 func_8003370C(s16 a0)
{
    return func_8003904C(a0);
}

/* A 172 (0xAC)-byte record; D_800902E8 is an array of pointers to arrays of
 * these, indexed [screen][slot]-style by two signed 16-bit indices. This is
 * a reduced LOCAL view -- only the fields this unit's functions touch are
 * named. See code_179d8_f.c's own Entry90902E8 for a fuller layout of the
 * same array; each unit keeps its own independent reading, per project
 * convention (multiple local views of one struct are expected here). */
typedef struct {
    u8 pad0[0x2B];
    u8 unk2B;
    u8 pad2C[0x90 - 0x2C];
    s32 unk90;
    u8 pad94[0xAC - 0x94];
} Entry90902E8;

extern Entry90902E8 *D_800902E8[];

extern s32 D_8008E934;
extern s16 D_80090B68;
extern s16 D_80090B6C;
extern s32 D_8008EA00;

extern void func_8002F700(void);
extern s32 func_8003410C(s16 a0, s16 a1);
extern void func_80036528(s32 a0, s32 a1);
extern void func_80033AB0(s32 a0, s32 a1);
extern void func_80033C90(s32 a0, s32 a1);
extern void func_800339AC(s32 a0, s32 a1);
extern void func_80033FB8(s32 a0, s32 a1);
extern void func_800368E8(s32 a0, s32 a1);

void func_80033738(void)
{
    s32 screen;
    s32 slot;
    s32 flags;
    s16 screen16;
    s16 slot16;

    if (D_8008E934 == 1) {
        return;
    }
    D_8008E934 = 1;
    func_8002F700();
    for (screen = 0; screen < D_80090B68; screen++) {
        if (!((1 << screen) & D_8008EA00)) {
            continue;
        }
        for (slot = 0; slot < D_80090B6C; slot++) {
            flags = D_800902E8[screen][slot].unk90;
            if (flags & 1) {
                screen16 = screen;
                slot16 = slot;
                func_8003410C(screen16, slot16);
                flags = D_800902E8[screen][slot].unk90;
                if (flags & 0x10) {
                    func_80036528(screen16, slot16);
                }
                flags = D_800902E8[screen][slot].unk90;
                if (flags & 0x20) {
                    func_80033C90(screen16, slot16);
                }
                flags = D_800902E8[screen][slot].unk90;
                if (flags & 0x40) {
                    func_80033AB0(screen16, slot16);
                }
                flags = D_800902E8[screen][slot].unk90;
                if (flags & 0x80) {
                    func_80033AB0(screen16, slot16);
                }
            }
            flags = D_800902E8[screen][slot].unk90;
            if (flags & 2) {
                func_800339AC((s16)screen, (s16)slot);
            }
            flags = D_800902E8[screen][slot].unk90;
            if (flags & 8) {
                func_80033FB8((s16)screen, (s16)slot);
            }
            flags = D_800902E8[screen][slot].unk90;
            if (flags & 4) {
                func_800368E8(screen, slot);
                D_800902E8[screen][slot].unk90 = 0;
            }
        }
    }
    D_8008E934 = 0;
}

extern s32 func_8003069C(s32 a0);

void func_800339AC(s32 a0, s32 a1)
{
    s16 sa0 = (s16)a0;
    s16 sa1 = (s16)a1;
    Entry90902E8 *p = &D_800902E8[sa0][sa1];

    func_8003069C((sa1 << 8) | sa0);
    p->unk2B = 0;
    D_800902E8[sa0][sa1].unk90 &= ~2;
}

void func_80033A4C(s32 a0, s32 a1)
{
    s16 sa0 = (s16)a0;
    s16 sa1 = (s16)a1;

    D_800902E8[sa0][sa1].unk2B = 0;
    D_800902E8[sa0][sa1].unk90 &= ~0x100;
}

INCLUDE_ASM("asm/nonmatchings/code_179d8_i", func_80033AB0);

INCLUDE_ASM("asm/nonmatchings/code_179d8_i", func_80033C90);

void func_80033FB8(s32 a0, s32 a1)
{
    s16 sa0 = (s16)a0;
    s16 sa1 = (s16)a1;

    D_800902E8[sa0][sa1].unk2B = 1;
    D_800902E8[sa0][sa1].unk90 &= ~8;
}

extern void func_80038CD8(s32 a0);
extern s32 D_80090BD4[];
extern s16 D_80090BD0;

void func_80034020(s16 a0)
{
    if ((u16)a0 < 0x10 && D_8008EA2C[a0] == 1) {
        func_80038CD8(D_80090BD4[a0]);
        D_8008EA2C[a0] = 0;
        D_80090BD0--;
    }
}

extern s16 func_80032C98(void *a0, s16 a1);
extern s16 func_800335FC(s32 a0, s16 a1);
extern s32 D_80090C1C[];

s16 func_800340B0(void *a0)
{
    s16 idx = func_80032C98(a0, -1);
    s16 result = idx;

    if (idx != -1) {
        result = func_800335FC(D_80090C1C[idx], idx);
    }
    return result;
}

extern s32 func_80034138(s16 a0, s16 a1);

s32 func_8003410C(s16 a0, s16 a1)
{
    return func_80034138(a0, a1);
}
