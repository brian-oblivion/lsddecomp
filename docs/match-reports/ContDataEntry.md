# ContDataEntry -- STALL (4 words LONG, 380/376; 95/376 raw word-match; first real diff at word 83, vram 0x80035B1C, loop register numbering after a hoisted `a2 & 0x7F`)

## Round 94 (runner echo, track 6): Sony types

The union of the two VagAtr views in `DataEntryLocals` (`Scratch_800357B0 s; Scratch800351D0 v;`) is now one Sony `VagAtr vag`, and the leading count byte plus padding is Sony's `ProgAtr prog`. The loop fields: `unk4` = `center`, `unk5` = `shift`, `unkC`/`unkD` = `pbmin`/`pbmax`. `ProgAtr` holds `unsigned long`s, so the struct's alignment went from 2 to 4 and its size from 0x42 to 0x44. That moves nothing in the verified build (this function is `INCLUDE_ASM`); the NON_MATCHING body still compiles. Its frame should be re-measured before the next attempt on this function.

> Renamed from `func_800351D0` on 2026-09-23 (tools/rename.py). Address 0x800351d0.

**REVISITED, round 69: STALL, improved (44/376 rebuilt -> 95/376, ins/del
21/21 -> 6/6, frame exact) and promoted to `#ifdef NON_MATCHING` in
`src/psyq/libsnd_seqread.c`; names/types used** (the callee's real signature, a
union in `DataEntryLocals`, a loop counter width).

## Round 69 (runner bravo): repaired, rebuilt, three levers

### The repair, and the rebuilt figures

Round 66 was right: the preserved body did not compile. It called
`Snd_setVabAttr` with round 48's guessed slice `(u32 hdr, Blk1, Blk2)`,
while the callee's own matched definition takes
`(s16, s16, s16, Scratch_800357B0 scratch, AdsrFields resolved,
s16, u8)`. **Repair (rebuilding, not changing):** moved the two callee
typedefs above this function (typedefs only; no definition moved), retyped
`DataEntryLocals`'s tail from `hdrLo/hdrHi/blk1/blk2` to
`Scratch_800357B0 scratch; AdsrFields adsr;` (same 50 bytes at the
same +0x10, as round 49 predicted), re-added the forward declaration, and
passed `list.scratch, list.adsr` at both call sites. `Blk1_800351D0` and
`Blk2_800351D0` are gone.

Rebuilt round 48's body with only that repair: **386/376 words (10 LONG),
44/376 raw, `insertions 21 / deletions 21`** (positional skeleton diffs
322), first real diff at word 0: frame `-0x118` vs retail `-0x108`. The
repair itself costs 16 bytes of frame, because the by-value struct copies
need their own outgoing space, so round 48's `dead[40]` now overshoots.
With `dead[24]` the frame is exact again and the score is round 48's
48/376, so the old figures reproduce once the frame is re-balanced.

### Three levers (44/376 -> 95/376)

1. **`s32 i`, not `s16 i`.** Retail counts in `$s1` as a full word
   (`addiu s1,s1,1; slt; ... sll s0,s1,16`) and narrows only for the call
   argument. `s16 i` cost `addiu/move/sll` per loop and 3 extra words in
   each of four loops. -> 373/376, 52/376, ins/del 11/11.
2. **Keep the offset BYTE, re-add it per use.** Retail loads
   `rec->unk12` once into `$s7` and recomputes `addu $s4,$s7,$s2` before
   every loop. It does not cache `p = (u8 *)rec + rec->unk12`. Writing
   `u8 off = rec->unk12;` and `((u8 *)rec + off)[0x2C]` at each use made
   the whole prologue byte-exact, including retail's spill of `slot` to
   `0x90($sp)`. -> 84/376, ins/del 6/6.
3. **The VagAtr buffer IS `list + 0x10`.** Retail passes `sp+0x58` to
   `SsUtGet/SetVagAtr` and stores `a2 & 0x7F` at `0x64/0x65($sp)`. `list`
   is at `sp+0x48`, so that is `list.scratch`, not a separate local. The
   old "share stack space" comment was describing the same memory. Put a
   union of the two views (`Scratch_800357B0 s; Scratch800351D0 v;`) in
   `DataEntryLocals` and removed the separate `scratch` local. Every stack
   offset in the loops then matched. The frame fell to `-0xF8`, and
   `dead[16]` makes it exact. -> **95/376**, still 380 words.
   (`scratch.unkC = scratch.unkD = ...` puts the two stores in retail's
   order, 0x65 before 0x64. Score unchanged; kept.)

### Negatives this round (all measured, none kept)

- A non-volatile function-scope `s32 unused`: DCE'd as expected, 342
  words, 34/376.
- One shared function-scope `volatile s32 unused`: byte-identical to two
  block-scope ones.
- `label: ...; if (i < n) goto label;` form on the first loop (the
  documented LICM-defeat): worse, 42/376. It renumbered the whole function.

### What is left

`tools/asm-differ/diff.py` on the promoted body: the first real diff is word
83 (`0x80035B1C`). Retail has `addu s4,s7,s2; addiu s3,sp,0x58`. This build
has `s5`/`s4` for those, plus an extra `andi s3,s6,0x7f` hoisted out of the
first loop. Retail recomputes `andi v0,s6,0x7f` inside the loop. The cause
is the same as the `unused` residue: **retail keeps both dead `unused`
values in `$s5`** (`andi s5,v0,0xE000`, `sll s5,v0,8`, `move s5,zero`;
`$s5` is never read). That uses up retail's last callee-saved register, so
its LICM has no register to hoist the mask into. This body needs `volatile`
to keep the computation at all, so `$s5` is free and the hoist happens.
Hypothesis, not measured: find what keeps a dead register set alive through
2.6.3's flow pass. A use that a pass AFTER flow deletes is the likely
shape, e.g. one that jump2/cross-jump or combine removes. That should close
the LICM residue too. The 4-word length surplus has not been attributed
yet. The likely source is the extra `nop`/`sw` words asm-differ shows at
the two `volatile unused` sites, but that is not counted.

Builds on this function this session: 14 (best improved at build 12).
No permuter search.

### Best body (95/376, 380 words, frame exact) with every declaration it needs

