/*
 * code_179d8_l -- the SPU sound-effect voice manager (Sony's `libsnd/
 * vmanager`): voice-steal allocation (`SpuVmAlloc`), key-on setup
 * (`SpuVmKeyOnNow`, `SpuVmDoAllocate`), noise-voice setup (`vmNoiseOn`,
 * `vmNoiseOn2`), note/pitch conversion (`note2pitch`, `note2pitch2`,
 * `SePitchBend`) and volume fade-in/out (`SeAutoVol`). 9 of these 12
 * functions were identified round 74 (track 2) as Sony's own `libsnd/
 * vmanager` source by shape/fingerprint match against the disc-3.3 SDK --
 * they carry Sony's real names, not game-style guesses, and this unit
 * builds them as game C only because retail's copy diverges from the SDK
 * reference build (see each function's own `## Naming` section). The
 * exception, `ServiceSoundCueSet`, dispatches through a `VabStreamObj`
 * (the SPU/VAB streaming backend, `code_179d8_e.c`) via its
 * `gVabStreamObjMethods` vtable and is this unit's own game-style name,
 * confirmed tier A against `DreamSys`'s `cueServiceActive` field.
 * `func_8002E2F8`/`func_8002E300` are two unreferenced two-word stubs
 * (splat-emitted empty bodies; no known caller or table attachment).
 *
 * `code_179d8_l`/`code_179d8_m` split what was one uncarved 24-function
 * remainder (carved round 24, 2026-09-08); the split is a staffing cut,
 * not a density one. Owns no jump table and no rodata attach (checked at
 * carve time). See `docs/PROGRESS.md` and individual match reports for
 * carve/blocker history -- all four toolchain blockers this unit once
 * screened against are RESOLVED project-wide (CLAUDE.md).
 */
#include "common.h"
#include "SvmData.h"
#include "VabStreamObj.h"
#include "SoundCueSet.h"

