/*
 * code_179d8_m -- part of the game's SPU sound driver: the per-voice
 * envelope/fade stepper, the per-tick voice updater, and a NoteOn/NoteOff
 * pair for a 24-voice (0..0x17) PS1 SPU wavetable player driven by MIDI-
 * shaped events (confirmed: code_179d8_k.c's caller switches on a status
 * byte with the MIDI 0x90/0xB0/0xC0/0xE0/0xFF nibbles; D_8006DAD4 is the
 * PS1 SPU's real hardware base, 0x1F801C00, per func_8002DDBC's report in
 * code_179d8_l). Plain free functions, no vtable -- `tools/classtable.py`
 * lists no method table at these addresses.
 *
 * Twelve functions, functions 161..172 of the original 274-function
 * code_179d8 monolith, 0x1ECD8..0x20ADC (vram 0x8002E4D8..0x800302DC).
 * Carved round 24 (2026-09-08); `code_179d8_l` is the front half (owns the
 * shared carve-time census) and `code_179d8_j` follows behind. No jump
 * table, no rodata attach, no BIOS trampoline (every function ends in its
 * own `jr $ra`, none open on `$sp`) -- see code_179d8_l's header for the
 * full carve-time survey.
 *
 * WHAT EACH FUNCTION DOES (see docs/match-reports/<name>.md for the full
 * derivation and evidence):
 *   - StartNote / StopNote: a matched NoteOn/NoteOff pair. Given a packed
 *     [screen|slot] identity, note, volume/program and (for StartNote) a
 *     velocity and a computed stereo pan split, StartNote registers a new
 *     active-voice record; StopNote scans every voice for one whose
 *     identity fields match and releases it, returning the count released.
 *   - PlaySound / PlayFixedSound: find a free voice (func_8002CF18, in
 *     code_179d8_l) and, if one exists, key it on (func_8002DDBC, also
 *     code_179d8_l) with the caller's parameters or, for PlayFixedSound,
 *     two hardcoded constants.
 *   - BeginVoiceFade / StepVoiceFade: a linear-ramp pair over the
 *     gVoiceFade* per-voice arrays -- Begin sets a start/target/step-rate;
 *     Step advances the accumulator (throttled by an interval/countdown
 *     pair), clamps at the target, and writes the resulting stereo output
 *     level.
 *   - StepVoiceEnvelope: the same accumulate-until-limit shape over its
 *     own gVoiceEnv* family, but with no "Begin" counterpart in this
 *     unit -- whatever sets gVoiceEnvActive/gVoiceEnvStep/gVoiceEnvLimit
 *     is still undecompiled elsewhere. func_8002E308 in code_179d8_l opens
 *     with the identical prologue/argument-narrowing shape and is worth
 *     checking as that counterpart.
 *   - UpdateVoiceEnvelopes: the per-tick dispatcher. Maintains a 16-slot
 *     ring buffer of per-tick voice-activity bitmasks; when a voice has
 *     shown no activity for 16 consecutive ticks it force-releases it
 *     (disabling the SPU noise generator if that voice was in noise
 *     state); then calls StepVoiceEnvelope/StepVoiceFade for every voice
 *     whose respective flag is set. Called once at the end of
 *     InitSpuDriver and, going by its own ring-buffer/mask-clearing logic,
 *     meant to run every frame thereafter.
 *   - ClearNoiseVoices: releases every voice whose state byte reads
 *     exactly 2 (the same value StopNote/UpdateVoiceEnvelopes/func_8002CF18
 *     treat as "noise voice needing SpuSetNoiseVoice/func_800375E8 cleanup").
 *   - ApplyVoicePitchBend / ApplyPitchBendToAllVoices: match a voice by
 *     identity and apply a curve-table-driven pitch bend from a 0-127
 *     depth value centered at 0x40, writing the result through
 *     func_8002E038; the "AllVoices" wrapper calls Sony's SpuVmVSetUp once
 *     and then runs this over every voice, returning the count affected.
 *   - InitSpuDriver: the SPU driver's init call -- _spu_setInTransfer,
 *     SpuInitMalloc, zeroes every per-voice table and the two master
 *     volume globals (reset to 0x3FFF, the SPU's real max), then calls
 *     UpdateVoiceEnvelopes once.
 *
 * STALLS: ApplyVoicePitchBend, StartNote, UpdateVoiceEnvelopes,
 * StepVoiceEnvelope, StepVoiceFade -- all five are the same "whole-function
 * register-count decision predates any of the function's own locals"
 * class CLAUDE.md treats as banned-to-fix-by-pinning; see each report.
 */
#include "common.h"

/* Round 48 (echo): testing charlie's func_800351D0 frame-padding lever on
 * this function's frame gap (0x10 built vs retail's 0x18, 8 bytes; retail
 * saves ZERO callee-saved registers and addresses NOTHING via $sp beyond
 * the prologue/epilogue immediate itself, confirmed via grep -- textbook
 * pure-padding shape). See docs/match-reports/StepVoiceEnvelope.md for the
 * full derivation this body is otherwise unchanged from.
 *
 * This function sits FIRST in ROM order in this unit, so the shared
 * record-family types its sibling stalls also use (Rec34Half, Rec34HalfU,
 * Rec16D7F0, ObjE970, the volume/pan scratch bytes, D_8008E8C0) are
 * defined HERE instead of duplicated -- their old definitions further
 * down this file (originally written for StepVoiceFade's isolated splice)
 * are removed; the plain externs that used to accompany them there are
 * left in place and now just reference these same, earlier-defined types
 * (a harmless duplicate extern declaration, not a redefinition). Same
 * "move the shared prelude up, do not duplicate" fix round 37 already
 * used here; declaration order carries no code. */
