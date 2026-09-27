/*
 * code_179d8_m -- Sony libsnd `vmanager`, second half: voice key-on/off,
 * noise voices, pitch bend, the volume/pan ramps and the per-tick flush.
 *
 * Every function here is Sony's. Retail's vmanager is a build no SDK disc
 * carries, so its object never placed and the module is carried as C (the
 * symbols file identifies each function against discs 3.3/3.5); progress.py
 * counts all of it as library by address. The functions keep Sony's names
 * and the data keeps Sony's types: the current VAB's header, programs and
 * tones are <libsnd.h>'s VabHdr/ProgAtr/VagAtr behind libsnd's _svm_vh,
 * _svm_pg and _svm_tn (pinned in config/psyq-objects.ld, spelled here by
 * their D_ addresses), a sequence is include/SsScore.h's record, and the
 * per-voice state is include/SvmData.h's _svm_voice/_svm_sreg_buf. SpuRegs,
 * below, is the SPU's own register block at 0x1F801C00.
 *
 *   - SpuVmInit: resets the voice manager: every voice, its shadow
 *     registers and the SPU voice registers, the reverb depth to 0x3FFF,
 *     then one SpuVmFlush.
 *   - SpuVmKeyOn: keys on every tone of the current program whose note
 *     range holds the note, one allocated voice per tone; a volume of 0
 *     calls SpuVmKeyOff instead. SpuVmKeyOff releases every voice playing
 *     that sequence, VAB, program and note, and returns how many.
 *   - SpuVmNoiseOnWithAdsr / SpuVmNoiseOn: allocate a voice and key it on
 *     the noise generator (vmNoiseOn2, code_179d8_l); SpuVmNoiseOff
 *     releases every noise voice.
 *   - SpuVmPBVoice: bends one matching voice's pitch by a 0..127 value
 *     centred on 0x40, scaled by the tone's pbmin/pbmax; SpuVmPitchBend
 *     applies it to every voice and returns how many matched.
 *   - SeAutoPan sets a pan ramp; SetAutoVol / SetAutoPan step a voice's
 *     volume/pan ramp once (SeAutoVol, the volume setter, is in
 *     code_179d8_l).
 *   - SpuVmFlush, once per tick: records which voices' envelopes have died,
 *     releases voices silent across that history (unless _svm_auto_kof_mode
 *     is set), steps the ramps, copies dirty shadow registers to the SPU
 *     and writes the key-on/key-off/reverb masks.
 *
 * No jump table and no rodata attach. Five functions are preserved
 * NON_MATCHING bodies; each report gives its residue.
 */
#include "common.h"
#include <libsnd.h>
#include <libspu.h>
#include "SsScore.h"
#include "SvmData.h"

/* libsnd's _svm_vh (pinned at this address): the header of the VAB bank
 * currently selected. SetAutoVol/SetAutoPan scale by its master volume;
 * SpuVmKeyOn bounds the program number by its program count. */
extern VabHdr *D_8008E970;

extern u8 D_8008EA10;
extern u8 D_8008EA11;
extern u8 D_8008EA16;
extern u8 D_8008EA17;
extern u8 D_8008EA19;
extern u8 D_8008EA1A;

extern s16 D_8008E8C0;

#ifdef NON_MATCHING
/* NON_MATCHING: 231/231 words, length exact, 223/231 raw, funcdiff
 * insertions 1 / deletions 1 (round 73). This is libsnd's SetAutoVol
 * (sdkname shape 0.99). Residue: in the pan split's `else` arm retail
 * copies the volume into $a1 first and multiplies THAT register (no
 * andi); this body multiplies the volume register directly and masks
 * val1 -- same residue as SetAutoPan below
 * (docs/match-reports/SetAutoVol.md). */
