/*
 * libsnd_seqread -- Sony libsnd `seqread`: the SEQ event interpreter.
 *
 * Every function here is Sony's. Retail links libsnd 3.3's seqread with one
 * function changed (_SsSetControlChange), so the object never placed and the
 * module is carried as C. progress.py counts all of it as library by address
 * (config/sdk-in-game.txt, and the `identified` symbols entry for
 * _SsSetControlChange). The functions keep Sony's names and are never retyped
 * as game code.
 *
 * The file is the whole module: its 18 functions are seqread.o's, in
 * seqread.o's order.
 *
 * Each sequence's play state is an SsScore (include/SsScore.h), reached as
 * _ss_score[access][seq]; the (a0, a1) pair the event handlers take is that
 * (access, seq) pair. Per-channel state (pan, program, volume) is indexed by
 * the event's MIDI channel, unk12. SeqPlay is the per-tick scheduler. GetSeqData
 * decodes one event from the byte stream at rec->unk4 and dispatches it to:
 *   - NoteOn / SetProgramChange / SetPitchBend, the channel-voice events;
 *   - _SsSetControlChange, the MIDI CC dispatcher, which routes to the Cont*
 *     handlers: data entry, portamento, NRPN/RPN, reset-all-controllers;
 *   - GetMetaEvent, for End of Track (repeat or stop) and Set Tempo.
 * ReadDeltaValue reads the delta time that follows every event and schedules
 * the next one.
 *
 * The controller handlers edit the VAB in place through Sony's SsUtGet/Set
 * ProgAtr/VagAtr calls. Snd_setVabAttr is ContDataEntry's per-NRPN VagAtr
 * editor, and it unpacks the ADSR registers into an AdsrFields to do so.
 * ContModulation and ContPortaTime have no caller in this executable.
 *
 * This unit owns three switch jump tables in rodata: jtbl_80010CF0
 * (_SsSetControlChange), and jtbl_80010ED8 and jtbl_80010F38
 * (Snd_setVabAttr's nested switch).
 */
#include "common.h"
#include <libsnd.h>

#include "SsScore.h"

/* Calls into other modules, typed from the registers each call site loads;
 * a prototype for a function another file defines stays in the calling
 * file. */
extern void SpuVmPitchBend(s32 a0, s16 a1, u8 a2, u8 a3); /* libsnd_vmanager, not yet matched: local guess */
/* SpuVmKeyOn: Sony libsnd/vmanager internal, no public LIBSND.H prototype
 * (unlike SsUtKeyOn). */
extern s32 SpuVmKeyOn(s32 a0, s16 a1, s16 a2, u16 a3, u16 a4,
                      u16 a5); /* libsnd_vmanager, not yet matched: local guess, matches libsnd_vmanager's independent reading of the same call shape */
extern s32 SpuVmKeyOff(s32 a0, s16 a1, s16 a2, u16 a3); /* libsnd_vmanager, not yet matched: local guess, ditto */
/* Sony libsnd/vm_doff, internal: no public LIBSND.H prototype. */
extern void SpuVmDamperOff(void);

/* Shared delta-time decoder: reads a MIDI variable-length value (7 bits
 * per byte, big-endian, continuation bit first) from rec->unk4 (advancing the cursor as it goes), scales the
 * decoded magnitude by 10, adds it to rec->unk80, and returns the scaled
 * delta.  A first byte of 0 is a sentinel for "no delta" -- returns 0
 * without touching rec->unk80 at all. */
extern s32 ReadDeltaValue(s16 channel, s16 slot);

/* GetSeqData is defined below; SeqPlay's catch-up loop calls it with the
 * same (channel, slot) pair as every other helper in this file. */
extern void GetSeqData(s16 channel, s16 slot);

/* Catch-up scheduler tick.  When the re-armed counter is still reloading
 * its threshold (remain == 0) it copies the threshold rec->unk70 into the
 * counter.  The third parameter is unused; retail reuses its dead register
 * ($a2) to hold rec->unk70 for that store (docs/match-reports/SeqPlay.md). */
void SeqPlay(s16 a0, s16 a1, s16 a2) {
    SsScore *rec = &_ss_score[a0][a1];
    s16 last = rec->unk70;
    s32 elapsed = rec->unk88;
    s32 delta = elapsed - last;
    s16 remain;
    s32 sum;
    s32 step;
    s16 last2;

    if (delta > 0) {
        remain = rec->unk6E;
        if (remain > 0) {
            rec->unk6E = remain - 1;
            return;
        }
        if (remain == 0) {
            rec->unk6E = last;
            rec->unk88 = rec->unk88 - 1;
            return;
        }
        rec->unk88 = delta;
        return;
    }
    if (last < elapsed) {
        return;
    }
    sum = elapsed;
    for (;;) {
        GetSeqData(a0, a1);
        step = rec->unk88;
        if (step != 0) {
            last2 = rec->unk70;
            sum += step;
            if (sum < last2) {
                continue;
            }
            rec->unk88 = sum - last2;
            break;
        }
    }
}