typedef struct {
    s16 unk0; /* +0x0 */
    u8 pad2[0x34 - 0x2];
} Rec34Half;
extern Rec34Half gVoiceEnvActive[]; /* nonzero while this voice's envelope is still ramping; cleared by StepVoiceEnvelope when it reaches gVoiceEnvLimit */
extern Rec34Half gVoiceEnvStep[]; /* per-tick increment/decrement applied to gVoiceEnvAccum */
extern Rec34Half gVoiceEnvInterval[]; /* ticks between steps (0 = every tick), same throttle idiom as gVoiceFadeInterval below */
extern Rec34Half gVoiceEnvCountdown[]; /* countdown to the next step, reloaded from gVoiceEnvInterval */
extern Rec34Half gVoiceEnvAccum[]; /* running envelope value */
extern Rec34Half gVoiceEnvLimit[]; /* value the envelope clamps to once reached */

typedef struct {
    u16 unk0; /* +0x0 */
    u8 pad2[0x34 - 0x2];
} Rec34HalfU;

typedef struct {
    s16 unk0; /* +0x0 */
    u8 pad2[0x10 - 0x2];
} Rec16D7F0;
extern Rec16D7F0 D_8008D7F0[];
extern Rec16D7F0 D_8008D7F2[];

extern u8 D_8008D970[];

typedef struct {
    u8 pad[0x12];
    u16 difficultyThreshold; /* +0x12 -- compared unsigned against D_8008EA13, per StartNote's report */
    u8 pad14[0x18 - 0x14];
    u8 masterVolume; /* +0x18 -- scaled by 0x3FFF into the stereo-level product in StepVoiceEnvelope/StepVoiceFade */
} ObjE970;
extern ObjE970 *D_8008E970;

extern u8 D_8008EA10;
extern u8 D_8008EA11;
extern u8 D_8008EA16;
extern u8 D_8008EA17;
extern u8 D_8008EA19;
extern u8 D_8008EA1A;

extern s16 D_8008E8C0;

#ifdef NON_MATCHING
/* NON_MATCHING: 237/231 words, 6 words long. Residue: a redundant
 * sign-extension around the incU/incS accumulator add -- confirmed still
 * the real gap after round 48's frame-padding lever realigned the frame
 * byte-exactly without closing the length, and after a permuter search
 * (round 37) plateaued at 2105/4155 across 80,882 iterations with no
 * candidate reaching zero (docs/match-reports/StepVoiceEnvelope.md).
 * Hand-derived. */
void StepVoiceEnvelope(s16 a0)
{
    s16 idxCopy;
    s16 accum;
    u8 flagByte;
    u16 val1;
    u16 val2;
    s32 product1;
    s32 q1;
    s32 q1b;
    u32 q1c;
    u32 q2;
    s32 tmp;
    u8 tableval;
    s16 limit;
    s16 woff;

    /* Halfword-indexed byte-offset for the D_8008D7F2/D_8008D7F0 stores
     * near the end of this function: idx*8 computed here, kept `s16`
     * (matching retail's dependency -- its early `sll #3` reads the
     * sign-extended parameter and stays a plain register value, with no
     * separate unsigned materialization at this point), before the
     * 0x34-stride record accesses even start. Masked to `u16` only at
     * the point of use (indexing an `s16 *`, which then scales by 2),
     * reaching idx*16 -- the real per-channel byte stride for these two
     * 0x10-stride, single-field arrays. Same base idiom as
     * InitSpuDriver's D_8006DAD4 fix, but kept `s16` (not `u16`) until
     * point of use -- see this report's "non-loop" analysis for why the
     * loop-context version of the idiom does not transfer directly. */
    idxCopy = a0;
    woff = idxCopy << 3;

    if (gVoiceEnvInterval[idxCopy].unk0 != 0) {
        s16 orig = gVoiceEnvCountdown[idxCopy].unk0;

        gVoiceEnvCountdown[idxCopy].unk0 = orig - 1;
        if (orig > 0) {
            return;
        }
        gVoiceEnvCountdown[idxCopy].unk0 = gVoiceEnvInterval[idxCopy].unk0;
    }

    {
        u16 accumU = ((Rec34HalfU *) gVoiceEnvAccum)[idxCopy].unk0;
        u16 incU = ((Rec34HalfU *) gVoiceEnvStep)[idxCopy].unk0;
        s16 incS = gVoiceEnvStep[idxCopy].unk0;
        s16 accumS;

        accumU = accumU + incU;
        gVoiceEnvAccum[idxCopy].unk0 = accumU;
        accumS = gVoiceEnvAccum[idxCopy].unk0;

        if (incS > 0) {
            limit = gVoiceEnvLimit[idxCopy].unk0;
            if (accumS >= limit) {
                accumU = limit;
                gVoiceEnvAccum[idxCopy].unk0 = accumU;
                gVoiceEnvActive[idxCopy].unk0 = 0;
            }
        } else if (incS < 0) {
            limit = gVoiceEnvLimit[idxCopy].unk0;
            if (limit >= accumS) {
                accumU = limit;
                gVoiceEnvAccum[idxCopy].unk0 = accumU;
                gVoiceEnvActive[idxCopy].unk0 = 0;
            }
        }
    }

    accum = ((Rec34HalfU *) gVoiceEnvAccum)[idxCopy].unk0;
    D_8008EA10 = (u8) accum;
    tableval = D_8008E970->masterVolume;

    product1 = accum * (tableval * 0x3FFF);
    q1 = product1 / 16129;
    q1b = q1 * D_8008EA16;
    q1c = q1b * D_8008EA19;
    q2 = q1c / 16129u;

    if (D_8008EA1A < 0x40) {
        tmp = q2 * D_8008EA1A;
        val2 = (u32) tmp >> 6;
        val1 = q2;
    } else {
        tmp = q2 * (0x7F - D_8008EA1A);
        val1 = (u32) tmp >> 6;
        val2 = q2;
    }

    if (D_8008EA17 < 0x40) {
        tmp = val2 * D_8008EA17;
        val2 = tmp / 64;
    } else {
        tmp = val1 * (0x7F - D_8008EA17);
        val1 = tmp / 64;
    }

    if (D_8008EA11 < 0x40) {
        tmp = val2 * D_8008EA11;
        val2 = tmp / 64;
    } else {
        tmp = val1 * (0x7F - D_8008EA11);
        val1 = tmp / 64;
    }

    if (D_8008E8C0 == 1) {
        if (val1 < val2) {
            val1 = val2;
        } else {
            val2 = val1;
        }
    }

    ((s16 *) D_8008D7F2)[(u16) woff] = val2;
    flagByte = D_8008D970[idxCopy];
    ((s16 *) D_8008D7F0)[(u16) woff] = val1;
    flagByte |= 3;
    D_8008D970[idxCopy] = flagByte;
}
#else
INCLUDE_ASM("asm/nonmatchings/code_179d8_m", StepVoiceEnvelope);
#endif

