# BasicClass__func_1811c

**Unit:** code_8220 · **Size:** 8 instructions · **Status:** MATCHED (8/8 words)

BasicClass vtable slot `+0x024` (`removeParentRef`) — see
`BasicClass__BasicClass.md` for the class's overall design, and
`BasicClass__RemoveChild.md` (`removeChild`) for the caller that dispatches
through this slot on a child object.

## What it does

A one-line wrapper: tail-calls `RemoveBasicClassListNode(&self->parentRefs, parent)`
— finds, unlinks and frees the list node in `self`'s back-reference list
whose value is `parent`.

## The C

```c
void BasicClass__func_1811c(BasicClass *self, BasicClass *parent)
{
    RemoveBasicClassListNode(&self->parentRefs, parent);
}
```

## Why `void`, not a `return`

Same shape as `BasicClass__AddParentRef` (its `addParentRef` sibling), but
the OPPOSITE typing conclusion: `RemoveBasicClassListNode` (matched this round, see
its own report) is genuinely `void` — its own body leaves `$v0` holding
one of several unusable values depending on which path is taken, and no
caller anywhere in this unit reads its return. A plain statement call,
not a `return`, matched first attempt.

## Provenance

round 11 (2026-09-03), runner delta, unit code_8220, second pass. Matched
first attempt.