void ServiceSoundCueSet(VabStreamObj *sound, SoundCueSet *set) {
    s32 i;
    SoundCueSlot *e;
    s32 vol;
    s32 endVol;
    s32 toneIndex;

    if (set->tag > 0) {
        i = 0;
        e = &set->slots[0];
        do {
            i++;
            e->program = -1;
            e->octave = 0;
            e->vol = 0x7F;
            e->endVol = 0x40;
            e++;
        } while (i < 3);

        set->attenuation = 0;
        if (set->callback != NULL) {
            set->callback(set->owner, set);
        }

        if (set->attenuation >= 0) {
            e = &set->slots[0];
            i = 0;
            do {
                if (e->program >= 0) {
                    if (e->voice >= 0) {
                        sound->methods->stopVoice(sound, e->voice);
                    }
                    sound->methods->setPitchOffset(sound, e->octave);
                    toneIndex = e->program * 16;
                    vol = e->vol - (e->vol / set->attenuationSteps) * set->attenuation;
                    endVol = e->endVol - (e->endVol / set->attenuationSteps) * set->attenuation;
                    e->voice = sound->methods->playTone(sound, toneIndex, vol, endVol);
                } else if (e->program == -2 && e->voice >= 0) {
                    sound->methods->stopVoice(sound, e->voice);
                }
                i++;
                e++;
            } while (i < 3);
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
extern u8 D_8008E9D0;
extern u8 D_8008EA1B;
extern void SpuSetNoiseVoice(s32 a0, s32 a1);

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

    chosen = 0x63;
    bestSec = 0xFFFF;
    found = 0;
    bestTer = 0;
    bestIdx = 0x63;
    threshold = D_8008EA1B;

    for (idx = 0; (u8)idx < D_8008E9D0; idx++) {
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

    if ((u8)chosen == 0x63) {
        if ((u8)found != 0) {
            chosen = bestIdx;
        } else {
            chosen = D_8008E9D0;
        }
    }

    count = D_8008E9D0;
    if ((u8)chosen < count) {
        if (count != 0) {
            for (idx = 0; (u8)idx < count; idx++) {
                _svm_voice[(u8)idx].unk02 = _svm_voice[(u8)idx].unk02 + 1;
            }
        }
        _svm_voice[(u8)chosen].unk02 = 0;
        _svm_voice[(u8)chosen].unk18 = D_8008EA1B;
        if (_svm_voice[(u8)chosen].unk1B == 2) {
            SpuSetNoiseVoice(0, 0xFFFFFF);
        }
    }
    return (u8)chosen;
}
#else
INCLUDE_ASM("asm/nonmatchings/code_179d8_l", SpuVmAlloc);
#endif

/* Shared with vmNoiseOn below (same two-level entry table, same
 * blend-cascade shape); declared once here since SpuVmKeyOnNow is
 * ROM-earlier, reused there rather than redeclared. */
typedef struct {
    u8 pad0[0x74];
    u16 unk74;
    u16 unk76;
    u8 pad78[0xAC - 0x78];
} D800902E8Entry;

extern D800902E8Entry *D_800902E8[];

extern u8 D_8008EA16;
extern u8 D_8008EA19;
extern u8 D_8008EA17;
extern u8 D_8008EA11;
extern u8 D_8008EA1A;
extern s16 D_8008E8C0;
extern u16 D_8008EA22;
extern u8 D_8008EA20;
extern u16 D_8008E228;
extern u16 D_8008E22C;
extern u16 D_80090C60;
extern u16 D_80090C64;
extern u16 D_8008E230;
extern u16 D_8008E234;

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
/* Object holding a per-note "priority"-ish scale byte at +0x18; only field
 * this function needs. */
typedef struct {
    u8 pad0[0x18];
    u8 unk18; /* +0x18 */
} ObjE970;

extern ObjE970 *D_8008E970;

extern u8 D_8008EA10;
/* NOT volatile, and declared as an incomplete ARRAY on purpose: the array
 * spelling is what makes GCC 2.6.3 materialise the address once into a GPR
 * and spend one word per read, which is retail. Retail's five reloads come
 * from ordinary CSE invalidation by the stores between them. */
extern s16 D_8008EA26[];

void SpuVmKeyOnNow(s32 a0, s32 a1) {
    D800902E8Entry *e;
    s32 prio;
    s32 lvl0;
    u32 lvl1;
    u32 pan1;
    u32 pan2;
    u32 pan1sq;
    u32 pan2sq;
    s16 pan1out;
    s32 chanIdx;
    s32 lowBit;
    s32 highBit;

    prio = D_8008E970->unk18 * 0x3FFF;
    lvl0 = D_8008EA10 * prio / 16129;
    lvl1 = (u32)lvl0 * D_8008EA16 * D_8008EA19 / 16129;

    chanIdx = D_8008EA26[0] * 8;

    e = &D_800902E8[D_8008EA22 & 0xFF][D_8008EA22 >> 8];
    pan1 = lvl1;
    pan2 = lvl1;
    if ((s16)D_8008EA22 != 0x21) {
        pan1 = lvl1 * e->unk74 / 127;
        pan2 = lvl1 * e->unk76 / 127;
    }

    if ((u8)D_8008EA1A < 0x40) {
        pan2 = (pan2 * D_8008EA1A) / 63;
    } else {
        pan1 = (pan1 * (0x7F - D_8008EA1A)) / 63;
    }

    if ((u8)D_8008EA17 < 0x40) {
        pan2 = (pan2 * D_8008EA17) / 63;
    } else {
        pan1 = (pan1 * (0x7F - D_8008EA17)) / 63;
    }

    if ((u8)D_8008EA11 < 0x40) {
        pan2 = (D_8008EA11 * pan2) / 63;
    } else {
        pan1 = (pan1 * (0x7F - D_8008EA11)) / 63;
    }

    if (D_8008E8C0 == 1) {
        if (pan1 < pan2) {
            pan1 = pan2;
        } else {
            pan2 = pan1;
        }
    }
    pan1sq = pan1 * pan1;
    pan1out = (s16)(pan1sq / 16383);
    pan2sq = pan2 * pan2;

    ((s16 *)_svm_sreg_buf)[(u16)chanIdx + 2] = (s16)a1;
    ((s16 *)_svm_sreg_buf)[(u16)chanIdx] = pan1out;
    ((s16 *)_svm_sreg_buf)[(u16)chanIdx + 1] = (s16)(pan2sq / 16383);

    _svm_sreg_dirty[D_8008EA26[0]] |= 7;
    _svm_voice[D_8008EA26[0]].unk04 = (s16)a1;
    _svm_voice[D_8008EA26[0]].unk1B = 1;

    if (D_8008EA26[0] < 0x10) {
        lowBit = 1 << D_8008EA26[0];
        highBit = 0;
    } else {
        lowBit = 0;
        highBit = 1 << (D_8008EA26[0] - 0x10);
    }

    if (D_8008EA20 & 4) {
        D_8008E230 = lowBit | D_8008E230;
        D_8008E234 = highBit | D_8008E234;
    } else {
        D_8008E230 = D_8008E230 & ~lowBit;
        D_8008E234 = D_8008E234 & ~highBit;
    }

    D_8008E228 = lowBit | D_8008E228;
    D_8008E22C = highBit | D_8008E22C;
    D_80090C60 = D_80090C60 & ~D_8008E228;
    D_80090C64 = D_80090C64 & ~D_8008E22C;
}
#else
INCLUDE_ASM("asm/nonmatchings/code_179d8_l", SpuVmKeyOnNow);
#endif

extern u8 D_8008EA13;
extern u8 D_8008EA18;

/* Shared with note2pitch2 below (same base pointer, same table); this
 * function needs the +0x10/+0x12 halfwords too, so the struct is declared
 * once here (ROM-address order: SpuVmDoAllocate precedes note2pitch2) and
 * reused there rather than redeclared -- see docs/match-reports/SpuVmDoAllocate.md. */
typedef struct {
    u8 unk0[4];
    u8 unk4;
    u8 unk5;
    u8 unk6[6];
    u8 unk12;
    u8 unk13;
    u8 pad14[2];
    u16 unk16; /* +0x10 */
    u16 unk18; /* +0x12 */
    u8 pad20[0x20 - 20];
} D8008E978Entry;

extern D8008E978Entry *D_8008E978;

INCLUDE_ASM("asm/nonmatchings/code_179d8_l", SpuVmDoAllocate);

/* D800902E8Entry, D_800902E8 and the blend-cascade globals
 * (D_8008EA16/17/19/1A/11/20/22, D_8008E8C0, D_8008E228/22C, D_80090C60/64,
 * D_8008E230/234, _svm_sreg_dirty/98C/9A3) are already declared above, before
 * SpuVmKeyOnNow (ROM-earlier, same shapes) -- reused here, not redeclared. */
extern u8 D_8008EA0E;
extern u8 D_8008EA1C;
extern u16 *D_8006DAD4;
extern u8 D_8008E9D0;

/* STALL -- see docs/match-reports/vmNoiseOn.md. Best body reached
 * (309/311 built words, 2 words SHORT) preserved there in #if 0. */
INCLUDE_ASM("asm/nonmatchings/code_179d8_l", vmNoiseOn);

#ifdef NON_MATCHING
/* NON_MATCHING: 107/112 words, 5 words short. Residue: the a0/a3 role-swap
 * register-identity class (this unit's documented class) plus an 8-byte
 * frame retail allocates that this shape doesn't reach
 * (docs/match-reports/vmNoiseOn2.md). Hand-derived. The byte-shaped
 * body's order-only __asm__("") barrier is omitted here; it is in the report. */

void vmNoiseOn2(s32 a0, s32 a1, s32 a2) {
    s32 a3;
    s32 v1;
    s32 lowBit;
    s32 highBit;
    s32 i;
    s32 n;

    a3 = a0;
    a0 = (u8)a0;
    _svm_sreg_buf[a0].unk2 = a2;
    v1 = _svm_sreg_dirty[a0];
    _svm_sreg_buf[a0].unk0 = a1;
    v1 |= 3;
    _svm_sreg_dirty[a0] = v1;
    if ((u32)a0 < 16) {
        lowBit = 1 << a0;
        highBit = 0;
    } else {
        lowBit = 0;
        highBit = 1 << (a0 - 16);
    }

    n = D_8008E9D0;
    _svm_voice[(u8)a3].unk04 = 10;
    if (n != 0) {
        i = 0;
        do {
            _svm_voice[(u16)i].unk1B = _svm_voice[(u16)i].unk1B & 1;
            i++;
        } while ((u16)i < D_8008E9D0);
    }
    _svm_voice[(u8)a3].unk1B = 2;

    _svm_voice[(u8)a3].unk02 = 0;
    D_8008E228 = lowBit | D_8008E228;
    D_8008E22C = highBit | D_8008E22C;
    D_80090C60 = D_80090C60 & ~D_8008E228;
    D_80090C64 = D_80090C64 & ~D_8008E22C;
    D_8006DAD4[0xCA] = lowBit;
    D_8006DAD4[0xCB] = highBit;
}
#else
INCLUDE_ASM("asm/nonmatchings/code_179d8_l", vmNoiseOn2);
#endif

extern u8 D_8008EA0E;
extern u8 D_8008EA1C;
extern u8 D_8008EA1D;
extern u16 D_8006DAD8[];

s32 note2pitch(void) {
    s32 a0;
    s32 q12;
    s16 rem12;
    u8 a2;
    u16 v1;

    a0 = (s16)(D_8008EA0E + 0x3C - D_8008EA1C);
    q12 = a0 / 12;
    a2 = D_8008EA1D >> 3;
    rem12 = a0 - q12 * 12;
    if (a2 >= 16) {
        a2 = 15;
    }
    v1 = D_8006DAD8[a2 + rem12 * 16];
    if ((s16)(q12 - 5) > 0) {
        v1 <<= (s16)(q12 - 5);
    } else if ((s16)(q12 - 5) < 0) {
        v1 = (u16)v1 >> -(s16)(q12 - 5);
    }
    return v1;
}

extern u8 D_8008EA13;
extern u8 D_8008EA18;

s32 note2pitch2(s32 a0, s32 a1) {
    s32 origA0;
    s32 idx;
    s32 tblIdx;
    D8008E978Entry *e;
    s32 v0;
    s32 div8;
    u8 a2;
    s16 a3;
    s32 diff;
    s32 q12;
    s16 rem12;
    u16 v1;

    origA0 = a0;
    idx = D_8008EA18 + (D_8008EA13 << 4);
    e = &D_8008E978[idx];
    v0 = (u16)a1 + e->unk5;
    div8 = v0 / 8;
    a3 = div8;
    a2 = 0;
    if (div8 >= 16) {
        a2 = 1;
        a3 = div8 - 16;
    }
    diff = (s16)(a2 + (origA0 + 0x3C - e->unk4));
    q12 = diff / 12;
    rem12 = diff - q12 * 12;
    tblIdx = rem12 * 16;
    tblIdx = tblIdx + a3;
    v1 = D_8006DAD8[tblIdx];
    if ((s16)(q12 - 5) > 0) {
        v1 <<= (s16)(q12 - 5);
    } else if ((s16)(q12 - 5) < 0) {
        v1 = (u16)v1 >> -(s16)(q12 - 5);
    }
    return v1;
}

/* Matched round 73 -- docs/match-reports/SePitchBend.md. */
extern s16 D_8008EA26[];

void SePitchBend(s32 chan, s32 bend) {
    s32 off;
    s32 prod;
    s32 q;
    s32 note;
    s32 fine;
    s16 b;
    s32 idx;
    u8 *p;

    off = (chan & 0xFF) * 8;
    if ((u32)(chan & 0xFF) < 24) {
        p = &D_8008EA13;
        *p = (u8)_svm_voice[(chan & 0xFF)].unk10;
        D_8008EA18 = (u8)_svm_voice[(chan & 0xFF)].unk14;
        D_8008EA26[0] = (u8)chan;
        idx = D_8008EA18 + (*p << 4);
        b = bend;
        if (b >= 0) {
            prod = b * D_8008E978[idx].unk13;
            note = (u16)_svm_voice[(chan & 0xFF)].unk0C + prod / 127;
            fine = prod % 127;
        } else {
            q = (b * D_8008E978[idx].unk12) / 127;
            note = (u16)_svm_voice[(chan & 0xFF)].unk0C + q - 1;
            fine = q + 127;
        }
        ((u16 *)_svm_sreg_buf)[off + 2] = note2pitch2((u16)note, (u16)fine);
        _svm_sreg_dirty[(chan & 0xFF)] |= 4;
    }
}

void func_8002E2F8(void) {}

void func_8002E300(void) {}

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
