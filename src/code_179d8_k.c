/*
 * code_179d8_k -- functions 238..255 of the original 274-function code_179d8
 * monolith, 0x24938..0x2673C (vram 0x80034138..0x80035F3C).  Carved round 24
 * (2026-09-08) out of what had been the `code_179d8_tail` asm remainder,
 * which this unit consumes WHOLE -- there is no remainder left on either
 * side (code_179d8_i in front, code_179d8_f behind).
 *
 * WHY IT WAS UNCARVED FOR SIX ROUNDS, AND WHY THAT VERDICT IS DEAD.
 * The splat comment on the old remainder read "17 of its 18 functions are
 * addiu-$at blocked".  `addiu_at` was RESOLVED in round 21 (maspsx
 * `--addiu-at`; docs/research/addiu-at-blocker.md), so that census measured
 * an obstruction that no longer exists.  Re-censused 2026-09-08 with
 * `python3 tools/nearmiss.py`'s four screens, canonical shell forms
 * (`grep -A2` FORWARD for nop_mflo_mfhi -- the direction is load-bearing):
 *
 *   18 of 18 CLEAN.  Zero gp_rel, zero nop_mflo_mfhi, zero `jr $t2`
 *   trampolines.  This is the single best carve left in the executable.
 *
 * Sizes, cheapest first -- six functions at 31..51 words, which is the
 * cheap seam the `fresh` queue had run out of:
 *   func_80034614  31w   func_800350D8  31w   func_80035154  31w
 *   func_80035A7C  44w   func_80035E80  47w   func_80034D90  51w
 *   func_80034138  69w   func_800344FC  70w   func_80034E5C  77w
 *   func_800349B0  79w   func_80034AEC  79w   func_80034F90  82w
 *   func_80034C28  90w   func_8003424C 172w   func_800357B0 179w
 *   func_80034690 200w   func_80035B2C 213w   func_800351D0 376w
 *
 * THIS UNIT OWNS THREE SWITCH JUMP TABLES, not the two the old remainder
 * comment claimed: func_80034690 -> jtbl_80010CF0, and func_800357B0 ->
 * jtbl_80010ED8 AND jtbl_80010F38 (a double switch).  The 0x14F0 rodata
 * slot holds exactly those three tables and nothing else, is referenced
 * from nowhere outside this unit, and is attached whole in the splat yaml.
 * You do not need to do anything about it -- but if you match either
 * function, remember a `%lo(jtbl_*)` load is ordinary matchable code now.
 *
 * Boundary checks at carve time, both sides: no function has more than one
 * `addiu $sp, $sp, -N`, every one ends in its own `jr $ra`, zero `alabel`,
 * and the one frameless function (func_80035E80) opens on
 * `sll $a0, $a0, 16` -- leaf argument narrowing, not a caller-frame read.
 *
 * Expect this slice to span more than one class; a ~20-function slice cut
 * at ROM-address boundaries has no reason to align with class boundaries.
 * Identify each with tools/classtable.py rather than assuming the unit has
 * one.  Expect low-level driver-shaped code rather than class-framework
 * code, as elsewhere in code_179d8; confirm, do not assume.
 */
#include "common.h"

/* Cross-unit calls, typed per-call-site from the registers loaded before
 * each `jal` -- none of these callees have an established prototype from
 * their own unit's side yet except where noted, so these are local
 * guesses, not authoritative.  Per this project's convention, a prototype
 * for a function ANOTHER unit defines stays in this .c, not in a shared
 * header. */
extern void func_8002F610(s32 a0, s16 a1, u8 a2, u8 a3);   /* code_179d8_m, not yet matched: local guess */
extern s32 func_8002FAC4(s32 a0, s16 a1, s16 a2, u16 a3, u16 a4, u16 a5);  /* code_179d8_m, not yet matched: local guess, matches code_179d8_j's independent reading of the same call shape */
extern s32 func_800300D0(s32 a0, s16 a1, s16 a2, u16 a3);   /* code_179d8_m, not yet matched: local guess, ditto */
/* Psy-Q libsnd, linked from the SDK objects (round 34): `ut_rev` and
 * `vm_doff`. Both were carried as matched C in code_179d8_f.c until that
 * unit's prefix was given back to Sony; these are local views, as a Psy-Q
 * prototype must never go into a header this unit's siblings share. */
extern s32 SsUtReverbOff(void);
extern void SpuVmDamperOff(void);

/* A 172 (0xAC)-byte record; D_800902E8 is an array of pointers to arrays of
 * these, indexed [channel][slot]-style, same array documented from
 * code_179d8_f/_i/_j's own independent local readings -- see those units'
 * Entry90902E8 for a different reduced view of the same object; each unit
 * keeps its own per project convention.
 *
 * This unit's own reading is a low-level per-channel "sequencer voice"
 * playback record: unk4 is a cursor into a byte-encoded event stream,
 * unk80 an accumulated tick position, unk88 a scratch slot every function
 * in this file stores its own last computed value into (a pointer in one
 * caller, an updated counter in others -- genuinely overloaded, not a
 * misreading), and unk12 a BYTE OFFSET (already scaled, not an index) to
 * whichever of several embedded state blocks is presently active; several
 * functions here dereference `(u8 *)rec + rec->unk12` and then apply a
 * further FIXED displacement (0x17, 0x2C) from that computed base, which
 * is why those two fields are not named as fixed struct members below --
 * their address is only known at runtime, exactly the case this project's
 * pointer-arithmetic convention is for. */
