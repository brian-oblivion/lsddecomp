/*
 * libsnd_seqread -- Sony libsnd `seqread`: the SEQ event interpreter, the
 * whole module, its 18 functions in seqread.o's order. Every function here
 * is Sony's and keeps Sony's name: the game links libsnd 3.3's seqread with
 * one function changed (_SsSetControlChange), so the object cannot be
 * linked and the module is carried as C.
 *
 * Each sequence's play state is an SsScore (ss_score.h), reached as
 * _ss_score[access][seq]; the (a0, a1) pair the event handlers take is that
 * (access, seq) pair. Per-channel state (pan, program, volume) is indexed by
 * the event's MIDI channel, channel. SeqPlay is the per-tick scheduler.
 * GetSeqData decodes one event from the byte stream at rec->readPos and
 * dispatches it to NoteOn, SetProgramChange and SetPitchBend (the
 * channel-voice events), _SsSetControlChange (the MIDI CC dispatcher, which
 * routes to the Cont* handlers) and GetMetaEvent (End of Track and Set
 * Tempo). ReadDeltaValue reads the delta time that follows every event and
 * schedules the next one. The controller handlers edit the VAB in place
 * through Sony's SsUtGet/Set ProgAtr/VagAtr calls.
 */
#include "common.h"
#include "libsnd_internal.h"

/* libsnd/vmanager's pitch bend of every matching voice, declared with the
 * arguments as SetPitchBend passes them. */
/* MATCHING: the definition's (s16, s16, s16, u16) narrows the first argument, two extra words. */
s32 SpuVmPitchBend(s32 packed, s16 a1, u8 vol, u8 bend);

/* Shared delta-time decoder: reads a MIDI variable-length value (7 bits
 * per byte, big-endian, continuation bit first) from rec->readPos (advancing the cursor as it goes), scales the
 * decoded magnitude by 10, adds it to rec->ticksPlayed, and returns the scaled
 * delta.  A first byte of 0 is a sentinel for "no delta" -- returns 0
 * without touching rec->ticksPlayed at all. */
extern s32 ReadDeltaValue(s16 channel, s16 slot);

/* GetSeqData is defined below; SeqPlay's catch-up loop calls it with the
 * same (channel, slot) pair as every other helper in this file. */
extern void GetSeqData(s16 channel, s16 slot);

/* Catch-up scheduler tick.  When the re-armed counter is still reloading
 * its threshold (remain == 0) it copies the threshold rec->ticksPerCall into the
 * counter. */
