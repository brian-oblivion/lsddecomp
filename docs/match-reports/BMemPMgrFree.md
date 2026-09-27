# BMemPMgrFree — MATCHED (round 46): 107/107 words, byte-exact

> Renamed from `func_80017CFC` on 2026-09-24 (tools/rename.py). Address 0x80017cfc.

## ROUND 46 (bravo, second sitting): first-ever permuter search, MATCHED 99 -> 107/107

Picked up per this round's assignment as "never permuter-searched" despite
99/107 raw word-match and ~10 prior hand attempts (round 45, see below). Ran
all three of this round's mandated Gate-3 checks before searching, all
recorded:

1. **Correctness.** `tools/setup-permuter.sh BMemPMgrFree
   permuter-work/func_80017CFC_seed.c` (seed checked byte-for-byte identical
   to the current in-tree preserved body first) built a scaffold cleanly:
   "base compiles, target assembled."
2. **Cost (`--debug --stack-diffs`).** Base score = **495** = 0 (stack) + 0
   (branch) + 7×5=35 (register differences) + 1×60 (reordering) + 2×100=200
   (insertions) + 2×100=200 (deletions). NOT 0/0 — real insertions/deletions,
   in principle reachable by source mutation.
3. **Scaffold-vs-in-tree agreement.** Rebuilt the 99/107 seed body live in
   `src/BMemPMgr.c`, confirmed `funcdiff.py` reproduces the round-45 99/107
   figure exactly with zero out-of-range drift. Then objdumped the
   permuter's `base.o` and diffed it mnemonic-by-mnemonic against
   `build/lsdde.elf`'s linked disassembly for this function: **identical
   opcodes and registers throughout** — every one of the 20 diff lines was
   either a branch/jump target or the `lw t0,N(gp)` immediate (both expected:
   unlinked scaffold vs. linked build). No disagreement — safe to search.

**Search: blind, no PERM macros, `-j 6`, bounded `timeout 1500` (backgrounded
while `BMemPMgrAlloc`'s recovered-candidate work above was written up and
committed).** Found a **zero score at iteration 373** —
`permuter-work/BMemPMgrFree/output-0-1/`.

**The permuter's own diff carried two changes; only one was legitimate, and
they were tested separately before either was trusted:**

