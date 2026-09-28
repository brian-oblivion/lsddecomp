# BMemPMgrAlloc — MATCHED (round 73): length exact (114/114 words), 114/114 words match, no diff

> Renamed from `func_80017B34` on 2026-09-24 (tools/rename.py). Address 0x80017b34.

REVISITED, round 73: MATCHED 114/114 (whole-image SHA1 green), 6 builds from the preserved body, no permuter; names/types not relevant (no struct or header edit; locals renamed only)

## ROUND 73 (echo): revisit, MATCHED

**Preserved body rebuilt first, verbatim** (the round-46 101/114 body
below): `funcdiff.py` 101/114, `insertions 4 / deletions 4`, positional
skeleton diffs 13, zero out-of-range drift. Three clusters: `0x8370-0x837C`
(4), `0x83CC-0x83E0` (6), `0x8440-0x8448` (3).

Levers, one per line, with the funcdiff score each produced:

1. split-arm store before the remainder computation (store
   `cursor->sizeAndFlags` directly, then `remainder = cursor + (cursor->sizeAndFlags & mask)`,
   no `word` local) — 101/114, inert (kept: it is the shape the final match uses).
2. rounding with a named `rem = size & 3` local, `size = size + 4 - rem` — 101/114, inert.
3. `size`/`rem` declared `u32` — 101/114, inert.
4. **rounding through a temporary: `padded = size + 4; size = padded - (size & 3);`
   — 105/114, cluster 1 fixed.** One expression `size + 4 - (size & 3)`
   (in any parenthesisation, with or without a named `rem`) is reassociated
   at fold time into `size - ((size & 3) - 4)`, which is mine's
   `addiu v0,v0,-4; subu s0,s0,v0`. Splitting the `+ 4` into its own
   statement keeps retail's `addiu v0,s0,4` (reorg lifts it into the
   `beqz` delay slot) and `subu s0,v0,v1`.
   (`size += 4; size -= rem;` instead: 12/114, length change — rejected.)