This is the same body now in `src/psyq/libsnd_seqread.c` under `#ifdef
NON_MATCHING`. It needs the unit's `Entry90902E8`, `_ss_score`,
`ReadDeltaValue` and the `SsUtGetProgAtr`/`SsUtGetVagAtr`/`SsUtSetVagAtr`
externs declared earlier in the unit.

```c
#if 0
typedef struct {
    u8 prior;    /* +0x0 */
    u8 mode;     /* +0x1 */
    u8 pad2[0x6 - 0x2];
    u8 min;      /* +0x6 */
    u8 max;      /* +0x7 */
    u8 pad8[0x9 - 0x8];
    u8 vibT;     /* +0x9 */
    u8 porW;     /* +0xA */
    u8 padB[0x10 - 0xB];
    u16 adsr1;   /* +0x10 */
    u16 adsr2;   /* +0x12 */
    u8 pad14[0x20 - 0x14];
} Scratch_800357B0;

/* Same 9-halfword ADSR-decode layout as libsnd_cres.c's independent,
 * already-matched `UnkStruct80035F3C` (docs/match-reports/func_80035F3C.md)
 * -- a fresh LOCAL (uninitialized), filled by `_SsUtResolveADSR` from
 * `scratch.adsr1`/`adsr2` and consumed by `_SsUtBuildADSR`, never by the
 * caller. Renamed per-unit per project convention, not shared. */
typedef struct {
    s16 unk0;
    s16 unk2;
    s16 unk4;
    s16 unk6;
    s16 unk8;
    s16 unkA;
    s16 unkC;
    s16 unkE;
    s16 unk10;
} AdsrFields;

/* This function's own view of the same VagAtr buffer: only the four bytes
 * its unk29==2 loops touch. */
typedef struct {
    u8 pad0[0x4];
    u8 unk4;      /* +0x4: read back and stored unchanged in the unk13==2 loop -- see report */
    u8 unk5;      /* +0x5: read back and stored unchanged in the unk13==1 loop -- see report */
    u8 pad6[0xC - 0x6];
    u8 unkC;      /* +0xC */
    u8 unkD;      /* +0xD */
    u8 pad0E[0x20 - 0xE];
} Scratch800351D0;

/* SsUtGetProgAtr's fill at function entry. From +0x10 the SAME memory is
 * both the VagAtr buffer the unk29==2 loops hand to SsUtGet/SetVagAtr
 * (retail addresses it at sp+0x58 = list+0x10) and, with the 18 bytes
 * after it, the two by-value arguments of Snd_setVabAttr (round 69). */
typedef struct {
    u8 count;                  /* +0x00: item count, SsUtGetProgAtr's usual field */
    u8 pad1[0x10 - 0x1];
    union {
        Scratch_800357B0 s;    /* +0x10: passed by value to Snd_setVabAttr */
        Scratch800351D0 v;     /* +0x10: SsUtGet/SetVagAtr's buffer in the unk29==2 loops */
    } scratch;
    AdsrFields adsr;     /* +0x30: passed by value to Snd_setVabAttr */
} DataEntryLocals;

/* Snd_setVabAttr is defined later in this unit; its own definition fixes
 * this signature (round 49). */
extern void Snd_setVabAttr(s16 channel, s16 slot, s16 kind, Scratch_800357B0 scratch,
                          AdsrFields resolved, s16 arg5, u8 arg6);

void ContDataEntry(s16 a0, s16 a1, u8 a2)
{
    s16 ch = a0;
    s16 slot = a1;
    Entry90902E8 *rec = &_ss_score[ch][slot];
    u8 off = rec->unk12;
    DataEntryLocals list;
    s32 i;
    u8 kind;
    u8 dead[16];

    if (0) {
        dead[0] = 0;
    }
    SsUtGetProgAtr(rec->unk4C, ((u8 *)rec + off)[0x2C], &list);

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
            for (i = 0; i < list.count; i++) {
                SsUtGetVagAtr(rec->unk4C, ((u8 *)rec + off)[0x2C], i, &list.scratch.v);
                list.scratch.v.unkC = list.scratch.v.unkD = a2 & 0x7F;
                SsUtSetVagAtr(rec->unk4C, ((u8 *)rec + off)[0x2C], i, &list.scratch.v);
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
                SsUtGetVagAtr(rec->unk4C, ((u8 *)rec + off)[0x2C], i, &list.scratch.v);
                list.scratch.v.unk5 = list.scratch.v.unk5;
                SsUtSetVagAtr(rec->unk4C, ((u8 *)rec + off)[0x2C], i, &list.scratch.v);
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
                SsUtGetVagAtr(rec->unk4C, ((u8 *)rec + off)[0x2C], i, &list.scratch.v);
                list.scratch.v.unk4 = list.scratch.v.unk4;
                SsUtSetVagAtr(rec->unk4C, ((u8 *)rec + off)[0x2C], i, &list.scratch.v);
            }
        }
        rec->unk88 = ReadDeltaValue(ch, slot);
        rec->unk29 = 0;
        return;
    }
    if (rec->unk2A == 2) {
        kind = rec->unk16;
        if (kind == 0x10) {
            for (i = 0; i < list.count; i++) {
                Snd_setVabAttr(rec->unk4C, ((u8 *)rec + off)[0x2C], i,
                              list.scratch.s, list.adsr, rec->unk15, a2 & 0xFF);
            }
        } else {
            Snd_setVabAttr(rec->unk4C, ((u8 *)rec + off)[0x2C], (s16)kind,
                          list.scratch.s, list.adsr, rec->unk15, a2 & 0xFF);
        }
        rec->unk88 = ReadDeltaValue(ch, slot);
        rec->unk2A = 0;
        return;
    }
    rec->unk88 = ReadDeltaValue(ch, slot);
}
#endif
```

### Proposed learning

When a caller builds a callee's by-value struct argument out of a local it
has ALREADY had filled by another call, look at the retail stack offsets
before you give the two uses separate locals. Here the "separate scratch
local" and the "list tail" were one buffer (`sp+0x58 = list+0x10`), and
making them one moved the score from 84 to 95 and fixed every loop offset.

---

## History (earlier rounds; figures superseded by the round-69 section above)

Old title: ContDataEntry -- STALL (4 words LONG, 380/376; 18/376 raw word-match; first real diff at word 0, vram 0x800351D0, frame size)