/* Forward declarations for the handlers GetSeqData dispatches to by
 * status byte, defined later in this unit; the signatures of
 * _SsSetControlChange and GetMetaEvent are read from GetSeqData's calls. */
extern void NoteOn(s16 a0, s16 a1, s32 a2, s32 a3);
extern void SetProgramChange(s16 a0, s16 a1, u8 a2);
extern void _SsSetControlChange(s16 a0, s16 a1, u8 a2);
extern void SetPitchBend(s16 a0, s16 a1);
extern void GetMetaEvent(s16 a0, s16 a1, u8 a2);

/* A per-channel/slot "sequencer voice" event-stream byte reader.  Reads one
 * byte from rec->unk4 (advancing the cursor); if it has the high bit set it
 * is a new MIDI-style status byte -- the low nibble becomes rec->unk12 (the
 * MIDI channel) and the high nibble selects which
 * kind of event follows, consuming however many further data bytes that
 * kind needs and recording the high nibble into rec->unk11 as "running
 * status" (0xFF standing in for the 0xF0 kind).  If the high bit is clear,
 * the byte just read is itself the first DATA byte of a new event of
 * whichever kind rec->unk11 last recorded (MIDI running status) -- same
 * dispatch, one fewer byte consumed since this byte already stood in for
 * the first data byte.
 *
 */
#ifdef NON_MATCHING
/* NON_MATCHING: 122/172 words, length exact. Residue: register identity
 * (retail's widened "channel" lives in $s4 and its per-case data byte in
 * $s3; this C compiles the same roles the other way around)
 * (docs/match-reports/GetSeqData.md). */