5. move the sentinel-address computation back INSIDE the `blockSize < size+0x10`
   guard (undoing round 46's permuter hoist) — 96/114: the order is now
   retail's, but the address lands in `v1` and the `0x7FFFFFFF` constant in `a0`.
6. **same as 5, but the address held in `remainder` (the variable the split
   arm uses) instead of a separate `u32 *next` — 114/114, MATCH.**

Cause of the cluster-2 residue, checked by lever 6: `next` was used only
inside one basic block, so local-alloc gave it a register without
coordination, and it took `v1` from the dying word. When the same variable
is also assigned in the other arm, its pseudo spans blocks. Global
allocation then colours it first, `a0`, the same as the split arm's
`remainder`. That leaves `v1` free for the constant, which is retail's
allocation. Round 46's hoist got 4 words back only because it made `next`
live across the branch, which is another way of making it global, but it
put the computation in the wrong place.

Negative kept on record: the round-46 dead store `p = unused = cursor->prev;`
is still load-bearing in the matched body. Replacing it with a plain
`BMemBlockHdr *p = cursor->prev;` gives 109/114 (`0x8430` stores through
`v1` instead of `v0`). Function-scope `p`/`n` shared by all four unlink
blocks gives 106/114. The naive unscoped `cursor->prev->next = ...` form gives
48/114 with drift. The dead store stays: it is ugly but byte-verified.

### Proposed learning

- **One scratch pointer in two arms is ONE variable.** A pointer computed
  in only one arm of an if/else takes the wrong register (from a dying
  value, local-alloc). If retail gives it the same register as a
  structurally-parallel pointer in the other arm, the source used a single
  local for both. Assigning it in both arms makes it a global pseudo, and it
  is coloured before the arm's temporaries. Screen: retail uses the same
  `aN` for "the block after this one" in both arms of a split-or-consume.
- **`x + C - y` in one expression is reassociated by fold into `x - (y - C)`.**
  If retail computes `x + C` into its own register and subtracts afterwards
  (`addiu vA,x,C` ... `subu x,vA,vB`), the source had `t = x + C;` as a
  separate statement.

### Matched body (in `src/BMemPMgr.c`)

```c
void *BMemPMgrAlloc(size, pool)
    s32 size;
    void *pool;
{
    BMemPMgr *mgr;
    BMemBlockHdr *cursor;
    BMemBlockHdr *result;
    BMemBlockHdr *remainder;
    BMemBlockHdr *unused;
    u32 blockSize;
    u32 padded;
    ...
        if (size & 0x3) {
            padded = size + 4;
            size = padded - (size & 0x3);
        }
    ...
                if (blockSize < (u32)size + 0x10) {
                    remainder = (BMemBlockHdr *)((u8 *)cursor + (cursor->sizeAndFlags & 0xFFFFFFF));
                    remainder->sizeAndFlags &= 0x7FFFFFFF;
                    ... (unlink blocks unchanged from round 46) ...
                } else {
                    cursor->sizeAndFlags = (cursor->sizeAndFlags & 0xF0000000) | (u32)size;
                    remainder = (BMemBlockHdr *)((u8 *)cursor + (cursor->sizeAndFlags & 0xFFFFFFF));
                    ...
```

## Prior history (superseded title: STALL (round 46): 114/114 length, 101/114 words, first real diff at 0x80017B70)

## ROUND 46 (bravo): first permuter search on this function, +4 words (92 -> 96/114) from a real lead, oracle-verified

This function was flagged this round as "NEVER SEARCHED" despite being at a
high raw word-match. Ran all three of round-45's mandated permuter checks
before searching, all recorded:

1. **Correctness.** `tools/setup-permuter.sh BMemPMgrAlloc
   permuter-work/func_80017B34_seed.c` (seed = round 45's preserved 92/114
   body verbatim) built a scaffold cleanly: "base compiles, target
   assembled."
2. **Cost (`--debug --stack-diffs`).** Base score = **385** = 100
   (1 insertion) + 100 (1 deletion) + 1x60 (1 reordering) + 25x5 (register
   differences). NOT 0/0 — there is a real insertion/deletion pair, meaning
   the residue is not pure register identity and is in principle reachable
   by a source-mutation search, unlike round 45's `ServiceSoundCueSet`.
3. **Scaffold-vs-in-tree agreement (the new, load-bearing check).** Rebuilt
   the 92/114 seed body live in `src/BMemPMgr.c` first and confirmed
   `funcdiff.py` reproduces round 45's exact 92/114 figure and diff. Then
   objdumped the permuter's `base.o` and compared it instruction-by-
   instruction against `build/lsdde.elf`'s linked disassembly for this
   function: **identical opcodes and registers throughout, only branch/jal
   targets differ** (expected — the scaffold is unlinked, the in-tree copy
   is linked). No disagreement — safe to trust a scaffold-found lead.

**Search: blind, no PERM macros, `-j 6`, bounded `timeout 1500`.** Found one
candidate below the 385 floor: `output-250-1`, score 250, hoisting the
"clear the next block's high sentinel bit" address computation
(`*(u32*)(cursor + (cursor->sizeAndFlags & 0xFFFFFFF)) &= 0x7FFFFFFF;`) out
of the `if (blockSize < size+0x10)` guard into an unconditional `u32 *next =
...;` assignment computed right after `result = ...;`, then dereferencing
`*next &= 0x7FFFFFFF;` only inside the guard.

**Verified against the real oracle, not trusted on the permuter's own
number** (this project's own history has two prior instances of a permuter
-local improvement that was fake — `SubmitPolyF3`'s cached-OT-pointer lead
and `FlagLargePolyForDivide`'s rewritten-`end`-pointer lead, both in
`SubmitPolyF3.md`/`FlagLargePolyForDivide.md`). Translated verbatim into
`src/BMemPMgr.c`, ran `./build-and-verify.sh` + `funcdiff.py`: **REAL,
non-degenerate improvement — 96/114 words, still zero out-of-range drift**
(the file 0x83DC-0x83F4 cluster shrank and shifted to 0x83CC-0x83E0, net
4 fewer wrong words). This is genuine: hoisting the address computation
above the guard changes nothing about the VALUE, and GCC 2.6.3 apparently
prefers computing it there when the source no longer conditions the
*computation*, only the *use*.

**Two follow-up hand variants tried, both regressed back to 92/114 (not
adopted):**

- Introducing a named `u32 flags` local for `cursor->sizeAndFlags &
  0xBFFFFFFF` and referencing `flags` (not a fresh `cursor->sizeAndFlags`
  reload) from the hoisted `next` computation — reproduces the ORIGINAL
  92/114 diff exactly, byte for byte. The extra named local changes register
  pressure enough to undo the gain.
- Moving the `next = ...` computation back inside the `if`, as the first
  statement of the guarded block (instead of unconditionally before it) —
  also reproduces the original 92/114 diff exactly. The gain is specifically
  from computing it OUTSIDE the guard, unconditionally, not from anything
  about its position relative to the dereference.

**Not re-run further this round** — time budget went to `BMemPMgrFree`'s
own first-ever search instead (see that report). The remaining 18 wrong
words (four clusters: `0x8370-0x837C` mask-remainder rounding order,
`0x83CC-0x83E0` the same OT-sentinel address computation one placement
short of retail's own, `0x8414-0x8430` and `0x8440-0x8448`/`0x8490-0x84A4`
tested-vs-reused register preference in the per-if unlink hoists) are
candidates for a SECOND, deeper/longer search seeded from this round's
96/114 body — not attempted here for lack of remaining attempt budget this
round.

## ROUND 46 (bravo, second sitting): recovered an UNCOLLECTED permuter candidate, 96 -> 101/114

The first sitting's bounded search (see above) was still running when that
sitting ended its turn; it kept improving after the 250-score candidate
above was written up and produced a strictly better one that was never
collected: `permuter-work/BMemPMgrAlloc/output-220-1/` (permuter score
**220**, vs. the committed candidate's 250 — lower is better in permuter
units). Recovered from the untracked `permuter-work/` directory (which does
not survive this worktree) before doing anything else this sitting, per
this round's Gate 3 discipline: **a permuter score is a LEAD, not a proxy
for a funcdiff word count** (round 40/41 both measured this can go either
way), so it was translated and measured through the real oracle before
being trusted.

**What it does, on top of the already-committed 96/114 body:** the second
free-list-unlink block (inside the `blockSize < size+0x10` arm, the one that
patches `next->prev`) reads `cursor->prev` into `p` as before, but the
permuter additionally routes the same read through a second, otherwise-dead
local:

```c
BMemBlockHdr *n = cursor->next;
BMemBlockHdr *p;

p = unused = cursor->prev;
if (n != NULL) {
    n->prev = p;
} else {
    mgr->freeListStart = p;
}
```

(`unused` is declared once at function scope, alongside `next`, and is never
read again after this assignment.) This is a scheduling-only perturbation —
`unused` is dead-stored, not branched on, so it is not the UB class this
round's instructions call out for rejection (a value read before its first
assignment); it is a legitimate, if inelegant, way to nudge GCC 2.6.3's
allocator into a different register choice for `p` without touching program
behaviour.

