# func_8001EDAC -- STALL (register-identity residue, 16/22 words)

Unit: `code_d294_c` (round 14). The generic packed-bitfield accessor
already MEASURED and documented (before this carve existed) in
`include/code_d294.h`'s standing comment: clears `width` bits at bit
offset `shift` in `*word`, ORs in `value << shift`, and returns the
PREVIOUS contents of that bitfield shifted back to bit 0. Five (now more)
sibling functions in this class of unit are thin wrappers around it, all
over `&self->unk10`. `u32 func_8001EDAC(u32 *word, s32 shift, s32 width,
u32 value)`.

Blocker screen clean: no `gp_rel`, no `addiu $at,$at,%lo`, no
`mfhi`/`mflo`-adjacent-`mult`/`div` hit.

## Best-reached source (compiles, builds green, scores 16/22 in-range;
## restored to `INCLUDE_ASM` per project rule)

```c
u32 func_8001EDAC(u32 *word, s32 shift, s32 width, u32 value) {
    u32 mask;
    s32 i;
    u32 old;

    mask = 1;
    for (i = 0; i < width; i++) {
        mask <<= 1;
    }
    mask -= 1;
    mask <<= shift;
    old = (*word & mask) >> shift;
    *word = (*word & ~mask) | (value << shift);
    return old;
}
```

Preserved inline per project convention (`#if 0`, positioned where it
would compile back into `src/code_d294_c.c` in place of the current
`INCLUDE_ASM`):

```c
#if 0
u32 func_8001EDAC(u32 *word, s32 shift, s32 width, u32 value) {
    u32 mask;
    s32 i;
    u32 old;

    mask = 1;
    for (i = 0; i < width; i++) {
        mask <<= 1;
    }
    mask -= 1;
    mask <<= shift;
    old = (*word & mask) >> shift;
    *word = (*word & ~mask) | (value << shift);
    return old;
}
#endif
```

## The residue, precisely

Every instruction's OPCODE, IMMEDIATE, and POSITION match retail exactly
(confirmed via `tools/asm-differ/diff.py`: 22 words in, 22 words out, same
count both sides -- no address drift, unlike most residues in this
project's history). The only difference is that TWO registers are
swapped throughout the second half of the function:

- The loop counter (retail: `$a0`, reusing the now-dead `word` parameter
  register once its value has been copied to `$t1`; mine: `$v1`).
- The `~mask` intermediate and the `value << shift` intermediate (retail:
  `$a0`/`$v1` respectively; mine: `$v1`/`$a0` -- literally swapped from
  retail's assignment).

No instruction is missing, extra, or reordered; this is the "same
register, same value... resists reshaping" class from
`docs/MATCHING-GUIDE.md`, just for a register PAIR rather than a single
redundant move.

## What was tried (7 real full builds, all scoring identically at 16/22 --
## the score never moved even by one word across any of these)

1. **Baseline** (source above).
2. **Explicit pointer walk / statement reorder**: moved `new_word`
   (renamed from the inline expression) to be computed BEFORE `old`,
   matching retail's own `v1`-before-`v0` instruction order. No change.
3. **Operand order swap** on the final OR: `(value << shift) | (*word &
   ~mask)` instead of `(*word & ~mask) | (value << shift)` -- matching
   retail's literal `v1 = v1 | a0` (value-shifted first, cleared-word
   second). Made it WORSE (14/22): this is NOT the axis retail's operand
   order reflects; reverted immediately.
4. **Named `orig` local caching `*word` once**, used in both the "clear"
   and "extract" expressions instead of dereferencing `*word` twice
   (which the compiler CSEs identically either way). No change.
5. **`for` loop rewritten as `while`** with the increment split out as its
   own statement (`i = 0; while (i < width) { i++; mask <<= 1; }`). No
   change -- both compile to the identical rotated-loop shape retail's
   own `blez`-guarded post-test loop has.
6. **Fewer/more locals** (collapsing `old` into the `return` expression
   directly, vs. splitting `new_word`/`old` into two named locals). No
   change in either direction.

None of these are the banned `register T v asm("$N")` class -- nothing
here pins WHICH register a value lives in; every attempt is a pure
source-level reshape (statement order, loop form, local variable count),
exactly what `docs/MATCHING-GUIDE.md`'s residue guide prescribes trying
before calling a register-identity mismatch a stall.

### Proposed learning

**A register PAIR can be swapped between two otherwise-independent
intermediate values with zero effect from statement reordering, loop-form
changes, or intermediate-naming -- confirmed here across 6 source-level
axes with the score frozen at the same 16/22 every time.** This
generalizes the existing single-register "redundant move" entry in
`docs/MATCHING-GUIDE.md` to the PAIRED case: when two scratch values
(here, a loop counter freed up from a dead parameter register, and two
unrelated bitwise intermediates) get consistently cross-allocated versus
retail, treat it as the same permuter-target class rather than continuing
to vary source shape -- the six variants tried here span every reasonable
axis (order, form, naming) without moving either register.