void GetSeqData(s16 a0, s16 a1) {
    SsScore *rec = &_ss_score[a0][a1];
    u8 *p;
    u8 raw;
    u8 note, vel;

    p = rec->unk4;
    rec->unk4 = p + 1;
    raw = *p;
    if (raw & 0x80) {
        rec->unk12 = raw & 0xF;
        switch (raw & 0xF0) {
            case 0x90:
                p = rec->unk4;
                rec->unk11 = 0x90;
                rec->unk4 = p + 1;
                note = *p;
                rec->unk4 = p + 2;
                vel = *(p + 1);
                rec->unk88 = ReadDeltaValue(a0, a1);
                NoteOn(a0, a1, note, vel);
                return;
            case 0xB0:
                p = rec->unk4;
                rec->unk11 = 0xB0;
                rec->unk4 = p + 1;
                note = *p;
                _SsSetControlChange(a0, a1, note);
                return;
            case 0xC0:
                p = rec->unk4;
                rec->unk11 = 0xC0;
                rec->unk4 = p + 1;
                note = *p;
                SetProgramChange(a0, a1, note);
                return;
            case 0xE0:
                rec->unk11 = 0xE0;
                rec->unk4 = rec->unk4 + 1;
                SetPitchBend(a0, a1);
                return;
            case 0xF0:
                p = rec->unk4;
                rec->unk11 = 0xFF;
                rec->unk12 = raw & 0xF;
                rec->unk4 = p + 1;
                note = *p;
                GetMetaEvent(a0, a1, note);
                return;
            default:
                return;
        }
    } else {
        switch (rec->unk11) {
            case 0x90:
                vel = *rec->unk4;
                rec->unk4 = rec->unk4 + 1;
                rec->unk88 = ReadDeltaValue(a0, a1);
                NoteOn(a0, a1, raw, vel);
                return;
            case 0xB0:
                _SsSetControlChange(a0, a1, raw);
                return;
            case 0xC0:
                SetProgramChange(a0, a1, raw);
                return;
            case 0xE0:
                SetPitchBend(a0, a1);
                return;
            case 0xFF:
                GetMetaEvent(a0, a1, raw);
                return;
            default:
                return;
        }
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/libsnd_seqread", GetSeqData);
#endif

#ifdef NON_MATCHING
/* NON_MATCHING: 62/70 words, length exact. Residue: register identity
 * ($t0<->$a2 for the a0 copy kept live across the two calls, $a3/$s1<->$t0
 * for the masked-a3 copy), not a logic or CFG difference
 * (docs/match-reports/NoteOn.md). */
void NoteOn(s16 a0, s16 a1, s32 a2, s32 a3) {
    SsScore *rec = &_ss_score[a0][a1];
    u8 offset = rec->unk12;
    s16 speed = rec->unk4E[offset];
    s32 divided = ((u8)a3 * (s32)speed) / 127;
    u16 flag = rec->unk74;
    u8 status = rec->unk17[offset];

    speed = a3;
    if (flag == 0) {
        return;
    }
    if ((u8)a3 != 0) {
        s16 packed = (a1 << 8) | a0;
        s16 note = rec->unk4C;
        u8 vol = rec->unk2C[offset];
        SpuVmKeyOn(packed, note, vol, (u8)a3, (u16)divided, status);
        rec->unkA8 = (u8)speed;
    } else {
        s16 packed = (a1 << 8) | a0;
        s16 note = rec->unk4C;
        u8 vol = rec->unk2C[offset];
        SpuVmKeyOff(packed, note, vol, (u8)a3);
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/libsnd_seqread", NoteOn);
#endif

void SetProgramChange(s16 a0, s16 a1, u8 a2) {
    SsScore *rec = &_ss_score[a0][a1];

    rec->unk2C[rec->unk12] = a2;
    rec->unk88 = ReadDeltaValue(a0, a1);
}

/* Calls into other modules. SpuVmDamperOn, SsUtSetReverbDepth and
 * SpuVmSetProgVol are Sony's libsnd (`vm_don`, `ut_rev`, `vm_prog`).
 * SpuVmSetVol (libsnd_vmanager.c) is typed from this call site: a 5th
 * argument, on the stack at 0x10($sp), after the packed "(slot<<8)|channel"
 * first argument this file's siblings use. */
extern void SpuVmDamperOn(void);
extern s32 SpuVmSetProgVol(s16 p0, s16 p1, s32 p2);
extern void SpuVmSetVol(s32 packed, s16 note, u8 vol, s32 arg3, s32 arg4);

/* Forward declarations for the handlers defined later in this unit. */
extern void ContDataEntry(s16 a0, s16 a1, u8 a2);
extern void ContPortamento(s16 a0, s16 a1, s32 a2);
extern void ContNrpn1(s16 a0, s16 a1, u8 a2);
extern void ContNrpn2(s16 a0, s16 a1, u8 a2);
extern void ContRpn1(s16 a0, s16 a1, u8 a2);
extern void ContRpn2(s16 a0, s16 a1, u8 a2);
extern void ContResetAll(s16 a0, s16 a1);

/* Control-Change dispatcher: reads one data byte from the event stream
 * (the CC value) and routes on `a2`, the CC NUMBER, through a dense 0..121
 * switch that GCC lowers to the jump table this unit owns
 * (jtbl_80010CF0). The controller numbers with dedicated handling below are
 * exactly the standard MIDI CC assignments (0 bank-select MSB, 6 data-entry
 * MSB, 7 volume, 10 pan, 11 expression, 64 sustain, 65 portamento, 91 reverb
 * depth, 98/99 NRPN, 100/101 RPN, 121 reset-all-controllers), which is a
 * strong confirmation this really is a MIDI CC handler and not a
 * project-invented numbering. Every arm except 6/65/98/99/100/101/121 falls
 * through into the shared tail that re-arms the next scheduling delta via
 * ReadDeltaValue; those seven `return` immediately instead.
 *
 * MATCHING, two choices below: SpuVmSetVol's first parameter is a full
 * `s32` (its own body masks it with 0xFF/0xFF00), so `packed` is not
 * narrowed and the widened a0/a1 stay live across the call for the final
 * ReadDeltaValue; and each case copies `offset` into a case-local `u16`,
 * which keeps the switch-wide byte in a caller-saved register and gives
 * each case its own callee-saved copy.
 */
void _SsSetControlChange(s16 a0, s16 a1, u8 a2) {
    SsScore *rec = &_ss_score[a0][a1];
    u8 *p = rec->unk4;
    u8 offset = rec->unk12;
    u8 val;

    rec->unk4 = p + 1;
    val = *p;
    switch (a2) {
        case 0:
            rec->unk4C = val;
            rec->unk88 = ReadDeltaValue(a0, a1);
            return;
        case 6:
            ContDataEntry(a0, a1, val);
            return;
        case 7: {
            u16 o = offset;
            s32 packed = (a1 << 8) | a0;

            SpuVmSetVol(packed, rec->unk4C, rec->unk2C[o], val, rec->unk17[o]);
            rec->unk4E[o] = val;
            rec->unk88 = ReadDeltaValue(a0, a1);
            return;
        }
        case 10: {
            s32 packed = (a1 << 8) | a0;
            u16 o = offset;
            /* MATCHING: reaching unk2C/unk17 through one base pointer keeps
             * retail's register allocation; rec->unk2C[o] does not. */
            u8 *blk = (u8 *)rec + o;
            s16 wide = rec->unk4E[o];

            SpuVmSetVol(packed, rec->unk4C, blk[offsetof(SsScore, unk2C)], wide, val);
            blk[offsetof(SsScore, unk17)] = val;
            rec->unk88 = ReadDeltaValue(a0, a1);
            return;
        }
        case 11: {
            u16 o = offset;

            SpuVmSetProgVol(rec->unk4C, rec->unk2C[o], val);
            SpuVmSetVol((a1 << 8) | a0, rec->unk4C, rec->unk2C[o], rec->unk4E[o], rec->unk17[o]);
            rec->unk88 = ReadDeltaValue(a0, a1);
            return;
        }
        case 64:
            if (val < 0x40) {
                SpuVmDamperOff();
            } else {
                SpuVmDamperOn();
            }
            break;
        case 65:
            ContPortamento(a0, a1, val);
            return;
        case 91:
            SsUtSetReverbDepth(val, val);
            break;
        case 98:
            ContNrpn1(a0, a1, val);
            return;
        case 99:
            ContNrpn2(a0, a1, val);
            return;
        case 100:
            ContRpn1(a0, a1, val);
            return;
        case 101:
            ContRpn2(a0, a1, val);
            return;
        case 121:
            ContResetAll(a0, a1);
            return;
        default:
            break;
    }
    rec->unk88 = ReadDeltaValue(a0, a1);
}

/* The three per-controller VagAtr editors below share one shape: fetch the
 * channel's program with SsUtGetProgAtr, then for each of its `tones` read
 * the tone's VagAtr, overwrite one field from the controller value and
 * write it back. `tones` is re-read from memory on every iteration because
 * the compiler cannot prove the SsUt calls leave the ProgAtr alone.
 *
 * ContModulation: the value becomes every tone's vibrato depth (vibW).
 * Neither it nor ContPortaTime has a caller in this executable. */
void ContModulation(s16 a0, s16 a1, u8 a2) {
    SsScore *rec = &_ss_score[a0][a1];
    u8 offset;
    ProgAtr list;
    VagAtr scratch;
    s32 i;

    SsUtGetProgAtr(rec->unk4C, rec->unk2C[offset = rec->unk12], &list);
    for (i = 0; i < list.tones; i++) {
        SsUtGetVagAtr(rec->unk4C, rec->unk2C[offset], (s16)i, &scratch);
        scratch.vibW = a2;
        SsUtSetVagAtr(rec->unk4C, rec->unk2C[offset], (s16)i, &scratch);
    }
    rec->unk88 = ReadDeltaValue(a0, a1);
}

/* The value becomes every tone's portamento time (porT). */
void ContPortaTime(s16 a0, s16 a1, u8 a2) {
    SsScore *rec = &_ss_score[a0][a1];
    u8 offset;
    ProgAtr list;
    VagAtr scratch;
    s32 i;

    SsUtGetProgAtr(rec->unk4C, rec->unk2C[offset = rec->unk12], &list);
    for (i = 0; i < list.tones; i++) {
        SsUtGetVagAtr(rec->unk4C, rec->unk2C[offset], (s16)i, &scratch);
        scratch.porT = a2;
        SsUtSetVagAtr(rec->unk4C, rec->unk2C[offset], (s16)i, &scratch);
    }
    rec->unk88 = ReadDeltaValue(a0, a1);
}

/* CC65 (portamento): a value below 0x40 sets every tone's play mode
 * to 2, a value in 0x40..0x7F sets it to 0, anything else leaves it. */
void ContPortamento(s16 a0, s16 a1, s32 a2) {
    SsScore *rec = &_ss_score[a0][a1];
    u8 offset;
    ProgAtr list;
    VagAtr scratch;
    s32 i;
    s32 wrap;

    SsUtGetProgAtr(rec->unk4C, rec->unk2C[offset = rec->unk12], &list);
    for (i = 0; i < list.tones; i++) {
        SsUtGetVagAtr(rec->unk4C, rec->unk2C[offset], (s16)i, &scratch);
        if ((u8)a2 < 0x40) {
            scratch.mode = 2;
        } else {
            wrap = 0xC0;
            if ((u8)(a2 + wrap) < 0x40) {
                scratch.mode = 0;
            }
        }
        SsUtSetVagAtr(rec->unk4C, rec->unk2C[offset], (s16)i, &scratch);
    }
    rec->unk88 = ReadDeltaValue(a0, a1);
}

void ContResetAll(s16 a0, s16 a1) {
    SsScore *rec = &_ss_score[a0][a1];

    SsUtReverbOff();
    SpuVmDamperOff();

    rec->unk2C[rec->unk12] = rec->unk12;
    rec->unk13 = 0;
    rec->unk14 = 0;
    rec->unk4E[rec->unk12] = 0x7F;
    rec->unk17[rec->unk12] = 0x40;
    rec->unk88 = ReadDeltaValue(a0, a1);
}

/* The mark callbacks SsSetMarkCallback installs, one per (access number,
 * sequence number); ContNrpn1 calls the entry with (access, seq, data). */
extern SsMarkCallbackProc D_80090368[][16];

/* CC98 (NRPN LSB). While a loop start set by ContNrpn2's kind 0x14 still
 * waits for its count (unk27 == 1, unk10 == 0), the value becomes the loop
 * count unk28. Otherwise, unless the current NRPN kind is 0x14 or 0x1E, it
 * is cached in unk15 and bumps the unk2A step counter. Under kind 0x28 the
 * value also goes to the sequence's mark callback. */
void ContNrpn1(s16 a0, s16 a1, u8 a2) {
    SsScore *rec = &_ss_score[a0][a1];
    u8 kind;
    SsMarkCallbackProc fn;

    if (rec->unk27 == 1 && rec->unk10 == 0) {
        rec->unk28 = a2;
        rec->unk10 = 1;
    } else {
        kind = rec->unk16;
        if (kind != 0x1E && kind != 0x14) {
            rec->unk15 = a2;
            rec->unk2A = rec->unk2A + 1;
        }
    }
    if (rec->unk16 == 0x28) {
        s16 ch = a0;
        s16 sl = a1;

        fn = D_80090368[ch][sl];
        if (fn != NULL) {
            fn(ch, sl, a2);
        }
    }
    rec->unk88 = ReadDeltaValue(a0, a1);
}

#if 1
void ContNrpn2(s16 a0, s16 a1, u8 a2) {
    SsScore *rec = &_ss_score[a0][a1];
    u8 kind = a2;
    s32 result;

    switch (kind) {
        case 0x14:
            rec->unk16 = a2;
            rec->unk27 = 1;
            result = ReadDeltaValue(a0, a1);
            rec->unk88 = result;
            rec->unkC = rec->unk4;
            return;
        case 0x1E:
            rec->unk16 = a2;
            if (rec->unk28 == 0) {
                rec->unk10 = 0;
                rec->unk88 = ReadDeltaValue(a0, a1);
                return;
            }
            if (rec->unk28 < 0x7F) {
                rec->unk28--;
                result = ReadDeltaValue(a0, a1);
                rec->unk88 = result;
                if (rec->unk28 != 0) {
                    rec->unk4 = rec->unkC;
                } else {
                    rec->unk10 = 0;
                }
                return;
            }
            ReadDeltaValue(a0, a1);
            rec->unk4 = rec->unkC;
            rec->unk88 = 0;
            return;
        default:
            rec->unk16 = a2;
            rec->unk2A = rec->unk2A + 1;
            rec->unk88 = ReadDeltaValue(a0, a1);
            return;
    }
}
#endif

void ContRpn1(s16 a0, s16 a1, u8 a2) {
    SsScore *rec = &_ss_score[a0][a1];
    u8 counter = rec->unk29;

    rec->unk13 = a2;
    counter = counter + 1;
    rec->unk29 = counter;
    rec->unk88 = ReadDeltaValue(a0, a1);
}

void ContRpn2(s16 a0, s16 a1, u8 a2) {
    SsScore *rec = &_ss_score[a0][a1];
    u8 counter = rec->unk29;

    rec->unk14 = a2;
    counter = counter + 1;
    rec->unk29 = counter;
    rec->unk88 = ReadDeltaValue(a0, a1);
}

/* The SPU envelope registers ADSR1/ADSR2 (VagAtr.adsr1/adsr2) split into
 * their bit fields: _SsUtResolveADSR unpacks a pair into one, and
 * _SsUtBuildADSR packs one back. The rates and the level are the field
 * values shifted down; the four modes hold the masked bit as-is (nonzero
 * means exponential, or decreasing for sustainDir). */
typedef struct {
    s16 attackRate;   /* ADSR1 bits 14-8 */
    s16 decayRate;    /* ADSR1 bits 7-4 */
    s16 sustainLevel; /* ADSR1 bits 3-0 */
    s16 sustainRate;  /* ADSR2 bits 12-6 */
    s16 releaseRate;  /* ADSR2 bits 4-0 */
    s16 attackMode;   /* ADSR1 bit 15 */
    s16 sustainMode;  /* ADSR2 bit 15 */
    s16 releaseMode;  /* ADSR2 bit 5 */
    s16 sustainDir;   /* ADSR2 bit 14 */
} AdsrFields;

/* SsUtGetProgAtr's fill at function entry. From +0x10 the SAME memory is
 * both the VagAtr buffer the unk29==2 loops hand to SsUtGet/SetVagAtr
 * (retail addresses it at sp+0x58 = list+0x10) and, with the 18 bytes
 * after it, the two by-value arguments of Snd_setVabAttr. */
typedef struct {
    ProgAtr prog; /* +0x00: SsUtGetProgAtr's fill */

    VagAtr vag; /* +0x10: passed by value to Snd_setVabAttr, and the
                 * SsUtGet/SetVagAtr buffer of the unk29==2 loops */

    AdsrFields adsr; /* +0x30: passed by value to Snd_setVabAttr */
} DataEntryLocals;

/* Snd_setVabAttr is defined later in this unit; this is its signature. */
extern void Snd_setVabAttr(s16 channel, s16 slot, s16 kind, VagAtr scratch, AdsrFields resolved,
                           s16 arg5, u8 arg6);

#ifdef NON_MATCHING
/* NON_MATCHING: 380/376 words, 4 long; frame exact (-0x108). Residue:
 * retail keeps the two dead `unused` values in a callee-saved register
 * ($s5, set and never read) where this body needs `volatile` stack slots,
 * and with $s5 free this build hoists the loop-invariant `a2 & 0x7F` out of
 * the first loop, which renumbers $s3-$s5 through the loops
 * (docs/match-reports/ContDataEntry.md). Written for the reader: the
 * byte-shaped body's `dead[16]` frame pad and `volatile` on the two
 * `unused` locals are omitted here and kept in the report. */
void ContDataEntry(s16 a0, s16 a1, u8 a2) {
    s16 ch = a0;
    s16 slot = a1;
    SsScore *rec = &_ss_score[ch][slot];
    u8 off = rec->unk12;
    DataEntryLocals list;
    s32 i;
    u8 kind;

    SsUtGetProgAtr(rec->unk4C, rec->unk2C[off], &list);

    if (rec->unk27 == 1 && rec->unk10 == 0) {
        rec->unk28 = a2;
        rec->unk10 = 1;
        rec->unk88 = ReadDeltaValue(ch, slot);
        return;
    }
    if (rec->unk16 != 0x1E && rec->unk16 != 0x14) {
        rec->unk15 = a2;
        rec->unk2A = rec->unk2A + 1;
        rec->unk88 = ReadDeltaValue(ch, slot);
        return;
    }
    if (rec->unk29 == 2) {
        if (rec->unk13 == 0 && rec->unk14 == 0) {
            for (i = 0; i < list.prog.tones; i++) {
                SsUtGetVagAtr(rec->unk4C, rec->unk2C[off], i, &list.vag);
                list.vag.pbmin = list.vag.pbmax = a2 & 0x7F;
                SsUtSetVagAtr(rec->unk4C, rec->unk2C[off], i, &list.vag);
            }
        }
        if (rec->unk13 == 1 && rec->unk14 == 0) {
            s32 unused; /* computed and never read, as in retail */
            if ((u8)(a2 - 0x41) < 0x3F) {
                if (((a2 & 0xFF) * 100) >= 0) {
                    unused = ((a2 & 0xFF) * 100) & 0xE000;
                } else {
                    unused = (((a2 & 0xFF) * 100) + 0x1FFF) & 0xE000;
                }
            } else {
                unused = 0;
            }
            for (i = 0; i < list.prog.tones; i++) {
                SsUtGetVagAtr(rec->unk4C, rec->unk2C[off], i, &list.vag);
                list.vag.shift = list.vag.shift;
                SsUtSetVagAtr(rec->unk4C, rec->unk2C[off], i, &list.vag);
            }
        }
        if (rec->unk13 == 2 && rec->unk14 == 0) {
            s32 unused; /* computed and never read, as in retail */
            if ((u8)(a2 - 0x40) < 0x40) {
                unused = ((a2 & 0xFF) * 25) << 8;
            } else {
                unused = 0;
            }
            for (i = 0; i < list.prog.tones; i++) {
                SsUtGetVagAtr(rec->unk4C, rec->unk2C[off], i, &list.vag);
                list.vag.center = list.vag.center;
                SsUtSetVagAtr(rec->unk4C, rec->unk2C[off], i, &list.vag);
            }
        }
        rec->unk88 = ReadDeltaValue(ch, slot);
        rec->unk29 = 0;
        return;
    }
    if (rec->unk2A == 2) {
        kind = rec->unk16;
        if (kind == 0x10) {
            for (i = 0; i < list.prog.tones; i++) {
                Snd_setVabAttr(rec->unk4C, rec->unk2C[off], i, list.vag, list.adsr, rec->unk15, a2 & 0xFF);
            }
        } else {
            Snd_setVabAttr(rec->unk4C, rec->unk2C[off], (s16)kind, list.vag, list.adsr, rec->unk15,
                           a2 & 0xFF);
        }
        rec->unk88 = ReadDeltaValue(ch, slot);
        rec->unk2A = 0;
        return;
    }
    rec->unk88 = ReadDeltaValue(ch, slot);
}
#else
INCLUDE_ASM("asm/nonmatchings/libsnd_seqread", ContDataEntry);
#endif

/* Both Sony's libsnd/adsr: */
extern void _SsUtResolveADSR(s32 a0, s32 a1, AdsrFields *out);
extern void _SsUtBuildADSR(AdsrFields *in, u16 *adsr1, u16 *adsr2);

/* MIDI CC91 (Reverb Depth)/98/99/100/101 (NRPN/RPN LSB/MSB) and friends'
 * per-parameter handler, reached only from ContDataEntry via a double
 * jump-table dispatch this unit owns (jtbl_80010ED8 outer, jtbl_80010F38
 * inner). `kind` selects which VagAtr (per program-tone) is
 * fetched/stored; `arg5` is the outer parameter selector (0..22), `arg6`
 * the value byte nearly every arm uses.
 * `scratch` and `resolved` arrive by value and serve only as local buffers:
 * SsUtGetVagAtr refills `scratch` before any arm reads it, and
 * _SsUtResolveADSR fills `resolved`. */
void Snd_setVabAttr(s16 channel, s16 slot, s16 kind, VagAtr scratch, AdsrFields resolved, s16 arg5,
                    u8 arg6) {
    SsUtGetVagAtr(channel, slot, kind, &scratch);

    switch (arg5) {
        case 0:
            scratch.prior = arg6;
            goto tailA;
        case 1:
            scratch.mode = arg6;
            SsUtSetVagAtr(channel, slot, kind, &scratch);
            if (arg6 == 0) {
                SsUtReverbOff();
                return;
            }
            if (arg6 == 1) {
                return;
            }
            if (arg6 == 2) {
                return;
            }
            if (arg6 == 3) {
                return;
            }
            if (arg6 != 4) {
                return;
            }
            SsUtReverbOn();
            return;
        case 2:
            scratch.min = arg6;
            goto tailA;
        case 3:
            scratch.max = arg6;
        tailA:
            SsUtSetVagAtr(channel, slot, kind, &scratch);
            return;
        case 4:
        case 5:
        case 6:
        case 7:
        case 8:
        case 9:
        case 10:
        case 11:
        case 12:
        case 13:
        case 14: {
            s16 new_var;

            _SsUtResolveADSR(scratch.adsr1, scratch.adsr2, &resolved);
            new_var = channel;
            switch (arg5) {
                case 4:
                    resolved.attackMode = 0;
                    resolved.attackRate = arg6;
                    break;
                case 5:
                    resolved.attackMode = 1;
                    resolved.attackRate = arg6;
                    break;
                case 6:
                    resolved.decayRate = arg6;
                    break;
                case 7:
                    resolved.sustainLevel = arg6;
                    break;
                case 8:
                    resolved.sustainMode = 0;
                    resolved.sustainRate = arg6;
                    break;
                case 9:
                    resolved.sustainMode = 1;
                    resolved.sustainRate = arg6;
                    break;
                case 10:
                    resolved.releaseMode = 0;
                    resolved.releaseRate = arg6;
                    break;
                case 11:
                    resolved.releaseMode = 1;
                    resolved.releaseRate = arg6;
                    break;
                case 12: {
                    if (arg6 == 0) {
                        /* nothing -- falls to the shared check below */
                    } else if (arg6 < 0x40) {
                        resolved.sustainDir = 0;
                        break;
                    }
                    if ((u32)(arg6 - 0x40) < 0x40) {
                        resolved.sustainDir = 1;
                    }
                    break;
                }
                case 13:
                    scratch.vibT = arg6;
                    break;
                case 14:
                    scratch.porW = arg6;
                    break;
            }
            _SsUtBuildADSR(&resolved, &scratch.adsr1, &scratch.adsr2);
            SsUtSetVagAtr(new_var, slot, kind, &scratch);
            return;
        }
        case 15:
            SsUtSetReverbType(arg6);
            return;
        case 16:
            SsUtSetReverbDepth(arg6, arg6);
            return;
        case 17:
            SsUtSetReverbFeedback(arg6);
            return;
        case 18:
        case 19:
            SsUtSetReverbDelay(arg6);
            return;
        case 20:
        case 21:
        case 22:
        default:
            return;
    }
}

#if 1
void SetPitchBend(s16 a0, s16 a1) {
    SsScore *rec = &_ss_score[a0][a1];
    u8 *cursor = rec->unk4;
    s32 packed;
    u8 b;
    u8 vol;

    b = rec->unk12;
    rec->unk4 = cursor;
    packed = (a1 << 8) | a0;
    rec->unk4 = rec->unk4 + 1;
    vol = rec->unk2C[b];
    b = *cursor;
    SpuVmPitchBend(packed, rec->unk4C, vol, b);
    rec->unk88 = ReadDeltaValue(a0, a1);
}
#endif

/* Calls into other modules, typed from this call site: SpuVmSeqKeyOff
 * (libsnd_vmanager.c) takes the packed "(slot<<8)|channel", as in
 * libsnd_cres.c and libsnd_decre.c; _SsSndNextSep is Sony's libsnd/next. */
extern s32 SpuVmSeqKeyOff(s32 a0);
extern void _SsSndNextSep(s32 a0, s32 a1);

/* This unit's own reading of the same global libsnd_decre.c already reads
 * as `VBLANK_MINUS` (a tick-rate/PPQN-style constant) -- independent local
 * view, per project convention. */
extern u32 VBLANK_MINUS;

/* Meta-event handler, reached from GetSeqData's 0xFF ("running status
 * for a 0xF0 event") and new-status 0xF0 dispatch arms with `a2` = the
 * meta-event TYPE byte. Only two types are understood; everything else is
 * silently ignored:
 *
 * 0x2F (End of Track): bumps the repeat counter (unk48). unk46 == 0 means
 * "loop forever" -- rewind unk4 to the saved track start (unk8) and keep
 * going. Otherwise, while the counter is still under the limit (unk46),
 * rewind BOTH unk4 and unkC. Once the limit is reached, clear the
 * playback-state flags (unk90), rewind unkC one more time, and run the
 * stop-sequence callbacks (_SsSndNextSep gated on unk3C != 0xFF, then an
 * unconditional SpuVmSeqKeyOff notify) before priming unk88 from unk70 for
 * the next tick.
 *
 * 0x51 (Set Tempo): reads a 3-byte big-endian microseconds-per-quarter-note
 * value, converts it to a BPM-like rate (60000000 / value -- the standard
 * MIDI tempo formula) into unk8C, then recomputes the scheduling
 * threshold (unk6E/unk70) against unk4A and the global tick-rate constant
 * VBLANK_MINUS, in whichever of two regimes avoids losing precision to
 * integer truncation (the `else` regime also derives a rounding bit from
 * the division's remainder). unk6E doubles as a mode flag: -1 means
 * "unk70 holds the reciprocal-regime value", any other value means
 * "unk70 holds the same value unk6E does". */
#ifdef NON_MATCHING
/* NON_MATCHING: 211/213 words, length 2 short. Residue: register identity:
 * retail's second divu re-reads rec->unk4A with a plain `lh`, and this body
 * keeps the first read live in a register (docs/match-reports/GetMetaEvent.md).
 * The volatile on the word-sized unk8C, below, defeats GCC's div/mod fusion. */
void GetMetaEvent(s16 a0, s16 a1, u8 a2) {
    SsScore *rec = &_ss_score[a0][a1];

    if (a2 != 0x2F) {
        if (a2 != 0x51) {
            return;
        }
        {
            u8 *p = rec->unk4;
            s32 tempo;
            s32 bpm;
            u32 base;
            u32 divisor;

            rec->unk4 = p + 1;
            tempo = (s32)p[0] << 16;
            rec->unk4 = p + 2;
            tempo |= (s32)p[1] << 8;
            rec->unk4 = p + 3;
            tempo |= p[2];

            bpm = 60000000 / tempo;
            base = VBLANK_MINUS * 15;
            divisor = base * 4;
            rec->unk8C = bpm;
            if (rec->unk4A * rec->unk8C * 10 < divisor) {
                rec->unk6E = (VBLANK_MINUS * 600) / (rec->unk4A * rec->unk8C);
                rec->unk70 = rec->unk6E;
            } else {
                /* NON_MATCHING: only the word-sized field is volatile; that
                 * defeats GCC's div/mod fusion and leaves the plain `lh` for
                 * unk4A alone. */
                volatile s32 *pbpm = &rec->unk8C;
                s32 q = (rec->unk4A * *pbpm * 10) / divisor;
                s32 r = (rec->unk4A * *pbpm * 10) % divisor;

                rec->unk6E = -1;
                rec->unk70 = (base * 2 < r) ? q + 1 : q;
            }
        }
        rec->unk88 = ReadDeltaValue(a0, a1);
        return;
    }
    {
        u16 newCount = rec->unk48 + 1;
        s16 limit = rec->unk46;

        rec->unk48 = newCount;
        if (limit == 0) {
            rec->unk80 = 0;
            rec->unk27 = 0;
            rec->unk88 = 0;
            rec->unk4 = rec->unk8;
            return;
        }
        if ((s16)newCount < limit) {
            rec->unk80 = 0;
            rec->unk27 = 0;
            rec->unk88 = 0;
            rec->unk4 = rec->unk8;
            rec->unkC = rec->unk8;
            return;
        }
        _ss_score[a0][a1].unk90 &= ~1;
        _ss_score[a0][a1].unk90 &= ~8;
        _ss_score[a0][a1].unk90 &= ~2;
        _ss_score[a0][a1].unk90 |= 0x200;
        _ss_score[a0][a1].unk90 |= 0x4;
        rec->unkC = rec->unk8;
        rec->unk2B = 0;
        if (rec->unk3C != 0xFF) {
            _SsSndNextSep(rec->unk3C, rec->unk0);
            rec->unk2B = 0;
        }
        SpuVmSeqKeyOff((a1 << 8) | a0);
        rec->unk88 = rec->unk70;
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/libsnd_seqread", GetMetaEvent);
#endif

/* MATCHING: the `goto combine` keeps the single-byte and loop-exit `val`
 * writes as distinct arms reaching one merge point, the jump arm written
 * explicitly and the fall-through arm last, which gives both retail's
 * register. */
s32 ReadDeltaValue(s16 a0, s16 a1) {
    SsScore *rec = &_ss_score[a0][a1];
    u8 *cursor = rec->unk4;
    s32 acc;
    s32 val;
    s32 result;
    u8 nb;

    rec->unk4 = cursor + 1;
    acc = *cursor;
    if (acc == 0) {
        return 0;
    }
    if (!(acc & 0x80)) {
        val = acc * 4;
        goto combine;
    }
    acc &= 0x7F;
    do {
        cursor = rec->unk4;
        rec->unk4 = cursor + 1;
        nb = *cursor;
        acc = (acc << 7) + (nb & 0x7F);
    } while (nb & 0x80);
    val = acc * 4;
combine:
    result = (val + acc) * 2;
    rec->unk80 += result;
    return result;
}
