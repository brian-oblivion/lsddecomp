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
 * code_179d8_p -- SsUtAllKeyOff, 0x2273C..0x22948 (vram 0x80031F3C..
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
 * READ THIS BEFORE STARTING: SsUtAllKeyOff touches the same global family as
 * `code_179d8_m` -- _svm_voice/98A/98C/98E/996/998/99A/99C/9A3 and
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
extern Rec34U16 _svm_voice[];
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
 * This function's OWN reading of D_8006DAD4: a POINTER VARIABLE (loaded with
 * `lw`, not an array base) into the PS1 SPU voice register block -- the value
 * is 0x1F801C00, established when code_179d8_m was named as the 24-voice
 * sound driver.  Indexed as HALFWORDS: `D_8006DAD4[woff + N]` with
 * `s16 woff = i * 8`, i.e. 8 halfwords (0x10 bytes) per voice, which is the
 * SPU's own per-voice register stride.  code_179d8_m.c reads the SAME symbol
 * as a fixed-offset object pointer (its own ObjDAD4, offsets 0x194/0x196) --
 * a different, valid reading per the project's convention.
 *
 * ROUND 66: the POINTEE MUST BE `volatile`.  These are hardware registers, and
 * the qualifier is load-bearing for the MATCH, not just for correctness: cc1
 * 2.6.3 orders volatile accesses against other volatile accesses only, so
 * without it cc1 hoists the `D_8008EA26` volatile store/reload pair across
 * these six stores.  Worth 54/131 -> 98/131.  Round 31's `Rec16DAD4` struct
 * spelling (stride 0x10, fields f0..fA) is RETRACTED: it compiles the index as
 * a plain late `sll 4` instead of retail's split `sll 19` / `sra 15`.
 */
extern volatile u16 *D_8006DAD4;

/* PS1 SPU voice key-on/off pair, split low/high across two 16-bit halves
 * (voices 0-15 / 16-31) -- D_80090C60/64 are the hardware-mirrored "just
 * keyed on" mask, D_8008E228/22C a software mask this function clears the
 * same bit from (a "no longer fading out" bookkeeping flag). */
extern u16 D_80090C60;
extern u16 D_80090C64;
extern u16 D_8008E228;
extern u16 D_8008E22C;

/* STALL -- see docs/match-reports/SsUtAllKeyOff.md.  Round 66 revisit:
 * best-derived body now compiles to EXACT LENGTH (was 9 words SHORT);
 * raw word-match 103/131 (was 42/131); first real diff at vram 0x80032064
 * (file 0x22864) -- `sllv a3,t2,a0` vs `sllv a2,t2,a0`, a register-identity
 * residue on bitLo/bitHi.  Preserved here for the next attempt.  Note the
 * four load-bearing spellings: D_8006DAD4's POINTEE is volatile (it holds
 * 0x1F801C00, the SPU voice registers) which is what pins cc1's scheduler;
 * `s16 woff = i * 8` (SIGNED) is what fuses into retail's sll19/sra15 split
 * shift; the loop is a `for`, not a guard plus `for(;;)`; and the tail does
 * three stores off the reloaded index, not one.
 */
#if 0
void SsUtAllKeyOff(void)
{
    s16 i;
    s16 woff;
    u16 bitpos;
    u32 bitLo;
    u32 bitHi;
    u16 hw0;
    u16 hw1;

    for (i = 0; i < D_8008E9D0; i++) {
        woff = i * 8;
        D_8008D98A[i].unk0 = 0x18;
        _svm_voice[i].unk0 = 0xFF;
        D_8008D9A3[i].unk0 = 0;
        D_8008D98C[i].unk0 = 0;
        D_8008D98E[i].unk0 = 0;
        D_8008D996[i].unk0 = 0xFF;
        D_8008D998[i].unk0 = 0;
        D_8008D99A[i].unk0 = 0;
        D_8008D99C[i].unk0 = 0xFF;

        D_8006DAD4[woff + 3] = 0x200;
        D_8006DAD4[woff + 2] = 0x1000;
        D_8006DAD4[woff + 4] = 0x80FF;
        D_8006DAD4[woff + 0] = 0;
        D_8006DAD4[woff + 1] = 0;
        D_8006DAD4[woff + 5] = 0x4000;

        D_8008EA26 = i;
        bitpos = D_8008EA26 & 0xFFFF;
        if (bitpos < 0x10) {
            bitLo = 1u << bitpos;
            bitHi = 0;
        } else {
            bitLo = 0;
            bitHi = 1u << (bitpos - 0x10);
        }

        D_8008D9A3[bitpos & 0xFFFF].unk0 = 0;
        D_8008D98C[bitpos & 0xFFFF].unk0 = 0;
        _svm_voice[bitpos & 0xFFFF].unk0 = 0;

        hw0 = D_80090C60;
        hw1 = D_80090C64;
        hw0 = bitLo | hw0;
        D_80090C60 = hw0;
        D_8008E228 = D_8008E228 & ~hw0;
        hw1 = bitHi | hw1;
        D_80090C64 = hw1;
        D_8008E22C = D_8008E22C & ~hw1;
    }
}
#endif
INCLUDE_ASM("asm/nonmatchings/code_179d8_p", SsUtAllKeyOff);