/* Same 0x34-stride channel-configuration record family documented in
 * code_179d8_j.c (Rec34D994/Rec34Byte/Rec34Half); this unit keeps its
 * own local view rather than sharing that file's header-less types.
 * Six independent 2-bytes-apart symbols share this one shape, same
 * idiom as code_179d8_j.c's own D_8008D994/D_8008D996/... family. */
extern Rec34Half gVoiceFadeActive[]; /* fade-in-progress flag; set by BeginVoiceFade, cleared by StepVoiceFade when gVoiceFadeAccum reaches gVoiceFadeLimit */
extern Rec34Half gVoiceFadeStep[]; /* per-tick increment/decrement applied to gVoiceFadeAccum */
extern Rec34Half gVoiceFadeInterval[]; /* ticks between steps (0 = every tick); same throttle idiom as gVoiceEnvInterval above */
extern Rec34Half gVoiceFadeCountdown[]; /* countdown to the next step, reloaded from gVoiceFadeInterval */
extern Rec34Half gVoiceFadeAccum[]; /* running interpolated value, initialized to BeginVoiceFade's "from" argument */
extern Rec34Half gVoiceFadeLimit[]; /* target value the fade is moving toward, BeginVoiceFade's "to" argument */

void BeginVoiceFade(s16 a0, s16 a1, s16 a2, s16 a3) {
    s16 q;

    if (a1 == a2) {
        return;
    }
    gVoiceFadeActive[a0].unk0 = 1;
    gVoiceFadeAccum[a0].unk0 = a1;
    gVoiceFadeLimit[a0].unk0 = a2;
    if ((a1 - a2 < 0 ? a2 - a1 : a1 - a2) < a3) {
        q = a3 / (a1 - a2);
        gVoiceFadeStep[a0].unk0 = 1;
        gVoiceFadeInterval[a0].unk0 = q;
        gVoiceFadeCountdown[a0].unk0 = q;
    } else {
        q = (a1 - a2) / a3;
        gVoiceFadeInterval[a0].unk0 = 0;
        gVoiceFadeStep[a0].unk0 = q;
    }
}


#ifdef NON_MATCHING
/* NON_MATCHING: 222/228 words, 6 words short. Residue: a persisted early
 * value ($t1 = idx<<3, held live across the whole function) this C does
 * not reproduce -- confirmed still the real gap after round 48's
 * frame-padding lever realigned the frame byte-exactly without closing
 * the length, and after a permuter search (round 37) plateaued at
 * 2430/4125 with no candidate reaching zero (docs/match-reports/
 * StepVoiceFade.md). Hand-derived. */
