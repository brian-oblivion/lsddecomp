# ContModulation -- MATCHED: 79/79, byte-exact

## Round 94 (runner echo, track 6): Sony types

`NoteList_800349B0` and `Scratch_800349B0` are gone: they were reduced views of Sony's `ProgAtr` and `VagAtr` (`include/psyq/libsnd.h`), and the unit now includes `<libsnd.h>` and uses those types and Sony's `SsUtGetProgAtr`/`SsUtGetVagAtr`/`SsUtSetVagAtr` prototypes. `unk0` is `ProgAtr.tones`, the stamped `+0x08` byte is `VagAtr.vibW` (vibrato depth). Bytes unchanged. The bodies below keep their original declarations as history.

> Renamed from `func_800349B0` on 2026-09-23 (tools/rename.py). Address 0x800349b0.

`asm/nonmatchings/libsnd_seqread/ContModulation.s`, vram `0x800349B0`, unit
`libsnd_seqread`. Round 25, runner alpha (stall). Round 31, runner bravo
(closed).

## Round 31 update (runner bravo): closed, two independent fixes stacked

Re-verified round 25's baseline reproduces exactly before touching
anything: 28/79 raw word-match with the correct-offset (0x28-total)
struct, drift warning present exactly as this report already documents
(the 8-byte frame overshoot shifts everything downstream).

**Fix 1 -- the register-rescue residue, shared with `ContPortaTime` and
`ContPortamento` (see `ContPortaTime.md` for the decomp-permuter derivation
that found it first).** Caching the raw byte offset (`u8 offset =
rec->unk12;`) instead of a persistent pointer (`u8 *p = (u8 *)rec +
rec->unk12;`), and recomputing `((u8 *)rec + offset)[0x2C]` at each of the
three call sites, reproduces retail's register-rescue `move` because the
allocator no longer needs to keep a dedicated pointer register alive
across the loop. Applied alone (struct still at its correct-offset 0x28
total size), this closed the register-rescue residue and moved the score
to 59/79 -- but the frame-size residue (2) below was still present and
still drifting.

**Fix 2 -- the frame-size residue, now separable from the field offset.**
This report's round-25 finding described the frame-size fix as
inseparable from a WRONG field offset ("shrinking the SAME struct to
0x20 bytes... also puts `unk8` at the WRONG address"). That coupling was
an artifact of also removing the struct's LEADING pad; it is not
necessary. Keeping `unk8` at its own correct relative offset (`pad0[0x8]`
still in front of it) and shrinking only the TRAILING padding --
`pad9[0x20 - 0x9]` instead of `pad9[0x28 - 0x9]`, i.e. declaring the
struct's total size as 0x20 instead of 0x28 while `unk8` stays at +0x08
-- reproduces retail's exact `-0x70` frame with `unk8` at its correct
absolute address. Once fix 1 was already in place (lower register
pressure from the loop), this shrink carried no other side effect:
79/79, byte-exact, no drift.

```c
typedef struct {
    u8 pad0[0x8];
    u8 unk8;    /* +0x08: byte stamped between the two per-item calls */
    u8 pad9[0x20 - 0x9];   /* struct total is 0x20, NOT 0x28 -- see above */
} Scratch_800349B0;

void ContModulation(s16 a0, s16 a1, u8 a2)
{
    Entry90902E8 *rec = &_ss_score[a0][a1];
    u8 offset;
    NoteList_800349B0 list;
    Scratch_800349B0 scratch;
    s32 i;

    func_800334F0(rec->unk4C, ((u8 *)rec + (offset = rec->unk12))[0x2C], &list);
    for (i = 0; i < list.unk0; i++) {
        func_80033260(rec->unk4C, ((u8 *)rec + offset)[0x2C], (s16)i, &scratch);
        scratch.unk8 = a2;
        func_80036230(rec->unk4C, ((u8 *)rec + offset)[0x2C], (s16)i, &scratch);
    }
    rec->unk88 = ReadDeltaValue(a0, a1);
}
```

### Proposed learning

The round-25 report treated "correct field offset" and "correct total
struct size" as a single coupled axis (shrink the struct -> the field
moves too), because the only shrink attempted also relocated the field.
They are actually independent: a struct's DECLARED total size only needs
to reflect how much of it the compiler must actually reserve stack space
for, not the true size of whatever object retail's compile treated as
occupying that region beyond the last field this project has discovered.
Padding this project's own view of a struct as smaller than a field's own
offset would suggest is safe as long as no other code reads or writes
past the declared end -- which is exactly this project's own
"independent local views need not agree with retail's real struct" idiom,
here applied to a struct's tail rather than its head.

