# ReadDeltaValue -- MATCHED (47/47)

> Renamed from `func_80035E80` on 2026-09-23 (tools/rename.py). Address 0x80035e80.

`asm/nonmatchings/code_179d8_k/ReadDeltaValue.s`, vram `0x80035E80`, unit
`code_179d8_k`. Carved round 24, closed round 25, runner alpha. This
unit's one frameless function -- it opens on `sll $a0,$a0,16` (leaf
argument narrowing, not a caller-frame read), per the unit's own
carve-time census.

## What it is

The shared "advance the sequencer cursor" helper every other function in
this unit's family calls and caches the return value from. It decodes ONE
MIDI-style big-endian 7-bit-per-byte variable-length quantity from
`rec->unk4` (a byte cursor into a per-voice event stream, advanced as it
reads), scales the decoded magnitude by 10, adds that to `rec->unk80` (a
running tick position), and returns the scaled delta. A first byte of `0`
is a sentinel for "no delta due" -- it returns `0` immediately without
touching `rec->unk80` at all (retail explicitly sets `$v0=0` and jumps
past the accumulate-and-store code entirely; this is not merely "the
value happened to be zero").

```c
s32 ReadDeltaValue(s16 a0, s16 a1)
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
```

## History: stalled at 44/47 for a full round (round 24), closed round 25

Round 24 got this to length-EXACT (47/47) with 44/47 raw word-match, using
three levers already documented and unchanged here (all still load-bearing,
kept verbatim from the prior report):

1. **`s32`, not `u8`, for the running accumulator** -- a `u8`-typed
   accumulator produces a spurious `andi reg,reg,0xff` on every use because
   GCC 2.6.3 re-widens a *named* QImode variable on every use.
2. **Recompute the scaled value into a THIRD variable (`result`) at the
   tail**, rather than reassigning `val` itself, so the final combine gets
   a fresh register (`$v0`) with no bridging `move`.
3. **Statement order matching retail's own load-then-store-then-load
   sequence** (`rec->unk4 = cursor + 1;` textually BEFORE `acc = *cursor;`)
   reproduces a genuine MIPS-I load-delay-hazard `nop` retail has.

The remaining 3-word residue (word 15's already-SETTLED commutative-
operand-order class, plus words 27/39/40 -- a single register-identity
choice for `val`, landing in `$v1` against retail's `$v0` at BOTH
computation sites and the one place it's consumed) stalled for a full
round. Round 24's report flagged the axis as untested: "whether the four
[here, two] reads should be structured as a single call expression at
all... none has varied."

## The round-25 fix: an if/else ARM ORDERING lever, not an expression reshape

**This closed on the round-25 head's broadcast about block order, found
independently of the broadcast's own two worked examples (`CheckDreamAuxTriggerCondition`,
`Entity__UpdateActivationState`) but the SAME mechanism.** The prior C wrote:

```c
val = acc * 4;                 /* (A) unconditional, before the branch */
if (acc & 0x80) {
    ...
    val = acc * 4;              /* (B) inside the branch, LAST in source order */
}
result = (val + acc) * 2;
```

Both `val` writes reach the SAME merge point (`result = ...`), and per the
round-25 lever, **GCC 2.6.3 gives the shared merge-point's fallthrough
register preference to whichever write is LAST in source order** -- here,
write (B), inside the loop's own tail. Retail's disassembly, however,
shows the SKIP-LOOP write (the direct analogue of (A)) landing in `$v0`
(the "preferred"/fallthrough-shaped register), with the LOOP-EXIT write
(B) computed via an explicit code path that does NOT get the same
treatment. That is backwards from what (A)-then-(B) source order gives.

Inverting the guard and writing the skip-loop arm EXPLICITLY, with its own
`goto` over the loop code (making it structurally the "jump" arm instead
of the implicit "always runs first" one, and leaving the loop's own `val`
write as the version reached by simple textual/CFG fallthrough) closed
the residue completely in one rebuild -- 44/47 to 47/47, and the
whole-image SHA1 verifies. **Zero logic changed**; this is purely which
of two textually- and CFG-equivalent orderings of the same two statements
GCC's register allocator treats as "the" canonical path.

## Answering the round-25 head's explicit question

**Did an unconditional `j` to a join with real work in its delay slot
appear here, and did the block-order lever apply?** Yes to both. Retail's
own disassembly has exactly this diagnostic: `j .L80035F34` (the
early-return-0 path) with `addu $v0,$zero,$zero` (setting the return value
to 0) in its delay slot -- real work, not a `nop`. The C's `if (acc == 0)
{ return 0; }` early return already reproduced this correctly from the
very first draft (it was never part of the residue). The RESIDUE that
needed the lever was a second instance of the same shape one level
deeper in the function: the `val` write inside `if (acc & 0x80)` versus
before it, which is the "duplicated write reaching a merge point" shape
(closer to `Entity__UpdateActivationState`'s shape than `CheckDreamAuxTriggerCondition`'s), not the
early-return jump itself.

## Verification

`./build-and-verify.sh`: **exit 0, `OK: build matches retail
SLPS_015.56`** -- whole-image byte-exact. `funcdiff.py ReadDeltaValue`:
47/47 words match.
