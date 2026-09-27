/*
 * code_179d8_l -- the front of libsnd's voice manager (Sony's
 * `libsnd/vmanager`), plus one game function ahead of it.
 *
 * ServiceSoundCueSet (game code) runs one tick of a SoundCueSet
 * (include/SoundCueSet.h): it resets the set's three cue slots, lets the
 * set's callback fill them, then stops, retunes and replays each slot's tone
 * through the VabStreamObj's method table (include/VabStreamObj.h), scaled
 * down by the set's attenuation.
 *
 * The rest is Sony's, under Sony's names: picking a voice to steal
 * (SpuVmAlloc), keying a tone on (SpuVmKeyOnNow, SpuVmDoAllocate), switching
 * a voice to the noise generator (vmNoiseOn, vmNoiseOn2), turning a note
 * into an SPU pitch (note2pitch, note2pitch2, SePitchBend) and starting a
 * volume ramp (SeAutoVol). They read the current VAB through <libsnd.h>'s
 * VabHdr and VagAtr (_svm_vh, _svm_tn), the voice tables in
 * include/SvmData.h and the sequence records in include/SsScore.h.
 * SsUtVibrateOn and SsUtVibrateOff are empty and have no known caller.
 */
#include "common.h"
#include <libsnd.h>
#include <libspu.h>
#include "SsScore.h"
#include "SvmData.h"
#include "VabStreamObj.h"
#include "SoundCueSet.h"

/* Tones per VAB program: playTone's index is program * 16 + tone
 * (token-identical to code_179d8_e.c's, which owns PlayTone). */
#define VAB_TONES_PER_PROG 16

void ServiceSoundCueSet(VabStreamObj *sound, SoundCueSet *set) {
    s32 i;
    SoundCueSlot *slot;
    s32 vol;
    s32 endVol;
    s32 toneIndex;

    if (set->tag > 0) {
        i = 0;
        slot = &set->slots[0];
        do {
            i++;
            slot->program = SOUND_CUE_NONE;
            slot->octave = 0;
            slot->vol = SOUND_CUE_DEFAULT_VOL;
            slot->endVol = SOUND_CUE_DEFAULT_END_VOL;
            slot++;
        } while (i < ARRAY_COUNT(set->slots));

        set->attenuation = 0;
        if (set->callback != NULL) {
            set->callback(set->owner, set);
        }

        if (set->attenuation >= 0) {
            slot = &set->slots[0];
            i = 0;
            do {
                if (slot->program >= 0) {
                    if (slot->voice >= 0) {
                        sound->methods->stopVoice(sound, slot->voice);
                    }
                    sound->methods->setPitchOffset(sound, slot->octave);
                    toneIndex = slot->program * VAB_TONES_PER_PROG;
                    vol = slot->vol - (slot->vol / set->attenuationSteps) * set->attenuation;
                    endVol = slot->endVol - (slot->endVol / set->attenuationSteps) * set->attenuation;
                    slot->voice = sound->methods->playTone(sound, toneIndex, vol, endVol);
                } else if (slot->program == SOUND_CUE_STOP && slot->voice >= 0) {
                    sound->methods->stopVoice(sound, slot->voice);
                }
                i++;
                slot++;
            } while (i < ARRAY_COUNT(set->slots));
        }
        set->tick++;
    }
}


#ifdef NON_MATCHING
/* NON_MATCHING: 167/167 words, length exact (74/167 raw word-match; funcdiff
 * insertions/deletions 16/16). Residue: a systematic register rotation
 * (t3/t0/a2/a3 family) running through nearly the whole function, visible
 * from the very first instruction (docs/match-reports/SpuVmAlloc.md).
 * Hand-derived. */
extern u8 spuVmMaxVoice;
extern u8 D_8008EA1B;

