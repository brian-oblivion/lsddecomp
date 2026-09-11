/*
 * code_179d8_i -- functions 220..237 of the original code_179d8 monolith,
 * 0x23500..0x24938 (18 functions).  Carved round 21 (2026-09-06) as the
 * FRONT slice of the old code_179d8_tail; the remainder keeps both of that
 * segment's switch jump tables, so this unit owns no rodata.
 *
 * Blocker census at carve time (four screens, canonical shell forms):
 * 9 of 18 clean, 9 addiu-$at.  That census said of the nine "do not spend
 * attempts on them; they are the operator's call, not a matching problem."
 *
 * THAT IS STALE AND ITS GROUND HAS ALREADY BEEN RECOVERED -- re-screened
 * round 23 (2026-09-07).  `addiu_at` was resolved in round 21 (maspsx
 * `--addiu-at`; docs/research/addiu-at-blocker.md) and all nine were matched
 * in the rounds that followed.  The unit's ONLY remaining queue is three
 * blocker-clean, unreported functions -- func_80032D34 (274w),
 * func_80033260 (144w), func_80033C90 (202w) -- which are ordinary large
 * fresh ground, not blocked.  Verified with the two live screens (`gp_rel`
 * and `nop_mflo_mfhi`): zero hits on any of the three.
 *
 * The lesson, kept because the comment cost nothing here only by luck:
 * a carve-time census recorded as a DIRECTIVE ("do not spend attempts")
 * outlives the blocker it was measured against, and no tool can see it.
 * Screen with `python3 tools/nearmiss.py` rather than trusting any
 * transcribed census, this one included.
 *
 * Expect this slice to span more than one class -- a ~20-function window cut
 * at ROM-address boundaries has no reason to align with class boundaries.
 * Identify each with tools/classtable.py rather than assuming the unit has
 * one.  Keep every function in strict ROM-address order.
 */

#include "common.h"

extern s16 func_80032D34(void *a0, s16 a1, s16 a2, s32 a3);

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

/* Field layout matches VagAtr in include/psyq/LIBSND.H, which declares
 * `extern short SsUtGetVagAtr (short, short, short, VagAtr*);` -- same
 * evidence pattern as SsUtGetVabHdr just above: this game-unit function
 * copies exactly VagAtr's non-reserved fields (skips reserved1/reserved2
 * at 0xE/0xF and reserved[4] at 0x18-0x1F) from a global VagAtr table.
 * Not #include-d directly for the same reason as VabHdr above (LIBSND.H's
 * own `#include <sys/types.h>` does not resolve here); local copy of the
 * same 0x20-byte layout instead. The third parameter is NOT sign-extended
 * on entry (unlike the first two) -- it is only ever used inside an
 * expression that gets truncated to 16 bits by the later index scale, so
 * the retail source evidently typed it wider than `short` despite the SDK
 * prototype, and the compiler had no need to narrow it early.
 *
 * Kept as func_80033260, NOT renamed to SsUtGetVagAtr like SsUtGetVabHdr
 * above it: unlike that function, this one has no `= 0x8003....;` alias in
 * config/symbols.slps01556.lsdde.txt, so five still-INCLUDE_ASM'd callers
 * in code_179d8_k.c and code_179d8_e.c carry a literal `jal func_80033260`
 * in their own retail .s text. Renaming the C definition breaks the link
 * for every one of them; config/ is not this unit's to edit. */
typedef struct {
    u8 prior;
    u8 mode;
    u8 vol;
    u8 pan;
    u8 center;
    u8 shift;
    u8 min;
    u8 max;
    u8 vibW;
    u8 vibT;
    u8 porW;
    u8 porT;
    u8 pbmin;
    u8 pbmax;
    u8 reserved1;
    u8 reserved2;
    u16 adsr1;
    u16 adsr2;
    s16 prog;
    s16 vag;
    s16 reserved[4];
} VagAtr;

extern VagAtr *D_8008E978;
extern u8 D_8008EA13;
extern s32 func_80032148(s16 a0, s16 a1);

short func_80033260(short vabId, short prog, s32 tone, VagAtr *vagatr)
{
    s16 idx;

    if (D_8008EA2C[vabId] == 1) {
        func_80032148(vabId, prog);
        idx = tone + (D_8008EA13 << 4);
        vagatr->prior = D_8008E978[idx].prior;
        vagatr->mode = D_8008E978[idx].mode;
        vagatr->vol = D_8008E978[idx].vol;
        vagatr->pan = D_8008E978[idx].pan;
        vagatr->center = D_8008E978[idx].center;
        vagatr->shift = D_8008E978[idx].shift;
        vagatr->max = D_8008E978[idx].max;
        vagatr->min = D_8008E978[idx].min;
        vagatr->vibW = D_8008E978[idx].vibW;
        vagatr->vibT = D_8008E978[idx].vibT;
        vagatr->porW = D_8008E978[idx].porW;
        vagatr->porT = D_8008E978[idx].porT;
        vagatr->pbmin = D_8008E978[idx].pbmin;
        vagatr->pbmax = D_8008E978[idx].pbmax;
        vagatr->adsr1 = D_8008E978[idx].adsr1;
        vagatr->adsr2 = D_8008E978[idx].adsr2;
        vagatr->prog = D_8008E978[idx].prog;
        vagatr->vag = D_8008E978[idx].vag;
        return 0;
    }
    return -1;
}

