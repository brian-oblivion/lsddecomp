> Renamed from `func_800400B0` on 2026-09-20 (tools/rename.py). Address 0x800400b0.

# Class6E99C__StartFadeDefault -- STALL, PRIOR "29/41, same length" CLAIM CORRECTED (real one-word length regression, not a pure register/scheduling residue)

## Round 46 (runner delta): drift-checked fresh, no new attempt -- DELIBERATE SKIP

Re-spliced the exact preserved body (unchanged) in isolation and
rebuilt: **20/41 words match, `WARNING: differs OUTSIDE this range too
(224445 bytes)`** -- reproduces the round-21 CORRECTED figure exactly
(40/41 compiled words, one real missing instruction), confirming the
correction still holds and nothing has drifted since. Deliberate skip:
residue 1 (the shared `slotB8(self,1,tableEntry)` scheduling class) is
the same confirmed-negative mechanism as `Class6E99C__StartFadeToIndex`'s own
~94,000-iteration search; residue 2 (the register-only dead `move
$t0,$a2`) already has four negative attempts plus a 280-second bounded
permuter search (base score 440, no improvement) and a reasoned
explanation for why it may not be reachable from source at all (a
delay-slot filler invented by reorg from live-but-unread register state,
not a statement reorg can hoist). No new lever occurred to me for either
residue. Restored to `INCLUDE_ASM`; full oracle re-confirmed green.

## Round 21 (runner delta): drift found -- the inherited 29/41 figure is WRONG

**This is the "roughly one inherited body in six carries a false
clean/drift-free claim" case this round's assignment warned about.**
Restored the exact preserved body from round 20 verbatim (unchanged, see
below) and rebuilt in ISOLATION (`Class6E99C__StartFadeToIndex` reverted to
`INCLUDE_ASM` first, to rule out any cross-contamination from that
sibling): `tools/funcdiff.py` reports `20/41 words match` with a
**`WARNING: the build differs OUTSIDE this range too (224685 bytes)`**
-- genuine drift, not the "same total instruction count, same
registers, purely reordered" every prior round (18/19/20) claimed for
this residue. `build/lsdde.map` confirms it directly: `func_80040154`
(the very next function in ROM order) links at `0x80040150` in this
build, one word short of its retail address `0x80040154`. **This
function's own compiled body is 40 words, not the reported 41** --
genuinely one instruction shorter than retail, not merely
differently-registered.

Re-ran with the coordinator's corrected oracle grep this round
(`error:|parse error|undefined reference|\*\*\* \[[^]]*\.o\]`): zero
hits both times (isolated and combined with `Class6E99C__StartFadeToIndex`), so this
is not a masked compile error either -- the build genuinely, cleanly
compiles to a shorter function than retail.

### The real mechanism (found via `mipsel-linux-gnu-objdump` on the built object, cross-checked against retail's own `.s`)

Retail's `Class6E99C__StartFadeDefault.s` contains **two** separate
`addu $t0, $reg, $zero` copies, each sitting in a branch's delay slot
(both execute unconditionally, MIPS delay-slot semantics):

1. `addu $t0, $a2, $zero` -- delay slot of the top guard's `bnez`.
   Genuinely dead: `$t0` is overwritten by copy 2 before any read, on
   every path. This is the "residue 2" this report's prior rounds
   already investigated and correctly established resists the
   `volatile`-local and extended-asm tricks (both wrong-direction: they
   push a register-only dead value toward memory).