s32 SpuVmAlloc(void) {
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
INCLUDE_ASM("asm/nonmatchings/code_179d8_l", SpuVmAlloc);
#endif

extern u8 D_8008EA16;
extern u8 D_8008EA19;
extern u8 D_8008EA17;
extern u8 D_8008EA11;
extern u8 D_8008EA1A;
extern s16 _svm_stereo_mono;
extern u16 D_8008EA22;
extern u8 D_8008EA20;
extern u16 D_8008E228;
extern u16 _svm_okon2;
extern u16 _svm_okof1;
extern u16 _svm_okof2;
extern u16 _svm_orev1;
extern u16 _svm_orev2;

/* STALL -- see docs/match-reports/SpuVmKeyOnNow.md. Best body reached
 * (332/316 built words, 16 words LONG; 33/316 raw word-match, drift-
 * affected) preserved there in #if 0. */
#ifdef NON_MATCHING
/* NON_MATCHING: 316/316 words, length exact (201/316 raw word-match; funcdiff
 * insertions/deletions 46/46). Residue: frame SIZE only -- addiu sp,sp,-8
 * against retail's -0x10 -- plus this unit's documented register-identity
 * class (docs/match-reports/SpuVmKeyOnNow.md). Hand-derived, plus one
 * permuter hoist (round 65: pan1sq/16383 computed before pan2sq), reviewed
 * as a pure reordering and kept. */
/* libsnd's _svm_vh (pinned at this address): the header of the VAB bank
 * the voice manager is playing from. */
extern VabHdr *_svm_vh;

extern u8 D_8008EA10;
/* NOT volatile, and declared as an incomplete ARRAY on purpose: the array
 * spelling is what makes GCC 2.6.3 materialise the address once into a GPR
 * and spend one word per read, which is retail. Retail's five reloads come
 * from ordinary CSE invalidation by the stores between them. */
extern s16 D_8008EA26[];

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

    sregIndex = D_8008EA26[0] * 8;

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

    _svm_sreg_dirty[D_8008EA26[0]] |= 7;
    _svm_voice[D_8008EA26[0]].unk04 = (s16)pitch;
    _svm_voice[D_8008EA26[0]].unk1B = 1;

    if (D_8008EA26[0] < 16) {
        lowBit = 1 << D_8008EA26[0];
        highBit = 0;
    } else {
        lowBit = 0;
        highBit = 1 << (D_8008EA26[0] - 16);
    }

    if (D_8008EA20 & 4) {
        _svm_orev1 = lowBit | _svm_orev1;
        _svm_orev2 = highBit | _svm_orev2;
    } else {
        _svm_orev1 = _svm_orev1 & ~lowBit;
        _svm_orev2 = _svm_orev2 & ~highBit;
    }

    D_8008E228 = lowBit | D_8008E228;
    _svm_okon2 = highBit | _svm_okon2;
    _svm_okof1 = _svm_okof1 & ~D_8008E228;
    _svm_okof2 = _svm_okof2 & ~_svm_okon2;
}
#else
INCLUDE_ASM("asm/nonmatchings/code_179d8_l", SpuVmKeyOnNow);
#endif

extern u8 D_8008EA13;
extern u8 D_8008EA18;

/* libsnd's _svm_tn (pinned at this address): the current VAB's tone
 * attributes, 16 per program; SpuVmDoAllocate, note2pitch2 and SePitchBend
 * index it by program * 16 + tone. */
extern VagAtr *_svm_tn;

INCLUDE_ASM("asm/nonmatchings/code_179d8_l", SpuVmDoAllocate);

/* The blend-cascade globals
 * (D_8008EA16/17/19/1A/11/20/22, _svm_stereo_mono, D_8008E228/22C, _svm_okof1/64,
 * _svm_orev1/234, _svm_sreg_dirty/98C/9A3) are already declared above, before
 * SpuVmKeyOnNow (ROM-earlier, same shapes) -- reused here, not redeclared. */
extern u8 D_8008EA0E;
extern u8 D_8008EA1C;
extern SpuRegs *_svm_sreg;
extern u8 spuVmMaxVoice;

/* STALL -- see docs/match-reports/vmNoiseOn.md. Best body reached
 * (309/311 built words, 2 words SHORT) preserved there in #if 0. */
INCLUDE_ASM("asm/nonmatchings/code_179d8_l", vmNoiseOn);

#ifdef NON_MATCHING
/* NON_MATCHING: 107/112 words, 5 words short. Residue: the a0/a3 role-swap
 * register-identity class (this unit's documented class) plus an 8-byte
 * frame retail allocates that this shape doesn't reach
 * (docs/match-reports/vmNoiseOn2.md). Hand-derived. The byte-shaped
 * body's order-only __asm__("") barrier is omitted here; it is in the report. */

void vmNoiseOn2(s32 voice, s32 volL, s32 volR) {
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
    D_8008E228 = lowBit | D_8008E228;
    _svm_okon2 = highBit | _svm_okon2;
    _svm_okof1 = _svm_okof1 & ~D_8008E228;
    _svm_okof2 = _svm_okof2 & ~_svm_okon2;
    _svm_sreg->noiseOn[0] = lowBit;
    _svm_sreg->noiseOn[1] = highBit;
}
#else
INCLUDE_ASM("asm/nonmatchings/code_179d8_l", vmNoiseOn2);
#endif

extern u8 D_8008EA0E;
extern u8 D_8008EA1C;
extern u8 D_8008EA1D;
extern u16 D_8006DAD8[];

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

extern u8 D_8008EA13;
extern u8 D_8008EA18;

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

/* Matched round 73 -- docs/match-reports/SePitchBend.md. */
extern s16 D_8008EA26[];

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
        D_8008EA26[0] = (u8)chan;
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

/* Matched round 73 -- docs/match-reports/SeAutoVol.md. Same body as
 * SeAutoPan (code_179d8_m), over _svm_voice +0x1C..+0x26 instead of
 * +0x28..+0x32. */

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
