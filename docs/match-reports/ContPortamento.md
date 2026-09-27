# ContPortamento -- MATCHED: 90/90, byte-exact

## Round 94 (runner echo, track 6): Sony types

`Scratch_80034C28` is gone: a reduced view of Sony's `VagAtr`; the `+0x01` byte (called a velocity-curve flag below) is `VagAtr.mode`, the tone's play mode. `NoteList_800349B0` became `ProgAtr`. Sony's prototypes from `<libsnd.h>` replace the local ones. Bytes unchanged; the bodies below keep their original declarations as history.

> Renamed from `func_80034C28` on 2026-09-23 (tools/rename.py). Address 0x80034c28.

`asm/nonmatchings/code_179d8_k/ContPortamento.s`, vram `0x80034C28`, unit
`code_179d8_k`. Round 25, runner alpha (stall). Round 31, runner bravo
(closed).

## Round 31 update (runner bravo): both residues closed, one via permuter

Re-verified round 25's baseline reproduces exactly: 38/90 raw word-match,
compiled length 89/90 (one short).

**Residue 1 (register-rescue class) -- same fix as `ContPortaTime` and
`ContModulation`.** Caching the byte offset (`u8 offset = rec->unk12;`)
instead of a persistent pointer and recomputing `((u8 *)rec +
offset)[0x2C]` at each of the three use sites closed the missing-word
residue on its own: 89/90 raw match with length now EXACT at 90/90 (the
one remaining word is residue 2 below, an encoding difference, not a
missing instruction). See `ContPortaTime.md` for the decomp-permuter
derivation that found this fix.

**Residue 2 (immediate-constant canonicalization) -- closed via
decomp-permuter.** Set up `tools/decomp-permuter` against the
register-rescue-fixed body above (base score confirmed at 5 via `--debug
--stack-diffs`, matching this report's single-register-difference
description exactly). A 12-way search found a score-0 candidate at
iteration 116:

```c
else
{
  new_var2 = 0xC0;
  if (((u8) (a2 + new_var2)) < 0x40)
  {
    scratch.unk1 = 0;
  }
}
```

**Assigning the literal `0xC0` to an `s32` local FIRST, then adding that
local, defeats the constant-folding pass that otherwise canonicalizes
`(a2 + 0xC0) & 0xFF` to the smaller-magnitude `(a2 - 0x40) & 0xFF`.** This
report's round-25 attempts had already tried reformulating the MASK side
of the expression (explicit `& 0xFF`, pre-masking `a2` before the add) and
both failed to stop the canonicalization -- the untried axis was the
constant's own source form, not the mask's. Translated to idiomatic C:

```c
void ContPortamento(s16 a0, s16 a1, s32 a2)
{
    Entry90902E8 *rec = &_ss_score[a0][a1];
    u8 offset;
    NoteList_800349B0 list;
    Scratch_80034C28 scratch;
    s32 i;
    s32 wrap;

    func_800334F0(rec->unk4C, ((u8 *)rec + (offset = rec->unk12))[0x2C], &list);
    for (i = 0; i < list.unk0; i++) {
        func_80033260(rec->unk4C, ((u8 *)rec + offset)[0x2C], (s16)i, &scratch);
        if ((u8)a2 < 0x40) {
            scratch.unk1 = 2;
        } else {
            wrap = 0xC0;
            if ((u8)(a2 + wrap) < 0x40) {
                scratch.unk1 = 0;
            }
        }
        func_80036230(rec->unk4C, ((u8 *)rec + offset)[0x2C], (s16)i, &scratch);
    }
    rec->unk88 = ReadDeltaValue(a0, a1);
}
```

Byte-exact, 90/90, no drift; `./build-and-verify.sh` whole-image SHA1
green.

### Proposed learning

GCC 2.6.3's strength-reduction of an added constant to its
smaller-magnitude modular equivalent (documented in this report's round-25
half as inescapable regardless of how the MASK is spelled) is defeated by
routing the constant through a separate assignment first. This is the
same "give the compiler an extra step to obscure a compile-time-constant
identity" shape as other levers in this project, but applied to constant
folding rather than to register allocation or instruction scheduling --
worth a `DECOMPILATION_LEARNINGS.md` entry of its own if a similar
immediate-canonicalization residue recurs, since round 25's own two
attempts (varying the mask) explored the wrong half of the expression.

## Original stall report (round 25, runner alpha), preserved below

## What it is

The same `func_800334F0`-count / `func_80033260`+`func_80036230`-per-item
shape as `ContModulation`/`ContPortaTime`, plus a genuinely new piece of
logic: a range check on this function's OWN third parameter (`a2`, wider
than a byte -- retail explicitly `andi`s it, which a true `u8` parameter
would never need) that picks a one-byte "velocity curve" flag written into
the scratch buffer between the two per-item calls.

