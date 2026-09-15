/*
 * ROUND 42 CORRECTION (2026-09-15) -- READ BEFORE ANY "BLOCKED" LINE BELOW:
 * every claim in this comment that a function is BLOCKED by `gp_rel`,
 * `nop_mflo_mfhi` or `addiu_at` is STALE.  All three constructs are RESOLVED
 * by pinned maspsx flags (CLAUDE.md, "Open toolchain blockers");
 * `tools/nearmiss.py` reports them tagged (RESOLVED-not-a-blocker) and counts
 * none of them.  Any "do NOT spend attempts on these" directive below is
 * therefore RETRACTED: those functions are ordinary matching work, and most
 * carry a mechanism-correct partial derivation already.  The rest of this
 * comment still stands -- only the blocker verdicts are withdrawn.
 * Screen: `python3 tools/nearmiss.py`, round 43 (2026-09-15).
 *
 * code_179d8_f -- what is LEFT of functions 256..273 of the original
 * 274-function code_179d8 monolith after round 34 gave sixteen of its
 * eighteen functions back to Sony.  Now 0x26D28..0x270E8
 * (vram 0x80036528..0x800368E8), a ONE-function unit.
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
 * simply moved to 0x26D28.
 *
 * A SECOND RUN in the same round then took the other end.  `libsnd/stop`
 * (3.3, 0x270E8..0x272A8) is `Snd_stop`, `SsSeqStop` and `SsSepStop` --
 * func_800368E8/A54/A7C, all three previously MATCHED as C and all three now
 * deleted from here.  It sat in the MIDDLE of what the prefix trim had left,
 * so the slice became [c][o][c] and the tail half became the one-function
 * unit `src/code_179d8_f_b.c` (func_80036AA8).  Nothing moved with it: this
 * unit never owned a rodata attach.
 *
 * WHAT IS LEFT OF THIS UNIT IS ONE FUNCTION, func_80036528.
 *
 * Owns NO switch jump table (zero `jtbl_` in its disassembly, and the splat
 * yaml's rodata slot list names no `.rodata, code_179d8_f` line), so no
 * rodata attach, before or after the split.
 *
 * BLOCKER PROFILE: screen with `python3 tools/nearmiss.py`, never by
 * re-implementing the greps and never for `addiu_at` (resolved round 21).
 * func_80036528 (240w) is now this unit's ONLY function; it has been attempted
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
