# TaskObjF__CollectExistingMemcardFiles — MATCH (53/53 words)

> Renamed from `func_8004EB88` on 2026-09-24 (tools/rename.py). Address 0x8004eb88.

**Unit:** title_menu (round 14, `Node3bb8cE` class). This was one of
the two functions predicted hardest this round (6 distinct callee-saved
registers, the round's saturation threshold), yet it matched on the
first attempt — see below for why.

## What it does

`s32 TaskObjF__CollectExistingMemcardFiles(Node3bb8cE *self, s32 *values, char **outArr, char
*middle, char **entries)` (`entries` is the 5th/stack argument). Walks
the NULL-terminated `entries` array in lockstep with `values` (a parallel
`s32` array, only advanced on a match) and `outArr` (an output array,
only advanced on a match): builds `buf = middle + entries[i]`, tests it
via `self->methods->slot54(self, *values, buf)`, and on a NONZERO result
(the opposite polarity from `TaskObjF__FindUnusedMemcardName`'s zero-means-match
convention — same slot, different meaning per call site, consistent with
this project's established per-call-site convention) increments a count,
stores `*entries` into `*outArr`, and advances both `values` and
`outArr`. `entries` itself always advances. Returns the final count.

## Result

Matched immediately by applying `TaskObjF__FindUnusedMemcardName`'s freshly-derived lesson
(a plain top-tested `while (*entries != NULL)` loop, not an `if`-guard
plus `do-while`) from the start, rather than rediscovering it:

```c
s32 TaskObjF__CollectExistingMemcardFiles(Node3bb8cE *self, s32 *values, char **outArr, char *middle, char **entries)
{
    s32 count;
    char buf[0x20];

    count = 0;
    while (*entries != NULL) {
        strcpy(buf, middle);
        strcat(buf, *entries);
        if (self->methods->slot54(self, *values, buf) != 0) {
            count++;
            *outArr = *entries;
            values++;
            outArr++;
        }
        entries++;
    }
    return count;
}
```

### Proposed learning

**The predicted-hard classifier (5+ distinct callee-saved registers ->
likely stall) does not fire when the function is a near-identical
structural sibling of an already-matched function in the same unit.**
This function needed all 6 of `$s0`-`$s5` (matching the CLAUDE.md
prediction exactly) and still matched on the first try, because its
control-flow skeleton is byte-for-byte the same shape as
`TaskObjF__FindUnusedMemcardName`'s (already solved this round) — only the per-entry body
and loop-carried state differ. Register pressure predicts register-
identity stalls; it says nothing about whether the CONTROL FLOW shape is
already known. Check for a same-unit sibling with matching structure
before treating a high register count as a reason to defer a function.

**Same vtable slot (`+0x054`), opposite success polarity, at two
different call sites** — `TaskObjF__FindUnusedMemcardName` treats `slot54(...) == 0` as
"found, stop"; this function treats `slot54(...) != 0` as "keep this
one, count it." Both are correct readings of their own disassembly;
neither generalizes to the other. Consistent with the project's
established "arity/typing is a per-call-site property" family, extended
here to return-value POLARITY as well as arity.

## Naming (round 78, track 3)

`func_8004EB88` -> `TaskObjF__CollectExistingMemcardFiles`. **Tier B.** Sits at `gTaskObjFMethods` +0x05C, the sibling slot to `TaskObjF__FindUnusedMemcardName` (+0x058) with the inverted test: walks the same kind of `entries` array, and for each candidate where `self->methods->slot54(self, *values, buf) != 0` (found, with a real per-entry `destBuf` this time), records the matched entry pointer into `outArr`, advances `values`, and increments the returned count. Mechanics clear (collects the existing files among the candidates, reading each one's header into its own caller-supplied buffer); the caller-side use of the collected list is outside this unit, tier B.
