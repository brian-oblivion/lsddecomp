# FreeBasicClassList — MATCHED (17/17 words)

> Renamed from `func_80018288` on 2026-09-17 (tools/rename.py). Address 0x80018288.

Unit: `src/TmdRenderer.c`. Signature (already in `include/code_8220.h`):
`void FreeBasicClassList(BasicClassListNode **head);` — free every node in a
`BasicClassListNode` singly-linked list, without clearing `*head` itself.

## Final source

```c
void FreeBasicClassList(BasicClassListNode **head)
{
    BasicClassListNode *node = *head;

    while (node != NULL) {
        BasicClassListNode *cur = node;
        node = node->next;
        BMemPMgrFree(cur);
    }
}
```

## Notes

First attempt used the more obvious shape (`next` computed, `BMemPMgrFree(node)`
called, then `node = next`) and built one word too long: 6/17 words matching,
with the byte-length mismatch shifting everything after it in the unit. The
extra instruction was a `move a0,s0` needed because that version tests the
new `node`/`next` value directly for the loop condition register, whereas
retail's loop tests the SAME register (`$s0`) that already holds the
about-to-be-freed node, and prepares the call argument in the branch's own
delay slot from the previous iteration.

Reordering to `cur = node; node = node->next; BMemPMgrFree(cur);` (the
"cur" temp copied BEFORE advancing) reproduced retail exactly — 2nd attempt,
byte-exact. Retail's compiled loop:

```
lw    s0, 0($s0)      ; node = node->next (in loop body, overwrites s0 early)
jal   BMemPMgrFree    ; free(a0), a0 set from PRIOR iteration's delay slot
bnez  s0, loop          ; test node directly
 move a0, s0            ; delay slot: a0 = node, for the NEXT free() call
```

### Proposed learning

For a "free every node in a linked list" loop where the loop test and the
freed pointer are the SAME logical value at different points in time, write
`cur = node; node = node->next; free(cur);` (temp-then-advance-then-use), not
`next = node->next; free(node); node = next;` (advance-then-use-via-second-
name). Both are semantically identical and equally "obvious" C, but only the
first reproduces GCC 2.6.3's choice to test the advanced pointer directly in
the loop condition register rather than moving it to a second register first.
Confirmed once (`FreeBasicClassList`, 6/17 -> 17/17 words).

## Naming (round 51, bravo)

`func_80018288` -> `FreeBasicClassList`. **Tier A** -- a pure leaf whose
mechanics are its purpose.

Evidence: the body frees every node of a `BasicClassListNode` chain through
the pool allocator and nothing else. Its two callers in `code_8220.c` are
`BasicClass__RemoveAllChildren` (removeAllChildren) and `BasicClass__ClearParentRefs`
(clearParentRefs), and both do `FreeBasicClassList(&self->list);
self->list = NULL;` -- i.e. the caller, not this function, clears the head
pointer. The name says "free the list", and the "does not clear `*head`"
caveat stays in the declaration comment in `include/code_8220.h` because
it is the one thing the name cannot carry.
