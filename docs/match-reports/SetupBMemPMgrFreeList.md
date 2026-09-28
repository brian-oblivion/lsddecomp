# SetupBMemPMgrFreeList — MATCHED (round 45)

> Renamed from `func_80017AC8` on 2026-09-24 (tools/rename.py). Address 0x80017ac8.

**Unit:** `BMemPMgr` · **Size:** 27 words · **Status:** MATCHED, 27/27 words, byte-exact.

## Verdict correction

Filed as `gp_rel`-blocked before round 42; reopened round 42
(2026-09-15). Rebuilt this round: byte-exact on the **first** attempt.

**One part of the old stub's structural read was wrong, not just its
verdict**, and it's worth flagging since the same misreading could have
propagated into `BMemPMgrAlloc`/`BMemPMgrFree`'s still-open reports: it
said this function "stores `pool` itself (self-pointer) into two places
at `pool+0x1C+4` and `pool+0x1C+8`". The actual writes to those two words
are `header->prev = NULL` and `header->next = NULL` (zero, not a
self-pointer) — see the C below. The self/back-pointer write that DOES
happen is different: the pool's own `freeListStart`/`freeListEnd` fields
(`pool+0x8`/`pool+0xC`, previously untyped) get set to `header`
(`pool+0x1C`), and separately, a boundary sentinel 4 bytes before the end
of the pool area gets `header`'s address written into it for backward
coalescing. Both of those are genuinely "store this block's address
somewhere", which is presumably how the old report's reading happened —
right shape, wrong two locations.

## The function

```c
void SetupBMemPMgrFreeList(BMemPMgr *pool)
{
    BMemPMgr *mgr;
    BMemBlockHdr *header;
    u8 *end;

    mgr = sDefaultBMemPMgr;
    if (mgr == NULL) {
        mgr = pool;
    }
    mgr->unk10 = 1;
    header = mgr->freeListHead;
    mgr->freeListStart = header;
    mgr->freeListEnd = header;
    header->sizeAndFlags = mgr->poolSize | 0x40000000;
    mgr->freeListStart->prev = NULL;
    mgr->freeListEnd->next = NULL;
    end = (u8 *)header + (header->sizeAndFlags & 0xFFFFFFF);
    *(BMemBlockHdr **)(end - 4) = header;
    *(u32 *)end = 0x80000000;
}
```

`BMemBlockHdr` (new, `include/bmem_pmgr.h`) is the pool's free-list node
shape: `sizeAndFlags` (28-bit size, top 4 bits flags — `0x40000000` =
free) at +0x0, doubly-linked `prev`/`next` at +0x4/+0x8. `BMemPMgr` grew
three fields this round to match what this function actually touches:
`freeListStart`/`freeListEnd` (+0x8/+0xC, the free list's head/tail) and
`unk10` (+0x10, set to 1 here, not yet read by any decoded function).

**Why the two reloads of `mgr->freeListStart`/`mgr->freeListEnd` matter
for matching, not just style.** The obvious C would reuse the local
`header` variable for the `prev`/`next` writes too
(`header->prev = NULL; header->next = NULL;`), and that does NOT match:
GCC 2.6.3 -O2 has no reason to fold `mgr->freeListStart` back to the
already-in-register `header` once they're written as two different
lvalues, so it reloads both fields from memory. Writing the source as
`mgr->freeListStart->prev = NULL; mgr->freeListEnd->next = NULL;`
reproduces those two reloads exactly. Same reasoning for the final size
computation: it re-reads `header->sizeAndFlags` from memory (the word
just written) rather than reusing the local `mgr->poolSize | 0x40000000`
expression, because the source expresses it as `header->sizeAndFlags`,
not a kept-around local.

`mgr->freeListHead` stays typed `void *` (unchanged) — retyping it to
`BMemBlockHdr *` was tried and rejected as unnecessary churn on an
already-matched `BMemPMgrInit`'s `pool->freeListHead = (u8 *)pool + 0x1C;`
assignment; `void *` still converts implicitly into the local `header`
variable with no cast needed.

The `pool` argument is genuinely used only as the `sDefaultBMemPMgr == NULL`
fallback, confirmed byte-exact — the old report's read of the calling
convention (one real argument; `$a1`'s "fallback pool" role) was correct
and needed no revision.

## Provenance

round 45 (2026-09-15), runner alpha. `./build-and-verify.sh` green,
`tools/funcdiff.py SetupBMemPMgrFreeList` reports 27/27 words match, one attempt.

### Proposed learning

**A stale stub's prose summary of "what a store does" is not itself a
derivation — verify it against the actual offsets before reusing it.**
This report's own predecessor stated a plausible-sounding but wrong
target for two of the four writes it described (self-pointer vs. NULL,
and at the wrong offsets). The blocker-screen boilerplate around it was
accurate; the one paragraph of semantic reading was not, and nothing
about being "gp_rel-blocked" made that paragraph any less checkable at
the time it was written — the disassembly was already sitting right
there. Read the actual instruction operands, not just the report's prose,
even when a report is otherwise well-sourced.

## Naming (round 74)

`SetupBMemPMgrFreeList`, **tier A**: every write in the body is fully
derived (round 45 above) -- it seeds `freeListStart`/`freeListEnd` from
`freeListHead`, marks the whole pool area as one free block, and plants
the two boundary sentinels used for coalescing. Called only by
`BMemPMgrInit` (this unit), which is exactly what the name says: the
free-list half of pool setup.

## Polish (round 97, runner delta)

Zero bytes changed.

- `u8 *end` -> `BMemBlockHdr *sentinel` (`BMEM_NEXT_BLOCK(header)`): the
  word past the pool is a zero-size block header whose only content is
  `BMEM_PREV_FREE`, which stops BMemPMgrFree's upward merge at the pool's end.
  `*(BMemBlockHdr **)(end - 4)` -> `BMEM_PREV_FOOTER(sentinel)`.
- Fields: `freeListHead` (+0x0) -> `firstBlock` (it is the pool's first
  block, written by BMemPMgrInit, not a free-list end);
  `freeListStart`/`freeListEnd` -> `freeListTail`/`freeListHead`; `unk10` ->
  `initialized`, tier B: this function writes 1 to it and no code reads it.
- The `include/bmem_pmgr.h` comment that called this function
  "gp_rel-blocked" is gone: that blocker was resolved (CLAUDE.md) and the
  function is matched.

The constants are defined, with their evidence, in `include/bmem_pmgr.h`:
`BMEM_SIZE_MASK` 0x0FFFFFFF and `BMEM_FLAG_MASK` 0xF0000000 split
`sizeAndFlags`; `BMEM_FREE` 0x40000000 is set on every block put on the free
list (SetupBMemPMgrFreeList, BMemPMgrFree, the split remainder) and cleared
when BMemPMgrAlloc takes one; `BMEM_PREV_FREE` 0x80000000 is set on the block
above a freed one and on the sentinel, cleared on the block above an allocated
one, and tested by BMemPMgrFree before it reads the lower neighbour's footer.