void StepVoiceFade(s16 a0)
{
    s16 idxCopy;
    s16 step;
    u16 increment;
    s16 incrementS;
    u16 accum;
    s16 limit;
    u8 accumByte;
    u8 tableval;
    s32 product1;
    s32 q1;
    s32 q1b;
    u32 q1c;
    u32 q2;
    u16 val1;
    u16 val2;
    s32 tmp;
    u8 v0;

    idxCopy = a0;
    step = gVoiceFadeInterval[a0].unk0;
    if (step != 0) {
        u16 current = ((Rec34HalfU *) gVoiceFadeCountdown)[a0].unk0;

        ((Rec34HalfU *) gVoiceFadeCountdown)[a0].unk0 = current - 1;
        if ((s16) current > 0) {
            return;
        }
        ((Rec34HalfU *) gVoiceFadeCountdown)[a0].unk0 = gVoiceFadeInterval[a0].unk0;
    }

    increment = ((Rec34HalfU *) gVoiceFadeStep)[a0].unk0;
    accum = ((Rec34HalfU *) gVoiceFadeAccum)[a0].unk0;
    incrementS = gVoiceFadeStep[a0].unk0;
    accum = accum + increment;
    ((Rec34HalfU *) gVoiceFadeAccum)[a0].unk0 = accum;

    if (incrementS > 0) {
        limit = gVoiceFadeLimit[a0].unk0;
        if ((s16) accum >= limit) {
            accum = limit;
            ((Rec34HalfU *) gVoiceFadeAccum)[a0].unk0 = accum;
            gVoiceFadeActive[a0].unk0 = 0;
        }
    } else if (incrementS < 0) {
        limit = gVoiceFadeLimit[a0].unk0;
        if (limit >= (s16) accum) {
            accum = limit;
            ((Rec34HalfU *) gVoiceFadeAccum)[a0].unk0 = accum;
            gVoiceFadeActive[a0].unk0 = 0;
        }
    }

    accumByte = *(u8 *) &gVoiceFadeAccum[a0].unk0;
    D_8008EA11 = accumByte;
    tableval = D_8008E970->masterVolume;

    product1 = D_8008EA10 * (tableval * 0x3FFF);
    q1 = product1 / 16129;
    q1b = q1 * D_8008EA16;
    q1c = q1b * D_8008EA19;
    q2 = q1c / 16129u;

    if (D_8008EA1A < 0x40) {
        tmp = q2 * D_8008EA1A;
        val2 = (u32) tmp >> 6;
        val1 = q2;
    } else {
        tmp = q2 * (0x7F - D_8008EA1A);
        val1 = (u32) tmp >> 6;
        val2 = q2;
    }

    if (D_8008EA17 < 0x40) {
        tmp = val2 * D_8008EA17;
        val2 = tmp / 64;
    } else {
        tmp = val1 * (0x7F - D_8008EA17);
        val1 = tmp / 64;
    }

    if (accumByte < 0x40) {
        tmp = val2 * accumByte;
        val2 = tmp / 64;
    } else {
        tmp = val1 * (0x7F - accumByte);
        val1 = tmp / 64;
    }

    if (D_8008E8C0 == 1) {
        if (val1 < val2) {
            val1 = val2;
        } else {
            val2 = val1;
        }
    }

    D_8008D7F2[idxCopy].unk0 = val2;
    v0 = D_8008D970[idxCopy];
    D_8008D7F0[idxCopy].unk0 = val1;
    v0 |= 3;
    D_8008D970[idxCopy] = v0;
}
#else
INCLUDE_ASM("asm/nonmatchings/code_179d8_m", StepVoiceFade);
#endif

extern void _spu_setInTransfer(s32 a0);
extern void SpuInitMalloc(s32 a0, void *a1);
extern void UpdateVoiceEnvelopes(void);

extern u8 gSpuMallocArea[];
extern s16 D_8008E9FC;
extern s16 D_8008E84C;
extern s16 gMasterVolL;
extern s16 gMasterVolR;
extern s16 D_8008E230;
extern s16 D_8008E234;
extern s32 D_8008E258;
extern s32 D_8008E25C;
extern u8 gDisableVoiceStarveScan;
extern s16 D_8008E938;

extern Rec34Half D_8008D98A[]; /* value forced to 0x18 at init */

typedef struct {
    s16 unk0; /* +0x0 */
    u8 pad2[0x34 - 0x2];
} Rec34S16Edd4;
extern Rec34S16Edd4 D_8008D988Edd4[] __asm__("D_8008D988");
extern Rec34S16Edd4 D_8008D996Edd4[] __asm__("D_8008D996");
extern Rec34S16Edd4 D_8008D99AEdd4[] __asm__("D_8008D99A");

extern Rec34Half D_8008D990[];
extern Rec34Half D_8008D98C[];
extern Rec34Half D_8008D98E[];
extern Rec34Half D_8008D998[];
extern Rec34Half gVoiceEnvStep[];
extern Rec34Half gVoiceEnvInterval[];
extern Rec34Half gVoiceEnvCountdown[];
extern Rec34Half gVoiceEnvAccum[];

typedef struct {
    u16 unk0; /* +0x0 */
    u8 pad2[0x34 - 0x2];
} Rec34U16Edd4;
extern Rec34U16Edd4 D_8008D99CEdd4[] __asm__("D_8008D99C");

typedef struct {
    u8 unk0; /* +0x0 */
    u8 pad1[0x34 - 0x1];
} Rec34ByteEdd4;
extern Rec34ByteEdd4 D_8008D992[]; /* byte field, forced to 0x40 at init */
extern Rec34ByteEdd4 D_8008D9A3Edd4[] __asm__("D_8008D9A3");
extern Rec34Half gVoiceEnvActive[];

extern volatile u16 D_8008EA26;
extern u8 D_8008E9D0;
extern s16 D_80090BD0;
extern u8 D_8008EA2C[];
extern u16 D_80090C60;
extern u16 D_80090C64;
extern u16 D_8008E228;
extern u16 D_8008E22C;

typedef struct {
    u8 pad[0x194];
    u16 unk194; /* +0x194 */
    u16 unk196; /* +0x196 */
} ObjDAD4Edd4;
extern ObjDAD4Edd4 *D_8006DAD4Edd4 __asm__("D_8006DAD4");

/* MATCHED round 32 (bravo), 270/270 -- closes the register-identity stall
 * every prior round's hand-reshaping (10 axes) and one earlier permuter
 * run (100k+ iterations) could not move.  Permuter-found: a SINGLE scratch
 * variable (`scratch`, `s32`), reused for TWO textually unrelated
 * purposes -- once to break the `a0`-vs-`s0` min-clamp tie, and again
 * ~150 lines later as the `D_8006DAD4[woff]` store's index -- is what
 * reproduces retail's exact register allocation.  Splitting these into
 * two separately-named locals (the natural, more readable choice) gives a
 * WORSE result than either leaving `a0` alone or this single-variable
 * reuse; the reuse itself is load-bearing, not cosmetic.  See
 * docs/match-reports/InitSpuDriver.md for the full derivation, the
 * permuter trace, and why the naive two-variable translation regresses
 * sharply (47/270) despite being semantically identical. */
