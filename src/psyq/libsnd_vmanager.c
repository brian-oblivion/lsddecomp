/*
 * libsnd_vmanager -- Sony's libsnd voice manager (`libsnd/vmanager`),
 * carried as C.
 *
 * Everything here is Sony's, under Sony's names. Retail's
 * build of the voice manager is on no SDK disc, so it never placed as
 * objects; the symbols file identifies each function against discs 3.3/3.5
 * and progress.py counts them as library by address. On the 3.0/3.3/3.5
 * discs every function here is in one object, vmanager.o; on the 3.6 disc
 * they are sixteen, in this order: vm_aloc1 (SpuVmAlloc), vm_nowon
 * (SpuVmKeyOnNow), vm_aloc2 (SpuVmDoAllocate), vm_no1 (vmNoiseOn), vm_no2
 * (vmNoiseOn2), vm_n2p (note2pitch, note2pitch2), vm_spb (SePitchBend),
 * vm_vib (SsUtVibrateOn/Off), vm_autov (SeAutoVol, SetAutoVol), vm_autop
 * (SeAutoPan, SetAutoPan), vm_init (SpuVmInit), vm_noise
 * (SpuVmNoiseOnWithAdsr, SpuVmNoiseOff, SpuVmNoiseOn), vm_pb (SpuVmPBVoice,
 * SpuVmPitchBend), vm_f (SpuVmFlush), vm_key (SpuVmKeyOn, SpuVmKeyOff,
 * SpuVmSeKeyOn, SpuVmSeKeyOff, KeyOnCheck) and vm_seq (the SpuVm*SeqVol
 * accessors and SpuVmSeqKeyOff).
 *
 * The data keeps Sony's types: the key-on request in _svm_cur, the current
 * VAB through <libsnd.h>'s VabHdr, ProgAtr and VagAtr (_svm_vh, _svm_pg,
 * _svm_tn), the voice tables in include/SvmData.h and the sequence records
 * in include/SsScore.h. A key-on or key-off is not written to the SPU when
 * it is requested: it is collected in the _svm_okon/_svm_okof masks, which
 * SpuVmFlush writes out once per tick. No jump table and no rodata attach.
 */
#include "common.h"
#include "libsnd_internal.h"
#include <libspu.h>

/* _svm_cur + 0x0C, the tone within the program (the rest of _svm_cur is in
 * libsnd_internal.h). MATCHING: plain here; volatile changes SePitchBend
 * and SpuVmKeyOn. */
extern u8 D_8008EA18;

/* The SPU's register block. */
extern SpuRegs *_svm_sreg;
/* vmanager's static pitch table: 12 semitones x 16 fine steps, the SPU
 * pitch of each at octave 5. */
extern u16 D_8006DAD8[];

#ifdef NON_MATCHING
/* NON_MATCHING: 167/167 words, length exact; the residue is a register
 * rotation through the whole function (docs/match-reports/SpuVmAlloc.md). */

/* `unused`: nothing here reads it; see its declaration in
 * libsnd_internal.h. */
s32 SpuVmAlloc(s32 unused) {
    s32 chosen;
    u16 bestSec;
    s32 found;
    s32 bestTer;
    s32 bestIdx;
    u32 idx;
    s32 threshold;
    s32 pri;
    u32 newSec;
    u32 count;

    chosen = 99;
    bestSec = 0xFFFF;
    found = 0;
    bestTer = 0;
    bestIdx = 99;
    threshold = D_8008EA1B;

    for (idx = 0; (u8)idx < spuVmMaxVoice; idx++) {
        if (_svm_voice[(u8)idx].unk1B != 0 || _svm_voice[(u8)idx].unk06 != 0) {
            pri = _svm_voice[(u8)idx].unk18;
            if (pri < (s32)(u16)threshold) {
                threshold = pri;
                bestIdx = idx;
                bestSec = _svm_voice[(u8)idx].unk06;
                bestTer = _svm_voice[(u8)idx].unk02;
                found = 1;
            } else if (pri == (s32)(u16)threshold) {
                found++;
                newSec = _svm_voice[(u8)idx].unk06;
                if (newSec < bestSec) {
                    bestTer = _svm_voice[(u8)idx].unk02;
                    bestSec = newSec;
                    bestIdx = idx;
                } else if (newSec == bestSec) {
                    if (bestTer < (s16)_svm_voice[(u8)idx].unk02) {
                        bestTer = (s16)_svm_voice[(u8)idx].unk02;
                        bestIdx = idx;
                    }
                }
            }
        } else {
            chosen = idx;
        }
    }

    if ((u8)chosen == 99) {
        if ((u8)found != 0) {
            chosen = bestIdx;
        } else {
            chosen = spuVmMaxVoice;
        }
    }

    count = spuVmMaxVoice;
    if ((u8)chosen < count) {
        if (count != 0) {
            for (idx = 0; (u8)idx < count; idx++) {
                _svm_voice[(u8)idx].unk02 = _svm_voice[(u8)idx].unk02 + 1;
            }
        }
        _svm_voice[(u8)chosen].unk02 = 0;
        _svm_voice[(u8)chosen].unk18 = D_8008EA1B;
        if (_svm_voice[(u8)chosen].unk1B == 2) {
            SpuSetNoiseVoice(SPU_OFF, SPU_ALLCH);
        }
    }
    return (u8)chosen;
}
#else
INCLUDE_ASM("asm/nonmatchings/psyq/libsnd_vmanager", SpuVmAlloc);
#endif