1. Retyping the shared `extern void SetBMemPMgrBusy(s32 val);` declaration in
   this file's copy to `extern volatile unsigned long long SetBMemPMgrBusy(s32
   val);` — a fabricated, incompatible prototype against the function's real
   definition (`void SetBMemPMgrBusy(s32 val)` in `src/TmdRenderer.c`). This
   is exactly the class of scorer exploitation this round's instructions say
   to reject, so it was **not adopted and not even needed** — see below.
2. In the second (coalesce-with-next) free-list unlink block, replacing
   `if (n != NULL)` with reusing the already-dead `nextSize` local (its one
   real use, the `sizeAndFlags` fold two statements earlier, had already
   happened) to hold the comparison's boolean result: `nextSize = n !=
   NULL; if (nextSize) { ... }`. This is ordinary, well-defined C — no UB,
   no branch on an unassigned value, just a dead local repurposed as the
   condition's temporary instead of an anonymous one.

**Translated and measured, both halves, separately:**

- Applied ONLY change 2 (the `nextSize` reuse) on top of the committed
  99/107 body, leaving `SetBMemPMgrBusy`'s real single declaration untouched.
  `./build-and-verify.sh`: **`build exit=0`, `OK: build matches retail
  SLPS_015.56`.** `tools/funcdiff.py BMemPMgrFree`: **107/107 words,
  byte-exact.** The fabricated-prototype mutation was pure permuter noise
  that happened to ride along in the diff without being load-bearing — the
  real fix is entirely the dead-variable reuse.
- (Change 1 was therefore never applied to the tree at all — recorded here
  only because it appeared in the permuter's raw output and is worth naming
  as the "reject this" example the Gate-3 discipline calls for: a return-type
  fabrication against a real, differently-typed definition elsewhere in the
  project is not a legitimate translation, whatever the scorer says.)

**Adopted. `BMemPMgrFree` is MATCHED, 107/107 words, byte-exact against
retail.** A short comment was added at the `nextSize` reuse site explaining
why the dead variable is repurposed, since the idiom is unusual on its own
and was found by search rather than derivation. Whole-image oracle re-run
after the comment addition: still `OK: build matches retail`.

**Unit:** `BMemPMgr` — with this function's match, `BMemPMgr` has exactly
one remaining `INCLUDE_ASM` (`BMemPMgrAlloc`, see its own report).

### Proposed learning

**A local variable that is provably dead by a given point in its own scope
(all real uses already executed) can be legally repurposed as the holder of
an unrelated boolean/temporary a few lines later, and doing so can be
exactly what nudges GCC 2.6.3 -O2 into retail's register choice for that
temporary.** This is the read/no-branch-on-garbage-safe cousin of this
round's `BMemPMgrAlloc` finding (`p = unused = expr;`, a local declared
*only* to receive a dead store): here the local already existed for a real
purpose, and the trick is reusing it a second time after its first job is
done, rather than declaring a fresh anonymous one. Both are ordinary C, both
are outside HARD RULE 6 (no `asm` anywhere), and both were found by the
permuter rather than by hand — worth trying by hand on similar
same-length residues where a hand-written body already got the value and
timing right and only the register is wrong.

## Round 45 investigation (preserved below for the derivation this round built on)



**Unit:** `BMemPMgr` · **Size:** 107 words, EXACT. **Status:** STALL after
~10 attempts, restored to `INCLUDE_ASM`. Best raw score 99/107 words (92.5%);
`tools/asm-differ/diff.py BMemPMgrFree` (realigned) confirms zero drift and
the first real difference at file offset `0x85BC` / vram `0x80017DBC`.

## Verdict correction on the predecessor report

Filed as `gp_rel`-blocked, reopened round 42. This round attempted it for the
first time. The prior report's functional read (pool `free`, mirror of
`BMemPMgrAlloc`, same critical-section lock, coalesce-with-adjacent-free-block
logic, same packed size+flags header) was correct and is confirmed exactly by
the derivation below — including the ONE detail it left open ("depending on
the header's high bit... either coalesces... or inserts... as a new
standalone entry", which undersells it: it can do BOTH coalesces, with the
previous block and the next block, not an either/or).

## The three BMemPMgrAlloc levers all transferred directly

This function needed exactly the same three structural levers as its sibling
`BMemPMgrAlloc` (`docs/match-reports/BMemPMgrAlloc.md`), confirming they are
reusable project-wide, not one-off:

1. **K&R old-style definition** for the dead second `pool` parameter,
   against the same unspecified-parameter declaration in `BMemPMgr.h`
   (`extern void *BMemPMgrFree();`, already in place from `BMemPMgrAlloc`'s
   round).
2. **Reuse an existing pointer's register instead of a fresh local** where
   retail does. Here it's not a `+=` on the parameter (unlike
   `BMemPMgrAlloc`'s `size`) but the SAME idea applied to `header`: on the
   coalesce-with-previous path, retail reloads directly into the SAME
   register that held `header` (`lw a2,-8(s0)`, reusing `a2`) rather than
   loading into a new register and moving it. The equivalent C is a plain
   reassignment of the existing pointer variable, not a second named local:
   `header = *(BMemBlockHdr **)((u8 *)ptr - 8);` — and critically, computed
   FROM THE ORIGINAL ARGUMENT (`ptr`), not from the about-to-be-overwritten
   `header` (`(u8 *)header - 4` is the same address but round-tripping
   through the soon-dead `header` variable cost an extra `move`).
3. **Per-`if` scoped hoist of the twice-used free-list `prev`/`next` pair**,
   applied to BOTH unlink sites (one for the coalesce-with-previous case,
   one for the coalesce-with-next case) — same shape as `BMemPMgrAlloc`'s
   lever 3, confirmed to generalize to a second, independently-written pair
   of unlink sites in a sibling function.

**New wrinkle found here, not present in `BMemPMgrAlloc`:** the SIZE value
being folded into the coalesced header must be captured into a local BEFORE
reassigning the pointer that owned it, since the pointer swap destroys access
to the original field:

```c
if ((s32)header->sizeAndFlags < 0) {
    u32 freedSize = header->sizeAndFlags & 0xFFFFFFF;

    header = *(BMemBlockHdr **)((u8 *)ptr - 8);
    header->sizeAndFlags = (header->sizeAndFlags & 0xF0000000)
        | (freedSize + (header->sizeAndFlags & 0xFFFFFFF));
    ...
}
```

## The per-if hoist direction is NOT a fixed rule across blocks — measured, not guessed

`BMemPMgrAlloc`'s report already flagged that which of `{n, p}` should be
declared first to land the CONDITION variable in `v1` differs between two
structurally-identical-looking `if` pairs, with no visible source-level
explanation. This function reproduces the same fact a second time,
independently: of its FOUR per-if hoists (two coalesce sites × two unlink
checks each), the winning declaration order was `{n, p}` / `{p, n}` /
`{p, n}` / `{p, n}` — three of the four wanted `prev` declared first, one
wanted `next` first, and the only way found to determine which was building
each one and reading the diff. Do not assume a pattern from one instance;
verify each site.

## The one remaining residue: reversing operand order in one expression cascades badly elsewhere

The coalesce-with-next size fold (`header->sizeAndFlags = (header->sizeAndFlags
& 0xF0000000) | ((next->sizeAndFlags & 0xFFFFFFF) + (header->sizeAndFlags &
0xFFFFFFF));`, file `0x85BC`-`0x85E4`) has retail loading `next`'s field
BEFORE `header`'s (opposite of the natural left-to-right text order of that
expression, where `header` appears first). **Introducing a `u32 nextSize =
next->sizeAndFlags & 0xFFFFFFF;` local ahead of the assignment fixed the
load order and gained 3 words (96→99/107) with no side effects elsewhere.**
But every attempt to go further and ALSO match retail's exact mask-register
reuse (retail computes the `0xFFFFFFF` mask ONCE and applies it to BOTH
`next`'s and `header`'s size fields; my C's second `& 0xFFFFFFF` gets its own
fresh mask materialisation) caused a severe regression, not a local one:

- Restructuring the whole assignment as `(next_size + header_size) |
  header_flags` (moving the OR's operand order, not just adding a local):
  **57/107**, with the register renaming cascading past this expression
  into `mgr`'s own register (`t0`→`a3`) and the pool-node's (`a3`→`a2`)
  further down the function — a completely different, worse allocation for
  code that doesn't even touch this expression.
- The same reorder again after other fixes were in place: **59/107**,
  same cascade.
- Naming the shared mask (`u32 mask = 0xFFFFFFF;`, reused at both sites):
  **no change from 99/107** — GCC 2.6.3 did not share the register across
  the statement boundary regardless.
- Splitting into three named locals (`nextSize`, `headerFlags`,
  `headerSize`, computed in retail's own read order): **no change from
  99/107**.
- A single `combinedSize` local folding both terms in one statement: **back
  to 59/107**, the same cascade as the OR-reorder attempt.

**The lesson, stated for reuse:** at this residue, ANY restructuring that
touches which operand of the outer `|`/`+` is textually first reliably
regresses hard (tested 4 times, same failure mode every time), while
restructuring that keeps the outer shape and only changes WHEN a sub-value
gets read (via an extra local) is safe and can still help. This looks like
the same class of GCC 2.6.3 scheduler/allocator behavior documented as
unresolved in `BMemPMgrAlloc`'s report — not obviously fixable from the C
side, a permuter candidate.

## Struct layout

Same `BMemBlockHdr`/`BMemPMgr` as `BMemPMgrAlloc` — no new fields discovered
here; every offset this function touches was already established.

## Preserved near-miss body (99/107, length-exact)

```c
#if 0
void *BMemPMgrFree(ptr, pool)
    void *ptr;
    void *pool;
{
    BMemPMgr *mgr;
    BMemBlockHdr *header;
    BMemBlockHdr *next;
    u32 nextFree;

    SetBMemPMgrBusy(1);
    mgr = gDefaultBMemPMgr;
    if (mgr == NULL) {
        mgr = pool;
    }
    if (ptr != NULL) {
        header = (BMemBlockHdr *)((u8 *)ptr - 4);
        next = (BMemBlockHdr *)((u8 *)header + (header->sizeAndFlags & 0xFFFFFFF));
        nextFree = next->sizeAndFlags & 0x40000000;
        if ((s32)header->sizeAndFlags < 0) {
            u32 freedSize = header->sizeAndFlags & 0xFFFFFFF;

            header = *(BMemBlockHdr **)((u8 *)ptr - 8);
            header->sizeAndFlags = (header->sizeAndFlags & 0xF0000000)
                | (freedSize + (header->sizeAndFlags & 0xFFFFFFF));
            {
                BMemBlockHdr *n = header->next;
                BMemBlockHdr *p = header->prev;

                if (p != NULL) {
                    p->next = n;
                } else {
                    mgr->freeListEnd = n;
                }
            }
            {
                BMemBlockHdr *p = header->prev;
                BMemBlockHdr *n = header->next;

                if (n != NULL) {
                    n->prev = p;
                } else {
                    mgr->freeListStart = p;
                }
            }
        }
        if (nextFree) {
            u32 nextSize = next->sizeAndFlags & 0xFFFFFFF;

            header->sizeAndFlags = (header->sizeAndFlags & 0xF0000000) | (nextSize + (header->sizeAndFlags & 0xFFFFFFF));
            {
                BMemBlockHdr *n = next->next;
                BMemBlockHdr *p = next->prev;

                if (p != NULL) {
                    p->next = n;
                } else {
                    mgr->freeListEnd = n;
                }
            }
            {
                BMemBlockHdr *p = next->prev;
                BMemBlockHdr *n = next->next;

                if (n != NULL) {
                    n->prev = p;
                } else {
                    mgr->freeListStart = p;
                }
            }
            next = (BMemBlockHdr *)((u8 *)header + (header->sizeAndFlags & 0xFFFFFFF));
        }
        header->prev = mgr->freeListStart;
        mgr->freeListStart = header;
        header->next = NULL;
        if (header->prev != NULL) {
            header->prev->next = header;
        } else {
            mgr->freeListEnd = header;
        }
        *(BMemBlockHdr **)((u8 *)next - 4) = header;
        header->sizeAndFlags |= 0x40000000;
        next->sizeAndFlags |= 0x80000000;
    }
    SetBMemPMgrBusy(0);
    return NULL;
}
#endif
```

## Next steps (round 45; resolved round 46 — see top of report)

- The single remaining residue was a strong permuter candidate: length was
  exact, the residue confined to one 8-word cluster (`0x85BC`-`0x85E4`), and
  four independent hand-attempts at reordering all failed the same way. This
  is exactly what round 46's search closed — see the top of this report.

### Proposed learning (round 45)

- **`BMemPMgrAlloc`'s three levers generalize.** All three transferred to
  this sibling function with no modification needed beyond the obvious
  per-function field/variable renaming. Worth treating as the default
  starting shape for any further `BMemPMgr`/`TmdRenderer` pool-management
  function, not something to re-derive.
- **When reassigning a pointer variable to a NEW value derived from
  something OTHER than its own old value, capture any field of the OLD
  value you still need into a local FIRST.** `header = *(...)` needs
  `header`'s old `sizeAndFlags` size portion for the very next statement;
  computing it after the reassignment reads the WRONG object's field.
- **A per-if hoist's winning declaration order must be verified per
  site, not assumed from a sibling site** — confirmed a second time,
  independently, in a different function.
- **A register-identity residue that cascades hard the moment you touch
  outer operand order, but responds cleanly to an added local that changes
  nothing about final values, is worth distinguishing explicitly**: the
  first is almost never recoverable by more hand-editing (measured 4/4 this
  round), the second reliably is.

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

`BMemPMgrFree`, **tier A**: fully derived free-list release (round 46,
99->107/107 permuter match) -- coalesces with the previous and following
blocks and re-links the free list, the mirror of `BMemPMgrAlloc`. Called
across the same wide set of units as `BMemPMgrAlloc` (`code_2cc8c_f`,
`code_55dd4`, `code_d294`, `TmdRenderer`, `main`, plus this unit's own
`RemoveBasicClassListNode`), confirming the general-purpose pool pairing.

## Polish (round 97, runner delta)

Zero bytes changed; whole-image SHA1 green after each step.

- Pool parameter typed `BMemPMgr *`. The `nextSize` reuse and the
  per-unlink scoped `n`/`p` keep `/* MATCHING: */` lines; the permuter
  history that was in the source comment is in this report's round-46
  section already.
- `(s32)header->sizeAndFlags < 0` -> `header->sizeAndFlags & BMEM_PREV_FREE`,
  byte-identical.
- `*(BMemBlockHdr **)((u8 *)ptr - 8)` -> `BMEM_PREV_FOOTER(header)`, i.e.
  `(u8 *)header - 4`, byte-identical in the matched body. Round 45's lever 2
  above (computing it from `ptr` saves a `move`) was measured on the 99/107
  body and no longer holds in the 107/107 one: `funcdiff.py` 107/107 and the
  whole image green with the `header` form.
- The other literals and offsets: `BMEM_HEADER_OF`, `BMEM_NEXT_BLOCK`,
  `BMEM_BLOCK_SIZE`, `BMEM_FLAG_MASK`, `BMEM_FREE`, `BMEM_PREV_FREE`.
- Fields: `freeListStart`/`freeListEnd` -> `freeListTail`/`freeListHead`
  (evidence in BMemPMgrAlloc.md, "Polish").

The constants are defined, with their evidence, in `include/BMemPMgr.h`:
`BMEM_SIZE_MASK` 0x0FFFFFFF and `BMEM_FLAG_MASK` 0xF0000000 split
`sizeAndFlags`; `BMEM_FREE` 0x40000000 is set on every block put on the free
list (SetupBMemPMgrFreeList, BMemPMgrFree, the split remainder) and cleared
when BMemPMgrAlloc takes one; `BMEM_PREV_FREE` 0x80000000 is set on the block
above a freed one and on the sentinel, cleared on the block above an allocated
one, and tested by BMemPMgrFree before it reads the lower neighbour's footer.