void SetAutoVol(s16 voice) {
    s16 v;
    s16 off;
    s32 p;
    s16 acc;
    s32 vol;
    s32 q1;
    u16 val1;
    u16 val2;
    u32 q2;
    s32 tmp;

    v = voice;
    off = voice * 8;
    if (_svm_voice[voice].unk20 != 0) {
        if (_svm_voice[voice].unk22-- > 0) {
            return;
        }
        _svm_voice[voice].unk22 = _svm_voice[voice].unk20;
    }
    _svm_voice[voice].unk24 += _svm_voice[voice].unk1E;
    if (_svm_voice[voice].unk1E > 0) {
        if (_svm_voice[voice].unk24 >= _svm_voice[voice].unk26) {
            _svm_voice[voice].unk24 = _svm_voice[voice].unk26;
            _svm_voice[voice].unk1C = 0;
        }
    } else if (_svm_voice[voice].unk1E < 0) {
        if (_svm_voice[voice].unk24 <= _svm_voice[voice].unk26) {
            _svm_voice[voice].unk24 = _svm_voice[voice].unk26;
            _svm_voice[voice].unk1C = 0;
        }
    }

    acc = _svm_voice[v].unk24;
    D_8008EA10 = acc;

    vol = D_8008E970->mvol * 0x3FFF;
    q2 = (acc * vol) / 16129;
    q2 = (q2 * D_8008EA16 * D_8008EA19) / 16129u;

    p = D_8008EA1A;
    val1 = q2;
    if ((u32)p < 0x40) {
        val2 = (q2 * p) >> 6;
        val1 = q2;
    } else {
        val2 = val1;
        val1 = (val1 * (0x7F - p)) >> 6;
    }

    p = D_8008EA17;
    if ((u32)p < 0x40) {
        val2 = (val2 * p) / 64;
    } else {
        val1 = (val1 * (0x7F - p)) / 64;
    }

    p = D_8008EA11;
    if ((u32)p < 0x40) {
        val2 = (val2 * p) / 64;
    } else {
        val1 = (val1 * (0x7F - p)) / 64;
    }

    if (D_8008E8C0 == 1) {
        if (val2 > val1) {
            val1 = val2;
        } else {
            val2 = val1;
        }
    }

    ((s16 *)_svm_sreg_buf)[off + 1] = val2;
    ((s16 *)_svm_sreg_buf)[off] = val1;
    _svm_sreg_dirty[v] |= 3;
}
#else
INCLUDE_ASM("asm/nonmatchings/code_179d8_m", SetAutoVol);
#endif

void SeAutoPan(s16 a0, s16 a1, s16 a2, s16 a3) {
    s16 q;

    if (a1 == a2) {
        return;
    }
    _svm_voice[a0].unk28 = 1;
    _svm_voice[a0].unk30 = a1;
    _svm_voice[a0].unk32 = a2;
    if ((a1 - a2 < 0 ? a2 - a1 : a1 - a2) < a3) {
        q = a3 / (a1 - a2);
        _svm_voice[a0].unk2A = 1;
        _svm_voice[a0].unk2C = q;
        _svm_voice[a0].unk2E = q;
    } else {
        q = (a1 - a2) / a3;
        _svm_voice[a0].unk2C = 0;
        _svm_voice[a0].unk2A = q;
    }
}


#ifdef NON_MATCHING
/* NON_MATCHING: 228/228 words, length exact, 220/228 raw, funcdiff
 * insertions 1 / deletions 1 (round 73). This is libsnd's SetAutoPan.
 * Residue: the pan split's `else` arm -- retail copies the volume into
 * $a1 and multiplies that copy unmasked; this body masks val1 instead
 * (docs/match-reports/SetAutoPan.md). */
void SetAutoPan(s16 voice) {
    s16 v;
    s16 off;
    s32 p;
    s32 acc;
    s32 vol;
    s32 q1;
    u16 val1;
    u16 val2;
    u32 q2;
    s32 tmp;

    v = voice;
    off = voice * 8;
    if (_svm_voice[voice].unk2C != 0) {
        if (_svm_voice[voice].unk2E-- > 0) {
            return;
        }
        _svm_voice[voice].unk2E = _svm_voice[voice].unk2C;
    }
    _svm_voice[voice].unk30 += _svm_voice[voice].unk2A;
    if (_svm_voice[voice].unk2A > 0) {
        if (_svm_voice[voice].unk30 >= _svm_voice[voice].unk32) {
            _svm_voice[voice].unk30 = _svm_voice[voice].unk32;
            _svm_voice[voice].unk28 = 0;
        }
    } else if (_svm_voice[voice].unk2A < 0) {
        if (_svm_voice[voice].unk30 <= _svm_voice[voice].unk32) {
            _svm_voice[voice].unk30 = _svm_voice[voice].unk32;
            _svm_voice[voice].unk28 = 0;
        }
    }

    acc = *(u8 *)&_svm_voice[v].unk30;
    D_8008EA11 = acc;

    vol = D_8008E970->mvol * 0x3FFF;
    q2 = (D_8008EA10 * vol) / 16129;
    q2 = (q2 * D_8008EA16 * D_8008EA19) / 16129u;

    p = D_8008EA1A;
    val1 = q2;
    if ((u32)p < 0x40) {
        val2 = (q2 * p) >> 6;
        val1 = q2;
    } else {
        val2 = val1;
        val1 = (val1 * (0x7F - p)) >> 6;
    }

    p = D_8008EA17;
    if ((u32)p < 0x40) {
        val2 = (val2 * p) / 64;
    } else {
        val1 = (val1 * (0x7F - p)) / 64;
    }

    p = acc;
    if ((u32)p < 0x40) {
        val2 = (val2 * p) / 64;
    } else {
        val1 = (val1 * (0x7F - p)) / 64;
    }

    if (D_8008E8C0 == 1) {
        if (val2 > val1) {
            val1 = val2;
        } else {
            val2 = val1;
        }
    }

    ((s16 *)_svm_sreg_buf)[off + 1] = val2;
    ((s16 *)_svm_sreg_buf)[off] = val1;
    _svm_sreg_dirty[v] |= 3;
}
#else
INCLUDE_ASM("asm/nonmatchings/code_179d8_m", SetAutoPan);
#endif

