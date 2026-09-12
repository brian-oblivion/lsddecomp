/*
 * code_179d8_f -- what is LEFT of functions 256..273 of the original
 * 274-function code_179d8 monolith after round 34 gave its first thirteen
 * functions back to Sony.  Now 0x26D28..0x272A8 (vram 0x80036528..0x800368E8).
 *
 * ROUND 34 (2026-09-12): the unit's whole PREFIX, 0x2673C..0x26D28, is six
 * linked `libsnd` objects -- `adsr` (3.3), `ut_rev` (3.3), `ut_sva` (3.3),
 * `vm_don` (3.3), `next` (3.5), `vm_doff` (3.3) -- covering thirteen
 * functions:
 *   0x80035F3C _SsUtResolveADSR       (was matched C)
 *   0x80035F98 _SsUtBuildADSR         (was a 35w INCLUDE_ASM stall)
 *   0x80036024 SsUtReverbOn           (was matched C)
 *   0x80036044 SsUtReverbOff          (was matched C)
 *   0x80036064 SsUtSetReverbType      (was matched C)
 *   0x80036108 SsUtGetReverbType      (was matched C)
 *   0x80036118 SsUtSetReverbDepth     (was matched C)
 *   0x800361B0 SsUtSetReverbFeedback  (was matched C)
 *   0x800361F0 SsUtSetReverbDelay     (was matched C)
 *   0x80036230 SsUtSetVagAtr          (was matched C, 115w)
 *   0x800363FC SpuVmDamperOn          (was matched C)
 *   0x80036410 _SsSndNextSep          (was matched C)
 *   0x80036518 SpuVmDamperOff         (was matched C)
 * Their C is DELETED, not commented out.  Twelve of them were matched as game
 * code and were Sony's the whole time; reclassifying them is the correction
 * CLAUDE.md asks for, not a regression.  Do not write C for any of them again
 * -- `python3 tools/sdkstalls.py` and
 * `.venv/bin/python3 tools/psyq_sdk.py coverage` are the evidence.
 *
 * That was a pure PREFIX trim, so this unit kept its name and its `c` line
 * simply moved to 0x26D28.  There is a SECOND run inside what is left
 * (`libsnd/stop` at 0x270E8), which splits this unit again -- see the yaml.
 *
 * Owns NO switch jump table (zero `jtbl_` in its disassembly, and the splat
 * yaml's rodata slot list names no `.rodata, code_179d8_f` line), so no
 * rodata attach, before or after the split.
 *
 * BLOCKER PROFILE: screen with `python3 tools/nearmiss.py`, never by
 * re-implementing the greps and never for `addiu_at` (resolved round 21).
 * func_80036528 (240w) is the one INCLUDE_ASM left here; it has been attempted
 * and carries a full worked report (docs/match-reports/func_80036528.md) with
 * its 227/240 near-miss body preserved below.  It is not cold ground.
 *
 * Expect low-level driver-shaped code rather than class-framework code, as
 * elsewhere in code_179d8; confirm with tools/classtable.py, do not assume.
 */
#include "common.h"

/* A 172 (0xAC)-byte record; only the fields functions in this unit touch
 * are named. D_800902E8 is an array of pointers to arrays of these, indexed
 * [screen/player][slot]-style by two signed 16-bit indices. */
typedef struct {
    u8 pad0[0x4];
    s32 unk4;
    s32 unk8;
    s32 unkC;
    u8 unk10;
    u8 unk11;
    u8 unk12;
    u8 unk13;
    u8 unk14;
    u8 unk15;
    u8 unk16;
    u8 unk17[0x10];
    u8 unk27;
    u8 unk28;
    u8 unk29;
    u8 unk2A;
    u8 unk2B;
    u8 unk2C[0x10];
    u8 pad3C[0x3E - 0x3C];
    s16 unk3E;
    u16 unk40;
    s16 unk42;
    u8 pad44[0x46 - 0x44];
    s16 unk46;
    s16 unk48;
    u8 pad4A[0x4E - 0x4A];
    s16 unk4E[0x10];
    u8 pad6E[0x70 - 0x6E];
    s16 unk70;
    s16 unk72;
    u8 pad74[0x78 - 0x74];
    s16 unk78;
    s16 unk7A;
    s32 unk7C;
    s32 unk80;
    s32 unk84;
    s32 unk88;
    s32 unk8C;
    s32 unk90;
    u8 pad94[0x98 - 0x94];
    s32 unk98;
    u8 pad9C[0xAC - 0x9C];
} Entry90902E8;

extern Entry90902E8 *D_800902E8[];

/* STALL -- see docs/match-reports/func_80036528.md. Best reached: 227/240
 * words compiled (13 words SHORT of retail's length; whole-image red).
 * Frame size, control flow and field layout are all confirmed correct;
 * the residue is a parameter-to-callee-saved-register allocation choice
 * (a0/a1 hop through the plain argument registers for longer than this
 * body reproduces) this round's attempts could not close.
 * Restored to INCLUDE_ASM per project rule. */
#if 0
extern s32 func_80030404(s16 a0, s16 a1, s16 a2, s32 a3);
extern s32 func_80030584(s32 a0, s16 *out1, s16 *out2);