typedef struct {
    s32 unk0;
    s16 unk4;
    s16 unk6;
    u8 pad8[0x28 - 0x8];
} UnkStruct_800334A0;

extern void SpuSetCommonAttr(UnkStruct_800334A0 *arg);

void func_800334A0(s16 a0, s16 a1)
{
    UnkStruct_800334A0 s;

    s.unk0 = 3;
    s.unk4 = a0 * 129;
    s.unk6 = a1 * 129;
    SpuSetCommonAttr(&s);
}

/* 0x10-byte-strided table shared with code_179d8_j.c's own SlotE968 local
 * view of the same D_8008E968 global (a pointer variable, not an array
 * symbol -- confirmed by the `lw` of its VALUE here, matching that file's
 * own `extern SlotE968 *D_8008E968;`). This unit's own reduced view names
 * only the fields this function touches. */
typedef struct {
    u8 unk0;
    u8 unk1;
    u8 unk2;
    u8 unk3;
    u8 unk4;
    u8 pad5[0x6 - 0x5];
    u16 unk6;
    u8 pad8[0x10 - 0x8];
} Entry8E968;

extern Entry8E968 *D_8008E968;

s16 func_800334F0(s16 a0, s16 a1, Entry8E968 *out)
{
    s16 idx;

    if (D_8008EA2C[a0] == 1) {
        idx = a1;
        func_80032148(a0, idx);
        out->unk0 = D_8008E968[idx].unk0;
        out->unk1 = D_8008E968[idx].unk1;
        out->unk2 = D_8008E968[idx].unk2;
        out->unk3 = D_8008E968[idx].unk3;
        out->unk4 = D_8008E968[idx].unk4;
        out->unk6 = D_8008E968[idx].unk6;
        return 0;
    }
    return -1;
}

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

extern void SpuSetMute(s32 a0);

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
    SpuSetMute(mode);
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
    u8 pad2C[0x3E - 0x2C];
    s16 unk3E;
    u16 unk40;
    s16 unk42;
    s16 unk44;
    u8 pad46[0x4A - 0x46];
    s16 unk4A;
    u8 pad4C[0x70 - 0x4C];
    s16 unk70;
    u8 pad72[0x78 - 0x72];
    s16 unk78;
    s16 unk7A;
    u8 pad7C[0x8C - 0x7C];
    u32 unk8C;
    s32 unk90;
    u8 pad94[0x98 - 0x94];
    s32 unk98;
    u8 pad9C[0xA0 - 0x9C];
    s32 unkA0;
    u32 unkA4;
    u8 padA8[0xAC - 0xA8];
} Entry90902E8;

extern Entry90902E8 *D_800902E8[];

extern s32 D_8008E934;
extern s16 D_80090B68;
extern s16 D_80090B6C;
extern s32 D_8008EA00;

extern void func_8002F700(void);
extern s32 func_8003410C(s16 a0, s16 a1);
extern void func_80036528(s32 a0, s32 a1);
extern void func_80033AB0(s16 a0, s16 a1);
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
    D_800902E8[(s16)a0][(s16)a1].unk90 &= ~2;
}

void func_80033A4C(s32 a0, s32 a1)
{
    s16 sa0 = (s16)a0;
    s16 sa1 = (s16)a1;

    D_800902E8[sa0][sa1].unk2B = 0;
    D_800902E8[(s16)a0][(s16)a1].unk90 &= ~0x100;
}

extern u32 D_8009024C;

void func_80033AB0(s16 a0, s16 a1)
{
    Entry90902E8 *e = &D_800902E8[a0][a1];

    e->unkA0 -= 1;
    if (e->unk44 > 0) {
        if ((u32)e->unkA0 % (u32)e->unk44 != 0) {
            return;
        }
        {
            u32 new8C;

            if (e->unk8C > e->unkA4) {
                new8C = e->unk8C - 1;
            } else if (e->unk8C < e->unkA4) {
                new8C = e->unk8C + 1;
            } else {
                goto skip1;
            }
            e->unk8C = new8C;
        skip1:;
        }
    } else {
        if (e->unk8C > e->unkA4) {
            e->unk8C += e->unk44;
            if (e->unk8C < e->unkA4) {
                e->unk8C = e->unkA4;
            }
        } else if (e->unk8C < e->unkA4) {
            e->unk8C -= e->unk44;
            if (e->unkA4 < e->unk8C) {
                e->unk8C = e->unkA4;
            }
        }
    }

    e->unk70 = e->unk4A * (s32)e->unk8C * 10 / (D_8009024C * 60);
    if (e->unk70 <= 0) {
        e->unk70 = 1;
    }

    if (e->unkA0 == 0 || e->unk8C == e->unkA4) {
        D_800902E8[a0][a1].unk90 &= ~0x40;
        D_800902E8[a0][a1].unk90 &= ~0x80;
    }
}