#ifdef NON_MATCHING
/* NON_MATCHING: 316/316 words, length exact; the residue is the frame size
 * (8 bytes against retail's 16) and this unit's register-identity class
 * (docs/match-reports/SpuVmKeyOnNow.md). */
/* Sets the voice's volumes from _svm_cur's volumes and pans, and its pitch,
 * then marks it to be keyed on (and to or from reverb) at the next flush. */
#define KEYON_VOICE (*(s16 *)&D_8008EA26)

void SpuVmKeyOnNow(s32 unused, s32 pitch) {
    SsScore *score;
    s32 masterVol;
    s32 toneVol;
    u32 vol;
    u32 volL;
    u32 volR;
    u32 volLSq;
    u32 volRSq;
    s16 outL;
    s32 sregIndex;
    s32 lowBit;
    s32 highBit;

    masterVol = _svm_vh->mvol * 16383;
    toneVol = D_8008EA10 * masterVol / 16129;
    vol = (u32)toneVol * D_8008EA16 * D_8008EA19 / 16129;

    sregIndex = KEYON_VOICE * 8;

    score = &_ss_score[D_8008EA22 & 0xFF][D_8008EA22 >> 8];
    volL = vol;
    volR = vol;
    if ((s16)D_8008EA22 != 33) {
        volL = vol * score->unk74 / 127;
        volR = vol * score->unk76 / 127;
    }

    if ((u8)D_8008EA1A < 64) {
        volR = (volR * D_8008EA1A) / 63;
    } else {
        volL = (volL * (127 - D_8008EA1A)) / 63;
    }

    if ((u8)D_8008EA17 < 64) {
        volR = (volR * D_8008EA17) / 63;
    } else {
        volL = (volL * (127 - D_8008EA17)) / 63;
    }

    if ((u8)D_8008EA11 < 64) {
        volR = (D_8008EA11 * volR) / 63;
    } else {
        volL = (volL * (127 - D_8008EA11)) / 63;
    }

    if (_svm_stereo_mono == 1) {
        if (volL < volR) {
            volL = volR;
        } else {
            volR = volL;
        }
    }
    volLSq = volL * volL;
    outL = (s16)(volLSq / 16383);
    volRSq = volR * volR;

    ((s16 *)_svm_sreg_buf)[(u16)sregIndex + 2] = (s16)pitch;
    ((s16 *)_svm_sreg_buf)[(u16)sregIndex] = outL;
    ((s16 *)_svm_sreg_buf)[(u16)sregIndex + 1] = (s16)(volRSq / 16383);

    _svm_sreg_dirty[KEYON_VOICE] |= 7;
    _svm_voice[KEYON_VOICE].unk04 = (s16)pitch;
    _svm_voice[KEYON_VOICE].unk1B = 1;

    if (KEYON_VOICE < 16) {
        lowBit = 1 << KEYON_VOICE;
        highBit = 0;
    } else {
        lowBit = 0;
        highBit = 1 << (KEYON_VOICE - 16);
    }

    if (D_8008EA20 & 4) {
        _svm_orev1 = lowBit | _svm_orev1;
        _svm_orev2 = highBit | _svm_orev2;
    } else {
        _svm_orev1 = _svm_orev1 & ~lowBit;
        _svm_orev2 = _svm_orev2 & ~highBit;
    }

    _svm_okon1 = lowBit | _svm_okon1;
    _svm_okon2 = highBit | _svm_okon2;
    _svm_okof1 = _svm_okof1 & ~_svm_okon1;
    _svm_okof2 = _svm_okof2 & ~_svm_okon2;
}

