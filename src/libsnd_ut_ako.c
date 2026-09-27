/*
 * libsnd_ut_ako -- Sony's libsnd/ut_ako module, carried as disassembly
 * because no SDK disc carries the build retail linked.
 *
 * SsUtAllKeyOff resets every voice the voice manager owns (spuVmMaxVoice of
 * them): its _svm_voice record (include/SvmData.h), its SPU voice
 * registers (_svm_sreg points at them, 0x1F801C00, eight halfwords per
 * voice), and its bit in the key-off masks. It keeps Sony's name and
 * <libsnd.h>'s prototype.
 *
 * Which object (nm over sdk/work/<disc>/elf/libsnd): ut_ako.o on the 3.6
 * disc, with this one function as its only text symbol; on 3.0, 3.3 and
 * 3.5 it is the last function of vmanager.o. 3.6's text is 0x138 bytes
 * against retail's 0x20C, so it cannot be linked.
 *
 * What decided its edges (python3 tools/tuboundary.py --unit): the unit
 * before it, libsnd_ut_cp_ut_cadsr_ut_vvol_ut_autov_ut_autop.c, ends on
 * ut_autop's SsUtAutoPan ("start edge possible"; a Sony module edge by
 * content, so not merged), and the placed object libsnd/vm_vsu follows it.
 *
 * This file's declarations stay local, except Sony's _svm_voice, whose one
 * type is include/SvmData.h.
 */
#include "common.h"
#include "SvmData.h"

/* code_179d8_m.c's own comment on this exact symbol: "written as a side
 * effect, then re-read from the global (not a cached register) a few
 * instructions later... Genuinely needs volatile: without it, this
 * compiler proves... the re-read is redundant and elides it entirely."
 * Independently re-confirmed here: the same store-then-reload shape shows
 * up in this function's own disassembly. */
extern volatile u16 D_8008EA26;

/* "Loop bound / threshold" -- code_179d8_m.c's own comment on this symbol. */
extern u8 spuVmMaxVoice;

/*
 * This function's OWN reading of _svm_sreg: a POINTER VARIABLE (loaded with
 * `lw`, not an array base) into the PS1 SPU voice register block -- the value
 * is 0x1F801C00, established when code_179d8_m was named as the 24-voice
 * sound driver.  Indexed as HALFWORDS: `_svm_sreg[woff + N]` with
 * `s16 woff = i * 8`, i.e. 8 halfwords (0x10 bytes) per voice, which is the
 * SPU's own per-voice register stride.  code_179d8_m.c reads the SAME symbol
 * as a fixed-offset object pointer (its own SpuRegs, offsets 0x194/0x196) --
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
extern volatile u16 *_svm_sreg;

/* PS1 SPU voice key-on/off pair, split low/high across two 16-bit halves
 * (voices 0-15 / 16-31) -- _svm_okof1/64 are the hardware-mirrored "just
 * keyed on" mask, D_8008E228/22C a software mask this function clears the
 * same bit from (a "no longer fading out" bookkeeping flag). */
extern u16 _svm_okof1;
extern u16 _svm_okof2;
extern u16 D_8008E228;
extern u16 _svm_okon2;

/* STALL -- see docs/match-reports/SsUtAllKeyOff.md.  Round 66 revisit:
 * best-derived body now compiles to EXACT LENGTH (was 9 words SHORT);
 * raw word-match 103/131 (was 42/131); first real diff at vram 0x80032064
 * (file 0x22864) -- `sllv a3,t2,a0` vs `sllv a2,t2,a0`, a register-identity
 * residue on bitLo/bitHi.  Preserved here for the next attempt.  Note the
 * four load-bearing spellings: _svm_sreg's POINTEE is volatile (it holds
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

    for (i = 0; i < spuVmMaxVoice; i++) {
        woff = i * 8;
        _svm_voice[i].unk02 = 0x18;
        _svm_voice[i].unk00 = 0xFF;
        _svm_voice[i].unk1B = 0;
        _svm_voice[i].unk04 = 0;
        _svm_voice[i].unk06 = 0;
        _svm_voice[i].unk0E = 0xFF;
        _svm_voice[i].unk10 = 0;
        _svm_voice[i].unk12 = 0;
        _svm_voice[i].unk14 = 0xFF;

        _svm_sreg[woff + 3] = 0x200;
        _svm_sreg[woff + 2] = 0x1000;
        _svm_sreg[woff + 4] = 0x80FF;
        _svm_sreg[woff + 0] = 0;
        _svm_sreg[woff + 1] = 0;
        _svm_sreg[woff + 5] = 0x4000;

        D_8008EA26 = i;
        bitpos = D_8008EA26 & 0xFFFF;
        if (bitpos < 0x10) {
            bitLo = 1u << bitpos;
            bitHi = 0;
        } else {
            bitLo = 0;
            bitHi = 1u << (bitpos - 0x10);
        }

        _svm_voice[bitpos & 0xFFFF].unk1B = 0;
        _svm_voice[bitpos & 0xFFFF].unk04 = 0;
        _svm_voice[bitpos & 0xFFFF].unk00 = 0;

        hw0 = _svm_okof1;
        hw1 = _svm_okof2;
        hw0 = bitLo | hw0;
        _svm_okof1 = hw0;
        D_8008E228 = D_8008E228 & ~hw0;
        hw1 = bitHi | hw1;
        _svm_okof2 = hw1;
        _svm_okon2 = _svm_okon2 & ~hw1;
    }
}
#endif
INCLUDE_ASM("asm/nonmatchings/libsnd_ut_ako", SsUtAllKeyOff);
