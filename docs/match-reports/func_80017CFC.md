# func_80017CFC — STALL (round 45): length exact (107/107 words), 99/107 words match, first real diff at 0x80017DBC

**Unit:** `code_8220` · **Size:** 107 words, EXACT. **Status:** STALL after
~10 attempts, restored to `INCLUDE_ASM`. Best raw score 99/107 words (92.5%);
`tools/asm-differ/diff.py func_80017CFC` (realigned) confirms zero drift and
the first real difference at file offset `0x85BC` / vram `0x80017DBC`.

## Verdict correction on the predecessor report

Filed as `gp_rel`-blocked, reopened round 42. This round attempted it for the
first time. The prior report's functional read (pool `free`, mirror of
`func_80017B34`, same critical-section lock, coalesce-with-adjacent-free-block
logic, same packed size+flags header) was correct and is confirmed exactly by
the derivation below — including the ONE detail it left open ("depending on
the header's high bit... either coalesces... or inserts... as a new
standalone entry", which undersells it: it can do BOTH coalesces, with the
previous block and the next block, not an either/or).

## The three func_80017B34 levers all transferred directly

This function needed exactly the same three structural levers as its sibling
`func_80017B34` (`docs/match-reports/func_80017B34.md`), confirming they are
reusable project-wide, not one-off:

1. **K&R old-style definition** for the dead second `pool` parameter,
   against the same unspecified-parameter declaration in `code_8220.h`
   (`extern void *func_80017CFC();`, already in place from `func_80017B34`'s
   round).
2. **Reuse an existing pointer's register instead of a fresh local** where
   retail does. Here it's not a `+=` on the parameter (unlike
   `func_80017B34`'s `size`) but the SAME idea applied to `header`: on the
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
   one for the coalesce-with-next case) — same shape as `func_80017B34`'s
   lever 3, confirmed to generalize to a second, independently-written pair
   of unlink sites in a sibling function.

**New wrinkle found here, not present in `func_80017B34`:** the SIZE value
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

`func_80017B34`'s report already flagged that which of `{n, p}` should be
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
unresolved in `func_80017B34`'s report — not obviously fixable from the C
side, a permuter candidate.

## Struct layout

Same `BMemBlockHdr`/`BMemPMgr` as `func_80017B34` — no new fields discovered
here; every offset this function touches was already established.

## Preserved near-miss body (99/107, length-exact)

```c
#if 0
void *func_80017CFC(ptr, pool)
    void *ptr;
    void *pool;
{
    BMemPMgr *mgr;
    BMemBlockHdr *header;
    BMemBlockHdr *next;
    u32 nextFree;

    func_8001844C(1);
    mgr = D_8008A818;
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
    func_8001844C(0);
    return NULL;
}
#endif
```

## Next steps for whoever reopens this

- The single remaining residue is a strong permuter candidate: length is
  exact, the residue is confined to one 8-word cluster (`0x85BC`-`0x85E4`),
  and four independent hand-attempts at reordering all failed the same way.
- Do not re-attempt the `gp_rel` framing.

### Proposed learning

- **`func_80017B34`'s three levers generalize.** All three transferred to
  this sibling function with no modification needed beyond the obvious
  per-function field/variable renaming. Worth treating as the default
  starting shape for any further `code_8220`/`code_8220_b` pool-management
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
