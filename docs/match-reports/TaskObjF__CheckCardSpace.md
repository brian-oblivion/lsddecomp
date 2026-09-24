# TaskObjF__CheckCardSpace — MATCH (28/28 words)

> Renamed from `func_8004EC5C` on 2026-09-24 (tools/rename.py). Address 0x8004ec5c.

**Unit:** class_3bb8c_e (round 14, `Node3bb8cE` class).

## What it does

`s32 TaskObjF__CheckCardSpace(Node3bb8cE *self, u8 id, s32 sizeArg)`. Retries up to
10 times, calling `TaskObjF__ProbeCardFreeSpace(self, id, sizeArg)` until it returns
nonzero. Structurally identical to `TaskObjF__FormatCard`'s retry skeleton.

## Where it stood, and the fix

Reached 17/28 first (worse after trying a `__asm__("")` barrier, which
regressed further) with `id` declared `s32` and truncated inline at the
call site (`TaskObjF__ProbeCardFreeSpace(self, (u8)id, sizeArg)`) — the three
callee-saved registers holding `self`/`id`/`sizeArg` came out in the wrong
relative numbering (retail: `self`->`$s2`, `id`->`$s1`, `sizeArg`->`$s3`;
mine had a different assignment), a pure register-identity residue with
zero address drift.

**Declaring the parameter itself as `u8 id`** (matching
`TaskObjF__ProbeCardFreeSpace`'s real parameter width, rather than `s32 id` narrowed by
an explicit cast at the call site) matched immediately:

```c
extern s32 TaskObjF__ProbeCardFreeSpace(Node3bb8cE *self, u8 id, s32 sizeArg);

s32 TaskObjF__CheckCardSpace(Node3bb8cE *self, u8 id, s32 sizeArg)
{
    s32 retries;
    s32 result;

    retries = 10;
    do {
        result = TaskObjF__ProbeCardFreeSpace(self, id, sizeArg);
    } while (result == 0 && retries-- != 0);
    return result;
}
```

### Proposed learning

**A byte-narrowed value that's forwarded verbatim to a callee wants the
narrow TYPE on the parameter itself, not a narrowing CAST at the call
site.** `(u8)id` at the call site and `u8 id` as the parameter declaration
are semantically identical C, but they gave GCC 2.6.3 different register
allocation orderings for this function's three live-across-the-call
values -- the inline cast version put `self` and `id` in swapped
registers relative to retail; the narrow-parameter version matched
immediately. Try the narrow parameter type before chasing this class of
residue with barriers or declaration reordering (both tried here first
and made things worse).
