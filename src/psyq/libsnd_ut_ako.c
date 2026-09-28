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
 * Its declarations are include/libsnd_internal.h's, except _svm_sreg,
 * which it reads through its own spelling.
 */
#include "common.h"
#include "libsnd_internal.h"

/*
 * _svm_sreg as this function reads it: a pointer variable (loaded with `lw`,
 * not an array base) to the SPU voice register block at 0x1F801C00, indexed
 * as halfwords, `_svm_sreg[woff + N]` with `s16 woff = i * 8`: 8 halfwords
 * (0x10 bytes) per voice, the SPU's own per-voice stride. libsnd_vmanager.c
 * reads the same symbol as a pointer to its SpuRegs.
 *
 * MATCHING: the pointee is volatile. cc1 orders volatile accesses only
 * against other volatile accesses, so without it the D_8008EA26 store/reload
 * pair is hoisted across these six stores.
 */
extern volatile u16 *_svm_sreg;

#ifdef NON_MATCHING
/* NON_MATCHING: length exact, 131 words; a register-identity residue on
 * bitLo/bitHi (docs/match-reports/SsUtAllKeyOff.md). */
void SsUtAllKeyOff(s16 mode) {
    s16 i;
    s16 woff;
    u16 bitpos;
    u32 bitLo;
    u32 bitHi;
    u16 hw0;
    u16 hw1;

    for (i = 0; i < spuVmMaxVoice; i++) {
        woff = i * 8;
        _svm_voice[i].age = 0x18;
        _svm_voice[i].vag = 0xFF;
        _svm_voice[i].keyState = 0;
        _svm_voice[i].pitch = 0;
        _svm_voice[i].envx = 0;
        _svm_voice[i].seq = 0xFF;
        _svm_voice[i].progIndex = 0;
        _svm_voice[i].prog = 0;
        _svm_voice[i].tone = 0xFF;

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

        _svm_voice[bitpos & 0xFFFF].keyState = 0;
        _svm_voice[bitpos & 0xFFFF].pitch = 0;
        _svm_voice[bitpos & 0xFFFF].vag = 0;

        hw0 = _svm_okof1;
        hw1 = _svm_okof2;
        hw0 = bitLo | hw0;
        _svm_okof1 = hw0;
        _svm_okon1 = _svm_okon1 & ~hw0;
        hw1 = bitHi | hw1;
        _svm_okof2 = hw1;
        _svm_okon2 = _svm_okon2 & ~hw1;
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/psyq/libsnd_ut_ako", SsUtAllKeyOff);
#endif