extern void _spu_setInTransfer(s32 a0);
extern void SpuVmFlush(void);

extern char D_8008DEB0[]; /* Sony's _ss_spu_vm_rec + 8 (libsnd/vmanager.o bss; symbols file) */

extern s16 D_8008E9FC;
extern s16 D_8008E84C;
extern s16 D_8008E230;
extern s16 D_8008E234;

extern SpuReverbAttr _svm_rattr; /* pinned in config/psyq-objects.ld (libsnd/vm_g.o) */
extern u8 _svm_auto_kof_mode;
extern s16 D_8008E938;

extern volatile u16 D_8008EA26;
extern u8 D_8008E9D0;
extern s16 D_80090BD0;
extern u8 D_8008EA2C[];
extern u16 D_80090C60;
extern u16 D_80090C64;
extern u16 D_8008E228;
extern u16 D_8008E22C;

/* The PS1 SPU's register block, 0x1F801C00: libsnd/vmanager.o's first
 * .data word, which D_8006DAD4 holds. Only the registers this unit touches
 * are named. Declared without volatile, the spelling this unit's matched
 * bodies were derived against (code_179d8_p reads it through a volatile
 * view). */
typedef struct {
    s16 volL;  /* +0x0 -- left volume */
    s16 volR;  /* +0x2 -- right volume */
    s16 pitch; /* +0x4 -- sample rate; 0x1000 plays at 44.1 kHz */
    s16 addr;  /* +0x6 -- start address in sound RAM, in 8-byte units */
    s16 adsr1; /* +0x8 */
    s16 adsr2; /* +0xA */
    u16 envx;  /* +0xC -- current envelope level; 0 once the voice has died */
    u8 padE[0x10 - 0xE];
} SpuVoiceRegs;

typedef struct {
    SpuVoiceRegs voice[24]; /* +0x000 */
    u8 pad180[0x188 - 0x180];
    u16 keyOn[2];  /* +0x188 -- voices 0-15, 16-23: a set bit keys the voice on */
    u16 keyOff[2]; /* +0x18C -- a set bit releases the voice */
    u8 pad190[0x194 - 0x190];
    u16 noiseOn[2];  /* +0x194 -- a set bit plays the voice from the noise generator */
    u16 reverbOn[2]; /* +0x198 -- a set bit sends the voice to the reverb */
} SpuRegs;

extern SpuRegs *D_8006DAD4;

/* MATCHING: `scratch` is one local reused for the clamp and for the
 * volL store's index; two locals change the register allocation. */
