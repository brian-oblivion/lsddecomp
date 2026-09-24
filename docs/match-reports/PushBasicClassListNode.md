# PushBasicClassListNode

> Renamed from `func_800181AC` on 2026-09-24 (tools/rename.py). Address 0x800181ac.

**Unit:** code_8220 · **Size:** 23 instructions · **Status:** MATCHED (23/23 words)

The `BasicClassListNode` list-push primitive, used by both of BasicClass's
lists (`children` via `BasicClass__AddChild`, `parentRefs` via
`BasicClass__AddParentRef`) — see `BasicClass__BasicClass.md` for the
class's overall design. Both call sites and this function's own
signature were provisionally established last round while matching those
two callers (still `INCLUDE_ASM` at the time); this round derives the
body itself.

## What it does

Allocates an 8-byte `BasicClassListNode` from the pool
(`BMemPMgrAlloc(0x8)`); on success, sets `node->value = value`,
`node->next = *head` (the previous head), `*head = node` (prepend), and
returns `1`. On allocation failure, returns `0` without touching `*head`.

## The C

```c
s32 PushBasicClassListNode(BasicClassListNode **head, BasicClass *value)
{
    BasicClassListNode *node;
    BasicClassListNode *oldHead;

    node = BMemPMgrAlloc(0x8);
    if (node != NULL) {
        oldHead = *head;
        node->value = value;
        node->next = oldHead;
        *head = node;
        return 1;
    }
    return 0;
}
```

## One branch-polarity flip needed

First attempt used the inverted-guard/early-return shape (`if (node ==
NULL) return 0; ...push...; return 1;`), by reflex — DECOMPILATION_LEARNINGS
generally prefers this shape for a small early exit. It scored 16/23: the
`bnez`/`beqz` sense was flipped from retail (retail branches FORWARD to the
success block on `node != NULL`, i.e. the push code is the branch TARGET
and the `return 0` is the fall-through, not the other way round), and the
mis-ordered branch dragged the whole push block's physical position with
it. Flipping to the positive-`if` form (`if (node != NULL) { ...; return
1; } return 0;`) matched immediately, 23/23 — unlike `BMemPMgrInit`'s
similar-looking guard (see that report), there was no accompanying
register-vs-constant residue here, because this function returns a literal
`0`/`1` on both paths rather than reusing a just-nulled pointer register.
Worth remembering as the negative case: the "always invert small early
exits" heuristic is a default, not a rule, and checking the actual branch
sense (which block is the fall-through) before writing the guard saves an
attempt.

## Provenance

round 11 (2026-09-03), runner delta, unit code_8220, second pass. 2
attempts.

## Naming (round 74)

`PushBasicClassListNode`, **tier A**: allocates an 8-byte `BasicClassListNode`
from `BMemPMgrAlloc` and prepends it to `*head`; used identically for both
of `BasicClass`'s lists (`children` via `BasicClass__AddChild`/AddChild,
`parentRefs` via `BasicClass__AddParentRef`/AddParentRef), confirming it is
the shared list-push primitive rather than something list-specific.
