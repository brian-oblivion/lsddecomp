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

/* Matched round 73 -- docs/match-reports/ServiceSoundCueSet.md.
 * Round 75 (naming): `self`/`set` confirmed the same objects
 * `code_179d8_e.c` already names `VabStreamObj`/`SoundCueSet` -- the +0x80/
 * +0x84/+0x9C slots this function dispatches line up exactly with
 * `tools/classtable.py gVabStreamObjMethods`' `VabStreamObj__PlayTone`/
 * `VabStreamObj__StopVoice`/`VabStreamObj__SetPitchOffset`, and
 * `SoundCueSlot.index`/`SoundCueSet.tag`/`.owner`/`.slots` match this
 * function's own field usage (the `>= 0`-gated stop-voice call, the `> 0`
 * tag guard, `callback`'s first argument). VabStreamObj comes from
 * include/VabStreamObj.h since round 87 (track 4). SoundCueSet below is
 * still a second LOCAL view of the struct `code_179d8_e.c` defines: it is
 * not a class, and track 4 did not unify it (see FlushSoundCueSet.md /
 * DreamSys__SetSoundObj.md for the cross-unit identification trail). */
typedef struct {
    s32 index; /* matches code_179d8_e.c's SoundCueSlot.index: -1 sentinel, else a VabStreamObj__StopVoice-forwardable voice index */
    s32 note; /* packed as note*16 into VabStreamObj__PlayTone's `index` argument (hi=note, lo=0) */
    s32 pitchOffset; /* forwarded to VabStreamObj__SetPitchOffset unchanged */
    s32 word2; /* default 0x7F (127); feeds PlayTone's arg2 via a `/unk14*unk10` remainder -- proposed vol/pan, unconfirmed */
    s32 word3; /* default 0x40 (64); feeds PlayTone's arg3 the same way -- proposed vol/pan, unconfirmed */
} SoundCueSlot;

typedef struct SoundCueSet SoundCueSet;

struct SoundCueSet {
    s32 tag;  /* matches code_179d8_e.c's SoundCueSet.tag: guard, >0 required to service */
    s32 unk4; /* incremented once per service call here; code_179d8_e.c's own view never reads it */
    s32 owner; /* matches code_179d8_e.c's SoundCueSet.owner: passed as callback's first argument, unchanged */
    void (*callback)(s32 arg0, SoundCueSet *self);
    s32 unk10; /* set 0 before the callback runs; callback may set it negative to skip servicing this tick -- purpose beyond that not established */
    s32 unk14; /* matches code_179d8_e.c's SoundCueSet.unk14 (set to 10 by InitSoundCueSet); used here as a divisor */
    SoundCueSlot slots[3];
};

void ServiceSoundCueSet(VabStreamObj *a0, SoundCueSet *a1) {
    s32 i;
    SoundCueSlot *e;
    s32 rem1;
    s32 rem2;
    s32 note;

    if (a1->tag > 0) {
        i = 0;
        e = &a1->slots[0];
        do {
            i++;
            e->note = -1;
            e->pitchOffset = 0;
            e->word2 = 0x7F;
            e->word3 = 0x40;
            e++;
        } while (i < 3);

        a1->unk10 = 0;
        if (a1->callback != NULL) {
            a1->callback(a1->owner, a1);
        }

        if (a1->unk10 >= 0) {
            e = &a1->slots[0];
            i = 0;
            do {
                if (e->note >= 0) {
                    if (e->index >= 0) {
                        a0->methods->stopVoice(a0, e->index);
                    }
                    a0->methods->setPitchOffset(a0, e->pitchOffset);
                    note = e->note * 16;
                    rem1 = e->word2 - (e->word2 / a1->unk14) * a1->unk10;
                    rem2 = e->word3 - (e->word3 / a1->unk14) * a1->unk10;
                    e->index = a0->methods->playTone(a0, note, rem1, rem2);
                } else if (e->note == -2 && e->index >= 0) {
                    a0->methods->stopVoice(a0, e->index);
                }
                i++;
                e++;
            } while (i < 3);
        }
        a1->unk4++;
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