void SpuVmInit(s32 a0) {
    s16 i;
    s32 scratch;

    _spu_setInTransfer(0);
    D_8008E9FC = 0;
    D_8008E84C = 0;
    SpuInitMalloc(0x20, D_8008DEB0);

    for (i = 0; (u16)i < 0xC0; i++) {
        ((u16 *)_svm_sreg_buf)[(u16)i] = 0;
    }

    for (i = 0; (u16)i < 0x18; i++) {
        _svm_sreg_dirty[(u16)i] = 0;
    }

    D_80090BD0 = 0;

    for (i = 0; (u16)i < 0x10; i++) {
        D_8008EA2C[(u16)i] = 0;
    }

    a0 = (u8)a0;
    scratch = a0;
    if ((u32)a0 >= 0x18) {
        D_8008E9D0 = 0x18;
    } else {
        D_8008E9D0 = scratch;
    }

    for (i = 0; (u16)i < D_8008E9D0; i++) {
        u16 woff;
        u16 chan;
        u16 lowMask;
        u16 highMask;

        woff = (u16)i * 8;

        _svm_voice[(u16)i].unk02 = 0x18;
        _svm_voice[(u16)i].unk0E = -1;
        _svm_voice[(u16)i].unk00 = 0xFF;
        _svm_voice[(u16)i].unk1B = 0;
        _svm_voice[(u16)i].unk04 = 0;
        _svm_voice[(u16)i].unk06 = 0;
        _svm_voice[(u16)i].unk10 = 0;
        _svm_voice[(u16)i].unk12 = 0;
        _svm_voice[(u16)i].unk14 = 0xFF;
        _svm_voice[(u16)i].unk08 = 0;
        _svm_voice[(u16)i].unk0A = 0x40;
        _svm_voice[(u16)i].unk1C = 0;
        _svm_voice[(u16)i].unk1E = 0;
        _svm_voice[(u16)i].unk20 = 0;
        _svm_voice[(u16)i].unk22 = 0;
        _svm_voice[(u16)i].unk28 = 0;
        _svm_voice[(u16)i].unk2A = 0;
        _svm_voice[(u16)i].unk2C = 0;
        _svm_voice[(u16)i].unk2E = 0;
        _svm_voice[(u16)i].unk30 = 0;
        _svm_voice[(u16)i].unk24 = 0;

        ((s16 *)D_8006DAD4)[woff + 3] = 0x200; /* voice[i].addr */
        scratch = woff;
        ((s16 *)D_8006DAD4)[woff + 2] = 0x1000; /* voice[i].pitch */
        ((u16 *)D_8006DAD4)[woff + 4] = 0x80FF; /* voice[i].adsr1 */
        ((s16 *)D_8006DAD4)[scratch] = 0;       /* voice[i].volL */
        ((s16 *)D_8006DAD4)[woff + 1] = 0;      /* voice[i].volR */
        ((s16 *)D_8006DAD4)[woff + 5] = 0x4000; /* voice[i].adsr2 */

        /* Keeps the D_8008EA26 store and its reload below the six
         * D_8006DAD4 halfword stores; without it GCC hoists them above. */
        __asm__("");
        D_8008EA26 = i;
        chan = D_8008EA26;
        if (chan < 0x10) {
            lowMask = 1 << chan;
            highMask = 0;
        } else {
            lowMask = 0;
            highMask = 1 << (chan - 0x10);
        }

        _svm_voice[chan].unk1B = 0;
        _svm_voice[chan].unk04 = 0;
        _svm_voice[chan].unk00 = 0;
        D_80090C60 |= lowMask;
        D_80090C64 |= highMask;
        woff = D_80090C60;
        D_8008E228 &= ~woff;
        D_8008E22C &= ~D_80090C64;
    }

    _svm_rattr.depth.left = 0x3FFF;
    _svm_rattr.depth.right = 0x3FFF;
    D_8008E228 = 0;
    D_8008E22C = 0;
    D_80090C60 = 0;
    D_8008E230 = 0;
    D_8008E234 = 0;
    _svm_rattr.mask = 0;
    _svm_rattr.mode = 0;
    _svm_auto_kof_mode = 0;
    D_8008E8C0 = 0;
    D_8008E938 = 0x80;
    SpuVmFlush();
}

/* "Currently selected channel" scratch global: written as a side
 * effect, then re-read from the global (not a cached register) a few
 * instructions later -- same idiom, and same `volatile u16` type,
 * code_179d8_j.c documents for this symbol.  Genuinely needs
 * `volatile`: without it, this compiler proves (from the narrow range
 * of the values stored here) that the re-read is redundant and elides
 * it entirely, which retail's disassembly shows it does NOT do.
 * `volatile` alone reproduces retail's separate store/reload exactly
 * -- reading it back through `*(u8 *)&D_8008EA26` (a plain, NON-
 * volatile-qualified pointer type) still folds to retail's compact
 * `lui`+`lbu` two-instruction form; it is specifically a
 * VOLATILE-QUALIFIED POINTER TYPE (`volatile u8 *`) that defeats the
 * addressing fold, not the underlying object's volatility. */
extern volatile u16 D_8008EA26;
/* Loop bound / threshold, read fresh each call -- same symbol
 * code_179d8_j.c documents as "loop bound for a small table of active
 * objects". */
extern u8 D_8008E9D0;
/* Flag byte forced on unconditionally at entry. */
extern u8 D_8008EA1B;