**Translated and measured, both halves:**

1. Applied verbatim on top of the committed 96/114 body (adding the
   `BMemBlockHdr *unused;` declaration once at the top, and splitting the
   `p` declaration/read as shown) in `src/BMemPMgr.c`, swapped in for
   `INCLUDE_ASM` temporarily.
2. `./build-and-verify.sh` — clean compile (`build exit=2`, zero hits on the
   `error:`/`parse error`/`undefined reference`/`*** [….o]` grep — an
   ordinary "does not byte-match yet" state, not a build failure).
3. `tools/funcdiff.py BMemPMgrAlloc` — **101/114 words match, zero
   out-of-range drift** (no "differs OUTSIDE this range" warning). This is
   REAL: a genuine +5 words over the already-committed 96/114, on top of
   round 46's own already-oracle-verified gain over round 45's 92/114.
   First real diff is unchanged in location, `0x8370`/`0x80017B70`; the
   `0x83CC-0x83E0` cluster and one word of the `0x8440-0x8448` cluster both
   resolved.
4. Restored `INCLUDE_ASM`, re-ran `./build-and-verify.sh`: `OK: build
   matches retail SLPS_015.56` — worktree is back to a clean verified state
   with the improved body preserved in the `#if 0` block.

**Adopted as the new preserved best body** (shown in full below in place of
the round-46-first-sitting body, which it supersedes). `output-250-1` is
superseded and no longer the preferred candidate — `output-220-1` is
strictly better once measured, confirming permuter-score ordering (250 >
220) agreed with funcdiff ordering (96 < 101) in this instance, unlike the
two documented counter-examples cited above from round 40/41 history.

**Unit:** `BMemPMgr` · **Size:** 114 words, EXACT — the derivation below
compiles to the same length as retail, no address drift. **Status:** STALL
after ~22 attempts total (18 round-45 + 3 round-46-first-sitting + 1
round-46-second-sitting), restored to `INCLUDE_ASM`. Best raw score
**101/114 words**; `tools/asm-differ/diff.py BMemPMgrAlloc` (realigned,
confirms no drift) shows the first real difference at file offset `0x8370`
/ vram `0x80017B70`. Thirteen words remain wrong in three clusters:
`0x8370-0x837C` (4 words, mask-remainder rounding order), `0x83D4-0x83E0`
(3 words, one placement of the OT-sentinel address computation short of
retail's own), and `0x8440`/`0x8444` (2 words) plus `0x8490-0x84A4` (4
words, tested-vs-reused register preference in the per-if unlink hoists) —
candidates for a further, deeper/longer search seeded from this round's
101/114 body.

### Preserved body (current best, 101/114) — see `src/BMemPMgr.c`, still `#if 0`