/* unk3E, unk40, unk42, unk78, unk7A, unk98 added to Entry90902E8 above,
 * in place of existing padding -- no existing field's offset changed.
 * unk40 is loaded with `lhu` (declared u16) but sign-checked via an
 * explicit `(s16)` cast at every comparison site -- matches retail's
 * `sll 16`/`bltz` idiom for checking a 16-bit value's sign without a
 * plain `lh`.
 *
 * STALL -- see docs/match-reports/func_80033C90.md for the full
 * algorithm derivation (correct, byte-verified block-by-block against
 * the asm) and the best C body reached (18/202 words, first diff at
 * word 1 -- the prologue's own `-0x40` vs `-0x38` frame size). The
 * residue is register/stack allocation, not logic. */
extern s32 func_80030404(s16 a0, u16 a1, u16 a2, s32 a3);
extern s32 func_80030584(s32 p0, s16 *out1, s16 *out2);

#if 0
void func_80033C90(s32 a0, s32 a1)
{
    Entry90902E8 *p = &D_800902E8[(s16)a0][(s16)a1];
    s16 c42 = p->unk42;
    s32 newCnt = p->unk98 - 1;
    s16 pk;
    u16 sp10, sp12;
    u16 a1arg;
    s32 a2arg;

    p->unk98 = newCnt;
    if (c42 > 0) {
        if ((u32)newCnt % (u32)c42 == 0) {
            if (p->unk3E > 0) {
                p->unk40 -= 1;
                if ((s16)p->unk40 < 0) {
                    goto negHandler;
                }
                pk = (s16)(a0 | (a1 << 8));
                func_80030584(pk, (s16 *)&sp10, (s16 *)&sp12);
                if (sp10 == 0) {
                    goto viaE60;
                }
                if (sp12 == 0) {
                    goto viaE5C;
                }
                a1arg = sp10 + 0xFFFF;
                a2arg = sp12 + 0xFFFF;
                goto callReal;
            }
            goto tailCheck;
        }
        goto tailFinal;
    } else {
        if (p->unk3E > 0) {
            p->unk40 += c42;
            if ((s16)p->unk40 < 0) {
                goto negHandler;
            }
            pk = (s16)(a0 | (a1 << 8));
            func_80030584(pk, (s16 *)&sp10, (s16 *)&sp12);
            if ((s32)sp10 < -(s32)p->unk42) {
                goto viaE60;
            }
            if ((s32)sp12 < -(s32)p->unk42) {
                goto viaE64;
            }
            a1arg = sp10 + p->unk42;
            a2arg = sp12 + p->unk42;
            goto callReal;
        }
        goto tailCheck;
    }

negHandler:
    func_80030404((s16)(a0 | (a1 << 8)), 0, 0, 0);
    D_800902E8[(s16)a0][(s16)a1].unk90 &= ~0x20;
    goto tailCheck;

callReal:
    func_80030404(pk, a1arg, (u16)a2arg, 0);
    goto tailFinal;

viaE5C:
viaE60:
viaE64:
    func_80030404((s16)(a0 | (a1 << 8)), 0, 0, 0);
    D_800902E8[(s16)a0][(s16)a1].unk90 &= ~0x20;

tailCheck:
    if (p->unk98 == 0 || (s16)p->unk40 == 0) {
        D_800902E8[(s16)a0][(s16)a1].unk90 &= ~0x20;
    }

tailFinal:
    func_80030584((s16)(a0 | (a1 << 8)), &p->unk78, &p->unk7A);
}
#endif

INCLUDE_ASM("asm/nonmatchings/code_179d8_i", func_80033C90);

void func_80033FB8(s32 a0, s32 a1)
{
    s16 sa0 = (s16)a0;
    s16 sa1 = (s16)a1;

    D_800902E8[sa0][sa1].unk2B = 1;
    D_800902E8[(s16)a0][(s16)a1].unk90 &= ~8;
}

extern void SpuFree(s32 a0);
extern s32 D_80090BD4[];
extern s16 D_80090BD0;

void func_80034020(s16 a0)
{
    if ((u16)a0 < 0x10 && D_8008EA2C[a0] == 1) {
        SpuFree(D_80090BD4[a0]);
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