#undef KEYON_VOICE
#else
INCLUDE_ASM("asm/nonmatchings/psyq/libsnd_vmanager", SpuVmKeyOnNow);
#endif

/* Not yet C: the best body (142/143 words) is in
 * docs/match-reports/SpuVmDoAllocate.md. */
INCLUDE_ASM("asm/nonmatchings/psyq/libsnd_vmanager", SpuVmDoAllocate);

/* Not yet C: the best body (309/311 words) is in
 * docs/match-reports/vmNoiseOn.md. */
INCLUDE_ASM("asm/nonmatchings/psyq/libsnd_vmanager", vmNoiseOn);

#ifdef NON_MATCHING
/* NON_MATCHING: 107/112 words, 5 short; the residue is a register role swap
 * between the voice argument and its copy, and an 8-byte frame retail
 * allocates (docs/match-reports/vmNoiseOn2.md, which also has the order-only
 * __asm__("") barrier this copy omits). */
/* Switches the voice to the noise generator at volumes volL/volR and keys it
 * on. It becomes the only noise voice: any other is marked off (state 2 & 1),
 * and the SPU's noise mask gets this voice's bit alone. */

/* The last two parameters are the ADSR words SpuVmNoiseOnWithAdsr and
 * SpuVmNoiseOn pass; retail's body reads neither. */
void vmNoiseOn2(s32 voice, s32 volL, s32 volR, s32 unusedAdsr1, s32 unusedAdsr2) {
    s32 voiceArg;
    s32 dirty;
    s32 lowBit;
    s32 highBit;
    s32 i;
    s32 n;

    voiceArg = voice;
    voice = (u8)voice;
    _svm_sreg_buf[voice].unk2 = volR;
    dirty = _svm_sreg_dirty[voice];
    _svm_sreg_buf[voice].unk0 = volL;
    dirty |= 3;
    _svm_sreg_dirty[voice] = dirty;
    if ((u32)voice < 16) {
        lowBit = 1 << voice;
        highBit = 0;
    } else {
        lowBit = 0;
        highBit = 1 << (voice - 16);
    }

    n = spuVmMaxVoice;
    _svm_voice[(u8)voiceArg].unk04 = 10;
    if (n != 0) {
        i = 0;
        do {
            _svm_voice[(u16)i].unk1B = _svm_voice[(u16)i].unk1B & 1;
            i++;
        } while ((u16)i < spuVmMaxVoice);
    }
    _svm_voice[(u8)voiceArg].unk1B = 2;

    _svm_voice[(u8)voiceArg].unk02 = 0;
    _svm_okon1 = lowBit | _svm_okon1;
    _svm_okon2 = highBit | _svm_okon2;
    _svm_okof1 = _svm_okof1 & ~_svm_okon1;
    _svm_okof2 = _svm_okof2 & ~_svm_okon2;
    _svm_sreg->noiseOn[0] = lowBit;
    _svm_sreg->noiseOn[1] = highBit;
}
#else
INCLUDE_ASM("asm/nonmatchings/psyq/libsnd_vmanager", vmNoiseOn2);
#endif

/* The SPU pitch of _svm_cur's note on its tone: semitones from the centre
 * note, looked up at octave 5 and shifted to the note's octave. */
s32 note2pitch(void) {
    s32 semitones;
    s32 octave;
    s16 semitone;
    u8 step;
    u16 pitch;

    semitones = (s16)(D_8008EA0E + 60 - D_8008EA1C);
    octave = semitones / 12;
    step = D_8008EA1D >> 3;
    semitone = semitones - octave * 12;
    if (step >= 16) {
        step = 15;
    }
    pitch = D_8006DAD8[step + semitone * 16];
    if ((s16)(octave - 5) > 0) {
        pitch <<= (s16)(octave - 5);
    } else if ((s16)(octave - 5) < 0) {
        pitch = (u16)pitch >> -(s16)(octave - 5);
    }
    return pitch;
}

