# CD_readm

> Renamed from `func_8002ADE8` on 2026-09-23 (tools/rename.py). Address 0x8002ade8.

**Unit:** libcd_bios · **Size:** 62 words · **Status:** MATCHED (62/62 words)

## What it does

Configures the link driver's mode word (`D_8006D8F0`) from `(arg2 & 0x30)`,
stages the three call arguments and the current `CD_cbsync`/`CD_cbready`
callback pointers into the driver's staging globals, resets the retry
counter (`D_8006D8DC = 8`), optionally kicks `CD_cw(9, 0, 0, 0)` if
`CD_status & 0xE0`, then calls `CD_sync(0, 0)` and
`cd_read_retry()`, reducing the latter's result to a `0`/`-1` status.

## The C

```c
s32 CD_readm(s32 arg0, s32 arg1, s32 arg2)
{
    s32 t;
    volatile s32 *p;

    p = &D_8006D8EC;
    *p = arg2;
    t = *p & 0x30;

    /* Retail keeps all three D_8006D8F0 stores as separate, unmerged blocks
     * (three distinct address computations -- two folded through $at, one
     * unfolded through a real GPR) instead of the single shared store GCC's
     * cross-jump/tail-merge pass produces from the equivalent if/else-if/else
     * or switch. The barrier on case1 and the local `volatile s32 *` pointers
     * on case3/case2's-neighbour below are what keeps each store distinct
     * enough that the merge heuristic can't unify them -- removing any one
     * of the three re-merges a pair and drops 4-24 bytes. This is a
     * scheduling/block-identity lever (order/selection), not a register-
     * identity fix: no operand constraint pins a register here. */
    if (t == 0) {
        goto case1;
    }
    if (t == 0x20) {
        goto case2;
    }
    goto case3;
case1:
    D_8006D8F0 = 0x200;
    __asm__("");
    goto join;
case2:
    D_8006D8F0 = 0x249;
    goto join;
case3:
    {
        volatile s32 *q3 = &D_8006D8F0;
        *q3 = 0x246;
    }
join:

    {
        volatile s32 *q4 = &D_8006D8E4;
        *q4 = arg0;
    }
    D_8006D8E0 = arg1;
    D_8006D8DC = 8;
    D_8006D8FC = CD_cbsync;
    D_8006D900 = CD_cbready;

    if (CD_status & 0xE0) {
        CD_cw(9, 0, 0, 0);
    }
    CD_sync(0, 0);
    return -(cd_read_retry() < 1);
}
```

## How this was derived -- the cross-jump/tail-merge residue class

This function is the clearest example this unit produced of a residue class
not yet named in `docs/MATCHING-GUIDE.md`: **GCC 2.6.3 -O2 will unify two (or
three) branches that end in an otherwise-identical store, when a plain
`if`/`else if`/`else` or `switch` is used to assign the same global from
different constants.** Retail's disassembly shows the opposite -- three fully
separate, unmerged store blocks, each with its OWN address computation (case1
and case2 fold the symbol's `%lo` into the store's immediate via `$at`; case3
computes the full address into a general register first, then stores at
offset 0). A naive `if/else if/else` or `switch` translation merges all three
into one shared `lui/sw` reached from every branch -- 4 to 44 bytes short of
retail depending on exactly which pair merges.

**What worked, found empirically (documented as a lever, not a guess to
repeat blind):**

- A bare `__asm__("")` scheduling barrier placed AFTER a branch's store (before
  its `goto`) can split THAT branch off from a merge with its "partner", but
  only when combined with the right choice of which OTHER branch also gets
  distinguished -- a barrier alone on only one side did not reliably prevent
  merging with a DIFFERENT partner.
- Introducing a local `volatile s32 *` pointer, assigned `&D_8006D8F0` (or
  `&D_8006D8E4`) and dereferenced for the store, forces GCC to compute the
  FULL address into a real register rather than folding `%lo` into the store's
  immediate -- this is what produces retail's case3 shape, and applying the
  SAME trick to a plain `D_8006D8E4 = arg0;` a few lines later closed the
  final one-instruction gap (retail's next statement also uses an unfolded
  address for reasons not fully understood -- possibly `$at` register
  pressure at that specific point).
- The three levers are NOT independent: which pair merges depends on ALL of
  them at once. The final combination (barrier only on case1, plain code for
  case2, local pointer only for case3) was found by testing each of the three
  branches' barrier/pointer status roughly in isolation and reading which PAIR
  merged after each change, then combining the settings that left no pair
  matching.

**Classification:** scheduling/block-identity, not register identity. No
`asm` operand constraint or `register T v asm("$N")` was used -- the `volatile
s32 *` locals are ordinary C that happens to force a specific (still fully
portable) codegen shape, and the bare `__asm__("")` is the explicitly-permitted
barrier form (verified: removing it only changes which blocks merge, not
which REGISTER a given value lives in).

### Proposed learning

**A "the compiler merges two branches assigning the same variable from
different constants" residue is real and not covered by the existing residue
list.** Symptoms: byte count short by a multiple of 4-8 relative to retail,
`funcdiff`'s per-word diff shows the SAME small run of instructions repeating
with different immediates further down retail's side than in the built side,
and `objdump`ing the built `.o` directly (not the linked, possibly-drifted
image) shows two branch targets landing on the exact same address. Fix
lever, in order of how surgical it is: (1) a bare `__asm__("")` right after
the assignment and before the branch out; (2) if that doesn't isolate the
right pair, a local `volatile T *` pointer, assigned the target's address and
dereferenced for the write, forces an unfolded (non-`$at`-relocation) address
computation that further distinguishes the block. Expect to try several
combinations across however many branches converge on the shared store --
which one needs which lever is not predictable in advance and was only found
by testing and reading which pair re-merged after each change.

## asm sites

Round 89 (runner delta, track 5 `asm-sites`): the bare `__asm__("")` after
case1's `D_8006D8F0 = 0x200;` store is **retired**. Measured by deleting it
alone and rebuilding: `build/src/libcd_bios.c.o` came out byte-identical to
the object built with it (`cmp`), and `./build-and-verify.sh` stayed green. So
in the current source no pair of the three stores re-merges without it; the
"barrier only on case1" part of the combination above is no longer load-bearing.
The in-source comment was updated to say so. `CD_readm` now carries no
`__asm__`.

## History (moved from src/libcd_bios.c, comments pass)

The comment above the three D_8006D8F0 stores read:

> Retail keeps all three D_8006D8F0 stores as separate, unmerged blocks
> (three distinct address computations -- two folded through $at, one
> unfolded through a real GPR) instead of the single shared store GCC's
> cross-jump/tail-merge pass produces from the equivalent if/else-if/else
> or switch. The match report records how the `goto` layout and the
> local `volatile s32 *` pointers below were found to keep the stores
> apart. A bare `__asm__("")` after case1's store, once part of that
> recipe, was retired in round 89: removing it left the object
> byte-identical.

## History (source comments moved in track 12, round 106)

From `src/psyq/libcd_bios.c`:

> "MATCHING: the goto layout and the local `volatile s32 *` pointers keep the
> three D_8006D8F0 stores as separate blocks, each with its own address
> computation; an if/else chain or a switch lets GCC merge them into one
> store." Now one line.