typedef struct {
    u8 unk0;    /* +0x0: a byte passed alongside unk3C to _SsSndNextSep on
                 * end-of-track cleanup */
    u8 pad1[0x4 - 0x1];
    u8 *unk4;   /* +0x4: cursor into a byte-encoded (7-bit VLQ) event stream */
    u8 *unk8;   /* +0x8: saved "track start" cursor, restored into unk4 (and
                 * sometimes unkC) on end-of-track / repeat */
    u8 *unkC;   /* +0xC: a saved backup of unk4, restored into it on a "resume" path */
    u8 unk10;   /* +0x10: one-shot latch, set once a "kind 1" retrigger fires */
    u8 unk11;   /* +0x11: cached MIDI-style running-status byte (0xFF standing
                 * in for a 0xF0 "meta" status) -- read back to interpret a
                 * later event byte that has its high bit clear */
    u8 unk12;   /* +0x12: byte offset to the active embedded state block */
    u8 unk13;   /* +0x13 */
    u8 unk14;   /* +0x14 */
    u8 unk15;   /* +0x15: a cached byte, written from the "default kind" path */
    u8 unk16;   /* +0x16: event-kind selector (compared against 0x14/0x1E/0x28) */
    u8 pad17[0x27 - 0x17];
    u8 unk27;   /* +0x27: dispatch-mode selector (compared against 1) */
    u8 unk28;   /* +0x28: a cached byte, written from the "mode 1" latch path */
    u8 unk29;   /* +0x29: a retrigger/step counter */
    u8 unk2A;   /* +0x2A: a second, independent retrigger/step counter */
    u8 unk2B;   /* +0x2B: cleared on end-of-track stop (redundant double store
                 * in retail -- see func_80035B2C) */
    u8 pad2C[0x3C - 0x2C];
    u8 unk3C;   /* +0x3C: compared against 0xFF; a "track/channel select" byte
                 * passed to _SsSndNextSep on end-of-track stop */
    u8 pad3D[0x46 - 0x3D];
    s16 unk46;  /* +0x46: repeat-count LIMIT (0 = loop forever) */
    u16 unk48;  /* +0x48: repeat COUNTER, incremented per end-of-track;
                 * sign-checked via explicit (s16) cast at its compare site,
                 * same idiom as code_179d8_i.c's unk40 */
    s16 unk4A;  /* +0x4A: a per-tick scaling factor used in the tempo/rate
                 * recompute on a Set-Tempo meta event */
    s16 unk4C;  /* +0x4C */
    u8 pad4E[0x6E - 0x4E];
    s16 unk6E;  /* +0x6E: a repeat/skip counter, decremented per catch-up tick */
    s16 unk70;  /* +0x70: next scheduling threshold, compared against unk88 */
    u8 pad72[0x74 - 0x72];
    u16 unk74;  /* +0x74: nonzero-gated dispatch enable flag */
    u8 pad76[0x80 - 0x76];
    s32 unk80;  /* +0x80: accumulated tick position */
    u8 pad84[0x88 - 0x84];
    s32 unk88;  /* +0x88: last-value scratch, overloaded per call site */
    s32 unk8C;  /* +0x8C: recomputed BPM (60000000 / microseconds-per-quarter),
                 * cached from the last Set-Tempo meta event */
    u32 unk90;  /* +0x90: playback state flags */
    u8 pad94[0xA8 - 0x94];
    s16 unkA8;  /* +0xA8: a masked-byte parameter cached from a dispatch call */
    u8 padAA[0xAC - 0xAA];
} Entry90902E8;
extern Entry90902E8 *D_800902E8[];

/* Shared VLQ-style delta-time decoder: reads a 7-bit-per-byte
 * little-endian... no, MIDI-style BIG-endian continuation-bit-first
 * encoding from rec->unk4 (advancing the cursor as it goes), scales the
 * decoded magnitude by 10, adds it to rec->unk80, and returns the scaled
 * delta.  A first byte of 0 is a sentinel for "no delta" -- returns 0
 * without touching rec->unk80 at all. */
extern s32 func_80035E80(s16 channel, s16 slot);

/* Forward declaration for a sibling function defined later in THIS unit
 * (func_8003424C, still INCLUDE_ASM) -- called from func_80034138's
 * catch-up loop below with the same (channel, slot) pair as every other
 * helper in this file; its own return/side effects are not yet
 * characterised since it has not been matched. */
extern void func_8003424C(s16 channel, s16 slot);

/* STALL -- see docs/match-reports/func_80034138.md. length exact 69/69,
 * 66/69 raw word-match, first real diff at word 22: two pure scheduling
 * residues (a load-pair order swap and one delay-slot filler choice),
 * not a logic or CFG difference. */