```c
void *BMemPMgrAlloc(size, pool)
    s32 size;
    void *pool;
{
    BMemPMgr *mgr;
    BMemBlockHdr *cursor;
    BMemBlockHdr *result;
    BMemBlockHdr *remainder;
    u32 *next;
    BMemBlockHdr *unused;
    u32 blockSize;
    u32 word;

    SetBMemPMgrBusy(1);
    result = NULL;
    mgr = gDefaultBMemPMgr;
    if (mgr == NULL) {
        mgr = pool;
    }
    if (size != 0) {
        if (size & 0x3) {
            size = size + 4 - (size & 0x3);
        }
        if ((u32)size < 0xC) {
            size = 0xC;
        }
        cursor = mgr->freeListStart;
        size += 4;
        while (cursor != NULL) {
            blockSize = cursor->sizeAndFlags & 0xFFFFFFF;
            if (blockSize >= (u32)size) {
                cursor->sizeAndFlags &= 0xBFFFFFFF;
                result = (BMemBlockHdr *)((u8 *)cursor + 4);
                next = (u32 *)((u8 *)cursor + (cursor->sizeAndFlags & 0xFFFFFFF));
                if (blockSize < (u32)size + 0x10) {
                    *next &= 0x7FFFFFFF;
                    {
                        BMemBlockHdr *n = cursor->next;
                        BMemBlockHdr *p = cursor->prev;

                        if (p != NULL) {
                            p->next = n;
                        } else {
                            mgr->freeListEnd = n;
                        }
                    }
                    {
                        BMemBlockHdr *n = cursor->next;
                        BMemBlockHdr *p;

                        p = unused = cursor->prev;
                        if (n != NULL) {
                            n->prev = p;
                        } else {
                            mgr->freeListStart = p;
                        }
                    }
                } else {
                    word = (cursor->sizeAndFlags & 0xF0000000) | (u32)size;
                    remainder = (BMemBlockHdr *)((u8 *)cursor + (word & 0xFFFFFFF));
                    cursor->sizeAndFlags = word;
                    remainder->sizeAndFlags = (blockSize - (u32)size) | 0x40000000;
                    remainder->prev = cursor->prev;
                    remainder->next = cursor->next;
                    {
                        BMemBlockHdr *p = cursor->prev;

                        if (p != NULL) {
                            p->next = remainder;
                        } else {
                            mgr->freeListEnd = remainder;
                        }
                    }
                    {
                        BMemBlockHdr *n = cursor->next;

                        if (n != NULL) {
                            n->prev = remainder;
                        } else {
                            mgr->freeListStart = remainder;
                        }
                    }
                    *(BMemBlockHdr **)((u8 *)remainder + (remainder->sizeAndFlags & 0xFFFFFFF) - 4) = remainder;
                }
                break;
            }
            cursor = cursor->prev;
        }
    }
    SetBMemPMgrBusy(0);
    return result;
}
```

### Proposed learning

