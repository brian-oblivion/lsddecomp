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
 * code_179d8_p -- func_80031F3C, 0x2273C..0x22948 (vram 0x80031F3C..
 * 0x80032148).  A single 131-word function.  Carved round 26 (2026-09-09)
 * out of what had been the `code_179d8_mid_d` remainder; renamed on carve
 * because "mid_d" named a leftover and the leftover is now fully consumed.
 *
 * Blocker census at carve time, four screens: BLOCKER-CLEAN -- zero gp_rel,
 * zero forward nop_mflo_mfhi, zero `jr $t2` trampolines, zero `jtbl_`, zero
 * `alabel`.  The body is FRAMELESS (no `addiu $sp, $sp, -N` anywhere), which
 * is worth knowing before you write C for it.
 *
 * It had been parked for several rounds as "addiu-$at blocked"; `addiu_at`
 * was resolved in round 21 and was its only obstruction.
 *
 * Owns NO jump table, so no rodata sub-slot is attached.  It does reference
 * plain rodata/data SYMBOLS -- reference them as symbols, never re-type a
 * string literal (splat has already emitted those bytes; a literal emits a
 * second copy and shifts the whole image).
 *
 * READ THIS BEFORE STARTING: func_80031F3C touches the same global family as
 * `code_179d8_m` -- D_8008D988/98A/98C/98E/996/998/99A/99C/9A3 and
 * D_8006DAD4.  The "split scaled index" entry in
 * docs/DECOMPILATION_LEARNINGS.md (a mask on the PRODUCT means a halfword
 * array indexed by a truncated `idx*8`, NOT a struct array indexed by a cast
 * index) was derived on exactly those globals, together with its
 * loop-versus-non-loop refinement.  It is very likely to apply here.
 *
 * This unit's extern declarations stay LOCAL to this file; it shares no
 * project header with any other unit and should not acquire one.
 */
#include "common.h"

/*
 * This function's own local view of the shared 0x34-stride channel-
 * configuration record family that code_179d8_j.c/code_179d8_m.c already
 * document (Rec34D994/Rec34Half/Rec34Byte there) -- kept LOCAL per the
 * project's multiple-independent-local-views convention, not a shared
 * header. Every symbol here is 0x34 bytes from its neighbor; only the
 * one field this function writes is named per symbol.
 */
typedef struct {
    u16 unk0; /* +0x0 */
    u8 pad2[0x34 - 0x2];
} Rec34U16;
extern Rec34U16 D_8008D988[];
extern Rec34U16 D_8008D98A[];
extern Rec34U16 D_8008D98C[];
extern Rec34U16 D_8008D98E[];
extern Rec34U16 D_8008D996[];
extern Rec34U16 D_8008D998[];
extern Rec34U16 D_8008D99A[];
extern Rec34U16 D_8008D99C[];

typedef struct {
    u8 unk0; /* +0x0 */
    u8 pad1[0x34 - 0x1];
} Rec34U8;
extern Rec34U8 D_8008D9A3[];

/* code_179d8_m.c's own comment on this exact symbol: "written as a side
 * effect, then re-read from the global (not a cached register) a few
 * instructions later... Genuinely needs volatile: without it, this
 * compiler proves... the re-read is redundant and elides it entirely."
 * Independently re-confirmed here: the same store-then-reload shape shows
 * up in this function's own disassembly. */
extern volatile u16 D_8008EA26;

/* "Loop bound / threshold" -- code_179d8_m.c's own comment on this symbol. */
extern u8 D_8008E9D0;

/*
 * This function's OWN reading of D_8006DAD4: a POINTER VARIABLE (loaded
 * with `lw`, not an array base) to a 0x10-byte-stride record, indexed by
 * channel. code_179d8_m.c reads the SAME symbol as a fixed-offset object
 * pointer (its own ObjDAD4, offsets 0x194/0x196) -- a different, valid
 * reading per the project's convention: same global, two shapes, two
 * independent local views.
 */
typedef struct {
    u16 f0;  /* +0x0 */
    u16 f2;  /* +0x2 */
    u16 f4;  /* +0x4 */
    u16 f6;  /* +0x6 */
    u16 f8;  /* +0x8 */
    u16 fA;  /* +0xA */
    u8 pad[0x10 - 0xC];
} Rec16DAD4;
extern Rec16DAD4 *D_8006DAD4;

/* PS1 SPU voice key-on/off pair, split low/high across two 16-bit halves
 * (voices 0-15 / 16-31) -- D_80090C60/64 are the hardware-mirrored "just
 * keyed on" mask, D_8008E228/22C a software mask this function clears the
 * same bit from (a "no longer fading out" bookkeeping flag). */
extern u16 D_80090C60;
extern u16 D_80090C64;
extern u16 D_8008E228;
extern u16 D_8008E22C;

/* STALL -- see docs/match-reports/func_80031F3C.md.  Best-derived body
 * compiles to 122/131 words (9 SHORT); raw word-match 42/131 under that
 * drift; first real diff at vram 0x80031F4C (file 0x2274C) -- a missing
 * `sll a0,a0,0x13` (the early half of a split-shift index computation).
 * Preserved here for the next attempt. */
#if 0
void func_80031F3C(void)
{
    s16 i;
    u16 bitpos;
    u32 bitLo;
    u32 bitHi;
    u16 hw0;
    u16 hw1;
    u16 mask0;
    u16 mask1;
    if (D_8008E9D0 == 0) {
        return;
    }

    i = 0;
    for (;;) {
        D_8008D98A[i].unk0 = 0x18;
        D_8008D988[i].unk0 = 0xFF;
        D_8008D9A3[i].unk0 = 0;
        D_8008D98C[i].unk0 = 0;
        D_8008D98E[i].unk0 = 0;
        D_8008D996[i].unk0 = 0xFF;
        D_8008D998[i].unk0 = 0;
        D_8008D99A[i].unk0 = 0;
        D_8008D99C[i].unk0 = 0xFF;

        D_8006DAD4[i].f6 = 0x200;
        D_8006DAD4[i].f4 = 0x1000;
        D_8006DAD4[i].f8 = 0x80FF;
        D_8006DAD4[i].f0 = 0;
        D_8006DAD4[i].f2 = 0;
        D_8006DAD4[i].fA = 0x4000;

        D_8008EA26 = i;
        bitpos = D_8008EA26 & 0xFFFF;
        if (bitpos < 0x10) {
            bitLo = 1u << bitpos;
            bitHi = 0;
        } else {
            bitLo = 0;
            bitHi = 1u << (bitpos - 0x10);
        }

        i++;
        D_8008D9A3[bitpos & 0xFFFF].unk0 = 0;

        hw0 = D_80090C60;
        hw1 = D_80090C64;
        mask0 = D_8008E228;
        hw0 = (u16)(bitLo | hw0);
        D_80090C60 = hw0;
        mask0 = mask0 & (u16) ~hw0;
        D_8008E228 = mask0;

        mask1 = D_8008E22C;
        hw1 = (u16)(bitHi | hw1);
        D_80090C64 = hw1;
        mask1 = mask1 & (u16) ~hw1;
        D_8008E22C = mask1;

        if (!(i < D_8008E9D0)) {
            break;
        }
    }
}
#endif
INCLUDE_ASM("asm/nonmatchings/code_179d8_p", func_80031F3C);