/* note2pitch for an explicit note and fine tune, on _svm_cur's tone: the
 * fine tune plus the tone's shift is in 8ths of a table step, carrying into
 * the next semitone past 16 steps. */
s32 note2pitch2(s32 note, s32 fine) {
    s32 toneIndex;
    s32 tableIndex;
    VagAtr *tone;
    s32 fineTotal;
    s32 steps;
    u8 carry;
    s16 step;
    s32 semitones;
    s32 octave;
    s16 semitone;
    u16 pitch;

    toneIndex = D_8008EA18 + (D_8008EA13 << 4);
    tone = &_svm_tn[toneIndex];
    fineTotal = (u16)fine + tone->shift;
    steps = fineTotal / 8;
    step = steps;
    carry = 0;
    if (steps >= 16) {
        carry = 1;
        step = steps - 16;
    }
    semitones = (s16)(carry + (note + 60 - tone->center));
    octave = semitones / 12;
    semitone = semitones - octave * 12;
    tableIndex = semitone * 16;
    tableIndex = tableIndex + step;
    pitch = D_8006DAD8[tableIndex];
    if ((s16)(octave - 5) > 0) {
        pitch <<= (s16)(octave - 5);
    } else if ((s16)(octave - 5) < 0) {
        pitch = (u16)pitch >> -(s16)(octave - 5);
    }
    return pitch;
}

/* Bends a keyed voice by `bend` (signed, 127 = the tone's full pbmax
 * semitones up; below 0, its pbmin down) and retunes it. */
void SePitchBend(s32 chan, s32 bend) {
    s32 sregIndex;
    s32 prod;
    s32 q;
    s32 note;
    s32 fine;
    s16 amount;
    s32 toneIndex;
    u8 *curProg;

    sregIndex = (chan & 0xFF) * 8;
    if ((u32)(chan & 0xFF) < 24) {
        /* MATCHING: the direct D_8008EA13 spelling does not match. */
        curProg = &D_8008EA13;
        *curProg = (u8)_svm_voice[(chan & 0xFF)].unk10;
        D_8008EA18 = (u8)_svm_voice[(chan & 0xFF)].unk14;
        D_8008EA26 = (u8)chan;
        toneIndex = D_8008EA18 + (*curProg << 4);
        amount = bend;
        if (amount >= 0) {
            prod = amount * _svm_tn[toneIndex].pbmax;
            note = (u16)_svm_voice[(chan & 0xFF)].unk0C + prod / 127;
            fine = prod % 127;
        } else {
            q = (amount * _svm_tn[toneIndex].pbmin) / 127;
            note = (u16)_svm_voice[(chan & 0xFF)].unk0C + q - 1;
            fine = q + 127;
        }
        ((u16 *)_svm_sreg_buf)[sregIndex + 2] = note2pitch2((u16)note, (u16)fine);
        _svm_sreg_dirty[(chan & 0xFF)] |= 4;
    }
}

void SsUtVibrateOn(short vc, short vibW, short vibT) {}

void SsUtVibrateOff(short vc) {}

/* Starts a volume ramp on a voice from `from` to `to` over `duration`
 * ticks, which SetAutoVol steps. Same body as SeAutoPan (below),
 * over _svm_voice +0x1C..+0x26 instead of +0x28..+0x32. */
void SeAutoVol(s16 voice, s16 from, s16 to, s16 duration) {
    s16 q;

    if (from == to) {
        return;
    }
    _svm_voice[voice].unk1C = 1;
    _svm_voice[voice].unk24 = from;
    _svm_voice[voice].unk26 = to;
    if ((from - to < 0 ? to - from : from - to) < duration) {
        q = duration / (from - to);
        _svm_voice[voice].unk1E = 1;
        _svm_voice[voice].unk20 = q;
        _svm_voice[voice].unk22 = q;
    } else {
        q = (from - to) / duration;
        _svm_voice[voice].unk20 = 0;
        _svm_voice[voice].unk1E = q;
    }
}