void func_80036528(s32 a0, s32 a1)
{
    Entry90902E8 **arr;
    Entry90902E8 *entry;
    s16 count;
    s16 thresh;
    s16 sp10;
    s16 sp12;

    arr = &D_800902E8[(s16)a0];
    entry = &(*arr)[(s16)a1];
    count = entry->unk42;
    entry->unk98 = entry->unk98 - 1;
    if (count > 0) {
        if ((u32)entry->unk98 % (u32)entry->unk42 == 0) {
            if (entry->unk3E > 0) {
                entry->unk40 = entry->unk40 - 1;
                if ((s16)entry->unk40 >= 0) {
                    func_80030584((s16)(a0 | (a1 << 8)), &sp10, &sp12);
                    if ((sp10 + 1) < 0x80 && (sp12 + 1) < 0x80) {
                        func_80030404((s16)(a0 | (a1 << 8)), (sp10 + 1) & 0xFFFF, sp12 + 1, 0);
                        goto end;
                    }
                    func_80030404((s16)(a0 | (a1 << 8)), 0x7F, 0x7F, 0);
                    D_800902E8[(s16)a0][(s16)a1].unk90 &= ~0x10;
                    goto end;
                }
                func_80030404((s16)(a0 | (a1 << 8)), 0x7F, 0x7F, 0);
                (*arr)[(s16)a1].unk90 &= ~0x10;
            }
        }
    } else {
        if (entry->unk3E > 0) {
            entry->unk40 = entry->unk40 + count;
            func_80030584((s16)(a0 | (a1 << 8)), &sp10, &sp12);
            if ((s16)entry->unk40 >= 0) {
                s16 d1;
                s16 d2;

                thresh = entry->unk42;
                d1 = sp10 - thresh;
                d2 = sp12 - thresh;
                if (d1 < 0x80 && d2 < 0x80) {
                    func_80030404((s16)(a0 | (a1 << 8)), d1 & 0xFFFF, d2, 0);
                } else {
                    func_80030404((s16)(a0 | (a1 << 8)), 0x7F, 0x7F, 0);
                    D_800902E8[(s16)a0][(s16)a1].unk90 &= ~0x10;
                }
            } else {
                func_80030404((s16)(a0 | (a1 << 8)), 0x7F, 0x7F, 0);
                (*arr)[(s16)a1].unk90 &= ~0x10;
            }
        }
        if (entry->unk98 == 0 || (s16)entry->unk40 == 0) {
            D_800902E8[(s16)a0][(s16)a1].unk90 &= ~0x10;
        }
    }
end:
    func_80030584((s16)(a0 | (a1 << 8)), &entry->unk78, &entry->unk7A);
}
#endif

INCLUDE_ASM("asm/nonmatchings/code_179d8_f", func_80036528);

extern s32 func_8003069C(s32 a0);

void func_800368E8(s32 a0, s32 a1)
{
    s16 sa0 = (s16)a0;
    s16 sa1 = (s16)a1;
    Entry90902E8 *s0 = &D_800902E8[sa0][sa1];
    s32 v7c, v84, v8, vC;
    s16 v72;
    s32 i;

    s0->unk90 &= ~1;
    D_800902E8[sa0][sa1].unk90 &= ~2;
    D_800902E8[sa0][sa1].unk90 &= ~8;
    D_800902E8[sa0][sa1].unk90 |= 4;
    func_8003069C((sa1 << 8) | sa0);

    v7c = s0->unk7C;
    v84 = s0->unk84;
    v72 = s0->unk72;
    v8 = s0->unk8;
    /* Retail reloads +0x8 a second time here rather than reusing v8's
     * value; a plain re-read gets CSE'd back into one load. A volatile
     * read of the same address forces the second `lw` without acting as
     * a general scheduling fence. */
    vC = *(volatile s32 *)&s0->unk8;

    s0->unk2B = 0;
    s0->unk80 = 0;
    s0->unk27 = 0;
    s0->unk13 = 0;
    s0->unk14 = 0;
    s0->unk29 = 0;
    s0->unk15 = 0;
    s0->unk16 = 0;
    s0->unk2A = 0;
    s0->unk12 = 0;
    s0->unk48 = 0;
    s0->unk27 = 0;
    s0->unk28 = 0;
    s0->unk10 = 0;
    s0->unk11 = 0;
    s0->unk88 = v7c;
    s0->unk8C = v84;
    s0->unk70 = v72;
    s0->unk4 = v8;
    s0->unkC = vC;

    for (i = 0; i < 16; i++) {
        s0->unk2C[i] = i;
        s0->unk17[i] = 0x40;
        s0->unk4E[i] = 0x7F;
    }

    s0->unk78 = 0x7F;
    s0->unk7A = 0x7F;
}

void func_80036A54(s32 a0)
{
    func_800368E8((s16)a0, 0);
}

void func_80036A7C(s32 a0, s32 a1)
{
    func_800368E8((s16)a0, (s16)a1);
}

/* func_80038E44 is defined in the Psy-Q SPU/SND block at 0x272C8..0x2C054
 * (the game's own libspu build, which no SDK disc has); its own
 * body is a single straight-line path (no branches) ending in a chain of
 * global stores with $v0 never touched afterward -- genuinely void, not
 * just an unobserved return. */
extern void func_80038E44(s32 a0);

void func_80036AA8(void)
{
    func_80038E44(1);
}