> **Round 66 (runner charlie), track 1b: NO NON_MATCHING BODY: preserved
> body does not compile, and the fix is out of scope for a mechanical
> promotion pass.** This report's title figures are themselves stale --
> round 48 superseded them (frame-size fix + tail-duplication, 48/376,
> compiled length now 386/376 i.e. 10 words LONG, not the 4-long/18-match
> figures the title still quotes; see the round-48 and round-49 sections
> below for the current numbers). That is not why this is blocked, though.
>
> The body preserved in `src/code_179d8_k.c` (round 48's, `#if 0`-wrapped)
> calls `Snd_setVabAttr` with its OWN pre-round-49 guessed signature -- an
> 8-argument slice `(u32 hdr, Blk1_800351D0 blk1, Blk2_800351D0 blk2, ...)`.
> Round 49 matched `Snd_setVabAttr` byte-exact with a DIFFERENT real
> signature, `(..., Scratch_800357B0 scratch, AdsrFields resolved,
> ...)`, and removed this function's stale forward declaration as
> conflicting (see the round-49 section below, "will need reshaping on the
> next attempt"). The preserved body was never updated to match, so it
> does not compile as `INCLUDE_ASM`'s NON_MATCHING sibling: `Snd_setVabAttr`
> is declared once, with the real signature, and the preserved body's call
> sites pass the wrong argument shape (`too many arguments` /
> `conflicting types`, not tried here since the report already establishes
> the cause).
>
> Reshaping the two call sites requires constructing real
> `Scratch_800357B0`/`AdsrFields` by-value arguments from
> `list.hdrLo`/`hdrHi`/`blk1`/`blk2` -- sizes line up exactly (4+28=32=
> `sizeof(Scratch_800357B0)`, 18=`sizeof(AdsrFields)`) and both
> parameters' incoming values are provably dead on the callee side (fully
> overwritten by `SsUtGetVagAtr`/`_SsUtResolveADSR` before any read, per
> the comment at `src/code_179d8_k.c`'s `Scratch_800357B0` typedef), so a
> byte-reinterpreting construction would very likely compile and behave
> correctly. But choosing and verifying that construction is fresh
> derivation on a body track 1 has not yet re-scored since the reshape --
> not a wrapper-shape conversion of an already-complete preserved body,
> which is what this track promotes. Left as `INCLUDE_ASM`, unpromoted, no
> change to `src/code_179d8_k.c`. The round-49 section's own "what's next"
> note already flags this for the next matching attempt; this adjudication
> does not change that queue entry, only confirms it from the promotion
> side.

**Length**: retail 376 words (0x5E0). This round's best C compiled to 380
words (measured directly off `build/src/code_179d8_k.c.o` via objdump,
address-difference to the next function). **Raw word-match**: `18/376`
(`tools/funcdiff.py ContDataEntry`, re-measured against the near-miss body
reconstructed for this purpose) -- this number is mostly a symptom of the
length drift below, not 358 independent mismatches; see caveat. **First
real diff**: word 0 (vram `0x800351D0`, `addiu sp,sp,-0x108` vs this
build's `addiu sp,sp,-0xe0`), confirmed via `tools/asm-differ/diff.py
ContDataEntry`, which realigns the rest of the function around the length
difference and still shows the divergence starting at the function's own
opening instruction: this build's stack frame is 0x28 bytes smaller than
retail's, so the two disassemblies run in a different register/offset
numbering from word 0 onward. **The useful comparison is the two localized
residues described below** (found by direct objdump inspection of the
specific `unk13==1`/`unk13==2` blocks, not by reading the whole-function
diff word-by-word), which is what this report actually documents and what
a future attempt should target.

This is the largest function in the unit (round 27's #1 priority, FRESH
ground with no prior report) and the CC6 (Data Entry MSB) handler for
RPN/NRPN parameter writes. The overall control-flow skeleton, all three
`func_80033260`/`func_80036230` loops, and the `Snd_setVabAttr` call's
struct-marshaling are all now derived and (as far as could be verified
before the length drift made further comparison unreliable) byte-correct.
The remaining residue is narrow and specific -- see "What's left" below.

## Signature and struct layout (now settled)

```c
void ContDataEntry(s16 a0 /* channel */, s16 a1 /* slot */, u8 a2 /* value */);
```

`rec = &_ss_score[a0][a1]`, `p = (u8 *)rec + rec->unk12` -- same idiom as
every sibling function in this unit.

A single `func_800334F0(rec->unk4C, p[0x2C], &list)` call at function entry
fills a **larger-than-previously-assumed** output record. Every other
function in this unit that calls `func_800334F0`/`func_80033260` only reads
the item COUNT (offset 0) from its output; this call site is the first to
read much further into the same record (up to relative offset 0x41), which
is what finally settles the open question in `Snd_setVabAttr.md` about that
function's 4th-argument type and scratch layout:

```c
typedef struct {
    s16 raw[14];    /* 28 bytes, alignment 2: whole-struct-copied verbatim */
} Blk1_800351D0;

typedef struct {
    s16 raw[9];     /* 18 bytes, alignment 2: whole-struct-copied verbatim */
} Blk2_800351D0;

typedef struct {
    u8 count;             /* +0x00: item count, func_800334F0's usual field */
    u8 pad1[0x10 - 0x1];
    u16 hdrLo;            /* +0x10 */
    u16 hdrHi;            /* +0x12 */
    Blk1_800351D0 blk1;   /* +0x14 */
    Blk2_800351D0 blk2;   /* +0x30 */
} DataEntryLocals;
```

`hdrLo`/`hdrHi` are read as TWO SEPARATE `u16` loads (`lhu`+`lhu`, not one
`lw`) and combined via `(u32)hdrLo | ((u32)hdrHi << 16)` at the call site --
this specific shape (rather than a single word load) is what confirms they
are genuinely two `u16` fields in the source, not one `u32` reinterpreted.
`blk1`/`blk2` are passed to `Snd_setVabAttr` **by value**, which is why they
compile to the project's confirmed alignment-2-struct whole-copy idiom
(`lwl`/`lwr` + `swl`/`swr` in word-sized chunks, with a plain `lh`/`sh` for
any 2-byte remainder) -- their internal field breakdown is unconstrained by
anything observed; only the total size (28 and 18 bytes) and the alignment
(2, forcing the safe copy idiom regardless of the source address's actual
runtime alignment) are load-bearing.

```c
extern void Snd_setVabAttr(s16 a0, s16 a1, s16 a2, u32 a3, Blk1_800351D0 blk1,
                           Blk2_800351D0 blk2, s16 arg5, u8 arg6);
```

This settles `Snd_setVabAttr.md`'s open question: its "4th parameter" is
this `u32` header value (built from two `u16`s, not a pointer or single
word field), and its scratch struct is the `Blk1_800351D0`/`Blk2_800351D0`
pair passed by value on the stack (28 + 18 bytes, aligned/padded to a
20-byte slot for the second one), followed by `arg5` (`rec->unk15`) and
`arg6` (the masked value byte) each promoted to a full stack word.

A second, unrelated local record (`Scratch800351D0`, ~0x20 bytes) shares
the SAME stack address as `DataEntryLocals`'s `hdrLo`/`blk1`/`blk2` tail,
because the two live ranges never overlap at runtime (the `unk29==2` and
`unk2A==2` dispatch arms are mutually exclusive, each ending in its own
`return`):

```c
typedef struct {
    u8 pad0[0x4];
    u8 unk4;      /* +0x4 */
    u8 unk5;      /* +0x5 */
    u8 pad6[0xC - 0x6];
    u8 unkC;      /* +0xC */
    u8 unkD;      /* +0xD */
    u8 pad0E[0x20 - 0xE];
} Scratch800351D0;
```

## Control flow (fully decoded)

```c
/* stalesyms --fix 2026-09-22: func_80033260 -> SsUtGetVagAtr, func_800334F0 -> SsUtGetProgAtr, func_80036230 -> SsUtSetVagAtr -- names retrofitted so this body links as written; the residue it recorded is unverified until rebuilt. */
void ContDataEntry(s16 a0, s16 a1, u8 a2)
{
    s16 ch = a0;
    s16 slot = a1;
    Entry90902E8 *rec = &_ss_score[ch][slot];
    u8 *p = (u8 *)rec + rec->unk12;
    DataEntryLocals list;
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
        rec->unk88 = ReadDeltaValue(ch, slot);
        rec->unk29 = 0;
        return;
    }
    if (rec->unk2A == 2) {
        kind = rec->unk16;
        if (kind == 0x10) {
            for (i = 0; i < list.count; i++) {
                Snd_setVabAttr(rec->unk4C, p[0x2C], i,
                              (u32)list.hdrLo | ((u32)list.hdrHi << 16),
                              list.blk1, list.blk2, rec->unk15, a2 & 0xFF);
            }
        } else {
            Snd_setVabAttr(rec->unk4C, p[0x2C], (s16)kind,
                          (u32)list.hdrLo | ((u32)list.hdrHi << 16),
                          list.blk1, list.blk2, rec->unk15, a2 & 0xFF);
        }
        rec->unk88 = ReadDeltaValue(ch, slot);
        rec->unk2A = 0;
        return;
    }
combine:
    rec->unk88 = ReadDeltaValue(ch, slot);
}
```

Splicing this literal body back into `src/psyq/libsnd_seqread.c` in place of the
`INCLUDE_ASM`, together with the struct/prototype block above it, builds
clean (`build exit=2`, zero compile-error grep hits) at 380/376 words.

An early attempt used the bare parameters `a0`/`a1` directly at every
`ReadDeltaValue` call site instead of caching them into `ch`/`slot`; this
produced a WILDLY different, much smaller/differently-allocated function
(frame `-0x108` retail vs `-0xF8`, fused sign-extend-and-multiply shifts
where retail keeps a separate persistent register) -- exactly the fused-vs-
unfused shift pattern `ContNrpn1`'s own report already documents
("the same two-instruction shape retail also uses for the channel index
which IS reused later and so never gets fused"). Introducing the `ch`/
`slot` locals and using them uniformly is what got the function's overall
shape -- including the full struct-marshaling section -- into close
alignment with retail (same 10 callee-saved registers, matching struct-copy
instruction sequences, matching loop shapes).

## What's left: two genuinely dead retail computations

Retail computes a value into `$s5` in each of the `unk13==1` and
`unk13==2` sub-branches (a masked/rounded `val*100` in one, a shifted
`val*25` in the other) that is **never read anywhere in the function** --
confirmed with `grep -n '\$s5' asm/nonmatchings/libsnd_seqread/ContDataEntry.s`,
which shows only the writes (`andi`/`sll`/`addu $s5,zero,zero`) and the
final callee-save restore (`lw $s5, ...` in the epilogue, which is not a
real use). Four things were tried, in order:

1. **A plain dead local** (`s32 unused = expr; (void)unused;`): GCC 2.6.3
   eliminates the entire computation, including the range-check branch
   itself (since it has no side effects) -- function comes out **26 words
   SHORT** (350/376), i.e. almost exactly the size of these two blocks.
2. **`volatile s32 unused`**: keeps the computation, but volatile forces a
   REAL memory store at each assignment site. Retail's register-only dead
   value needs no such store and no control-flow join to select between
   the "in range" and "out of range" (zero) cases -- a register just holds
   whichever was last written, with no extra code, while a volatile local
   forces an explicit `sw` per arm plus a `j` to skip the alternate arm's
   store. This is the version above; it is 1-2 words too LONG per site
   (measured directly: the `unk13==1` block is 18 words vs retail's 17;
   the `unk13==2` block is 14 words vs retail's 12).
3. **Restructuring to avoid an intermediate named temporary** (inlining
   the multiply into each arm instead of hoisting it to a local `t`):
   removed an extra register-reconciliation `move` instruction that a
   shared-temporary version produced, but did not close the remaining gap
   (the `sw`+`j`-to-skip overhead is structural, not a scheduling
   artifact).
4. **A bare `__asm__("")` scheduling barrier** placed after each inner
   assignment: no effect (matches this round's HEAD BROADCAST finding that
   a bare barrier was inert on a comparable residue in `_card_clear`).
   The barrier is placed too late to affect the ANDI-hoisting/tail-merge
   decision, which is made while computing the assignment's RHS, not after
   the statement.

**No extended-asm operand constraint was tried** (CLAUDE.md's register-
fixing ban applies regardless of purpose, and even setting that rule aside
this residue is not a register-identity mismatch to "fix" so much as a
control-flow/store shape difference).

This is a **STALL**, not a register-identity mismatch (CLAUDE.md's banned-
fix category) -- it is a case where GCC 2.6.3 retains a computation with a
side-effect-free result for reasons this session could not reproduce
through ordinary C, and where the one lever this project has for defeating
dead-code elimination (`volatile`) has an unavoidable side cost (a real
memory store) that a register-only dead value does not pay. A future
attempt might try: giving the "dead" value an actual (never-taken-at-
runtime) consumer that costs zero additional instructions, or investigating
whether GCC 2.6.3's SPECIFIC local-register-allocation heuristics (rather
than its dead-code-elimination pass) are what is responsible for retaining
it, which might point to a different C shape entirely (e.g. a value that
GCC's allocator decides "spans a call" and promotes to a callee-saved
register before dead-code elimination gets a chance to prove it unused --
this round's HEAD BROADCAST on parameter-address-taking forcing home-slot
residency is a related but not identical mechanism; it was not directly
applicable here since `unused` is not an incoming parameter).

## Head-broadcast lever (round 27, callee-saved save/restore pair)

Checked against this function per this round's broadcast: the save/restore
register LIST fully matches retail's (10 registers: `ra`, `fp`/`s8`, `s7`,
`s6`, `s5`, `s4`, `s3`, `s2`, `s1`, `s0`, in both the matched-shape build
and retail). The function is LONG, not short, and the excess is NOT a
callee-saved pair retail lacks -- it is the two `sw`+`j` sequences described
above, which are stack-frame-local (offset 176 in this build), not a
register promotion/demotion difference. The lever does not apply here.

## Proposed learning

`func_800334F0`'s output record is NOT a fixed small "count" struct --
different call sites read arbitrarily far into it depending on what they
need (this call site reads out to relative offset 0x41, versus offset 0
for every other user in this unit). Treat its size as call-site-specific,
per this project's multiple-independent-local-views convention, rather
than assuming the smallest previously-seen instance is the whole story.

Also: a `volatile` local is not a universal drop-in replacement for "a
retail register value that outlives its last real use." It works when the
retained computation's RESULT truly needs to persist untouched (the
`GetMetaEvent` precedent, round 26), but when the value is genuinely
**dead** (no read anywhere, ever), volatile's memory-observability
guarantee adds real store/branch overhead that a true register-only dead
value never had, and the two are not the same lever.

## ROUND 35 (runner alpha): SDK-renamed callees rebuilt and reconfirmed; permuter scaffold REJECTED at sanity-check (frame is 40 bytes off, not 4 words)

Re-verified this round's preserved body builds clean at the recorded score
(`SsUtGetProgAtr`/`SsUtGetVagAtr`/`SsUtSetVagAtr` -- the round-34 SDK
renames of `func_800334F0`/`func_80033260`/`func_80036230` this report
originally used -- are already reflected in `src/psyq/libsnd_seqread.c`'s preserved
`#if 0` body). Rebuilding it in isolation reproduces exactly the same
**380/376 compiled length** this report already recorded (confirmed via
`mipsel-linux-gnu-objdump` symbol-to-symbol distance on
`build/src/libsnd_seqread.c.o`, not funcdiff's drifted word-match number) --
the SDK renames changed nothing about this function's own bytes, as
expected, since they are just this unit's own local-view spelling of
already-linked Sony symbols.

**Set up `tools/setup-permuter.sh` against this preserved body and ran
`--debug --stack-diffs` per `docs/MATCHING-GUIDE.md`'s mandatory
sanity-check before spending any search budget. REJECTED, not searched:**
base score **71818** (penalty breakdown: reorderings 36, insertions 628,
deletions 36, register differences 118 -- an order of magnitude past what a
"4 words long" report predicts) and, more decisively, **`Stack Differences:
2668`, nonzero**, which this report's own title should never have produced
if the frame genuinely differed only in the two documented dead-`$s5`
blocks (a register-only residue with no stack-frame consequence at all).

**Direct measurement settles it: this build's frame is `-0xE0` (224 bytes);
retail's is `-0x108` (264 bytes) -- a 40-byte / 10-word gap**, not the
"4 words longer overall, same frame" picture this report's own opening line
implies. The two facts are not in tension -- "4 words longer" was measured
as a NET, whole-function byte-count difference, and a 40-byte-SMALLER frame
combined with a proportionally larger body elsewhere nets out to a small
positive total -- but the earlier report's per-block direct-objdump
comparisons (the two `unk13==1`/`unk13==2` dead-value sites) never actually
established that everything OUTSIDE those two blocks matches; they were
compared in isolation precisely because the whole-function diff was already
known to be drift-poisoned. This round's permuter sanity-check is the first
measurement that put a NUMBER on how much else differs, and it says: a lot.

**Per `docs/MATCHING-GUIDE.md`'s explicit instruction ("a scaffold that
scores a different residue than the real build is ... scoring something
other than this function's real residue ... not searched"), no search was
run.** This is the same disposition `_SsSetControlChange.md` round 32 recorded
for its own scaffold (base score 4982 against a documented 5-word residue) --
a second confirmed instance of the same failure mode, worth trusting as a
pattern: **a clean, narrow written residue does not by itself guarantee the
REST of the function is close**, and the permuter's own sanity gate is what
catches the gap before it wastes a search budget on a scaffold that isn't
scoring the residue anyone thinks it is.

**What this changes about the recommended next step.** The 40-byte frame
gap means a future attempt should look for a MISSING local (something
retail allocates ~10 words of stack for that this reading's `DataEntryLocals`
/`Scratch800351D0` pair does not currently account for) before trusting
ANY of this report's per-block claims about `unk13==1`/`unk13==2` being the
only residue. The likeliest candidate, given `Snd_setVabAttr.md`'s round-35
finding that its own two by-value struct parameters (a `VagAtr` and
`libsnd_cres.c`'s `UnkStruct80035F3C`) are ENTIRELY caller-marshaled with
no local frame space of their own, is that THIS function's own construction
of those two by-value arguments (the `lwl`/`lwr`/`swl`/`swr` copy sequence
already identified at its two `Snd_setVabAttr` call sites) needs MORE stack
space to stage than this report's `DataEntryLocals`/`Blk1`/`Blk2` reading
currently allocates for it -- not investigated further this round; flagged
for whoever next stages this function.

### Round 35 lever checklist

- **Lever "rebuild before trusting a score"**: done, reconfirmed 380/376 at
  the object level, unchanged from the original report.
- **Permuter**: scaffold built successfully but REJECTED at the mandatory
  `--debug --stack-diffs` sanity check (nonzero stack differences, score
  71818) -- not searched, per the standing rule that a bad scaffold's search
  proves nothing.
- **New finding**: the frame-size gap is 10 words, not the ~1-word residue
  the "4 words long" framing suggested; this is a genuine new measurement,
  not a reinterpretation of an old one.

### Proposed learning

**A function's own report title (a short LENGTH delta) can be a NET figure
that hides a much larger two-sided gap** (frame 10 words smaller here,
body elsewhere correspondingly larger) -- when a later permuter sanity
check reports nonzero stack differences for a residue this report describes
as register-only, that is the tell to re-open the frame-size question
before trusting the report's own "what's left" section. `--stack-diffs`
earns its "ALWAYS pass it" status in `docs/MATCHING-GUIDE.md` twice over: it
catches a false zero (round 18's documented case) AND, as here, it catches a
report whose own scope was narrower than its title implied.

## Round 39 update (runner alpha): re-verified; hoist-both-before-either checked, not applicable; the Snd_setVabAttr "drop the name" lever checked, does not transfer

Rebuilt the preserved near-miss body and cross-checked compiled length
directly via `objdump` symbol-to-symbol distance: `ContDataEntry` is
`0x5f0` bytes (380 words), reproducing "4 words LONG (380/376)" exactly.

**Hoist-both-before-either: not applicable.** The residue is two
genuinely-dead retail computations (a masked/shifted byte computed into
`$s5` on the "in range" path of two independent range checks, never read
anywhere) kept alive across a branch join with zero extra instructions --
not a pair of adjacent loads/multiplies with a later shared consumer.

**Checked whether this round's `Snd_setVabAttr` fix (drop the named local
entirely, let the one real consumer recompute the expression inline)
transfers here** -- it does not, and the reason is the key difference
between the two residues: `Snd_setVabAttr`'s case 12 value IS read, on
exactly one of three converging paths, so "no name, recompute at the one
real use" reproduces retail exactly. This function's `$s5` values are read
on **zero** paths -- there is no "one real use" to recompute the
expression at. This was already tried and measured as this report's own
lever 1 ("a plain dead local... GCC 2.6.3 eliminates the entire
computation... 26 words SHORT"), which is the same failure mode dropping
the name here would hit: with no consumer anywhere, unnamed or not, GCC's
dead-code elimination removes the computation outright. The two residues
share a family (`docs/match-reports/Snd_setVabAttr.md`'s round-39 update
extends this report's own "dead value kept live across a branch" finding)
but are NOT the same fix -- one has a real consumer that a hoisted name
over-commits to a register, the other has none at all and needs the value
kept alive with no consumer whatsoever, which is the harder ask this
report's four already-tried levers (plain dead local, `volatile`,
temporary-avoidance restructuring, bare `__asm__("")` barrier) all fail to
produce for a different reason each time. No new experiment run this
round; `INCLUDE_ASM` unchanged, `build-and-verify.sh` confirmed byte-exact.

## Round 48 update (runner charlie): the round-35 frame-size mystery SOLVED (`dead[40]`); tail-duplication applied; 18/376 -> 48/376 raw word-match, still NOT byte-exact

**Rebuilt round 35/39's preserved body first, in isolation, per this round's
"rebuild before trust" discipline.** Result was NOT what the title's
"380/376, 18/376 raw word-match" figure implies at face value once actually
compared word-for-word: `funcdiff.py` reported **18/376** (not the 380-word
*length* figure, which is a totally different measurement -- objdump
symbol-to-symbol distance, not funcdiff's window compare) with **216426
bytes of whole-image drift outside the function's own range**. This matches
round 35's own finding exactly (a scaffold sanity-check showed the SAME
thing: nonzero stack differences, frame `-0xE0` vs retail's `-0x108`) --
just restated via the plain oracle instead of the permuter's own scorer.
Round 35 flagged this as "not investigated further" and no round since
(round 39 included) picked it up; this round did.

### The frame-size mystery, solved

Direct measurement, `python3 -c` on the disassembly's own `$sp`-relative
literal offsets (every `0x??($sp)` operand in
`asm/nonmatchings/libsnd_seqread/ContDataEntry.s`, both loads/stores):
**the highest offset any instruction in the WHOLE function ever
addresses is `0x90` (144).** The register-save block occupies the frame's
top 40 bytes (`0xE0`-`0x108`, confirmed via `objdump`: `s0`-`s8` (9 regs)
+ `ra`, exactly the same 10-register save this build's own compiled object
also uses -- register COUNT was never the gap). That leaves retail's frame
with **80 bytes (`0x90`-`0xE0`) that no instruction ever touches** -- pure
allocated-but-unaddressed padding, the same shape this project's own
`SeqPlay.md` already named and fixed with a dead array
("declared genuinely dead... this project's established idiom for forcing
frame size onto an otherwise-undersized body").

**Fix: added `u8 dead[40];` (declared with the other top-level locals, C89)
guarded by the established `if (0) { dead[0] = 0; }` idiom.** Rebuilt:
`addiu $sp,$sp,-0x108` -- **frame now byte-IDENTICAL to retail**, confirmed
via `objdump -d build/src/libsnd_seqread.c.o`. (40, not 80: the array itself
only needs to inflate the total frame by the GAP between this build's
already-224-byte frame and retail's 264, not by the full 80-byte
unaddressed span retail happens to carry -- the other 40 bytes of retail's
"unaddressed" region are apparently side-effects of retail's own layout,
e.g. alignment padding the emitted struct order produces, that a smaller
uniformly-packed frame doesn't need to reproduce byte-for-byte to hit the
same TOTAL size.)

This alone did not fix the word-match (still low, since the frame's
byte-shift was previously poisoning every offset in the function), but it
turned the diagnostic picture from "structurally wrong everywhere" into
"aligned with only real, local residues" -- see the tail-duplication finding
below, found by reading the now-meaningful `asm-differ` output.

### The second finding: retail does NOT share one `combine:` tail

With the frame fixed, `tools/asm-differ/diff.py ContDataEntry` (now
alignment-trustworthy near the top of the function) showed the two early
`goto combine;` sites reaching **two DIFFERENT physical copies** of
`rec->unk88 = ReadDeltaValue(a0, a1);` in retail, each with its own
register-widening sequence (one re-widens from `$s8`/reloads from a stack
slot at `0x90`; the other re-widens from `$s1`/`$s0` directly) -- not one
shared block reached by two gotos, which is what this report's preserved
body modeled. This is the project's already-documented tail-duplication
idiom (`_SsSetControlChange.md`'s case-11 finding, `DECOMPILATION_LEARNINGS.md`'s
"an arm that must jump has to be written not-last" family).

**Fix: replaced both `goto combine;` sites with their own inlined
`rec->unk88 = ReadDeltaValue(ch, slot); return;`, and dropped the `combine:`
label** (the third convergence point -- the implicit fallthrough when
neither `unk29==2` nor `unk2A==2` -- keeps the single remaining copy at the
function's natural end, since it is structurally LAST in source order,
matching this project's "the fallthrough goes to whichever candidate is
LAST in source order" rule).

**Result: 18/376 -> 48/376 raw word-match**, a real, oracle-verified
improvement (not a permuter-local score). Compiled length is now 386/376
(10 words OVER, up from 380/376's 4 words over before this round -- the
duplicated tails legitimately cost more instructions than the shared one
did, and retail pays that same cost, so this is expected, not a
regression signal by itself).

### One new lever tried and NEGATIVE: forcing per-iteration recompute of a loop-invariant mask

`asm-differ` on the now-aligned function shows one further concrete
residue: in the `unk13==0 && unk14==0` loop, retail recomputes
`s6 & 0x7F` (the `a2`-derived mask written into `scratch.unkC`/`unkD`)
**fresh via `andi v0,s6,0x7f` on every iteration**, immediately before
each of the two stores it feeds. This build's compiler CSEs and then
loop-invariant-hoists the identical `a2 & 0x7F` expression (both
`scratch.unkC = a2 & 0x7F;` and `scratch.unkD = a2 & 0x7F;` are already
written as two separate un-cached expressions in the source, but GCC
recognizes them as loop-invariant and hoists the shared value out of the
loop into `$s3` regardless).

**Tried:** casting the read through a `volatile` pointer at each site
(`scratch.unkC = (*(volatile u8 *)&a2) & 0x7F;`, same for `unkD`), the
project's documented LICM-defeating instrument used successfully
elsewhere for narrower folds. **Hard NEGATIVE**: regressed to **6/376**
with **274908 bytes of whole-image drift** -- markedly worse than either
the pre-fix or post-tail-duplication state. Reverted immediately;
`build-and-verify.sh` reconfirmed byte-exact after revert. This is a
different LICM shape than the folds `volatile` has fixed before (an
invariant HOIST across loop iterations, not a fold/fusion within one
expression), and the instrument is confirmed the wrong one for it here.

### Current best body (48/376, frame byte-exact, NOT byte-exact overall -- preserved for the next attempt)

**Round 49 note (runner charlie): rewrapped in literal `#if 0`/`#endif`**,
per `tools/stalesyms.py`'s finding (relayed by the head) that this report's
preserved body sat in a plain fenced code block with no `#if 0` markers --
CLAUDE.md's mandated preservation form -- so it was not literally
"positioned where it would compile" the way a copy-paste back into
`src/psyq/libsnd_seqread.c` needs. No change to the body itself, and see the
round-49 note above: this body's `Snd_setVabAttr` call sites still use the
now-stale 3-way (`u32`/`Blk1_800351D0`/`Blk2_800351D0`) argument slice and
will need reshaping to the real `(Scratch_800357B0, AdsrFields)`
two-struct signature before this body can build again.

```c
#if 0
void ContDataEntry(s16 a0, s16 a1, u8 a2)
{
    s16 ch = a0;
    s16 slot = a1;
    Entry90902E8 *rec = &_ss_score[ch][slot];
    u8 *p = (u8 *)rec + rec->unk12;
    DataEntryLocals list;
    Scratch800351D0 scratch;
    s16 i;
    u8 kind;
    u8 dead[40];

    if (0) {
        dead[0] = 0;
    }
    SsUtGetProgAtr(rec->unk4C, p[0x2C], &list);

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
        rec->unk88 = ReadDeltaValue(ch, slot);
        rec->unk29 = 0;
        return;
    }
    if (rec->unk2A == 2) {
        kind = rec->unk16;
        if (kind == 0x10) {
            for (i = 0; i < list.count; i++) {
                Snd_setVabAttr(rec->unk4C, p[0x2C], i,
                              (u32)list.hdrLo | ((u32)list.hdrHi << 16),
                              list.blk1, list.blk2, rec->unk15, a2 & 0xFF);
            }
        } else {
            Snd_setVabAttr(rec->unk4C, p[0x2C], (s16)kind,
                          (u32)list.hdrLo | ((u32)list.hdrHi << 16),
                          list.blk1, list.blk2, rec->unk15, a2 & 0xFF);
        }
        rec->unk88 = ReadDeltaValue(ch, slot);
        rec->unk2A = 0;
        return;
    }
    rec->unk88 = ReadDeltaValue(ch, slot);
}
#endif
```

(Needs the same `Blk1_800351D0`/`Blk2_800351D0`/`DataEntryLocals`/
`Scratch800351D0` typedefs and the local `Snd_setVabAttr` prototype already
declared earlier in `src/psyq/libsnd_seqread.c`, unchanged from before this
round.)

### What is left (not investigated further this round -- flagged for next attempt)

- The per-iteration LICM residue above (mask recompute) -- `volatile`
  tried and NEGATIVE; a different instrument (a scheduling barrier
  INSIDE the loop body, or restructuring to write through a pointer that
  isn't provably the same address each iteration) is untried.
- Register-color differences further into the function (visible in
  `asm-differ` past the section discussed above -- e.g. `s4`/`s5` roles
  in the `unk29==2` dispatch) not yet individually classified.
- The remaining +10-word length-over gap: not yet attributed to a specific
  cause; could be a second, smaller instance of the same
  tail-duplication/shared-block question elsewhere in the function (the
  `unk29==2` and `unk2A==2` return blocks were NOT touched this round).

### Round 48 lever checklist (this function)

- **Rebuild-before-trust**: done -- the previously-recorded "380/376,
  18/376" figures were reproduced exactly, confirming the report's own
  numbers, not stale.
- **Frame-size root cause**: found and FIXED (`dead[40]`, this project's
  established idiom, applied at a new size).
- **Tail-duplication**: found and FIXED for 2 of the (at least) 3
  convergence points on `rec->unk88 = ReadDeltaValue(...)`.
- **`volatile` for LICM-hoisted loop-invariant mask**: tried, NEGATIVE
  (hard regression, 274908 bytes drift) -- the wrong instrument for a
  hoist-out-of-loop residue as opposed to a within-expression fold.
- Net: **18/376 -> 48/376**, frame now byte-exact, `INCLUDE_ASM` restored,
  whole-image `build-and-verify.sh` reconfirmed byte-exact after revert.

### Proposed learning

The `if (0) { dead[N] = 0; }` frame-padding idiom generalizes beyond small
gaps (previously seen at 8 bytes, `SeqPlay.md`) to a 10-word/40-byte
gap here, and the padding amount needed is **the gap between the
compiler's own already-allocated frame and retail's**, not necessarily the
full span of retail's own unaddressed stack region (which was 80 bytes
here, twice the fix that was actually needed) -- measure the CURRENT
build's frame size after ruling out other causes, not just retail's raw
unaddressed-byte count, before sizing the dead array. Also: a scaffold's
"REJECTED, nonzero stack diff" sanity-check verdict (round 35's own
finding) is not just a reason to decline searching -- the nonzero stack
diff IS the frame-size bug already surfacing, and is worth chasing to a
root cause with the plain `$sp`-offset census shown above before writing
the function off to "look for a missing local" without ever doing so.

## Round 49 note (runner charlie): `Snd_setVabAttr` MATCHED -- this function's forward declaration of it was REMOVED, will need reshaping on the next attempt

`Snd_setVabAttr` (called from this function's `unk2A==2` dispatch arm, both
in the preserved body above) matched byte-exact this round (see its own
report). Its REAL signature is `(s16 channel, s16 slot, s16 kind,
Scratch_800357B0 scratch, AdsrFields resolved, s16 arg5, u8 arg6)` --
a 0x20-byte scratch struct plus an 18-byte ADSR struct, both by value.
This function's own forward declaration of it (just above this function's
`#if 0` body in `src/psyq/libsnd_seqread.c`) instead sliced the same 50 bytes as
`(u32 a3, Blk1_800351D0 blk1, Blk2_800351D0 blk2)` -- a different, never-
confirmed guess. Once `Snd_setVabAttr` had a real definition, the two
conflicted (`` conflicting types for `Snd_setVabAttr' ``), so the stale
declaration was REMOVED rather than reconciled (removing it is safe: this
function's own body that used it is itself still `#if 0`, so nothing live
referenced it).

**Whoever next attempts this function will need to:** reshape the two
call sites (`Snd_setVabAttr(rec->unk4C, p[0x2C], i, (u32)list.hdrLo |
((u32)list.hdrHi << 16), list.blk1, list.blk2, rec->unk15, a2 & 0xFF);`
and its non-loop sibling) to pass a `Scratch_800357B0`/`AdsrFields`
pair instead of the `u32`/`Blk1_800351D0`/`Blk2_800351D0` triple, and add
a fresh forward declaration (after `Scratch_800357B0`/`AdsrFields`
are visible, or with local copies of those typedefs declared earlier).
Given `DataEntryLocals`'s own `hdrLo`/`hdrHi`/`blk1`/`blk2` fields already
total exactly 50 bytes at the same stack-relative position, the most
likely resolution is that `DataEntryLocals`'s tail (from `+0x10` onward) IS
byte-identical to the `Scratch_800357B0`+`AdsrFields` pair, just
never named that way from this function's side -- worth checking with
`tools/asm-differ` before re-deriving `blk1`/`blk2`'s "unconstrained"
internal field breakdown from scratch.
the function off to "look for a missing local" without ever doing so.

## Round 97 types pass (echo)

code_179d8_k's local `Entry90902E8` view retired onto `include/ss_score.h`:
the same 0xAC-byte (`SS_SEQ_TABSIZ`) `_ss_score[access][seq]` record that
libsnd_cres, libsnd_decre and libsnd_vmanager already use. The header gained
this unit's fields by splitting padding (no offset, size or existing type
moved); field names stay offset-only (`unkNN`) as the header's convention for
Sony-only fields, with each one's mechanics in its comment. The unit's
`(u8 *)rec + unk12 + 0x17/0x2C` and `(s16 *)((u8 *)rec + 0x4E + ch * 2)`
arithmetic became the header's per-channel arrays `unk17[16]` (pan),
`unk2C[16]` (program) and `unk4E[16]` (volume): `unk12` is the event's MIDI
channel (GetSeqData stores a status byte's low nibble), not a byte offset to
an "embedded state block" as the old local comment read it. Byte-exact
unchanged; the NON_MATCHING object is identical too (objdump of
`build/nonmatching/src/code_179d8_k.c.o` before/after).

## History (moved from src/libsnd_seqread.c, comments pass)

The comment above this function's NON_MATCHING body in src/libsnd_seqread.c read:

> NON_MATCHING: 380/376 words, 4 LONG; 95/376 raw, frame exact (-0x108).
> Residue: retail keeps the two dead `unused` values in a callee-saved
> register ($s5, set and never read) where this body needs `volatile` stack
> slots, and with $s5 free this build hoists the loop-invariant `a2 & 0x7F`
> out of the first loop, which renumbers $s3-$s5 through the loops
> (docs/match-reports/ContDataEntry.md). Hand-derived. Written for the
> reader: the byte-shaped body's `dead[16]` frame pad and `volatile` on the
> two `unused` locals are omitted here and kept in the report.

The forward declaration of Snd_setVabAttr above ContDataEntry carried:

> Snd_setVabAttr is defined later in this unit; its own definition fixes
> this signature (round 49).

The comment on DataEntryLocals, ContDataEntry's frame view, read:

> SsUtGetProgAtr's fill at function entry. From +0x10 the SAME memory is
> both the VagAtr buffer the unk29==2 loops hand to SsUtGet/SetVagAtr
> (retail addresses it at sp+0x58 = list+0x10) and, with the 18 bytes
> after it, the two by-value arguments of Snd_setVabAttr (round 69).