/*
 * From SetAutoVol to SpuVmKeyOff: voice key-on/off, noise voices, pitch
 * bend, the volume/pan ramps and the per-tick flush. _svm_pg and _svm_tn are
 * pinned in config/psyq-objects.ld and spelled here by their D_ addresses;
 * the per-voice state is include/SvmData.h's _svm_voice/_svm_sreg_buf, and
 * _svm_sreg points at the SPU's own register block, SvmData.h's SpuRegs.
 *
 *   - SpuVmInit: resets the voice manager: every voice, its shadow
 *     registers and the SPU voice registers, the reverb depth to 0x3FFF,
 *     then one SpuVmFlush.
 *   - SpuVmKeyOn: keys on every tone of the current program whose note
 *     range holds the note, one allocated voice per tone; a volume of 0
 *     calls SpuVmKeyOff instead. SpuVmKeyOff releases every voice playing
 *     that sequence, VAB, program and note, and returns how many.
 *   - SpuVmNoiseOnWithAdsr / SpuVmNoiseOn: allocate a voice and key it on
 *     the noise generator (vmNoiseOn2, above); SpuVmNoiseOff
 *     releases every noise voice.
 *   - SpuVmPBVoice: bends one matching voice's pitch by a 0..127 value
 *     centred on 0x40, scaled by the tone's pbmin/pbmax; SpuVmPitchBend
 *     applies it to every voice and returns how many matched.
 *   - SeAutoPan sets a pan ramp; SetAutoVol / SetAutoPan step a voice's
 *     volume/pan ramp once (SeAutoVol, the volume setter, is above).
 *   - SpuVmFlush, once per tick: records which voices' envelopes have died,
 *     releases voices silent across that history (unless _svm_auto_kof_mode
 *     is set), steps the ramps, copies dirty shadow registers to the SPU
 *     and writes the key-on/key-off/reverb masks.
 *
 * Five functions in this stretch are preserved NON_MATCHING bodies; each
 * report gives its residue.
 */