void InitSpuDriver(s32 a0) {
    s16 i;
    s32 scratch;

    _spu_setInTransfer(0);
    D_8008E9FC = 0;
    D_8008E84C = 0;
    SpuInitMalloc(0x20, gSpuMallocArea);

    for (i = 0; (u16) i < 0xC0; i++) {
        ((u16 *) D_8008D7F0)[(u16) i] = 0;
    }

    for (i = 0; (u16) i < 0x18; i++) {
        D_8008D970[(u16) i] = 0;
    }

    D_80090BD0 = 0;

    for (i = 0; (u16) i < 0x10; i++) {
        D_8008EA2C[(u16) i] = 0;
    }

    a0 = (u8) a0;
    scratch = a0;
    if ((u32) a0 >= 0x18) {
        D_8008E9D0 = 0x18;
    } else {
        D_8008E9D0 = scratch;
    }

    for (i = 0; (u16) i < D_8008E9D0; i++) {
        u16 woff;
        u16 chan;
        u16 lowMask;
        u16 highMask;

        woff = (u16) i * 8;

        D_8008D98A[(u16) i].unk0 = 0x18;
        D_8008D996Edd4[(u16) i].unk0 = -1;
        D_8008D988Edd4[(u16) i].unk0 = 0xFF;
        D_8008D9A3Edd4[(u16) i].unk0 = 0;
        D_8008D98C[(u16) i].unk0 = 0;
        D_8008D98E[(u16) i].unk0 = 0;
        D_8008D998[(u16) i].unk0 = 0;
        D_8008D99AEdd4[(u16) i].unk0 = 0;
        D_8008D99CEdd4[(u16) i].unk0 = 0xFF;
        D_8008D990[(u16) i].unk0 = 0;
        D_8008D992[(u16) i].unk0 = 0x40;
        gVoiceEnvActive[(u16) i].unk0 = 0;
        gVoiceEnvStep[(u16) i].unk0 = 0;
        gVoiceEnvInterval[(u16) i].unk0 = 0;
        gVoiceEnvCountdown[(u16) i].unk0 = 0;
        gVoiceFadeActive[(u16) i].unk0 = 0;
        gVoiceFadeStep[(u16) i].unk0 = 0;
        gVoiceFadeInterval[(u16) i].unk0 = 0;
        gVoiceFadeCountdown[(u16) i].unk0 = 0;
        gVoiceFadeAccum[(u16) i].unk0 = 0;
        gVoiceEnvAccum[(u16) i].unk0 = 0;

        ((s16 *) D_8006DAD4Edd4)[woff + 3] = 0x200;   /* +0x6 */
        scratch = woff;
        ((s16 *) D_8006DAD4Edd4)[woff + 2] = 0x1000;  /* +0x4 */
        ((u16 *) D_8006DAD4Edd4)[woff + 4] = 0x80FF;  /* +0x8 */
        ((s16 *) D_8006DAD4Edd4)[scratch] = 0;         /* +0x0 */
        ((s16 *) D_8006DAD4Edd4)[woff + 1] = 0;       /* +0x2 */
        ((s16 *) D_8006DAD4Edd4)[woff + 5] = 0x4000;  /* +0xA */

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

        D_8008D9A3Edd4[chan].unk0 = 0;
        D_8008D98C[chan].unk0 = 0;
        D_8008D988Edd4[chan].unk0 = 0;
        D_80090C60 |= lowMask;
        D_80090C64 |= highMask;
        woff = D_80090C60;
        D_8008E228 &= ~woff;
        D_8008E22C &= ~D_80090C64;
    }

    gMasterVolL = 0x3FFF;
    gMasterVolR = 0x3FFF;
    D_8008E228 = 0;
    D_8008E22C = 0;
    D_80090C60 = 0;
    D_8008E230 = 0;
    D_8008E234 = 0;
    D_8008E258 = 0;
    D_8008E25C = 0;
    gDisableVoiceStarveScan = 0;
    D_8008E8C0 = 0;
    D_8008E938 = 0x80;
    UpdateVoiceEnvelopes();
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

extern s32 func_8002CF18(s32 a0); /* arity-ok: the callee (still INCLUDE_ASM, 0x8002CF18) reads NO argument register, but this unit's argument is byte-load-bearing -- retail emits `li a0,0xff` in the delay slot at 0x8002F244 */
extern void func_8002DDBC(s32 a0, s32 a1, s32 a2, s32 a3, s32 a4);

void PlaySound(s32 a0, s32 a1, s32 a2, s32 a3) {
    s32 v0;

    D_8008EA1B = 0x7F;
    v0 = func_8002CF18(0xFF) & 0xFF;
    D_8008EA26 = v0;
    if (v0 < D_8008E9D0) {
        func_8002DDBC(*(u8 *)&D_8008EA26, a0 & 0xFFFF, a1 & 0xFFFF, a2 & 0xFFFF, a3 & 0xFFFF);
    }
}

/* Same 0x34-stride channel-configuration record family documented in
 * code_179d8_j.c (Rec34D994/Rec34Byte); this unit keeps its own local
 * view rather than sharing that file's header-less types. Rec34Half
 * itself is declared above, before its first user BeginVoiceFade. */
typedef struct {
    u8 unk0; /* +0x0 */
    u8 pad1[0x34 - 0x1];
} Rec34Byte;
extern Rec34Byte D_8008D9A3[];

extern Rec34Half D_8008D98C[];

/* Pointer to a "current object" whose only fields this function
 * touches sit at a fixed byte offset from the base, not scaled by any
 * index -- a different reading of the same D_8006DAD4 symbol from
 * code_179d8_j.c's array-of-0x10-byte-records view, per this project's
 * multiple-independent-local-views convention. */
typedef struct {
    u8 pad[0x194];
    u16 unk194; /* +0x194 */
    u16 unk196; /* +0x196 */
} ObjDAD4;
extern ObjDAD4 *D_8006DAD4;

void ClearNoiseVoices(void) {
    s16 i;

    for (i = 0; i < D_8008E9D0; i++) {
        if (D_8008D9A3[i].unk0 == 2) {
            D_8008D9A3[(u8) i].unk0 = 0;
            D_8008D98C[(u8) i].unk0 = 0;
            D_8006DAD4->unk194 = 0;
            D_8006DAD4->unk196 = 0;
        }
    }
}

void PlayFixedSound(s32 a0, s32 a1) {
    s32 v0;

    D_8008EA1B = 0x7F;
    v0 = func_8002CF18(0xFF) & 0xFF;
    D_8008EA26 = v0;
    if (v0 < D_8008E9D0) {
        func_8002DDBC(*(u8 *)&D_8008EA26, a0 & 0xFFFF, a1 & 0xFFFF, 0x80FF, 0x5FC8);
    }
}

/* Same 0x34-stride channel-configuration record family, s16-field
 * view -- matches code_179d8_j.c's own Rec34D994 shape (that unit's
 * D_8008D994/D_8008D996/D_8008D99A/D_8008D99C/D_8008D99E family); this
 * unit keeps its own independent view rather than sharing the header-
 * less type. D_8008D988 shares the shape too (read here with `lh`, a
 * signed load, unlike D_8008D98C's `lhu`-driven Rec34Half above). */
typedef struct {
    s16 unk0; /* +0x0 */
    u8 pad2[0x34 - 0x2];
} Rec34S16;
extern Rec34S16 D_8008D994[];
extern Rec34S16 D_8008D996[];
extern Rec34S16 D_8008D99A[];
extern Rec34S16 D_8008D99E[];
extern Rec34S16 D_8008D988[];

/* Same 0x34-stride record family, UNSIGNED 16-bit view -- D_8008D99C
 * needs this width (`lhu`) here, and BOTH this width and a plain byte
 * width (`lbu`, same offset) elsewhere in this same function; the byte
 * view is reached via a plain pointer cast, same idiom as D_8008EA26's
 * mixed sh/lbu access.  D_8008D994 needs the same unsigned re-reading
 * here even though StopNote (above) reads the SAME symbol signed
 * (`lh`) -- reinterpreted through a cast rather than redeclared, since
 * one extern symbol cannot carry two conflicting C types in one file. */
typedef struct {
    u16 unk0; /* +0x0 */
    u8 pad2[0x34 - 0x2];
} Rec34U16;
extern Rec34U16 D_8008D99C[];

/* Debug/selected-difficulty byte, read fresh each call. */
extern u8 D_8008EA13;

/* Pointer to a 0x20-byte-stride table. Originally only the two
 * trailing byte fields ApplyVoicePitchBend reads (unkC/unkD) were named;
 * StartNote (below) additionally needs unk0/unk1/unk2/unk3/unk4/
 * unk5/unk6/unk7/unk16, all in the same struct (no offset conflicts,
 * per this project's convention of extending rather than duplicating
 * a local view when the fields don't overlap). */
typedef struct {
    u8 unk0; /* +0x0 */
    u8 unk1; /* +0x1 */
    u8 unk2; /* +0x2 */
    u8 unk3; /* +0x3 */
    u8 unk4; /* +0x4 */
    u8 unk5; /* +0x5 */
    u8 unk6; /* +0x6 */
    u8 unk7; /* +0x7 */
    u8 pad8[0xC - 0x8];
    u8 bendCurveUp; /* +0xC -- multiplier used when the bend threshold is positive, per ApplyVoicePitchBend's report */
    u8 bendCurveDown; /* +0xD -- multiplier used when the bend threshold is negative */
    u8 pad0E[0x16 - 0xE];
    u8 unk16; /* +0x16 */
    u8 pad17[0x20 - 0x17];
} Tbl32E978;
extern Tbl32E978 *D_8008E978;

/* Same 0x10-byte-stride record family code_179d8_j.c documents as
 * Rec16D7F0 (that unit's D_8008D7F0/D_8008D7F4 pair); local view. */
typedef struct {
    s16 unk0; /* +0x0 */
    u8 pad2[0x10 - 0x2];
} Rec16D7F4;
extern Rec16D7F4 D_8008D7F4[];

/* Plain byte-stride flags array (no per-record multiply in its own
 * addressing -- unlike every 0x34/0x10-stride array above). */
extern u8 D_8008D970[];

/* Selected-channel debug byte, write-only here. */
extern u8 D_8008EA18;

extern s16 func_8002E038(u16 a0, u16 a1);

#ifdef NON_MATCHING
/* NON_MATCHING: 77/138 words, length exact. Residue: register-class
 * renumbering plus one deferred `& 0xFFFF` mask on the second
 * func_8002E038 argument -- a banned-to-fix register-identity case, per
 * a permuter search that plateaued at 485/770 with no candidate reaching
 * zero (docs/match-reports/ApplyVoicePitchBend.md). Hand-derived. */
s16 ApplyVoicePitchBend(s16 a0, s16 a1, s16 a2, s16 a3, u16 a4)
{
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
    if (D_8008D996[a0].unk0 != a1) {
        return 0;
    }
    if (D_8008D99E[a0].unk0 != a2) {
        return 0;
    }
    if (D_8008D99A[a0].unk0 != a3) {
        return 0;
    }

    someTotal = D_8008D99C[a0].unk0 + (D_8008EA13 << 4);
    baseValue = ((Rec34U16 *) D_8008D994)[a0].unk0;

    if (threshold > 0) {
        tableByte = D_8008E978[someTotal].bendCurveDown;
        product = threshold * tableByte;
        q = product / 63;
        outA2 = baseValue + q;
        r = product % 63;
        outA1 = r * 2;
    } else {
        outA2 = baseValue;
        if (threshold < 0) {
            tableByte = D_8008E978[someTotal].bendCurveUp;
            product = threshold * tableByte;
            q = product / 64;
            outA2 = baseValue + q - 1;
            r = product % 64;
            outA1 = r * 2 + 0x7F;
        } else {
            outA1 = 0;
        }
    }

    byteVal = *(u8 *) &D_8008D99C[a0].unk0;
    D_8008EA26 = a0;
    D_8008EA18 = byteVal;
    D_8008D7F4[a0].unk0 = func_8002E038(outA2 & 0xFFFF, outA1 & 0xFFFF);
    D_8008D970[a0] |= 4;
    return 1;
}
#else
INCLUDE_ASM("asm/nonmatchings/code_179d8_m", ApplyVoicePitchBend);
#endif

/* "Currently selected channel" scratch global -- same idiom as
 * D_8008EA26 above, write-only here (see code_179d8_j.c's own reading
 * of this symbol). */
extern u16 D_8008EA22;

extern s32 SpuVmVSetUp(s16 a0, s16 a1);
extern s16 ApplyVoicePitchBend(s16 a0, s16 a1, s16 a2, s16 a3, u16 a4);

s32 ApplyPitchBendToAllVoices(s16 a0, s16 a1, s16 a2, u16 a3) {
    s16 i;
    s32 sum;

    SpuVmVSetUp(a1, a2);
    D_8008EA22 = a0;
    sum = 0;
    for (i = 0; i < D_8008E9D0; i++) {
        sum += ApplyVoicePitchBend(i, a0, a1, a2, a3);
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
 * candidate reaching zero (docs/match-reports/UpdateVoiceEnvelopes.md).
 * Hand-derived. */

/* Ring buffer of "channel activity" bitmasks, one slot appended per
 * call, most-recent index tracked by gVoiceActivityRingIdx (mod 16). */
extern s32 gVoiceActivityRingIdx;
extern s32 gVoiceActivityRing[];

/* 0x34-stride record family, UNSIGNED 16-bit view -- this function
 * writes it via `lhu`-driven re-reads (store, then re-check the SAME
 * field unsigned) rather than the `Rec34Half` signed view. D_8008D98E
 * already declared elsewhere in the unit as `Rec34Half`; reinterpreted
 * here via cast, not redeclared. */
typedef struct {
    u16 unk0; /* +0x0 */
    u8 pad2[0x34 - 0x2];
} Rec34HalfU2;

extern void SpuSetNoiseVoice(s32 a0, s32 a1);
extern void StepVoiceEnvelope(s16 a0);
extern void StepVoiceFade(s16 a0);
extern Rec16D7F4 D_8008D7F6[];

/* Same 0x10-byte-stride record family as `Rec16D7F0`/D_8008D7F0's other
 * field (declared above, `unk0` only) -- this function ALSO reads this
 * array's `+0x2`, `+0x8` and `+0xA` sub-fields, so it needs a wider
 * local view of the same base symbol, reached via a cast per this
 * project's multiple-independent-local-views convention. */
typedef struct {
    s16 unk0; /* +0x0 */
    s16 unk2; /* +0x2 */
    u8 pad4[0x8 - 0x4];
    s16 unk8; /* +0x8 */
    s16 unkA; /* +0xA */
    u8 padC[0x10 - 0xC];
} Rec16D7F0Wide;

/* D_8006DAD4, already declared above as `ObjDAD4 *` (one struct, fields
 * at +0x194/+0x196), is ALSO the base of an array of 0x10-byte
 * per-channel records here -- another independent local view of the
 * same pointed-to object (see also code_179d8_j.c's own array-of-0x10
 * reading of a sibling symbol). */
typedef struct {
    s16 unk0; /* +0x0 */
    s16 unk2; /* +0x2 */
    s16 unk4; /* +0x4 */
    s16 unk6; /* +0x6 */
    s16 unk8; /* +0x8 */
    s16 unkA; /* +0xA */
    u16 unkC; /* +0xC */
    u8 padE[0x10 - 0xE];
} Rec16DAD4C;

void UpdateVoiceEnvelopes(void) {
    s32 i = 0;
    s32 ringIdx;
    s32 *slot;
    s32 count;
    u8 dead[8];

    if (0) {
        dead[0] = 0;
    }

    ringIdx = (gVoiceActivityRingIdx + 1) & 0xF;
    gVoiceActivityRingIdx = ringIdx;
    slot = &gVoiceActivityRing[ringIdx];
    count = D_8008E9D0;
    *slot = 0;

    if (count > 0) {
        Rec34HalfU2 *p98E = (Rec34HalfU2 *) D_8008D98E;
        Rec16DAD4C *pDad = (Rec16DAD4C *) D_8006DAD4;

        for (i = 0; i < count; i++) {
            p98E->unk0 = pDad->unkC;
            if (p98E->unk0 == 0) {
                *slot |= 1 << i;
            }
            p98E++;
            pDad++;
        }
    }

    if (gDisableVoiceStarveScan == 0) {
        s32 mask;
        s32 j;

        mask = -1;
        for (j = 0; j < 0xF; j++) {
            mask &= gVoiceActivityRing[j];
        }

        for (i = 0; i < D_8008E9D0; i++) {
            s32 bit = 1 << i;

            if (mask & bit) {
                if (D_8008D9A3[i].unk0 == 2) {
                    SpuSetNoiseVoice(0, 0xFFFFFF);
                }
                D_8008D9A3[i].unk0 = 0;
            }
        }
    }

    D_8008E228 &= ~D_80090C60;
    D_8008E22C &= ~D_80090C64;

    for (i = 0; i < 0x18; i++) {
        if (gVoiceEnvActive[i].unk0 != 0) {
            StepVoiceEnvelope(i);
        }
        if (gVoiceFadeActive[i].unk0 != 0) {
            StepVoiceFade(i);
        }
    }

    {
    Rec16D7F0Wide *p7F0 = D_8008D7F0;

    for (i = 0; i < 0x18; i++) {
        if (D_8008D970[i] & 1) {
            ((Rec16DAD4C *) D_8006DAD4)[i].unk0 = p7F0->unk0;
            ((Rec16DAD4C *) D_8006DAD4)[i].unk2 = p7F0->unk2;
        }
        if (D_8008D970[i] & 4) {
            ((Rec16DAD4C *) D_8006DAD4)[i].unk4 = D_8008D7F4[i].unk0;
        }
        if (D_8008D970[i] & 8) {
            ((Rec16DAD4C *) D_8006DAD4)[i].unk6 = D_8008D7F6[i].unk0;
        }
        if (D_8008D970[i] & 0x10) {
            ((Rec16DAD4C *) D_8006DAD4)[i].unk8 = p7F0->unk8;
            ((Rec16DAD4C *) D_8006DAD4)[i].unkA = p7F0->unkA;
        }

        D_8008D970[i] = 0;
        p7F0++;
    }
    }

    {
        ObjDAD4 *rec = D_8006DAD4;
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

        *(u16 *) ((u8 *) rec + 0x18C) = lowMask;
        *(u16 *) ((u8 *) rec + 0x18E) = highMask;
        *(u16 *) ((u8 *) rec + 0x188) = lowActive;
        *(u16 *) ((u8 *) rec + 0x18A) = highActive;
        *(s16 *) ((u8 *) rec + 0x198) = v230;
        *(s16 *) ((u8 *) rec + 0x19A) = v234;
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/code_179d8_m", UpdateVoiceEnvelopes);
#endif

/* STALL -- see docs/match-reports/StartNote.md. Round 48 (echo):
 * tested charlie's frame-padding lever (u8 dead[8], sized to the build's
 * frame gap: 0x140 -> 0x148, byte-exact vs retail). THIRD confirmed
 * negative for length closure this round (same as UpdateVoiceEnvelopes and
 * StepVoiceFade above) -- built length UNCHANGED (402/387, still 15
 * words LONG). This function's own report already diagnoses its gap as
 * two unrelated residues (an early-materialization scheduling point, and
 * a mid-loop addressing-cost difference for D_8008EA26 and neighbors) --
 * frame padding does not touch either. Best body (402/387 built words,
 * 15 words LONG) preserved there in #if 0. */
INCLUDE_ASM("asm/nonmatchings/code_179d8_m", StartNote);

/* A pair of 16-bit bitmasks split across a 0..0x1F channel space (low
 * 16 channels in the first word, next 16 in the second), each paired
 * with an "active mask" word cleared wherever the channel mask bit is
 * set -- same symbols and reading code_179d8_j.c already documents. */
extern u16 D_80090C60;
extern u16 D_80090C64;
extern u16 D_8008E228;
extern u16 D_8008E22C;

u8 StopNote(s16 a0, s16 a1, s16 a2, u16 a3) {
    u8 i;
    u8 count;

    count = 0;
    for (i = 0; i < D_8008E9D0; i++) {
        if (D_8008D994[i].unk0 != a3) {
            continue;
        }
        if (D_8008D99A[i].unk0 != a2) {
            continue;
        }
        if (D_8008D996[i].unk0 != a0) {
            continue;
        }
        if (D_8008D99E[i].unk0 != a1) {
            continue;
        }
        if (D_8008D988[i].unk0 == 0xFF) {
            D_8008D9A3[i].unk0 = 0;
            D_8008D98C[i].unk0 = 0;
            D_8006DAD4->unk194 = 0;
            D_8006DAD4->unk196 = 0;
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
            D_8008D9A3[chan].unk0 = 0;
            D_8008D98C[chan].unk0 = 0;
            D_8008D988[chan].unk0 = 0;
            D_80090C60 |= lowMask;
            D_80090C64 |= highMask;
            D_8008E228 &= ~D_80090C60;
            D_8008E22C &= ~D_80090C64;
        }
        count++;
    }
    return count;
}