2. `addu $t0, $v0, $zero` -- delay slot of the SECOND branch
   (`beqz $v0/$v1,...` testing `self->unk98`), copying the just-computed
   `idx` (still live in `$v0` from the `slotDC` dispatch) into `$t0`.
   **This one is NOT dead** -- on the branch-taken path it is read by
   the table-index computation (`sll $a2,$t0,1` / `addu $a2,$a2,$t0`),
   i.e. `idx * 3`. Every C shape tried (this report's own preserved
   body, and this round's `t = idx;`/branch-forced-copy/`__asm__`
   barrier/named-`entry`-local attempts, below) computes `idx * 3`
   directly from `$v0` with NO intervening copy, and instead schedules
   `move $a0,$s0` (setting up the `slotB8` call's first argument) into
   that SAME delay slot -- a different, but equally "free," candidate
   filler. Retail's compiler chose copy 2 for the slot and left
   `move a0,s0` at the block's own first instruction; this project's
   pinned GCC 2.6.3 chooses the reverse, and the two choices differ by
   exactly one instruction (copy 2 costs a word retail "spends" that
   this compiler doesn't).

This is the SAME underlying "GCC schedules a delay slot differently
than retail, and retail's choice happens to look like a redundant
register copy" class as `func_8004042C`'s and `func_8003FCFC`'s
already-documented coalescing residues in this unit -- but manifesting
as a missing WORD (length regression) rather than a same-length
register swap, which is why the drift guard fires here and not there.

### Attempts this round targeting the missing delay-slot filler (4, all negative)

1. Named local `t = idx;` right after the `slotDC` dispatch, used in
   place of `idx` in the else-branch's `idx * 3`: no change, still 40
   words (`func_80040154` still links at `0x80040150`).
2. Branch-forced-copy trick (`if (self->unk98) { t = idx; } else { t =
   idx; }`, the `func_80051858`-precedent idiom used elsewhere in this
   project): no change.
3. Bare `__asm__("" ::: "memory")` immediately after the `slotDC`
   dispatch, attempting to block the `move a0,s0` hoist so the
   scheduler would need a different filler: no change -- confirmed via
   direct disassembly, byte-identical to the baseline.
4. Named local `entry` for the table-address computation, declared at
   the top of the else-block and used in the `slotB8` call
   (`void *entry = &D_8006EAA8[idx * 3]; ...slotB8(self, 1, entry);`):
   no change.

None of the four moved the delay-slot filler choice at all -- confirmed
via `tools/decomp-permuter/permuter.py --debug --stack-diffs` on the
unmodified preserved body, which reports base score **440** (0
stack/branch differences, 4 register differences, 2 reorderings, 1
insertion, 2 deletions -- a real insertion/deletion signature, not the
pure-register-difference signature a same-length residue would show).
A bounded unguided search (`-j 6 --stack-diffs --stop-on-zero
--best-only`, `timeout 280`) ran the full bound with no candidate
beating 440.

### Verdict correction

**The prior "29/41, same total instruction count, purely reordered"
verdict is WRONG for this function's real state and is corrected here.**
The true state is: **40/41 compiled words (one real missing
instruction), residue 1 (the shared `li a1,1` scheduling class with
`Class6E99C__StartFadeToIndex`) unconfirmed on its own since the length mismatch makes
`funcdiff`'s per-word window untrustworthy past the divergence point**
-- residue 1's own bytes may well still hold once residue 2's length is
fixed, but that cannot be verified against a drifted window, only
against `build/lsdde.map`-confirmed correct addresses. Restored to
`INCLUDE_ASM` (best preserved body, unchanged, below); full oracle
re-confirmed green after restoration.

### Proposed learning (round 21)

**A "same total instruction count, purely reordered" claim needs a
`build/lsdde.map` address check on the NEXT function in ROM order, not
just a funcdiff word-count read, before it can be trusted** -- this
report's own word count (29/41) was internally consistent and looked
exactly like a plausible partial match for three rounds, but the
`WARNING: outside this range` line (and the map's own 4-byte address
disagreement) was sitting there the whole time, unread. This is a
sharper, function-local instance of the project's own "check
`build/lsdde.map`... before reading anything else" drift-attribution
guidance (CLAUDE.md) -- it applies to a function's OWN claimed
same-length residue, not only to blaming a sibling's drift onto the
wrong function.

## Round 20 (runner delta): not re-attempted, downgraded shared axis

Did not spend a fresh attempt on this function directly this round.
`Class6E99C__StartFadeToIndex`'s own residue 1 is the SAME `slotB8(self, 1,
tableEntry)`-after-a-fresh-dispatch shape as this function's residue 1
(both reports already cross-reference each other on this), and that
shared axis was permuter-searched to ~94,000 unguided iterations this
round with zero improvement (see `Class6E99C__StartFadeToIndex.md`). Since residue 1
here is the identical mechanism, treating it as re-confirmed negative by
transfer rather than re-spending a second ~94,000-iteration budget on a
byte-identical scheduling shape.

**Residue 2 (the register-only dead store) reasoned about, not
attempted with a real search.** The existing report's two attempts
(volatile-local, banned extended-asm) already established the value is
register-only, never spilled. Working through the mechanism further: the
dead `move $t0,$a2` sits in the delay slot of the FUNCTION'S OWN TOP
GUARD branch (`bnez $v0,...`, testing `self->unk6C != 0`) -- i.e. it is
a genuine **delay-slot filler pulled from nowhere the source can name**,
not a scheduled-forward real statement. The existing report's own
attempt 1 already showed that writing `idx = a2;` explicitly (before the
dispatch overwrites it) produces **zero instructions** -- GCC eliminates
it as dead code before the scheduling/reorg pass ever runs, so there is
no live C statement left for reorg to hoist INTO the delay slot. That
means retail's filler is not "a statement moved early" the way the
project's documented "prologue callee-save stores in the wrong order"
class is; it looks instead like GCC's reorg pass inventing a filler
instruction from whatever register happens to be both live and
immediately available (`$a2`, the incoming 3rd argument, sitting unused
until the dispatch's return value overwrites the same destination a few
instructions later) with NO C-level statement driving it at all. If that
reading is right, this residue is not reachable from source the way a
real dead-store class is -- there is no statement to place, rename, or
block-scope, because the compiler is filling the slot from its own
internal accounting rather than from anything in the AST. Flagging this
as the likely reason the two already-tried source-level levers (both of
which assume a REAL statement exists to manipulate) failed, rather than
recommending a third variation on the same assumption. Not verified
against GCC 2.6.3 internals directly (no `-dr`/RTL dump support in this
project's pinned `cc1`, checked: no such flag in `tools/gcc263/cc1
--help`) -- this is inference from the instruction pattern and the
existing negative attempts, not a confirmed root cause. A permuter
search seeded from the 29/41 body, specifically targeting this one
instruction (not the shared residue-1 axis), remains the concrete
untried lever and the most likely way to either close it or confirm it
compiler-internal beyond reasoning.

Unit `code_2cc8c_e`, carved round 14. `Class6E99CMethods::slotD8` (`+0x0D8`).

**Correction to an earlier version of this report**, which claimed a full
41/41 match under the stale-build window described in `New_Class6E99C.md`
(same cause). Re-verified genuinely stale-free, this function has two
distinct residues.

## Best body reached (29/41)

```c
#if 0
void Class6E99C__StartFadeDefault(Class6E99CObj *self, s32 a1, s32 a2) {
    s32 idx;

    if (self->unk6C != 0) {
        return;
    }
    idx = self->methods->slotDC(self);
    if (self->unk98 != 0) {
        self->unk80--;
    } else {
        self->methods->slotB8(self, 1, &D_8006EAA8[idx * 3]);
    }
    self->unk6C = 2;
}
#endif
```

## Residue 1 -- same `li a1,1` scheduling class as `Class6E99C__StartFadeToIndex`

Identical to that function's own residue: retail materialises the `1`
literal right after the `slotDC` dispatch, before computing `idx*3`; this
body defers it to just before the `slotB8` call. See that report for the
attempts already spent on this exact shape (not repeated here since the
mechanism is the same call, `slotB8(self, 1, tableEntry)`).

## Residue 2 -- a genuinely MISSING dead store

Retail's own 3rd parameter (`a2`, this function's own incoming argument) is
copied into a scratch register (`move $t0, $a2`) immediately after the top
guard's branch, and is IMMEDIATELY overwritten by the `slotDC` dispatch's
own return value one instruction later -- a provably dead value with no
later use anywhere in the function. GCC 2.6.3 at `-O2` eliminates a plain
`idx = a2;` statement written before `idx = self->methods->slotDC(self);`
as dead code (confirmed: adding it produced no instruction at all).

### Attempts to force the dead store (2)

1. `volatile s32 unused; unused = a2;` -- forces the store to survive, but
   ALSO forces `unused` onto the stack (a real memory location, since
   `volatile` implies it cannot live purely in a register the way retail's
   register-only `move $t0,$a2` does) -- grew the frame by 8 bytes and
   changed the prologue/epilogue shape entirely. Wrong mechanism: retail's
   dead value lives in a REGISTER, not a stack slot.
2. An extended-asm operand constraint (`__asm__("" : "+r"(idx))`) to force
   the register write without a memory side effect: **not used** -- this is
   exactly the extended-asm-operand-constraint construct CLAUDE.md HARD
   RULE 6 bans outright ("Never fix a register mismatch with... an
   extended-asm operand constraint"), regardless of whether the target
   register would come out matching retail's `$t0` or not. Written once,
   recognised as banned, and reverted before building -- recorded here so
   the next attempt does not re-derive and then use it.

## What I did NOT try, and why

- **The established "address-taken local" idiom**
  (`docs/DECOMPILATION_LEARNINGS.md`: *"GCC 2.6.3 at -O2 does not eliminate
  a dead store into an address-taken local"*, `func_8001D204`). Considered
  and rejected on reasoning rather than tried: that idiom's own precedent
  is for a value that ends up in a real MEMORY location either way: taking
  a local's address forces it out of pure-register treatment, which is the
  opposite of what's needed here (retail's dead value is register-only,
  never spilled). Flagging the rejection explicitly per this round's
  "record the lever you didn't pull and why" convention, in case the
  reasoning is wrong -- it would cost one attempt to check directly.
- **The permuter**, for the same reasons given in `Class6E99C__StartFadeToIndex`'s report
  -- time budget, and this looks like the same underlying scheduling
  mechanism as that function's residue 1, so a `PERM_VAR`-guided search
  seeded from either function's near-miss body might close both at once.

## Proposed learning

A dead store whose value visibly lives in a scratch register in retail
(not a stack slot) does not respond to the `volatile`-local or
address-taken-local tricks, both of which push the value toward memory
instead of preserving its register-only, still-eliminable nature. This is
a narrower and harder case than the already-documented "dead store into an
address-taken local survives" class -- worth a distinct entry rather than
assuming the existing lever generalises.

## Round 59 (runner charlie): NON_MATCHING body promoted

NON_MATCHING body promoted, round 59. The exact preserved body above (40/41
compiled words, 1 word short, per the round-21/46 corrected reading -- the
raw funcdiff word-match count is not trustworthy past the length divergence)
is now live in `src/code_2cc8c_e.c` under `#ifdef NON_MATCHING`, with the
verified build still taking the `#else INCLUDE_ASM` branch.
`./build-and-verify.sh` and `tools/check-nonmatching.sh` both green.