#if 0
void func_80034138(s16 a0, s16 a1, s16 a2)
{
    Entry90902E8 *rec = &D_800902E8[a0][a1];
    s32 dead[2];
    s16 last = rec->unk70;
    s32 elapsed = rec->unk88;
    s32 delta = elapsed - last;
    s16 remain;
    s32 sum;
    s32 step;
    s16 last2;

    if (0) {
        dead[0] = 0;
        dead[1] = 0;
    }
    if (delta > 0) {
        remain = rec->unk6E;
        if (remain > 0) {
            rec->unk6E = remain - 1;
            return;
        }
        if (remain == 0) {
            rec->unk6E = a2;
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
        func_8003424C(a0, a1);
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
#endif
INCLUDE_ASM("asm/nonmatchings/code_179d8_k", func_80034138);

/* Forward declarations for sibling functions defined later in THIS unit,
 * needed because func_8003424C dispatches to them by MIDI-style status
 * byte before they appear in ROM-address order below.  func_800344FC's
 * signature is the one already established in its own (still-stalled) STALL
 * comment above; func_80034690's and func_80035B2C's are this function's own
 * reading, derived from the registers loaded before each call below. */
extern void func_800344FC(s16 a0, s16 a1, s32 a2, s32 a3);
extern void func_80034614(s16 a0, s16 a1, u8 a2);
extern void func_80034690(s16 a0, s16 a1, u8 a2);
extern void func_80035A7C(s16 a0, s16 a1);
extern void func_80035B2C(s16 a0, s16 a1, u8 a2);

/* A per-channel/slot "sequencer voice" event-stream byte reader.  Reads one
 * byte from rec->unk4 (advancing the cursor); if it has the high bit set it
 * is a new MIDI-style status byte -- the low nibble becomes rec->unk12 (the
 * active embedded-state-block offset) and the high nibble selects which
 * kind of event follows, consuming however many further data bytes that
 * kind needs and recording the high nibble into rec->unk11 as "running
 * status" (0xFF standing in for the 0xF0 kind).  If the high bit is clear,
 * the byte just read is itself the first DATA byte of a new event of
 * whichever kind rec->unk11 last recorded (MIDI running status) -- same
 * dispatch, one fewer byte consumed since this byte already stood in for
 * the first data byte.
 *
 * STALL -- see docs/match-reports/func_8003424C.md. length exact 172/172,
 * 122/172 raw word-match, first real diff at word 2: a pure register-identity
 * swap (retail's widened "channel" lives in $s4 and its per-case data byte in
 * $s3; this C's compiles the same roles into $s3/$s4 the other way around).
 * CLAUDE.md's register-identity STALL rule -- reshaping tried and did not
 * move it (see report for the full list of variants). */
#if 0
void func_8003424C(s16 a0, s16 a1)
{
    Entry90902E8 *rec = &D_800902E8[a0][a1];
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
            rec->unk88 = func_80035E80(a0, a1);
            func_800344FC(a0, a1, note, vel);
            return;
        case 0xB0:
            p = rec->unk4;
            rec->unk11 = 0xB0;
            rec->unk4 = p + 1;
            note = *p;
            func_80034690(a0, a1, note);
            return;
        case 0xC0:
            p = rec->unk4;
            rec->unk11 = 0xC0;
            rec->unk4 = p + 1;
            note = *p;
            func_80034614(a0, a1, note);
            return;
        case 0xE0:
            rec->unk11 = 0xE0;
            rec->unk4 = rec->unk4 + 1;
            func_80035A7C(a0, a1);
            return;
        case 0xF0:
            p = rec->unk4;
            rec->unk11 = 0xFF;
            rec->unk12 = raw & 0xF;
            rec->unk4 = p + 1;
            note = *p;
            func_80035B2C(a0, a1, note);
            return;
        default:
            return;
        }
    } else {
        switch (rec->unk11) {
        case 0x90:
            vel = *rec->unk4;
            rec->unk4 = rec->unk4 + 1;
            rec->unk88 = func_80035E80(a0, a1);
            func_800344FC(a0, a1, raw, vel);
            return;
        case 0xB0:
            func_80034690(a0, a1, raw);
            return;
        case 0xC0:
            func_80034614(a0, a1, raw);
            return;
        case 0xE0:
            func_80035A7C(a0, a1);
            return;
        case 0xFF:
            func_80035B2C(a0, a1, raw);
            return;
        default:
            return;
        }
    }
}
#endif
INCLUDE_ASM("asm/nonmatchings/code_179d8_k", func_8003424C);

/* STALL -- see docs/match-reports/func_800344FC.md. length exact 70/70,
 * 61/70 raw word-match (round 33, runner bravo -- up from 44/70), first
 * real diff at word 1: a register-identity rename ($t0<->$a2 for the a0
 * copy kept live across the two calls, $s1<->$t0 for the masked-a3 copy),
 * not a logic or CFG difference -- CLAUDE.md's register-identity STALL
 * rule. */
#if 0
void func_800344FC(s16 a0, s16 a1, s32 a2, s32 a3)
{
    Entry90902E8 *rec = &D_800902E8[a0][a1];
    u8 offset = rec->unk12;
    s16 speed = *(s16 *)((u8 *)rec + 0x4E + offset * 2);
    s32 divided = ((u8)a3 * (s32)speed) / 127;
    u8 *ptr = offset + (u8 *)rec;
    u16 flag = rec->unk74;
    u8 status = ptr[0x17];

    speed = a3;
    if (flag == 0) {
        return;
    }
    if ((u8)a3 != 0) {
        s16 packed = (a1 << 8) | a0;
        s16 note = rec->unk4C;
        u8 vol = ptr[0x2C];
        func_8002FAC4(packed, note, vol, (u8)a3, (u16)divided, status);
        rec->unkA8 = (u8)speed;
    } else {
        s16 packed = (a1 << 8) | a0;
        s16 note = rec->unk4C;
        u8 vol = ptr[0x2C];
        func_800300D0(packed, note, vol, (u8)a3);
    }
}
#endif
INCLUDE_ASM("asm/nonmatchings/code_179d8_k", func_800344FC);

void func_80034614(s16 a0, s16 a1, u8 a2)
{
    Entry90902E8 *rec = &D_800902E8[a0][a1];
    u8 *p = (u8 *)rec + rec->unk12;

    p[0x2C] = a2;
    rec->unk88 = func_80035E80(a0, a1);
}

/* Cross-unit calls, local guesses per this project's convention (a prototype
 * for a function another unit defines stays in this .c). SpuVmDamperOn and
 * SsUtSetReverbDepth are Psy-Q libsnd (`vm_don`, `ut_rev`), linked from the
 * SDK objects since round 34 -- never write C for them.  So is
 * `SpuVmSetProgVol` (`libsnd/vm_prog`, 3.6), which was code_179d8_j.c's
 * matched func_800307F0 until the same round; func_80030980 is still
 * INCLUDE_ASM
 * there, so its signature below is this call site's own reading -- a 5th
 * argument (the one spilling to the stack at 0x10($sp)) alongside the usual
 * "packed (slot<<8)|channel" first argument this file's siblings already
 * use. */
extern void SpuVmDamperOn(void);
extern void SsUtSetReverbDepth(s32 a0, s32 a1);
extern s32 SpuVmSetProgVol(s16 p0, s16 p1, s32 p2);
extern void func_80030980(s16 packed, s16 note, u8 vol, s32 arg3, s32 arg4);

/* Forward declarations for sibling functions defined later in THIS unit's
 * ROM-address order. func_800351D0's signature is this call site's own
 * reading; the rest are already established (matched, or from their own
 * STALL comments) elsewhere in this file. */
extern void func_800351D0(s16 a0, s16 a1, u8 a2);
extern void func_80034C28(s16 a0, s16 a1, s32 a2);
extern void func_80034E5C(s16 a0, s16 a1, u8 a2);
extern void func_80034F90(s16 a0, s16 a1, u8 a2);
extern void func_800350D8(s16 a0, s16 a1, u8 a2);
extern void func_80035154(s16 a0, s16 a1, u8 a2);
extern void func_80034D90(s16 a0, s16 a1);

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
 * func_80035E80; those seven `return` immediately instead.
 *
 * STALL -- see docs/match-reports/func_80034690.md. 5 words SHORT (195/200,
 * compiled length measured directly off build/src/code_179d8_k.c.o, not
 * from funcdiff which cannot read a meaningful word-match number once
 * length drifts). Every other case body's word count matches retail's
 * exactly (verified case by case); the two measured causes are (1) this
 * build allocates one FEWER callee-saved register overall (6 vs retail's
 * 7 -- "val" never gets its own persistent $s6), costing 2 words in the
 * prologue/epilogue, and (2) case 11 (CC 11, Expression) reaches the
 * shared combine-tail by JUMPING INTO retail's default-path widening
 * (saving 3 words) instead of duplicating its own full widening and
 * jumping straight to the call the way retail -- and this file's own
 * cases 7 and 10, which DO match exactly -- do. Register-identity /
 * tail-merge-choice residue, not a logic difference; see report for the
 * reshapes tried. */
#if 0
void func_80034690(s16 a0, s16 a1, u8 a2)
{
    Entry90902E8 *rec = &D_800902E8[a0][a1];
    u8 *p = rec->unk4;
    u8 offset = rec->unk12;
    u8 val;

    rec->unk4 = p + 1;
    val = *p;
    switch (a2) {
    case 0:
        rec->unk4C = val;
        rec->unk88 = func_80035E80(a0, a1);
        return;
    case 6:
        func_800351D0(a0, a1, val);
        return;
    case 7: {
        u8 *blk = (u8 *)rec + offset;
        s16 packed = (a1 << 8) | a0;

        func_80030980(packed, rec->unk4C, blk[0x2C], val, blk[0x17]);
        *(s16 *)((u8 *)rec + offset * 2 + 0x4E) = val;
        rec->unk88 = func_80035E80(a0, a1);
        return;
    }
    case 10: {
        u8 *blk = (u8 *)rec + offset;
        s16 packed = (a1 << 8) | a0;
        s16 wide = *(s16 *)((u8 *)rec + offset * 2 + 0x4E);

        func_80030980(packed, rec->unk4C, blk[0x2C], wide, val);
        blk[0x17] = val;
        rec->unk88 = func_80035E80(a0, a1);
        return;
    }
    case 11: {
        u8 *blk = (u8 *)rec + offset;

        SpuVmSetProgVol(rec->unk4C, blk[0x2C], val);
        func_80030980((a1 << 8) | a0, rec->unk4C, blk[0x2C],
                      *(s16 *)((u8 *)rec + offset * 2 + 0x4E), blk[0x17]);
        rec->unk88 = func_80035E80(a0, a1);
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
        func_80034C28(a0, a1, val);
        return;
    case 91:
        SsUtSetReverbDepth(val, val);
        break;
    case 98:
        func_80034E5C(a0, a1, val);
        return;
    case 99:
        func_80034F90(a0, a1, val);
        return;
    case 100:
        func_800350D8(a0, a1, val);
        return;
    case 101:
        func_80035154(a0, a1, val);
        return;
    case 121:
        func_80034D90(a0, a1);
        return;
    default:
        break;
    }
    rec->unk88 = func_80035E80(a0, a1);
}
#endif
INCLUDE_ASM("asm/nonmatchings/code_179d8_k", func_80034690);

/* A stack-local buffer this function passes to three cross-unit callees:
 * `SsUtGetProgAtr(ch, byte, out)` (established elsewhere as
 * `s16 SsUtGetProgAtr(s16, s16, Entry8E968 *)` in code_179d8_i.c, a
 * different unit's own reduced view -- this unit's own view only needs
 * `unk0`, an item count) fills the FIRST 0x10 bytes; `SsUtGetVagAtr` and
 * `SsUtSetVagAtr` are then called once per item with a pointer to the NEXT
 * 0x28 bytes of the SAME object.  ALL THREE ARE SONY'S, linked from the SDK
 * objects since round 34 -- `libsnd/ut_gpa`, `libsnd/ut_gva` and
 * `libsnd/ut_sva`.  The signatures below stay as this call site's own reading
 * (the return types and the second argument's width are what retail's code
 * here uses); they are LOCAL views and must never move into a shared header
 * next to include/psyq/LIBSND.H's real prototypes.  Modeled as one struct, not two separate
 * locals, because the loop's exit test re-reads `unk0` from memory on
 * every iteration even though nothing in this function's own source
 * writes it after the first call -- the per-item pointer passed to the
 * other two callees aliases the same object, so the compiler cannot prove
 * `unk0` is unchanged and must reload it. */
typedef struct {
    u8 unk0;    /* +0x00: item count, written by SsUtGetProgAtr */
    u8 pad1[0x10 - 0x1];
} NoteList_800349B0;

typedef struct {
    u8 pad0[0x8];
    u8 unk8;    /* +0x08: byte stamped between the two per-item calls -- the
                 * struct's TOTAL size is 0x20, not the 0x28 that offset
                 * alone would suggest; see func_800349B0.md's round-31
                 * update for why the two are decoupled once the
                 * register-rescue fix below is applied. */
    u8 pad9[0x20 - 0x9];
} Scratch_800349B0;

extern s16 SsUtGetProgAtr(s16 a0, s16 a1, void *out);
extern void SsUtGetVagAtr(s16 a0, u8 a1, s16 a2, void *out);
extern void SsUtSetVagAtr(s16 a0, u8 a1, s16 a2, void *out);

void func_800349B0(s16 a0, s16 a1, u8 a2)
{
    Entry90902E8 *rec = &D_800902E8[a0][a1];
    u8 offset;
    NoteList_800349B0 list;
    Scratch_800349B0 scratch;
    s32 i;

    SsUtGetProgAtr(rec->unk4C, ((u8 *)rec + (offset = rec->unk12))[0x2C], &list);
    for (i = 0; i < list.unk0; i++) {
        SsUtGetVagAtr(rec->unk4C, ((u8 *)rec + offset)[0x2C], (s16)i, &scratch);
        scratch.unk8 = a2;
        SsUtSetVagAtr(rec->unk4C, ((u8 *)rec + offset)[0x2C], (s16)i, &scratch);
    }
    rec->unk88 = func_80035E80(a0, a1);
}

/* Same shape as func_800349B0 -- see that function's own struct comment.
 * Only the scratch byte's offset differs (0xB here vs 0x8 there). */
typedef struct {
    u8 pad0[0xB];
    u8 unkB;    /* +0x0B: byte stamped between the two per-item calls */
    u8 pad9[0x20 - 0xC];
} Scratch_80034AEC;

void func_80034AEC(s16 a0, s16 a1, u8 a2)
{
    Entry90902E8 *rec = &D_800902E8[a0][a1];
    u8 offset;
    NoteList_800349B0 list;
    Scratch_80034AEC scratch;
    s32 i;

    SsUtGetProgAtr(rec->unk4C, ((u8 *)rec + (offset = rec->unk12))[0x2C], &list);
    for (i = 0; i < list.unk0; i++) {
        SsUtGetVagAtr(rec->unk4C, ((u8 *)rec + offset)[0x2C], (s16)i, &scratch);
        scratch.unkB = a2;
        SsUtSetVagAtr(rec->unk4C, ((u8 *)rec + offset)[0x2C], (s16)i, &scratch);
    }
    rec->unk88 = func_80035E80(a0, a1);
}

/* Same NoteList/callee shape as func_800349B0/func_80034AEC, plus a
 * range check on this function's own third parameter that picks a
 * one-byte flag written into the scratch buffer at relative offset 1. */
typedef struct {
    u8 pad0[0x1];
    u8 unk1;    /* +0x01: velocity-curve flag, 2 / 0 / left untouched */
    u8 pad2[0x20 - 0x2];
} Scratch_80034C28;

void func_80034C28(s16 a0, s16 a1, s32 a2)
{
    Entry90902E8 *rec = &D_800902E8[a0][a1];
    u8 offset;
    NoteList_800349B0 list;
    Scratch_80034C28 scratch;
    s32 i;
    s32 wrap;

    SsUtGetProgAtr(rec->unk4C, ((u8 *)rec + (offset = rec->unk12))[0x2C], &list);
    for (i = 0; i < list.unk0; i++) {
        SsUtGetVagAtr(rec->unk4C, ((u8 *)rec + offset)[0x2C], (s16)i, &scratch);
        if ((u8)a2 < 0x40) {
            scratch.unk1 = 2;
        } else {
            wrap = 0xC0;
            if ((u8)(a2 + wrap) < 0x40) {
                scratch.unk1 = 0;
            }
        }
        SsUtSetVagAtr(rec->unk4C, ((u8 *)rec + offset)[0x2C], (s16)i, &scratch);
    }
    rec->unk88 = func_80035E80(a0, a1);
}

/* STALL -- see docs/match-reports/func_80034D90.md. length exact 51/51,
 * 49/51 raw word-match, residue is the project's settled commutative-
 * operand-order canonicalization class (2 words). Near-miss body preserved
 * in the report; #if 0 body kept here too so it travels with this .c. */
void func_80034D90(s16 a0, s16 a1)
{
    Entry90902E8 *rec = &D_800902E8[a0][a1];

    SsUtReverbOff();
    SpuVmDamperOff();

    *((u8 *)rec + rec->unk12 + 0x2C) = rec->unk12;
    rec->unk13 = 0;
    rec->unk14 = 0;
    *(s16 *)((u8 *)rec + 0x4E + rec->unk12 * 2) = 0x7F;
    *((u8 *)rec + rec->unk12 + 0x17) = 0x40;
    rec->unk88 = func_80035E80(a0, a1);
}

/* A per-(channel,slot) dispatch table of function pointers, row-major with
 * a 0x40 (64)-byte stride (16 pointers per row) -- also referenced from
 * code_179d8_c's func_8003221C. Not yet given a real element count; the
 * outer dimension is left open. */
typedef void (*Fn80090368)(s32 channel, u8 arg1);
extern Fn80090368 D_80090368[][16];

/* STALL -- see docs/match-reports/func_80034E5C.md. Compiled length ONE WORD
 * SHORT (76/77), 54/77 raw word-match, first real content diff at word 55/56:
 * GCC folds the "slot" array-index multiply into the sign-extension in one
 * shift pair because the slot value is used exactly once, where retail
 * materializes the sign-extended slot into its own register first and
 * multiplies separately (the same two-instruction shape retail also uses for
 * the "channel" index, which IS reused later and so never gets fused here
 * either) -- a register/instruction-count residue, not a logic difference. */
#if 0
void func_80034E5C(s16 a0, s16 a1, u8 a2)
{
    Entry90902E8 *rec = &D_800902E8[a0][a1];
    u8 kind;
    Fn80090368 fn;

    if (rec->unk27 == 1) {
        if (rec->unk10 == 0) {
            rec->unk28 = a2;
            rec->unk10 = 1;
            goto check;
        }
    }
    kind = rec->unk16;
    if (kind != 0x1E && kind != 0x14) {
        rec->unk15 = a2;
        rec->unk2A = rec->unk2A + 1;
    }
check:
    if (rec->unk16 != 0x28) {
        goto skip_call;
    }
    {
        s16 ch = a0;
        s16 sl = a1;
        fn = D_80090368[ch][sl];
        if (fn != NULL) {
            fn(ch, a2 & 0xFF);
        }
    }
skip_call:
    rec->unk88 = func_80035E80(a0, a1);
}
#endif
INCLUDE_ASM("asm/nonmatchings/code_179d8_k", func_80034E5C);

/* STALL -- see docs/match-reports/func_80034F90.md. Compiled length EXACT
 * (82/82), 77/82 raw word-match, first real diff at word 40: a single
 * independent instruction (`sltiu`) the compiler hoists into a branch
 * delay slot one branch earlier than retail places it -- a pure
 * instruction-scheduling residue, not a logic or CFG difference. */
#if 1
void func_80034F90(s16 a0, s16 a1, u8 a2)
{
    Entry90902E8 *rec = &D_800902E8[a0][a1];
    u8 kind = a2;
    s32 result;

    switch (kind) {
    case 0x14:
        rec->unk16 = a2;
        rec->unk27 = 1;
        result = func_80035E80(a0, a1);
        rec->unk88 = result;
        rec->unkC = rec->unk4;
        return;
    case 0x1E:
        rec->unk16 = a2;
        if (rec->unk28 == 0) {
            rec->unk10 = 0;
            rec->unk88 = func_80035E80(a0, a1);
            return;
        }
        if (rec->unk28 < 0x7F) {
            rec->unk28--;
            result = func_80035E80(a0, a1);
            rec->unk88 = result;
            if (rec->unk28 != 0) {
                rec->unk4 = rec->unkC;
            } else {
                rec->unk10 = 0;
            }
            return;
        }
        func_80035E80(a0, a1);
        rec->unk4 = rec->unkC;
        rec->unk88 = 0;
        return;
    default:
        rec->unk16 = a2;
        rec->unk2A = rec->unk2A + 1;
        rec->unk88 = func_80035E80(a0, a1);
        return;
    }
}
#endif

void func_800350D8(s16 a0, s16 a1, u8 a2)
{
    Entry90902E8 *rec = &D_800902E8[a0][a1];
    u8 counter = rec->unk29;

    rec->unk13 = a2;
    counter = counter + 1;
    rec->unk29 = counter;
    rec->unk88 = func_80035E80(a0, a1);
}

void func_80035154(s16 a0, s16 a1, u8 a2)
{
    Entry90902E8 *rec = &D_800902E8[a0][a1];
    u8 counter = rec->unk29;

    rec->unk14 = a2;
    counter = counter + 1;
    rec->unk29 = counter;
    rec->unk88 = func_80035E80(a0, a1);
}

/* func_800351D0's own construction of the scratch/argument blob that feeds
 * func_800357B0 -- see docs/match-reports/func_800357B0.md, which stalled
 * partly for lack of this derivation.  A single SsUtGetProgAtr fill at
 * function entry writes a LARGER-than-usual record here (count at +0, then
 * a 4-byte "header" pair of u16s and two more alignment-2 chunks used only
 * by the unk2A==2 dispatch below); Blk1/Blk2 exist purely to be whole-
 * struct-copied byte for byte into func_800357B0's outgoing stack args,
 * per this project's confirmed alignment-2-struct-assignment idiom -- their
 * internal field breakdown is unconstrained by anything observed so far. */
#if 0
typedef struct {
    s16 raw[14];    /* 28 bytes, alignment 2: whole-struct-copied verbatim */
} Blk1_800351D0;

typedef struct {
    s16 raw[9];     /* 18 bytes, alignment 2: whole-struct-copied verbatim */
} Blk2_800351D0;

typedef struct {
    u8 count;             /* +0x00: item count, SsUtGetProgAtr's usual field */
    u8 pad1[0x10 - 0x1];
    u16 hdrLo;            /* +0x10 */
    u16 hdrHi;            /* +0x12 */
    Blk1_800351D0 blk1;   /* +0x14 */
    Blk2_800351D0 blk2;   /* +0x30 */
} List_800351D0;

/* Same per-item scratch role as NoteList_800349B0's sibling Scratch_* types
 * (filled by SsUtGetVagAtr, consumed by SsUtSetVagAtr); this call site's
 * own fields happen to share stack space with List_800351D0's tail fields
 * above since the two never have overlapping lifetimes at runtime (mutually
 * exclusive unk29==2 / unk2A==2 dispatch arms). */
typedef struct {
    u8 pad0[0x4];
    u8 unk4;      /* +0x4: read back and stored unchanged in the unk13==2 loop -- see report */
    u8 unk5;      /* +0x5: read back and stored unchanged in the unk13==1 loop -- see report */
    u8 pad6[0xC - 0x6];
    u8 unkC;      /* +0xC */
    u8 unkD;      /* +0xD */
    u8 pad0E[0x20 - 0xE];
} Scratch800351D0;

/* Local guess; see docs/match-reports/func_800357B0.md for the derivation
 * this signature settles (the 4th argument's type, and the scratch struct's
 * layout, were both left explicitly unresolved there). */
extern void func_800357B0(s16 a0, s16 a1, s16 a2, u32 a3, Blk1_800351D0 blk1,
                           Blk2_800351D0 blk2, s16 arg5, u8 arg6);

/* STALL -- see docs/match-reports/func_800351D0.md. Compiled length 4 words
 * LONG (380/376, measured directly off build/src/code_179d8_k.c.o since
 * length has drifted). The whole 22-word RPN/NRPN dispatch skeleton, the
 * three SsUtGetVagAtr/SsUtSetVagAtr loops, and the func_800357B0 struct-
 * marshaling (which also settles that function's own previously-unresolved
 * 4th-argument/scratch-layout question, see func_800357B0.md) all come out
 * byte-correct. The residue is two isolated dead-value computations
 * (retail computes a masked/shifted byte into $s5 that is NEVER READ
 * anywhere in the function, only restored as part of the ordinary
 * callee-save epilogue) that a plain C dead local gets optimized away
 * entirely and a `volatile` local keeps but wraps in real store+join
 * control flow retail's register-only dead value never needed -- see
 * report for the full derivation and the four things tried. CC6 (Data
 * Entry MSB) handler for RPN/NRPN parameter writes. */
void func_800351D0(s16 a0, s16 a1, u8 a2)
{
    s16 ch = a0;
    s16 slot = a1;
    Entry90902E8 *rec = &D_800902E8[ch][slot];
    u8 *p = (u8 *)rec + rec->unk12;
    List_800351D0 list;
    Scratch800351D0 scratch;
    s16 i;
    u8 kind;

    SsUtGetProgAtr(rec->unk4C, p[0x2C], &list);

    if (rec->unk27 == 1 && rec->unk10 == 0) {
        rec->unk28 = a2;
        rec->unk10 = 1;
        goto combine;
    }
    if (rec->unk16 != 0x1E && rec->unk16 != 0x14) {
        rec->unk15 = a2;
        rec->unk2A = rec->unk2A + 1;
        goto combine;
    }
    if (rec->unk29 == 2) {
        if (rec->unk13 == 0 && rec->unk14 == 0) {
            for (i = 0; i < list.count; i++) {
                SsUtGetVagAtr(rec->unk4C, p[0x2C], i, &scratch);
                scratch.unkC = a2 & 0x7F;
                scratch.unkD = a2 & 0x7F;
                SsUtSetVagAtr(rec->unk4C, p[0x2C], i, &scratch);
            }
        }
        if (rec->unk13 == 1 && rec->unk14 == 0) {
            volatile s32 unused;
            if ((u8)(a2 - 0x41) < 0x3F) {
                if (((a2 & 0xFF) * 100) >= 0) {
                    unused = ((a2 & 0xFF) * 100) & 0xE000;
                } else {
                    unused = (((a2 & 0xFF) * 100) + 0x1FFF) & 0xE000;
                }
            } else {
                unused = 0;
            }
            for (i = 0; i < list.count; i++) {
                SsUtGetVagAtr(rec->unk4C, p[0x2C], i, &scratch);
                scratch.unk5 = scratch.unk5;
                SsUtSetVagAtr(rec->unk4C, p[0x2C], i, &scratch);
            }
        }
        if (rec->unk13 == 2 && rec->unk14 == 0) {
            volatile s32 unused;
            if ((u8)(a2 - 0x40) < 0x40) {
                unused = ((a2 & 0xFF) * 25) << 8;
            } else {
                unused = 0;
            }
            for (i = 0; i < list.count; i++) {
                SsUtGetVagAtr(rec->unk4C, p[0x2C], i, &scratch);
                scratch.unk4 = scratch.unk4;
                SsUtSetVagAtr(rec->unk4C, p[0x2C], i, &scratch);
            }
        }
        rec->unk88 = func_80035E80(ch, slot);
        rec->unk29 = 0;
        return;
    }
    if (rec->unk2A == 2) {
        kind = rec->unk16;
        if (kind == 0x10) {
            for (i = 0; i < list.count; i++) {
                func_800357B0(rec->unk4C, p[0x2C], i,
                              (u32)list.hdrLo | ((u32)list.hdrHi << 16),
                              list.blk1, list.blk2, rec->unk15, a2 & 0xFF);
            }
        } else {
            func_800357B0(rec->unk4C, p[0x2C], (s16)kind,
                          (u32)list.hdrLo | ((u32)list.hdrHi << 16),
                          list.blk1, list.blk2, rec->unk15, a2 & 0xFF);
        }
        rec->unk88 = func_80035E80(ch, slot);
        rec->unk2A = 0;
        return;
    }
combine:
    rec->unk88 = func_80035E80(ch, slot);
}
#endif
INCLUDE_ASM("asm/nonmatchings/code_179d8_k", func_800351D0);

INCLUDE_ASM("asm/nonmatchings/code_179d8_k", func_800357B0);

/* STALL -- see docs/match-reports/func_80035A7C.md. length exact 44/44,
 * 36/44 raw word-match, first real diff at word 23: an independent value
 * (rec->unk4C) the compiler schedules earlier than retail does, not a
 * logic or CFG difference -- no if/else arm ordering applies here (see
 * report for the round-25 head lever's explicit negative answer). */
#if 1
void func_80035A7C(s16 a0, s16 a1)
{
    Entry90902E8 *rec = &D_800902E8[a0][a1];
    u8 *cursor = rec->unk4;
    s32 packed;
    u8 b;
    u8 vol;

    b = rec->unk12;
    rec->unk4 = cursor;
    packed = (a1 << 8) | a0;
    rec->unk4 = rec->unk4 + 1;
    vol = *((u8 *)rec + b + 0x2C);
    b = *cursor;
    func_8002F610(packed, rec->unk4C, vol, b);
    rec->unk88 = func_80035E80(a0, a1);
}
#endif

/* Cross-unit calls, local guesses per project convention. func_8003069C is
 * matched in code_179d8_j.c and already has this exact "(slot<<8)|channel"
 * single-argument reading in both code_179d8_f.c and code_179d8_i.c;
 * _SsSndNextSep is Sony's `libsnd/next`, linked from the SDK object since
 * round 34; this signature is the one code_179d8_f.c's matched C used
 * before the conversion. */
extern s32 func_8003069C(s32 a0);
extern void _SsSndNextSep(s32 a0, s32 a1);

/* This unit's own reading of the same global code_179d8_i.c already reads
 * as `D_8009024C` (a tick-rate/PPQN-style constant) -- independent local
 * view, per project convention. */
extern u32 D_8009024C;

/* Meta-event handler, reached from func_8003424C's 0xFF ("running status
 * for a 0xF0 event") and new-status 0xF0 dispatch arms with `a2` = the
 * meta-event TYPE byte. Only two types are understood; everything else is
 * silently ignored:
 *
 * STALL -- see docs/match-reports/func_80035B2C.md. 2 words SHORT
 * (211/213, compiled length measured off build/src/code_179d8_k.c.o since
 * funcdiff's word-match number is not trustworthy once length drifts).
 * First real diff at word 56 (`tools/funcdiff.py func_80035B2C`), a
 * register-identity symptom: retail re-reads `rec->unk4A` fresh (a plain
 * `lh`) before EACH of the Set-Tempo rate recompute's two divisions; this
 * body keeps the first read's value live in a register instead. The
 * round-26 head's narrowed-`volatile` lever (only the WORD-sized
 * `rec->unk8C` field marked `volatile`, not the sub-word `unk4A`) closed
 * 18 of the 20 words this stalled at previously by defeating GCC's
 * div/mod fusion without retail's plain `lh` turning into `lhu`+widen.
 * The one remaining word resisted six further reshapes (see report) --
 * every one of them either had no effect or regressed.
 *
 * 0x2F (End of Track): bumps the repeat counter (unk48). unk46 == 0 means
 * "loop forever" -- rewind unk4 to the saved track start (unk8) and keep
 * going. Otherwise, while the counter is still under the limit (unk46),
 * rewind BOTH unk4 and unkC. Once the limit is reached, clear the
 * playback-state flags (unk90), rewind unkC one more time, and run the
 * stop-sequence callbacks (_SsSndNextSep gated on unk3C != 0xFF, then an
 * unconditional func_8003069C notify) before priming unk88 from unk70 for
 * the next tick.
 *
 * 0x51 (Set Tempo): reads a 3-byte big-endian microseconds-per-quarter-note
 * value, converts it to a BPM-like rate (60000000 / value -- the standard
 * MIDI tempo formula) into unk8C, then recomputes the scheduling
 * threshold (unk6E/unk70) against unk4A and the global tick-rate constant
 * D_8009024C, in whichever of two regimes avoids losing precision to
 * integer truncation (the `else` regime also derives a rounding bit from
 * the division's remainder). unk6E doubles as a mode flag: -1 means
 * "unk70 holds the reciprocal-regime value", any other value means
 * "unk70 holds the same value unk6E does". */
#if 0
void func_80035B2C(s16 a0, s16 a1, u8 a2)
{
    Entry90902E8 *rec = &D_800902E8[a0][a1];

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
            base = D_8009024C * 15;
            divisor = base * 4;
            rec->unk8C = bpm;
            if (rec->unk4A * rec->unk8C * 10 < divisor) {
                rec->unk6E = (D_8009024C * 600) / (rec->unk4A * rec->unk8C);
                rec->unk70 = rec->unk6E;
            } else {
                /* Narrowed volatile lever (round 26 head ruling): only the
                 * WORD-sized field needs to be volatile to defeat GCC's
                 * div/mod fusion -- there is no load-width to get wrong for
                 * a full-word read, so retail's plain `lh` for unk4A is
                 * unaffected. See docs/match-reports/func_80035B2C.md. */
                volatile s32 *pbpm = &rec->unk8C;
                s32 q = (rec->unk4A * *pbpm * 10) / divisor;
                s32 r = (rec->unk4A * *pbpm * 10) % divisor;

                rec->unk6E = -1;
                rec->unk70 = (base * 2 < r) ? q + 1 : q;
            }
        }
        rec->unk88 = func_80035E80(a0, a1);
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
        D_800902E8[a0][a1].unk90 &= ~1;
        D_800902E8[a0][a1].unk90 &= ~8;
        D_800902E8[a0][a1].unk90 &= ~2;
        D_800902E8[a0][a1].unk90 |= 0x200;
        D_800902E8[a0][a1].unk90 |= 0x4;
        rec->unkC = rec->unk8;
        rec->unk2B = 0;
        if (rec->unk3C != 0xFF) {
            _SsSndNextSep(rec->unk3C, rec->unk0);
            rec->unk2B = 0;
        }
        func_8003069C((a1 << 8) | a0);
        rec->unk88 = rec->unk70;
    }
}
#endif
INCLUDE_ASM("asm/nonmatchings/code_179d8_k", func_80035B2C);

/* MATCHED -- see docs/match-reports/func_80035E80.md. The `goto combine`
 * is load-bearing: retail keeps the "single-byte" and "loop-exit" `val`
 * writes as textually distinct arms reaching one merge point, and this
 * exact shape (jump-arm written explicitly, fallthrough-arm last in
 * source order) is what makes GCC 2.6.3 choose retail's own register for
 * both. See the round-25 head broadcast on if/else arm ordering. */
s32 func_80035E80(s16 a0, s16 a1)
{
    Entry90902E8 *rec = &D_800902E8[a0][a1];
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