extern s32 SpuVmAlloc(s32 a0); /* arity-ok: the callee (still INCLUDE_ASM, 0x8002CF18) reads NO argument register, but this unit's argument is byte-load-bearing -- retail emits `li a0,0xff` in the delay slot at 0x8002F244 */
extern void vmNoiseOn2(s32 a0, s32 a1, s32 a2, s32 a3, s32 a4);

void SpuVmNoiseOnWithAdsr(s32 a0, s32 a1, s32 a2, s32 a3) {
    s32 v0;

    D_8008EA1B = 0x7F;
    v0 = SpuVmAlloc(0xFF) & 0xFF;
    D_8008EA26 = v0;
    if (v0 < D_8008E9D0) {
        vmNoiseOn2(*(u8 *)&D_8008EA26, a0 & 0xFFFF, a1 & 0xFFFF, a2 & 0xFFFF, a3 & 0xFFFF);
    }
}

void SpuVmNoiseOff(void) {
    s16 i;

    for (i = 0; i < D_8008E9D0; i++) {
        if (_svm_voice[i].unk1B == 2) {
            _svm_voice[(u8)i].unk1B = 0;
            _svm_voice[(u8)i].unk04 = 0;
            D_8006DAD4->noiseOn[0] = 0;
            D_8006DAD4->noiseOn[1] = 0;
        }
    }
}

void SpuVmNoiseOn(s32 a0, s32 a1) {
    s32 v0;

    D_8008EA1B = 0x7F;
    v0 = SpuVmAlloc(0xFF) & 0xFF;
    D_8008EA26 = v0;
    if (v0 < D_8008E9D0) {
        vmNoiseOn2(*(u8 *)&D_8008EA26, a0 & 0xFFFF, a1 & 0xFFFF, 0x80FF, 0x5FC8);
    }
}

/* _svm_cur + 7: the current program number. A VAB gives each program 16
 * tone slots, so it indexes _svm_tn in steps of 16, and SpuVmKeyOn
 * refuses one at or past _svm_vh->ps. */
extern u8 D_8008EA13;

/* libsnd's _svm_tn (pinned at this address): the current VAB's tone
 * attributes, 16 per program. */
extern VagAtr *D_8008E978;

/* _svm_cur + 0xC: the current tone number within the program. */
extern u8 D_8008EA18;

extern s16 note2pitch2(u16 a0, u16 a1);

#ifdef NON_MATCHING
/* NON_MATCHING: 77/138 words, length exact. Residue: register-class
 * renumbering plus one deferred `& 0xFFFF` mask on the second
 * note2pitch2 argument -- a banned-to-fix register-identity case, per
 * a permuter search that plateaued at 485/770 with no candidate reaching
 * zero (docs/match-reports/SpuVmPBVoice.md). Hand-derived. */
s16 SpuVmPBVoice(s16 a0, s16 a1, s16 a2, s16 a3, u16 a4) {
    s16 threshold;
    u16 someTotal;
    u16 baseValue;
    s32 outA2;
    s32 outA1;
    s32 product;
    s32 q;
    s32 r;
    u8 tableByte;
    u8 byteVal;

    threshold = a4 - 0x40;
    if (_svm_voice[a0].unk0E != a1) {
        return 0;
    }
    if (_svm_voice[a0].unk16 != a2) {
        return 0;
    }
    if (_svm_voice[a0].unk12 != a3) {
        return 0;
    }

    someTotal = _svm_voice[a0].unk14 + (D_8008EA13 << 4);
    baseValue = _svm_voice[a0].unk0C;

    if (threshold > 0) {
        tableByte = D_8008E978[someTotal].pbmax;
        product = threshold * tableByte;
        q = product / 63;
        outA2 = baseValue + q;
        r = product % 63;
        outA1 = r * 2;
    } else {
        outA2 = baseValue;
        if (threshold < 0) {
            tableByte = D_8008E978[someTotal].pbmin;
            product = threshold * tableByte;
            q = product / 64;
            outA2 = baseValue + q - 1;
            r = product % 64;
            outA1 = r * 2 + 0x7F;
        } else {
            outA1 = 0;
        }
    }

    byteVal = *(u8 *)&_svm_voice[a0].unk14;
    D_8008EA26 = a0;
    D_8008EA18 = byteVal;
    _svm_sreg_buf[a0].unk4 = note2pitch2(outA2 & 0xFFFF, outA1 & 0xFFFF);
    _svm_sreg_dirty[a0] |= 4;
    return 1;
}
#else
INCLUDE_ASM("asm/nonmatchings/code_179d8_m", SpuVmPBVoice);
#endif