void SeqPlay(s16 a0, s16 a1) {
    SsScore *rec = &_ss_score[a0][a1];
    s16 last = rec->ticksPerCall;
    s32 elapsed = rec->deltaLeft;
    s32 delta = elapsed - last;
    s16 remain;
    s32 sum;
    s32 step;
    s16 last2;

    if (delta > 0) {
        remain = rec->callsPerTick;
        if (remain > 0) {
            rec->callsPerTick = remain - 1;
            return;
        }
        if (remain == 0) {
            rec->callsPerTick = last;
            rec->deltaLeft = rec->deltaLeft - 1;
            return;
        }
        rec->deltaLeft = delta;
        return;
    }
    if (last < elapsed) {
        return;
    }
    sum = elapsed;
    for (;;) {
        GetSeqData(a0, a1);
        step = rec->deltaLeft;
        if (step != 0) {
            last2 = rec->ticksPerCall;
            sum += step;
            if (sum < last2) {
                continue;
            }
            rec->deltaLeft = sum - last2;
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
 * byte from rec->readPos (advancing the cursor); if it has the high bit set it
 * is a new MIDI-style status byte -- the low nibble becomes rec->channel (the
 * MIDI channel) and the high nibble selects which
 * kind of event follows, consuming however many further data bytes that
 * kind needs and recording the high nibble into rec->runningStatus as "running
 * status" (0xFF standing in for the 0xF0 kind).  If the high bit is clear,
 * the byte just read is itself the first DATA byte of a new event of
 * whichever kind rec->runningStatus last recorded (MIDI running status) -- same
 * dispatch, one fewer byte consumed since this byte already stood in for
 * the first data byte.
 *
 */
#ifdef NON_MATCHING
void GetSeqData(s16 a0, s16 a1) {
    SsScore *rec = &_ss_score[a0][a1];
    u8 *p;
    u8 raw;
    u8 note, vel;

    p = rec->readPos;
    rec->readPos = p + 1;
    raw = *p;
    if (raw & 0x80) {
        rec->channel = raw & 0xF;
        switch (raw & 0xF0) {
            case 0x90:
                p = rec->readPos;
                rec->runningStatus = 0x90;
                rec->readPos = p + 1;
                note = *p;
                rec->readPos = p + 2;
                vel = *(p + 1);
                rec->deltaLeft = ReadDeltaValue(a0, a1);
                NoteOn(a0, a1, note, vel);
                return;
            case 0xB0:
                p = rec->readPos;
                rec->runningStatus = 0xB0;
                rec->readPos = p + 1;
                note = *p;
                _SsSetControlChange(a0, a1, note);
                return;
            case 0xC0:
                p = rec->readPos;
                rec->runningStatus = 0xC0;
                rec->readPos = p + 1;
                note = *p;
                SetProgramChange(a0, a1, note);
                return;
            case 0xE0:
                rec->runningStatus = 0xE0;
                rec->readPos = rec->readPos + 1;
                SetPitchBend(a0, a1);
                return;
            case 0xF0:
                p = rec->readPos;
                rec->runningStatus = 0xFF;
                rec->channel = raw & 0xF;
                rec->readPos = p + 1;
                note = *p;
                GetMetaEvent(a0, a1, note);
                return;
            default:
                return;
        }
    } else {
        switch (rec->runningStatus) {
            case 0x90:
                vel = *rec->readPos;
                rec->readPos = rec->readPos + 1;
                rec->deltaLeft = ReadDeltaValue(a0, a1);
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
INCLUDE_ASM("asm/nonmatchings/psyq/libsnd_seqread", GetSeqData);
#endif

/* Note On: keys the note on at the velocity scaled by the channel volume,
 * or off for velocity 0; ignored while the sequence's left volume is 0. */
#ifdef NON_MATCHING
void NoteOn(s16 a0, s16 a1, s32 note, s32 vel) {
    SsScore *rec = &_ss_score[a0][a1];
    u8 channel = rec->channel;
    s16 channelVol = rec->channelVol[channel];
    s32 vol = ((u8)vel * (s32)channelVol) / 127;
    u16 seqVolL = rec->volL;
    u8 pan = rec->pan[channel];

    channelVol = vel;
    if (seqVolL == 0) {
        return;
    }
    if ((u8)vel != 0) {
        s16 packed = (a1 << 8) | a0;
        s16 vabId = rec->vabId;
        u8 program = rec->program[channel];
        SpuVmKeyOn(packed, vabId, program, (u8)note, (u16)vol, pan);
        rec->lastVelocity = (u8)channelVol;
    } else {
        s16 packed = (a1 << 8) | a0;
        s16 vabId = rec->vabId;
        u8 program = rec->program[channel];
        SpuVmKeyOff(packed, vabId, program, (u8)note);
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/psyq/libsnd_seqread", NoteOn);
#endif

void SetProgramChange(s16 a0, s16 a1, u8 a2) {
    SsScore *rec = &_ss_score[a0][a1];

    rec->program[rec->channel] = a2;
    rec->deltaLeft = ReadDeltaValue(a0, a1);
}

/* Forward declarations for the handlers defined later in this unit. */
extern void ContDataEntry(s16 a0, s16 a1, u8 a2);
extern void ContPortamento(s16 a0, s16 a1, s32 a2);
extern void ContNrpn1(s16 a0, s16 a1, u8 a2);
extern void ContNrpn2(s16 a0, s16 a1, u8 a2);
extern void ContRpn1(s16 a0, s16 a1, u8 a2);
extern void ContRpn2(s16 a0, s16 a1, u8 a2);
extern void ContResetAll(s16 a0, s16 a1);

/* Control-Change dispatcher: reads one data byte from the event stream
 * (the CC value) and routes on `a2`, the CC number, through a dense 0..121
 * switch. The controllers with dedicated handling are the standard MIDI
 * assignments: 0 bank-select MSB, 6 data-entry MSB, 7 volume, 10 pan, 11
 * expression, 64 sustain, 65 portamento, 91 reverb depth, 98/99 NRPN,
 * 100/101 RPN, 121 reset-all-controllers. Every arm except
 * 6/65/98/99/100/101/121 reads the next delta time (ReadDeltaValue); those
 * seven leave it to the handler they call.
 */
/* MATCHING: `packed` stays s32 (SpuVmSetVol masks it), and each case copies `offset` to its own u16. */
void _SsSetControlChange(s16 a0, s16 a1, u8 a2) {
    SsScore *rec = &_ss_score[a0][a1];
    u8 *p = rec->readPos;
    u8 offset = rec->channel;
    u8 val;

    rec->readPos = p + 1;
    val = *p;
    switch (a2) {
        case 0:
            rec->vabId = val;
            rec->deltaLeft = ReadDeltaValue(a0, a1);
            return;
        case 6:
            ContDataEntry(a0, a1, val);
            return;
        case 7: {
            u16 o = offset;
            s32 packed = (a1 << 8) | a0;

            SpuVmSetVol(packed, rec->vabId, rec->program[o], val, rec->pan[o]);
            rec->channelVol[o] = val;
            rec->deltaLeft = ReadDeltaValue(a0, a1);
            return;
        }
        case 10: {
            s32 packed = (a1 << 8) | a0;
            u16 o = offset;
            /* MATCHING: reaching program/pan through one base pointer keeps
             * retail's register allocation; rec->program[o] does not. */
            u8 *blk = (u8 *)rec + o;
            s16 wide = rec->channelVol[o];

            SpuVmSetVol(packed, rec->vabId, blk[offsetof(SsScore, program)], wide, val);
            blk[offsetof(SsScore, pan)] = val;
            rec->deltaLeft = ReadDeltaValue(a0, a1);
            return;
        }
        case 11: {
            u16 o = offset;

            SpuVmSetProgVol(rec->vabId, rec->program[o], val);
            SpuVmSetVol((a1 << 8) | a0, rec->vabId, rec->program[o], rec->channelVol[o], rec->pan[o]);
            rec->deltaLeft = ReadDeltaValue(a0, a1);
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
    rec->deltaLeft = ReadDeltaValue(a0, a1);
}

/* The three per-controller VagAtr editors below share one shape: fetch the
 * channel's program with SsUtGetProgAtr, then for each of its `tones` read
 * the tone's VagAtr, overwrite one field from the controller value and
 * write it back. `tones` is re-read on every iteration, since the SsUt
 * calls may change the ProgAtr.
 *
 * ContModulation: the value becomes every tone's vibrato depth (vibW).
 * Neither it nor ContPortaTime has a caller in this executable. */
void ContModulation(s16 a0, s16 a1, u8 a2) {
    SsScore *rec = &_ss_score[a0][a1];
    u8 offset;
    ProgAtr list;
    VagAtr scratch;
    s32 i;

    SsUtGetProgAtr(rec->vabId, rec->program[offset = rec->channel], &list);
    for (i = 0; i < list.tones; i++) {
        SsUtGetVagAtr(rec->vabId, rec->program[offset], (s16)i, &scratch);
        scratch.vibW = a2;
        SsUtSetVagAtr(rec->vabId, rec->program[offset], (s16)i, &scratch);
    }
    rec->deltaLeft = ReadDeltaValue(a0, a1);
}

/* The value becomes every tone's portamento time (porT). */
void ContPortaTime(s16 a0, s16 a1, u8 a2) {
    SsScore *rec = &_ss_score[a0][a1];
    u8 offset;
    ProgAtr list;
    VagAtr scratch;
    s32 i;

    SsUtGetProgAtr(rec->vabId, rec->program[offset = rec->channel], &list);
    for (i = 0; i < list.tones; i++) {
        SsUtGetVagAtr(rec->vabId, rec->program[offset], (s16)i, &scratch);
        scratch.porT = a2;
        SsUtSetVagAtr(rec->vabId, rec->program[offset], (s16)i, &scratch);
    }
    rec->deltaLeft = ReadDeltaValue(a0, a1);
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

    SsUtGetProgAtr(rec->vabId, rec->program[offset = rec->channel], &list);
    for (i = 0; i < list.tones; i++) {
        SsUtGetVagAtr(rec->vabId, rec->program[offset], (s16)i, &scratch);
        if ((u8)a2 < 0x40) {
            scratch.mode = 2;
        } else {
            wrap = 0xC0;
            if ((u8)(a2 + wrap) < 0x40) {
                scratch.mode = 0;
            }
        }
        SsUtSetVagAtr(rec->vabId, rec->program[offset], (s16)i, &scratch);
    }
    rec->deltaLeft = ReadDeltaValue(a0, a1);
}

void ContResetAll(s16 a0, s16 a1) {
    SsScore *rec = &_ss_score[a0][a1];

    SsUtReverbOff();
    SpuVmDamperOff();

    rec->program[rec->channel] = rec->channel;
    rec->rpnLsb = 0;
    rec->rpnMsb = 0;
    rec->channelVol[rec->channel] = 0x7F;
    rec->pan[rec->channel] = 0x40;
    rec->deltaLeft = ReadDeltaValue(a0, a1);
}

/* CC98 (NRPN LSB). While a loop start set by ContNrpn2's kind 0x14 still
 * waits for its count (loopOpen == 1, loopCountSet == 0), the value becomes the loop
 * count loopCount. Otherwise, unless the current NRPN kind is 0x14 or 0x1E, it
 * is cached in nrpnLsb and bumps the nrpnBytes step counter. Under kind 0x28 the
 * value also goes to the sequence's mark callback. */
void ContNrpn1(s16 a0, s16 a1, u8 a2) {
    SsScore *rec = &_ss_score[a0][a1];
    u8 kind;
    SsMarkCallbackProc fn;

    if (rec->loopOpen == 1 && rec->loopCountSet == 0) {
        rec->loopCount = a2;
        rec->loopCountSet = 1;
    } else {
        kind = rec->nrpnMsb;
        if (kind != 0x1E && kind != 0x14) {
            rec->nrpnLsb = a2;
            rec->nrpnBytes = rec->nrpnBytes + 1;
        }
    }
    if (rec->nrpnMsb == 0x28) {
        s16 ch = a0;
        s16 sl = a1;

        fn = _ss_MarkCallback[ch][sl];
        if (fn != NULL) {
            fn(ch, sl, a2);
        }
    }
    rec->deltaLeft = ReadDeltaValue(a0, a1);
}

void ContNrpn2(s16 a0, s16 a1, u8 a2) {
    SsScore *rec = &_ss_score[a0][a1];
    u8 kind = a2;
    s32 result;

    switch (kind) {
        case 0x14:
            rec->nrpnMsb = a2;
            rec->loopOpen = 1;
            result = ReadDeltaValue(a0, a1);
            rec->deltaLeft = result;
            rec->loopPos = rec->readPos;
            return;
        case 0x1E:
            rec->nrpnMsb = a2;
            if (rec->loopCount == 0) {
                rec->loopCountSet = 0;
                rec->deltaLeft = ReadDeltaValue(a0, a1);
                return;
            }
            if (rec->loopCount < 0x7F) {
                rec->loopCount--;
                result = ReadDeltaValue(a0, a1);
                rec->deltaLeft = result;
                if (rec->loopCount != 0) {
                    rec->readPos = rec->loopPos;
                } else {
                    rec->loopCountSet = 0;
                }
                return;
            }
            ReadDeltaValue(a0, a1);
            rec->readPos = rec->loopPos;
            rec->deltaLeft = 0;
            return;
        default:
            rec->nrpnMsb = a2;
            rec->nrpnBytes = rec->nrpnBytes + 1;
            rec->deltaLeft = ReadDeltaValue(a0, a1);
            return;
    }
}

void ContRpn1(s16 a0, s16 a1, u8 a2) {
    SsScore *rec = &_ss_score[a0][a1];
    u8 counter = rec->rpnBytes;

    rec->rpnLsb = a2;
    counter = counter + 1;
    rec->rpnBytes = counter;
    rec->deltaLeft = ReadDeltaValue(a0, a1);
}

void ContRpn2(s16 a0, s16 a1, u8 a2) {
    SsScore *rec = &_ss_score[a0][a1];
    u8 counter = rec->rpnBytes;

    rec->rpnMsb = a2;
    counter = counter + 1;
    rec->rpnBytes = counter;
    rec->deltaLeft = ReadDeltaValue(a0, a1);
}

/** @brief The SPU envelope registers ADSR1/ADSR2 (VagAtr.adsr1/adsr2) split
 * into their bit fields: _SsUtResolveADSR unpacks a pair into one, and
 * _SsUtBuildADSR packs one back. The rates and the level are the field
 * values shifted down; the four modes hold the masked bit as-is (nonzero
 * means exponential, or decreasing for sustainDir). */
typedef struct {
    s16 attackRate;   /**< ADSR1 bits 14-8 */
    s16 decayRate;    /**< ADSR1 bits 7-4 */
    s16 sustainLevel; /**< ADSR1 bits 3-0 */
    s16 sustainRate;  /**< ADSR2 bits 12-6 */
    s16 releaseRate;  /**< ADSR2 bits 4-0 */
    s16 attackMode;   /**< ADSR1 bit 15 */
    s16 sustainMode;  /**< ADSR2 bit 15 */
    s16 releaseMode;  /**< ADSR2 bit 5 */
    s16 sustainDir;   /**< ADSR2 bit 14 */
} AdsrFields;

/** @brief SsUtGetProgAtr's fill at function entry. From +0x10 the same
 * memory is both the VagAtr buffer the rpnBytes==2 loops hand to
 * SsUtGet/SetVagAtr and, with the 18 bytes after it, the two by-value
 * arguments of Snd_setVabAttr. */
typedef struct {
    ProgAtr prog; /**< +0x00: SsUtGetProgAtr's fill */

    VagAtr vag; /**< +0x10: passed by value to Snd_setVabAttr, and the
                 * SsUtGet/SetVagAtr buffer of the rpnBytes==2 loops */

    AdsrFields adsr; /**< +0x30: passed by value to Snd_setVabAttr */
} DataEntryLocals;

/* Snd_setVabAttr is defined later in this unit; this is its signature. */
extern void Snd_setVabAttr(s16 channel, s16 slot, s16 kind, VagAtr scratch, AdsrFields resolved,
                           s16 arg5, u8 arg6);

/* CC6 (data entry MSB). After a loop start the value is the loop count;
 * outside the loop NRPNs it is stored as nrpnLsb and counted. Otherwise,
 * once both RPN bytes are in, RPN 0 sets every tone's pitch-bend range
 * (RPN 1 and 2 rewrite each tone unchanged); once both NRPN bytes are in,
 * Snd_setVabAttr applies it to tone nrpnMsb (every tone for 0x10). */
#ifdef NON_MATCHING
void ContDataEntry(s16 a0, s16 a1, u8 a2) {
    s16 ch = a0;
    s16 slot = a1;
    SsScore *rec = &_ss_score[ch][slot];
    u8 off = rec->channel;
    DataEntryLocals list;
    s32 i;
    u8 kind;

    SsUtGetProgAtr(rec->vabId, rec->program[off], &list);

    if (rec->loopOpen == 1 && rec->loopCountSet == 0) {
        rec->loopCount = a2;
        rec->loopCountSet = 1;
        rec->deltaLeft = ReadDeltaValue(ch, slot);
        return;
    }
    if (rec->nrpnMsb != 0x1E && rec->nrpnMsb != 0x14) {
        rec->nrpnLsb = a2;
        rec->nrpnBytes = rec->nrpnBytes + 1;
        rec->deltaLeft = ReadDeltaValue(ch, slot);
        return;
    }
    if (rec->rpnBytes == 2) {
        if (rec->rpnLsb == 0 && rec->rpnMsb == 0) {
            for (i = 0; i < list.prog.tones; i++) {
                SsUtGetVagAtr(rec->vabId, rec->program[off], i, &list.vag);
                list.vag.pbmin = list.vag.pbmax = a2 & 0x7F;
                SsUtSetVagAtr(rec->vabId, rec->program[off], i, &list.vag);
            }
        }
        if (rec->rpnLsb == 1 && rec->rpnMsb == 0) {
            s32 unused; /* computed and never read */
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
                SsUtGetVagAtr(rec->vabId, rec->program[off], i, &list.vag);
                list.vag.shift = list.vag.shift;
                SsUtSetVagAtr(rec->vabId, rec->program[off], i, &list.vag);
            }
        }
        if (rec->rpnLsb == 2 && rec->rpnMsb == 0) {
            s32 unused; /* computed and never read */
            if ((u8)(a2 - 0x40) < 0x40) {
                unused = ((a2 & 0xFF) * 25) << 8;
            } else {
                unused = 0;
            }
            for (i = 0; i < list.prog.tones; i++) {
                SsUtGetVagAtr(rec->vabId, rec->program[off], i, &list.vag);
                list.vag.center = list.vag.center;
                SsUtSetVagAtr(rec->vabId, rec->program[off], i, &list.vag);
            }
        }
        rec->deltaLeft = ReadDeltaValue(ch, slot);
        rec->rpnBytes = 0;
        return;
    }
    if (rec->nrpnBytes == 2) {
        kind = rec->nrpnMsb;
        if (kind == 0x10) {
            for (i = 0; i < list.prog.tones; i++) {
                Snd_setVabAttr(rec->vabId, rec->program[off], i, list.vag, list.adsr, rec->nrpnLsb,
                               a2 & 0xFF);
            }
        } else {
            Snd_setVabAttr(rec->vabId, rec->program[off], (s16)kind, list.vag, list.adsr,
                           rec->nrpnLsb, a2 & 0xFF);
        }
        rec->deltaLeft = ReadDeltaValue(ch, slot);
        rec->nrpnBytes = 0;
        return;
    }
    rec->deltaLeft = ReadDeltaValue(ch, slot);
}
#else
INCLUDE_ASM("asm/nonmatchings/psyq/libsnd_seqread", ContDataEntry);
#endif

/* Both Sony's libsnd/adsr: */
extern void _SsUtResolveADSR(s32 a0, s32 a1, AdsrFields *out);
extern void _SsUtBuildADSR(AdsrFields *in, u16 *adsr1, u16 *adsr2);

/* MIDI CC91 (Reverb Depth)/98/99/100/101 (NRPN/RPN LSB/MSB) and friends'
 * per-parameter handler, reached only from ContDataEntry, through a nested
 * switch. `kind` selects which VagAtr (per program-tone) is
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

void SetPitchBend(s16 a0, s16 a1) {
    SsScore *rec = &_ss_score[a0][a1];
    u8 *cursor = rec->readPos;
    s32 packed;
    u8 b;
    u8 vol;

    b = rec->channel;
    rec->readPos = cursor;
    packed = (a1 << 8) | a0;
    rec->readPos = rec->readPos + 1;
    vol = rec->program[b];
    b = *cursor;
    SpuVmPitchBend(packed, rec->vabId, vol, b);
    rec->deltaLeft = ReadDeltaValue(a0, a1);
}

/* libsnd/next. */
extern void _SsSndNextSep(s32 a0, s32 a1);

/* Meta-event handler, reached from GetSeqData's 0xFF and 0xF0 arms with
 * `a2` = the meta-event type byte. Only two types are understood; the rest
 * are ignored.
 *
 * 0x2F (End of Track): counts playsDone up. playCount == 0 loops forever:
 * readPos goes back to trackStart. Below the limit, readPos and loopPos
 * both rewind. At the limit the sequence stops: the playback flags change,
 * loopPos rewinds, _SsSndNextSep starts the next sequence (unless
 * nextSepAccess is 0xFF), SpuVmSeqKeyOff releases its voices, and
 * deltaLeft is primed from ticksPerCall.
 *
 * 0x51 (Set Tempo): reads a 3-byte big-endian microseconds-per-quarter-note
 * value into tempo as 60000000 / value (beats per minute), then recomputes
 * callsPerTick/ticksPerCall from ticksPerBeat and VBLANK_MINUS in whichever
 * of two regimes keeps the precision (the second rounds by the division's
 * remainder). callsPerTick -1 means ticksPerCall holds the ticks per call;
 * any other value means ticksPerCall holds the same value as callsPerTick. */
#ifdef NON_MATCHING
void GetMetaEvent(s16 a0, s16 a1, u8 a2) {
    SsScore *rec = &_ss_score[a0][a1];

    if (a2 != 0x2F) {
        if (a2 != 0x51) {
            return;
        }
        {
            u8 *p = rec->readPos;
            s32 tempo;
            s32 bpm;
            u32 base;
            u32 divisor;

            rec->readPos = p + 1;
            tempo = (s32)p[0] << 16;
            rec->readPos = p + 2;
            tempo |= (s32)p[1] << 8;
            rec->readPos = p + 3;
            tempo |= p[2];

            bpm = 60000000 / tempo;
            base = VBLANK_MINUS * 15;
            divisor = base * 4;
            rec->tempo = bpm;
            if (rec->ticksPerBeat * rec->tempo * 10 < divisor) {
                rec->callsPerTick = (VBLANK_MINUS * 600) / (rec->ticksPerBeat * rec->tempo);
                rec->ticksPerCall = rec->callsPerTick;
            } else {
                volatile s32 *pbpm = &rec->tempo;
                s32 q = (rec->ticksPerBeat * *pbpm * 10) / divisor;
                s32 r = (rec->ticksPerBeat * *pbpm * 10) % divisor;

                rec->callsPerTick = -1;
                rec->ticksPerCall = (base * 2 < r) ? q + 1 : q;
            }
        }
        rec->deltaLeft = ReadDeltaValue(a0, a1);
        return;
    }
    {
        u16 newCount = rec->playsDone + 1;
        s16 limit = rec->playCount;

        rec->playsDone = newCount;
        if (limit == 0) {
            rec->ticksPlayed = 0;
            rec->loopOpen = 0;
            rec->deltaLeft = 0;
            rec->readPos = rec->trackStart;
            return;
        }
        if ((s16)newCount < limit) {
            rec->ticksPlayed = 0;
            rec->loopOpen = 0;
            rec->deltaLeft = 0;
            rec->readPos = rec->trackStart;
            rec->loopPos = rec->trackStart;
            return;
        }
        _ss_score[a0][a1].flags &= ~1;
        _ss_score[a0][a1].flags &= ~8;
        _ss_score[a0][a1].flags &= ~2;
        _ss_score[a0][a1].flags |= 0x200;
        _ss_score[a0][a1].flags |= 0x4;
        rec->loopPos = rec->trackStart;
        rec->unk2B = 0;
        if (rec->nextSepAccess != 0xFF) {
            _SsSndNextSep(rec->nextSepAccess, rec->nextSepSeq);
            rec->unk2B = 0;
        }
        SpuVmSeqKeyOff((a1 << 8) | a0);
        rec->deltaLeft = rec->ticksPerCall;
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/psyq/libsnd_seqread", GetMetaEvent);
#endif

/* MATCHING: the `goto combine` keeps the two `val` writes as distinct arms into one merge point. */
s32 ReadDeltaValue(s16 a0, s16 a1) {
    SsScore *rec = &_ss_score[a0][a1];
    u8 *cursor = rec->readPos;
    s32 acc;
    s32 val;
    s32 result;
    u8 nb;

    rec->readPos = cursor + 1;
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
        cursor = rec->readPos;
        rec->readPos = cursor + 1;
        nb = *cursor;
        acc = (acc << 7) + (nb & 0x7F);
    } while (nb & 0x80);
    val = acc * 4;
combine:
    result = (val + acc) * 2;
    rec->ticksPlayed += result;
    return result;
}