## Original stall report (round 25, runner alpha), preserved below

**The title's 28/79 is the score for the structurally-CORRECT scratch
struct preserved below** (the one whose field offset actually matches
retail's `sb $s7,0x28($sp)`). A DIFFERENT, structurally-wrong variant
(scratch struct shrunk to 0x20 bytes, putting the stamped field at the
WRONG absolute address) scores higher, 38/79, purely by accident of stack
layout -- see "Axes tried" below. The 28/79 figure is the honest one for
the body actually kept.

## What it is

A per-(channel, slot) "dispatch note events" function. It calls
`func_800334F0(rec->unk4C, p[0x2C], &list)` to fill a small stack-local
struct whose first byte is an item count, then loops that many times
calling `func_80033260` and `func_80036230` (both still unmatched
elsewhere, no established prototype) with a per-item scratch struct in
between which it stamps one byte (`a2`, the function's own third
parameter) at offset 8. It finishes with the same
`rec->unk88 = ReadDeltaValue(channel, slot);` tail every function in this
family has.

```c
typedef struct {
    u8 unk0;    /* +0x00: item count, written by func_800334F0 */
    u8 pad1[0x10 - 0x1];
} NoteList_800349B0;

typedef struct {
    u8 pad0[0x8];
    u8 unk8;    /* +0x08: byte stamped between the two per-item calls */
    u8 pad9[0x28 - 0x9];
} Scratch_800349B0;

extern s16 func_800334F0(s16 a0, s16 a1, void *out);
extern void func_80033260(s16 a0, u8 a1, s16 a2, void *out);
extern void func_80036230(s16 a0, u8 a1, s16 a2, void *out);

void ContModulation(s16 a0, s16 a1, u8 a2)
{
    Entry90902E8 *rec = &_ss_score[a0][a1];
    u8 *p = (u8 *)rec + rec->unk12;
    NoteList_800349B0 list;
    Scratch_800349B0 scratch;
    s32 i;

    func_800334F0(rec->unk4C, p[0x2C], &list);
    for (i = 0; i < list.unk0; i++) {
        func_80033260(rec->unk4C, p[0x2C], (s16)i, &scratch);
        scratch.unk8 = a2;
        func_80036230(rec->unk4C, p[0x2C], (s16)i, &scratch);
    }
    rec->unk88 = ReadDeltaValue(a0, a1);
}
```

`func_800334F0` is `func_800334F0(s16 a0, s16 a1, Entry8E968 *out)` per
libsnd_decre.c's own reduced view (a 0x10-byte struct); this unit's own
call site only needs to read `out->unk0` (the count), so the local view
above only names that one field, per the project's own multiple-
independent-local-views convention.

## Two locals, not one -- the aliasing finding

The loop's exit test (`i < list.unk0`) re-reads `list.unk0` from memory on
EVERY iteration in retail's disassembly, even though nothing in this
function's own source writes it after the first call. First guess was a
single combined struct spanning both regions (so the compiler couldn't
prove no aliasing since one address escapes). Splitting into two
independent locals (`list`, `scratch`) reproduces the SAME reload
behavior and scores noticeably better (28/79 -> 38/79 in one rebuild) --
GCC 2.6.3 is conservative enough that ANY external call after a local's
address has escaped forces a reload of every address-taken local, not just
the one whose address was passed to that particular call. **Proposed
learning:** when a loop's exit condition rereads a struct field from
memory on every iteration despite no visible write in the function's own
source, don't reach for one combined struct as the explanation -- check
whether ANY local in the function has had its address taken and escaped
to an external call; GCC's aliasing model here appears to invalidate all
of them, not just the aliased one.

## The two residues that did not close

**(1) One missing instruction, a register-rescue `move`.** Retail computes
the "state block" pointer (`p`) into `$s0` first, then explicitly copies it
to `$s4` (`move $s4,$s0`) immediately before repurposing `$s0` as the loop
counter's sign-extended holding register. This build's allocator picks
`$s4` for `p` DIRECTLY from the point it is computed, never touching `$s0`
for that value at all, so the rescue copy has nothing to do and is elided
-- one fewer instruction, everything after ripples by a word. This is the
same class of residue as `ContNrpn1`'s (an allocator choosing a more
economical register path than retail's), not a logic difference, and none
of the usual source-shape levers changed it: neither reordering the local
declarations (`p` before/after `list`/`scratch`/`i`) nor changing the loop
counter's type (`s16` vs `s32`, with or without a separate `idx` copy)
affected which register `p` lands in.

**(2) Frame size still 8 bytes over.** With `Scratch_800349B0` sized to
its structurally-correct 0x28 bytes (offset 8 for `unk8`, matching
retail's `sb $s7,0x28($sp)`), this build's frame comes out `-0x78` against
retail's `-0x70`. Shrinking the SAME struct to 0x20 bytes (folding away the
leading 8-byte pad, which also puts `unk8` at the WRONG address,
`sp+0x20` instead of `sp+0x28`) happens to produce the CORRECT `-0x70`
frame size instead -- i.e. this build's stack allocator is spending 8
bytes on something (most likely a spill slot for the `s32 i` loop counter)
that in retail's compile apparently lands inside what this report is
modeling as `Scratch_800349B0`'s own declared bytes, rather than in
separate frame space. The two structs used in this report are almost
certainly not laid out at exactly the addresses retail's real locals sit
at; a further axis worth trying (not yet attempted, flagged rather than
asserted) is combining `i`'s storage back into the scratch struct's own
padding explicitly (i.e. treat the pad as a spill target on purpose) rather
than leaving it to the two independently-sized locals above.

**Axes tried for the two residues, all inert or regressive within this
report's budget:** combined single struct (regressed both residues,
28/79); split struct with wrong offset (fixed the frame size by luck,
wrong field address, 38/79); split struct with correct offset (fixed the
field address, frame size regressed again, 28/79); loop counter as `s16`
directly (extra spurious re-narrowing instructions after `i++`, worse);
loop counter as `s32` with a separate `s16 idx` snapshot per iteration
(matched the per-iteration codegen shape exactly but did not change either
residue); reordering local declarations (`p` first vs. last -- inert on
both residues).