/* "Currently selected channel" scratch global -- same idiom as
 * D_8008EA26 above, write-only here (see code_179d8_j.c's own reading
 * of this symbol). */
extern u16 D_8008EA22;

extern s32 SpuVmVSetUp(s16 a0, s16 a1);
extern s16 SpuVmPBVoice(s16 a0, s16 a1, s16 a2, s16 a3, u16 a4);

s32 SpuVmPitchBend(s16 a0, s16 a1, s16 a2, u16 a3) {
    s16 i;
    s32 sum;

    SpuVmVSetUp(a1, a2);
    D_8008EA22 = a0;
    sum = 0;
    for (i = 0; i < D_8008E9D0; i++) {
        sum += SpuVmPBVoice(i, a0, a1, a2, a3);
    }
    return sum;
}

#ifdef NON_MATCHING
/* NON_MATCHING: 236/241 words, 5 words short. Residue: retail's
 * unconditional `move a2,v0`/`li t0,1`/`move a3,a0` do-while-style setup
 * before the count>0 loop, which this C's `for` does not reproduce (a
 * literal do-while conversion was tried and regressed hard, 236/241,
 * 93/241 raw match -> 233/241, 16/241 -- reverted), plus cosmetic
 * s0/s1 register-color swaps in phases 2-6. Round 48's frame-padding
 * lever and an `s32 count` fix (drop a spurious andi mask) already
 * applied; a permuter search (round 37) plateaued at 1578/2276 with no
 * candidate reaching zero (docs/match-reports/SpuVmFlush.md).
 * Hand-derived. */

/* Ring buffer of "channel activity" bitmasks, one slot appended per
 * call, most-recent index tracked by _svm_envx_ptr (mod 16). */
extern s32 _svm_envx_ptr;
extern s32 _svm_envx_hist[];

/* A u16 at a 0x34 stride: SpuVmFlush walks _svm_voice's +0x06 field
 * with a pointer of this type (store, then re-check the SAME field via
 * `lhu`). Kept as the walk's own element type so the pointer steps one
 * record at a time from &_svm_voice[0].unk06. */
typedef struct {
    u16 unk0; /* +0x0 */
    u8 pad2[0x34 - 0x2];
} Rec34HalfU2;

extern void SetAutoVol(s16 a0);
extern void SetAutoPan(s16 a0);