**A dead-store to an otherwise-unused local, assigned in parallel with a
live one (`p = unused = expr;`), is a legitimate, non-UB way to perturb
GCC 2.6.3 -O2's register allocation for the live half of the assignment.**
Found by the permuter, not by hand, on a same-length residue where the
*value* and *timing* were already right and only the *register* GCC chose
for `p` was wrong. Distinct from HARD RULE 6's banned directed-register
constructs (no `asm` register variable, no operand constraint) and from an
`__asm__` barrier (no inline asm at all) — it is ordinary C, and the
mechanism is presumably that materializing two live ranges out of one
right-hand-side value changes the allocator's spill/coalescing heuristics
enough to pick a different physical register for the one that survives.
Worth trying on other same-length register-identity-flavored residues
before reaching for a barrier or filing a stall, especially ones with a
short-lived local (like this unlink block's `p`) that another sibling block
in the same function computes identically but scores correctly.

## Verdict correction on the predecessor report

Filed round-before-round-12 as `gp_rel`-blocked, reopened round 42
(2026-09-15). This round attempted it in full for the first time. The prior
report's functional read (generic pool allocator, critical-section lock via
`SetBMemPMgrBusy`, free-list walk packing a size+flags header, split-or-consume
a candidate block, unlink from the free list, return a pointer past the
consumed block's header, `NULL` on failure) was **entirely correct** and is
reproduced below with the exact field layout now nailed down. What follows is
new: the actual C, the two structural levers that got it from "won't even
compile" to 118 words to exactly 114 words length-matched, and the four
remaining register-identity residues that stalled it short of byte-exact.

## Lever 1 (load-bearing): K&R (old-style) definition for the dead second parameter

The old report was right that `BMemPMgrAlloc`'s own body genuinely reads
`$a1` as a fallback pool pointer (live only when the global default pool
`gDefaultBMemPMgr` is unset), and right that every external caller passes only one
argument and must keep doing so. What it did not resolve is HOW a function
can have a real second parameter in its own body while every external
prototype — including a same-file call, `PushBasicClassListNode`'s
`BMemPMgrAlloc(0x8)` — stays single-argument. A single ANSI prototype cannot
do both: whichever parameter count the header declares, either the body
can't see the fallback or the 1-arg call sites stop compiling.

The fix is C89's old-style (K&R identifier-list) function definition, which
does NOT install a prototype for the rest of the translation unit:

```c
/* include/BMemPMgr.h -- unspecified-parameter declaration, deliberately,
 * so a same-file definition can use K&R syntax without conflicting with
 * the ~15 other headers' single-argument ANSI prototypes, or this same
 * file's own single-argument call sites that appear textually AFTER the
 * definition (PushBasicClassListNode's `BMemPMgrAlloc(0x8)`,
 * RemoveBasicClassListNode's `BMemPMgrFree(node)`). */
extern void *BMemPMgrAlloc();
extern void *BMemPMgrFree();
```

```c
/* src/BMemPMgr.c */
void *BMemPMgrAlloc(size, pool)
    s32 size;
    void *pool;
{
    ...
}
```

Confirmed this actually works end to end: `PushBasicClassListNode`'s existing
`BMemPMgrAlloc(0x8)` call, appearing later in the same file, kept compiling
with no arity warning once this was in place. This is the same
escape-hatch class as the project's already-documented "vtable slot
declared unprototyped" lever (`docs/DECOMPILATION_LEARNINGS.md`, round 14),
generalised from a function POINTER to a plain external function.

**`BMemPMgrFree` (this unit, next in ROM order, also in this round's queue)
needs the identical treatment** — its own disassembly reads `$a1` the same
way, for the same reason (see its own report, filed alongside this one).

## Lever 2: mutate `size` in place instead of a separate `need` local

The single biggest register-pressure fix. Retail's disassembly reuses `$s0`
(the `size` parameter's own register) for the "+4 header allowance" value —
`addiu $s0, $s0, 0x4` — rather than materialising a new register for it. My
first working draft declared a separate `u32 need = size + 4;`, which kept
`size` and `need` alive as two DIFFERENT callee-relevant values through the
whole search loop. That one extra live range cascaded into every later
register choice in the function (`a1`→`a2`, `a3`→`t0`, `t0`→`t1`, and so on,
one slot higher throughout, plus $s2 not getting saved/restored at all where
retail has it) and the function compiled to 118-119 words instead of 114.
Replacing the separate local with `size += 4;` (reusing the parameter's own
storage, matching retail's `addiu $s0,$s0,4`) fixed the WHOLE cascade in one
edit and got the register-name alignment matching everywhere except the four
clusters below — a single-lever, high-leverage fix.

**General form of the lesson, stated for reuse:** when your C's register
naming is shifted by a UNIFORM one-slot offset throughout a whole function
(not scattered), suspect one extra persistent local competing for a
callee-saved or argument register, and look for a place retail's disassembly
REUSES an argument's own register for what you gave a fresh name.

## Lever 3: hoist BOTH fields of a "used on every path" pair, PER if-statement, not across if-statements, not merged into ONE hoist point

The free-list unlink logic reads `cursor->prev` and `cursor->next` in two
separate `if (x != NULL) { x->fld = y; } else { mgr->something = y; }`
checks. Writing this the naive way — `cursor->prev`/`cursor->next` spelled
out fresh in each of the four arms — compiles CORRECTLY but costs extra
words: GCC 2.6.3 does not common-subexpression-eliminate a struct field read
across the condition and the body of the SAME arm, so each `if`/`else` pair
reloads its operand from memory a second time relative to retail, which
schedules ONE combined load of both fields (the tested one and the
also-needed one) immediately before each `if`. The fix that closed the gap
from 37/114 (with 5 words of drift) to 92/114 (zero drift) was a scoped block
per `if`:

```c
{
    BMemBlockHdr *n = cursor->next;
    BMemBlockHdr *p = cursor->prev;

    if (p != NULL) {
        p->next = n;
    } else {
        mgr->freeListEnd = n;
    }
}
```

**Fully hoisting BOTH checks to ONE shared pair of locals (read once, reused
by both `if`s) is a DIFFERENT and WRONG shape** — tried and measured: it
removes retail's second, genuinely-repeated pair of loads too, undershooting
to 105 words (9 short). Retail re-reads `cursor->prev`/`cursor->next` fresh
for the SECOND `if`, it just doesn't re-read within a single `if`'s own
condition-vs-body. Scope the hoist to exactly one `if` at a time.

## What stalled it: four clusters of pure register-identity swaps, zero length drift

With levers 1-3 in place the function is EXACTLY 114 words, matching
retail's length byte-for-byte in every region except four small clusters
where the SAME data ends up in `v0` where retail has `v1` (or the reverse) —
never a different VALUE, never a different instruction COUNT, purely which
physical temp register holds it. All four were probed with multiple
equivalent C phrasings (named locals vs. inline expressions, compound `&=`
vs. explicit load/mask/store, swapped declaration order, extra scoping
blocks) and **every rephrasing produced byte-identical output** — strong
evidence this is GCC 2.6.3's own scheduling/allocation choice for this exact
instruction shape, not something the C source visibly controls:

1. **`size = size + 4 - (size & 3)` rounds to a 4-byte multiple** (file
   `0x8370`-`0x837C`, 4 words). Retail: `andi v1,s0,3` / `addiu v0,s0,4`
   (independent of the mask) / `subu s0,v0,v1`. Mine: `andi v0,s0,3` /
   `addiu v0,v0,-4` / `subu s0,s0,v0` — algebraically identical result,
   computed via a different intermediate (retail keeps "size+4" and "mask"
   in two independent registers computed separately; mine derives one from
   the other). Tried: `size = (size - mask) + 4` with a named `mask` local,
   inline, both orders — no change in any of them.
2. **Clearing the boundary-sentinel's high bit**,
   `*(u32*)(cursor + offset) &= 0x7FFFFFFF` (file `0x83DC`-`0x83F4`, register
   swap only, no length change). Retail computes the ADDRESS first (`and`
   then `addu`) and only THEN materialises the `0x7fffffff` constant
   (`lui`/`ori`) immediately before the load. Mine consistently materialises
   the constant FIRST regardless of C statement order — tried a named `u8
   *next` local, a `BMemBlockHdr *` typed local, a fully inlined single
   expression, and an explicit load/mask/store split into four statements;
   all four produced the identical instruction sequence with the constant
   load scheduled early.
3. **The SECOND unlink check in the no-split path** (file `0x8414`-`0x8430`,
   register swap only). Same per-if hoist as lever 3 above, but here retail
   assigns the TESTED variable (`cursor->next`) to `v1` and the reused one
   (`cursor->prev`) to `v0` — the OPPOSITE mapping from the FIRST check
   (`0x83F8`-`0x8410`, which matches byte-exact with tested-variable-in-v1,
   reused-in-v0 the same way lever 3 describes). Tried every declaration
   order of the two locals in this second block; all gave the SAME (wrong)
   mapping, which rules out declaration order as the lever here specifically
   — whatever distinguishes retail's two blocks is not visible from this
   angle.
4. **The split path's second unlink check and the final `cursor->sizeAndFlags
   = word` store timing** (file `0x8440`-`0x8448` and `0x8490`-`0x84A4`,
   register swap only). Same shape as (3); tried scoping the store earlier
   vs. later relative to the address computation, no change.

All four residues are downstream of the SAME kind of choice (which of two
simultaneously-live values GCC assigns to `v0` vs `v1`) but do not share one
lever that fixes all of them — (1) and (2) look like scheduling around
independent-constant materialisation, (3) and (4) look like a preference
whose direction flips between two structurally-identical blocks for a reason
not evident from the C source.

## The struct layout (confirmed, shared with SetupBMemPMgrFreeList and BMemPMgrFree)

```c
typedef struct BMemBlockHdr {
    /* +0x000 */ u32 sizeAndFlags;   /* low 28 bits size, bit 0x40000000 = free */
    /* +0x004 */ BMemBlockHdr *prev;
    /* +0x008 */ BMemBlockHdr *next;
} BMemBlockHdr;

struct BMemPMgr {
    /* +0x000 */ void *freeListHead;
    /* +0x004 */ s32 poolSize;
    /* +0x008 */ BMemBlockHdr *freeListStart;
    /* +0x00C */ BMemBlockHdr *freeListEnd;
    /* +0x010 */ s32 unk10;
};
```

## Preserved near-miss body (96/114, length-exact)

```c
#if 0
void *BMemPMgrAlloc(size, pool)
    s32 size;
    void *pool;
{
    BMemPMgr *mgr;
    BMemBlockHdr *cursor;
    BMemBlockHdr *result;
    BMemBlockHdr *remainder;
    u32 *next;
    u32 blockSize;
    u32 word;

    SetBMemPMgrBusy(1);
    result = NULL;
    mgr = gDefaultBMemPMgr;
    if (mgr == NULL) {
        mgr = pool;
    }
    if (size != 0) {
        if (size & 0x3) {
            size = size + 4 - (size & 0x3);
        }
        if ((u32)size < 0xC) {
            size = 0xC;
        }
        cursor = mgr->freeListStart;
        size += 4;
        while (cursor != NULL) {
            blockSize = cursor->sizeAndFlags & 0xFFFFFFF;
            if (blockSize >= (u32)size) {
                cursor->sizeAndFlags &= 0xBFFFFFFF;
                result = (BMemBlockHdr *)((u8 *)cursor + 4);
                next = (u32 *)((u8 *)cursor + (cursor->sizeAndFlags & 0xFFFFFFF));
                if (blockSize < (u32)size + 0x10) {
                    *next &= 0x7FFFFFFF;
                    {
                        BMemBlockHdr *n = cursor->next;
                        BMemBlockHdr *p = cursor->prev;

                        if (p != NULL) {
                            p->next = n;
                        } else {
                            mgr->freeListEnd = n;
                        }
                    }
                    {
                        BMemBlockHdr *n = cursor->next;
                        BMemBlockHdr *p = cursor->prev;

                        if (n != NULL) {
                            n->prev = p;
                        } else {
                            mgr->freeListStart = p;
                        }
                    }
                } else {
                    word = (cursor->sizeAndFlags & 0xF0000000) | (u32)size;
                    remainder = (BMemBlockHdr *)((u8 *)cursor + (word & 0xFFFFFFF));
                    cursor->sizeAndFlags = word;
                    remainder->sizeAndFlags = (blockSize - (u32)size) | 0x40000000;
                    remainder->prev = cursor->prev;
                    remainder->next = cursor->next;
                    {
                        BMemBlockHdr *p = cursor->prev;

                        if (p != NULL) {
                            p->next = remainder;
                        } else {
                            mgr->freeListEnd = remainder;
                        }
                    }
                    {
                        BMemBlockHdr *n = cursor->next;

                        if (n != NULL) {
                            n->prev = remainder;
                        } else {
                            mgr->freeListStart = remainder;
                        }
                    }
                    *(BMemBlockHdr **)((u8 *)remainder + (remainder->sizeAndFlags & 0xFFFFFFF) - 4) = remainder;
                }
                break;
            }
            cursor = cursor->prev;
        }
    }
    SetBMemPMgrBusy(0);
    return result;
}
#endif
```

## Next steps for whoever reopens this

- Levers 1-3 above are almost certainly reusable verbatim on `BMemPMgrFree`
  (same unit, same globals, same free-list data structure, same K&R
  parameter shape, same free-list-node reload pattern — it is the `free()`
  counterpart of this `malloc()`).
- **Round 46 update:** the permuter search DID find a real, oracle-verified
  lever for cluster 2 (the "clear next block's sentinel bit" address
  computation) — hoist it unconditionally above the guard that uses it. See
  the round-46 section at the top. The other three clusters (mask-remainder
  rounding order at `0x8370`, and the two tested-vs-reused register
  preferences at `0x8414-0x8430`/`0x8440-0x84A4`) are still open and are
  good candidates for a longer/re-seeded search from this round's 96/114
  body (round 46's search was blind, single 1500s pass, and never revisited
  those three clusters with the cluster-2 fix already in place).
- Do not re-attempt the `gp_rel` framing — irrelevant, confirmed resolved,
  zero `%gp_rel` hits remaining as an obstacle here.

### Proposed learning

- **A uniform one-register-slot shift throughout an otherwise-correct
  function means one extra persistent local, not scattered small
  mistakes** — find the ONE place retail reuses an argument's own register
  for a derived value (`size += 4` vs a fresh `need` local) rather than
  auditing every register individually.
- **Hoisting a load pair to avoid a duplicate-reload penalty must be scoped
  to exactly the `if` that needs it.** Hoisting too far (sharing one pair
  across two sequential `if`s that each independently re-read in retail)
  undershoots the word count exactly as reliably as not hoisting at all
  overshoots it.
- **Not every register-identity residue has a source-level lever found BY
  HAND — round 46 correction: one of these four DID have one, found by the
  permuter.** Cluster 2 (the constant-materialisation-timing one, item 2
  above) moved from "byte-identical across four hand rephrasings" to a real
  +4-word gain once a BLIND permuter search tried "hoist the computation
  unconditionally above the guard" — a fifth axis none of the four hand
  attempts had tried (they varied the EXPRESSION computing the address, not
  WHETHER its evaluation was conditioned on the same guard that uses it).
  The other three clusters are still genuinely unmoved. Lesson: "four
  rephrasings failed" bounds the rephrasings tried, not the residue's
  reachability — a permuter search is still worth one bounded pass even
  after exhausting the obvious hand axes, and the search should be re-run
  in this project's future long-search when there is a real insertion/
  deletion in the `--debug` cost check.

## Extern arity (round 59)

**Verdict: arity-ok idiom**, for the whole family. Nothing converged; the
odd-one-out declaration is the load-bearing one.

**Callee evidence.** Both bodies genuinely read a SECOND argument register
before writing it:

```
80017b34 <BMemPMgrAlloc>:        80017cfc <BMemPMgrFree>:
80017b3c:  move  s0,a0           80017d04:  move  s0,a0
80017b40:  move  s1,a1   <-      80017d0c:  move  s1,a1   <-
...                              ...
80017b68:  move  t0,s1   <-      80017d2c:  move  t0,s1   <-
```

`$a1` is a fallback pool pointer, consumed only on the path where the
`$gp`-relative default pool `gDefaultBMemPMgr` is unset (`lw t0,16(gp)` /
`bnez t0,...` immediately before). It is dead in practice at every decoded call
site, because `SetupBMemPMgrFreeList`/`SetDefaultBMemPMgr` set that global first — but it is
read, so these are two-argument functions.

**Why the 22 one-parameter declarations are right anyway.** Every carved call
site passes one argument, and retail emits only `$a0` for it — e.g.
`FileResource__LoadFile`'s call:

```
80026b70:  jal   80017b34 <BMemPMgrAlloc>
80026b74:  move  a0,s2            <- $a0 only; $a1 is left as the caller had it
```

This is the dead-argument idiom in its ordinary direction: the callee reads a
register the caller happens to leave loaded.

**Why `include/BMemPMgr.h`'s unprototyped pair must stay unprototyped.** Round
45 established this and it re-measures correct. Both functions are DEFINED in
`src/BMemPMgr.c` with old-style (K&R identifier-list) parameter lists, which is
the only way to expose the second parameter to their own bodies without
contradicting either the ~15 external single-argument prototypes or this same
unit's own later one-argument call sites (`PushBasicClassListNode`'s
`BMemPMgrAlloc(0x8)`, `RemoveBasicClassListNode`'s `BMemPMgrFree(node)`). A K&R definition
installs no prototype, so those later calls stay clean; an unspecified-parameter
declaration does the same for everything before the definition. A full prototype
here reintroduces exactly that conflict.

So the family is not a family with one wrong member: 22 declarations describe
their own call sites correctly, and one describes the definition's translation
unit correctly. Converging them would break one side or the other.

**Declaration sites changed:** none (no arity anywhere changed).
`/* arity-ok: ... */` added to the two lines in `include/BMemPMgr.h`. Oracle
green.

## Naming (round 74)

`BMemPMgrAlloc`, **tier A**: fully derived byte-exact free-list allocator
(round 73), matching `SetupBMemPMgrFreeList`'s setup and `BMemPMgrFree`'s
(this unit's `BMemPMgrFree`) release, same list, same `sizeAndFlags`
encoding. Called from a wide cross-section of units (`GameApplicationFileResource`,
`code_179d8_*`, `Task`, `code_2cc8c_*`, `TodActor`, `code_d294`,
`main`, plus this unit's own `PushBasicClassListNode`) — confirming it is
the game's general small-object pool allocator, not something narrower.

## Polish (round 97, runner delta)

Zero bytes changed; whole-image SHA1 green after each step.

- Pool parameter typed `BMemPMgr *`, `result` typed `void *`; `remainder` ->
  `nextBlock`, since it is the block after `cursor` in both arms (the
  split-off remainder in one, the neighbour whose `BMEM_PREV_FREE` is cleared
  in the other). The single-variable requirement from round 73 is kept as a
  `/* MATCHING: */` line, as are the `padded` temporary, the per-unlink
  scoped `n`/`p` and the `unused` dead store.
- Literals: 0xC -> `BMEM_MIN_PAYLOAD` (12), 4 -> `BMEM_HEADER_SIZE`,
  0x10 -> `BMEM_MIN_BLOCK` (16: the smallest block that can be free, header
  word plus two links plus footer), 0xFFFFFFF -> `BMEM_BLOCK_SIZE()`,
  0xBFFFFFFF -> `~BMEM_FREE`, 0x7FFFFFFF -> `~BMEM_PREV_FREE`,
  0xF0000000 -> `BMEM_FLAG_MASK`, 0x40000000 -> `BMEM_FREE`. The payload,
  next-block and footer arithmetic go through `BMEM_PAYLOAD`,
  `BMEM_NEXT_BLOCK`, `BMEM_FOOTER`.
- Fields (struct `BMemPMgr`): `freeListStart` -> `freeListTail`,
  `freeListEnd` -> `freeListHead`. Evidence: the unlink code sets +0xC when
  the node has no `prev` and +0x8 when it has no `next`, and BMemPMgrFree
  appends at +0x8, so +0xC is the head and +0x8 the tail. This function
  searches from the tail backward through `prev`.

The constants are defined, with their evidence, in `include/BMemPMgr.h`:
`BMEM_SIZE_MASK` 0x0FFFFFFF and `BMEM_FLAG_MASK` 0xF0000000 split
`sizeAndFlags`; `BMEM_FREE` 0x40000000 is set on every block put on the free
list (SetupBMemPMgrFreeList, BMemPMgrFree, the split remainder) and cleared
when BMemPMgrAlloc takes one; `BMEM_PREV_FREE` 0x80000000 is set on the block
above a freed one and on the sentinel, cleared on the block above an allocated
one, and tested by BMemPMgrFree before it reads the lower neighbour's footer.