## Round 27 update (runner alpha): two more axes tried against residue 1, both inert

Re-verified the round-25 baseline reproduces exactly (28/79, correct-offset
struct). Per this round's instruction to cross-check the one-word-short
cluster, two further axes against the register-rescue residue, neither
moving the score:

- **Explicit `s16 ch = a0; s16 slot = a1;` locals, used only at the final
  `ReadDeltaValue(ch, slot)` call** (mirroring retail's own apparent
  channel/slot rescue into `$s5`/`$s6` visible in the disassembly, on the
  theory that reproducing THOSE rescues might shift enough register
  pressure to also reproduce `p`'s). **Inert** -- GCC folds the trivial
  copies away identically to the bare-parameter form; byte-for-byte
  identical output. (Same null result as trying the equivalent lever on
  `ContDataEntry` this round, for what that is worth as a second data
  point on this class of "does introducing a same-valued named local
  change anything" experiment.)
- **Inlining `p[0x2C]` as `((u8 *)rec + rec->unk12)[0x2C]` at both call
  sites instead of a cached `u8 *p`** (removing the persistent pointer
  local entirely, forcing recomputation at each site). **Regressive**:
  18/79, with the compiled length unchanged (still 79 words total) but
  a different, worse register allocation throughout -- reverted.

**Cross-cluster answer (this round's explicit ask):** `ContNrpn1`'s
residue (a fused sign-extend-and-multiply shift, see that report) is
CONFIRMED NOT the same cause as this family's register-rescue residue --
the two are structurally unrelated (one is about an index multiply
collapsing into a sign-extension because a value is used once; this one is
about a persistent pointer register needing an extra `move` before the
loop counter reclaims its original register), and no lever tried against
either transferred to the other.

## Verification

`./build-and-verify.sh` build exit=2 (clean compile, expected -- function
restored to `INCLUDE_ASM`). `funcdiff.py ContModulation`: compiled length
78/79 words (one short) with the correct-offset struct above; raw
word-match figures cited per variant above were all measured before
reverting.

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

## History (source comments moved in track 12, round 106)

From `src/psyq/libsnd_seqread.c`:

> The shared comment above the three VagAtr editors said "`tones` is re-read
> from memory on every iteration because the compiler cannot prove the SsUt
> calls leave the ProgAtr alone."
