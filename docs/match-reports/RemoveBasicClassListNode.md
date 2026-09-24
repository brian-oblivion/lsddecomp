# RemoveBasicClassListNode

> Renamed from `func_80018208` on 2026-09-24 (tools/rename.py). Address 0x80018208.

**Unit:** code_8220 · **Size:** 32 instructions · **Status:** MATCHED (32/32 words)

The `BasicClassListNode` list find-unlink-free primitive, used by both of
BasicClass's lists (`children` via `BasicClass__func_17ff0`, `parentRefs`
via `BasicClass__func_1811c`) — see `BasicClass__BasicClass.md` for the
class's overall design. Both call sites and this function's `void`
signature were provisionally established last round while matching those
two callers (still `INCLUDE_ASM` at the time); this round derives the
body itself.

## What it does

Walks `*head` looking for the first node whose `value == value`; if
found, unlinks it (updating `*head` if it was the list head, or the
previous node's `next` otherwise) and frees it via `BMemPMgrFree`. Does
nothing if no match is found.

## The C

```c
void RemoveBasicClassListNode(BasicClassListNode **head, BasicClass *value)
{
    BasicClassListNode *prev;
    BasicClassListNode *node;

    prev = NULL;
    node = *head;
    while (node != NULL) {
        if (node->value == value) {
            if (prev != NULL) {
                prev->next = node->next;
            } else {
                *head = node->next;
            }
            BMemPMgrFree(node);
            return;
        }
        prev = node;
        node = node->next;
    }
}
```

## One branch-polarity flip needed, same family as PushBasicClassListNode's

First attempt wrote the intuitively-ordered `if (prev == NULL) { *head =
...; } else { prev->next = ...; }` — reads naturally as "handle the head
case first" — and it scored 29/32 with exactly three words wrong: the
outer `beqz`/`bnez` sense on `prev` was inverted (retail's fall-through is
the `prev != NULL` case, not `prev == NULL`), and the two assignment
bodies landed in each other's basic blocks as a direct consequence.
Swapping to test `prev != NULL` first fixed all three words in one edit —
consistent with DECOMPILATION_LEARNINGS' "a two-armed if/else's LAYOUT and
its VALUE-PER-ARM are independently wrong-able": here it was purely a
layout/polarity issue, the per-arm values were already correct (the
diff was two SWAPPED words, not two wrong ones).

## Provenance

round 11 (2026-09-03), runner delta, unit code_8220, second pass. 2
attempts.

## Naming (round 74)

`RemoveBasicClassListNode`, **tier A**: finds the node in `*head` whose
`value == value`, unlinks it, and releases it via `BMemPMgrFree`; used
identically for both of `BasicClass`'s lists (`children` via
`BasicClass__func_17ff0`/RemoveChild, `parentRefs` via
`BasicClass__func_1811c`/RemoveParentRef), the release-side mirror of
`PushBasicClassListNode`.
