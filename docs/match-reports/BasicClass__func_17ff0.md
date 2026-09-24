# BasicClass__func_17ff0

**Unit:** code_8220 · **Size:** 20 instructions · **Status:** MATCHED (20/20 words)

BasicClass vtable slot `+0x014` (`removeChild`) — see
`BasicClass__BasicClass.md` for the class's overall design, and
`BasicClass__func_17f98.md` (`addChild`) for the symmetric add operation
this one undoes.

## What it does

`RemoveChild`: finds and unlinks/frees the list node in `self->children`
whose `value == child` (via `RemoveBasicClassListNode`, still `INCLUDE_ASM` this round
but its extern signature is established — see below), then UNCONDITIONALLY
(no result check, unlike `addChild`'s success-gated notify) dispatches the
child's own vtable slot `+0x024` (`removeParentRef`) to drop `self` from the
child's `parentRefs` back-reference list.

## The C

```c
void BasicClass__func_17ff0(BasicClass *self, BasicClass *child)
{
    RemoveBasicClassListNode(&self->children, child);
    child->methods->removeParentRef(child, self);
}
```

## `RemoveBasicClassListNode`'s signature, established here — genuinely `void`

`RemoveBasicClassListNode(BasicClassListNode **head, BasicClass *value)` returning
`void`. This one took more evidence than `PushBasicClassListNode` to type
correctly: its own body leaves `$v0` holding one of THREE different
things depending on path (the freed node's pool-free result on the
match-and-remove path; untouched loop-scan garbage on the not-found path),
which cannot be a real, usable return value — no caller could rely on it.
Confirmed void from the USE side too: this function calls it and never
reads `$v0` afterward, and `BasicClass__func_18040`'s own dispatch through
`removeChild` (this function, at slot `+0x014`) likewise discards whatever
comes back. See `include/code_8220.h` for the full declaration.

## Provenance

round 11 (2026-09-03), runner delta, unit code_8220 (fresh carve). One
collateral-drift residue (1 word, the `jal RemoveBasicClassListNode` target address)
resolved itself once `BasicClass__func_18040` reached its correct size —
see that function's own report; nothing needed changing here.