```c
typedef struct {
    u8 pad0[0x1];
    u8 unk1;    /* +0x01: velocity-curve flag, 2 / 0 / left untouched */
    u8 pad2[0x20 - 0x2];
} Scratch_80034C28;

void ContPortamento(s16 a0, s16 a1, s32 a2)
{
    Entry90902E8 *rec = &_ss_score[a0][a1];
    u8 *p = (u8 *)rec + rec->unk12;
    NoteList_800349B0 list;    /* shared with ContModulation, see that report */
    Scratch_80034C28 scratch;
    s32 i;

    func_800334F0(rec->unk4C, p[0x2C], &list);
    for (i = 0; i < list.unk0; i++) {
        func_80033260(rec->unk4C, p[0x2C], (s16)i, &scratch);
        if ((u8)a2 < 0x40) {
            scratch.unk1 = 2;
        } else if ((u8)(a2 + 0xC0) < 0x40) {
            scratch.unk1 = 0;
        }
        func_80036230(rec->unk4C, p[0x2C], (s16)i, &scratch);
    }
    rec->unk88 = ReadDeltaValue(a0, a1);
}
```

Retail hoists `(u8)a2 < 0x40` OUT of the loop into a precomputed boolean
(`$s5`, set once before the loop) since `a2` never changes across
iterations -- this build's own `-O2` performed the identical
loop-invariant hoist unprompted from the same C, matching retail's shape
here without any manual intervention. The second condition,
`(u8)(a2 + 0xC0) < 0x40` (equivalent to "a2 in [0x40,0x7F]" once wrapped
mod 256, i.e. `a2 - 0x40` computed the "wrapping-add" way), stays inside
the loop in both retail and this build since it also depends on the
freshly-read-back scratch state in between -- actually on inspection it
does NOT depend on per-iteration state either, but retail computes it
INSIDE the loop every iteration rather than hoisting it alongside the
first check; this build reproduces that placement exactly, so the
non-hoist is not itself a residue.

## Residue 1 -- the family's register-rescue class, confirmed a THIRD time

Identical in shape to `ContModulation`/`ContPortaTime` (see those reports
for the full derivation): retail computes the "state block" pointer
(`p`) into `$s0` first, then explicitly rescues it into `$s3`
(`move $s3,$s0`) before repurposing `$s0` as the loop's fixed-point
position accumulator (incremented by `0x10000` per iteration rather than
`1`, a variant of the same "raw counter, sign-extend only where used"
idiom already documented for this family). This build's allocator assigns
`$s3` to `p` directly, eliding the rescue -- one word short, matching the
sibling functions' residue exactly. Three instances now (this one,
`ContModulation`, `ContPortaTime`) confirm this is a genuine CLASS specific
to this "count-then-per-item-loop-with-a-rescued-state-pointer" shape, not
a one-off; see `ContModulation.md` for the axes already tried and ruled out
against it.

## Residue 2 -- a NEW class: immediate-constant canonicalization

```
25500: TARGET addiu v0,s4,0xc0      CURRENT addiu v0,s4,-0x40
25504: TARGET andi  v0,v0,0xff      CURRENT andi  v0,v0,0xff   (unchanged)
```

`(a2 + 0xC0) & 0xFF` and `(a2 - 0x40) & 0xFF` are the SAME value for every
`a2` (192 = -64 mod 256), and this build's compiler substitutes the
smaller-magnitude constant (`-0x40`) once it can see the result gets
masked to 8 bits -- a legitimate strength-reduction retail's own compile
apparently does not apply here (it keeps the literal `+0xC0` from source).
This is a DIFFERENT class from the register-rescue residue: it doesn't
change function length (one `addiu` either way) and doesn't ripple into
anything else in this function (confirmed -- every earlier and later word
matches exactly). It is a pure immediate-encoding byte mismatch.

**Axes tried, both inert (same score, same encoding):**
- `((a2 + 0xC0) & 0xFF) < 0x40` (explicit mask instead of relying on the
  `(u8)` cast) -- regressed length (32/90, extra instructions), reverted.
- `(u8)((u8)a2 + 0xC0) < 0x40` (mask `a2` to a byte BEFORE adding) -- no
  change, compiler still canonicalized to `-0x40`.

**Proposed learning:** when a computed value is immediately masked to a
narrower width (`& 0xFF`, or an implicit truncating store), GCC 2.6.3's
constant folding can substitute an added constant for its
smaller-magnitude modular equivalent even when the literal in the C
source is unambiguous. This is a new residue shape for this project's
`DECOMPILATION_LEARNINGS.md` -- distinct from the already-documented
"wrong TYPE produces register-shaped symptoms" and "commutative-operand-
order" classes, both of which are about REGISTER identity/order, not
about which of two numerically-equivalent IMMEDIATE VALUES the compiler
picks for the same instruction slot. Neither of the two source
reformulations tried here defeated it; whether some other formulation
(a lookup table, a different comparison operator, or splitting the
addition and mask into separate statements not yet tried) would is left
open, flagged rather than asserted.

## Verification

`./build-and-verify.sh` build exit=2 (clean compile, expected -- function
restored to `INCLUDE_ASM`). `funcdiff.py ContPortamento`: 38/90 words
match, compiled length 89/90 (one short, from residue 1 above; residue 2
is a same-length byte mismatch that does not itself change the count).

## Round 97 types pass (echo)

code_179d8_k's local `Entry90902E8` view retired onto `include/SsScore.h`:
the same 0xAC-byte (`SS_SEQ_TABSIZ`) `_ss_score[access][seq]` record that
libsnd_cres, code_179d8_i and code_179d8_j already use. The header gained
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