void SpuVmFlush(void) {
    s32 i = 0;
    s32 ringIdx;
    s32 *slot;
    s32 count;
    u8 dead[8];

    if (0) {
        dead[0] = 0;
    }

    ringIdx = (_svm_envx_ptr + 1) & 0xF;
    _svm_envx_ptr = ringIdx;
    slot = &_svm_envx_hist[ringIdx];
    count = D_8008E9D0;
    *slot = 0;

    if (count > 0) {
        Rec34HalfU2 *p98E = (Rec34HalfU2 *)&_svm_voice[0].unk06;
        SpuVoiceRegs *pDad = D_8006DAD4->voice;

        for (i = 0; i < count; i++) {
            p98E->unk0 = pDad->envx;
            if (p98E->unk0 == 0) {
                *slot |= 1 << i;
            }
            p98E++;
            pDad++;
        }
    }

    if (_svm_auto_kof_mode == 0) {
        s32 mask;
        s32 j;

        mask = -1;
        for (j = 0; j < 0xF; j++) {
            mask &= _svm_envx_hist[j];
        }

        for (i = 0; i < D_8008E9D0; i++) {
            s32 bit = 1 << i;

            if (mask & bit) {
                if (_svm_voice[i].unk1B == 2) {
                    SpuSetNoiseVoice(0, 0xFFFFFF);
                }
                _svm_voice[i].unk1B = 0;
            }
        }
    }

    D_8008E228 &= ~D_80090C60;
    D_8008E22C &= ~D_80090C64;

    for (i = 0; i < 0x18; i++) {
        if (_svm_voice[i].unk1C != 0) {
            SetAutoVol(i);
        }
        if (_svm_voice[i].unk28 != 0) {
            SetAutoPan(i);
        }
    }

    {
        SvmSreg *p7F0 = _svm_sreg_buf;

        for (i = 0; i < 0x18; i++) {
            if (_svm_sreg_dirty[i] & 1) {
                D_8006DAD4->voice[i].volL = p7F0->unk0;
                D_8006DAD4->voice[i].volR = p7F0->unk2;
            }
            if (_svm_sreg_dirty[i] & 4) {
                D_8006DAD4->voice[i].pitch = _svm_sreg_buf[i].unk4;
            }
            if (_svm_sreg_dirty[i] & 8) {
                D_8006DAD4->voice[i].addr = _svm_sreg_buf[i].unk6;
            }
            if (_svm_sreg_dirty[i] & 0x10) {
                D_8006DAD4->voice[i].adsr1 = p7F0->unk8;
                D_8006DAD4->voice[i].adsr2 = p7F0->unkA;
            }

            _svm_sreg_dirty[i] = 0;
            p7F0++;
        }
    }

    {
        SpuRegs *rec = D_8006DAD4;
        u16 lowMask = D_80090C60;
        u16 highMask = D_80090C64;
        u16 lowActive = D_8008E228;
        u16 highActive = D_8008E22C;
        s16 v230 = D_8008E230;
        s16 v234 = D_8008E234;

        D_80090C60 = 0;
        D_80090C64 = 0;
        D_8008E228 = 0;
        D_8008E22C = 0;

        rec->keyOff[0] = lowMask;
        rec->keyOff[1] = highMask;
        rec->keyOn[0] = lowActive;
        rec->keyOn[1] = highActive;
        rec->reverbOn[0] = v230;
        rec->reverbOn[1] = v234;
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/code_179d8_m", SpuVmFlush);
#endif

#ifdef NON_MATCHING
/* NON_MATCHING: 414 built words vs 387 (measured round 96; an earlier
 * score read 402). Residue as recorded then: two independent
 * pieces -- an early-materialization scheduling point (~2-3 words) and a
 * mid-loop addressing-cost difference for D_8008EA26 and neighbors
 * (~10-12 words); round 48's frame-padding lever realigned the frame
 * byte-exactly without closing either. A permuter search (round 37)
 * plateaued at 12471/15053 across 51,596 iterations with no candidate
 * reaching zero (docs/match-reports/SpuVmKeyOn.md). Hand-derived; two
 * stale-symbol fixes applied per tools/stalesyms.py (round 37): the
 * renamed-to-SpuVmVSetUp call (was func_80032148) and D_8008EA0D, which
 * has no linker symbol of its own and is read through the already-linked
 * D_8008EA24 base pointer instead. */
/* libsnd's _svm_pg (pinned at this address): the current VAB's program
 * attributes. */
extern ProgAtr *_svm_pg;

extern u8 D_8008EA0C;
extern u8 D_8008EA0E;
extern u8 D_8008EA0F;
extern u8 D_8008EA1C;
extern u8 D_8008EA1D;
extern u8 D_8008EA1E;
extern u8 D_8008EA1F;
extern u8 D_8008EA20;
extern u16 D_8008EA24;


extern void SpuVmDoAllocate(void);
extern void vmNoiseOn(s32 a0);
extern s32 note2pitch(void);
extern void SpuVmKeyOnNow(s32 a0, u16 a1);
extern u8 SpuVmKeyOff(s16 a0, s16 a1, s16 a2, u16 a3);

s32 SpuVmKeyOn(s32 a0, s16 a1, s16 a2, u16 a3, u16 a4, u16 a5) {
    SsScore *s6;
    ProgAtr *slot;
    s32 s3;
    s32 chan;
    u8 matchCount;
    u8 chanScan;
    s16 origA2;
    s32 shifted;
    s16 a0s16;
    u8 byte0;
    u8 byte1;
    u8 idBuf[0x80];
    u8 chanBuf[0x80];

    origA2 = a2;
    byte0 = (u8)a0;
    shifted = a0 << 16;
    a0s16 = (s16)(shifted >> 16);
    byte1 = (u32)shifted >> 24;
    s6 = &_ss_score[byte0][byte1];

    if (SpuVmVSetUp(a1, a2) != 0) {
        return -1;
    }

    slot = &_svm_pg[a2];
    D_8008EA22 = (s16)a0;
    D_8008EA0E = (u8)a3;
    D_8008EA0F = 0;
    D_8008EA10 = (u8)a4;
    D_8008EA11 = (u8)a5;
    D_8008EA16 = slot->mvol;
    D_8008EA17 = slot->mpan;
    D_8008EA0C = slot->tones;

    if ((u32)D_8008EA13 >= D_8008E970->ps) {
        return -1;
    }

    s3 = 0;
    if (a4 != 0) {
        matchCount = 0;
        for (chanScan = 0; chanScan < D_8008EA0C; chanScan++) {
            VagAtr *entry = &D_8008E978[D_8008EA13 * 16 + chanScan];

            if (D_8008EA0E < entry->min) {
                continue;
            }
            if (entry->max < D_8008EA0E) {
                continue;
            }
            idBuf[matchCount] = entry->vag;
            chanBuf[matchCount] = chanScan;
            matchCount++;
        }

        if (matchCount != 0) {
            s32 s2;
            u8 s1;

            s2 = a4 * 127;
            for (s1 = 0; s1 < matchCount; s1++) {
                VagAtr *entry2;

                D_8008EA24 = idBuf[s1];
                D_8008EA18 = chanBuf[s1];

                entry2 = &D_8008E978[D_8008EA13 * 16 + D_8008EA18];
                D_8008EA1B = entry2->prior;
                D_8008EA19 = entry2->vol;
                D_8008EA1A = entry2->pan;
                D_8008EA1C = entry2->center;
                D_8008EA1D = entry2->shift;
                D_8008EA20 = entry2->mode;
                D_8008EA1E = entry2->min;
                D_8008EA1F = entry2->max;

                chan = SpuVmAlloc(0) & 0xFF;
                D_8008EA26 = chan;
                if (chan < D_8008E9D0) {
                    _svm_voice[chan].unk1B = 1;
                    _svm_voice[D_8008EA26].unk02 = 0;
                    _svm_voice[D_8008EA26].unk0E = (s16)a0;
                    _svm_voice[D_8008EA26].unk16 = *((u8 *)&D_8008EA24 - 0x17);
                    _svm_voice[D_8008EA26].unk10 = D_8008EA13;
                    _svm_voice[D_8008EA26].unk12 = origA2;

                    if ((s16)a0 != 0x21) {
                        s16 speed = s6->unk4E[s6->unk12];

                        _svm_voice[D_8008EA26].unk08 = s2 / speed;
                    }

                    _svm_voice[D_8008EA26].unk0A = (u8)a5;
                    _svm_voice[D_8008EA26].unk14 = D_8008EA18;
                    _svm_voice[D_8008EA26].unk0C = a3;
                    _svm_voice[D_8008EA26].unk18 = D_8008EA1B;
                    _svm_voice[D_8008EA26].unk00 = D_8008EA24;

                    SpuVmDoAllocate();
                    if (D_8008EA24 == 0xFF) {
                        vmNoiseOn(*(u8 *)&D_8008EA26);
                    } else {
                        SpuVmKeyOnNow(matchCount, note2pitch());
                    }
                    s3 = (s3 << 4) | D_8008EA26;
                }
            }
        }
    } else {
        SpuVmKeyOff(a0s16, a1, a2, a3);
    }

    return s3;
}
#else
INCLUDE_ASM("asm/nonmatchings/code_179d8_m", SpuVmKeyOn);
#endif

/* A pair of 16-bit bitmasks split across a 0..0x1F channel space (low
 * 16 channels in the first word, next 16 in the second), each paired
 * with an "active mask" word cleared wherever the channel mask bit is
 * set -- same symbols and reading code_179d8_j.c already documents. */
extern u16 D_80090C60;
extern u16 D_80090C64;
extern u16 D_8008E228;
extern u16 D_8008E22C;

u8 SpuVmKeyOff(s16 a0, s16 a1, s16 a2, u16 a3) {
    u8 i;
    u8 count;

    count = 0;
    for (i = 0; i < D_8008E9D0; i++) {
        if (_svm_voice[i].unk0C != a3) {
            continue;
        }
        if (_svm_voice[i].unk12 != a2) {
            continue;
        }
        if (_svm_voice[i].unk0E != a0) {
            continue;
        }
        if (_svm_voice[i].unk16 != a1) {
            continue;
        }
        if (_svm_voice[i].unk00 == 0xFF) {
            _svm_voice[i].unk1B = 0;
            _svm_voice[i].unk04 = 0;
            D_8006DAD4->noiseOn[0] = 0;
            D_8006DAD4->noiseOn[1] = 0;
        } else {
            u16 chan;
            u16 lowMask;
            u16 highMask;

            D_8008EA26 = i;
            chan = D_8008EA26;
            if (chan < 0x10) {
                lowMask = 1 << chan;
                highMask = 0;
            } else {
                lowMask = 0;
                highMask = 1 << (chan - 0x10);
            }
            _svm_voice[chan].unk1B = 0;
            _svm_voice[chan].unk04 = 0;
            _svm_voice[chan].unk00 = 0;
            D_80090C60 |= lowMask;
            D_80090C64 |= highMask;
            D_8008E228 &= ~D_80090C60;
            D_8008E22C &= ~D_80090C64;
        }
        count++;
    }
    return count;
}
