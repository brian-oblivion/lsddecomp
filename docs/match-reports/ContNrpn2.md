# ContNrpn2 -- MATCHED: 82/82, byte-exact

> Renamed from `func_80034F90` on 2026-09-23 (tools/rename.py). Address 0x80034f90.

`asm/nonmatchings/code_179d8_k/ContNrpn2.s`, vram `0x80034F90`, unit
`code_179d8_k`. Round 25, runner alpha (stall). Round 31, runner bravo
(closed).

## Round 31 update (runner bravo): the "delay-slot filler" WAS a missing statement, not a scheduling quirk

Re-verified round 25's baseline reproduces exactly: 77/82, length exact,
same 5-word residue (a `sltiu` this build's compiler places in the
`bnez`'s own delay slot, where retail places it after the `unk28==0`
arm's three instructions instead).

**Re-reading the disassembly rather than the report's own framing found
the actual cause.** MIPS branch delay slots execute UNCONDITIONALLY,
regardless of whether the branch is taken. Retail's `bnez $v0,<target>`
(the `rec->unk28 == 0` test) has `sb $a2,0x16($s0)` -- i.e. `rec->unk16 =
a2` -- in its delay slot. That instruction therefore executes on BOTH the
taken and not-taken paths of this branch, not just on the fallthrough
(`unk28==0`) arm the stalled C modeled it as belonging to exclusively.
The stalled body wrote `rec->unk16 = a2;` only inside the `if (rec->unk28
== 0)` block; retail's actual behaviour is to write it once, UNCONDITIONALLY,
for the entire `case 0x1E` -- the round-25 report's framing of this as a
"pure instruction-scheduling residue" was the wrong diagnosis. It is a
genuine control-flow/semantics difference (one statement in the wrong
scope), which merely LOOKED like scheduling because moving a store
between "runs on one path" and "runs on both paths" changes only which
delay slots are eligible to hold it, not any value computed.

Moving the statement out of the inner `if` and to the top of `case 0x1E`
(still functionally harmless on the other two sub-paths, since `a2 ==
0x1E` for the whole case and `rec->unk16` is not read again before this
case's own end) closed the whole residue in one rebuild:

```c
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
```

Byte-exact, 82/82, no drift; `./build-and-verify.sh` whole-image SHA1
green.

### Proposed learning

**A delay-slot filler that "looks like it belongs" to one branch arm is
worth checking against BOTH sides of that branch before accepting the
"dead-but-scheduled filler" explanation.** The round-25 report's own
residue description was accurate about the SYMPTOM (this build placed
the independent `sltiu` computation one slot earlier than retail) but
wrong about the CAUSE: it assumed the delay-slot instruction's home was
the fallthrough arm alone, when the defining property of a delay slot is
that it runs on EITHER outcome. Any statement retail places in a
branch's delay slot is, by construction, safe to execute on both paths
-- which is a positive signal that the same statement may belong
OUTSIDE the branch entirely (hoisted to before it) rather than inside
one arm, especially when (as here) the value it writes is a constant
already implied by the surrounding `switch` case.

## Original stall report (round 25, runner alpha), preserved below
## What it is

A per-(channel, slot) "note-off / control" dispatcher, switching on the
low byte of its third parameter (`a2 & 0xFF`):

```c
void ContNrpn2(s16 a0, s16 a1, u8 a2)
{
    Entry90902E8 *rec = &_ss_score[a0][a1];
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
        if (rec->unk28 == 0) {
            rec->unk16 = a2;
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
```

`kind == 0x14` arms a latch (`unk27 = 1`) and snapshots the cursor
(`unk4`) into a new field, `unkC`, added to this unit's own
`Entry90902E8` view this round (a `u8 *` -- see below). `kind == 0x1E` is
a "note release" with its own small countdown at `unk28`: at zero it just
re-stamps `unk16`/clears `unk10`; below `0x7F` it decrements, calls the
shared VLQ-advance helper, and either restores the cursor from the
`unkC` backup or clears `unk10` depending on whether the countdown
reached zero; saturated at `>=0x7F` it still calls the helper (for its
side effect -- the RETURN VALUE is thrown away, `rec->unk88` is
unconditionally zeroed instead) and unconditionally restores the cursor.
Everything else falls to a default path that just re-stamps `unk16` and
bumps a step counter (`unk2A`).

## New struct field

`+0xC unkC` (`u8 *`): a saved backup of `unk4` (the byte-stream cursor),
restored into it on the "0x1E, still counting down" resume path. Carved
out of the existing `pad8[0x10-0x8]` padding array without moving any
other field or `sizeof`, the same safe-split pattern used for this
round's other new fields in this unit -- verified with a byte-exact whole
image rebuild.

## Two source-shape findings, both confirmed as levers this round

**1. Write the switch as `switch`, not as sequential nested `if`s "for
readability."** A first draft used `if (kind==0x14) {...return;} if
(kind==0x1E) {...} else {...}`. Retail's disassembly places the `0x14`
and `0x1E` handler CODE physically AFTER both comparisons (a `beq` to
each, with the "neither" case falling through to an explicit `j` PAST
both handler blocks to the default code) -- the classic sequential-
compare switch lowering. The nested-`if` draft inverted the first test
(`bne ... skip-14-block`) instead of an affirmative `beq` to the handler,
which is a different branch shape entirely and cost the whole rest of the
function's alignment (25/82 raw). Rewriting as an actual C `switch`
statement over the same three arms reproduced retail's exact branch
shape immediately (36/82 in one rebuild, before any other fix).

**2. The head's round-25 block-order lever (an if/else arm ordering
retail keeps via an explicit `goto`/`j`, not a fallthrough) applies here,
found independently before it was broadcast.** Retail's `kind==0x1E`
case has an unconditional `j .L800350B8` (with real work, `sb
a2,0x16(s0)`, in its delay slot) whose EXISTENCE is only explained if the
compiler had a block placed AFTER it in source order -- i.e. the
`unk28==0` arm was NOT the last arm textually. The original draft put
`if (rec->unk28 == 0) { ...; break; }` FIRST with the countdown logic
falling through last, which is the natural reading order but gives GCC
the fallthrough for the WRONG arm. Restructuring so the `unk28==0` arm's
own final call (`rec->unk88 = ReadDeltaValue(...); return;`) is written
out explicitly rather than `break`-ing to a shared tail, and confirming
the countdown logic remains the block reached by simple fallthrough,
reproduced retail's jump exactly and took the score from 38/82 to 68/82
in the same rebuild that also fixed finding 3 below.

**3. Statement order inside an arm matters when a stored value is
recomputed independently.** The `0x1E`/`unk28<0x7F` arm originally read
`result = ReadDeltaValue(a0, a1); rec->unk28--; rec->unk88 = result;` --
decrement AFTER the call. Retail decrements BEFORE the call (computing
the decremented byte into a register that survives into the call's own
delay slot for the store). Swapping the two statements' order
(decrement first, call second) took the byte-store's immediate encoding
from a spurious zero-extended `0xff` mismatch to matching, and combined
with using `rec->unk28--;` rather than `rec->unk28 = rec->unk28 - 1;` for
the decrement itself (the compound form alone made no difference; the
statement reorder was what mattered) brought the function to its final
82/82-length, 77/82-word state.

## The residue that did not close (5 words, one class)

```
25830: TARGET bnez v0,25840        CURRENT bnez v0,25844   (branch target only, symptom)
25834: TARGET sb  a2,0x16(s0)      CURRENT sltiu v0,v0,0x7f  <- real diff: hoisted one slot early
25838: TARGET j   350b8             CURRENT sb  a2,0x16(s0)
2583c: TARGET sb  zero,0x10(s0)    CURRENT j   350b8
25840: TARGET sltiu v0,v0,0x7f     CURRENT sb  zero,0x10(s0)
```

`sltiu $v0,$v0,0x7f` depends only on the already-loaded `rec->unk28` byte
and nothing writes that register before its result is used (at the
`beqz` a few instructions later), so it is free to be scheduled anywhere
in that window. This build's scheduler fills the `bnez`'s own delay slot
with it (the earliest legal point); retail's compiler places it at the
LATEST legal point instead (right before its first use, after the
unrelated `unk28==0` arm's own code). Both are correct MIPS -- this is a
pure delay-slot-filling/scheduling preference, not a logic difference:
every value computed is identical, only WHEN the independent `sltiu`
executes differs.

**Axes tried, both inert:**
- A bare `__asm__("")` scheduling barrier placed between the `unk28==0`
  arm's `return` and the `if (rec->unk28 < 0x7F)` test (the allowed form
  per CLAUDE.md's register rule, since it does not pin a register) --
  this ADDED an instruction rather than blocking the hoist, regressing
  the score to 37/82 with an out-of-range drift warning. Reverted.
- No other reordering of the `unk28==0` arm's own three statements
  changed which delay slot the compiler chose for `sltiu`.

**Proposed learning:** an independent, side-effect-free instruction that
this build hoists into an EARLIER branch's delay slot than retail uses is
a distinct residue shape from the two the round-25 head broadcasts
already cover (if/else arm order and duplicated-assignment order) --
both of those are about which SOURCE BLOCK falls through, and this one is
about which of several EQUALLY VALID delay slots a single scalar
computation lands in once the block order already matches. A bare
`__asm__("")` barrier, which works for the two broadcast shapes by
blocking cross-block movement, does not block this kind of same-value
delay-slot retiming and instead costs a real instruction; whether some
OTHER barrier placement or a different value order avoids it is left
untried, flagged rather than asserted, given the diminishing returns
after three real structural fixes already landed 77/82.

## Verification

`./build-and-verify.sh` build exit=2 (clean compile, expected -- function
restored to `INCLUDE_ASM`). `funcdiff.py ContNrpn2`: 77/82 words
match, compiled length exact (82/82, no outside-range drift).

## Round 97 types pass (echo)

code_179d8_k's local `Entry90902E8` view retired onto `include/SsScore.h`:
the same 0xAC-byte (`SS_SEQ_TABSIZ`) `_ss_score[access][seq]` record that
libsnd_cres, libsnd_decre and code_179d8_j already use. The header gained
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