#ifdef NON_MATCHING
/* NON_MATCHING: 231/231 words, length exact, 223/231 raw. This is libsnd's
 * SetAutoVol. Residue: in the pan split's `else` arm retail copies the
 * volume into $a1 first and multiplies that register (no andi); this body
 * multiplies the volume register directly and masks val1, as in SetAutoPan
 * below (docs/match-reports/SetAutoVol.md). */
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

    vol = _svm_vh->mvol * 0x3FFF;
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

    if (_svm_stereo_mono == 1) {
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
INCLUDE_ASM("asm/nonmatchings/psyq/libsnd_vmanager", SetAutoVol);
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
/* NON_MATCHING: 228/228 words, length exact, 220/228 raw. This is libsnd's
 * SetAutoPan. Residue: the pan split's `else` arm: retail copies the volume
 * into $a1 and multiplies that copy unmasked; this body masks val1 instead
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

    vol = _svm_vh->mvol * 0x3FFF;
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

    if (_svm_stereo_mono == 1) {
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
INCLUDE_ASM("asm/nonmatchings/psyq/libsnd_vmanager", SetAutoPan);
#endif

extern void _spu_setInTransfer(s32 a0);

extern char D_8008DEB0[]; /* Sony's _ss_spu_vm_rec + 8 (libsnd/vmanager.o bss; symbols file) */

extern s16 D_8008E9FC;
extern s16 _svm_damper;

extern SpuReverbAttr _svm_rattr; /* pinned in config/psyq-objects.ld (libsnd/vm_g.o) */
extern u8 _svm_auto_kof_mode;
extern s16 kMaxPrograms;

extern s16 _svm_vab_count;
extern u8 _svm_vab_used[];

/* MATCHING: `scratch` is one local reused for the clamp and for the
 * volL store's index; two locals change the register allocation. */
void SpuVmInit(s32 a0) {
    s16 i;
    s32 scratch;

    _spu_setInTransfer(0);
    D_8008E9FC = 0;
    _svm_damper = 0;
    SpuInitMalloc(0x20, D_8008DEB0);

    for (i = 0; (u16)i < 0xC0; i++) {
        ((u16 *)_svm_sreg_buf)[(u16)i] = 0;
    }

    for (i = 0; (u16)i < 0x18; i++) {
        _svm_sreg_dirty[(u16)i] = 0;
    }

    _svm_vab_count = 0;

    for (i = 0; (u16)i < 0x10; i++) {
        _svm_vab_used[(u16)i] = 0;
    }

    a0 = (u8)a0;
    scratch = a0;
    if ((u32)a0 >= 0x18) {
        spuVmMaxVoice = 0x18;
    } else {
        spuVmMaxVoice = scratch;
    }

    for (i = 0; (u16)i < spuVmMaxVoice; i++) {
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

        ((s16 *)_svm_sreg)[woff + 3] = 0x200; /* voice[i].addr */
        scratch = woff;
        ((s16 *)_svm_sreg)[woff + 2] = 0x1000; /* voice[i].pitch */
        ((u16 *)_svm_sreg)[woff + 4] = 0x80FF; /* voice[i].adsr1 */
        ((s16 *)_svm_sreg)[scratch] = 0;       /* voice[i].volL */
        ((s16 *)_svm_sreg)[woff + 1] = 0;      /* voice[i].volR */
        ((s16 *)_svm_sreg)[woff + 5] = 0x4000; /* voice[i].adsr2 */

        /* Keeps the D_8008EA26 store and its reload below the six
         * _svm_sreg halfword stores; without it GCC hoists them above. */
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
        _svm_okof1 |= lowMask;
        _svm_okof2 |= highMask;
        woff = _svm_okof1;
        _svm_okon1 &= ~woff;
        _svm_okon2 &= ~_svm_okof2;
    }

    _svm_rattr.depth.left = 0x3FFF;
    _svm_rattr.depth.right = 0x3FFF;
    _svm_okon1 = 0;
    _svm_okon2 = 0;
    _svm_okof1 = 0;
    _svm_orev1 = 0;
    _svm_orev2 = 0;
    _svm_rattr.mask = 0;
    _svm_rattr.mode = 0;
    _svm_auto_kof_mode = 0;
    _svm_stereo_mono = 0;
    kMaxPrograms = 0x80;
    SpuVmFlush();
}

extern void vmNoiseOn2(s32 a0, s32 a1, s32 a2, s32 a3, s32 a4);

void SpuVmNoiseOnWithAdsr(s32 a0, s32 a1, s32 a2, s32 a3) {
    s32 v0;

    D_8008EA1B = 0x7F;
    v0 = SpuVmAlloc(0xFF) & 0xFF;
    D_8008EA26 = v0;
    if (v0 < spuVmMaxVoice) {
        vmNoiseOn2(*(u8 *)&D_8008EA26, a0 & 0xFFFF, a1 & 0xFFFF, a2 & 0xFFFF, a3 & 0xFFFF);
    }
}

void SpuVmNoiseOff(void) {
    s16 i;

    for (i = 0; i < spuVmMaxVoice; i++) {
        if (_svm_voice[i].unk1B == 2) {
            _svm_voice[(u8)i].unk1B = 0;
            _svm_voice[(u8)i].unk04 = 0;
            _svm_sreg->noiseOn[0] = 0;
            _svm_sreg->noiseOn[1] = 0;
        }
    }
}

void SpuVmNoiseOn(s32 a0, s32 a1) {
    s32 v0;

    D_8008EA1B = 0x7F;
    v0 = SpuVmAlloc(0xFF) & 0xFF;
    D_8008EA26 = v0;
    if (v0 < spuVmMaxVoice) {
        vmNoiseOn2(*(u8 *)&D_8008EA26, a0 & 0xFFFF, a1 & 0xFFFF, 0x80FF, 0x5FC8);
    }
}

#ifdef NON_MATCHING
/* NON_MATCHING: 77/138 words, length exact. Residue: register-class
 * renumbering plus one deferred `& 0xFFFF` mask on the second note2pitch2
 * argument, a register-identity case (docs/match-reports/SpuVmPBVoice.md). */
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
        tableByte = _svm_tn[someTotal].pbmax;
        product = threshold * tableByte;
        q = product / 63;
        outA2 = baseValue + q;
        r = product % 63;
        outA1 = r * 2;
    } else {
        outA2 = baseValue;
        if (threshold < 0) {
            tableByte = _svm_tn[someTotal].pbmin;
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
INCLUDE_ASM("asm/nonmatchings/psyq/libsnd_vmanager", SpuVmPBVoice);
#endif


s32 SpuVmPitchBend(s16 a0, s16 a1, s16 a2, u16 a3) {
    s16 i;
    s32 sum;

    SpuVmVSetUp(a1, a2);
    D_8008EA22 = a0;
    sum = 0;
    for (i = 0; i < spuVmMaxVoice; i++) {
        sum += SpuVmPBVoice(i, a0, a1, a2, a3);
    }
    return sum;
}

#ifdef NON_MATCHING
/* NON_MATCHING: 236/241 words, 5 words short. Residue: retail's
 * unconditional `move a2,v0`/`li t0,1`/`move a3,a0` do-while-style setup
 * before the count>0 loop, which this C's `for` does not reproduce, plus
 * s0/s1 register swaps in phases 2-6 (docs/match-reports/SpuVmFlush.md). */

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
    count = spuVmMaxVoice;
    *slot = 0;

    if (count > 0) {
        Rec34HalfU2 *p98E = (Rec34HalfU2 *)&_svm_voice[0].unk06;
        SpuVoiceRegs *pDad = _svm_sreg->voice;

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

        for (i = 0; i < spuVmMaxVoice; i++) {
            s32 bit = 1 << i;

            if (mask & bit) {
                if (_svm_voice[i].unk1B == 2) {
                    SpuSetNoiseVoice(0, 0xFFFFFF);
                }
                _svm_voice[i].unk1B = 0;
            }
        }
    }

    _svm_okon1 &= ~_svm_okof1;
    _svm_okon2 &= ~_svm_okof2;

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
                _svm_sreg->voice[i].volL = p7F0->unk0;
                _svm_sreg->voice[i].volR = p7F0->unk2;
            }
            if (_svm_sreg_dirty[i] & 4) {
                _svm_sreg->voice[i].pitch = _svm_sreg_buf[i].unk4;
            }
            if (_svm_sreg_dirty[i] & 8) {
                _svm_sreg->voice[i].addr = _svm_sreg_buf[i].unk6;
            }
            if (_svm_sreg_dirty[i] & 0x10) {
                _svm_sreg->voice[i].adsr1 = p7F0->unk8;
                _svm_sreg->voice[i].adsr2 = p7F0->unkA;
            }

            _svm_sreg_dirty[i] = 0;
            p7F0++;
        }
    }

    {
        SpuRegs *rec = _svm_sreg;
        u16 lowMask = _svm_okof1;
        u16 highMask = _svm_okof2;
        u16 lowActive = _svm_okon1;
        u16 highActive = _svm_okon2;
        s16 v230 = _svm_orev1;
        s16 v234 = _svm_orev2;

        _svm_okof1 = 0;
        _svm_okof2 = 0;
        _svm_okon1 = 0;
        _svm_okon2 = 0;

        rec->keyOff[0] = lowMask;
        rec->keyOff[1] = highMask;
        rec->keyOn[0] = lowActive;
        rec->keyOn[1] = highActive;
        rec->reverbOn[0] = v230;
        rec->reverbOn[1] = v234;
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/psyq/libsnd_vmanager", SpuVmFlush);
#endif

#ifdef NON_MATCHING
/* NON_MATCHING: 414 built words vs 387. Residue: an early-materialization
 * scheduling point and a mid-loop addressing-cost difference for D_8008EA26
 * and its neighbours (docs/match-reports/SpuVmKeyOn.md). D_8008EA0D has no
 * linker symbol of its own and is read through the D_8008EA24 base pointer. */
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

    if ((u32)D_8008EA13 >= _svm_vh->ps) {
        return -1;
    }

    s3 = 0;
    if (a4 != 0) {
        matchCount = 0;
        for (chanScan = 0; chanScan < D_8008EA0C; chanScan++) {
            VagAtr *entry = &_svm_tn[D_8008EA13 * 16 + chanScan];

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

                entry2 = &_svm_tn[D_8008EA13 * 16 + D_8008EA18];
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
                if (chan < spuVmMaxVoice) {
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
INCLUDE_ASM("asm/nonmatchings/psyq/libsnd_vmanager", SpuVmKeyOn);
#endif


s32 SpuVmKeyOff(s16 a0, s16 a1, s16 a2, u16 a3) {
    u8 i;
    u8 count;

    count = 0;
    for (i = 0; i < spuVmMaxVoice; i++) {
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
            _svm_sreg->noiseOn[0] = 0;
            _svm_sreg->noiseOn[1] = 0;
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
            _svm_okof1 |= lowMask;
            _svm_okof2 |= highMask;
            _svm_okon1 &= ~_svm_okof1;
            _svm_okon2 &= ~_svm_okof2;
        }
        count++;
    }
    return count;
}

/*
 * The last eight functions:
 *
 *   SpuVmSeKeyOn    keys on a sound effect: SpuVmKeyOn on the SE pseudo-
 *                   sequence, with the louder of the two channel volumes as
 *                   the volume and their ratio as the pan.
 *   SpuVmSeKeyOff   keys off a sound effect SpuVmSeKeyOn started.
 *   KeyOnCheck      vmanager's empty internal hook.
 *   SpuVmSetSeqVol  sets one sequence's L/R volume (SsScore unk74/unk76,
 *                   clamped to 127) and rescales the voices it is playing.
 *   SpuVmGetSeqVol, SpuVmGetSeqLVol, SpuVmGetSeqRVol
 *                   read that volume back.
 *   SpuVmSeqKeyOff  keys off every voice one sequence is playing.
 *
 * A sequence is named by one packed number: the SEQ/SEP access in the low
 * byte, the sequence within it in the high byte, reaching
 * _ss_score[access][seq] (include/SsScore.h). Each volume accessor
 * records it in D_8008EA22, vmanager's current-sequence global
 * (SpuVmGetSeqLVol records only the access byte).
 */

/* The pseudo-sequence number sound effects are keyed on and off under;
 * libsnd's SE paths store it in D_8008EA22 and in a voice's owner field. */
#define SPUVM_SE_SEQ 0x21

/* Keys on note of vabId's program prog as a sound effect. The pan is 64
 * (centre) for equal volumes and leans toward the louder side by the ratio
 * of the quieter to the louder, 64ths of the way; the fourth argument is
 * unused. */
s32 SpuVmSeKeyOn(s32 vabId, s32 prog, s32 note, s32 unused, u16 volL, u16 volR) {
    u16 vol;
    u16 pan;

    if (volL == volR) {
        pan = 64;
        vol = volL;
    } else if (volR < volL) {
        vol = volL;
        pan = (volR << 6) / volL;
    } else {
        vol = volR;
        pan = 127 - ((volL << 6) / volR);
    }
    return SpuVmKeyOn(SPUVM_SE_SEQ, (s16)vabId, (s16)prog, (u16)note, vol, pan);
}

s32 SpuVmSeKeyOff(s16 vabId, s16 prog, u16 note) {
    return SpuVmKeyOff(SPUVM_SE_SEQ, vabId, prog, note);
}

void KeyOnCheck(void) {}

INCLUDE_ASM("asm/nonmatchings/psyq/libsnd_vmanager", SpuVmSetSeqVol);

/* Returns the packed number, read back through D_8008EA22. */
s32 SpuVmGetSeqVol(s32 seqSepNo, s16 *volL, s16 *volR) {
    SsScore *seqs = _ss_score[(u8)seqSepNo];
    s16 *cur = (s16 *)&D_8008EA22;

    *cur = (s16)seqSepNo;
    *volL = seqs[(seqSepNo & 0xFF00) >> 8].unk74;
    *volR = seqs[(seqSepNo & 0xFF00) >> 8].unk76;
    return *cur;
}

s32 SpuVmGetSeqLVol(s32 seqSepNo) {
    s32 access = seqSepNo & 0xFF;
    SsScore *seqs = _ss_score[access];
    s32 seq = (seqSepNo & 0xFF00) >> 8;

    /* MATCHING: keeps the D_8008EA22 store after the index arithmetic;
     * without it GCC hoists the store to the top. */
    __asm__("");
    D_8008EA22 = access;
    /* MATCHING: s16, for retail's sign-extending lh of the u16 field. */
    return (s16)seqs[seq].unk74;
}

s32 SpuVmGetSeqRVol(s32 seqSepNo) {
    SsScore *seqs = _ss_score[(u8)seqSepNo];

    /* MATCHING: keeps the _ss_score load above the D_8008EA22 store;
     * without it the load sinks below the store and the index arithmetic. */
    __asm__("");
    D_8008EA22 = seqSepNo;
    /* MATCHING: s16, for retail's sign-extending lh of the u16 field. */
    return (s16)seqs[(seqSepNo & 0xFF00) >> 8].unk76;
}

INCLUDE_ASM("asm/nonmatchings/psyq/libsnd_vmanager", SpuVmSeqKeyOff);
